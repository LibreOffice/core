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

#include <sdr/properties/emptyproperties.hxx>
#include <tools/debug.hxx>
#include <svl/itemset.hxx>
#include <svx/svddef.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpool.hxx>
#include <vcl/outdev.hxx>


namespace sdr
{
    namespace properties
    {
        // create a new itemset
        SfxItemSet* EmptyProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            // Basic implementation; Basic object has NO attributes
            SAL_WARN("svx.sdr", "EmptyProperties::CreateObjectSpecificItemSet() should never be called");
            return new SfxItemSet(rPool);
        }

        EmptyProperties::EmptyProperties(SdrObject& rObj)
        :   BaseProperties(rObj),
            mpEmptyItemSet(nullptr)
        {
        }

        EmptyProperties::EmptyProperties(const EmptyProperties& rProps, SdrObject& rObj)
        :   BaseProperties(rProps, rObj),
            mpEmptyItemSet(nullptr)
        {
            // do not gererate an assert, else derivations like PageProperties will generate an assert
            // using the Clone() operator path.
        }

        EmptyProperties::~EmptyProperties()
        {
            if(mpEmptyItemSet)
            {
                delete mpEmptyItemSet;
                mpEmptyItemSet = nullptr;
            }
        }

        BaseProperties& EmptyProperties::Clone(SdrObject& rObj) const
        {
            return *(new EmptyProperties(*this, rObj));
        }

        const SfxItemSet& EmptyProperties::GetObjectItemSet() const
        {
            if(!mpEmptyItemSet)
            {
                const_cast<EmptyProperties*>(this)->mpEmptyItemSet = const_cast<EmptyProperties*>(this)->CreateObjectSpecificItemSet(GetSdrObject().GetObjectItemPool());
            }

            SAL_WARN_IF(!mpEmptyItemSet, "svx.sdr", "Could not create an SfxItemSet(!)");
            SAL_WARN("svx.sdr", "EmptyProperties::GetObjectItemSet() should never be called (!)");

            return *mpEmptyItemSet;
        }

        void EmptyProperties::SetObjectItem(const SfxPoolItem& /*rItem*/)
        {
            SAL_WARN("svx.sdr", "EmptyProperties::SetObjectItem() should never be called (!)");
        }

        void EmptyProperties::SetObjectItemDirect(const SfxPoolItem& /*rItem*/)
        {
            SAL_WARN("svx.sdr", "EmptyProperties::SetObjectItemDirect() should never be called (!)");
        }

        void EmptyProperties::ClearObjectItem(const sal_uInt16 /*nWhich*/)
        {
            SAL_WARN("svx.sdr", "EmptyProperties::ClearObjectItem() should never be called (!)");
        }

        void EmptyProperties::ClearObjectItemDirect(const sal_uInt16 /*nWhich*/)
        {
            SAL_WARN("svx.sdr", "EmptyProperties::ClearObjectItemDirect() should never be called (!)");
        }

        void EmptyProperties::SetObjectItemSet(const SfxItemSet& /*rSet*/)
        {
            SAL_WARN("svx.sdr", "EmptyProperties::SetObjectItemSet() should never be called (!)");
        }

        void EmptyProperties::ItemSetChanged(const SfxItemSet& /*rSet*/)
        {
            SAL_WARN("svx.sdr", "EmptyProperties::ItemSetChanged() should never be called (!)");
        }

        bool EmptyProperties::AllowItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/) const
        {
            SAL_WARN("svx.sdr", "EmptyProperties::AllowItemChange() should never be called (!)");
            return true;
        }

        void EmptyProperties::ItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/)
        {
            SAL_WARN("svx.sdr", "EmptyProperties::ItemChange() should never be called (!)");
        }

        void EmptyProperties::PostItemChange(const sal_uInt16 /*nWhich*/)
        {
            SAL_WARN("svx.sdr", "EmptyProperties::PostItemChange() should never be called (!)");
        }

        void EmptyProperties::SetStyleSheet(SfxStyleSheet* /*pNewStyleSheet*/, bool /*bDontRemoveHardAttr*/)
        {
            SAL_WARN("svx.sdr", "EmptyProperties::SetStyleSheet() should never be called (!)");
        }

        SfxStyleSheet* EmptyProperties::GetStyleSheet() const
        {
            SAL_WARN("svx.sdr", "EmptyProperties::GetStyleSheet() should never be called (!)");
            return nullptr;
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
