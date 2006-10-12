/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdoattr.cxx,v $
 *
 *  $Revision: 1.47 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 13:11:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "svdoattr.hxx"
#include "xpool.hxx"
#include "svditext.hxx"
#include "svdtouch.hxx"
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

//BFS01void SdrAttrObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
//BFS01{
//BFS01 if(rIn.GetError())
//BFS01     return;
//BFS01
//BFS01 // call parent
//BFS01 SdrObject::ReadData(rHead, rIn);
//BFS01
//BFS01 SdrDownCompat aCompat(rIn, STREAM_READ);
//BFS01#ifdef DBG_UTIL
//BFS01 aCompat.SetID("SdrAttrObj");
//BFS01#endif
//BFS01 SfxItemPool* pPool = GetItemPool();
//BFS01
//BFS01 if(pPool)
//BFS01 {
//BFS01     sal_uInt16 nSetID;
//BFS01
//BFS01     // #89025# if mpObjectItemSet is set and contains items, it is because of ForceDefaultAttr()
//BFS01     // and the items need to be deleted.
//BFS01     GetProperties().ClearObjectItemDirect();
//BFS01
//BFS01     // Do this initialization AFTER the above fix
//BFS01     SfxItemSet aNewSet(GetMergedItemSet());
//BFS01
//BFS01     if(rHead.GetVersion() < 11)
//BFS01         { sal_uInt16 nWhichDum; rIn >> nWhichDum; }
//BFS01     nSetID = XATTRSET_LINE;
//BFS01     const XLineAttrSetItem* pLineAttr = (const XLineAttrSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
//BFS01     if(pLineAttr)
//BFS01         aNewSet.Put(pLineAttr->GetItemSet());
//BFS01
//BFS01     if(rHead.GetVersion() < 11)
//BFS01         { sal_uInt16 nWhichDum; rIn >> nWhichDum; }
//BFS01     nSetID = XATTRSET_FILL;
//BFS01     const XFillAttrSetItem* pFillAttr = (const XFillAttrSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
//BFS01     if(pFillAttr)
//BFS01         aNewSet.Put(pFillAttr->GetItemSet());
//BFS01
//BFS01     if(rHead.GetVersion() < 11)
//BFS01         { sal_uInt16 nWhichDum; rIn >> nWhichDum; }
//BFS01     nSetID = XATTRSET_TEXT;
//BFS01     const XTextAttrSetItem* pTextAttr = (const XTextAttrSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
//BFS01     if(pTextAttr)
//BFS01         aNewSet.Put(pTextAttr->GetItemSet());
//BFS01
//BFS01     if(rHead.GetVersion() < 11)
//BFS01         { sal_uInt16 nWhichDum; rIn >> nWhichDum; }
//BFS01     nSetID = SDRATTRSET_SHADOW;
//BFS01     const SdrShadowSetItem* pShadAttr = (const SdrShadowSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
//BFS01     if(pShadAttr)
//BFS01         aNewSet.Put(pShadAttr->GetItemSet());
//BFS01
//BFS01     if(rHead.GetVersion() >= 5)
//BFS01     {
//BFS01         if(rHead.GetVersion() < 11)
//BFS01             { sal_uInt16 nWhichDum; rIn >> nWhichDum; }
//BFS01         nSetID = SDRATTRSET_OUTLINER;
//BFS01         const SdrOutlinerSetItem* pOutlAttr = (const SdrOutlinerSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
//BFS01         if(pOutlAttr)
//BFS01             aNewSet.Put(pOutlAttr->GetItemSet());
//BFS01     }
//BFS01
//BFS01     if(rHead.GetVersion() >= 6)
//BFS01     {
//BFS01         if(rHead.GetVersion() < 11)
//BFS01             { sal_uInt16 nWhichDum; rIn >> nWhichDum; }
//BFS01         nSetID = SDRATTRSET_MISC;
//BFS01         const SdrMiscSetItem* pMiscAttr = (const SdrMiscSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
//BFS01         if(pMiscAttr)
//BFS01             aNewSet.Put(pMiscAttr->GetItemSet());
//BFS01     }
//BFS01
//BFS01     SetMergedItemSet(aNewSet);
//BFS01 }
//BFS01 else
//BFS01 {
//BFS01     // an den Surrogaten und ggf. auch Whiches vorbeiseeken
//BFS01     // ganz zu anfang waren es 4 SetItems
//BFS01     sal_uInt16 nAnz(4);
//BFS01
//BFS01     if(rHead.GetVersion() >= 5)
//BFS01         nAnz++;
//BFS01
//BFS01     if(rHead.GetVersion() >= 6)
//BFS01         nAnz++;
//BFS01
//BFS01     nAnz *= sizeof(sal_uInt16);
//BFS01
//BFS01     if(rHead.GetVersion() < 11)
//BFS01         nAnz *= 2;
//BFS01
//BFS01     rIn.SeekRel(nAnz);
//BFS01 }
//BFS01
//BFS01 // TextToContour: altes Format(Flag) in neues Format(Item) wandeln
//BFS01 if(rHead.GetVersion() <= 4 && pPool)
//BFS01 {
//BFS01     SetMergedItem(XFormTextStyleItem(XFT_NONE));
//BFS01 }
//BFS01
//BFS01 // Fuer die StyleSheetgeschichte gehoert eigentlich auch noch eine
//BFS01 // Versionsabfrage hierher.
//BFS01 // Name und Familie des StyleSheet einlesen, in Pointer auf StyleSheet
//BFS01 // umwandeln lassen (SB)
//BFS01 XubString aStyleSheetName;
//BFS01 SfxStyleFamily eFamily;
//BFS01 sal_uInt16 nRead;
//BFS01
//BFS01 // UNICODE: rIn>>aStyleSheetName;
//BFS01 rIn.ReadByteString(aStyleSheetName);
//BFS01
//BFS01 if(aStyleSheetName.Len())
//BFS01 {
//BFS01     rIn >> nRead;
//BFS01     eFamily = (SfxStyleFamily)(int)nRead;
//BFS01
//BFS01     // ab Version 1 wird der CharacterSet gelesen, ab V11 nicht mehr
//BFS01     if(rHead.GetVersion() > 0 && rHead.GetVersion() < 11)
//BFS01     {
//BFS01         sal_Int16 nCharSet;
//BFS01         rIn >> nCharSet;
//BFS01         //aStyleSheetName.Convert((CharSet)nCharSet);
//BFS01         // nicht mehr noetig, da ab Vers 11 der CharSet bereits am
//BFS01         // Stream gesetzt wird.
//BFS01     }
//BFS01
//BFS01     DBG_ASSERT(pModel, "SdrAttrObj::ReadData(): pModel=NULL, StyleSheet kann nicht gesetzt werden!");
//BFS01     if(pModel)
//BFS01     {
//BFS01         SfxStyleSheetBasePool *pPool = pModel->GetStyleSheetPool();
//BFS01         if(pPool)
//BFS01         {
//BFS01             SfxStyleSheet *pTmpStyleSheet = (SfxStyleSheet*)pPool->Find(aStyleSheetName, eFamily);
//BFS01             DBG_ASSERT(pTmpStyleSheet, "SdrAttrObj::ReadData(): StyleSheet nicht gefunden");
//BFS01
//BFS01             if(pTmpStyleSheet)
//BFS01             {
//BFS01                 NbcSetStyleSheet(pTmpStyleSheet, sal_True);
//BFS01             }
//BFS01         }
//BFS01     }
//BFS01 }
//BFS01}

