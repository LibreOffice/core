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

#ifndef _SDR_PROPERTIES_E3DSCENEPROPERTIES_HXX
#define _SDR_PROPERTIES_E3DSCENEPROPERTIES_HXX

#include <svx/sdr/properties/e3dproperties.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        class E3dSceneProperties : public E3dProperties
        {
        protected:
            // Called after ItemChange() is done for all items.
            virtual void PostItemChange(const sal_uInt16 nWhich);

        public:
            // basic constructor
            explicit E3dSceneProperties(SdrObject& rObj);

            // constructor for copying, but using new object
            E3dSceneProperties(const E3dSceneProperties& rProps, SdrObject& rObj);

            // destructor
            virtual ~E3dSceneProperties();

            // Clone() operator, normally just calls the local copy constructor
            virtual BaseProperties& Clone(SdrObject& rObj) const;

            // get itemset
            virtual const SfxItemSet& GetObjectItemSet() const;

            // get merged ItemSet. Normappl, this maps directly to GetObjectItemSet(), but may
            // be overloaded e.g for group objects to return a merged ItemSet of the object.
            // When using this method the returned ItemSet may contain items in the state
            // SFX_ITEM_DONTCARE which means there were several such items with different
            // values.
            virtual const SfxItemSet& GetMergedItemSet() const;

            // Set merged ItemSet. Normally, this maps to SetObjectItemSet().
            virtual void SetMergedItemSet(const SfxItemSet& rSet, sal_Bool bClearAllItems = sal_False);

            // Set a single item, iterate over hierarchies if necessary.
            virtual void SetMergedItem(const SfxPoolItem& rItem);

            // Clear a single item, iterate over hierarchies if necessary.
            virtual void ClearMergedItem(const sal_uInt16 nWhich = 0);

            // set a new StyleSheet and broadcast
            virtual void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr);

            // get the installed StyleSheet
            virtual SfxStyleSheet* GetStyleSheet() const;

            // Move properties to a new ItemPool. Default implementation does nothing.
            virtual void MoveToItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel = 0L);

            // Special for scene:
            void SetSceneItemsFromCamera();
        };
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif // _SDR_PROPERTIES_E3DSCENEPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
