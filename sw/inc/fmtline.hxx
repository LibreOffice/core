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
#include <tools/solar.h>
#include "hintids.hxx"
#include "swdllapi.h"
#include "swatrset.hxx"

/// Contains the line numbering properties of this paragraph. Format -> Paragraph -> Outline & List
/// tab, Line Numbering section is the UI for this.
class SW_DLLPUBLIC SwFormatLineNumber final : public SfxPoolItem
{
    sal_uLong m_nStartValue   :24; ///< Starting value for the paragraph. 0 == no starting value.
    bool      m_bCountLines   :1;  ///< Also count lines of paragraph.

public:
    DECLARE_ITEM_TYPE_FUNCTION(SwFormatLineNumber)
    SwFormatLineNumber();
    virtual ~SwFormatLineNumber() override;

    SwFormatLineNumber(SwFormatLineNumber const &) = default;
    SwFormatLineNumber(SwFormatLineNumber &&) = default;
    SwFormatLineNumber & operator =(SwFormatLineNumber const &) = delete; // due to SfxPoolItem
    SwFormatLineNumber & operator =(SwFormatLineNumber &&) = delete; // due to SfxPoolItem

    static SfxPoolItem* CreateDefault();

    /// "Pure virtual methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SwFormatLineNumber* Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& rIntl ) const override;
    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    sal_uLong GetStartValue() const { return m_nStartValue; }
    bool  IsCount()           const { return m_bCountLines; }

    void SetStartValue( sal_uLong nNew ) { m_nStartValue = nNew; }
    void SetCountLines( bool b )     { m_bCountLines = b;    }
};

inline const SwFormatLineNumber &SwAttrSet::GetLineNumber(bool bInP) const
    { return Get( RES_LINENUMBER,bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
