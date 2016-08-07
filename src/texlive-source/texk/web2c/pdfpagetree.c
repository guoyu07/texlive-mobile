/*1:*/
#line 20 "./luatexdir/pdf/pdfpagetree.w"


#include "ptexlib.h"

/*:1*//*3:*/
#line 26 "./luatexdir/pdf/pdfpagetree.w"

# define PAGES_TREE_KIDSMAX 10

static struct avl_table*divert_list_tree= NULL;

typedef struct pages_entry_{
int objnum;
int number_of_pages;
int number_of_kids;
int kids[PAGES_TREE_KIDSMAX];
struct pages_entry_*next;
}pages_entry;

typedef struct divert_list_entry_{
int divnum;
pages_entry*first;
pages_entry*last;
}divert_list_entry;

static int comp_divert_list_entry(const void*pa,const void*pb,void*p)
{
(void)p;
if(((const divert_list_entry*)pa)->divnum> ((const divert_list_entry*)pb)->divnum)
return 1;
if(((const divert_list_entry*)pa)->divnum<((const divert_list_entry*)pb)->divnum)
return-1;
return 0;
}

/*:3*//*4:*/
#line 55 "./luatexdir/pdf/pdfpagetree.w"

static pages_entry*new_pages_entry(PDF pdf)
{
int i;
pages_entry*p= xtalloc(1,pages_entry);
p->number_of_pages= p->number_of_kids= 0;
for(i= 0;i<PAGES_TREE_KIDSMAX;i++)
p->kids[i]= 0;
p->next= NULL;
p->objnum= pdf_create_obj(pdf,obj_type_pages,0);
return p;
}

/*:4*//*5:*/
#line 68 "./luatexdir/pdf/pdfpagetree.w"

static divert_list_entry*new_divert_list_entry(void)
{
divert_list_entry*d;
d= xtalloc(1,divert_list_entry);
d->first= d->last= NULL;
return d;
}

/*:5*//*6:*/
#line 77 "./luatexdir/pdf/pdfpagetree.w"

static void ensure_list_tree(void)
{
if(divert_list_tree==NULL){
divert_list_tree= avl_create(comp_divert_list_entry,NULL,&avl_xallocator);
}
}

/*:6*//*7:*/
#line 85 "./luatexdir/pdf/pdfpagetree.w"

static divert_list_entry*get_divert_list(int divnum)
{
divert_list_entry*d,tmp;
void**aa;
tmp.divnum= divnum;
d= (divert_list_entry*)avl_find(divert_list_tree,&tmp);
if(d==NULL){
d= new_divert_list_entry();
d->divnum= divnum;

aa= avl_probe(divert_list_tree,d);
if(aa==NULL){
normal_error("pdf backend","page list lookup error");
}
}
return d;
}

/*:7*//*8:*/
#line 105 "./luatexdir/pdf/pdfpagetree.w"

int pdf_do_page_divert(PDF pdf,int objnum,int divnum)
{
divert_list_entry*d;
pages_entry*p;

ensure_list_tree();

d= get_divert_list(divnum);
if(d->first==NULL||d->last->number_of_kids==PAGES_TREE_KIDSMAX){

p= new_pages_entry(pdf);
if(d->first==NULL)
d->first= p;
else
d->last->next= p;
d->last= p;
}
p= d->last;
p->kids[p->number_of_kids++]= objnum;
p->number_of_pages++;
return p->objnum;
}

/*:8*//*9:*/
#line 129 "./luatexdir/pdf/pdfpagetree.w"

static void movelist(divert_list_entry*d,divert_list_entry*dto)
{
if(d!=NULL&&d->first!=NULL&&d->divnum!=dto->divnum){

if(dto->first==NULL)
dto->first= d->first;
else
dto->last->next= d->first;
dto->last= d->last;

d->first= d->last= NULL;
}
}

/*:9*//*10:*/
#line 145 "./luatexdir/pdf/pdfpagetree.w"

void pdf_do_page_undivert(int divnum,int curdivnum)
{
divert_list_entry*d,*dto,tmp;
struct avl_traverser t;

ensure_list_tree();

dto= get_divert_list(curdivnum);
if(divnum==0){

avl_t_init(&t,divert_list_tree);
for(d= avl_t_first(&t,divert_list_tree);d!=NULL;
d= avl_t_next(&t))
movelist(d,dto);
}else{
tmp.divnum= divnum;
d= (divert_list_entry*)avl_find(divert_list_tree,&tmp);
movelist(d,dto);
}
}

/*:10*//*11:*/
#line 168 "./luatexdir/pdf/pdfpagetree.w"


static void write_pages(PDF pdf,pages_entry*p,int parent)
{
int i;
int pages_attributes;
pdf_begin_obj(pdf,p->objnum,OBJSTM_ALWAYS);
pdf_begin_dict(pdf);
pdf_dict_add_name(pdf,"Type","Pages");
if(parent==0){

pages_attributes= pdf_pages_attr;
if(pages_attributes!=null){
pdf_print_toks(pdf,pages_attributes);
pdf_out(pdf,' ');
}
print_pdf_table_string(pdf,"pagesattributes");
pdf_out(pdf,' ');
}else
pdf_dict_add_ref(pdf,"Parent",parent);
pdf_dict_add_int(pdf,"Count",(int)p->number_of_pages);
pdf_add_name(pdf,"Kids");
pdf_begin_array(pdf);
for(i= 0;i<p->number_of_kids;i++)
pdf_add_ref(pdf,(int)p->kids[i]);
pdf_end_array(pdf);
pdf_end_dict(pdf);
pdf_end_obj(pdf);
}

/*:11*//*12:*/
#line 201 "./luatexdir/pdf/pdfpagetree.w"

static int output_pages_list(PDF pdf,pages_entry*pe)
{
pages_entry*p,*q,*r;
if(pe->next==NULL){

write_pages(pdf,pe,0);
return pe->objnum;
}
q= r= new_pages_entry(pdf);
for(p= pe;p!=NULL;p= p->next){
if(q->number_of_kids==PAGES_TREE_KIDSMAX){
q->next= new_pages_entry(pdf);
q= q->next;
}
q->kids[q->number_of_kids++]= p->objnum;
q->number_of_pages+= p->number_of_pages;
write_pages(pdf,p,q->objnum);
}
return output_pages_list(pdf,r);
}

/*:12*//*13:*/
#line 223 "./luatexdir/pdf/pdfpagetree.w"

int output_pages_tree(PDF pdf)
{
divert_list_entry*d;
pdf_do_page_undivert(0,0);
d= get_divert_list(0);
return output_pages_list(pdf,d->first);
}/*:13*/
