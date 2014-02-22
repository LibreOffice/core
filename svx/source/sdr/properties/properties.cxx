/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svx/sdr/properties/properties.hxx>
#include <svx/sdr/properties/itemsettools.hxx>
#include <svl/itemset.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svditer.hxx>
#include <svx/xfillit0.hxx>



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
            
            return GetObjectItemSet();
        }

        void BaseProperties::SetMergedItemSet(const SfxItemSet& rSet, sal_Bool bClearAllItems)
        {
            
            if(bClearAllItems)
            {
                ClearObjectItem();
            }

            
            SetObjectItemSet(rSet);
        }

        void BaseProperties::SetMergedItem(const SfxPoolItem& rItem)
        {
            
            SetObjectItem(rItem);
        }

        void BaseProperties::ClearMergedItem(const sal_uInt16 nWhich)
        {
            
            ClearObjectItem(nWhich);
        }

        void BaseProperties::Scale(const Fraction& /*rScale*/)
        {
            
            
        }

        void BaseProperties::MoveToItemPool(SfxItemPool* /*pSrcPool*/, SfxItemPool* /*pDestPool*/, SdrModel* /*pNewModel*/)
        {
            
            
        }

        void BaseProperties::SetModel(SdrModel* /*pOldModel*/, SdrModel* /*pNewModel*/)
        {
            
            
        }

        void BaseProperties::ForceStyleToHardAttributes()
        {
            
            
            
        }

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

            
            //
            
            
            
            

            
            if(GetSdrObject().ISA(SdrObjGroup))
            {
                SdrObjListIter aIter((SdrObjGroup&)GetSdrObject(), IM_DEEPNOGROUPS);

                while(aIter.IsMore())
                {
                    SdrObject* pObj = aIter.Next();
                    
                    
                    pObj->BroadcastObjectChange();
                }
            }
            else
            {
                
                
                GetSdrObject().BroadcastObjectChange();
            }

            
            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                GetSdrObject().SendUserCall(SDRUSERCALL_CHGATTR, rChange.GetRectangle(a));
            }
        }

        sal_uInt32 BaseProperties::getVersion() const
        {
            return 0;
        }

        void CleanupFillProperties( SfxItemSet& rItemSet )
        {
            const bool bFillBitmap = rItemSet.GetItemState(XATTR_FILLBITMAP, false) == SFX_ITEM_SET;
            const bool bFillGradient = rItemSet.GetItemState(XATTR_FILLGRADIENT, false) == SFX_ITEM_SET;
            const bool bFillHatch = rItemSet.GetItemState(XATTR_FILLHATCH, false) == SFX_ITEM_SET;
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

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
