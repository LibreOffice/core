/*************************************************************************
 *
 *  $RCSfile: xpool.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-11-03 11:10:08 $
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

//BFS01#pragma hdrstop

#include "xtable.hxx"
#include "xattr.hxx"
#include "xpool.hxx"
#include "svdattr.hxx"
#include "svxids.hrc"

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

//BFS01#define GLOBALOVERFLOW3

//BFS01static USHORT nVersion1Map[40];
//BFS01static USHORT nVersion2Map[66];
//BFS01static USHORT nVersion3Map[124];
//BFS01static USHORT nVersion4Map[141];

//BFS01static XOutdevItemPool* pPool=0;

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/


//BFS01XOutdevItemPool::XOutdevItemPool(USHORT nAttrStart, USHORT nAttrEnd, FASTBOOL bLoadRefCounts)
//BFS04XOutdevItemPool::XOutdevItemPool(FASTBOOL bLoadRefCounts)
//BFS04:    SfxItemPool(String("XOutdevItemPool", gsl_getSystemTextEncoding()),
//BFS04 SDRATTR_START, SDRATTR_END,
//BFS04 NULL, NULL,
//BFS04 bLoadRefCounts)
//BFS04{
//BFS04//BFS01  Ctor(NULL,nAttrStart,nAttrEnd);
//BFS04 Ctor(NULL);
//BFS04}


//BFS01XOutdevItemPool::XOutdevItemPool(SfxItemPool* pMaster, USHORT nAttrStart, USHORT nAttrEnd, FASTBOOL bLoadRefCounts)
//BFS04XOutdevItemPool::XOutdevItemPool(SfxItemPool* pMaster, FASTBOOL bLoadRefCounts)
XOutdevItemPool::XOutdevItemPool(
    SfxItemPool* pMaster,
    sal_uInt16 nAttrStart,
    sal_uInt16 nAttrEnd,
    sal_Bool bLoadRefCounts)
:   SfxItemPool(String("XOutdevItemPool", gsl_getSystemTextEncoding()), nAttrStart, nAttrEnd, 0L, 0L, bLoadRefCounts)
{
//BFS01 Ctor(pMaster,nAttrStart,nAttrEnd);
//BFS04 Ctor(pMaster);
    // prepare some defaults
    const XubString aNullStr;
    const Bitmap aNullBmp;
    const XPolygon aNullPol;
    const Color aNullLineCol(RGB_Color(COL_BLACK));
    const Color aNullFillCol(Color(0,184,255));  // "Blau 7"
    const Color aNullShadowCol(RGB_Color(COL_LIGHTGRAY));
    const XDash aNullDash;
    const XGradient aNullGrad(aNullLineCol, RGB_Color(COL_WHITE));
    const XHatch aNullHatch(aNullLineCol);

    // get master pointer, evtl. add myself to the end of the pools
    if(!pMaster)
    {
        pMaster = this;
    }
    else
    {
        SfxItemPool* pParent = pMaster;

        while(pParent->GetSecondaryPool())
        {
            pParent = pParent->GetSecondaryPool();
        }

        pParent->SetSecondaryPool(this);
    }

    // prepare PoolDefaults
    mppLocalPoolDefaults = new SfxPoolItem*[GetLastWhich() - GetFirstWhich() + 1];

    mppLocalPoolDefaults[XATTR_LINESTYLE          -XATTR_START] = new XLineStyleItem;
    mppLocalPoolDefaults[XATTR_LINEDASH           -XATTR_START] = new XLineDashItem(this,aNullDash);
    mppLocalPoolDefaults[XATTR_LINEWIDTH          -XATTR_START] = new XLineWidthItem;
    mppLocalPoolDefaults[XATTR_LINECOLOR          -XATTR_START] = new XLineColorItem(aNullStr,aNullLineCol);
    mppLocalPoolDefaults[XATTR_LINESTART          -XATTR_START] = new XLineStartItem(this,aNullPol);
    mppLocalPoolDefaults[XATTR_LINEEND            -XATTR_START] = new XLineEndItem  (this,aNullPol);
    mppLocalPoolDefaults[XATTR_LINESTARTWIDTH     -XATTR_START] = new XLineStartWidthItem;
    mppLocalPoolDefaults[XATTR_LINEENDWIDTH       -XATTR_START] = new XLineEndWidthItem;
    mppLocalPoolDefaults[XATTR_LINESTARTCENTER    -XATTR_START] = new XLineStartCenterItem;
    mppLocalPoolDefaults[XATTR_LINEENDCENTER      -XATTR_START] = new XLineEndCenterItem;
    mppLocalPoolDefaults[XATTR_LINETRANSPARENCE   -XATTR_START] = new XLineTransparenceItem;
    mppLocalPoolDefaults[XATTR_LINEJOINT            -XATTR_START] = new XLineJointItem;
    mppLocalPoolDefaults[XATTR_FILLSTYLE                -XATTR_START] = new XFillStyleItem;
    mppLocalPoolDefaults[XATTR_FILLCOLOR                -XATTR_START] = new XFillColorItem   (aNullStr,aNullFillCol);
    mppLocalPoolDefaults[XATTR_FILLGRADIENT         -XATTR_START] = new XFillGradientItem(this,aNullGrad);
    mppLocalPoolDefaults[XATTR_FILLHATCH                -XATTR_START] = new XFillHatchItem   (this,aNullHatch);
    mppLocalPoolDefaults[XATTR_FILLBITMAP               -XATTR_START] = new XFillBitmapItem  (this,aNullBmp);
    mppLocalPoolDefaults[XATTR_FILLTRANSPARENCE     -XATTR_START] = new XFillTransparenceItem;
    mppLocalPoolDefaults[XATTR_GRADIENTSTEPCOUNT        -XATTR_START] = new XGradientStepCountItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_TILE         -XATTR_START] = new XFillBmpTileItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_POS          -XATTR_START] = new XFillBmpPosItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_SIZEX            -XATTR_START] = new XFillBmpSizeXItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_SIZEY            -XATTR_START] = new XFillBmpSizeYItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_SIZELOG      -XATTR_START] = new XFillBmpSizeLogItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_TILEOFFSETX  -XATTR_START] = new XFillBmpTileOffsetXItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_TILEOFFSETY  -XATTR_START] = new XFillBmpTileOffsetYItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_STRETCH      -XATTR_START] = new XFillBmpStretchItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_POSOFFSETX       -XATTR_START] = new XFillBmpPosOffsetXItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_POSOFFSETY       -XATTR_START] = new XFillBmpPosOffsetYItem;
    mppLocalPoolDefaults[XATTR_FILLFLOATTRANSPARENCE    -XATTR_START] = new XFillFloatTransparenceItem( this, aNullGrad, FALSE );
    mppLocalPoolDefaults[XATTR_SECONDARYFILLCOLOR       -XATTR_START] = new XSecondaryFillColorItem(aNullStr, aNullFillCol);
    mppLocalPoolDefaults[XATTR_FILLBACKGROUND           -XATTR_START] = new XFillBackgroundItem;
    mppLocalPoolDefaults[XATTR_FORMTXTSTYLE       -XATTR_START] = new XFormTextStyleItem;
    mppLocalPoolDefaults[XATTR_FORMTXTADJUST      -XATTR_START] = new XFormTextAdjustItem;
    mppLocalPoolDefaults[XATTR_FORMTXTDISTANCE    -XATTR_START] = new XFormTextDistanceItem;
    mppLocalPoolDefaults[XATTR_FORMTXTSTART       -XATTR_START] = new XFormTextStartItem;
    mppLocalPoolDefaults[XATTR_FORMTXTMIRROR      -XATTR_START] = new XFormTextMirrorItem;
    mppLocalPoolDefaults[XATTR_FORMTXTOUTLINE     -XATTR_START] = new XFormTextOutlineItem;
    mppLocalPoolDefaults[XATTR_FORMTXTSHADOW      -XATTR_START] = new XFormTextShadowItem;
    mppLocalPoolDefaults[XATTR_FORMTXTSHDWCOLOR   -XATTR_START] = new XFormTextShadowColorItem(aNullStr,aNullShadowCol);
    mppLocalPoolDefaults[XATTR_FORMTXTSHDWXVAL    -XATTR_START] = new XFormTextShadowXValItem;
    mppLocalPoolDefaults[XATTR_FORMTXTSHDWYVAL    -XATTR_START] = new XFormTextShadowYValItem;
    mppLocalPoolDefaults[XATTR_FORMTXTSTDFORM     -XATTR_START] = new XFormTextStdFormItem;
    mppLocalPoolDefaults[XATTR_FORMTXTHIDEFORM    -XATTR_START] = new XFormTextHideFormItem;
    mppLocalPoolDefaults[XATTR_FORMTXTSHDWTRANSP  -XATTR_START] = new XFormTextShadowTranspItem;

    // create SetItems
    SfxItemSet* pSet=new SfxItemSet(*pMaster, XATTR_LINE_FIRST, XATTR_LINE_LAST);
    mppLocalPoolDefaults[XATTRSET_LINE - XATTR_START] = new XLineAttrSetItem(pSet);
    pSet=new SfxItemSet(*pMaster, XATTR_FILL_FIRST, XATTR_FILL_LAST);
    mppLocalPoolDefaults[XATTRSET_FILL - XATTR_START] = new XFillAttrSetItem(pSet);

    // create ItemInfos
    mpLocalItemInfos = new SfxItemInfo[GetLastWhich() - GetFirstWhich() + 1];
    for(sal_uInt16 i(GetFirstWhich()); i <= GetLastWhich(); i++)
    {
        mpLocalItemInfos[i - XATTR_START]._nSID = 0;
        mpLocalItemInfos[i - XATTR_START]._nFlags = SFX_ITEM_POOLABLE;
    }

    mpLocalItemInfos[XATTR_LINESTYLE        -XATTR_START]._nSID = SID_ATTR_LINE_STYLE;
    mpLocalItemInfos[XATTR_LINEDASH         -XATTR_START]._nSID = SID_ATTR_LINE_DASH;
    mpLocalItemInfos[XATTR_LINEWIDTH        -XATTR_START]._nSID = SID_ATTR_LINE_WIDTH;
    mpLocalItemInfos[XATTR_LINECOLOR        -XATTR_START]._nSID = SID_ATTR_LINE_COLOR;
    mpLocalItemInfos[XATTR_LINESTART        -XATTR_START]._nSID = SID_ATTR_LINE_START;
    mpLocalItemInfos[XATTR_LINEEND          -XATTR_START]._nSID = SID_ATTR_LINE_END;
    mpLocalItemInfos[XATTR_LINESTARTWIDTH   -XATTR_START]._nSID = SID_ATTR_LINE_STARTWIDTH;
    mpLocalItemInfos[XATTR_LINEENDWIDTH     -XATTR_START]._nSID = SID_ATTR_LINE_ENDWIDTH;
    mpLocalItemInfos[XATTR_LINESTARTCENTER  -XATTR_START]._nSID = SID_ATTR_LINE_STARTCENTER;
    mpLocalItemInfos[XATTR_LINEENDCENTER    -XATTR_START]._nSID = SID_ATTR_LINE_ENDCENTER;
    mpLocalItemInfos[XATTR_FILLSTYLE        -XATTR_START]._nSID = SID_ATTR_FILL_STYLE;
    mpLocalItemInfos[XATTR_FILLCOLOR        -XATTR_START]._nSID = SID_ATTR_FILL_COLOR;
    mpLocalItemInfos[XATTR_FILLGRADIENT     -XATTR_START]._nSID = SID_ATTR_FILL_GRADIENT;
    mpLocalItemInfos[XATTR_FILLHATCH        -XATTR_START]._nSID = SID_ATTR_FILL_HATCH;
    mpLocalItemInfos[XATTR_FILLBITMAP       -XATTR_START]._nSID = SID_ATTR_FILL_BITMAP;
    mpLocalItemInfos[XATTR_FORMTXTSTYLE     -XATTR_START]._nSID = SID_FORMTEXT_STYLE;
    mpLocalItemInfos[XATTR_FORMTXTADJUST    -XATTR_START]._nSID = SID_FORMTEXT_ADJUST;
    mpLocalItemInfos[XATTR_FORMTXTDISTANCE  -XATTR_START]._nSID = SID_FORMTEXT_DISTANCE;
    mpLocalItemInfos[XATTR_FORMTXTSTART     -XATTR_START]._nSID = SID_FORMTEXT_START;
    mpLocalItemInfos[XATTR_FORMTXTMIRROR    -XATTR_START]._nSID = SID_FORMTEXT_MIRROR;
    mpLocalItemInfos[XATTR_FORMTXTOUTLINE   -XATTR_START]._nSID = SID_FORMTEXT_OUTLINE;
    mpLocalItemInfos[XATTR_FORMTXTSHADOW    -XATTR_START]._nSID = SID_FORMTEXT_SHADOW;
    mpLocalItemInfos[XATTR_FORMTXTSHDWCOLOR -XATTR_START]._nSID = SID_FORMTEXT_SHDWCOLOR;
    mpLocalItemInfos[XATTR_FORMTXTSHDWXVAL  -XATTR_START]._nSID = SID_FORMTEXT_SHDWXVAL;
    mpLocalItemInfos[XATTR_FORMTXTSHDWYVAL  -XATTR_START]._nSID = SID_FORMTEXT_SHDWYVAL;
    mpLocalItemInfos[XATTR_FORMTXTSTDFORM   -XATTR_START]._nSID = SID_FORMTEXT_STDFORM;
    mpLocalItemInfos[XATTR_FORMTXTHIDEFORM  -XATTR_START]._nSID = SID_FORMTEXT_HIDEFORM;

    // if it's my own creation level, set Defaults and ItemInfos
    if(XATTR_START == GetFirstWhich() && XATTR_END == GetLastWhich())
    {
        SetDefaults(mppLocalPoolDefaults);
        SetItemInfos(mpLocalItemInfos);
    }
}

//BFS01XOutdevItemPool* XOutdevItemPool::Get()
//BFS01{
//BFS01 if ( !pPool )
//BFS01     pPool = new XOutdevItemPool;
//BFS01 return pPool;
//BFS01}

//BFS04//BFS01void XOutdevItemPool::Ctor(SfxItemPool* pMaster, USHORT nAttrStart, USHORT nAttrEnd)
//BFS04void XOutdevItemPool::Ctor(SfxItemPool* pMaster)
//BFS04{
//BFS04 // Mich als Secondary an den MasterPool (Joe)
//BFS04 if (pMaster==NULL) {
//BFS04     pMaster=this;
//BFS04 } else {
//BFS04     // Ich ganz hinten dran
//BFS04     SfxItemPool* pParent=pMaster;
//BFS04     while (pParent->GetSecondaryPool()!=NULL) {
//BFS04         pParent=pParent->GetSecondaryPool();
//BFS04     }
//BFS04     pParent->SetSecondaryPool(this);
//BFS04 }
//BFS04
//BFS04 SfxItemSet* pSet;
//BFS04
//BFS04//BFS01  nStart=nAttrStart;
//BFS04//BFS01  nEnd  =nAttrEnd;
//BFS04//BFS01  ppPoolDefaults = new SfxPoolItem* [nEnd-nStart+1];
//BFS04 ppPoolDefaults = new SfxPoolItem*[XATTR_END - XATTR_START + 1];
//BFS04
//BFS04//BFS01  USHORT i;
//BFS04 sal_uInt16 i;
//BFS04
//BFS04 // Am 27-06-1995 hat Elmar 13 neue Whiches fuer XATTR_FORMTXT bei Which 1021
//BFS04 // eingebaut.
//BFS04 // Alles was zu diesem Zeitpunkt >1021 war verschiebt sich also um
//BFS04 // 13 Positionen nach hinten.
//BFS04 // Alles was davor liegt (11 LineItems, 6 FillItems, 5 von 6 FormTextItems)
//BFS04 // behaelt seine WhichId.
//BFS04
//BFS04//BFS01  for (i=1000; i<=1021; i++) {
//BFS04//BFS01      nVersion1Map[i-1000]=i;
//BFS04//BFS01  }
//BFS04//BFS01  for (i=1022; i<=1039; i++) {
//BFS04//BFS01      nVersion1Map[i-1000]=i+13;
//BFS04//BFS01  }
//BFS04//BFS01  SetVersionMap(1,1000,1039,nVersion1Map);
//BFS04
//BFS04 // Am 09-11-1995 haben dann wiederum Kai Ahrens 14 und Joe 44 (also beide
//BFS04 // insgesamt 58) neue Whiches an verschiedenen Stellen spendiert (siehe
//BFS04 // auch die Beschreibung in svx\inc\SvdAttr.HXX).
//BFS04
//BFS04//BFS01  for (i=1000; i<=1009; i++) {  // XOut
//BFS04//BFS01      nVersion2Map[i-1000]=i;
//BFS04//BFS01  }
//BFS04//BFS01  for (i=1010; i<=1015; i++) {  // XOut
//BFS04//BFS01      nVersion2Map[i-1000]=i+7;
//BFS04//BFS01  }
//BFS04//BFS01  for (i=1016; i<=1035; i++) {  // XOut
//BFS04//BFS01      nVersion2Map[i-1000]=i+14;
//BFS04//BFS01  }
//BFS04//BFS01  for (i=1036; i<=1039; i++) {  // SvDraw
//BFS04//BFS01      nVersion2Map[i-1000]=i+14;
//BFS04//BFS01  }
//BFS04//BFS01  for (i=1040; i<=1050; i++) {  // SvDraw
//BFS04//BFS01      nVersion2Map[i-1000]=i+22;
//BFS04//BFS01  }
//BFS04//BFS01  for (i=1051; i<=1056; i++) {  // SvDraw
//BFS04//BFS01      nVersion2Map[i-1000]=i+27;
//BFS04//BFS01  }
//BFS04//BFS01  for (i=1057; i<=1065; i++) {  // SvDraw
//BFS04//BFS01      nVersion2Map[i-1000]=i+52;
//BFS04//BFS01  }
//BFS04//BFS01  SetVersionMap(2,1000,1065,nVersion2Map);
//BFS04
//BFS04
//BFS04 // Am 29-02-1996 hat KA 17 neue Whiches
//BFS04 // fuer das XOut spendiert
//BFS04
//BFS04//BFS01  for (i=1000; i<=1029; i++) {
//BFS04//BFS01      nVersion3Map[i-1000]=i;
//BFS04//BFS01  }
//BFS04//BFS01  for (i=1030; i<=1123; i++) {
//BFS04//BFS01      nVersion3Map[i-1000]=i+17;
//BFS04//BFS01  }
//BFS04//BFS01  SetVersionMap(3,1000,1123,nVersion3Map);
//BFS04
//BFS04
//BFS04 // Am 10-08-1996 hat Joe 45 neue Items in SvDraw eingebaut
//BFS04 // fuer Bemassung, Verbinder und Laufschrift
//BFS04
//BFS04//BFS01  for (i=1000; i<=1126; i++) {
//BFS04//BFS01      nVersion4Map[i-1000]=i;
//BFS04//BFS01  }
//BFS04//BFS01  for (i=1127; i<=1140; i++) {
//BFS04//BFS01      nVersion4Map[i-1000]=i+45;
//BFS04//BFS01  }
//BFS04//BFS01  SetVersionMap(4,1000,1140,nVersion4Map);
//BFS04
//BFS04
//BFS04 XubString    aNullStr;
//BFS04 Bitmap      aNullBmp;
//BFS04 XPolygon    aNullPol;
//BFS04 Color       aNullLineCol(RGB_Color(COL_BLACK));
//BFS04
//BFS04 Color       aNullFillCol(Color(0,184,255));  // "Blau 7"
//BFS04
//BFS04 Color       aNullShadowCol(RGB_Color(COL_LIGHTGRAY));
//BFS04 XDash       aNullDash;
//BFS04 XGradient   aNullGrad(aNullLineCol, RGB_Color(COL_WHITE));
//BFS04 XHatch      aNullHatch(aNullLineCol);
//BFS04
//BFS04 // LineItems
//BFS04 ppPoolDefaults[XATTR_LINESTYLE          -XATTR_START] = new XLineStyleItem;
//BFS04 ppPoolDefaults[XATTR_LINEDASH           -XATTR_START] = new XLineDashItem(this,aNullDash);
//BFS04 ppPoolDefaults[XATTR_LINEWIDTH          -XATTR_START] = new XLineWidthItem;
//BFS04 ppPoolDefaults[XATTR_LINECOLOR          -XATTR_START] = new XLineColorItem(aNullStr,aNullLineCol);
//BFS04 ppPoolDefaults[XATTR_LINESTART          -XATTR_START] = new XLineStartItem(this,aNullPol);
//BFS04 ppPoolDefaults[XATTR_LINEEND            -XATTR_START] = new XLineEndItem  (this,aNullPol);
//BFS04 ppPoolDefaults[XATTR_LINESTARTWIDTH     -XATTR_START] = new XLineStartWidthItem;
//BFS04 ppPoolDefaults[XATTR_LINEENDWIDTH       -XATTR_START] = new XLineEndWidthItem;
//BFS04 ppPoolDefaults[XATTR_LINESTARTCENTER    -XATTR_START] = new XLineStartCenterItem;
//BFS04 ppPoolDefaults[XATTR_LINEENDCENTER      -XATTR_START] = new XLineEndCenterItem;
//BFS04 ppPoolDefaults[XATTR_LINETRANSPARENCE   -XATTR_START] = new XLineTransparenceItem;
//BFS04 ppPoolDefaults[XATTR_LINEJOINT          -XATTR_START] = new XLineJointItem;
//BFS04
//BFS04 // Reserven fuer LineItems
//BFS04//BFS01  ppPoolDefaults[XATTR_LINERESERVED2      -XATTR_START] = new SfxVoidItem(XATTR_LINERESERVED2);
//BFS04//BFS01  ppPoolDefaults[XATTR_LINERESERVED3      -XATTR_START] = new SfxVoidItem(XATTR_LINERESERVED3);
//BFS04//BFS01  ppPoolDefaults[XATTR_LINERESERVED4      -XATTR_START] = new SfxVoidItem(XATTR_LINERESERVED4);
//BFS04//BFS01  ppPoolDefaults[XATTR_LINERESERVED5      -XATTR_START] = new SfxVoidItem(XATTR_LINERESERVED5);
//BFS04//BFS01  ppPoolDefaults[XATTR_LINERESERVED_LAST  -XATTR_START] = new SfxVoidItem(XATTR_LINERESERVED_LAST);
//BFS04
//BFS04 // FillItems
//BFS04 ppPoolDefaults[XATTR_FILLSTYLE              -XATTR_START] = new XFillStyleItem;
//BFS04 ppPoolDefaults[XATTR_FILLCOLOR              -XATTR_START] = new XFillColorItem   (aNullStr,aNullFillCol);
//BFS04 ppPoolDefaults[XATTR_FILLGRADIENT           -XATTR_START] = new XFillGradientItem(this,aNullGrad);
//BFS04 ppPoolDefaults[XATTR_FILLHATCH              -XATTR_START] = new XFillHatchItem   (this,aNullHatch);
//BFS04 ppPoolDefaults[XATTR_FILLBITMAP             -XATTR_START] = new XFillBitmapItem  (this,aNullBmp);
//BFS04 ppPoolDefaults[XATTR_FILLTRANSPARENCE       -XATTR_START] = new XFillTransparenceItem;
//BFS04 ppPoolDefaults[XATTR_GRADIENTSTEPCOUNT      -XATTR_START] = new XGradientStepCountItem;
//BFS04 ppPoolDefaults[XATTR_FILLBMP_TILE           -XATTR_START] = new XFillBmpTileItem;
//BFS04 ppPoolDefaults[XATTR_FILLBMP_POS            -XATTR_START] = new XFillBmpPosItem;
//BFS04 ppPoolDefaults[XATTR_FILLBMP_SIZEX          -XATTR_START] = new XFillBmpSizeXItem;
//BFS04 ppPoolDefaults[XATTR_FILLBMP_SIZEY          -XATTR_START] = new XFillBmpSizeYItem;
//BFS04 ppPoolDefaults[XATTR_FILLBMP_SIZELOG        -XATTR_START] = new XFillBmpSizeLogItem;
//BFS04 ppPoolDefaults[XATTR_FILLBMP_TILEOFFSETX    -XATTR_START] = new XFillBmpTileOffsetXItem;
//BFS04 ppPoolDefaults[XATTR_FILLBMP_TILEOFFSETY    -XATTR_START] = new XFillBmpTileOffsetYItem;
//BFS04 ppPoolDefaults[XATTR_FILLBMP_STRETCH        -XATTR_START] = new XFillBmpStretchItem;
//BFS04 ppPoolDefaults[XATTR_FILLBMP_POSOFFSETX     -XATTR_START] = new XFillBmpPosOffsetXItem;
//BFS04 ppPoolDefaults[XATTR_FILLBMP_POSOFFSETY     -XATTR_START] = new XFillBmpPosOffsetYItem;
//BFS04 ppPoolDefaults[XATTR_FILLFLOATTRANSPARENCE  -XATTR_START] = new XFillFloatTransparenceItem( this, aNullGrad, FALSE );
//BFS04
//BFS04 // Reserven fuer FillItems
//BFS04 ppPoolDefaults[XATTR_SECONDARYFILLCOLOR     -XATTR_START] = new XSecondaryFillColorItem(aNullStr, aNullFillCol);
//BFS04//BFS01  ppPoolDefaults[XATTR_FILLRESERVED3          -XATTR_START] = new SfxVoidItem(XATTR_FILLRESERVED3);
//BFS04//BFS01  ppPoolDefaults[XATTR_FILLRESERVED4          -XATTR_START] = new SfxVoidItem(XATTR_FILLRESERVED4);
//BFS04//BFS01  ppPoolDefaults[XATTR_FILLRESERVED5          -XATTR_START] = new SfxVoidItem(XATTR_FILLRESERVED5);
//BFS04//BFS01  ppPoolDefaults[XATTR_FILLRESERVED6          -XATTR_START] = new SfxVoidItem(XATTR_FILLRESERVED6);
//BFS04//BFS01  ppPoolDefaults[XATTR_FILLRESERVED7          -XATTR_START] = new SfxVoidItem(XATTR_FILLRESERVED7);
//BFS04//BFS01  ppPoolDefaults[XATTR_FILLRESERVED8          -XATTR_START] = new SfxVoidItem(XATTR_FILLRESERVED8);
//BFS04 ppPoolDefaults[XATTR_FILLBACKGROUND         -XATTR_START] = new XFillBackgroundItem;
//BFS04//BFS01  ppPoolDefaults[XATTR_FILLRESERVED10         -XATTR_START] = new SfxVoidItem(XATTR_FILLRESERVED10);
//BFS04//BFS01  ppPoolDefaults[XATTR_FILLRESERVED11         -XATTR_START] = new SfxVoidItem(XATTR_FILLRESERVED11);
//BFS04//BFS01  ppPoolDefaults[XATTR_FILLRESERVED_LAST      -XATTR_START] = new SfxVoidItem(XATTR_FILLRESERVED_LAST);
//BFS04
//BFS04 // FormTextItems
//BFS04 ppPoolDefaults[XATTR_FORMTXTSTYLE       -XATTR_START] = new XFormTextStyleItem;
//BFS04 ppPoolDefaults[XATTR_FORMTXTADJUST      -XATTR_START] = new XFormTextAdjustItem;
//BFS04 ppPoolDefaults[XATTR_FORMTXTDISTANCE    -XATTR_START] = new XFormTextDistanceItem;
//BFS04 ppPoolDefaults[XATTR_FORMTXTSTART       -XATTR_START] = new XFormTextStartItem;
//BFS04 ppPoolDefaults[XATTR_FORMTXTMIRROR      -XATTR_START] = new XFormTextMirrorItem;
//BFS04 ppPoolDefaults[XATTR_FORMTXTOUTLINE     -XATTR_START] = new XFormTextOutlineItem;
//BFS04 ppPoolDefaults[XATTR_FORMTXTSHADOW      -XATTR_START] = new XFormTextShadowItem;
//BFS04 ppPoolDefaults[XATTR_FORMTXTSHDWCOLOR   -XATTR_START] = new XFormTextShadowColorItem(aNullStr,aNullShadowCol);
//BFS04 ppPoolDefaults[XATTR_FORMTXTSHDWXVAL    -XATTR_START] = new XFormTextShadowXValItem;
//BFS04 ppPoolDefaults[XATTR_FORMTXTSHDWYVAL    -XATTR_START] = new XFormTextShadowYValItem;
//BFS04 ppPoolDefaults[XATTR_FORMTXTSTDFORM     -XATTR_START] = new XFormTextStdFormItem;
//BFS04 ppPoolDefaults[XATTR_FORMTXTHIDEFORM    -XATTR_START] = new XFormTextHideFormItem;
//BFS04
//BFS04 // Reserven fuer FormTextItems
//BFS04 ppPoolDefaults[XATTR_FORMTXTSHDWTRANSP  -XATTR_START] = new XFormTextShadowTranspItem;
//BFS04//BFS01  ppPoolDefaults[XATTR_FTRESERVED2        -XATTR_START] = new SfxVoidItem(XATTR_FTRESERVED2);
//BFS04//BFS01  ppPoolDefaults[XATTR_FTRESERVED3        -XATTR_START] = new SfxVoidItem(XATTR_FTRESERVED3);
//BFS04//BFS01  ppPoolDefaults[XATTR_FTRESERVED4        -XATTR_START] = new SfxVoidItem(XATTR_FTRESERVED4);
//BFS04//BFS01  ppPoolDefaults[XATTR_FTRESERVED5        -XATTR_START] = new SfxVoidItem(XATTR_FTRESERVED5);
//BFS04//BFS01  ppPoolDefaults[XATTR_FTRESERVED_LAST    -XATTR_START] = new SfxVoidItem(XATTR_FTRESERVED_LAST);
//BFS04
//BFS04 // SetItems erzeugen
//BFS04 pSet=new SfxItemSet(*pMaster, XATTR_LINE_FIRST, XATTR_LINE_LAST);
//BFS04 ppPoolDefaults[XATTRSET_LINE - XATTR_START] = new XLineAttrSetItem(pSet);
//BFS04 pSet=new SfxItemSet(*pMaster, XATTR_FILL_FIRST, XATTR_FILL_LAST);
//BFS04 ppPoolDefaults[XATTRSET_FILL - XATTR_START] = new XFillAttrSetItem(pSet);
//BFS04//BFS01  pSet=new SfxItemSet(*pMaster, XATTR_TEXT_FIRST, XATTR_TEXT_LAST);
//BFS04//BFS01  ppPoolDefaults[XATTRSET_TEXT - XATTR_START] = new XTextAttrSetItem(pSet);
//BFS04
//BFS04 // ItemInfos
//BFS04//BFS01  pItemInfos=new SfxItemInfo[nEnd-nStart+1];
//BFS04 pItemInfos = new SfxItemInfo[XATTR_END - XATTR_START + 1];
//BFS04
//BFS04//BFS01  for (i=nStart; i<=nEnd; i++)
//BFS04 for(i = XATTR_START; i <= XATTR_END; i++)
//BFS04 {
//BFS04//BFS01      pItemInfos[i-nStart]._nSID=0;
//BFS04//BFS01      pItemInfos[i-nStart]._nFlags=SFX_ITEM_POOLABLE;
//BFS04     pItemInfos[i - XATTR_START]._nSID = 0;
//BFS04     pItemInfos[i - XATTR_START]._nFlags = SFX_ITEM_POOLABLE;
//BFS04 }
//BFS04
//BFS04 pItemInfos[XATTR_LINESTYLE        -XATTR_START]._nSID = SID_ATTR_LINE_STYLE;
//BFS04 pItemInfos[XATTR_LINEDASH         -XATTR_START]._nSID = SID_ATTR_LINE_DASH;
//BFS04 pItemInfos[XATTR_LINEWIDTH        -XATTR_START]._nSID = SID_ATTR_LINE_WIDTH;
//BFS04 pItemInfos[XATTR_LINECOLOR        -XATTR_START]._nSID = SID_ATTR_LINE_COLOR;
//BFS04 pItemInfos[XATTR_LINESTART        -XATTR_START]._nSID = SID_ATTR_LINE_START;
//BFS04 pItemInfos[XATTR_LINEEND          -XATTR_START]._nSID = SID_ATTR_LINE_END;
//BFS04 pItemInfos[XATTR_LINESTARTWIDTH   -XATTR_START]._nSID = SID_ATTR_LINE_STARTWIDTH;
//BFS04 pItemInfos[XATTR_LINEENDWIDTH     -XATTR_START]._nSID = SID_ATTR_LINE_ENDWIDTH;
//BFS04 pItemInfos[XATTR_LINESTARTCENTER  -XATTR_START]._nSID = SID_ATTR_LINE_STARTCENTER;
//BFS04 pItemInfos[XATTR_LINEENDCENTER    -XATTR_START]._nSID = SID_ATTR_LINE_ENDCENTER;
//BFS04
//BFS04 pItemInfos[XATTR_FILLSTYLE        -XATTR_START]._nSID = SID_ATTR_FILL_STYLE;
//BFS04 pItemInfos[XATTR_FILLCOLOR        -XATTR_START]._nSID = SID_ATTR_FILL_COLOR;
//BFS04 pItemInfos[XATTR_FILLGRADIENT     -XATTR_START]._nSID = SID_ATTR_FILL_GRADIENT;
//BFS04 pItemInfos[XATTR_FILLHATCH        -XATTR_START]._nSID = SID_ATTR_FILL_HATCH;
//BFS04 pItemInfos[XATTR_FILLBITMAP       -XATTR_START]._nSID = SID_ATTR_FILL_BITMAP;
//BFS04
//BFS04 pItemInfos[XATTR_FORMTXTSTYLE     -XATTR_START]._nSID = SID_FORMTEXT_STYLE;
//BFS04 pItemInfos[XATTR_FORMTXTADJUST    -XATTR_START]._nSID = SID_FORMTEXT_ADJUST;
//BFS04 pItemInfos[XATTR_FORMTXTDISTANCE  -XATTR_START]._nSID = SID_FORMTEXT_DISTANCE;
//BFS04 pItemInfos[XATTR_FORMTXTSTART     -XATTR_START]._nSID = SID_FORMTEXT_START;
//BFS04 pItemInfos[XATTR_FORMTXTMIRROR    -XATTR_START]._nSID = SID_FORMTEXT_MIRROR;
//BFS04 pItemInfos[XATTR_FORMTXTOUTLINE   -XATTR_START]._nSID = SID_FORMTEXT_OUTLINE;
//BFS04 pItemInfos[XATTR_FORMTXTSHADOW    -XATTR_START]._nSID = SID_FORMTEXT_SHADOW;
//BFS04 pItemInfos[XATTR_FORMTXTSHDWCOLOR -XATTR_START]._nSID = SID_FORMTEXT_SHDWCOLOR;
//BFS04 pItemInfos[XATTR_FORMTXTSHDWXVAL  -XATTR_START]._nSID = SID_FORMTEXT_SHDWXVAL;
//BFS04 pItemInfos[XATTR_FORMTXTSHDWYVAL  -XATTR_START]._nSID = SID_FORMTEXT_SHDWYVAL;
//BFS04 pItemInfos[XATTR_FORMTXTSTDFORM   -XATTR_START]._nSID = SID_FORMTEXT_STDFORM;
//BFS04 pItemInfos[XATTR_FORMTXTHIDEFORM  -XATTR_START]._nSID = SID_FORMTEXT_HIDEFORM;
//BFS04
//BFS04//BFS01#if SUPD<355
//BFS04//BFS01  pSlotIds = new USHORT[nEnd-nStart+1];
//BFS04//BFS01
//BFS04//BFS01  pSlotIds[XATTR_LINESTYLE        -XATTR_START] = SID_ATTR_LINE_STYLE;
//BFS04//BFS01  pSlotIds[XATTR_LINEDASH         -XATTR_START] = SID_ATTR_LINE_DASH;
//BFS04//BFS01  pSlotIds[XATTR_LINEWIDTH        -XATTR_START] = SID_ATTR_LINE_WIDTH;
//BFS04//BFS01  pSlotIds[XATTR_LINECOLOR        -XATTR_START] = SID_ATTR_LINE_COLOR;
//BFS04//BFS01  pSlotIds[XATTR_LINESTART        -XATTR_START] = SID_ATTR_LINE_START;
//BFS04//BFS01  pSlotIds[XATTR_LINEEND          -XATTR_START] = SID_ATTR_LINE_END;
//BFS04//BFS01  pSlotIds[XATTR_LINESTARTWIDTH   -XATTR_START] = SID_ATTR_LINE_STARTWIDTH;
//BFS04//BFS01  pSlotIds[XATTR_LINEENDWIDTH     -XATTR_START] = SID_ATTR_LINE_ENDWIDTH;
//BFS04//BFS01  pSlotIds[XATTR_LINESTARTCENTER  -XATTR_START] = SID_ATTR_LINE_STARTCENTER;
//BFS04//BFS01  pSlotIds[XATTR_LINEENDCENTER    -XATTR_START] = SID_ATTR_LINE_ENDCENTER;
//BFS04//BFS01  pSlotIds[XATTR_LINETRANSPARENCE -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_LINEJOINT        -XATTR_START] = 0;
//BFS04//BFS01
//BFS04//BFS01  // Line-Reserven
//BFS04//BFS01  pSlotIds[XATTR_LINERESERVED2        -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_LINERESERVED3        -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_LINERESERVED4        -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_LINERESERVED5        -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_LINERESERVED_LAST    -XATTR_START] = 0;
//BFS04//BFS01
//BFS04//BFS01  pSlotIds[XATTR_FILLSTYLE        -XATTR_START] = SID_ATTR_FILL_STYLE;
//BFS04//BFS01  pSlotIds[XATTR_FILLCOLOR        -XATTR_START] = SID_ATTR_FILL_COLOR;
//BFS04//BFS01  pSlotIds[XATTR_FILLGRADIENT     -XATTR_START] = SID_ATTR_FILL_GRADIENT;
//BFS04//BFS01  pSlotIds[XATTR_FILLHATCH        -XATTR_START] = SID_ATTR_FILL_HATCH;
//BFS04//BFS01  pSlotIds[XATTR_FILLBITMAP       -XATTR_START] = SID_ATTR_FILL_BITMAP;
//BFS04//BFS01  pSlotIds[XATTR_FILLTRANSPARENCE -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_GRADIENTSTEPCOUNT-XATTR_START] = 0;
//BFS04//BFS01
//BFS04//BFS01  pSlotIds[XATTR_FILLBMP_TILE             -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLBMP_POS              -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLBMP_SIZEX            -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLBMP_SIZEY            -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLBMP_SIZELOG          -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLBMP_TILEOFFSETX      -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLBMP_TILEOFFSETY      -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLBMP_STRETCH          -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLBMP_POSOFFSETX       -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLBMP_POSOFFSETY       -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLFLOATTRANSPARENCE    -XATTR_START] = 0;
//BFS04//BFS01
//BFS04//BFS01  // Fill-Reserven
//BFS04//BFS01  pSlotIds[XATTR_SECONDARYFILLCOLOR   -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLRESERVED3        -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLRESERVED4        -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLRESERVED5        -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLRESERVED6        -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLRESERVED7        -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLRESERVED8        -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLBACKGROUND       -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLRESERVED10       -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLRESERVED11       -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FILLRESERVED_LAST    -XATTR_START] = 0;
//BFS04//BFS01
//BFS04//BFS01  pSlotIds[XATTR_FORMTXTSTYLE     -XATTR_START] = SID_FORMTEXT_STYLE;
//BFS04//BFS01  pSlotIds[XATTR_FORMTXTADJUST    -XATTR_START] = SID_FORMTEXT_ADJUST;
//BFS04//BFS01  pSlotIds[XATTR_FORMTXTDISTANCE  -XATTR_START] = SID_FORMTEXT_DISTANCE;
//BFS04//BFS01  pSlotIds[XATTR_FORMTXTSTART     -XATTR_START] = SID_FORMTEXT_START;
//BFS04//BFS01  pSlotIds[XATTR_FORMTXTMIRROR    -XATTR_START] = SID_FORMTEXT_MIRROR;
//BFS04//BFS01  pSlotIds[XATTR_FORMTXTOUTLINE   -XATTR_START] = SID_FORMTEXT_OUTLINE;
//BFS04//BFS01  pSlotIds[XATTR_FORMTXTSHADOW    -XATTR_START] = SID_FORMTEXT_SHADOW;
//BFS04//BFS01  pSlotIds[XATTR_FORMTXTSHDWCOLOR -XATTR_START] = SID_FORMTEXT_SHDWCOLOR;
//BFS04//BFS01  pSlotIds[XATTR_FORMTXTSHDWXVAL  -XATTR_START] = SID_FORMTEXT_SHDWXVAL;
//BFS04//BFS01  pSlotIds[XATTR_FORMTXTSHDWYVAL  -XATTR_START] = SID_FORMTEXT_SHDWYVAL;
//BFS04//BFS01  pSlotIds[XATTR_FORMTXTSTDFORM   -XATTR_START] = SID_FORMTEXT_STDFORM;
//BFS04//BFS01  pSlotIds[XATTR_FORMTXTHIDEFORM  -XATTR_START] = SID_FORMTEXT_HIDEFORM;
//BFS04//BFS01
//BFS04//BFS01  // FormText-Reserven
//BFS04//BFS01  pSlotIds[XATTR_FORMTXTSHDWTRANSP-XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FTRESERVED2      -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FTRESERVED3      -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FTRESERVED4      -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FTRESERVED5      -XATTR_START] = 0;
//BFS04//BFS01  pSlotIds[XATTR_FTRESERVED_LAST  -XATTR_START] = 0;
//BFS04//BFS01
//BFS04//BFS01  pSlotIds[XATTRSET_LINE - XATTR_START] = 0; // 20136; alter Hack von MI
//BFS04//BFS01  pSlotIds[XATTRSET_FILL - XATTR_START] = 0; // 20135; alter Hack von MI
//BFS04//BFS01//BFS01   pSlotIds[XATTRSET_TEXT - XATTR_START] = 0;
//BFS04//BFS01#endif
//BFS04
//BFS04//BFS04  if (XATTR_START == GetFirstWhich() && XATTR_END == GetLastWhich())
//BFS04//BFS04  {
//BFS04//BFS04      SetDefaults(ppPoolDefaults);
//BFS04//BFS04      SetItemInfos(pItemInfos);
//BFS04//BFS04//BFS01#if SUPD<355
//BFS04//BFS04//BFS01       SetMap(pSlotIds);
//BFS04//BFS04//BFS01#endif
//BFS04//BFS04  }
//BFS04}


/*************************************************************************
|*
|* copy ctor, sorgt dafuer, dass die static defaults gecloned werden
|*            (Parameter 2 = TRUE)
|*
\************************************************************************/

