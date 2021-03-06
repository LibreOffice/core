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

#pragma once

#include <algorithm>
#include <i18nlangtag/lang.h>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include "address.hxx"

namespace com::sun::star::lang { struct Locale; }

class ScUnoConversion
{
public:
    static LanguageType GetLanguage( const css::lang::Locale& rLocale );
    static void FillLocale( css::lang::Locale& rLocale, LanguageType eLang );

    // CellAddress -> ScAddress
    static inline void  FillScAddress(
                            ScAddress& rScAddress,
                            const css::table::CellAddress& rApiAddress );
    // ScAddress -> CellAddress
    static inline void  FillApiAddress(
                            css::table::CellAddress& rApiAddress,
                            const ScAddress& rScAddress );
    // CellRangeAddress -> ScRange
    static inline void  FillScRange(
                            ScRange& rScRange,
                            const css::table::CellRangeAddress& rApiRange );
    // ScRange -> CellRangeAddress
    static inline void  FillApiRange(
                            css::table::CellRangeAddress& rApiRange,
                            const ScRange& rScRange );

    /** Returns true, if the passed ranges have at least one common cell. */
    static inline bool  Intersects(
                            const css::table::CellRangeAddress& rApiARange1,
                            const css::table::CellRangeAddress& rApiARange2 );
    /** Returns true, if the passed range rApiInner is completely inside the passed range rApiOuter. */
    static inline bool  Contains(
                            const css::table::CellRangeAddress& rApiOuter,
                            const css::table::CellRangeAddress& rApiInner );
};

inline void ScUnoConversion::FillScAddress(
        ScAddress& rScAddress,
        const css::table::CellAddress& rApiAddress )
{
    rScAddress.Set( static_cast<SCCOL>(rApiAddress.Column), static_cast<SCROW>(rApiAddress.Row), static_cast<SCTAB>(rApiAddress.Sheet) );
}

inline void ScUnoConversion::FillApiAddress(
        css::table::CellAddress& rApiAddress,
        const ScAddress& rScAddress )
{
    rApiAddress.Column = rScAddress.Col();
    rApiAddress.Row = rScAddress.Row();
    rApiAddress.Sheet = rScAddress.Tab();
}

inline void ScUnoConversion::FillScRange(
        ScRange& rScRange,
        const css::table::CellRangeAddress& rApiRange )
{
    rScRange.aStart.Set( static_cast<SCCOL>(rApiRange.StartColumn), static_cast<SCROW>(rApiRange.StartRow), static_cast<SCTAB>(rApiRange.Sheet) );
    rScRange.aEnd.Set( static_cast<SCCOL>(rApiRange.EndColumn), static_cast<SCROW>(rApiRange.EndRow), static_cast<SCTAB>(rApiRange.Sheet) );
}

inline void ScUnoConversion::FillApiRange(
        css::table::CellRangeAddress& rApiRange,
        const ScRange& rScRange )
{
    rApiRange.StartColumn = rScRange.aStart.Col();
    rApiRange.StartRow = rScRange.aStart.Row();
    rApiRange.Sheet = rScRange.aStart.Tab();
    rApiRange.EndColumn = rScRange.aEnd.Col();
    rApiRange.EndRow = rScRange.aEnd.Row();
}

inline bool ScUnoConversion::Intersects(
        const css::table::CellRangeAddress& rApiRange1,
        const css::table::CellRangeAddress& rApiRange2 )
{
    return (rApiRange1.Sheet == rApiRange2.Sheet) &&
        (::std::max( rApiRange1.StartColumn, rApiRange2.StartColumn ) <= ::std::min( rApiRange1.EndColumn, rApiRange2.EndColumn )) &&
        (::std::max( rApiRange1.StartRow, rApiRange2.StartRow ) <= ::std::min( rApiRange1.EndRow, rApiRange2.EndRow ));
}

inline bool ScUnoConversion::Contains(
        const css::table::CellRangeAddress& rApiOuter,
        const css::table::CellRangeAddress& rApiInner )
{
    return (rApiOuter.Sheet == rApiInner.Sheet) &&
        (rApiOuter.StartColumn <= rApiInner.StartColumn) && (rApiInner.EndColumn <= rApiOuter.EndColumn) &&
        (rApiOuter.StartRow <= rApiInner.StartRow) && (rApiInner.EndRow <= rApiOuter.EndRow);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
