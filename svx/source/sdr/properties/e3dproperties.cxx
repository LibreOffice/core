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

#include <sdr/properties/e3dproperties.hxx>
#include <svl/itemset.hxx>
#include <svx/svddef.hxx>
#include <svx/obj3d.hxx>


namespace sdr
{
    namespace properties
    {
        // create a new itemset
        std::unique_ptr<SfxItemSet> E3dProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return std::make_unique<SfxItemSet>(rPool,

                // ranges from SdrAttrObj
                svl::Items<SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                // ranges from E3dObject, contains object and scene because of GetMergedItemSet()
                SDRATTR_3D_FIRST, SDRATTR_3D_LAST>{});
        }

        E3dProperties::E3dProperties(SdrObject& rObj)
        :   AttributeProperties(rObj)
        {
        }

        E3dProperties::E3dProperties(const E3dProperties& rProps, SdrObject& rObj)
        :   AttributeProperties(rProps, rObj)
        {
        }

        E3dProperties::~E3dProperties()
        {
        }

        std::unique_ptr<BaseProperties> E3dProperties::Clone(SdrObject& rObj) const
        {
            return std::unique_ptr<BaseProperties>(new E3dProperties(*this, rObj));
        }

        void E3dProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            E3dObject& rObj = static_cast<E3dObject&>(GetSdrObject());

            // call parent
            AttributeProperties::ItemSetChanged(rSet);

            // local changes
            rObj.StructureChanged();
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
