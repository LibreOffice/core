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

#include <libxml/xmlwriter.h>

#include <svx/sdr/properties/properties.hxx>
#include <sdr/properties/itemsettools.hxx>
#include <svl/itemset.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svditer.hxx>
#include <svx/xdef.hxx>
#include <svx/xfillit0.hxx>

using namespace com::sun::star;

namespace sdr::properties
{
        BaseProperties::BaseProperties(SdrObject& rObj)
        :   mrObject(rObj)
        {
        }

        BaseProperties::~BaseProperties()
        {
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

        void BaseProperties::SetMergedItemSet(const SfxItemSet& rSet, bool bClearAllItems, bool bAdjustTextFrameWidthAndHeight)
        {
            // clear items if requested
            if(bClearAllItems)
            {
                ClearObjectItem();
            }

            // default implementation falls back to SetObjectItemSet()
            SetObjectItemSet(rSet, bAdjustTextFrameWidthAndHeight);
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
            SdrObject* pObj = &GetSdrObject();
            if (pObj->GetObjIdentifier() == SdrObjKind::Group)
            {
                SdrObjGroup* pObjGroup = static_cast<SdrObjGroup*>(pObj);
                SdrObjListIter aIter(pObjGroup, SdrIterMode::DeepNoGroups);

                while(aIter.IsMore())
                {
                    SdrObject* pChildObj = aIter.Next();
                    pChildObj->BroadcastObjectChange();
                }
            }
            else
            {
                pObj->BroadcastObjectChange();
            }

            // also send the user calls
            for(sal_uInt32 a(0); a < nCount; a++)
            {
                pObj->SendUserCall(SdrUserCallType::ChangeAttr, rChange.GetRectangle(a));
            }
        }

        sal_uInt32 BaseProperties::getVersion() const
        {
            return 0;
        }

        void BaseProperties::dumpAsXml(xmlTextWriterPtr pWriter) const
        {
            (void)xmlTextWriterStartElement(pWriter, BAD_CAST("BaseProperties"));
            (void)xmlTextWriterEndElement(pWriter);
        }

        void CleanupFillProperties( SfxItemSet& rItemSet )
        {
            const bool bFillBitmap = rItemSet.GetItemState(XATTR_FILLBITMAP, false) == SfxItemState::SET;
            const bool bFillGradient = rItemSet.GetItemState(XATTR_FILLGRADIENT, false) == SfxItemState::SET;
            const bool bFillHatch = rItemSet.GetItemState(XATTR_FILLHATCH, false) == SfxItemState::SET;
            if( !(bFillBitmap || bFillGradient || bFillHatch) )
                return;

            const XFillStyleItem* pFillStyleItem = rItemSet.GetItem(XATTR_FILLSTYLE);
            if( !pFillStyleItem )
                return;

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

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
