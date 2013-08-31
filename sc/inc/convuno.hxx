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

#ifndef SC_CONVUNO_HXX
#define SC_CONVUNO_HXX

#include <algorithm>
#include <i18nlangtag/lang.h>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include "global.hxx"
#include "address.hxx"

class ScUnoConversion
{
public:
    static LanguageType GetLanguage( const com::sun::star::lang::Locale& rLocale );
    static void FillLocale( com::sun::star::lang::Locale& rLocale, LanguageType eLang );

    // CellAddress -> ScAddress
    static inline void  FillScAddress(
                            ScAddress& rScAddress,
                            const ::com::sun::star::table::CellAddress& rApiAddress );
    // ScAddress -> CellAddress
    static inline void  FillApiAddress(
                            ::com::sun::star::table::CellAddress& rApiAddress,
                            const ScAddress& rScAddress );
    // CellRangeAddress -> ScRange
    static inline void  FillScRange(
                            ScRange& rScRange,
                            const ::com::sun::star::table::CellRangeAddress& rApiRange );
    // ScRange -> CellRangeAddress
    static inline void  FillApiRange(
                            ::com::sun::star::table::CellRangeAddress& rApiRange,
                            const ScRange& rScRange );
    // CellAddress -> CellRangeAddress
    static inline void  FillApiRange(
                            ::com::sun::star::table::CellRangeAddress& rApiRange,
                            const ::com::sun::star::table::CellAddress& rApiAddress );
    // CellRangeAddress-Start -> CellAddress
    static inline void  FillApiStartAddress(
                            ::com::sun::star::table::CellAddress& rApiAddress,
                            const ::com::sun::star::table::CellRangeAddress& rApiRange );
    // CellRangeAddress-End -> CellAddress
    static inline void  FillApiEndAddress(
                            ::com::sun::star::table::CellAddress& rApiAddress,
                            const ::com::sun::star::table::CellRangeAddress& rApiRange );

    /** Returns true, if the passed ranges have at least one common cell. */
    static inline bool  Intersects(
                            const ::com::sun::star::table::CellRangeAddress& rApiARange1,
                            const ::com::sun::star::table::CellRangeAddress& rApiARange2 );
    /** Returns true, if the passed address rApiInner is inside the passed range rApiOuter. */
    static inline bool  Contains(
                            const ::com::sun::star::table::CellRangeAddress& rApiOuter,
                            const ::com::sun::star::table::CellAddress& rApiInner );
    /** Returns true, if the passed range rApiInner is completely inside the passed range rApiOuter. */
    static inline bool  Contains(
                            const ::com::sun::star::table::CellRangeAddress& rApiOuter,
                            const ::com::sun::star::table::CellRangeAddress& rApiInner );
};

inline void ScUnoConversion::FillScAddress(
        ScAddress& rScAddress,
        const ::com::sun::star::table::CellAddress& rApiAddress )
{
    rScAddress.Set( (SCCOL)rApiAddress.Column, (SCROW)rApiAddress.Row, (SCTAB)rApiAddress.Sheet );
}

inline void ScUnoConversion::FillApiAddress(
        ::com::sun::star::table::CellAddress& rApiAddress,
        const ScAddress& rScAddress )
{
    rApiAddress.Column = rScAddress.Col();
    rApiAddress.Row = rScAddress.Row();
    rApiAddress.Sheet = rScAddress.Tab();
}

inline void ScUnoConversion::FillScRange(
        ScRange& rScRange,
        const ::com::sun::star::table::CellRangeAddress& rApiRange )
{
    rScRange.aStart.Set( (SCCOL)rApiRange.StartColumn, (SCROW)rApiRange.StartRow, (SCTAB)rApiRange.Sheet );
    rScRange.aEnd.Set( (SCCOL)rApiRange.EndColumn, (SCROW)rApiRange.EndRow, (SCTAB)rApiRange.Sheet );
}

inline void ScUnoConversion::FillApiRange(
        ::com::sun::star::table::CellRangeAddress& rApiRange,
        const ScRange& rScRange )
{
    rApiRange.StartColumn = rScRange.aStart.Col();
    rApiRange.StartRow = rScRange.aStart.Row();
    rApiRange.Sheet = rScRange.aStart.Tab();
    rApiRange.EndColumn = rScRange.aEnd.Col();
    rApiRange.EndRow = rScRange.aEnd.Row();
}

