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

#include <sdr/properties/rectangleproperties.hxx>
#include <svx/svdorect.hxx>


namespace sdr::properties
{
        RectangleProperties::RectangleProperties(SdrObject& rObj)
        :   TextProperties(rObj)
        {
        }

        RectangleProperties::RectangleProperties(const RectangleProperties& rProps, SdrObject& rObj)
        :   TextProperties(rProps, rObj)
        {
        }

        RectangleProperties::~RectangleProperties()
        {
        }

        std::unique_ptr<BaseProperties> RectangleProperties::Clone(SdrObject& rObj) const
        {
            return std::unique_ptr<BaseProperties>(new RectangleProperties(*this, rObj));
        }

        void RectangleProperties::ItemSetChanged(std::span< const SfxPoolItem* const > aChangedItems, sal_uInt16 nDeletedWhich, bool bAdjustTextFrameWidthAndHeight)
        {
            SdrRectObj& rObj = static_cast<SdrRectObj&>(GetSdrObject());

            // call parent
            TextProperties::ItemSetChanged(aChangedItems, nDeletedWhich, bAdjustTextFrameWidthAndHeight);

            // local changes
            rObj.SetXPolyDirty();
        }

        // set a new StyleSheet and broadcast
        void RectangleProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr,
                bool bBroadcast, bool bAdjustTextFrameWidthAndHeight)
        {
            // call parent (always first thing to do, may create the SfxItemSet)
            TextProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr, bBroadcast, bAdjustTextFrameWidthAndHeight);

            // local changes
            SdrRectObj& rObj = static_cast<SdrRectObj&>(GetSdrObject());
            rObj.SetXPolyDirty();
        }
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
