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
#ifndef _FMTROWSPLT_HXX
#define _FMTROWSPLT_HXX

#include <svl/eitem.hxx>
#include "swdllapi.h"
#include <hintids.hxx>
#include <format.hxx>

class IntlWrapper;

class SW_DLLPUBLIC SwFmtRowSplit : public SfxBoolItem
{
public:
    SwFmtRowSplit( sal_Bool bSplit = sal_True ) : SfxBoolItem( RES_ROW_SPLIT, bSplit ) {}

    // "pure virtual methods" of SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
};

inline const SwFmtRowSplit &SwAttrSet::GetRowSplit(sal_Bool bInP) const
    { return (const SwFmtRowSplit&)Get( RES_ROW_SPLIT,bInP); }

inline const SwFmtRowSplit &SwFmt::GetRowSplit(sal_Bool bInP) const
    { return aSet.GetRowSplit(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
