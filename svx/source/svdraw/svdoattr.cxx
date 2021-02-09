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
#include <svx/svdmodel.hxx>
#include <svl/hint.hxx>
#include <svl/itemset.hxx>
#include <svx/xdef.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlnwtit.hxx>
#include <sdr/properties/attributeproperties.hxx>

using namespace com::sun::star;

std::unique_ptr<sdr::properties::BaseProperties> SdrAttrObj::CreateObjectSpecificProperties()
{
    return std::make_unique<sdr::properties::AttributeProperties>(*this);
}

SdrAttrObj::SdrAttrObj(SdrModel& rSdrModel)
    : SdrObject(rSdrModel)
{
}

SdrAttrObj::SdrAttrObj(SdrModel& rSdrModel, SdrAttrObj const& rSource)
    : SdrObject(rSdrModel, rSource)
{
}

SdrAttrObj::~SdrAttrObj() {}

const tools::Rectangle& SdrAttrObj::GetSnapRect() const
{
    if (bSnapRectDirty)
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

    if (bDataChg)
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

    if (drawing::LineStyle_NONE != GetObjectItem(XATTR_LINESTYLE).GetValue())
    {
        nRetval = GetObjectItem(XATTR_LINEWIDTH).GetValue();
    }

    return nRetval;
}

bool SdrAttrObj::HasFill() const
{
    return bClosedObj
           && GetProperties().GetObjectItemSet().Get(XATTR_FILLSTYLE).GetValue()
                  != drawing::FillStyle_NONE;
}

bool SdrAttrObj::HasLine() const
{
    return GetProperties().GetObjectItemSet().Get(XATTR_LINESTYLE).GetValue()
           != drawing::LineStyle_NONE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
