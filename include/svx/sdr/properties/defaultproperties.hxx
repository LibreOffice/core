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

#include <memory>

#include <svx/sdr/properties/properties.hxx>
#include <svx/svxdllapi.h>

struct _xmlTextWriter;
typedef struct _xmlTextWriter* xmlTextWriterPtr;

namespace sdr
{
    namespace properties
    {
        class SVX_DLLPUBLIC DefaultProperties : public BaseProperties
        {
        protected:
            // the to be used ItemSet
            std::unique_ptr<SfxItemSet> mpItemSet;

            // create a new itemset
            virtual std::unique_ptr<SfxItemSet> CreateObjectSpecificItemSet(SfxItemPool& rPool) override;

            // test changeability for a single item
            virtual bool AllowItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = nullptr) const override;

            // Do the ItemChange, may do special handling
            virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = nullptr) override;

            // Called after ItemChange() is done for all items.
            virtual void PostItemChange(const sal_uInt16 nWhich) override;

            // react on ItemSet changes
            virtual void ItemSetChanged(const SfxItemSet& rSet) override;

            // check if SfxItemSet exists
            bool HasSfxItemSet() const { return bool(mpItemSet); }

        public:
            // basic constructor
            explicit DefaultProperties(SdrObject& rObj);

            // constructor for copying, but using new object
            DefaultProperties(const DefaultProperties& rProps, SdrObject& rObj);

            // destructor
            virtual ~DefaultProperties() override;

            void dumpAsXml(xmlTextWriterPtr pWriter) const;

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
            virtual void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr) override;

            // get the installed StyleSheet
            virtual SfxStyleSheet* GetStyleSheet() const override;

            // force default attributes for a specific object type, called from
            // DefaultProperties::GetObjectItemSet() if a new ItemSet is created.
            // Default implementation does nothing.
            virtual void ForceDefaultAttributes();
        };
    } // end of namespace properties
} // end of namespace sdr

#endif // INCLUDED_SVX_SDR_PROPERTIES_DEFAULTPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
