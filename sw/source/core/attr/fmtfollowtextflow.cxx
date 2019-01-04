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

#include <fmtfollowtextflow.hxx>
#include <unomid.h>

#include <sal/log.hxx>


SfxPoolItem*    SwFormatFollowTextFlow::Clone( SfxItemPool * ) const
{
    return new SwFormatFollowTextFlow(*this);
}


bool SwFormatFollowTextFlow::PutValue(const css::uno::Any& rVal, sal_uInt8 aInt)
{
    switch( aInt )
    {
        case MID_FOLLOW_TEXT_FLOW :
        {
            bool bTheValue = bool();
            if (rVal >>= bTheValue)
            {
                SetValue( bTheValue );
                return true;
            }
            break;
        }
        case MID_FTF_LAYOUT_IN_CELL :
        {
            bool bTheValue = bool();
            if (rVal >>= bTheValue)
            {
                mbLayoutInCell = bTheValue;
                return true;
            }
            break;
        }
    }
    SAL_WARN("sw.ui", "SfxBoolItem::PutValue(): Wrong type");
    return false;
}


bool SwFormatFollowTextFlow::QueryValue(css::uno::Any& rVal, sal_uInt8 aInt) const
{
    switch( aInt )
    {
        case MID_FOLLOW_TEXT_FLOW :
        {
            rVal <<= GetValue();
            break;
        }
        case MID_FTF_LAYOUT_IN_CELL :
        {
            rVal <<= GetLayoutInCell();
            break;
        }
    }
    return true;
}

bool SwFormatFollowTextFlow::operator==(const SfxPoolItem& rItem) const
{
    assert(dynamic_cast<const SwFormatFollowTextFlow*>(&rItem) != nullptr);
    return SfxBoolItem::operator==(rItem)
           && mbLayoutInCell == static_cast<SwFormatFollowTextFlow const*>(&rItem)->mbLayoutInCell;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
