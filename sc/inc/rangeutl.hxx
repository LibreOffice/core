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

#include "address.hxx"
#include "rangenam.hxx"
#include "dbdata.hxx"
#include "scdllapi.h"

namespace com::sun::star::table { struct CellAddress; }
namespace com::sun::star::table { struct CellRangeAddress; }
namespace com::sun::star::uno { template <typename > class Sequence; }

class ScArea;
class ScDocument;
class ScRangeList;

enum RutlNameScope
{
    RUTL_NONE = 0,
    RUTL_NAMES,
    RUTL_NAMES_LOCAL,
    RUTL_NAMES_GLOBAL,
    RUTL_DBASE
};

class SAL_DLLPUBLIC_RTTI ScRangeUtil
{
public:
    ScRangeUtil() = delete;

    static bool MakeArea        ( const OUString&   rAreaStr,
                                  ScArea&           rArea,
                                  const ScDocument& rDoc,
                                  SCTAB         nTab,
                                  ScAddress::Details const & rDetails );

    static void CutPosString    ( const OUString&     theAreaStr,
                                  OUString&           thePosStr );

    static bool IsAbsTabArea    ( const OUString&   rAreaStr,
                                  const ScDocument* pDoc,
                                  std::unique_ptr<ScArea[]>* ppAreas,
                                  sal_uInt16*       pAreaCount,
                                  bool              bAcceptCellRef = false,
                                  ScAddress::Details const & rDetails = ScAddress::detailsOOOa1 );

    SC_DLLPUBLIC static bool IsAbsArea       ( const OUString& rAreaStr,
                                  const ScDocument& rDoc,
                                  SCTAB     nTab,
                                  OUString*     pCompleteStr,
                                  ScRefAddress* pStartPos    = nullptr,
                                  ScRefAddress* pEndPos      = nullptr,
                                  ScAddress::Details const & rDetails = ScAddress::detailsOOOa1 );

    SC_DLLPUBLIC static bool IsAbsPos        ( const OUString& rPosStr,
                                  const ScDocument& rDoc,
                                  SCTAB     nTab,
                                  OUString*       pCompleteStr,
                                  ScRefAddress* pPosTripel   = nullptr,
                                  ScAddress::Details const & rDetails = ScAddress::detailsOOOa1 );

    static bool MakeRangeFromName( const OUString& rName,
                                   const ScDocument& rDoc,
                                    SCTAB           nCurTab,
                                    ScRange&        rRange,
                                  RutlNameScope eScope=RUTL_NAMES,
                                  ScAddress::Details const & rDetails = ScAddress::detailsOOOa1,
                                  bool bUseDetailsPos = false );
};

class SAL_DLLPUBLIC_RTTI ScRangeStringConverter
{
public:

/// helper methods
    static void         AssignString(
                            OUString& rString,
                            const OUString& rNewStr,
                            bool bAppendStr,
                            sal_Unicode cSeparator = ' ');

    static sal_Int32    IndexOf(
                            std::u16string_view rString,
                            sal_Unicode cSearchChar,
                            sal_Int32 nOffset,
                            sal_Unicode cQuote = '\'');

    static sal_Int32    IndexOfDifferent(
                            std::u16string_view rString,
                            sal_Unicode cSearchChar,
                            sal_Int32 nOffset );

    static sal_Int32    GetTokenCount(
                            std::u16string_view rString,
                            sal_Unicode cSeparator = ' ');

    static void         GetTokenByOffset(
                            OUString& rToken,
                            std::u16string_view rString,
                            sal_Int32& nOffset,
                            sal_Unicode cSeparator = ' ',
                            sal_Unicode cQuote = '\'');

    static void         AppendTableName(
                            OUStringBuffer& rBuf,
                            const OUString& rTabName);

/// String to Range core
    SC_DLLPUBLIC static bool GetAddressFromString(
                            ScAddress& rAddress,
                            std::u16string_view rAddressStr,
                            const ScDocument& rDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Int32& nOffset,
                            sal_Unicode cSeparator = ' ',
                            sal_Unicode cQuote = '\'');
    static bool     GetRangeFromString(
                            ScRange& rRange,
                            std::u16string_view rRangeStr,
                            const ScDocument& rDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Int32& nOffset,
                            sal_Unicode cSeparator = ' ',
                            sal_Unicode cQuote = '\'');
    SC_DLLPUBLIC static bool GetRangeListFromString(
                            ScRangeList& rRangeList,
                            std::u16string_view rRangeListStr,
                            const ScDocument& rDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Unicode cSeparator = ' ',
                            sal_Unicode cQuote = '\'');

