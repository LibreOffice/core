/*************************************************************************
 *
 *  $RCSfile: svdoattr.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 13:21:11 $
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

#include "svdoattr.hxx"
#include "xpool.hxx"
#include "svditext.hxx"
#include "svdtouch.hxx"
#include "svdio.hxx"
#include "svdmodel.hxx"
#include "svdxout.hxx"
#include "svdpage.hxx"
#include "svdattr.hxx"
#include "svdattrx.hxx"
#include "svdpool.hxx"
#include "svdotext.hxx"
#include "svdocapt.hxx"
#include "svdograf.hxx"
#include "svdoole2.hxx"
#include "svdorect.hxx"
#include "svdocirc.hxx"
#include "svdomeas.hxx"

#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif

#ifndef _XENUM_HXX //autogen
#include "xenum.hxx"
#endif

#ifndef _SVX_XLINEIT0_HXX //autogen
#include "xlineit0.hxx"
#endif

#ifndef _SVX_XLNSTWIT_HXX //autogen
#include "xlnstwit.hxx"
#endif

#ifndef _SVX_XLNEDWIT_HXX //autogen
#include "xlnedwit.hxx"
#endif

#ifndef SVX_XFILLIT0_HXX //autogen
#include "xfillit0.hxx"
#endif

#ifndef _SVX_XFLBMTIT_HXX //autogen
#include "xflbmtit.hxx"
#endif

#ifndef _SVX_TEXTIT0_HXX //autogen
#include "xtextit0.hxx"
#endif

#ifndef _SVX_XFLBSTIT_HXX //autogen
#include "xflbstit.hxx"
#endif

#ifndef _SVX_XFLBTOXY_HXX //autogen
#include "xflbtoxy.hxx"
#endif

#ifndef _SVX_XFTSHIT_HXX //autogen
#include "xftshit.hxx"
#endif

#ifndef _EEITEMID_HXX
#include <eeitemid.hxx>
#endif

#ifndef _SVX_COLRITEM_HXX //autogen
#include "colritem.hxx"
#endif

#ifndef _SVX_FONTITEM_HXX //autogen
#include "fontitem.hxx"
#endif

#ifndef _SVX_FHGTITEM_HXX //autogen
#include "fhgtitem.hxx"
#endif

//#include <charscaleitem.hxx>

#ifndef _SVX_XLNSTCIT_HXX //autogen
#include <xlnstcit.hxx>
#endif

#ifndef _SVX_XLNWTIT_HXX //autogen
#include <xlnwtit.hxx>
#endif

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif

#ifndef _SVX_XLNCLIT_HXX //autogen
#include <xlnclit.hxx>
#endif

#ifndef _SVX_XFLCLIT_HXX //autogen
#include <xflclit.hxx>
#endif

#ifndef _SVX_XLNTRIT_HXX //autogen
#include <xlntrit.hxx>
#endif

#ifndef _SVX_XFLTRIT_HXX //autogen
#include <xfltrit.hxx>
#endif

#ifndef _SVX_XLNEDCIT_HXX //autogen
#include <xlnedcit.hxx>
#endif

#ifndef _SVX_ADJITEM_HXX
#include <adjitem.hxx>
#endif

#ifndef _SVX_XFLBCKIT_HXX
#include "xflbckit.hxx"
#endif

#ifndef _XTABLE_HXX
#include "xtable.hxx"
#endif

#ifndef _SVX_XBTMPIT_HXX
#include "xbtmpit.hxx"
#endif
#ifndef _SVX_XLNDSIT_HXX
#include "xlndsit.hxx"
#endif
#ifndef _SVX_XLNEDIT_HXX //autogen
#include "xlnedit.hxx"
#endif
#ifndef _SVX_XFLGRIT_HXX
#include "xflgrit.hxx"
#endif
#ifndef _SVX_XFLFTRIT_HXX
#include "xflftrit.hxx"
#endif
#ifndef _SVX_XFLHTIT_HXX //autogen
#include "xflhtit.hxx"
#endif
#ifndef _SVX_XLNSTIT_HXX
#include "xlnstit.hxx"
#endif

#ifndef _SDR_PROPERTIES_ATTRIBUTEPROPERTIES_HXX
#include <svx/sdr/properties/attributeproperties.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* SdrAttrObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::AttributeProperties(*this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrAttrObj,SdrObject);

SdrAttrObj::SdrAttrObj()
{
}

SdrAttrObj::~SdrAttrObj()
{
}

const Rectangle& SdrAttrObj::GetSnapRect() const
{
    if(bSnapRectDirty)
    {
        ((SdrAttrObj*)this)->RecalcSnapRect();
        ((SdrAttrObj*)this)->bSnapRectDirty = FALSE;
    }
    return maSnapRect;
}

void SdrAttrObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    if(rIn.GetError())
        return;

    // call parent
    SdrObject::ReadData(rHead, rIn);

    SdrDownCompat aCompat(rIn, STREAM_READ);
#ifdef DBG_UTIL
    aCompat.SetID("SdrAttrObj");
#endif
    SfxItemPool* pPool = GetItemPool();

    if(pPool)
    {
        sal_uInt16 nSetID;

        // #89025# if mpObjectItemSet is set and contains items, it is because of ForceDefaultAttr()
        // and the items need to be deleted.
        GetProperties().ClearObjectItemDirect();

        // Do this initialization AFTER the above fix
        SfxItemSet aNewSet(GetMergedItemSet());

        if(rHead.GetVersion() < 11)
            { sal_uInt16 nWhichDum; rIn >> nWhichDum; }
        nSetID = XATTRSET_LINE;
        const XLineAttrSetItem* pLineAttr = (const XLineAttrSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
        if(pLineAttr)
            aNewSet.Put(pLineAttr->GetItemSet());

        if(rHead.GetVersion() < 11)
            { sal_uInt16 nWhichDum; rIn >> nWhichDum; }
        nSetID = XATTRSET_FILL;
        const XFillAttrSetItem* pFillAttr = (const XFillAttrSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
        if(pFillAttr)
            aNewSet.Put(pFillAttr->GetItemSet());

        if(rHead.GetVersion() < 11)
            { sal_uInt16 nWhichDum; rIn >> nWhichDum; }
        nSetID = XATTRSET_TEXT;
        const XTextAttrSetItem* pTextAttr = (const XTextAttrSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
        if(pTextAttr)
            aNewSet.Put(pTextAttr->GetItemSet());

        if(rHead.GetVersion() < 11)
            { sal_uInt16 nWhichDum; rIn >> nWhichDum; }
        nSetID = SDRATTRSET_SHADOW;
        const SdrShadowSetItem* pShadAttr = (const SdrShadowSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
        if(pShadAttr)
            aNewSet.Put(pShadAttr->GetItemSet());

        if(rHead.GetVersion() >= 5)
        {
            if(rHead.GetVersion() < 11)
                { sal_uInt16 nWhichDum; rIn >> nWhichDum; }
            nSetID = SDRATTRSET_OUTLINER;
            const SdrOutlinerSetItem* pOutlAttr = (const SdrOutlinerSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
            if(pOutlAttr)
                aNewSet.Put(pOutlAttr->GetItemSet());
        }

        if(rHead.GetVersion() >= 6)
        {
            if(rHead.GetVersion() < 11)
                { sal_uInt16 nWhichDum; rIn >> nWhichDum; }
            nSetID = SDRATTRSET_MISC;
            const SdrMiscSetItem* pMiscAttr = (const SdrMiscSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
            if(pMiscAttr)
                aNewSet.Put(pMiscAttr->GetItemSet());
        }

        SetMergedItemSet(aNewSet);
    }
    else
    {
        // an den Surrogaten und ggf. auch Whiches vorbeiseeken
        // ganz zu anfang waren es 4 SetItems
        sal_uInt16 nAnz(4);

        if(rHead.GetVersion() >= 5)
            nAnz++;

        if(rHead.GetVersion() >= 6)
            nAnz++;

        nAnz *= sizeof(sal_uInt16);

        if(rHead.GetVersion() < 11)
            nAnz *= 2;

        rIn.SeekRel(nAnz);
    }

    // TextToContour: altes Format(Flag) in neues Format(Item) wandeln
    if(rHead.GetVersion() <= 4 && pPool)
    {
        SetMergedItem(XFormTextStyleItem(XFT_NONE));
    }

    // Fuer die StyleSheetgeschichte gehoert eigentlich auch noch eine
    // Versionsabfrage hierher.
    // Name und Familie des StyleSheet einlesen, in Pointer auf StyleSheet
    // umwandeln lassen (SB)
    XubString aStyleSheetName;
    SfxStyleFamily eFamily;
    sal_uInt16 nRead;

    // UNICODE: rIn>>aStyleSheetName;
    rIn.ReadByteString(aStyleSheetName);

    if(aStyleSheetName.Len())
    {
        rIn >> nRead;
        eFamily = (SfxStyleFamily)(int)nRead;

        // ab Version 1 wird der CharacterSet gelesen, ab V11 nicht mehr
        if(rHead.GetVersion() > 0 && rHead.GetVersion() < 11)
        {
            sal_Int16 nCharSet;
            rIn >> nCharSet;
            //aStyleSheetName.Convert((CharSet)nCharSet);
            // nicht mehr noetig, da ab Vers 11 der CharSet bereits am
            // Stream gesetzt wird.
        }

        DBG_ASSERT(pModel, "SdrAttrObj::ReadData(): pModel=NULL, StyleSheet kann nicht gesetzt werden!");
        if(pModel)
        {
            SfxStyleSheetBasePool *pPool = pModel->GetStyleSheetPool();
            if(pPool)
            {
                SfxStyleSheet *pTmpStyleSheet = (SfxStyleSheet*)pPool->Find(aStyleSheetName, eFamily);
                DBG_ASSERT(pTmpStyleSheet, "SdrAttrObj::ReadData(): StyleSheet nicht gefunden");

                if(pTmpStyleSheet)
                {
                    NbcSetStyleSheet(pTmpStyleSheet, sal_True);
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrAttrObj::WriteData(SvStream& rOut) const
{
    // call parent
    SdrObject::WriteData(rOut);

    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
    SdrDownCompat aCompat(rOut, STREAM_WRITE);
#ifdef DBG_UTIL
    aCompat.SetID("SdrAttrObj");
#endif
    SfxItemPool* pPool = GetItemPool();

    if(pPool)
    {
        const SfxItemSet& rSet = GetMergedItemSet();

        pPool->StoreSurrogate(rOut, &rSet.Get(XATTRSET_LINE));
        pPool->StoreSurrogate(rOut, &rSet.Get(XATTRSET_FILL));
        pPool->StoreSurrogate(rOut, &rSet.Get(XATTRSET_TEXT));
        pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_SHADOW));
        pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_OUTLINER));
        pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_MISC));
    }
    else
    {
        rOut << sal_uInt16(SFX_ITEMS_NULL);
        rOut << sal_uInt16(SFX_ITEMS_NULL);
        rOut << sal_uInt16(SFX_ITEMS_NULL);
        rOut << sal_uInt16(SFX_ITEMS_NULL);
        rOut << sal_uInt16(SFX_ITEMS_NULL);
        rOut << sal_uInt16(SFX_ITEMS_NULL);
    }

    // StyleSheet-Pointer als Name, Familie abspeichern
    // wenn kein StyleSheet vorhanden: leeren String speichern
    if(GetStyleSheet())
    {
        // UNICODE: rOut << pStyleSheet->GetName();
        rOut.WriteByteString(GetStyleSheet()->GetName());
        rOut << (sal_uInt16)(int)(GetStyleSheet()->GetFamily());
    }
    else
    {
        // UNICODE: rOut << String();
        rOut.WriteByteString(String());
    }
}

void SdrAttrObj::SetModel(SdrModel* pNewModel)
{
    SdrModel* pOldModel = pModel;

    // test for correct pool in ItemSet; move to new pool if necessary
    if(pNewModel && GetItemPool() && GetItemPool() != &pNewModel->GetItemPool())
    {
        MigrateItemPool(GetItemPool(), &pNewModel->GetItemPool(), pNewModel);
    }

    // call parent
    SdrObject::SetModel(pNewModel);

    // modify properties
    GetProperties().SetModel(pOldModel, pNewModel);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// syntactical sugar for ItemSet accesses

void __EXPORT SdrAttrObj::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
    const SfxHint& rHint, const TypeId& rHintType)
{
    SfxSimpleHint *pSimple = PTR_CAST(SfxSimpleHint, &rHint);
    BOOL bDataChg(pSimple && SFX_HINT_DATACHANGED == pSimple->GetId());

    if(bDataChg)
    {
        Rectangle aBoundRect = GetLastBoundRect();

        //if(pPage && pPage->IsInserted())
        //  BroadcastObjectChange(); // Erstmal mit dem alten Rect

        SetBoundRectDirty();
        SetRectsDirty(sal_True);

        // This may have lead to object change
        SetChanged();
        BroadcastObjectChange();

        //if(pPage && pPage->IsInserted())
        //{
        //  // looks like one more invalidate, not like a HINT_OBJCHANGED
        //  ActionChanged();
        //  // BroadcastObjectChange();
        //}

        SendUserCall(SDRUSERCALL_CHGATTR, aBoundRect);
    }
}

sal_Int32 SdrAttrObj::ImpGetLineWdt() const
{
    sal_Int32 nRetval(0);

    if(XLINE_NONE != ((XLineStyleItem&)(GetObjectItem(XATTR_LINESTYLE))).GetValue())
    {
        nRetval = ((XLineWidthItem&)(GetObjectItem(XATTR_LINEWIDTH))).GetValue();
    }

    return nRetval;

//#i25616#  const SfxItemSet& rSet = GetMergedItemSet();
//#i25616#  XLineStyle eLine = ((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue();
//#i25616#
//#i25616#  if(XLINE_NONE == eLine)
//#i25616#      return 0; // Garkeine Linie da.
//#i25616#
//#i25616#  sal_Int32 nWdt = ((XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();
//#i25616#
//#i25616#  return nWdt;
}

INT32 SdrAttrObj::ImpGetLineEndAdd() const
{
    const SfxItemSet& rSet = GetMergedItemSet();
    BOOL bStartSet(TRUE);
    BOOL bEndSet(TRUE);

    if(SFX_ITEM_DONTCARE != rSet.GetItemState(XATTR_LINESTART))
    {
        String aStr(((const XLineStartItem&)rSet.Get(XATTR_LINESTART)).GetName());
        if(!aStr.Len())
            bStartSet = FALSE;
    }

    if(rSet.GetItemState(XATTR_LINEEND) != SFX_ITEM_DONTCARE)
    {
        String aStr(((const XLineEndItem&)rSet.Get(XATTR_LINEEND)).GetName());
        if(!aStr.Len())
            bEndSet = FALSE;
    }

    BOOL bLineEndSet = bStartSet || bEndSet;
    XLineStyle eLine = ((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue();

    if(XLINE_NONE == eLine)
        return 0; // Garkeine Linie da.

    // Strichstaerke
    sal_Int32 nLineWdt = ((XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();
    sal_Int32 nSttWdt = ((const XLineStartWidthItem&)(rSet.Get(XATTR_LINESTARTWIDTH))).GetValue();

    if(nSttWdt < 0)
        nSttWdt = -nLineWdt * nSttWdt / 100;

    if(!bLineEndSet)
        nSttWdt = 0;

    BOOL bSttCenter = ((const XLineStartCenterItem&)(rSet.Get(XATTR_LINESTARTCENTER))).GetValue();
    sal_Int32 nSttHgt = 0;

    if(bSttCenter)
    {
        // Linienende steht um die Haelfe ueber
        XPolygon aSttPoly(((const XLineStartItem&)(rSet.Get(XATTR_LINESTART))).GetValue());
        nSttHgt = XOutputDevice::InitLineStartEnd(aSttPoly, nSttWdt, bSttCenter);
        // InitLineStartEnd liefert bei bCenter=TRUE die halbe Hoehe
    }

    nSttWdt++;
    nSttWdt /= 2;

    // Lieber etwas mehr, dafuer keine Wurzel ziehen
    long nSttAdd = Max(nSttWdt, nSttHgt);
    nSttAdd *= 3;
    nSttAdd /= 2;

    long nEndWdt = ((const XLineEndWidthItem&)(rSet.Get(XATTR_LINEENDWIDTH))).GetValue();

    if(nEndWdt < 0)
        nEndWdt = -nLineWdt * nEndWdt / 100; // <0 = relativ

    if(!bLineEndSet)
        nEndWdt = 0;

    BOOL bEndCenter = ((const XLineEndCenterItem&)(rSet.Get(XATTR_LINEENDCENTER))).GetValue();
    sal_Int32 nEndHgt = 0;

    if(bEndCenter)
    {
        // Linienende steht um die Haelfe ueber
        XPolygon aEndPoly(((const XLineEndItem&)(rSet.Get(XATTR_LINEEND))).GetValue());
        nEndHgt = XOutputDevice::InitLineStartEnd(aEndPoly, nEndWdt, bEndCenter);
        // InitLineStartEnd liefert bei bCenter=TRUE die halbe Hoehe
    }

    nEndWdt++;
    nEndWdt /= 2;

    // Lieber etwas mehr, dafuer keine Wurzel ziehen
    sal_Int32 nEndAdd = Max(nEndWdt, nEndHgt);
    nEndAdd *= 3;
    nEndAdd /= 2;

    return Max(nSttAdd, nEndAdd);
}

//////////////////////////////////////////////////////////////////////////////

FASTBOOL SdrAttrObj::ImpLineEndHitTest(const Point& rEndPt, double nSin, double nCos, FASTBOOL bStart, const Point& rHit, USHORT nTol) const
{
    const SfxItemSet& rSet = GetMergedItemSet();
    sal_Int32 nWdt = 0;
    BOOL bCenter = FALSE;
    XPolygon aXPoly;

    if(bStart)
    {
        nWdt = ((const XLineStartWidthItem&)(rSet.Get(XATTR_LINESTARTWIDTH))).GetValue();
        bCenter = ((const XLineStartCenterItem&)(rSet.Get(XATTR_LINESTARTCENTER))).GetValue();
        aXPoly = ((const XLineStartItem&)(rSet.Get(XATTR_LINESTART))).GetValue();
    }
    else
    {
        nWdt = ((const XLineEndWidthItem&)(rSet.Get(XATTR_LINEENDWIDTH))).GetValue();
        bCenter = ((const XLineEndCenterItem&)(rSet.Get(XATTR_LINEENDCENTER))).GetValue();
        aXPoly = ((const XLineEndItem&)(rSet.Get(XATTR_LINEEND))).GetValue();
    }

    if(nWdt < 0)
    {
        sal_Int32 nLineWdt = ((XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue(); // Strichstaerke
        nWdt = -nLineWdt * nWdt / 100; // <0 = relativ
    }

    // InitLineStartEnd liefert bei bCenter=TRUE die halbe Hoehe
    XOutputDevice::InitLineStartEnd(aXPoly, nWdt, bCenter);
    RotateXPoly(aXPoly, Point(), nSin, nCos);
    Point aHit(rHit);
    aHit -= rEndPt;
    Rectangle aHitRect(aHit.X() - nTol, aHit.Y() - nTol, aHit.X() + nTol, aHit.Y() + nTol);
    FASTBOOL bHit = IsRectTouchesPoly(XOutCreatePolygon(aXPoly, NULL), aHitRect);

    return bHit;
}

FASTBOOL SdrAttrObj::ImpGetShadowDist(sal_Int32& nXDist, sal_Int32& nYDist) const
{
    const SfxItemSet& rSet = GetMergedItemSet();

    nXDist = 0L;
    nYDist = 0L;

    BOOL bShadOn = ((SdrShadowItem&)(rSet.Get(SDRATTR_SHADOW))).GetValue();
    if(bShadOn)
    {
        nXDist = ((SdrShadowXDistItem&)(rSet.Get(SDRATTR_SHADOWXDIST))).GetValue();
        nYDist = ((SdrShadowYDistItem&)(rSet.Get(SDRATTR_SHADOWYDIST))).GetValue();
        return TRUE;
    }

    return FALSE;
}

void SdrAttrObj::ImpAddShadowToBoundRect()
{
    sal_Int32 nXDist;
    sal_Int32 nYDist;

    if(ImpGetShadowDist(nXDist, nYDist))
    {
        if(nXDist > 0)
            aOutRect.Right() += nXDist;
        else
            aOutRect.Left() += nXDist;

        if(nYDist > 0)
            aOutRect.Bottom() += nYDist;
        else
            aOutRect.Top() += nYDist;
    }
}

FASTBOOL SdrAttrObj::ImpSetShadowAttributes( const SfxItemSet& rSet, SfxItemSet& rShadowSet ) const
{
    BOOL bShadOn=((SdrShadowItem&)(rSet.Get(SDRATTR_SHADOW))).GetValue();

    if(bShadOn)
    {
// LineAttr for shadow no longer necessary, lines and line shadows are drawn in Paint()
// routines individually (grep for CreateLinePoly())
//
//          if (pLineAttr!=NULL) {
//              XLineAttrSetItem aL(*pLineAttr);
//              aL.GetItemSet().Put(XLineColorItem(String(),aShadCol));
//              aL.GetItemSet().Put(XLineTransparenceItem(nTransp));
//              rXOut.SetLineAttr(aL);
//          }

// #103692# Caller must now handle noFill case
//      if(!bNoFill)
//      {

        const SdrShadowColorItem& rShadColItem = ((const SdrShadowColorItem&)(rSet.Get(SDRATTR_SHADOWCOLOR)));
        Color aShadCol(rShadColItem.GetValue());
        sal_uInt16 nTransp = ((const SdrShadowTransparenceItem&)(rSet.Get(SDRATTR_SHADOWTRANSPARENCE))).GetValue();
        XFillStyle eStyle = ((const XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue();
        BOOL bFillBackground = ((const XFillBackgroundItem&)(rSet.Get(XATTR_FILLBACKGROUND))).GetValue();

        if(eStyle==XFILL_HATCH && !bFillBackground)
        {
            // #41666#
            XHatch aHatch = ((XFillHatchItem&)(rSet.Get(XATTR_FILLHATCH))).GetValue();
            aHatch.SetColor(aShadCol);
            rShadowSet.Put(XFillHatchItem(String(), aHatch));
        }
        else
        {
            if(eStyle != XFILL_NONE && eStyle != XFILL_SOLID)
            {
                // also fuer Gradient und Bitmap
                rShadowSet.Put(XFillStyleItem(XFILL_SOLID));
            }

            rShadowSet.Put(XFillColorItem(String(),aShadCol));

            // #92183# set XFillTransparenceItem only when no FloatTransparence is used,
            // else the OutDev will use the wrong method
            if(nTransp)
            {
                const XFillFloatTransparenceItem& rFillFloatTransparence =
                    (const XFillFloatTransparenceItem&)rSet.Get(XATTR_FILLFLOATTRANSPARENCE);
                if(!rFillFloatTransparence.IsEnabled())
                    rShadowSet.Put(XFillTransparenceItem(nTransp));
            }
        }

        return TRUE;
    }

    return FALSE;
}

BOOL SdrAttrObj::HasFill() const
{
    return (!bClosedObj) ? FALSE
        : ((XFillStyleItem&)(GetProperties().GetObjectItemSet().Get(XATTR_FILLSTYLE))).GetValue()!=XFILL_NONE;
}

BOOL SdrAttrObj::HasLine() const
{
    return ((XLineStyleItem&)(GetProperties().GetObjectItemSet().Get(XATTR_LINESTYLE))).GetValue()!=XLINE_NONE;
}

// eof
