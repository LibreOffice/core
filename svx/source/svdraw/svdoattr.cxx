/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include <svx/svdoattr.hxx>
#include <svx/xpool.hxx>
#include "svx/svditext.hxx"
#include <svx/svdmodel.hxx>
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
#include <svl/smplhint.hxx>
#include <svl/itemiter.hxx>
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


#include <editeng/colritem.hxx>
#include "editeng/fontitem.hxx"
#include <editeng/fhgtitem.hxx>

#include <svx/xlnstcit.hxx>
#include <svx/xlnwtit.hxx>
#include <svl/style.hxx>
#include <svl/whiter.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xlnedcit.hxx>
#include <editeng/adjitem.hxx>
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
#include "svx/xlinjoit.hxx"

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
        ((SdrAttrObj*)this)->bSnapRectDirty = false;
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

void SdrAttrObj::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    SfxSimpleHint *pSimple = PTR_CAST(SfxSimpleHint, &rHint);
    sal_Bool bDataChg(pSimple && SFX_HINT_DATACHANGED == pSimple->GetId());

    if(bDataChg)
    {
        Rectangle aBoundRect = GetLastBoundRect();
        SetBoundRectDirty();
        SetRectsDirty(sal_True);

        // This may have led to object change
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

sal_Bool SdrAttrObj::HasFill() const
{
    return bClosedObj && ((XFillStyleItem&)(GetProperties().GetObjectItemSet().Get(XATTR_FILLSTYLE))).GetValue()!=XFILL_NONE;
}

sal_Bool SdrAttrObj::HasLine() const
{
    return ((XLineStyleItem&)(GetProperties().GetObjectItemSet().Get(XATTR_LINESTYLE))).GetValue()!=XLINE_NONE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