inline void ScUnoConversion::FillApiRange(
        ::com::sun::star::table::CellRangeAddress& rApiRange,
        const ::com::sun::star::table::CellAddress& rApiAddress )
{
    rApiRange.StartColumn = rApiRange.EndColumn = rApiAddress.Column;
    rApiRange.StartRow = rApiRange.EndRow = rApiAddress.Row;
    rApiRange.Sheet = rApiAddress.Sheet;
}

inline void ScUnoConversion::FillApiStartAddress(
        ::com::sun::star::table::CellAddress& rApiAddress,
        const ::com::sun::star::table::CellRangeAddress& rApiRange )
{
    rApiAddress.Column = rApiRange.StartColumn;
    rApiAddress.Row = rApiRange.StartRow;
    rApiAddress.Sheet = rApiRange.Sheet;
}

inline void ScUnoConversion::FillApiEndAddress(
        ::com::sun::star::table::CellAddress& rApiAddress,
        const ::com::sun::star::table::CellRangeAddress& rApiRange )
{
    rApiAddress.Column = rApiRange.EndColumn;
    rApiAddress.Row = rApiRange.EndRow;
    rApiAddress.Sheet = rApiRange.Sheet;
}

inline bool ScUnoConversion::Intersects(
        const ::com::sun::star::table::CellRangeAddress& rApiRange1,
        const ::com::sun::star::table::CellRangeAddress& rApiRange2 )
{
    return (rApiRange1.Sheet == rApiRange2.Sheet) &&
        (::std::max( rApiRange1.StartColumn, rApiRange2.StartColumn ) <= ::std::min( rApiRange1.EndColumn, rApiRange2.EndColumn )) &&
        (::std::max( rApiRange1.StartRow, rApiRange2.StartRow ) <= ::std::min( rApiRange1.EndRow, rApiRange2.EndRow ));
}

inline bool ScUnoConversion::Contains(
        const ::com::sun::star::table::CellRangeAddress& rApiOuter,
        const ::com::sun::star::table::CellAddress& rApiInner )
{
    return (rApiOuter.Sheet == rApiInner.Sheet) &&
        (rApiOuter.StartColumn <= rApiInner.Column) && (rApiInner.Column <= rApiOuter.EndColumn) &&
        (rApiOuter.StartRow <= rApiInner.Row) && (rApiInner.Row <= rApiOuter.EndRow);
}

inline bool ScUnoConversion::Contains(
        const ::com::sun::star::table::CellRangeAddress& rApiOuter,
        const ::com::sun::star::table::CellRangeAddress& rApiInner )
{
    return (rApiOuter.Sheet == rApiInner.Sheet) &&
        (rApiOuter.StartColumn <= rApiInner.StartColumn) && (rApiInner.EndColumn <= rApiOuter.EndColumn) &&
        (rApiOuter.StartRow <= rApiInner.StartRow) && (rApiInner.EndRow <= rApiOuter.EndRow);
}

inline sal_Bool operator==(
        const ::com::sun::star::table::CellAddress& rApiAddress1,
        const ::com::sun::star::table::CellAddress& rApiAddress2 )
{
    return
        (rApiAddress1.Column == rApiAddress2.Column) &&
        (rApiAddress1.Row == rApiAddress2.Row) &&
        (rApiAddress1.Sheet == rApiAddress2.Sheet);
}

inline sal_Bool operator!=(
        const ::com::sun::star::table::CellAddress& rApiAddress1,
        const ::com::sun::star::table::CellAddress& rApiAddress2 )
{
    return !(rApiAddress1 == rApiAddress2);
}

inline sal_Bool operator==(
        const ::com::sun::star::table::CellRangeAddress& rApiRange1,
        const ::com::sun::star::table::CellRangeAddress& rApiRange2 )
{
    return
        (rApiRange1.StartColumn == rApiRange2.StartColumn) &&
        (rApiRange1.StartRow == rApiRange2.StartRow) &&
        (rApiRange1.EndColumn == rApiRange2.EndColumn) &&
        (rApiRange1.EndRow == rApiRange2.EndRow) &&
        (rApiRange1.Sheet == rApiRange2.Sheet);
}

inline sal_Bool operator!=(
        const ::com::sun::star::table::CellRangeAddress& rApiRange1,
        const ::com::sun::star::table::CellRangeAddress& rApiRange2 )
{
    return !(rApiRange1 == rApiRange2);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
