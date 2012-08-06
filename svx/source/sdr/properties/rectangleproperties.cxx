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

#include <svx/sdr/properties/rectangleproperties.hxx>
#include <svx/svdorect.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
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

        BaseProperties& RectangleProperties::Clone(SdrObject& rObj) const
        {
            return *(new RectangleProperties(*this, rObj));
        }

        void RectangleProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            SdrRectObj& rObj = (SdrRectObj&)GetSdrObject();

            // call parent
            TextProperties::ItemSetChanged(rSet);

            // local changes
            rObj.SetXPolyDirty();
        }

        // set a new StyleSheet and broadcast
        void RectangleProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            SdrRectObj& rObj = (SdrRectObj&)GetSdrObject();

            // call parent
            TextProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            // local changes
            rObj.SetXPolyDirty();
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
