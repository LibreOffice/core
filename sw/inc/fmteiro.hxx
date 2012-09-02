/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _FMTEIRO_HXX
#define _FMTEIRO_HXX


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
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
};

inline const SwFmtEditInReadonly &SwAttrSet::GetEditInReadonly(sal_Bool bInP) const
    { return (const SwFmtEditInReadonly&)Get( RES_EDIT_IN_READONLY,bInP); }

inline const SwFmtEditInReadonly &SwFmt::GetEditInReadonly(sal_Bool bInP) const
    { return aSet.GetEditInReadonly(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
