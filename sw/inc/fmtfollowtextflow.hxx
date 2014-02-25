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
#ifndef INCLUDED_SW_INC_FMTFOLLOWTEXTFLOW_HXX
#define INCLUDED_SW_INC_FMTFOLLOWTEXTFLOW_HXX

#include <svl/eitem.hxx>
#include <hintids.hxx>
#include <format.hxx>
#include "swdllapi.h"

class IntlWrapper;

class SW_DLLPUBLIC SwFmtFollowTextFlow : public SfxBoolItem
{
public:
    SwFmtFollowTextFlow( sal_Bool bFlag = sal_False )
        : SfxBoolItem( RES_FOLLOW_TEXT_FLOW, bFlag ) {}

    TYPEINFO();

    /// "pure virtual methods" of SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
};

inline const SwFmtFollowTextFlow &SwAttrSet::GetFollowTextFlow(sal_Bool bInP) const
    { return (const SwFmtFollowTextFlow&)Get( RES_FOLLOW_TEXT_FLOW, bInP ); }

inline const SwFmtFollowTextFlow &SwFmt::GetFollowTextFlow(sal_Bool bInP) const
    { return aSet.GetFollowTextFlow( bInP ); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
