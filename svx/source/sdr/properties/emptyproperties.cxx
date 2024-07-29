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

#include <sdr/properties/emptyproperties.hxx>
#include <svl/itemset.hxx>
#include <svx/svdobj.hxx>


namespace sdr::properties
{
        EmptyProperties::EmptyProperties(SdrObject& rObj)
        :   BaseProperties(rObj)
        {
        }

        std::unique_ptr<BaseProperties> EmptyProperties::Clone(SdrObject& rObj) const
        {
            return std::unique_ptr<BaseProperties>(new EmptyProperties(rObj));
        }

        const SfxItemSet& EmptyProperties::GetObjectItemSet() const
        {
            assert(!"EmptyProperties::GetObjectItemSet() should never be called");
            abort();
        }

        SfxItemSet EmptyProperties::CreateObjectSpecificItemSet(SfxItemPool&)
        {
            assert(!"EmptyProperties::CreateObjectSpecificItemSet() should never be called");
            abort();
        }

        void EmptyProperties::SetObjectItem(const SfxPoolItem& /*rItem*/)
        {
            assert(!"EmptyProperties::SetObjectItem() should never be called");
        }

        void EmptyProperties::SetObjectItemDirect(const SfxPoolItem& /*rItem*/)
        {
            assert(!"EmptyProperties::SetObjectItemDirect() should never be called");
        }

        void EmptyProperties::ClearObjectItem(const sal_uInt16 /*nWhich*/)
        {
            assert(!"EmptyProperties::ClearObjectItem() should never be called");
        }

        void EmptyProperties::ClearObjectItemDirect(const sal_uInt16 /*nWhich*/)
        {
            assert(!"EmptyProperties::ClearObjectItemDirect() should never be called");
        }

        void EmptyProperties::SetObjectItemSet(const SfxItemSet& /*rSet*/, bool /*bAdjustTextFrameWidthAndHeight*/)
        {
            assert(!"EmptyProperties::SetObjectItemSet() should never be called");
        }

        void EmptyProperties::SetStyleSheet(SfxStyleSheet* /*pNewStyleSheet*/, bool /*bDontRemoveHardAttr*/,
                bool /*bBroadcast*/, bool /*bAdjustTextFrameWidthAndHeight*/)
        {
            assert(!"EmptyProperties::SetStyleSheet() should never be called");
        }

        SfxStyleSheet* EmptyProperties::GetStyleSheet() const
        {
            assert(!"EmptyProperties::GetStyleSheet() should never be called");
            return nullptr;
        }
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
