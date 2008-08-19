/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svdoattr.cxx,v $
 * $Revision: 1.52 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdoattr.hxx>
#include <svx/xpool.hxx>
#include "svditext.hxx"
#include "svdtouch.hxx"
#include <svx/svdmodel.hxx>
#include "svdxout.hxx"
#include <svx/svdpage.hxx>
#include <svx/svdattr.hxx>
#include <svx/svdattrx.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdomeas.hxx>
#include <svtools/smplhint.hxx>
#include <svtools/itemiter.hxx>
#include <svx/xenum.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xtextit0.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xflbtoxy.hxx>
#include <svx/xftshit.hxx>


#include <svx/colritem.hxx>
#include "fontitem.hxx"
#include <svx/fhgtitem.hxx>

//#include <svx/charscaleitem.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/xlnwtit.hxx>
#include <svtools/style.hxx>
#include <svtools/style.hxx>
#include <svtools/whiter.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xlnedcit.hxx>
#include <svx/adjitem.hxx>
#include <svx/xflbckit.hxx>
#include <svx/xtable.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/sdr/properties/attributeproperties.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include "xlinjoit.hxx"
#include <svdoimp.hxx>

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
        SetBoundRectDirty();
        SetRectsDirty(sal_True);

        // This may have lead to object change
        SetChanged();
        BroadcastObjectChange();
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
        basegfx::B2DPolyPolygon aSttPoly(((const XLineStartItem&)(rSet.Get(XATTR_LINESTART))).GetLineStartValue());
        nSttHgt = XOutputDevice::getLineStartEndDistance(aSttPoly, nSttWdt, bSttCenter);
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
        basegfx::B2DPolyPolygon aEndPoly(((const XLineEndItem&)(rSet.Get(XATTR_LINEEND))).GetLineEndValue());
        nEndHgt = XOutputDevice::getLineStartEndDistance(aEndPoly, nEndWdt, bEndCenter);
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
