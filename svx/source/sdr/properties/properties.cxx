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

#include <svx/sdr/properties/properties.hxx>
#include <sdr/properties/itemsettools.hxx>
#include <svl/itemset.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svditer.hxx>
#include <svx/xfillit0.hxx>
#include <vcl/outdev.hxx>
#include <svx/svdmodel.hxx>

using namespace com::sun::star;

namespace sdr
{
    namespace properties
    {
        BaseProperties::BaseProperties(SdrObject& rObj)
        :   mrObject(rObj)
        {
        }

        BaseProperties::~BaseProperties()
        {
        }

        void BaseProperties::applyDefaultStyleSheetFromSdrModel()
        {
            SfxStyleSheet* pDefaultStyleSheet(GetSdrObject().getSdrModelFromSdrObject().GetDefaultStyleSheet());

            // tdf#118139 Only do this when StyleSheet really differs. It may e.g.
            // be the case that nullptr == pDefaultStyleSheet and there is none set yet,
            // so indeed no need to set it (needed for some strange old MSWord2003
            // documents with CustomShape-'Group' and added Text-Frames, see task description)
            if(pDefaultStyleSheet != GetStyleSheet())
            {
                // do not delete hard attributes when setting dsefault Style
                SetStyleSheet(pDefaultStyleSheet, true);
            }
        }

        const SdrObject& BaseProperties::GetSdrObject() const
        {
            return mrObject;
        }

        SdrObject& BaseProperties::GetSdrObject()
        {
            return mrObject;
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

        void BaseProperties::ForceStyleToHardAttributes()
        {
            // force all attributes which come from styles to hard attributes
            // to be able to live without the style. Default implementation does nothing.
            // Override where an ItemSet is implemented.
        }

        void BaseProperties::SetMergedItemSetAndBroadcast(const SfxItemSet& rSet, bool bClearAllItems)
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

            // invalidate all new rectangles
            if(dynamic_cast<const SdrObjGroup*>( &GetSdrObject() ) != nullptr)
            {
                SdrObjListIter aIter(static_cast<SdrObjGroup&>(GetSdrObject()), SdrIterMode::DeepNoGroups);

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

            // also send the user calls
            for(sal_uInt32 a(0); a < nCount; a++)
            {
                GetSdrObject().SendUserCall(SdrUserCallType::ChangeAttr, rChange.GetRectangle(a));
            }
        }

        sal_uInt32 BaseProperties::getVersion() const
        {
            return 0;
        }

        void CleanupFillProperties( SfxItemSet& rItemSet )
        {
            const bool bFillBitmap = rItemSet.GetItemState(XATTR_FILLBITMAP, false) == SfxItemState::SET;
            const bool bFillGradient = rItemSet.GetItemState(XATTR_FILLGRADIENT, false) == SfxItemState::SET;
            const bool bFillHatch = rItemSet.GetItemState(XATTR_FILLHATCH, false) == SfxItemState::SET;
            if( bFillBitmap || bFillGradient || bFillHatch )
            {
                const XFillStyleItem* pFillStyleItem = rItemSet.GetItem(XATTR_FILLSTYLE);
                if( pFillStyleItem )
                {
                    if( bFillBitmap && (pFillStyleItem->GetValue() != drawing::FillStyle_BITMAP) )
                    {
                        rItemSet.ClearItem( XATTR_FILLBITMAP );
                    }

                    if( bFillGradient && (pFillStyleItem->GetValue() != drawing::FillStyle_GRADIENT) )
                    {
                        rItemSet.ClearItem( XATTR_FILLGRADIENT );
                    }

                    if( bFillHatch && (pFillStyleItem->GetValue() != drawing::FillStyle_HATCH) )
                    {
                        rItemSet.ClearItem( XATTR_FILLHATCH );
                    }
                }
            }
        }

    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
