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


namespace sdr
{
    namespace properties
    {
        // create a new itemset
        std::unique_ptr<SfxItemSet> CircleProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return std::make_unique<SfxItemSet>(
                rPool,
                svl::Items<
                    // Ranges from SdrAttrObj, SdrCircObj
                    SDRATTR_START, SDRATTR_SHADOW_LAST,
                    SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                    SDRATTR_CIRC_FIRST, SDRATTR_CIRC_LAST,
                    SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,
                    // Range from SdrTextObj:
                    EE_ITEMS_START, EE_ITEMS_END>{});
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

        void CircleProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            SdrCircObj& rObj = static_cast<SdrCircObj&>(GetSdrObject());

            // call parent
            RectangleProperties::ItemSetChanged(rSet);

            // local changes
            rObj.ImpSetAttrToCircInfo();
        }

        void CircleProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
        {
            // call parent (always first thing to do, may create the SfxItemSet)
            RectangleProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            // local changes
            SdrCircObj& rObj = static_cast<SdrCircObj&>(GetSdrObject());
            rObj.SetXPolyDirty();

            // local changes
            rObj.ImpSetAttrToCircInfo();
        }

        void CircleProperties::ForceDefaultAttributes()
        {
            SdrCircObj& rObj = static_cast<SdrCircObj&>(GetSdrObject());
            SdrCircKind eKindA = SDRCIRC_FULL;
            SdrObjKind eKind = rObj.GetCircleKind();

            if(eKind == OBJ_SECT)
            {
                eKindA = SDRCIRC_SECT;
            }
            else if(eKind == OBJ_CARC)
            {
                eKindA = SDRCIRC_ARC;
            }
            else if(eKind == OBJ_CCUT)
            {
                eKindA = SDRCIRC_CUT;
            }

            if(eKindA != SDRCIRC_FULL)
            {
                // force ItemSet
                GetObjectItemSet();

                mpItemSet->Put(SdrCircKindItem(eKindA));

                if(rObj.GetStartAngle())
                {
                    mpItemSet->Put(makeSdrCircStartAngleItem(rObj.GetStartAngle()));
                }

                if(rObj.GetEndAngle() != 36000)
                {
                    mpItemSet->Put(makeSdrCircEndAngleItem(rObj.GetEndAngle()));
                }
            }

            // call parent after SetObjectItem(SdrCircKindItem())
            // because ForceDefaultAttr() will call
            // ImpSetAttrToCircInfo() which needs a correct
            // SdrCircKindItem
            RectangleProperties::ForceDefaultAttributes();
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
