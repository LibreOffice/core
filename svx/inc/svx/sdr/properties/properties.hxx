/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SDR_PROPERTIES_PROPERTIES_HXX
#define _SDR_PROPERTIES_PROPERTIES_HXX

#include <sal/types.h>
#include "svx/svxdllapi.h"

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrObject;
class SfxItemSet;
class SfxPoolItem;
class SfxStyleSheet;
class Fraction;
class SfxItemPool;
class SdrModel;

namespace sdr
{
    namespace properties
    {
        class ItemChangeBroadcaster;
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        class SVX_DLLPUBLIC BaseProperties
        {
        protected:
            // the owner of this Properties. Set from constructor and not
            // to be changed in any way.
            SdrObject&                                      mrObject;

            // create a new object specific itemset with object specific ranges.
            virtual SfxItemSet& CreateObjectSpecificItemSet(SfxItemPool& pPool) = 0;

            // internal access to SdrObject
            SdrObject& GetSdrObject() const
            {
                return mrObject;
            }

            // Test changeability for a single item. If a implementation wants to prevent
            // changing an item this method may be overloaded.
            virtual sal_Bool AllowItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = 0) const = 0;

            // Do the internal ItemChange. If only nWhich is given, the item needs to be cleared.
            // Also needs to handle if nWhich and pNewItem is 0, which means to clear all items.
            virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = 0) = 0;

            // Called after ItemChange() is done for all items. Allows local reactions on
            // specific item changes
            virtual void PostItemChange(const sal_uInt16 nWhich) = 0;

            // Internally react on ItemSet changes. The given ItemSet contains all changed items, the new ones.
            virtual void ItemSetChanged(const SfxItemSet& rSet) = 0;

        public:
            // basic constructor, used from SdrObject.
            explicit BaseProperties(SdrObject& rObj);

            // constructor for copying, but using new object. Used from the Clone()
            // method.
            BaseProperties(const BaseProperties& rProps, SdrObject& rObj);

            // destructor
            virtual ~BaseProperties();

            // Clone() operator, normally just calls the local copy constructor,
            // see above.
            virtual BaseProperties& Clone(SdrObject& rObj) const = 0;

            // Get the local ItemSet. This directly returns the local ItemSet of the object. No
            // merging of ItemSets is done for e.g. Group objects.
            virtual const SfxItemSet& GetObjectItemSet() const = 0;

            // get merged ItemSet. Normappl, this maps directly to GetObjectItemSet(), but may
            // be overloaded e.g for group objects to return a merged ItemSet of the object.
            // When using this method the returned ItemSet may contain items in the state
            // SFX_ITEM_DONTCARE which means there were several such items with different
            // values.
            virtual const SfxItemSet& GetMergedItemSet() const;

            // Sets all items which are on state SFX_ITEM_SET in rSet at the local ItemSet.
            // Uses AllowItemChange(), ItemChange(), PostItemChange() and ItemSetChanged() calls.
            virtual void SetObjectItemSet(const SfxItemSet& rSet) = 0;

            // Set merged ItemSet. Normally, this maps to SetObjectItemSet().
            virtual void SetMergedItemSet(const SfxItemSet& rSet, sal_Bool bClearAllItems = sal_False);

            // Set single item at the local ItemSet. Uses AllowItemChange(),
            // ItemChange(), PostItemChange() and ItemSetChanged() calls.
            virtual void SetObjectItem(const SfxPoolItem& rItem) = 0;

            // Set a single item direct. Only uses AllowItemChange() and ItemChange(),
            // but not PostItemChange() and ItemSetChanged() calls.
            virtual void SetObjectItemDirect(const SfxPoolItem& rItem) = 0;

            // Clear a single local item. Uses AllowItemChange(),
            // ItemChange(), PostItemChange() and ItemSetChanged() calls.
            virtual void ClearObjectItem(const sal_uInt16 nWhich = 0) = 0;

            // Set a single item, iterate over hierarchies if necessary. Default
            // Implementation falls back to ClearObjectItem().
            virtual void SetMergedItem(const SfxPoolItem& rItem);

            // Clear a single item, iterate over hierarchies if necessary. Default
            // Implementation falls back to ClearObjectItem().
            virtual void ClearMergedItem(const sal_uInt16 nWhich = 0);

            // Clear single item direct. Only uses AllowItemChange() and ItemChange(),
            // but not PostItemChange() and ItemSetChanged() calls.
            // Also supports complete deletion of items when default parameter 0 is used.
            virtual void ClearObjectItemDirect(const sal_uInt16 nWhich = 0) = 0;

            // Set a new StyleSheet. Registers as listener at the StyleSheet to get knowledge
            // of StyleSheet changes.
            virtual void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr) = 0;

            // Get the installed StyleSheet.
            virtual SfxStyleSheet* GetStyleSheet() const = 0;

            // Scale the local ItemSet as far as it contains metric items. This needs to be
            // overloaded to do it for hierarchical objects like e.g. groups.
            virtual void Scale(const Fraction& rScale);

            // Move local items to a new ItemPool. This needs to be
            // overloaded to do it for hierarchical objects like e.g. groups.
            virtual void MoveToItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel = 0L);

            // Set new model.
            virtual void SetModel(SdrModel* pOldModel, SdrModel* pNewModel);

            // force all attributes which come from styles to hard attributes
            // to be able to live without the style.
            virtual void ForceStyleToHardAttributes();

            // syntactical sugar for ItemSet accesses. Broadcasts before and after the changes
            // to invalidate views in old and new BoundRects. As soon as the repaint mechanism
            // will be changed these broadcasts will no longer be needed.
            //void SetItemAndBroadcast(const SfxPoolItem& rItem);
            //void ClearItemAndBroadcast(const sal_uInt16 nWhich = 0);
            void SetMergedItemSetAndBroadcast(const SfxItemSet& rSet, sal_Bool bClearAllItems = sal_False);

            // Just a convenient shortcut for GetObjectItemSet().Get(nWhich).
            const SfxPoolItem& GetItem(const sal_uInt16 nWhich) const;

            // support for convenient broadcasting. Used from SetMergedItemAndBroadcast(),
            // ClearItemAndBroadcast() and SetItemSetAndBroadcast(), see above.
            // But also from inside SdrObjects.
            void BroadcastItemChange(const ItemChangeBroadcaster& rChange);

            // #i101556# add versioning mechanism; used from e.g. text attribute set to
            // allow detection of e.g. style sheet or single text attribute changes. The
            // default implementation returns 0 (zero)
            virtual sal_uInt32 getVersion() const;
        };

        // checks the FillStyle item and removes unneeded Gradient, FillBitmap and Hatch items
        void SVX_DLLPUBLIC CleanupFillProperties( SfxItemSet& rItemSet );

    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_PROPERTIES_PROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
