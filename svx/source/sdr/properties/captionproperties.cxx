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

#include <sdr/properties/captionproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svx/svddef.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svdocapt.hxx>


namespace sdr
{
    namespace properties
    {
        // create a new itemset
        std::unique_ptr<SfxItemSet> CaptionProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return std::make_unique<SfxItemSet>(
                rPool,
                svl::Items<
                    // Ranges from SdrAttrObj, SdrCaptionObj:
                    SDRATTR_START, SDRATTR_MISC_LAST,
                    SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,
                    // Range from SdrTextObj:
                    EE_ITEMS_START, EE_ITEMS_END>{});
        }

        CaptionProperties::CaptionProperties(SdrObject& rObj)
        :   RectangleProperties(rObj)
        {
        }

        CaptionProperties::CaptionProperties(const CaptionProperties& rProps, SdrObject& rObj)
        :   RectangleProperties(rProps, rObj)
        {
        }

        CaptionProperties::~CaptionProperties()
        {
        }

        std::unique_ptr<BaseProperties> CaptionProperties::Clone(SdrObject& rObj) const
        {
            return std::unique_ptr<BaseProperties>(new CaptionProperties(*this, rObj));
        }

        void CaptionProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            SdrCaptionObj& rObj = static_cast<SdrCaptionObj&>(GetSdrObject());

            // local changes
            rObj.ImpRecalcTail();

            // call parent
            RectangleProperties::ItemSetChanged(rSet);
        }

        void CaptionProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
        {
            // call parent (always first thing to do, may create the SfxItemSet)
            RectangleProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            // local changes
            SdrCaptionObj& rObj = static_cast<SdrCaptionObj&>(GetSdrObject());
            rObj.ImpRecalcTail();
        }

        void CaptionProperties::ForceDefaultAttributes()
        {
            // call parent
            RectangleProperties::ForceDefaultAttributes();

            // force ItemSet
            GetObjectItemSet();

            // this was set by TextProperties::ForceDefaultAttributes(),
            // reset to default
            mpItemSet->ClearItem(XATTR_LINESTYLE);
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
