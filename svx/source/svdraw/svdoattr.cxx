/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <svx/svdoattr.hxx>
#include <svx/xpool.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdattr.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdomeas.hxx>
#include <svl/hint.hxx>
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

#include <editeng/editdata.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fontitem.hxx>
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
#include <editeng/adjustitem.hxx>
#include <svx/xflbckit.hxx>
#include <svx/xtable.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xlnstit.hxx>
#include <sdr/properties/attributeproperties.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svx/xlinjoit.hxx>

using namespace com::sun::star;

std::unique_ptr<sdr::properties::BaseProperties> SdrAttrObj::CreateObjectSpecificProperties()
{
    return std::make_unique<sdr::properties::AttributeProperties>(*this);
}


SdrAttrObj::SdrAttrObj(SdrModel& rSdrModel)
:   SdrObject(rSdrModel)
{
}

SdrAttrObj::~SdrAttrObj()
{
}

const tools::Rectangle& SdrAttrObj::GetSnapRect() const
{
    if(bSnapRectDirty)
    {
        const_cast<SdrAttrObj*>(this)->RecalcSnapRect();
        const_cast<SdrAttrObj*>(this)->bSnapRectDirty = false;
    }

    return maSnapRect;
}

// syntactical sugar for ItemSet accesses
void SdrAttrObj::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    bool bDataChg(SfxHintId::DataChanged == rHint.GetId());

    if(bDataChg)
    {
        tools::Rectangle aBoundRect = GetLastBoundRect();
        SetBoundRectDirty();
        SetRectsDirty(true);

        // This may have led to object change
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SdrUserCallType::ChangeAttr, aBoundRect);
    }
}

sal_Int32 SdrAttrObj::ImpGetLineWdt() const
{
    sal_Int32 nRetval(0);

    if(drawing::LineStyle_NONE != GetObjectItem(XATTR_LINESTYLE).GetValue())
    {
        nRetval = GetObjectItem(XATTR_LINEWIDTH).GetValue();
    }

    return nRetval;
}

bool SdrAttrObj::HasFill() const
{
    return bClosedObj && GetProperties().GetObjectItemSet().Get(XATTR_FILLSTYLE).GetValue() != drawing::FillStyle_NONE;
}

bool SdrAttrObj::HasLine() const
{
    return GetProperties().GetObjectItemSet().Get(XATTR_LINESTYLE).GetValue() != drawing::LineStyle_NONE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
