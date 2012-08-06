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

#include <svx/sdr/properties/e3dproperties.hxx>
#include <svl/itemset.hxx>
#include <svx/svddef.hxx>
#include <svx/obj3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        // create a new itemset
        SfxItemSet& E3dProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return *(new SfxItemSet(rPool,

                // ranges from SdrAttrObj
                SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                // ranges from E3dObject, contains object and scene because of GetMergedItemSet()
                SDRATTR_3D_FIRST, SDRATTR_3D_LAST,

                // end
                0, 0));
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

        BaseProperties& E3dProperties::Clone(SdrObject& rObj) const
        {
            return *(new E3dProperties(*this, rObj));
        }

        void E3dProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            E3dObject& rObj = (E3dObject&)GetSdrObject();

            // call parent
            AttributeProperties::ItemSetChanged(rSet);

            // local changes
            rObj.StructureChanged();
        }

        void E3dProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            // call parent
            AttributeProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            // propagate call to contained objects
            const SdrObjList* pSub = ((const E3dObject&)GetSdrObject()).GetSubList();
            const sal_uInt32 nCount(pSub->GetObjCount());

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                pSub->GetObj(a)->GetProperties().SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
            }
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
