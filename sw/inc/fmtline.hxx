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
#ifndef INCLUDED_SW_INC_FMTLINE_HXX
#define INCLUDED_SW_INC_FMTLINE_HXX

#include <svl/poolitem.hxx>
#include <hintids.hxx>
#include <format.hxx>
#include "swdllapi.h"

class IntlWrapper;

class SW_DLLPUBLIC SwFmtLineNumber: public SfxPoolItem
{
    sal_uLong nStartValue   :24; ///< Starting value for the paragraph. 0 == no starting value.
    sal_uLong bCountLines   :1;  ///< Also count lines of paragraph.

public:
    SwFmtLineNumber();
    virtual ~SwFmtLineNumber();

    TYPEINFO_OVERRIDE();

    /// "Pure virtual methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const SAL_OVERRIDE;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const SAL_OVERRIDE;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const SAL_OVERRIDE;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) SAL_OVERRIDE;

    sal_uLong GetStartValue() const { return nStartValue; }
    sal_Bool  IsCount()       const { return bCountLines != 0; }

    void SetStartValue( sal_uLong nNew ) { nStartValue = nNew; }
    void SetCountLines( sal_Bool b )     { bCountLines = b;    }
};

inline const SwFmtLineNumber &SwAttrSet::GetLineNumber(sal_Bool bInP) const
    { return (const SwFmtLineNumber&)Get( RES_LINENUMBER,bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
