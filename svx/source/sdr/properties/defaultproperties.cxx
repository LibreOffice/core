/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: defaultproperties.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:36:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SDR_PROPERTIES_DEFAULTPROPERTIES_HXX
#include <svx/sdr/properties/defaultproperties.hxx>
#endif

#ifndef _SDR_PROPERTIES_ITEMSETTOOLS_HXX
#include <svx/sdr/properties/itemsettools.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif

#include <vector>

#ifndef _SVDOBJ_HXX
#include <svdobj.hxx>
#endif

#ifndef _SVDDEF_HXX
#include <svddef.hxx>
#endif

#ifndef _SVDPOOL_HXX
#include <svdpool.hxx>
#endif

#ifndef _EEITEM_HXX
#include <eeitem.hxx>
#endif

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
                mpItemSet = rProps.mpItemSet->Clone(TRUE);

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
            SfxItemSet aSet(*GetSdrObject().GetObjectItemPool(), SDRATTR_START, EE_ITEMS_END, 0, 0);

            // give a hint to STL_Vector
            aPostItemChangeList.reserve(rSet.Count());

            while(nWhich)
            {
                if(SFX_ITEM_SET == rSet.GetItemState(nWhich, FALSE, &pPoolItem))
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
                    aIter++;
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

        void DefaultProperties::PostItemChange(const sal_uInt16 /*nWhich*/)
        {
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
