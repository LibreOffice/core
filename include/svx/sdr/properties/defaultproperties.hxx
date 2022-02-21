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

#ifndef INCLUDED_SVX_SDR_PROPERTIES_DEFAULTPROPERTIES_HXX
#define INCLUDED_SVX_SDR_PROPERTIES_DEFAULTPROPERTIES_HXX

#include <sal/config.h>

#include <optional>

#include <svx/sdr/properties/properties.hxx>
#include <svx/svxdllapi.h>
#include <svl/itemset.hxx>
#include <o3tl/span.hxx>

struct _xmlTextWriter;
typedef struct _xmlTextWriter* xmlTextWriterPtr;

namespace sdr::properties
    {
        class SVXCORE_DLLPUBLIC DefaultProperties : public BaseProperties
        {
        protected:
            // the to be used ItemSet
            mutable std::optional<SfxItemSet> mxItemSet;

            // create a new object specific itemset with object specific ranges.
            virtual SfxItemSet CreateObjectSpecificItemSet(SfxItemPool& pPool) override;

            // Test changeability for a single item. If an implementation wants to prevent
            // changing an item it should override this method.
            virtual bool AllowItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = nullptr) const;

            // Do the internal ItemChange. If only nWhich is given, the item needs to be cleared.
            // Also needs to handle if nWhich and pNewItem is 0, which means to clear all items.
            virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = nullptr);

            // Called after ItemChange() is done for all items. Allows local reactions on
            // specific item changes
            virtual void PostItemChange(const sal_uInt16 nWhich);

            // Internally react on ItemSet changes. The given span contains changed items.
            // If nDeletedWhich is not 0, it indicates a deleted item.
            virtual void ItemSetChanged(o3tl::span< const SfxPoolItem* const > aChangedItems, sal_uInt16 nDeletedWhich);

            // check if SfxItemSet exists
            bool HasSfxItemSet() const { return bool(mxItemSet); }

        public:
            // basic constructor
            explicit DefaultProperties(SdrObject& rObj);

            // constructor for copying, but using new object
            DefaultProperties(const DefaultProperties& rProps, SdrObject& rObj);

            // destructor
            virtual ~DefaultProperties() override;

            void dumpAsXml(xmlTextWriterPtr pWriter) const override;

            // Clone() operator, normally just calls the local copy constructor
            virtual std::unique_ptr<BaseProperties> Clone(SdrObject& rObj) const override;

            // get itemset
            virtual const SfxItemSet& GetObjectItemSet() const override;

            // set single item
            virtual void SetObjectItem(const SfxPoolItem& rItem) override;

            // set single item direct, do not do any notifies or things like that
            virtual void SetObjectItemDirect(const SfxPoolItem& rItem) override;

            // clear single item
            virtual void ClearObjectItem(const sal_uInt16 nWhich = 0) override;

            // clear single item direct, do not do any notifies or things like that.
            // Also supports complete deletion of items when default parameter 0 is used.
            virtual void ClearObjectItemDirect(const sal_uInt16 nWhich) override;

            // set complete item set
            virtual void SetObjectItemSet(const SfxItemSet& rSet) override;

            // set a new StyleSheet and broadcast
            virtual void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr,
                bool bBroadcast) override;

            // get the installed StyleSheet
            virtual SfxStyleSheet* GetStyleSheet() const override;

            // force default attributes for a specific object type, called from
            // DefaultProperties::GetObjectItemSet() if a new ItemSet is created.
            // Default implementation does nothing.
            virtual void ForceDefaultAttributes();
        };

} // end of namespace sdr::properties

#endif // INCLUDED_SVX_SDR_PROPERTIES_DEFAULTPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
