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

#include <sdr/properties/connectorproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svx/svddef.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svdoedge.hxx>


namespace sdr
{
    namespace properties
    {
        // create a new itemset
        std::unique_ptr<SfxItemSet> ConnectorProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return std::make_unique<SfxItemSet>(
                rPool,
                svl::Items<
                    // Ranges from SdrAttrObj, SdrEdgeObj:
                    SDRATTR_START, SDRATTR_SHADOW_LAST,
                    SDRATTR_MISC_FIRST, SDRATTR_EDGE_LAST,
                    SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,
                    // Range from SdrTextObj:
                    EE_ITEMS_START, EE_ITEMS_END>{});
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

        std::unique_ptr<BaseProperties> ConnectorProperties::Clone(SdrObject& rObj) const
        {
            return std::unique_ptr<BaseProperties>(new ConnectorProperties(*this, rObj));
        }

        void ConnectorProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            SdrEdgeObj& rObj = static_cast<SdrEdgeObj&>(GetSdrObject());

            // call parent
            TextProperties::ItemSetChanged(rSet);

            // local changes
            rObj.ImpSetAttrToEdgeInfo();
        }

        void ConnectorProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
        {
            // call parent (always first thing to do, may create the SfxItemSet)
            TextProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            // local changes
            SdrEdgeObj& rObj = static_cast<SdrEdgeObj&>(GetSdrObject());
            rObj.ImpSetAttrToEdgeInfo();
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
