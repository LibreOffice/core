/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: properties.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-21 16:45:48 $
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

#ifndef _SDR_PROPERTIES_PROPERTIES_HXX
#include <svx/sdr/properties/properties.hxx>
#endif

#ifndef _SDR_PROPERTIES_ITEMSETTOOLS_HXX
#include <svx/sdr/properties/itemsettools.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _SVDOGRP_HXX
#include <svdogrp.hxx>
#endif

#ifndef _SVDITER_HXX
#include <svditer.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        BaseProperties::BaseProperties(SdrObject& rObj)
        :   mrObject(rObj)
        {
        }

        BaseProperties::BaseProperties(const BaseProperties& /*rProps*/, SdrObject& rObj)
        :   mrObject(rObj)
        {
        }

        BaseProperties::~BaseProperties()
        {
        }

        const SfxItemSet& BaseProperties::GetMergedItemSet() const
        {
            // default implementation falls back to GetObjectItemSet()
            return GetObjectItemSet();
        }

        void BaseProperties::SetMergedItemSet(const SfxItemSet& rSet, sal_Bool bClearAllItems)
        {
            // clear items if requested
            if(bClearAllItems)
            {
                ClearObjectItem();
            }

            // default implementation falls back to SetObjectItemSet()
            SetObjectItemSet(rSet);
        }

        void BaseProperties::SetMergedItem(const SfxPoolItem& rItem)
        {
            // default implementation falls back to SetObjectItem()
            SetObjectItem(rItem);
        }

        void BaseProperties::ClearMergedItem(const sal_uInt16 nWhich)
        {
            // default implementation falls back to ClearObjectItem()
            ClearObjectItem(nWhich);
        }

        void BaseProperties::Scale(const Fraction& /*rScale*/)
        {
            // default implementation does nothing; overload where
            // an ItemSet is implemented.
        }

        void BaseProperties::MoveToItemPool(SfxItemPool* /*pSrcPool*/, SfxItemPool* /*pDestPool*/, SdrModel* /*pNewModel*/)
        {
            // Move properties to a new ItemPool. Default implementation does nothing.
            // Overload where an ItemSet is implemented.
        }

        void BaseProperties::SetModel(SdrModel* /*pOldModel*/, SdrModel* /*pNewModel*/)
        {
            // Set new model. Default implementation does nothing.
            // Overload where an ItemSet is implemented.
        }

        void BaseProperties::ForceStyleToHardAttributes()
        {
            // force all attributes which come from styles to hard attributes
            // to be able to live without the style. Default implementation does nothing.
            // Overload where an ItemSet is implemented.
        }

        //void BaseProperties::SetItemAndBroadcast(const SfxPoolItem& rItem)
        //{
        //  ItemChangeBroadcaster aC(GetSdrObject());
        //  SetObjectItem(rItem);
        //  BroadcastItemChange(aC);
        //}

        //void BaseProperties::ClearItemAndBroadcast(const sal_uInt16 nWhich)
        //{
        //  ItemChangeBroadcaster aC(GetSdrObject());
        //  ClearObjectItem(nWhich);
        //  BroadcastItemChange(aC);
        //}

        void BaseProperties::SetMergedItemSetAndBroadcast(const SfxItemSet& rSet, sal_Bool bClearAllItems)
        {
            ItemChangeBroadcaster aC(GetSdrObject());

            if(bClearAllItems)
            {
                ClearObjectItem();
            }

            SetMergedItemSet(rSet);
            BroadcastItemChange(aC);
        }

        const SfxPoolItem& BaseProperties::GetItem(const sal_uInt16 nWhich) const
        {
            return GetObjectItemSet().Get(nWhich);
        }

        void BaseProperties::BroadcastItemChange(const ItemChangeBroadcaster& rChange)
        {
            const sal_uInt32 nCount(rChange.GetRectangleCount());

            // #110094#-14 Reduce to do only second change
            //// invalidate all remembered rectangles
            //for(sal_uInt32 a(0); a < nCount; a++)
            //{
            //  GetSdrObject().BroadcastObjectChange(rChange.GetRectangle(a));
            //}

            // invalidate all new rectangles
            if(GetSdrObject().ISA(SdrObjGroup))
            {
                SdrObjListIter aIter((SdrObjGroup&)GetSdrObject(), IM_DEEPNOGROUPS);

                while(aIter.IsMore())
                {
                    SdrObject* pObj = aIter.Next();
                    // This is done with ItemSetChanged
                    // pObj->SetChanged();
                    pObj->BroadcastObjectChange();
                }
            }
            else
            {
                // This is done with ItemSetChanged
                // GetSdrObject().SetChanged();
                GetSdrObject().BroadcastObjectChange();
            }

            // also send the user calls
            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                GetSdrObject().SendUserCall(SDRUSERCALL_CHGATTR, rChange.GetRectangle(a));
            }
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
