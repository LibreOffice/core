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

#include <sal/config.h>

#include <sdr/properties/pageproperties.hxx>
#include <svl/itemset.hxx>
#include <svx/svdobj.hxx>
#include <svx/xdef.hxx>
#include <tools/debug.hxx>


namespace sdr::properties
{
        // create a new itemset
        SfxItemSet PageProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            // override to legally return a valid ItemSet
            return SfxItemSet(rPool);
        }

        PageProperties::PageProperties(SdrObject& rObj)
        :   BaseProperties(rObj)
        {
        }

        PageProperties::PageProperties(const PageProperties& /*rProps*/, SdrObject& rObj)
        :   BaseProperties(rObj)
        {
        }

        PageProperties::~PageProperties()
        {
        }

        std::unique_ptr<BaseProperties> PageProperties::Clone(SdrObject& rObj) const
        {
            return std::unique_ptr<BaseProperties>(new PageProperties(*this, rObj));
        }

        // get itemset. Override here to allow creating the empty itemset
        // without asserting
        const SfxItemSet& PageProperties::GetObjectItemSet() const
        {
            if(!mxEmptyItemSet)
            {
                mxEmptyItemSet.emplace(const_cast<PageProperties*>(this)->CreateObjectSpecificItemSet(GetSdrObject().GetObjectItemPool()));
            }

            DBG_ASSERT(mxEmptyItemSet, "Could not create an SfxItemSet(!)");

            return *mxEmptyItemSet;
        }

        void PageProperties::ItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/)
        {
            // #86481# simply ignore item setting on page objects
        }

        SfxStyleSheet* PageProperties::GetStyleSheet() const
        {
            // override to legally return a 0L pointer here
            return nullptr;
        }

        void PageProperties::SetStyleSheet(SfxStyleSheet* /*pStyleSheet*/, bool /*bDontRemoveHardAttr*/)
        {
            // override to legally ignore the StyleSheet here
        }

        void PageProperties::PostItemChange(const sal_uInt16 nWhich )
        {
            if( (nWhich == XATTR_FILLSTYLE) && mxEmptyItemSet )
                CleanupFillProperties(*mxEmptyItemSet);
        }

        void PageProperties::ClearObjectItem(const sal_uInt16 /*nWhich*/)
        {
            // simply ignore item clearing on page objects
        }

        bool PageProperties::AllowItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/) const
        {
            assert(!"PageProperties::AllowItemChange() should never be called");
            return true;
        }

        void PageProperties::ItemSetChanged(const SfxItemSet* /*pSet*/)
        {
            assert(!"PageProperties::ItemSetChanged() should never be called");
        }

        void PageProperties::SetObjectItem(const SfxPoolItem& /*rItem*/)
        {
            assert(!"PageProperties::SetObjectItem() should never be called");
        }

        void PageProperties::SetObjectItemDirect(const SfxPoolItem& /*rItem*/)
        {
            assert(!"PageProperties::SetObjectItemDirect() should never be called");
        }

        void PageProperties::ClearObjectItemDirect(const sal_uInt16 /*nWhich*/)
        {
            assert(!"PageProperties::ClearObjectItemDirect() should never be called");
        }

        void PageProperties::SetObjectItemSet(const SfxItemSet& /*rSet*/)
        {
            assert(!"PageProperties::SetObjectItemSet() should never be called");
        }
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
