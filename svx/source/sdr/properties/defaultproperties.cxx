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

#include <svx/sdr/properties/defaultproperties.hxx>
#include <svx/sdr/properties/itemsettools.hxx>
#include <svl/itemset.hxx>
#include <svl/whiter.hxx>

#include <vector>
#include <svx/svdobj.hxx>
#include <svx/svddef.hxx>
#include <svx/svdpool.hxx>
#include <editeng/eeitem.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        SfxItemSet& DefaultProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            // Basic implementation; Basic object has NO attributes
            return *(new SfxItemSet(rPool));
        }

        DefaultProperties::DefaultProperties(SdrObject& rObj)
        :   BaseProperties(rObj),
            mpItemSet(0L)
        {
        }

        DefaultProperties::DefaultProperties(const DefaultProperties& rProps, SdrObject& rObj)
        :   BaseProperties(rObj),
            mpItemSet(0L)
        {
            if(rProps.mpItemSet)
            {
                mpItemSet = rProps.mpItemSet->Clone(sal_True);

                // do not keep parent info, this may be changed by later construrtors.
                // This class just copies the ItemSet, ignore parent.
                if(mpItemSet && mpItemSet->GetParent())
                {
                    mpItemSet->SetParent(0L);
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
                mpItemSet = 0L;
            }
        }

        const SfxItemSet& DefaultProperties::GetObjectItemSet() const
        {
            if(!mpItemSet)
            {
                ((DefaultProperties*)this)->mpItemSet = &(((DefaultProperties*)this)->CreateObjectSpecificItemSet(*GetSdrObject().GetObjectItemPool()));
                ((DefaultProperties*)this)->ForceDefaultAttributes();
            }

            DBG_ASSERT(mpItemSet, "Could not create an SfxItemSet(!)");

            return *mpItemSet;
        }

        void DefaultProperties::SetObjectItem(const SfxPoolItem& rItem)
        {
            const sal_uInt16 nWhichID(rItem.Which());

            if(AllowItemChange(nWhichID, &rItem))
            {
                ItemChange(nWhichID, &rItem);
                PostItemChange(nWhichID);

                SfxItemSet aSet(*GetSdrObject().GetObjectItemPool(), nWhichID, nWhichID);
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
                    SfxItemSet aSet(*GetSdrObject().GetObjectItemPool(), nWhich, nWhich, 0, 0);
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
            sal_Bool bDidChange(sal_False);
            SfxItemSet aSet(*GetSdrObject().GetObjectItemPool(), SDRATTR_START, EE_ITEMS_END);

            // give a hint to STL_Vector
            aPostItemChangeList.reserve(rSet.Count());

            while(nWhich)
            {
                if(SFX_ITEM_SET == rSet.GetItemState(nWhich, sal_False, &pPoolItem))
                {
                    if(AllowItemChange(nWhich, pPoolItem))
                    {
                        bDidChange = sal_True;
                        ItemChange(nWhich, pPoolItem);
                        aPostItemChangeList.push_back( nWhich );
                        aSet.Put(*pPoolItem);
                    }
                }

                nWhich = aWhichIter.NextWhich();
            }

            if(bDidChange)
            {
                std::vector< sal_uInt16 >::iterator aIter = aPostItemChangeList.begin();
                const std::vector< sal_uInt16 >::iterator aEnd = aPostItemChangeList.end();

                while(aIter != aEnd)
                {
                    PostItemChange(*aIter);
                    ++aIter;
                }

                ItemSetChanged(aSet);
            }
        }

        void DefaultProperties::ItemSetChanged(const SfxItemSet& /*rSet*/)
        {
        }

        sal_Bool DefaultProperties::AllowItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/) const
        {
            return sal_True;
        }

        void DefaultProperties::ItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/)
        {
        }

        void DefaultProperties::PostItemChange(const sal_uInt16 nWhich )
        {
            if( (nWhich == XATTR_FILLSTYLE) && (mpItemSet != NULL) )
                CleanupFillProperties(*mpItemSet);
        }

        void DefaultProperties::SetStyleSheet(SfxStyleSheet* /*pNewStyleSheet*/, sal_Bool /*bDontRemoveHardAttr*/)
        {
            // no StyleSheet in DefaultProperties
        }

        SfxStyleSheet* DefaultProperties::GetStyleSheet() const
        {
            // no StyleSheet in DefaultProperties
            return 0L;
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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
