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

#ifndef INCLUDED_SVX_INC_SDR_PROPERTIES_CUSTOMSHAPEPROPERTIES_HXX
#define INCLUDED_SVX_INC_SDR_PROPERTIES_CUSTOMSHAPEPROPERTIES_HXX

#include <sdr/properties/textproperties.hxx>


namespace sdr
{
    namespace properties
    {
        class CustomShapeProperties : public TextProperties
        {
        private:
            void UpdateTextFrameStatus(bool bInvalidateRenderGeometry);

        protected:
            // create a new itemset
            virtual SfxItemSet* CreateObjectSpecificItemSet(SfxItemPool& rPool) override;

            // test changeability for a single item
            virtual bool AllowItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = nullptr) const override;

            // react on ItemSet changes
            virtual void ItemSetChanged(const SfxItemSet& rSet) override;

            // react on Item change
            virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = nullptr) override;

            // Called after ItemChange() is done for all items. Allows local reactions on
            // specific item changes
            virtual void PostItemChange(const sal_uInt16 nWhich) override;

            // clear single item
            virtual void ClearObjectItem(const sal_uInt16 nWhich = 0) override;

            // clear single item direct, do not do any notifies or things like that.
            // Also supports complete deleteion of items when default parameter 0 is used.
            virtual void ClearObjectItemDirect(const sal_uInt16 nWhich = 0) override;

        public:

            // set a new StyleSheet and broadcast
            virtual void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr) override;

            // force default attributes for a specific object type, called from
            // DefaultProperties::GetObjectItemSet() if a new ItemSet is created
            virtual void ForceDefaultAttributes() override;

            // basic constructor
            explicit CustomShapeProperties(SdrObject& rObj);

            // constructor for copying, but using new object
            CustomShapeProperties(const CustomShapeProperties& rProps, SdrObject& rObj);

            // destructor
            virtual ~CustomShapeProperties();

            // Clone() operator, normally just calls the local copy constructor
            virtual BaseProperties& Clone(SdrObject& rObj) const override;

            // This is the notifier from SfxListener
            virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;
        };
    } // end of namespace properties
} // end of namespace sdr


#endif // INCLUDED_SVX_INC_SDR_PROPERTIES_CUSTOMSHAPEPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
