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

#ifndef INCLUDED_SVX_SDR_PROPERTIES_EMPTYPROPERTIES_HXX
#define INCLUDED_SVX_SDR_PROPERTIES_EMPTYPROPERTIES_HXX

#include <svx/sdr/properties/properties.hxx>
#include <svx/svxdllapi.h>



namespace sdr
{
    namespace properties
    {
        class SVX_DLLPUBLIC EmptyProperties : public BaseProperties
        {
        protected:
            // the to be used ItemSet
            SfxItemSet*                                     mpEmptyItemSet;

            // create a new itemset
            virtual SfxItemSet& CreateObjectSpecificItemSet(SfxItemPool& rPool) SAL_OVERRIDE;

            // test changeability for a single item
            virtual bool AllowItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = 0) const SAL_OVERRIDE;

            // Do the ItemChange, may do special handling
            virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = 0) SAL_OVERRIDE;

            // Called after ItemChange() is done for all items.
            virtual void PostItemChange(const sal_uInt16 nWhich) SAL_OVERRIDE;

            // react on ItemSet changes
            virtual void ItemSetChanged(const SfxItemSet& rSet) SAL_OVERRIDE;

        public:
            // basic constructor
            explicit EmptyProperties(SdrObject& rObj);

            // constructor for copying, but using new object
            EmptyProperties(const EmptyProperties& rProps, SdrObject& rObj);

            // destructor
            virtual ~EmptyProperties();

            // Clone() operator, normally just calls the local copy constructor
            virtual BaseProperties& Clone(SdrObject& rObj) const SAL_OVERRIDE;

            // get itemset
            virtual const SfxItemSet& GetObjectItemSet() const SAL_OVERRIDE;

            // set single item
            virtual void SetObjectItem(const SfxPoolItem& rItem) SAL_OVERRIDE;

            // set single item direct, do not do any notifies or things like that
            virtual void SetObjectItemDirect(const SfxPoolItem& rItem) SAL_OVERRIDE;

            // clear single item
            virtual void ClearObjectItem(const sal_uInt16 nWhich = 0) SAL_OVERRIDE;

            // clear single item direct, do not do any notifies or things like that.
            // Also supports complete deleteion of items when default parameter 0 is used.
            virtual void ClearObjectItemDirect(const sal_uInt16 nWhich = 0) SAL_OVERRIDE;

            // set complete item set
            virtual void SetObjectItemSet(const SfxItemSet& rSet) SAL_OVERRIDE;

            // set a new StyleSheet and broadcast
            virtual void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr) SAL_OVERRIDE;

            // get the installed StyleSheet
            virtual SfxStyleSheet* GetStyleSheet() const SAL_OVERRIDE;
        };
    } // end of namespace properties
} // end of namespace sdr

#endif // INCLUDED_SVX_SDR_PROPERTIES_EMPTYPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
