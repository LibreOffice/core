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

#include <svx/sdr/properties/connectorproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svx/svddef.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svdoedge.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        // create a new itemset
        SfxItemSet& ConnectorProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return *(new SfxItemSet(rPool,

                // range from SdrAttrObj
                SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                // range from SdrEdgeObj
                SDRATTR_EDGE_FIRST, SDRATTR_EDGE_LAST,

                // range from SdrTextObj
                EE_ITEMS_START, EE_ITEMS_END,

                // end
                0, 0));
        }

        ConnectorProperties::ConnectorProperties(SdrObject& rObj)
        :   TextProperties(rObj)
        {
        }

        ConnectorProperties::ConnectorProperties(const ConnectorProperties& rProps, SdrObject& rObj)
        :   TextProperties(rProps, rObj)
        {
        }

        ConnectorProperties::~ConnectorProperties()
        {
        }

        BaseProperties& ConnectorProperties::Clone(SdrObject& rObj) const
        {
            return *(new ConnectorProperties(*this, rObj));
        }

        void ConnectorProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            SdrEdgeObj& rObj = (SdrEdgeObj&)GetSdrObject();

            // call parent
            TextProperties::ItemSetChanged(rSet);

            // local changes
            rObj.ImpSetAttrToEdgeInfo();
        }

        void ConnectorProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            SdrEdgeObj& rObj = (SdrEdgeObj&)GetSdrObject();

            // call parent
            TextProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            // local changes
            rObj.ImpSetAttrToEdgeInfo();
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
