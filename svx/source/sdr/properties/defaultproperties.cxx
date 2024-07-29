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

#include <sal/config.h>

#include <svx/sdr/properties/defaultproperties.hxx>
#include <sdr/properties/itemsettools.hxx>
#include <svl/itemset.hxx>
#include <svl/whiter.hxx>
#include <vector>
#include <svx/svdobj.hxx>
#include <libxml/xmlwriter.h>
#include <svx/svdmodel.hxx>
#include <svx/svdtrans.hxx>
#include <svx/xbtmpit.hxx>

namespace sdr::properties
{
        SfxItemSet DefaultProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            // Basic implementation; Basic object has NO attributes
            return SfxItemSet(rPool);
        }

        DefaultProperties::DefaultProperties(SdrObject& rObj)
        :   BaseProperties(rObj)
        {
        }

        DefaultProperties::DefaultProperties(const DefaultProperties& rProps, SdrObject& rObj)
        :   BaseProperties(rObj)
        {
            if(!rProps.moItemSet)
                return;

            // Clone may be to another model and thus another ItemPool.
            // SfxItemSet supports that thus we are able to Clone all
            // SfxItemState::SET items to the target pool.
            moItemSet.emplace(rProps.moItemSet->CloneAsValue(
                true,
                &rObj.getSdrModelFromSdrObject().GetItemPool()));

            // React on ModelChange: If metric has changed, scale items.
            // As seen above, clone is supported, but scale is not included,
            // thus: TTTT maybe add scale to SfxItemSet::Clone() (?)
            // tdf#117707 correct ModelChange detection
            const bool bModelChange(&rObj.getSdrModelFromSdrObject() != &rProps.GetSdrObject().getSdrModelFromSdrObject());

            if(bModelChange)
            {
                const MapUnit aOldUnit(rProps.GetSdrObject().getSdrModelFromSdrObject().GetScaleUnit());
                const MapUnit aNewUnit(rObj.getSdrModelFromSdrObject().GetScaleUnit());
                const bool bScaleUnitChanged(aNewUnit != aOldUnit);

                if(bScaleUnitChanged)
                {
                    const Fraction aMetricFactor(GetMapFactor(aOldUnit, aNewUnit).X());

                    ScaleItemSet(*moItemSet, aMetricFactor);
                }
            }

            // do not keep parent info, this may be changed by later constructors.
            // This class just copies the ItemSet, ignore parent.
            if(moItemSet && moItemSet->GetParent())
            {
                moItemSet->SetParent(nullptr);
            }
        }

        std::unique_ptr<BaseProperties> DefaultProperties::Clone(SdrObject& rObj) const
        {
            return std::unique_ptr<BaseProperties>(new DefaultProperties(*this, rObj));
        }

        DefaultProperties::~DefaultProperties() {}

        const SfxItemSet& DefaultProperties::GetObjectItemSet() const
        {
            if(!moItemSet)
            {
                moItemSet.emplace(const_cast<DefaultProperties*>(this)->CreateObjectSpecificItemSet(GetSdrObject().GetObjectItemPool()));
                const_cast<DefaultProperties*>(this)->ForceDefaultAttributes();
            }

            assert(moItemSet && "Could not create an SfxItemSet(!)");

            return *moItemSet;
        }

        void DefaultProperties::SetObjectItem(const SfxPoolItem& rItem)
        {
            const sal_uInt16 nWhichID(rItem.Which());

            if(!AllowItemChange(nWhichID, &rItem))
                return;

            ItemChange(nWhichID, &rItem);
            PostItemChange(nWhichID);

            const SfxPoolItem* pItem = &rItem;
            ItemSetChanged( {&pItem, 1}, 0);
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
            if(!AllowItemChange(nWhich))
                return;

            ItemChange(nWhich);
            PostItemChange(nWhich);

            if(nWhich)
            {
                ItemSetChanged({}, nWhich);
            }
        }

        void DefaultProperties::ClearObjectItemDirect(const sal_uInt16 nWhich)
        {
            if(AllowItemChange(nWhich))
            {
                ItemChange(nWhich);
            }
        }

        void DefaultProperties::SetObjectItemSet(const SfxItemSet& rSet, bool bAdjustTextFrameWidthAndHeight)
        {
            if (rSet.HasItem(XATTR_FILLBITMAP))
            {
                const XFillBitmapItem* pItem = rSet.GetItem(XATTR_FILLBITMAP);
                const std::shared_ptr<VectorGraphicData>& pVectorData
                    = pItem->GetGraphicObject().GetGraphic().getVectorGraphicData();
                if (pVectorData)
                {
                    // Shape is filled by a vector graphic: tell it our size as a hint.
                    basegfx::B2DTuple aSizeHint;
                    aSizeHint.setX(GetSdrObject().GetSnapRect().getOpenWidth());
                    aSizeHint.setY(GetSdrObject().GetSnapRect().getOpenHeight());
                    pVectorData->setSizeHint(aSizeHint);
                }
            }

            SfxWhichIter aWhichIter(rSet);
            sal_uInt16 nWhich(aWhichIter.FirstWhich());
            std::vector< const SfxPoolItem * > aPostItemChangeList;
            // give a hint to STL_Vector
            aPostItemChangeList.reserve(rSet.Count());

            while(nWhich)
            {
                const SfxPoolItem* pPoolItem;
                if(SfxItemState::SET == aWhichIter.GetItemState(false, &pPoolItem))
                {
                    if(AllowItemChange(nWhich, pPoolItem))
                    {
                        ItemChange(nWhich, pPoolItem);
                        aPostItemChangeList.emplace_back( pPoolItem );
                    }
                }

                nWhich = aWhichIter.NextWhich();
            }

            if(!aPostItemChangeList.empty())
            {
                for (const auto& rItem : aPostItemChangeList)
                {
                    PostItemChange(rItem->Which());
                }

                ItemSetChanged(aPostItemChangeList, 0, bAdjustTextFrameWidthAndHeight);
            }
        }

        void DefaultProperties::ItemSetChanged(std::span< const SfxPoolItem* const > /*aChangedItems*/, sal_uInt16 /*nDeletedWhich*/, bool /*bAdjustTextFrameWidthAndHeight*/)
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
            if( (nWhich == XATTR_FILLSTYLE) && moItemSet )
                CleanupFillProperties(*moItemSet);
        }

        void DefaultProperties::SetStyleSheet(SfxStyleSheet* /*pNewStyleSheet*/, bool /*bDontRemoveHardAttr*/,
                bool /*bBroadcast*/, bool /*bAdjustTextFrameWidthAndHeight*/)
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

        void DefaultProperties::dumpAsXml(xmlTextWriterPtr pWriter) const
        {
            (void)xmlTextWriterStartElement(pWriter, BAD_CAST("DefaultProperties"));
            BaseProperties::dumpAsXml(pWriter);
            if (moItemSet)
            {
                moItemSet->dumpAsXml(pWriter);
            }
            (void)xmlTextWriterEndElement(pWriter);
        }
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
