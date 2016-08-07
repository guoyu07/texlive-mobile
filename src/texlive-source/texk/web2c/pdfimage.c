/*1:*/
#line 20 "./luatexdir/pdf/pdfimage.w"


#include "ptexlib.h"

/*:1*//*2:*/
#line 24 "./luatexdir/pdf/pdfimage.w"

void place_img(PDF pdf,image_dict*idict,scaled_whd dim,int transform)
{
float a[6];
float xoff,yoff,tmp;
pdfstructure*p= pdf->pstruct;
scaledpos pos= pdf->posstruct->pos;
int r;
int k;
scaledpos tmppos;
pdffloat cm[6];
int groupref;
a[0]= a[3]= 1.0e6;
a[1]= a[2]= 0;
if(img_type(idict)==IMG_TYPE_PDF||img_type(idict)==IMG_TYPE_PDFMEMSTREAM
||img_type(idict)==IMG_TYPE_PDFSTREAM){
a[0]/= (float)img_xsize(idict);
a[3]/= (float)img_ysize(idict);
xoff= (float)img_xorig(idict)/(float)img_xsize(idict);
yoff= (float)img_yorig(idict)/(float)img_ysize(idict);
r= 6;
}else{

if(img_type(idict)==IMG_TYPE_PNG){
groupref= img_group_ref(idict);
if((groupref> 0)&&(pdf->img_page_group_val==0))
pdf->img_page_group_val= groupref;
}

a[0]/= (float)one_hundred_bp;
a[3]= a[0];
xoff= yoff= 0;
r= 4;
}
if((transform&7)> 3){
a[0]*= -1;
xoff*= -1;
}
switch((transform+img_rotation(idict))&3){
case 0:
break;
case 1:
a[1]= a[0];
a[2]= -a[3];
a[3]= a[0]= 0;
tmp= yoff;
yoff= xoff;
xoff= -tmp;
break;
case 2:
a[0]*= -1;
a[3]*= -1;
xoff*= -1;
yoff*= -1;
break;
case 3:
a[1]= -a[0];
a[2]= a[3];
a[3]= a[0]= 0;
tmp= yoff;
yoff= -xoff;
xoff= tmp;
break;
default:;
}
xoff*= (float)dim.wd;
yoff*= (float)(dim.ht+dim.dp);
a[0]*= (float)dim.wd;
a[1]*= (float)(dim.ht+dim.dp);
a[2]*= (float)dim.wd;
a[3]*= (float)(dim.ht+dim.dp);
a[4]= (float)pos.h-xoff;
a[5]= (float)pos.v-yoff;
k= transform+img_rotation(idict);
if((transform&7)> 3)
k++;
switch(k&3){
case 0:
break;
case 1:
a[4]+= (float)dim.wd;
break;
case 2:
a[4]+= (float)dim.wd;
a[5]+= (float)(dim.ht+dim.dp);
break;
case 3:
a[5]+= (float)(dim.ht+dim.dp);
break;
default:;
}

setpdffloat(cm[0],i64round(a[0]),r);
setpdffloat(cm[1],i64round(a[1]),r);
setpdffloat(cm[2],i64round(a[2]),r);
setpdffloat(cm[3],i64round(a[3]),r);
tmppos.h= round(a[4]);
tmppos.v= round(a[5]);
pdf_goto_pagemode(pdf);
(void)calc_pdfpos(p,tmppos);
cm[4]= p->cm[4];
cm[5]= p->cm[5];
if(pdf->img_page_group_val==0)
pdf->img_page_group_val= img_group_ref(idict);
pdf_puts(pdf,"q\n");
pdf_print_cm(pdf,cm);
pdf_puts(pdf,"/Im");
pdf_print_int(pdf,img_index(idict));
pdf_print_resname_prefix(pdf);
pdf_puts(pdf," Do\nQ\n");
addto_page_resources(pdf,obj_type_ximage,img_objnum(idict));
if(img_state(idict)<DICT_OUTIMG)
img_state(idict)= DICT_OUTIMG;
}

/*:2*//*3:*/
#line 141 "./luatexdir/pdf/pdfimage.w"

void pdf_place_image(PDF pdf,halfword p)
{
scaled_whd dim;
image_dict*idict= idict_array[rule_index(p)];
dim.wd= width(p);
dim.ht= height(p);
dim.dp= depth(p);
place_img(pdf,idict,dim,rule_transform(p));
}/*:3*/
