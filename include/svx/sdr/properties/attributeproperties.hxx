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
#include <svx/svxdllapi.h>



namespace sdr
{
    namespace properties
    {
        class SVX_DLLPUBLIC AttributeProperties : public DefaultProperties, public SfxListener, public svl::StyleSheetUser
        {
            // add style sheet, do all the necessary handling
            void ImpAddStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr);

            // remove StyleSheet, do all the necessary handling
            void ImpRemoveStyleSheet();

        protected:
            // the SytleSheet of this object
            SfxStyleSheet*                                  mpStyleSheet;

            // create a new itemset
            virtual SfxItemSet& CreateObjectSpecificItemSet(SfxItemPool& pPool) SAL_OVERRIDE;

            // Do the ItemChange, may do special handling
            virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = 0) SAL_OVERRIDE;

            // react on ItemSet changes
            virtual void ItemSetChanged(const SfxItemSet& rSet) SAL_OVERRIDE;

        public:
            // basic constructor
            explicit AttributeProperties(SdrObject& rObj);

            // constructor for copying, but using new object
            AttributeProperties(const AttributeProperties& rProps, SdrObject& rObj);

            // Clone() operator, normally just calls the local copy constructor
            virtual BaseProperties& Clone(SdrObject& rObj) const SAL_OVERRIDE;

            // destructor
            virtual ~AttributeProperties();

            // set a new StyleSheet and broadcast
            virtual void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr) SAL_OVERRIDE;

            // get the installed StyleSheet
            virtual SfxStyleSheet* GetStyleSheet() const SAL_OVERRIDE;

            // Move properties to a new ItemPool.
            virtual void MoveToItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel = 0L) SAL_OVERRIDE;

            // Set new model.
            virtual void SetModel(SdrModel* pOldModel, SdrModel* pNewModel) SAL_OVERRIDE;

            // force all attributes which come from styles to hard attributes
            // to be able to live without the style.
            virtual void ForceStyleToHardAttributes() SAL_OVERRIDE;

            // This is the Notify(...) from 2nd base class SfxListener
            virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) SAL_OVERRIDE;

            virtual bool isUsedByModel() const SAL_OVERRIDE;
        };
    } // end of namespace properties
} // end of namespace sdr



#endif // INCLUDED_SVX_SDR_PROPERTIES_ATTRIBUTEPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
