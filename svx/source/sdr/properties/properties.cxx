/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdr/properties/properties.hxx>
#include <svx/sdr/properties/itemsettools.hxx>
#include <svl/itemset.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svditer.hxx>
#include <svx/xfillit0.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        BaseProperties::BaseProperties(SdrObject& rObj)
        :   boost::noncopyable(),
            mrObject(rObj)
        {
        }

        BaseProperties::BaseProperties(const BaseProperties& /*rProps*/, SdrObject& rObj)
        :   boost::noncopyable(),
            mrObject(rObj)
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

        void BaseProperties::SetMergedItemSet(const SfxItemSet& rSet, bool bClearAllItems)
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

        void BaseProperties::ForceStyleToHardAttributes()
        {
            // force all attributes which come from styles to hard attributes
            // to be able to live without the style. Default implementation does nothing.
            // Overload where an ItemSet is implemented.
        }

        void BaseProperties::SetMergedItemSetAndBroadcast(const SfxItemSet& rSet, bool bClearAllItems)
        {
            ItemChangeBroadcaster aC(GetSdrObject());

            if(bClearAllItems)
            {
                ClearObjectItem();
            }

            SetMergedItemSet(rSet);
        }

        const SfxPoolItem& BaseProperties::GetItem(const sal_uInt16 nWhich) const
        {
            return GetObjectItemSet().Get(nWhich);
        }

        sal_uInt32 BaseProperties::getVersion() const
        {
            return 0;
        }

        void CleanupFillProperties( SfxItemSet& rItemSet )
        {
            const bool bFillBitmap = rItemSet.GetItemState(XATTR_FILLBITMAP, sal_False) == SFX_ITEM_SET;
            const bool bFillGradient = rItemSet.GetItemState(XATTR_FILLGRADIENT, sal_False) == SFX_ITEM_SET;
            const bool bFillHatch = rItemSet.GetItemState(XATTR_FILLHATCH, sal_False) == SFX_ITEM_SET;
            if( bFillBitmap || bFillGradient || bFillHatch )
            {
                const XFillStyleItem* pFillStyleItem = dynamic_cast< const XFillStyleItem* >( rItemSet.GetItem(XATTR_FILLSTYLE) );
                if( pFillStyleItem )
                {
                    if( bFillBitmap && (pFillStyleItem->GetValue() != XFILL_BITMAP) )
                    {
                        rItemSet.ClearItem( XATTR_FILLBITMAP );
                    }

                    if( bFillGradient && (pFillStyleItem->GetValue() != XFILL_GRADIENT) )
                    {
                        rItemSet.ClearItem( XATTR_FILLGRADIENT );
                    }

                    if( bFillHatch && (pFillStyleItem->GetValue() != XFILL_HATCH) )
                    {
                        rItemSet.ClearItem( XATTR_FILLHATCH );
                    }
                }
            }
        }

    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
