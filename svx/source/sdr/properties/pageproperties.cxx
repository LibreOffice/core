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
#include <svx/svdpool.hxx>
#include <vcl/outdev.hxx>
#include <tools/debug.hxx>


namespace sdr
{
    namespace properties
    {
        // create a new itemset
        std::unique_ptr<SfxItemSet> PageProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            // override to legally return a valid ItemSet
            return std::make_unique<SfxItemSet>(rPool);
        }

        PageProperties::PageProperties(SdrObject& rObj)
        :   EmptyProperties(rObj)
        {
        }

        PageProperties::PageProperties(const PageProperties& /*rProps*/, SdrObject& rObj)
        :   EmptyProperties(rObj)
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
            if(!mpEmptyItemSet)
            {
                const_cast<PageProperties*>(this)->mpEmptyItemSet = const_cast<PageProperties*>(this)->CreateObjectSpecificItemSet(GetSdrObject().GetObjectItemPool());
            }

            DBG_ASSERT(mpEmptyItemSet, "Could not create an SfxItemSet(!)");

            return *mpEmptyItemSet;
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
            if( (nWhich == XATTR_FILLSTYLE) && (mpEmptyItemSet != nullptr) )
                CleanupFillProperties(*mpEmptyItemSet);
        }

        void PageProperties::ClearObjectItem(const sal_uInt16 /*nWhich*/)
        {
            // simply ignore item clearing on page objects
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
