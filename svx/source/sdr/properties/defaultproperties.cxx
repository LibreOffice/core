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

#include <svx/sdr/properties/defaultproperties.hxx>
#include <sdr/properties/itemsettools.hxx>
#include <svl/itemset.hxx>
#include <svl/whiter.hxx>
#include <vcl/outdev.hxx>

#include <vector>
#include <svx/svdobj.hxx>
#include <svx/svddef.hxx>
#include <svx/svdpool.hxx>
#include <editeng/eeitem.hxx>



namespace sdr
{
    namespace properties
    {
        SfxItemSet* DefaultProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            // Basic implementation; Basic object has NO attributes
            return new SfxItemSet(rPool);
        }

        DefaultProperties::DefaultProperties(SdrObject& rObj)
        :   BaseProperties(rObj),
            mpItemSet(nullptr)
        {
        }

        DefaultProperties::DefaultProperties(const DefaultProperties& rProps, SdrObject& rObj)
        :   BaseProperties(rObj),
            mpItemSet(nullptr)
        {
            if(rProps.mpItemSet)
            {
                mpItemSet = rProps.mpItemSet->Clone();

                // do not keep parent info, this may be changed by later construrtors.
                // This class just copies the ItemSet, ignore parent.
                if(mpItemSet && mpItemSet->GetParent())
                {
                    mpItemSet->SetParent(nullptr);
                }
            }
        }

        BaseProperties& DefaultProperties::Clone(SdrObject& rObj) const
        {
            return *(new DefaultProperties(*this, rObj));
        }

        DefaultProperties::~DefaultProperties()
        {
            if(mpItemSet)
            {
                delete mpItemSet;
                mpItemSet = nullptr;
            }
        }

        const SfxItemSet& DefaultProperties::GetObjectItemSet() const
        {
            if(!mpItemSet)
            {
                const_cast<DefaultProperties*>(this)->mpItemSet = const_cast<DefaultProperties*>(this)->CreateObjectSpecificItemSet(GetSdrObject().GetObjectItemPool());
                const_cast<DefaultProperties*>(this)->ForceDefaultAttributes();
            }

            assert(mpItemSet && "Could not create an SfxItemSet(!)");

            return *mpItemSet;
        }

        void DefaultProperties::SetObjectItem(const SfxPoolItem& rItem)
        {
            const sal_uInt16 nWhichID(rItem.Which());

            if(AllowItemChange(nWhichID, &rItem))
            {
                ItemChange(nWhichID, &rItem);
                PostItemChange(nWhichID);

                SfxItemSet aSet(GetSdrObject().GetObjectItemPool(), nWhichID, nWhichID);
                aSet.Put(rItem);
                ItemSetChanged(aSet);
            }
        }

        void DefaultProperties::SetObjectItemDirect(const SfxPoolItem& rItem)
        {
            const sal_uInt16 nWhichID(rItem.Which());

            if(AllowItemChange(nWhichID, &rItem))
            {
                ItemChange(nWhichID, &rItem);
            }
        }

        void DefaultProperties::ClearObjectItem(const sal_uInt16 nWhich)
        {
            if(AllowItemChange(nWhich))
            {
                ItemChange(nWhich);
                PostItemChange(nWhich);

                if(nWhich)
                {
                    SfxItemSet aSet(GetSdrObject().GetObjectItemPool(), nWhich, nWhich, 0, 0);
                    ItemSetChanged(aSet);
                }
            }
        }

        void DefaultProperties::ClearObjectItemDirect(const sal_uInt16 nWhich)
        {
            if(AllowItemChange(nWhich))
            {
                ItemChange(nWhich);
            }
        }

        void DefaultProperties::SetObjectItemSet(const SfxItemSet& rSet)
        {
            SfxWhichIter aWhichIter(rSet);
            sal_uInt16 nWhich(aWhichIter.FirstWhich());
            const SfxPoolItem *pPoolItem;
            std::vector< sal_uInt16 > aPostItemChangeList;
            bool bDidChange(false);
            SfxItemSet aSet(GetSdrObject().GetObjectItemPool(), SDRATTR_START, EE_ITEMS_END);

            // give a hint to STL_Vector
            aPostItemChangeList.reserve(rSet.Count());

            while(nWhich)
            {
                if(SfxItemState::SET == rSet.GetItemState(nWhich, false, &pPoolItem))
                {
                    if(AllowItemChange(nWhich, pPoolItem))
                    {
                        bDidChange = true;
                        ItemChange(nWhich, pPoolItem);
                        aPostItemChangeList.push_back( nWhich );
                        aSet.Put(*pPoolItem);
                    }
                }

                nWhich = aWhichIter.NextWhich();
            }

            if(bDidChange)
            {
                for (std::vector< sal_uInt16 >::const_iterator aIter(aPostItemChangeList.begin()), aEnd(aPostItemChangeList.end()); aIter != aEnd; ++aIter)
                {
                    PostItemChange(*aIter);
                }

                ItemSetChanged(aSet);
            }
        }

        void DefaultProperties::ItemSetChanged(const SfxItemSet& /*rSet*/)
        {
        }

        bool DefaultProperties::AllowItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/) const
        {
            return true;
        }

        void DefaultProperties::ItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/)
        {
        }

        void DefaultProperties::PostItemChange(const sal_uInt16 nWhich )
        {
            if( (nWhich == XATTR_FILLSTYLE) && (mpItemSet != nullptr) )
                CleanupFillProperties(*mpItemSet);
        }

        void DefaultProperties::SetStyleSheet(SfxStyleSheet* /*pNewStyleSheet*/, bool /*bDontRemoveHardAttr*/)
        {
            // no StyleSheet in DefaultProperties
        }

        SfxStyleSheet* DefaultProperties::GetStyleSheet() const
        {
            // no StyleSheet in DefaultProperties
            return nullptr;
        }

        void DefaultProperties::ForceDefaultAttributes()
        {
        }

        void DefaultProperties::Scale(const Fraction& rScale)
        {
            if(mpItemSet)
            {
                ScaleItemSet(*mpItemSet, rScale);
            }
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
