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

#ifndef INCLUDED_SVX_INC_SDR_PROPERTIES_PAGEPROPERTIES_HXX
#define INCLUDED_SVX_INC_SDR_PROPERTIES_PAGEPROPERTIES_HXX

#include <sdr/properties/emptyproperties.hxx>


namespace sdr::properties
    {
        class PageProperties final : public EmptyProperties
        {
            // create a new object specific itemset with object specific ranges.
            virtual SfxItemSet CreateObjectSpecificItemSet(SfxItemPool& pPool) override;

            // Do the ItemChange, may do special handling
            virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = nullptr) override;

            // Called after ItemChange() is done for all items.
            virtual void PostItemChange(const sal_uInt16 nWhich) override;

        public:
            // basic constructor
            explicit PageProperties(SdrObject& rObj);

            // constructor for copying, but using new object
            PageProperties(const PageProperties& rProps, SdrObject& rObj);

            // destructor
            virtual ~PageProperties() override;

            // Clone() operator, normally just calls the local copy constructor
            virtual std::unique_ptr<BaseProperties> Clone(SdrObject& rObj) const override;

            // get itemset. Override here to allow creating the empty itemset
            // without asserting
            virtual const SfxItemSet& GetObjectItemSet() const override;

            // get the installed StyleSheet
            virtual SfxStyleSheet* GetStyleSheet() const override;

            // set the installed StyleSheet
            virtual void SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr) override;

            // clear single item
            virtual void ClearObjectItem(const sal_uInt16 nWhich = 0) override;
        };
} // end of namespace sdr::properties


#endif // INCLUDED_SVX_INC_SDR_PROPERTIES_PAGEPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