////////////////////////////////////////////////////////////////////////////////////////////////////

//BFS01void SdrAttrObj::WriteData(SvStream& rOut) const
//BFS01{
//BFS01 // call parent
//BFS01 SdrObject::WriteData(rOut);
//BFS01
//BFS01 // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//BFS01 SdrDownCompat aCompat(rOut, STREAM_WRITE);
//BFS01#ifdef DBG_UTIL
//BFS01 aCompat.SetID("SdrAttrObj");
//BFS01#endif
//BFS01 SfxItemPool* pPool = GetItemPool();
//BFS01
//BFS01 if(pPool)
//BFS01 {
//BFS01     const SfxItemSet& rSet = GetMergedItemSet();
//BFS01
//BFS01     pPool->StoreSurrogate(rOut, &rSet.Get(XATTRSET_LINE));
//BFS01     pPool->StoreSurrogate(rOut, &rSet.Get(XATTRSET_FILL));
//BFS01     pPool->StoreSurrogate(rOut, &rSet.Get(XATTRSET_TEXT));
//BFS01     pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_SHADOW));
//BFS01     pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_OUTLINER));
//BFS01     pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_MISC));
//BFS01 }
//BFS01 else
//BFS01 {
//BFS01     rOut << sal_uInt16(SFX_ITEMS_NULL);
//BFS01     rOut << sal_uInt16(SFX_ITEMS_NULL);
//BFS01     rOut << sal_uInt16(SFX_ITEMS_NULL);
//BFS01     rOut << sal_uInt16(SFX_ITEMS_NULL);
//BFS01     rOut << sal_uInt16(SFX_ITEMS_NULL);
//BFS01     rOut << sal_uInt16(SFX_ITEMS_NULL);
//BFS01 }
//BFS01
//BFS01 // StyleSheet-Pointer als Name, Familie abspeichern
//BFS01 // wenn kein StyleSheet vorhanden: leeren String speichern
//BFS01 if(GetStyleSheet())
//BFS01 {
//BFS01     // UNICODE: rOut << pStyleSheet->GetName();
//BFS01     rOut.WriteByteString(GetStyleSheet()->GetName());
//BFS01     rOut << (sal_uInt16)(int)(GetStyleSheet()->GetFamily());
//BFS01 }
//BFS01 else
//BFS01 {
//BFS01     // UNICODE: rOut << String();
//BFS01     rOut.WriteByteString(String());
//BFS01 }
//BFS01}

