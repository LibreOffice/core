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

#ifndef SC_RANGEUTL_HXX
#define SC_RANGEUTL_HXX

#include <tools/string.hxx>

#include "address.hxx"
#include "rangenam.hxx"
#include "scdllapi.h"

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/uno/Sequence.hxx>

//------------------------------------------------------------------------

class SvStream;

class ScArea;
class ScDocument;
class ScRange;
class ScRangeName;
class ScRangeList;
class ScDBCollection;

enum RutlNameScope { RUTL_NONE=0, RUTL_NAMES, RUTL_DBASE };

//------------------------------------------------------------------------

class SC_DLLPUBLIC ScRangeUtil
{
public:
                ScRangeUtil()  {}
                ~ScRangeUtil() {}

    sal_Bool    MakeArea            ( const String&     rAreaStr,
                                  ScArea&           rArea,
                                  ScDocument*       pDoc,
                                  SCTAB         nTab,
                                  ScAddress::Details const & rDetails = ScAddress::detailsOOOa1 ) const;

    void    CutPosString        ( const String&     theAreaStr,
                                  String&           thePosStr ) const;

    sal_Bool    IsAbsTabArea        ( const String&     rAreaStr,
                                  ScDocument*       pDoc,
                                  ScArea***         pppAreas    = 0,
                                  sal_uInt16*           pAreaCount  = 0,
                                  sal_Bool              bAcceptCellRef = false,
                                  ScAddress::Details const & rDetails = ScAddress::detailsOOOa1 ) const;

    sal_Bool    IsAbsArea           ( const String& rAreaStr,
                                  ScDocument*   pDoc,
                                  SCTAB     nTab,
                                  String*       pCompleteStr = 0,
                                  ScRefAddress* pStartPos    = 0,
                                  ScRefAddress* pEndPos      = 0,
                                  ScAddress::Details const & rDetails = ScAddress::detailsOOOa1 ) const;

    sal_Bool    IsRefArea           ( const String&,
                                  ScDocument*,
                                  SCTAB,
                                  String* = 0,
                                  ScRefAddress* = 0 ) const
                                      { return false; }

    sal_Bool    IsAbsPos            ( const String& rPosStr,
                                  ScDocument*   pDoc,
                                  SCTAB     nTab,
                                  String*       pCompleteStr = 0,
                                  ScRefAddress* pPosTripel   = 0,
                                  ScAddress::Details const & rDetails = ScAddress::detailsOOOa1 ) const;

    sal_Bool    MakeRangeFromName   ( const String& rName,
                                    ScDocument*     pDoc,
                                    SCTAB           nCurTab,
                                    ScRange&        rRange,
                                  RutlNameScope eScope=RUTL_NAMES,
                                  ScAddress::Details const & rDetails = ScAddress::detailsOOOa1 ) const;
};

//------------------------------------------------------------------------

class SC_DLLPUBLIC ScRangeStringConverter
{
public:

// helper methods
    static void         AssignString(
                            ::rtl::OUString& rString,
                            const ::rtl::OUString& rNewStr,
                            sal_Bool bAppendStr,
                            sal_Unicode cSeperator = ' ');

    static sal_Int32    IndexOf(
                            const ::rtl::OUString& rString,
                            sal_Unicode cSearchChar,
                            sal_Int32 nOffset,
                            sal_Unicode cQuote = '\'');

    static sal_Int32    IndexOfDifferent(
                            const ::rtl::OUString& rString,
                            sal_Unicode cSearchChar,
                            sal_Int32 nOffset );

    static sal_Int32    GetTokenCount(
                            const ::rtl::OUString& rString,
                            sal_Unicode cSeperator = ' ',
                            sal_Unicode cQuote = '\'');

    static void         GetTokenByOffset(
                            ::rtl::OUString& rToken,
                            const ::rtl::OUString& rString,
                            sal_Int32& nOffset,
                            sal_Unicode cSeperator = ' ',
                            sal_Unicode cQuote = '\'');

    static void         AppendTableName(
                            ::rtl::OUStringBuffer& rBuf,
                            const ::rtl::OUString& rTabName,
                            sal_Unicode cQuote = '\'');

// String to Range core
    static sal_Bool     GetAddressFromString(
                            ScAddress& rAddress,
                            const ::rtl::OUString& rAddressStr,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Int32& nOffset,
                            sal_Unicode cSeperator = ' ',
                            sal_Unicode cQuote = '\'');
    static sal_Bool     GetRangeFromString(
                            ScRange& rRange,
                            const ::rtl::OUString& rRangeStr,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Int32& nOffset,
                            sal_Unicode cSeperator = ' ',
                            sal_Unicode cQuote = '\'');
    static sal_Bool     GetRangeListFromString(
                            ScRangeList& rRangeList,
                            const ::rtl::OUString& rRangeListStr,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Unicode cSeperator = ' ',
                            sal_Unicode cQuote = '\'');

