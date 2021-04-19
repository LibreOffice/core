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

#ifndef INCLUDED_SVX_SDR_PROPERTIES_ATTRIBUTEPROPERTIES_HXX
#define INCLUDED_SVX_SDR_PROPERTIES_ATTRIBUTEPROPERTIES_HXX

#include <svl/lstner.hxx>
#include <svl/stylesheetuser.hxx>
#include <svx/sdr/properties/defaultproperties.hxx>


namespace sdr::properties
    {
        class AttributeProperties : public DefaultProperties, public SfxListener, public svl::StyleSheetUser
        {
            // core to set parent at SfxItemSet and to execute the hard attribute computations
            void ImpSetParentAtSfxItemSet(bool bDontRemoveHardAttr);

            // add style sheet, do all the necessary handling
            void ImpAddStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr);

            // remove StyleSheet, do all the necessary handling
            void ImpRemoveStyleSheet();

        protected:
            // the StyleSheet of this object
            SfxStyleSheet*                                  mpStyleSheet;

            // create a new itemset
            virtual SfxItemSet CreateObjectSpecificItemSet(SfxItemPool& pPool) override;

            // Do the ItemChange, may do special handling
            virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = nullptr) override;

            // react on ItemSet changes
            virtual void ItemSetChanged(const SfxItemSet& rSet) override;

        public:
            // basic constructor
            explicit AttributeProperties(SdrObject& rObj);

            // constructor for copying, but using new object
            AttributeProperties(const AttributeProperties& rProps, SdrObject& rObj);

            // Clone() operator, normally just calls the local copy constructor
            virtual std::unique_ptr<BaseProperties> Clone(SdrObject& rObj) const override;

            // Get the local ItemSet. This directly returns the local ItemSet of the object. No
            // merging of ItemSets is done for e.g. Group objects.
            virtual const SfxItemSet& GetObjectItemSet() const override;

            // destructor
            virtual ~AttributeProperties() override;

            // set a new StyleSheet and broadcast
            virtual void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr) override;

            // get the installed StyleSheet
            virtual SfxStyleSheet* GetStyleSheet() const override;

            // force all attributes which come from styles to hard attributes
            // to be able to live without the style.
            virtual void ForceStyleToHardAttributes() override;

            // This is the Notify(...) from 2nd base class SfxListener
            virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

            virtual bool isUsedByModel() const override;
        };

} // end of namespace sdr::properties


#endif // INCLUDED_SVX_SDR_PROPERTIES_ATTRIBUTEPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