void SdrAttrObj::SetModel(SdrModel* pNewModel)
{
    SdrModel* pOldModel = pModel;

    // test for correct pool in ItemSet; move to new pool if necessary
    if(pNewModel && GetObjectItemPool() && GetObjectItemPool() != &pNewModel->GetItemPool())
    {
        MigrateItemPool(GetObjectItemPool(), &pNewModel->GetItemPool(), pNewModel);
    }

    // call parent
    SdrObject::SetModel(pNewModel);

    // modify properties
    GetProperties().SetModel(pOldModel, pNewModel);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// syntactical sugar for ItemSet accesses

void __EXPORT SdrAttrObj::SFX_NOTIFY(SfxBroadcaster& /*rBC*/, const TypeId& rBCType,
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
        XPolygon aSttPoly(((const XLineStartItem&)(rSet.Get(XATTR_LINESTART))).GetLineStartValue());
        nSttHgt = XOutputDevice::InitLineStartEnd(aSttPoly, nSttWdt, bSttCenter);
        // InitLineStartEnd liefert bei bCenter=TRUE die halbe Hoehe
    }

    nSttWdt++;
    nSttWdt /= 2;

    // Lieber etwas mehr, dafuer keine Wurzel ziehen
    sal_Int32 nSttAdd = Max(nSttWdt, nSttHgt);
    nSttAdd *= 3;
    nSttAdd /= 2;

    sal_Int32 nEndWdt = ((const XLineEndWidthItem&)(rSet.Get(XATTR_LINEENDWIDTH))).GetValue();

    if(nEndWdt < 0)
        nEndWdt = -nLineWdt * nEndWdt / 100; // <0 = relativ

    if(!bLineEndSet)
        nEndWdt = 0;

    BOOL bEndCenter = ((const XLineEndCenterItem&)(rSet.Get(XATTR_LINEENDCENTER))).GetValue();
    sal_Int32 nEndHgt = 0;

    if(bEndCenter)
    {
        // Linienende steht um die Haelfe ueber
        XPolygon aEndPoly(((const XLineEndItem&)(rSet.Get(XATTR_LINEEND))).GetLineEndValue());
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

//BFS09sal_Bool SdrAttrObj::ImpLineEndHitTest(const Point& rEndPt, double nSin, double nCos, FASTBOOL bStart, const Point& rHit, USHORT nTol) const
//BFS09{
//BFS09 const SfxItemSet& rSet = GetMergedItemSet();
//BFS09 sal_Int32 nWdt = 0;
//BFS09 BOOL bCenter = FALSE;
//BFS09 XPolygon aXPoly;
//BFS09
//BFS09 if(bStart)
//BFS09 {
//BFS09     nWdt = ((const XLineStartWidthItem&)(rSet.Get(XATTR_LINESTARTWIDTH))).GetValue();
//BFS09     bCenter = ((const XLineStartCenterItem&)(rSet.Get(XATTR_LINESTARTCENTER))).GetValue();
//BFS09     aXPoly = ((const XLineStartItem&)(rSet.Get(XATTR_LINESTART))).GetValue();
//BFS09 }
//BFS09 else
//BFS09 {
//BFS09     nWdt = ((const XLineEndWidthItem&)(rSet.Get(XATTR_LINEENDWIDTH))).GetValue();
//BFS09     bCenter = ((const XLineEndCenterItem&)(rSet.Get(XATTR_LINEENDCENTER))).GetValue();
//BFS09     aXPoly = ((const XLineEndItem&)(rSet.Get(XATTR_LINEEND))).GetValue();
//BFS09 }
//BFS09
//BFS09 if(nWdt < 0)
//BFS09 {
//BFS09     sal_Int32 nLineWdt = ((XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue(); // Strichstaerke
//BFS09     nWdt = -nLineWdt * nWdt / 100; // <0 = relativ
//BFS09 }
//BFS09
//BFS09 // InitLineStartEnd liefert bei bCenter=TRUE die halbe Hoehe
//BFS09 XOutputDevice::InitLineStartEnd(aXPoly, nWdt, bCenter);
//BFS09 RotateXPoly(aXPoly, Point(), nSin, nCos);
//BFS09 Point aHit(rHit);
//BFS09 aHit -= rEndPt;
//BFS09 Rectangle aHitRect(aHit.X() - nTol, aHit.Y() - nTol, aHit.X() + nTol, aHit.Y() + nTol);
//BFS09//BFS09  FASTBOOL bHit = IsRectTouchesPoly(XOutCreatePolygon(aXPoly, NULL), aHitRect);
//BFS09 sal_Bool bHit(IsRectTouchesPoly(XOutCreatePolygon(aXPoly), aHitRect));
//BFS09
//BFS09 return bHit;
//BFS09}

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
        Color aShadCol(rShadColItem.GetColorValue());
        sal_uInt16 nTransp = ((const SdrShadowTransparenceItem&)(rSet.Get(SDRATTR_SHADOWTRANSPARENCE))).GetValue();
        XFillStyle eStyle = ((const XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue();
        BOOL bFillBackground = ((const XFillBackgroundItem&)(rSet.Get(XATTR_FILLBACKGROUND))).GetValue();

        if(eStyle==XFILL_HATCH && !bFillBackground)
        {
            // #41666#
            XHatch aHatch = ((XFillHatchItem&)(rSet.Get(XATTR_FILLHATCH))).GetHatchValue();
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
    return bClosedObj && ((XFillStyleItem&)(GetProperties().GetObjectItemSet().Get(XATTR_FILLSTYLE))).GetValue()!=XFILL_NONE;
}

BOOL SdrAttrObj::HasLine() const
{
    return ((XLineStyleItem&)(GetProperties().GetObjectItemSet().Get(XATTR_LINESTYLE))).GetValue()!=XLINE_NONE;
}

// eof