    static sal_Bool     GetAreaFromString(
                            ScArea& rArea,
                            const ::rtl::OUString& rRangeStr,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Int32& nOffset,
                            sal_Unicode cSeperator = ' ',
                            sal_Unicode cQuote = '\'');

// String to Range API
    static sal_Bool     GetAddressFromString(
                            ::com::sun::star::table::CellAddress& rAddress,
                            const ::rtl::OUString& rAddressStr,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Int32& nOffset,
                            sal_Unicode cSeperator = ' ',
                            sal_Unicode cQuote = '\'');
    static sal_Bool     GetRangeFromString(
                            ::com::sun::star::table::CellRangeAddress& rRange,
                            const ::rtl::OUString& rRangeStr,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Int32& nOffset,
                            sal_Unicode cSeperator = ' ',
                            sal_Unicode cQuote = '\'');
    static sal_Bool     GetRangeListFromString(
                            ::com::sun::star::uno::Sequence< ::com::sun::star::table::CellRangeAddress >& rRangeSeq,
                            const ::rtl::OUString& rRangeListStr,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Unicode cSeperator = ' ',
                            sal_Unicode cQuote = '\'');

// Range to String core
    static void         GetStringFromAddress(
                            ::rtl::OUString& rString,
                            const ScAddress& rAddress,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Unicode cSeperator = ' ',
                            sal_Bool bAppendStr = false,
                            sal_uInt16 nFormatFlags = (SCA_VALID | SCA_TAB_3D) );
    static void         GetStringFromRange(
                            ::rtl::OUString& rString,
                            const ScRange& rRange,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Unicode cSeperator = ' ',
                            sal_Bool bAppendStr = false,
                            sal_uInt16 nFormatFlags = (SCA_VALID | SCA_TAB_3D) );
    static void         GetStringFromRangeList(
                            ::rtl::OUString& rString,
                            const ScRangeList* pRangeList,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Unicode cSeperator = ' ',
                            sal_uInt16 nFormatFlags = (SCA_VALID | SCA_TAB_3D));

    static void         GetStringFromArea(
                            ::rtl::OUString& rString,
                            const ScArea& rArea,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Unicode cSeperator = ' ',
                            sal_Bool bAppendStr = false,
                            sal_uInt16 nFormatFlags = (SCA_VALID | SCA_TAB_3D) );

// Range to String API
    static void         GetStringFromAddress(
                            ::rtl::OUString& rString,
                            const ::com::sun::star::table::CellAddress& rAddress,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Unicode cSeperator = ' ',
                            sal_Bool bAppendStr = false,
                            sal_uInt16 nFormatFlags = (SCA_VALID | SCA_TAB_3D) );
    static void         GetStringFromRange(
                            ::rtl::OUString& rString,
                            const ::com::sun::star::table::CellRangeAddress& rRange,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Unicode cSeperator = ' ',
                            sal_Bool bAppendStr = false,
                            sal_uInt16 nFormatFlags = (SCA_VALID | SCA_TAB_3D) );
    static void         GetStringFromRangeList(
                            ::rtl::OUString& rString,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::table::CellRangeAddress >& rRangeSeq,
                            const ScDocument* pDocument,
                            formula::FormulaGrammar::AddressConvention eConv,
                            sal_Unicode cSeperator = ' ',
                            sal_uInt16 nFormatFlags = (SCA_VALID | SCA_TAB_3D) );

// XML Range to Calc Range
    static void         GetStringFromXMLRangeString(
                            ::rtl::OUString& rString,
                            const ::rtl::OUString& rXMLRange,
                            ScDocument* pDoc );
};

//------------------------------------------------------------------------

class ScArea
{
public:
            ScArea( SCTAB tab      = 0,
                    SCCOL colStart = 0,
                    SCROW rowStart = 0,
                    SCCOL colEnd   = 0,
                    SCROW rowEnd   = 0 );

            ScArea( const ScArea& r );

    ScArea& operator=   ( const ScArea& r );
    sal_Bool    operator==  ( const ScArea& r ) const;
    sal_Bool    operator!=  ( const ScArea& r ) const  { return !( operator==(r) ); }

public:
    SCTAB nTab;
    SCCOL nColStart;
    SCROW nRowStart;
    SCCOL nColEnd;
    SCROW nRowEnd;
};

//------------------------------------------------------------------------

//
//  returns areas with reference and all db-areas
//

class SC_DLLPUBLIC ScAreaNameIterator
{
private:
    String          aStrNoName;
    ScRangeName*    pRangeName;
    ScRangeName::const_iterator maRNPos;
    ScRangeName::const_iterator maRNEnd;
    ScDBCollection* pDBCollection;
    bool            bFirstPass;
    size_t          nPos;

public:
            ScAreaNameIterator( ScDocument* pDoc );
            ~ScAreaNameIterator() {}

    sal_Bool    Next( String& rName, ScRange& rRange );
    sal_Bool    WasDBName() const   { return !bFirstPass; }
};


#endif // SC_RANGEUTL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
