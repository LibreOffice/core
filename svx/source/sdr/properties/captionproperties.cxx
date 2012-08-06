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

#include <svx/sdr/properties/captionproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svx/svddef.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svdocapt.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        // create a new itemset
        SfxItemSet& CaptionProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return *(new SfxItemSet(rPool,

                // range from SdrAttrObj
                SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                // range from SdrCaptionObj
                SDRATTR_CAPTION_FIRST, SDRATTR_CAPTION_LAST,

                // range from SdrTextObj
                EE_ITEMS_START, EE_ITEMS_END,

                // end
                0, 0));
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

        BaseProperties& CaptionProperties::Clone(SdrObject& rObj) const
        {
            return *(new CaptionProperties(*this, rObj));
        }

        void CaptionProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            SdrCaptionObj& rObj = (SdrCaptionObj&)GetSdrObject();

            // local changes
            rObj.ImpRecalcTail();

            // call parent
            RectangleProperties::ItemSetChanged(rSet);
        }

        void CaptionProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            SdrCaptionObj& rObj = (SdrCaptionObj&)GetSdrObject();

            // call parent
            RectangleProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            // local changes
            rObj.ImpRecalcTail();
        }

        void CaptionProperties::ForceDefaultAttributes()
        {
            // call parent
            RectangleProperties::ForceDefaultAttributes();

            // force ItemSet
            GetObjectItemSet();

            // this was set by TextProperties::ForceDefaultAttributes(),
            // retet to default
            mpItemSet->ClearItem(XATTR_LINESTYLE);
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