    static bool     GetAreaFromString(
                            ScArea& rArea,
                            std::u16string_view rRangeStr,
                            const ScDocument& rDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Int32& nOffset,
                            sal_Unicode cSeparator = ' ');

/// String to Range API
    static bool     GetRangeFromString(
                            css::table::CellRangeAddress& rRange,
                            std::u16string_view rRangeStr,
                            const ScDocument& rDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Int32& nOffset,
                            sal_Unicode cSeparator = ' ');

/// Range to String core
    static void         GetStringFromAddress(
                            OUString& rString,
                            const ScAddress& rAddress,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Unicode cSeparator = ' ',
                            bool bAppendStr = false,
                            ScRefFlags nFormatFlags = ScRefFlags::VALID | ScRefFlags::TAB_3D );
    static void         GetStringFromRange(
                            OUString& rString,
                            const ScRange& rRange,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Unicode cSeparator = ' ',
                            bool bAppendStr = false,
                            ScRefFlags nFormatFlags = ScRefFlags::VALID | ScRefFlags::TAB_3D );
    SC_DLLPUBLIC static void GetStringFromRangeList(
                            OUString& rString,
                            const ScRangeList* pRangeList,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Unicode cSeparator = ' ');

    static void         GetStringFromArea(
                            OUString& rString,
                            const ScArea& rArea,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Unicode cSeparator,
                            bool bAppendStr = false,
                            ScRefFlags nFormatFlags = ScRefFlags::VALID | ScRefFlags::TAB_3D );

/// Range to String API
    static void         GetStringFromAddress(
                            OUString& rString,
                            const css::table::CellAddress& rAddress,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Unicode cSeparator = ' ',
                            bool bAppendStr = false );
    static void         GetStringFromRange(
                            OUString& rString,
                            const css::table::CellRangeAddress& rRange,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Unicode cSeparator = ' ',
                            bool bAppendStr = false,
                            ScRefFlags nFormatFlags = ScRefFlags::VALID | ScRefFlags::TAB_3D );
    static void         GetStringFromRangeList(
                            OUString& rString,
                            const css::uno::Sequence< css::table::CellRangeAddress >& rRangeSeq,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Unicode cSeparator = ' ' );

/// XML Range to Calc Range
    static void         GetStringFromXMLRangeString(
                            OUString& rString,
                            std::u16string_view rXMLRange,
                            const ScDocument& rDoc );

/// String to RangeData core
    static ScRangeData* GetRangeDataFromString( const OUString& rString, const SCTAB nTab,
                            const ScDocument& rDoc, formula::FormulaGrammar::AddressConvention eConv );
};

class ScArea
{
public:
            ScArea( SCTAB tab      = 0,
                    SCCOL colStart = 0,
                    SCROW rowStart = 0,
                    SCCOL colEnd   = 0,
                    SCROW rowEnd   = 0 );

    bool    operator==  ( const ScArea& r ) const;

public:
    SCTAB nTab;
    SCCOL nColStart;
    SCROW nRowStart;
    SCCOL nColEnd;
    SCROW nRowEnd;
};

///  @return areas with reference and all db-areas

class SC_DLLPUBLIC ScAreaNameIterator
{
private:
    ScRangeName*    pRangeName;
    ScDBCollection* pDBCollection;
    ScRangeName::const_iterator maRNPos;
    ScRangeName::const_iterator maRNEnd;
    ScDBCollection::NamedDBs::const_iterator maDBPos;
    ScDBCollection::NamedDBs::const_iterator maDBEnd;
    bool            bFirstPass;

public:
    ScAreaNameIterator( const ScDocument& rDoc );

    bool Next( OUString& rName, ScRange& rRange );
    bool WasDBName() const { return !bFirstPass; }
};

struct SC_DLLPUBLIC ScRangeUpdater
{
    ScRangeUpdater() = delete;

    static void UpdateInsertTab(ScAddress& rAddr, const sc::RefUpdateInsertTabContext& rCxt);

    /** This is for the base-cell-address of a defined name or conditional
        format, not for references. A sheet position on or after the start of
        the deleted range is moved towards the beginning by the amount of
        deleted sheets, within the deleted range to the front of that or set to
        0 (as there is always at least one sheet in a document) if the position
        would result in a negative value, e.g. if position was 0 and (only)
        sheet 0 is deleted it would had become -1.
     */
    static void UpdateDeleteTab(ScAddress& rAddr, const sc::RefUpdateDeleteTabContext& rCxt);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