XOutdevItemPool::XOutdevItemPool(const XOutdevItemPool& rPool)
:   SfxItemPool(rPool, TRUE),
    mppLocalPoolDefaults(0L),
    mpLocalItemInfos(0L)
{
//BFS05 // damit geclonete Pools nicht im dtor auf die Nase fallen
//BFS05 // endgueltige Loesung --> MI
//BFS05 ppPoolDefaults = NULL;
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
//BFS01#if SUPD<355
//BFS01 if (pSlotIds!=NULL) delete[] pSlotIds;
//BFS01#endif

    Delete();

    // remove own static defaults
    if(mppLocalPoolDefaults)
    {
        SfxPoolItem** ppDefaultItem = mppLocalPoolDefaults;
//BFS01     for ( USHORT i = nEnd - nStart + 1; i; --i, ++ppDefaultItem )
        for(sal_uInt16 i(GetLastWhich() - GetFirstWhich() + 1); i; --i, ++ppDefaultItem)
        {
            if ( *ppDefaultItem ) //Teile schon von abgel. Klasse abgeraeumt!
            {
//BFS04#ifndef PRODUCT
                SetRefCount( **ppDefaultItem, 0 );
//BFS04#endif
                delete *ppDefaultItem;
            }
        }

        delete[] mppLocalPoolDefaults;
    }

    if(mpLocalItemInfos)
    {
        delete[] mpLocalItemInfos;
    }
}

// eof
