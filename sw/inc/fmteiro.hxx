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
#ifndef INCLUDED_SW_INC_FMTEIRO_HXX
#define INCLUDED_SW_INC_FMTEIRO_HXX

#include <svl/eitem.hxx>
#include <hintids.hxx>
#include <format.hxx>
#include "swdllapi.h"

class IntlWrapper;

class SW_DLLPUBLIC SwFmtEditInReadonly : public SfxBoolItem
{
public:
    SwFmtEditInReadonly( sal_uInt16 nId = RES_EDIT_IN_READONLY,
                     sal_Bool bPrt = sal_False ) : SfxBoolItem( nId, bPrt ) {}

    /// "pure virtual methos" of SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const SAL_OVERRIDE;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const SAL_OVERRIDE;
};

inline const SwFmtEditInReadonly &SwAttrSet::GetEditInReadonly(sal_Bool bInP) const
    { return (const SwFmtEditInReadonly&)Get( RES_EDIT_IN_READONLY,bInP); }

inline const SwFmtEditInReadonly &SwFmt::GetEditInReadonly(sal_Bool bInP) const
    { return aSet.GetEditInReadonly(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
