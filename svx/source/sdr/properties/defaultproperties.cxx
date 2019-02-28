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
#include <vcl/outdev.hxx>
#include <vector>
#include <svx/svdobj.hxx>
#include <svx/svddef.hxx>
#include <svx/svdpool.hxx>
#include <editeng/eeitem.hxx>
#include <libxml/xmlwriter.h>
#include <svx/svdmodel.hxx>
#include <svx/svdtrans.hxx>

namespace sdr
{
    namespace properties
    {
        std::unique_ptr<SfxItemSet> DefaultProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            // Basic implementation; Basic object has NO attributes
            return std::make_unique<SfxItemSet>(rPool);
        }

        DefaultProperties::DefaultProperties(SdrObject& rObj)
        :   BaseProperties(rObj)
        {
        }

        DefaultProperties::DefaultProperties(const DefaultProperties& rProps, SdrObject& rObj)
        :   BaseProperties(rObj)
        {
            if(rProps.mpItemSet)
            {
                // Clone may be to another model and thus another ItemPool.
                // SfxItemSet supports that thus we are able to Clone all
                // SfxItemState::SET items to the target pool.
                mpItemSet = rProps.mpItemSet->Clone(
                    true,
                    &rObj.getSdrModelFromSdrObject().GetItemPool());

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

                        ScaleItemSet(*mpItemSet, aMetricFactor);
                    }
                }

                // do not keep parent info, this may be changed by later constructors.
                // This class just copies the ItemSet, ignore parent.
                if(mpItemSet && mpItemSet->GetParent())
                {
                    mpItemSet->SetParent(nullptr);
                }
            }
        }

        std::unique_ptr<BaseProperties> DefaultProperties::Clone(SdrObject& rObj) const
        {
            return std::unique_ptr<BaseProperties>(new DefaultProperties(*this, rObj));
        }

        DefaultProperties::~DefaultProperties() {}

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

                SfxItemSet aSet(GetSdrObject().GetObjectItemPool(), {{nWhichID, nWhichID}});
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
                    SfxItemSet aSet(GetSdrObject().GetObjectItemPool(), {{nWhich, nWhich}});
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
            SfxItemSet aSet(GetSdrObject().GetObjectItemPool(), svl::Items<SDRATTR_START, EE_ITEMS_END>{});

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
                for (const auto& rItem : aPostItemChangeList)
                {
                    PostItemChange(rItem);
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

        void DefaultProperties::dumpAsXml(xmlTextWriterPtr pWriter) const
        {
            xmlTextWriterStartElement(pWriter, BAD_CAST("DefaultProperties"));
            mpItemSet->dumpAsXml(pWriter);
            xmlTextWriterEndElement(pWriter);
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
