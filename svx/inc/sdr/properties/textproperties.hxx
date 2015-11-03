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

#ifndef INCLUDED_SVX_SDR_PROPERTIES_TEXTPROPERTIES_HXX
#define INCLUDED_SVX_SDR_PROPERTIES_TEXTPROPERTIES_HXX

#include <svx/itextprovider.hxx>
#include <sdr/properties/attributeproperties.hxx>



namespace sdr
{
    namespace properties
    {
        class TextProperties : public AttributeProperties
        {
        private:
            // #i101556# versioning support
            sal_uInt32                  maVersion;

        protected:
            // create a new itemset
            virtual SfxItemSet* CreateObjectSpecificItemSet(SfxItemPool& rPool) override;

            // Do the ItemChange, may do special handling
            virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = 0) override;

            // react on ItemSet changes
            virtual void ItemSetChanged(const SfxItemSet& rSet) override;

            /// Get the TextProvider related to our SdrObject
            virtual const svx::ITextProvider& getTextProvider() const;

        public:
            // basic constructor
            explicit TextProperties(SdrObject& rObj);

            // constructor for copying, but using new object
            TextProperties(const TextProperties& rProps, SdrObject& rObj);

            // destructor
            virtual ~TextProperties();

            // Clone() operator, normally just calls the local copy constructor
            virtual BaseProperties& Clone(SdrObject& rObj) const override;

            // set a new StyleSheet and broadcast
            virtual void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr) override;

            // force default attributes for a specific object type, called from
            // DefaultProperties::GetObjectItemSet() if a new ItemSet is created
            virtual void ForceDefaultAttributes() override;

            // force all attributes which come from styles to hard attributes
            // to be able to live without the style.
            virtual void ForceStyleToHardAttributes() override;

            // This is the notifier from SfxListener
            virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

            // Set single item at the local ItemSet. *Does not use* AllowItemChange(),
            // ItemChange(), PostItemChange() and ItemSetChanged() calls.
            void SetObjectItemNoBroadcast(const SfxPoolItem& rItem);

            // #i101556# versioning support
            virtual sal_uInt32 getVersion() const override;
        };
    } // end of namespace properties
} // end of namespace sdr



#endif // INCLUDED_SVX_SDR_PROPERTIES_TEXTPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
