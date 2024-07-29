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

#include <sdr/properties/circleproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svx/svddef.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svdocirc.hxx>
#include <sxcikitm.hxx>
#include <svx/sxciaitm.hxx>


namespace sdr::properties
{
        // create a new itemset
        SfxItemSet CircleProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return SfxItemSet(
                rPool,
                svl::Items<
                    // Ranges from SdrAttrObj, SdrCircObj
                    SDRATTR_START, SDRATTR_SHADOW_LAST,
                    SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                    SDRATTR_CIRC_FIRST, SDRATTR_CIRC_LAST,
                    SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,
                    SDRATTR_TEXTCOLUMNS_FIRST, SDRATTR_TEXTCOLUMNS_LAST,
                    // Range from SdrTextObj:
                    EE_ITEMS_START, EE_ITEMS_END>);
        }

        CircleProperties::CircleProperties(SdrObject& rObj)
        :   RectangleProperties(rObj)
        {
        }

        CircleProperties::CircleProperties(const CircleProperties& rProps, SdrObject& rObj)
        :   RectangleProperties(rProps, rObj)
        {
        }

        CircleProperties::~CircleProperties()
        {
        }

        std::unique_ptr<BaseProperties> CircleProperties::Clone(SdrObject& rObj) const
        {
            return std::unique_ptr<BaseProperties>(new CircleProperties(*this, rObj));
        }

        void CircleProperties::ItemSetChanged(std::span< const SfxPoolItem* const > aChangedItems, sal_uInt16 nDeletedWhich)
        {
            SdrCircObj& rObj = static_cast<SdrCircObj&>(GetSdrObject());

            // call parent
            RectangleProperties::ItemSetChanged(aChangedItems, nDeletedWhich);

            // local changes
            rObj.ImpSetAttrToCircInfo();
        }

        void CircleProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr,
                bool bBroadcast, bool bAdjustTextFrameWidthAndHeight)
        {
            // call parent (always first thing to do, may create the SfxItemSet)
            RectangleProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr, bBroadcast, bAdjustTextFrameWidthAndHeight);

            // local changes
            SdrCircObj& rObj = static_cast<SdrCircObj&>(GetSdrObject());
            rObj.SetXPolyDirty();

            // local changes
            rObj.ImpSetAttrToCircInfo();
        }

        void CircleProperties::ForceDefaultAttributes()
        {
            SdrCircObj& rObj = static_cast<SdrCircObj&>(GetSdrObject());
            SdrCircKind eKind = rObj.GetCircleKind();

            if(eKind != SdrCircKind::Full)
            {
                moItemSet->Put(SdrCircKindItem(eKind));

                if(rObj.GetStartAngle())
                {
                    moItemSet->Put(makeSdrCircStartAngleItem(rObj.GetStartAngle()));
                }

                if(rObj.GetEndAngle() != 36000_deg100)
                {
                    moItemSet->Put(makeSdrCircEndAngleItem(rObj.GetEndAngle()));
                }
            }

            // call parent after SetObjectItem(SdrCircKindItem())
            // because ForceDefaultAttr() will call
            // ImpSetAttrToCircInfo() which needs a correct
            // SdrCircKindItem
            RectangleProperties::ForceDefaultAttributes();
        }
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
