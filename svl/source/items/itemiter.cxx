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

#include <svl/itemiter.hxx>
#include <svl/itemset.hxx>

SfxItemState SfxItemIter::GetItemState(bool bSrchInParent, const SfxPoolItem** ppItem) const
{
    if (IsAtEnd())
        return SfxItemState::UNKNOWN;

    const sal_uInt16 nWhich(maCurrent->first);
    SfxItemState eState(
        m_rSet.GetItemState_ForWhichID(SfxItemState::UNKNOWN, nWhich, true, ppItem));

    // search in parent?
    if (bSrchInParent && nullptr != m_rSet.GetParent()
        && (SfxItemState::UNKNOWN == eState || SfxItemState::DEFAULT == eState))
    {
        // nOffset was only valid for *local* SfxItemSet, need to continue with WhichID
        // const sal_uInt16 nWhich(m_rSet.GetWhichByOffset(m_nCurrent));
        eState = m_rSet.GetParent()->GetItemState_ForWhichID(eState, nWhich, true, ppItem);
    }

    return eState;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
