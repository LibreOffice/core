/*************************************************************************
 *
 *  $RCSfile: xpool.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#include "xtable.hxx"
#include "xattr.hxx"
#include "xpool.hxx"
#include "svdattr.hxx"
#include "svxids.hrc"

#define GLOBALOVERFLOW3

static USHORT nVersion1Map[40];
static USHORT nVersion2Map[66];
static USHORT nVersion3Map[124];
static USHORT nVersion4Map[141];

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/


XOutdevItemPool::XOutdevItemPool(USHORT nAttrStart, USHORT nAttrEnd, FASTBOOL bLoadRefCounts):
    SfxItemPool(String("XOutdevItemPool", gsl_getSystemTextEncoding()), nAttrStart, nAttrEnd, NULL, NULL, bLoadRefCounts)
{
    Ctor(NULL,nAttrStart,nAttrEnd);
}


XOutdevItemPool::XOutdevItemPool(SfxItemPool* pMaster, USHORT nAttrStart, USHORT nAttrEnd, FASTBOOL bLoadRefCounts):
    SfxItemPool(String("XOutdevItemPool", gsl_getSystemTextEncoding()), nAttrStart, nAttrEnd, NULL, NULL, bLoadRefCounts)
{
    Ctor(pMaster,nAttrStart,nAttrEnd);
}


void XOutdevItemPool::Ctor(SfxItemPool* pMaster, USHORT nAttrStart, USHORT nAttrEnd)
{
    // Mich als Secondary an den MasterPool (Joe)
    if (pMaster==NULL) {
        pMaster=this;
    } else {
        // Ich ganz hinten dran
        SfxItemPool* pParent=pMaster;
        while (pParent->GetSecondaryPool()!=NULL) {
            pParent=pParent->GetSecondaryPool();
        }
        pParent->SetSecondaryPool(this);
    }

    SfxItemSet* pSet;

    nStart=nAttrStart;
    nEnd  =nAttrEnd;
    ppPoolDefaults = new SfxPoolItem* [nEnd-nStart+1];

    USHORT i;

    // Am 27-06-1995 hat Elmar 13 neue Whiches fuer XATTR_FORMTXT bei Which 1021
    // eingebaut.
    // Alles was zu diesem Zeitpunkt >1021 war verschiebt sich also um
    // 13 Positionen nach hinten.
    // Alles was davor liegt (11 LineItems, 6 FillItems, 5 von 6 FormTextItems)
    // behaelt seine WhichId.

    for (i=1000; i<=1021; i++) {
        nVersion1Map[i-1000]=i;
    }
    for (i=1022; i<=1039; i++) {
        nVersion1Map[i-1000]=i+13;
    }
    SetVersionMap(1,1000,1039,nVersion1Map);

    // Am 09-11-1995 haben dann wiederum Kai Ahrens 14 und Joe 44 (also beide
    // insgesamt 58) neue Whiches an verschiedenen Stellen spendiert (siehe
    // auch die Beschreibung in svx\inc\SvdAttr.HXX).

    for (i=1000; i<=1009; i++) {  // XOut
        nVersion2Map[i-1000]=i;
    }
    for (i=1010; i<=1015; i++) {  // XOut
        nVersion2Map[i-1000]=i+7;
    }
    for (i=1016; i<=1035; i++) {  // XOut
        nVersion2Map[i-1000]=i+14;
    }
    for (i=1036; i<=1039; i++) {  // SvDraw
        nVersion2Map[i-1000]=i+14;
    }
    for (i=1040; i<=1050; i++) {  // SvDraw
        nVersion2Map[i-1000]=i+22;
    }
    for (i=1051; i<=1056; i++) {  // SvDraw
        nVersion2Map[i-1000]=i+27;
    }
    for (i=1057; i<=1065; i++) {  // SvDraw
        nVersion2Map[i-1000]=i+52;
    }
    SetVersionMap(2,1000,1065,nVersion2Map);


    // Am 29-02-1996 hat KA 17 neue Whiches
    // fuer das XOut spendiert

    for (i=1000; i<=1029; i++) {
        nVersion3Map[i-1000]=i;
    }
    for (i=1030; i<=1123; i++) {
        nVersion3Map[i-1000]=i+17;
    }
    SetVersionMap(3,1000,1123,nVersion3Map);


    // Am 10-08-1996 hat Joe 45 neue Items in SvDraw eingebaut
    // fuer Bemassung, Verbinder und Laufschrift

    for (i=1000; i<=1126; i++) {
        nVersion4Map[i-1000]=i;
    }
    for (i=1127; i<=1140; i++) {
        nVersion4Map[i-1000]=i+45;
    }
    SetVersionMap(4,1000,1140,nVersion4Map);


    XubString    aNullStr;
    Bitmap      aNullBmp;
    XPolygon    aNullPol;
    Color       aNullLineCol(RGB_Color(COL_BLACK));

#ifndef VCL
    Color       aNullFillCol(Color(0<<8,184<<8,255<<8));  // "Blau 7"
#else
    Color       aNullFillCol(Color(0,184,255));  // "Blau 7"
#endif

    Color       aNullShadowCol(RGB_Color(COL_LIGHTGRAY));
    XDash       aNullDash;
    XGradient   aNullGrad(aNullLineCol, RGB_Color(COL_WHITE));
    XHatch      aNullHatch(aNullLineCol);

    // LineItems
    ppPoolDefaults[XATTR_LINESTYLE          -XATTR_START] = new XLineStyleItem;
    ppPoolDefaults[XATTR_LINEDASH           -XATTR_START] = new XLineDashItem(aNullStr,aNullDash);
    ppPoolDefaults[XATTR_LINEWIDTH          -XATTR_START] = new XLineWidthItem;
    ppPoolDefaults[XATTR_LINECOLOR          -XATTR_START] = new XLineColorItem(aNullStr,aNullLineCol);
    ppPoolDefaults[XATTR_LINESTART          -XATTR_START] = new XLineStartItem(aNullStr,aNullPol);
    ppPoolDefaults[XATTR_LINEEND            -XATTR_START] = new XLineEndItem  (aNullStr,aNullPol);
    ppPoolDefaults[XATTR_LINESTARTWIDTH     -XATTR_START] = new XLineStartWidthItem;
    ppPoolDefaults[XATTR_LINEENDWIDTH       -XATTR_START] = new XLineEndWidthItem;
    ppPoolDefaults[XATTR_LINESTARTCENTER    -XATTR_START] = new XLineStartCenterItem;
    ppPoolDefaults[XATTR_LINEENDCENTER      -XATTR_START] = new XLineEndCenterItem;
    ppPoolDefaults[XATTR_LINETRANSPARENCE   -XATTR_START] = new XLineTransparenceItem;
    ppPoolDefaults[XATTR_LINEJOINT          -XATTR_START] = new XLineJointItem;

    // Reserven fuer LineItems
    ppPoolDefaults[XATTR_LINERESERVED2      -XATTR_START] = new SfxVoidItem(XATTR_LINERESERVED2);
    ppPoolDefaults[XATTR_LINERESERVED3      -XATTR_START] = new SfxVoidItem(XATTR_LINERESERVED3);
    ppPoolDefaults[XATTR_LINERESERVED4      -XATTR_START] = new SfxVoidItem(XATTR_LINERESERVED4);
    ppPoolDefaults[XATTR_LINERESERVED5      -XATTR_START] = new SfxVoidItem(XATTR_LINERESERVED5);
    ppPoolDefaults[XATTR_LINERESERVED_LAST  -XATTR_START] = new SfxVoidItem(XATTR_LINERESERVED_LAST);

    // FillItems
    ppPoolDefaults[XATTR_FILLSTYLE              -XATTR_START] = new XFillStyleItem;
    ppPoolDefaults[XATTR_FILLCOLOR              -XATTR_START] = new XFillColorItem   (aNullStr,aNullFillCol);
    ppPoolDefaults[XATTR_FILLGRADIENT           -XATTR_START] = new XFillGradientItem(aNullStr,aNullGrad);
    ppPoolDefaults[XATTR_FILLHATCH              -XATTR_START] = new XFillHatchItem   (aNullStr,aNullHatch);
    ppPoolDefaults[XATTR_FILLBITMAP             -XATTR_START] = new XFillBitmapItem  (aNullStr,aNullBmp);
    ppPoolDefaults[XATTR_FILLTRANSPARENCE       -XATTR_START] = new XFillTransparenceItem;
    ppPoolDefaults[XATTR_GRADIENTSTEPCOUNT      -XATTR_START] = new XGradientStepCountItem;
    ppPoolDefaults[XATTR_FILLBMP_TILE           -XATTR_START] = new XFillBmpTileItem;
    ppPoolDefaults[XATTR_FILLBMP_POS            -XATTR_START] = new XFillBmpPosItem;
    ppPoolDefaults[XATTR_FILLBMP_SIZEX          -XATTR_START] = new XFillBmpSizeXItem;
    ppPoolDefaults[XATTR_FILLBMP_SIZEY          -XATTR_START] = new XFillBmpSizeYItem;
    ppPoolDefaults[XATTR_FILLBMP_SIZELOG        -XATTR_START] = new XFillBmpSizeLogItem;
    ppPoolDefaults[XATTR_FILLBMP_TILEOFFSETX    -XATTR_START] = new XFillBmpTileOffsetXItem;
    ppPoolDefaults[XATTR_FILLBMP_TILEOFFSETY    -XATTR_START] = new XFillBmpTileOffsetYItem;
    ppPoolDefaults[XATTR_FILLBMP_STRETCH        -XATTR_START] = new XFillBmpStretchItem;
    ppPoolDefaults[XATTR_FILLBMP_POSOFFSETX     -XATTR_START] = new XFillBmpPosOffsetXItem;
    ppPoolDefaults[XATTR_FILLBMP_POSOFFSETY     -XATTR_START] = new XFillBmpPosOffsetYItem;
    ppPoolDefaults[XATTR_FILLFLOATTRANSPARENCE  -XATTR_START] = new XFillFloatTransparenceItem( aNullStr, aNullGrad, FALSE );

    // Reserven fuer FillItems
    ppPoolDefaults[XATTR_FILLRESERVED2          -XATTR_START] = new SfxVoidItem(XATTR_FILLRESERVED2);
    ppPoolDefaults[XATTR_FILLRESERVED3          -XATTR_START] = new SfxVoidItem(XATTR_FILLRESERVED3);
    ppPoolDefaults[XATTR_FILLRESERVED4          -XATTR_START] = new SfxVoidItem(XATTR_FILLRESERVED4);
    ppPoolDefaults[XATTR_FILLRESERVED5          -XATTR_START] = new SfxVoidItem(XATTR_FILLRESERVED5);
    ppPoolDefaults[XATTR_FILLRESERVED6          -XATTR_START] = new SfxVoidItem(XATTR_FILLRESERVED6);
    ppPoolDefaults[XATTR_FILLRESERVED7          -XATTR_START] = new SfxVoidItem(XATTR_FILLRESERVED7);
    ppPoolDefaults[XATTR_FILLRESERVED8          -XATTR_START] = new SfxVoidItem(XATTR_FILLRESERVED8);
    ppPoolDefaults[XATTR_FILLBACKGROUND         -XATTR_START] = new XFillBackgroundItem;
    ppPoolDefaults[XATTR_FILLRESERVED10         -XATTR_START] = new SfxVoidItem(XATTR_FILLRESERVED10);
    ppPoolDefaults[XATTR_FILLRESERVED11         -XATTR_START] = new SfxVoidItem(XATTR_FILLRESERVED11);
    ppPoolDefaults[XATTR_FILLRESERVED_LAST      -XATTR_START] = new SfxVoidItem(XATTR_FILLRESERVED_LAST);

    // FormTextItems
    ppPoolDefaults[XATTR_FORMTXTSTYLE       -XATTR_START] = new XFormTextStyleItem;
    ppPoolDefaults[XATTR_FORMTXTADJUST      -XATTR_START] = new XFormTextAdjustItem;
    ppPoolDefaults[XATTR_FORMTXTDISTANCE    -XATTR_START] = new XFormTextDistanceItem;
    ppPoolDefaults[XATTR_FORMTXTSTART       -XATTR_START] = new XFormTextStartItem;
    ppPoolDefaults[XATTR_FORMTXTMIRROR      -XATTR_START] = new XFormTextMirrorItem;
    ppPoolDefaults[XATTR_FORMTXTOUTLINE     -XATTR_START] = new XFormTextOutlineItem;
    ppPoolDefaults[XATTR_FORMTXTSHADOW      -XATTR_START] = new XFormTextShadowItem;
    ppPoolDefaults[XATTR_FORMTXTSHDWCOLOR   -XATTR_START] = new XFormTextShadowColorItem(aNullStr,aNullShadowCol);
    ppPoolDefaults[XATTR_FORMTXTSHDWXVAL    -XATTR_START] = new XFormTextShadowXValItem;
    ppPoolDefaults[XATTR_FORMTXTSHDWYVAL    -XATTR_START] = new XFormTextShadowYValItem;
    ppPoolDefaults[XATTR_FORMTXTSTDFORM     -XATTR_START] = new XFormTextStdFormItem;
    ppPoolDefaults[XATTR_FORMTXTHIDEFORM    -XATTR_START] = new XFormTextHideFormItem;

    // Reserven fuer FormTextItems
    ppPoolDefaults[XATTR_FORMTXTSHDWTRANSP  -XATTR_START] = new XFormTextShadowTranspItem;
    ppPoolDefaults[XATTR_FTRESERVED2        -XATTR_START] = new SfxVoidItem(XATTR_FTRESERVED2);
    ppPoolDefaults[XATTR_FTRESERVED3        -XATTR_START] = new SfxVoidItem(XATTR_FTRESERVED3);
    ppPoolDefaults[XATTR_FTRESERVED4        -XATTR_START] = new SfxVoidItem(XATTR_FTRESERVED4);
    ppPoolDefaults[XATTR_FTRESERVED5        -XATTR_START] = new SfxVoidItem(XATTR_FTRESERVED5);
    ppPoolDefaults[XATTR_FTRESERVED_LAST    -XATTR_START] = new SfxVoidItem(XATTR_FTRESERVED_LAST);

    // SetItems erzeugen
    pSet=new SfxItemSet(*pMaster, XATTR_LINE_FIRST, XATTR_LINE_LAST);
    ppPoolDefaults[XATTRSET_LINE - XATTR_START] = new XLineAttrSetItem(pSet);
    pSet=new SfxItemSet(*pMaster, XATTR_FILL_FIRST, XATTR_FILL_LAST);
    ppPoolDefaults[XATTRSET_FILL - XATTR_START] = new XFillAttrSetItem(pSet);
    pSet=new SfxItemSet(*pMaster, XATTR_TEXT_FIRST, XATTR_TEXT_LAST);
    ppPoolDefaults[XATTRSET_TEXT - XATTR_START] = new XTextAttrSetItem(pSet);

    // ItemInfos
    pItemInfos=new SfxItemInfo[nEnd-nStart+1];
    for (i=nStart; i<=nEnd; i++) {
        pItemInfos[i-nStart]._nSID=0;
        pItemInfos[i-nStart]._nFlags=SFX_ITEM_POOLABLE;
    }

    pItemInfos[XATTR_LINESTYLE        -XATTR_START]._nSID = SID_ATTR_LINE_STYLE;
    pItemInfos[XATTR_LINEDASH         -XATTR_START]._nSID = SID_ATTR_LINE_DASH;
    pItemInfos[XATTR_LINEWIDTH        -XATTR_START]._nSID = SID_ATTR_LINE_WIDTH;
    pItemInfos[XATTR_LINECOLOR        -XATTR_START]._nSID = SID_ATTR_LINE_COLOR;
    pItemInfos[XATTR_LINESTART        -XATTR_START]._nSID = SID_ATTR_LINE_START;
    pItemInfos[XATTR_LINEEND          -XATTR_START]._nSID = SID_ATTR_LINE_END;
    pItemInfos[XATTR_LINESTARTWIDTH   -XATTR_START]._nSID = SID_ATTR_LINE_STARTWIDTH;
    pItemInfos[XATTR_LINEENDWIDTH     -XATTR_START]._nSID = SID_ATTR_LINE_ENDWIDTH;
    pItemInfos[XATTR_LINESTARTCENTER  -XATTR_START]._nSID = SID_ATTR_LINE_STARTCENTER;
    pItemInfos[XATTR_LINEENDCENTER    -XATTR_START]._nSID = SID_ATTR_LINE_ENDCENTER;

    pItemInfos[XATTR_FILLSTYLE        -XATTR_START]._nSID = SID_ATTR_FILL_STYLE;
    pItemInfos[XATTR_FILLCOLOR        -XATTR_START]._nSID = SID_ATTR_FILL_COLOR;
    pItemInfos[XATTR_FILLGRADIENT     -XATTR_START]._nSID = SID_ATTR_FILL_GRADIENT;
    pItemInfos[XATTR_FILLHATCH        -XATTR_START]._nSID = SID_ATTR_FILL_HATCH;
    pItemInfos[XATTR_FILLBITMAP       -XATTR_START]._nSID = SID_ATTR_FILL_BITMAP;

    pItemInfos[XATTR_FORMTXTSTYLE     -XATTR_START]._nSID = SID_FORMTEXT_STYLE;
    pItemInfos[XATTR_FORMTXTADJUST    -XATTR_START]._nSID = SID_FORMTEXT_ADJUST;
    pItemInfos[XATTR_FORMTXTDISTANCE  -XATTR_START]._nSID = SID_FORMTEXT_DISTANCE;
    pItemInfos[XATTR_FORMTXTSTART     -XATTR_START]._nSID = SID_FORMTEXT_START;
    pItemInfos[XATTR_FORMTXTMIRROR    -XATTR_START]._nSID = SID_FORMTEXT_MIRROR;
    pItemInfos[XATTR_FORMTXTOUTLINE   -XATTR_START]._nSID = SID_FORMTEXT_OUTLINE;
    pItemInfos[XATTR_FORMTXTSHADOW    -XATTR_START]._nSID = SID_FORMTEXT_SHADOW;
    pItemInfos[XATTR_FORMTXTSHDWCOLOR -XATTR_START]._nSID = SID_FORMTEXT_SHDWCOLOR;
    pItemInfos[XATTR_FORMTXTSHDWXVAL  -XATTR_START]._nSID = SID_FORMTEXT_SHDWXVAL;
    pItemInfos[XATTR_FORMTXTSHDWYVAL  -XATTR_START]._nSID = SID_FORMTEXT_SHDWYVAL;
    pItemInfos[XATTR_FORMTXTSTDFORM   -XATTR_START]._nSID = SID_FORMTEXT_STDFORM;
    pItemInfos[XATTR_FORMTXTHIDEFORM  -XATTR_START]._nSID = SID_FORMTEXT_HIDEFORM;

#if SUPD<355
    pSlotIds = new USHORT[nEnd-nStart+1];

    pSlotIds[XATTR_LINESTYLE        -XATTR_START] = SID_ATTR_LINE_STYLE;
    pSlotIds[XATTR_LINEDASH         -XATTR_START] = SID_ATTR_LINE_DASH;
    pSlotIds[XATTR_LINEWIDTH        -XATTR_START] = SID_ATTR_LINE_WIDTH;
    pSlotIds[XATTR_LINECOLOR        -XATTR_START] = SID_ATTR_LINE_COLOR;
    pSlotIds[XATTR_LINESTART        -XATTR_START] = SID_ATTR_LINE_START;
    pSlotIds[XATTR_LINEEND          -XATTR_START] = SID_ATTR_LINE_END;
    pSlotIds[XATTR_LINESTARTWIDTH   -XATTR_START] = SID_ATTR_LINE_STARTWIDTH;
    pSlotIds[XATTR_LINEENDWIDTH     -XATTR_START] = SID_ATTR_LINE_ENDWIDTH;
    pSlotIds[XATTR_LINESTARTCENTER  -XATTR_START] = SID_ATTR_LINE_STARTCENTER;
    pSlotIds[XATTR_LINEENDCENTER    -XATTR_START] = SID_ATTR_LINE_ENDCENTER;
    pSlotIds[XATTR_LINETRANSPARENCE -XATTR_START] = 0;
    pSlotIds[XATTR_LINEJOINT        -XATTR_START] = 0;

    // Line-Reserven
    pSlotIds[XATTR_LINERESERVED2        -XATTR_START] = 0;
    pSlotIds[XATTR_LINERESERVED3        -XATTR_START] = 0;
    pSlotIds[XATTR_LINERESERVED4        -XATTR_START] = 0;
    pSlotIds[XATTR_LINERESERVED5        -XATTR_START] = 0;
    pSlotIds[XATTR_LINERESERVED_LAST    -XATTR_START] = 0;

    pSlotIds[XATTR_FILLSTYLE        -XATTR_START] = SID_ATTR_FILL_STYLE;
    pSlotIds[XATTR_FILLCOLOR        -XATTR_START] = SID_ATTR_FILL_COLOR;
    pSlotIds[XATTR_FILLGRADIENT     -XATTR_START] = SID_ATTR_FILL_GRADIENT;
    pSlotIds[XATTR_FILLHATCH        -XATTR_START] = SID_ATTR_FILL_HATCH;
    pSlotIds[XATTR_FILLBITMAP       -XATTR_START] = SID_ATTR_FILL_BITMAP;
    pSlotIds[XATTR_FILLTRANSPARENCE -XATTR_START] = 0;
    pSlotIds[XATTR_GRADIENTSTEPCOUNT-XATTR_START] = 0;

    pSlotIds[XATTR_FILLBMP_TILE             -XATTR_START] = 0;
    pSlotIds[XATTR_FILLBMP_POS              -XATTR_START] = 0;
    pSlotIds[XATTR_FILLBMP_SIZEX            -XATTR_START] = 0;
    pSlotIds[XATTR_FILLBMP_SIZEY            -XATTR_START] = 0;
    pSlotIds[XATTR_FILLBMP_SIZELOG          -XATTR_START] = 0;
    pSlotIds[XATTR_FILLBMP_TILEOFFSETX      -XATTR_START] = 0;
    pSlotIds[XATTR_FILLBMP_TILEOFFSETY      -XATTR_START] = 0;
    pSlotIds[XATTR_FILLBMP_STRETCH          -XATTR_START] = 0;
    pSlotIds[XATTR_FILLBMP_POSOFFSETX       -XATTR_START] = 0;
    pSlotIds[XATTR_FILLBMP_POSOFFSETY       -XATTR_START] = 0;
    pSlotIds[XATTR_FILLFLOATTRANSPARENCE    -XATTR_START] = 0;

    // Fill-Reserven
    pSlotIds[XATTR_FILLRESERVED2        -XATTR_START] = 0;
    pSlotIds[XATTR_FILLRESERVED3        -XATTR_START] = 0;
    pSlotIds[XATTR_FILLRESERVED4        -XATTR_START] = 0;
    pSlotIds[XATTR_FILLRESERVED5        -XATTR_START] = 0;
    pSlotIds[XATTR_FILLRESERVED6        -XATTR_START] = 0;
    pSlotIds[XATTR_FILLRESERVED7        -XATTR_START] = 0;
    pSlotIds[XATTR_FILLRESERVED8        -XATTR_START] = 0;
    pSlotIds[XATTR_FILLBACKGROUND       -XATTR_START] = 0;
    pSlotIds[XATTR_FILLRESERVED10       -XATTR_START] = 0;
    pSlotIds[XATTR_FILLRESERVED11       -XATTR_START] = 0;
    pSlotIds[XATTR_FILLRESERVED_LAST    -XATTR_START] = 0;

    pSlotIds[XATTR_FORMTXTSTYLE     -XATTR_START] = SID_FORMTEXT_STYLE;
    pSlotIds[XATTR_FORMTXTADJUST    -XATTR_START] = SID_FORMTEXT_ADJUST;
    pSlotIds[XATTR_FORMTXTDISTANCE  -XATTR_START] = SID_FORMTEXT_DISTANCE;
    pSlotIds[XATTR_FORMTXTSTART     -XATTR_START] = SID_FORMTEXT_START;
    pSlotIds[XATTR_FORMTXTMIRROR    -XATTR_START] = SID_FORMTEXT_MIRROR;
    pSlotIds[XATTR_FORMTXTOUTLINE   -XATTR_START] = SID_FORMTEXT_OUTLINE;
    pSlotIds[XATTR_FORMTXTSHADOW    -XATTR_START] = SID_FORMTEXT_SHADOW;
    pSlotIds[XATTR_FORMTXTSHDWCOLOR -XATTR_START] = SID_FORMTEXT_SHDWCOLOR;
    pSlotIds[XATTR_FORMTXTSHDWXVAL  -XATTR_START] = SID_FORMTEXT_SHDWXVAL;
    pSlotIds[XATTR_FORMTXTSHDWYVAL  -XATTR_START] = SID_FORMTEXT_SHDWYVAL;
    pSlotIds[XATTR_FORMTXTSTDFORM   -XATTR_START] = SID_FORMTEXT_STDFORM;
    pSlotIds[XATTR_FORMTXTHIDEFORM  -XATTR_START] = SID_FORMTEXT_HIDEFORM;

    // FormText-Reserven
    pSlotIds[XATTR_FORMTXTSHDWTRANSP-XATTR_START] = 0;
    pSlotIds[XATTR_FTRESERVED2      -XATTR_START] = 0;
    pSlotIds[XATTR_FTRESERVED3      -XATTR_START] = 0;
    pSlotIds[XATTR_FTRESERVED4      -XATTR_START] = 0;
    pSlotIds[XATTR_FTRESERVED5      -XATTR_START] = 0;
    pSlotIds[XATTR_FTRESERVED_LAST  -XATTR_START] = 0;

    pSlotIds[XATTRSET_LINE - XATTR_START] = 0; // 20136; alter Hack von MI
    pSlotIds[XATTRSET_FILL - XATTR_START] = 0; // 20135; alter Hack von MI
    pSlotIds[XATTRSET_TEXT - XATTR_START] = 0;
#endif

    if (nAttrStart==XATTR_START && nAttrEnd==XATTR_END)
    {
        SetDefaults(ppPoolDefaults);
        SetItemInfos(pItemInfos);
#if SUPD<355
        SetMap(pSlotIds);
#endif
    }
}


/*************************************************************************
|*
|* copy ctor, sorgt dafuer, dass die static defaults gecloned werden
|*            (Parameter 2 = TRUE)
|*
\************************************************************************/

XOutdevItemPool::XOutdevItemPool(const XOutdevItemPool& rPool) :
    SfxItemPool(rPool, TRUE)
{
    // damit geclonete Pools nicht im dtor auf die Nase fallen
    // endgueltige Loesung --> MI
    ppPoolDefaults = NULL;
}

/*************************************************************************
|*
|* Clone()
|*
\************************************************************************/

SfxItemPool* XOutdevItemPool::Clone() const
{
    return new XOutdevItemPool(*this);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

XOutdevItemPool::~XOutdevItemPool()
{
#if SUPD<355
    if (pSlotIds!=NULL) delete pSlotIds;
#endif

    Delete();
    if ( ppPoolDefaults )
    {
        SfxPoolItem** ppDefaultItem = ppPoolDefaults;
        for ( USHORT i = nEnd - nStart + 1; i; --i, ++ppDefaultItem )
        {
            if ( *ppDefaultItem ) //Teile schon von abgel. Klasse abgeraeumt!
            {
#ifndef PRODUCT
                SetRef( **ppDefaultItem, 0 );
#endif
                delete *ppDefaultItem;
            }
        }
        delete ppPoolDefaults;
    }
    if (pItemInfos!=NULL) delete[] pItemInfos;

}


