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
#include <tools/debug.hxx>


namespace sdr::properties
{
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

        SfxItemSet PageProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return SfxItemSet(rPool);
        }

        // get itemset. Override here to allow creating the empty itemset
        // without asserting
        const SfxItemSet& PageProperties::GetObjectItemSet() const
        {
            if(!mxEmptyItemSet)
            {
                mxEmptyItemSet.emplace(GetSdrObject().GetObjectItemPool());
            }

            DBG_ASSERT(mxEmptyItemSet, "Could not create an SfxItemSet(!)");

            return *mxEmptyItemSet;
        }

        SfxStyleSheet* PageProperties::GetStyleSheet() const
        {
            // override to legally return a 0L pointer here
            return nullptr;
        }

        void PageProperties::SetStyleSheet(SfxStyleSheet* /*pStyleSheet*/, bool /*bDontRemoveHardAttr*/,
                bool /*bBroadcast*/)
        {
            // override to legally ignore the StyleSheet here
        }

        void PageProperties::ClearObjectItem(const sal_uInt16 /*nWhich*/)
        {
            // simply ignore item clearing on page objects
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
