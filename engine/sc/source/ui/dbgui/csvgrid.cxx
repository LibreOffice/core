/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <boost/property_tree/json_parser.hpp>
#include <csvgrid.hxx>
#include <csvtablebox.hxx>
#include <tools/json_writer.hxx>
#include <optutil.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <o3tl/string_view.hxx>
#include <sal/log.hxx>

#include <algorithm>
#include <optional>

#include <osl/diagnose.h>
#include <vcl/keycodes.hxx>
#include <vcl/weld.hxx>
#include <asciiopt.hxx>
#include <impex.hxx>

namespace {

struct Func_SetType
{
    sal_Int32                   mnType;
    explicit                    Func_SetType( sal_Int32 nType ) : mnType( nType ) {}
    void                 operator()( ScCsvColState& rState ) const
        { rState.mnType = mnType; }
};

struct Func_Select
{
    bool                        mbSelect;
    explicit                    Func_Select( bool bSelect ) : mbSelect( bSelect ) {}
    void                 operator()( ScCsvColState& rState ) const
        { rState.Select( mbSelect ); }
};

/** client-event commands. The string form is the wire identifier; keep in
    sync with the client-side Widget.CsvGrid handler. */
enum class CsvEvent
{
    Unknown,
    SelectColumn,
    SetColumnType,
    ToggleSplit
};

CsvEvent parseCsvEvent(std::u16string_view rCmd)
{
    if (rCmd == u"selectcolumn")
        return CsvEvent::SelectColumn;
    if (rCmd == u"setcolumntype")
        return CsvEvent::SetColumnType;
    if (rCmd == u"togglesplit")
        return CsvEvent::ToggleSplit;
    return CsvEvent::Unknown;
}

std::optional<boost::property_tree::ptree> parseEventJson(std::u16string_view rData)
{
    std::stringstream aStream(OUStringToOString(rData, RTL_TEXTENCODING_UTF8).getStr());
    boost::property_tree::ptree aTree;
    try
    {
        boost::property_tree::read_json(aStream, aTree);
    }
    catch (const std::exception& e)
    {
        SAL_WARN("sc.ui", "ScCsvGrid: malformed JSON event payload: " << e.what());
        return std::nullopt;
    }
    return aTree;
}

}

using namespace com::sun::star::uno;

constexpr OUString SEP_PATH = u"Office.Calc/Dialogs/CSVImport"_ustr;
constexpr OUString FIXED_WIDTH_LIST = u"FixedWidthList"_ustr;

static void load_FixedWidthList(ScCsvSplits &rSplits)
{
    Sequence<Any>aValues;
    const Any *pProperties;
    Sequence<OUString> aNames { FIXED_WIDTH_LIST };
    ScLinkConfigItem aItem( SEP_PATH );

    aValues = aItem.GetProperties( aNames );
    pProperties = aValues.getConstArray();

    if( !pProperties[0].hasValue() )
        return;

    rSplits.Clear();

    OUString sFixedWidthLists;
    pProperties[0] >>= sFixedWidthLists;

    sal_Int32 nIdx {0};
    for(;;)
    {
        const sal_Int32 n = o3tl::toInt32(o3tl::getToken(sFixedWidthLists, 0, ';', nIdx));
        if (nIdx<0)
        {
            break;
        }
        rSplits.Insert(n);
    }
}

static void save_FixedWidthList(const ScCsvSplits& rSplits)
{
    OUStringBuffer sSplits;
    sal_uInt32 n = rSplits.Count();
    for (sal_uInt32 i = 0; i < n; ++i)
    {
        sSplits.append(OUString::number(rSplits[i]) + ";");
    }

    OUString sFixedWidthLists = sSplits.makeStringAndClear();
    Sequence<Any> aValues;
    Any *pProperties;
    Sequence<OUString> aNames { FIXED_WIDTH_LIST };
    ScLinkConfigItem aItem( SEP_PATH );

    aValues = aItem.GetProperties( aNames );
    pProperties = aValues.getArray();
    pProperties[0] <<= sFixedWidthLists;

    aItem.PutProperties(aNames, aValues);
}

ScCsvGrid::ScCsvGrid(const ScCsvLayoutData& rData, ScCsvTableBox* pTableBox)
    : ScCsvControl(rData)
    , mpTableBox(pTableBox)
    , maColStates( 1 )
    , maTypeNames( 1 )
    , mnFirstImpLine( 0 )
    , mnRecentSelCol( CSV_COLUMN_INVALID )
{
    ImplClearSplits();
}

ScCsvSplits ScCsvGrid::LoadFixedWidthList()
{
    ScCsvSplits aSplits;
    load_FixedWidthList( aSplits );
    return aSplits;
}

void ScCsvGrid::SaveFixedWidthList()
{
    save_FixedWidthList( maSplits );
}

ScCsvGrid::~ScCsvGrid()
{
}

// common grid handling -------------------------------------------------------

void ScCsvGrid::ApplyPosCountChange( sal_Int32 nOldPosCount )
{
    DisableRepaint();

    if (GetPosCount() < nOldPosCount)
    {
        SelectAll(false);
        maSplits.RemoveRange(GetPosCount(), nOldPosCount);
    }
    else
        maSplits.Remove(nOldPosCount);
    maSplits.Insert(GetPosCount());
    maColStates.resize(maSplits.Count() - 1);

    EnableRepaint();
}

void ScCsvGrid::SetFirstImportedLine( sal_Int32 nLine )
{
    mnFirstImpLine = nLine;
    Repaint();
}

// split handling -------------------------------------------------------------

void ScCsvGrid::InsertSplit( sal_Int32 nPos )
{
    if( ImplInsertSplit( nPos ) )
    {
        DisableRepaint();
        Execute( CSVCMD_EXPORTCOLUMNTYPE );
        Execute( CSVCMD_UPDATECELLTEXTS );
        EnableRepaint();
    }
}

void ScCsvGrid::RemoveSplit( sal_Int32 nPos )
{
    if( ImplRemoveSplit( nPos ) )
    {
        DisableRepaint();
        Execute( CSVCMD_EXPORTCOLUMNTYPE );
        Execute( CSVCMD_UPDATECELLTEXTS );
        EnableRepaint();
    }
}

void ScCsvGrid::SetSplits( const ScCsvSplits& rSplits )
{
    DisableRepaint();
    ImplClearSplits();
    sal_uInt32 nCount = rSplits.Count();
    for( sal_uInt32 nIx = 0; nIx < nCount; ++nIx )
        maSplits.Insert( rSplits[ nIx ] );
    maColStates.clear();
    maColStates.resize( maSplits.Count() - 1 );
    Execute( CSVCMD_EXPORTCOLUMNTYPE );
    Execute( CSVCMD_UPDATECELLTEXTS );
    EnableRepaint();
}

bool ScCsvGrid::ImplInsertSplit( sal_Int32 nPos )
{
    sal_uInt32 nColIx = GetColumnFromPos( nPos );
    bool bRet = (nColIx < GetColumnCount()) && maSplits.Insert( nPos );
    if( bRet )
    {
        ScCsvColState aState( GetColumnType( nColIx ) );
        aState.Select( IsSelected( nColIx ) && IsSelected( nColIx + 1 ) );
        maColStates.insert( maColStates.begin() + nColIx + 1, aState );
    }
    return bRet;
}

bool ScCsvGrid::ImplRemoveSplit( sal_Int32 nPos )
{
    bool bRet = maSplits.Remove( nPos );
    if( bRet )
    {
        sal_uInt32 nColIx = GetColumnFromPos( nPos );
        bool bSel = IsSelected( nColIx ) || IsSelected( nColIx + 1 );
        maColStates.erase( maColStates.begin() + nColIx + 1 );
        maColStates[ nColIx ].Select( bSel );
    }
    return bRet;
}

void ScCsvGrid::ImplClearSplits()
{
    maSplits.Clear();
    maSplits.Insert( 0 );
    maSplits.Insert( GetPosCount() );
    maColStates.resize( 1 );
}

// columns/column types -------------------------------------------------------

bool ScCsvGrid::IsValidColumn( sal_uInt32 nColIndex ) const
{
    return nColIndex < GetColumnCount();
}

sal_uInt32 ScCsvGrid::GetColumnFromPos( sal_Int32 nPos ) const
{
    return maSplits.UpperBound( nPos );
}

sal_Int32 ScCsvGrid::GetColumnWidth( sal_uInt32 nColIndex ) const
{
    // coverity[ tainted_data_return : SUPPRESS ] 2024.6.1
    return IsValidColumn( nColIndex ) ? (GetColumnPos( nColIndex + 1 ) - GetColumnPos( nColIndex )) : 0;
}

void ScCsvGrid::SetColumnStates( ScCsvColStateVec&& rStates )
{
    maColStates = std::move(rStates);
    maColStates.resize( maSplits.Count() - 1 );
    Execute( CSVCMD_EXPORTCOLUMNTYPE );
}

sal_Int32 ScCsvGrid::GetColumnType( sal_uInt32 nColIndex ) const
{
    return IsValidColumn( nColIndex ) ? maColStates[ nColIndex ].mnType : CSV_TYPE_NOSELECTION;
}

void ScCsvGrid::SetColumnType( sal_uInt32 nColIndex, sal_Int32 nColType )
{
    if( IsValidColumn( nColIndex ) )
        maColStates[ nColIndex ].mnType = nColType;
}

sal_Int32 ScCsvGrid::GetSelColumnType() const
{
    sal_uInt32 nColIx = GetFirstSelected();
    if( nColIx == CSV_COLUMN_INVALID )
        return CSV_TYPE_NOSELECTION;

    sal_Int32 nType = GetColumnType( nColIx );
    while( (nColIx != CSV_COLUMN_INVALID) && (nType != CSV_TYPE_MULTI) )
    {
        if( nType != GetColumnType( nColIx ) )
            nType = CSV_TYPE_MULTI;
        nColIx = GetNextSelected( nColIx );
    }
    return nType;
}

void ScCsvGrid::SetSelColumnType( sal_Int32 nType )
{
    if( (nType != CSV_TYPE_MULTI) && (nType != CSV_TYPE_NOSELECTION) )
    {
        for( sal_uInt32 nColIx = GetFirstSelected(); nColIx != CSV_COLUMN_INVALID; nColIx = GetNextSelected( nColIx ) )
            SetColumnType( nColIx, nType );
        Repaint();
        Execute( CSVCMD_EXPORTCOLUMNTYPE );
    }
}

void ScCsvGrid::SetTypeNames( std::vector<OUString>&& rTypeNames )
{
    OSL_ENSURE( !rTypeNames.empty(), "ScCsvGrid::SetTypeNames - vector is empty" );
    maTypeNames = std::move(rTypeNames);
    Repaint();

    ::std::for_each( maColStates.begin(), maColStates.end(), Func_SetType( CSV_TYPE_DEFAULT ) );
}

OUString ScCsvGrid::GetColumnTypeName( sal_uInt32 nColIndex ) const
{
    sal_uInt32 nTypeIx = static_cast< sal_uInt32 >( GetColumnType( nColIndex ) );
    return (nTypeIx < maTypeNames.size()) ? maTypeNames[ nTypeIx ] : OUString();
}

static sal_uInt8 lcl_GetExtColumnType( sal_Int32 nIntType )
{
    static const sal_uInt8 pExtTypes[] =
        { SC_COL_STANDARD, SC_COL_TEXT, SC_COL_DMY, SC_COL_MDY, SC_COL_YMD, SC_COL_ENGLISH, SC_COL_SKIP };
    static const sal_Int32 nExtTypeCount = SAL_N_ELEMENTS(pExtTypes);
    return pExtTypes[ ((0 <= nIntType) && (nIntType < nExtTypeCount)) ? nIntType : 0 ];
}

void ScCsvGrid::FillColumnDataSep( ScAsciiOptions& rOptions ) const
{
    sal_uInt32 nCount = GetColumnCount();
    ScCsvExpDataVec aDataVec;

    for( sal_uInt32 nColIx = 0; nColIx < nCount; ++nColIx )
    {
        if( GetColumnType( nColIx ) != CSV_TYPE_DEFAULT )
            // 1-based column index
            aDataVec.emplace_back(
                static_cast< sal_Int32 >( nColIx + 1 ),
                lcl_GetExtColumnType( GetColumnType( nColIx ) ) );
    }
    rOptions.SetColumnInfo( aDataVec );
}

void ScCsvGrid::FillColumnDataFix( ScAsciiOptions& rOptions ) const
{
    sal_uInt32 nCount = std::min( GetColumnCount(), static_cast<sal_uInt32>(MAXCOLCOUNT) );
    ScCsvExpDataVec aDataVec( nCount + 1 );

    for( sal_uInt32 nColIx = 0; nColIx < nCount; ++nColIx )
    {
        ScCsvExpData& rData = aDataVec[ nColIx ];
        rData.mnIndex = GetColumnPos( nColIx );
        rData.mnType = lcl_GetExtColumnType( GetColumnType( nColIx ) );
    }
    aDataVec[ nCount ].mnIndex = SAL_MAX_INT32;
    aDataVec[ nCount ].mnType = SC_COL_SKIP;
    rOptions.SetColumnInfo( aDataVec );
}

// selection handling ---------------------------------------------------------

bool ScCsvGrid::IsSelected( sal_uInt32 nColIndex ) const
{
    return IsValidColumn( nColIndex ) && maColStates[ nColIndex ].IsSelected();
}

sal_uInt32 ScCsvGrid::GetFirstSelected() const
{
    return IsSelected( 0 ) ? 0 : GetNextSelected( 0 );
}

sal_uInt32 ScCsvGrid::GetNextSelected( sal_uInt32 nFromIndex ) const
{
    sal_uInt32 nColCount = GetColumnCount();
    for( sal_uInt32 nColIx = nFromIndex + 1; nColIx < nColCount; ++nColIx )
        if( IsSelected( nColIx ) )
            return nColIx;
    return CSV_COLUMN_INVALID;
}

void ScCsvGrid::Select( sal_uInt32 nColIndex, bool bSelect )
{
    if( IsValidColumn( nColIndex ) )
    {
        maColStates[ nColIndex ].Select( bSelect );
        Repaint();
        Execute( CSVCMD_EXPORTCOLUMNTYPE );
        if( bSelect )
            mnRecentSelCol = nColIndex;
    }
}

void ScCsvGrid::ToggleSelect( sal_uInt32 nColIndex )
{
    Select( nColIndex, !IsSelected( nColIndex ) );
}

void ScCsvGrid::SelectRange( sal_uInt32 nColIndex1, sal_uInt32 nColIndex2, bool bSelect )
{
    if( nColIndex1 == CSV_COLUMN_INVALID )
        Select( nColIndex2 );
    else if( nColIndex2 == CSV_COLUMN_INVALID )
        Select( nColIndex1 );
    else if( nColIndex1 > nColIndex2 )
    {
        SelectRange( nColIndex2, nColIndex1, bSelect );
        if( bSelect )
            mnRecentSelCol = nColIndex1;
    }
    else if( IsValidColumn( nColIndex1 ) && IsValidColumn( nColIndex2 ) )
    {
        for( sal_uInt32 nColIx = nColIndex1; nColIx <= nColIndex2; ++nColIx )
            maColStates[ nColIx ].Select( bSelect );
        Repaint();
        Execute( CSVCMD_EXPORTCOLUMNTYPE );
        if( bSelect )
            mnRecentSelCol = nColIndex1;
    }
}

void ScCsvGrid::SelectAll( bool bSelect )
{
    SelectRange( 0, GetColumnCount() - 1, bSelect );
}

void ScCsvGrid::ImplClearSelection()
{
    ::std::for_each( maColStates.begin(), maColStates.end(), Func_Select( false ) );
}

void ScCsvGrid::DoSelectAction( sal_uInt32 nColIndex, sal_uInt16 nModifier )
{
    if( !(nModifier & KEY_MOD1) )
        ImplClearSelection();
    if( nModifier & KEY_SHIFT )             // SHIFT always expands
        SelectRange( mnRecentSelCol, nColIndex );
    else if( !(nModifier & KEY_MOD1) )      // no SHIFT/CTRL always selects 1 column
        Select( nColIndex );
    else                                    // CTRL only toggles
        ToggleSelect( nColIndex );
}

// cell contents --------------------------------------------------------------

void ScCsvGrid::ImplSetTextLineSep(
        sal_Int32 nLine, const OUString& rTextLine,
        const OUString& rSepChars, sal_Unicode cTextSep, bool bMergeSep, bool bRemoveSpace )
{
    sal_uInt32 nLineIx = static_cast<sal_uInt32>(nLine);
    while( maTexts.size() <= nLineIx )
        maTexts.emplace_back( );
    std::vector<OUString>& rStrVec = maTexts[ nLineIx ];
    rStrVec.clear();

    // scan for separators
    OUString aCellText;
    const sal_Unicode* pSepChars = rSepChars.getStr();
    const sal_Unicode* pChar = rTextLine.getStr();
    sal_uInt32 nColIx = 0;

    while( *pChar && (nColIx < sal::static_int_cast<sal_uInt32>(CSV_MAXCOLCOUNT)) )
    {
        // scan for next cell text
        bool bIsQuoted = false;
        bool bOverflowCell = false;
        pChar = ScImportExport::ScanNextFieldFromString( pChar, aCellText,
                cTextSep, pSepChars, bMergeSep, bIsQuoted, bOverflowCell, bRemoveSpace );
        /* TODO: signal overflow somewhere in UI */

        // update column width
        sal_Int32 nWidth = std::max( CSV_MINCOLWIDTH, ScImportExport::CountVisualWidth( aCellText ) + 1 );
        if( IsValidColumn( nColIx ) )
        {
            // expand existing column
            sal_Int32 nDiff = nWidth - GetColumnWidth( nColIx );
            if( nDiff > 0 )
            {
                Execute( CSVCMD_SETPOSCOUNT, GetPosCount() + nDiff );
                for( sal_uInt32 nSplitIx = GetColumnCount() - 1; nSplitIx > nColIx; --nSplitIx )
                {
                    sal_Int32 nPos = maSplits[ nSplitIx ];
                    maSplits.Remove( nPos );
                    maSplits.Insert( nPos + nDiff );
                }
            }
        }
        else
        {
            // append new column
            sal_Int32 nLastPos = GetPosCount();
            Execute( CSVCMD_SETPOSCOUNT, nLastPos + nWidth );
            ImplInsertSplit( nLastPos );
        }

        if( aCellText.getLength() <= CSV_MAXSTRLEN )
            rStrVec.push_back( aCellText );
        else
            rStrVec.push_back( aCellText.copy( 0, CSV_MAXSTRLEN ) );
        ++nColIx;
    }
}

void ScCsvGrid::ImplSetTextLineFix( sal_Int32 nLine, std::u16string_view rTextLine )
{
    sal_Int32 nWidth = ScImportExport::CountVisualWidth( rTextLine );
    if( nWidth > GetPosCount() )
        Execute( CSVCMD_SETPOSCOUNT, nWidth );

    sal_uInt32 nLineIx = static_cast<sal_uInt32>(nLine);
    while( maTexts.size() <= nLineIx )
        maTexts.emplace_back( );

    std::vector<OUString>& rStrVec = maTexts[ nLineIx ];
    rStrVec.clear();
    sal_uInt32 nColCount = GetColumnCount();
    sal_Int32 nStrLen = rTextLine.size();
    sal_Int32 nStrIx = 0;
    for( sal_uInt32 nColIx = 0; (nColIx < nColCount) && (nStrIx < nStrLen); ++nColIx )
    {
        sal_Int32 nColWidth = GetColumnWidth( nColIx );
        sal_Int32 nLastIx = nStrIx;
        ScImportExport::CountVisualWidth( rTextLine, nLastIx, nColWidth );
        sal_Int32 nLen = std::min( CSV_MAXSTRLEN, nLastIx - nStrIx );
        rStrVec.push_back( OUString(rTextLine.substr( nStrIx, nLen )) );
        nStrIx = nStrIx + nLen;
    }
}

void ScCsvGrid::DumpWidgetData(tools::JsonWriter& rWriter)
{
    if (IsNoRepaint())
        return;

    const bool bFixedMode = mpTableBox && mpTableBox->IsFixedWidthMode();
    rWriter.put("isFixedMode", bFixedMode);
    rWriter.put("firstImportedLine", mnFirstImpLine);
    rWriter.put("posCount", GetPosCount());

    sal_uInt32 nColCount = static_cast<sal_uInt32>(maColStates.size());

    {
        auto aColsNode = rWriter.startArray("columns");
        for (sal_uInt32 nCol = 0; nCol < nColCount; ++nCol)
        {
            auto aColNode = rWriter.startStruct();
            rWriter.put("index", static_cast<sal_Int32>(nCol));
            rWriter.put("type", maColStates[nCol].mnType);
            rWriter.put("typeName", GetColumnTypeName(nCol));
            rWriter.put("selected", maColStates[nCol].IsSelected());
            rWriter.put("pos", GetColumnPos(nCol));
            rWriter.put("width", GetColumnWidth(nCol));
        }
    }

    {
        auto aRowsNode = rWriter.startArray("rows");
        sal_Int32 nLineCount = static_cast<sal_Int32>(maTexts.size());
        for (sal_Int32 nLineIx = 0; nLineIx < nLineCount; ++nLineIx)
        {
            auto aRowNode = rWriter.startStruct();
            rWriter.put("lineIndex", nLineIx);
            {
                auto aCellsNode = rWriter.startArray("cells");
                const std::vector<OUString>& rCells = maTexts[nLineIx];
                for (const OUString& rText : rCells)
                {
                    auto aCellNode = rWriter.startStruct();
                    rWriter.put("text", rText);
                }
            }
        }
    }

    {
        auto aTypesNode = rWriter.startArray("typeNames");
        for (const OUString& rName : maTypeNames)
        {
            rWriter.putSimpleValue(rName);
        }
    }

    {
        auto aSplitsNode = rWriter.startArray("splits");
        for (sal_uInt32 nIdx = 0; nIdx < maSplits.Count(); ++nIdx)
        {
            rWriter.putSimpleValue(OUString::number(maSplits[nIdx]));
        }
    }
}

bool ScCsvGrid::HandleCustomEvent(const OUString& rCmd, const OUString& rData)
{
    switch (parseCsvEvent(rCmd))
    {
        case CsvEvent::SelectColumn:
        {
            // data: JSON string { "column": N, "ctrl": bool, "shift": bool }
            auto oTree = parseEventJson(rData);
            if (!oTree)
                return true;
            const sal_Int32 nCol = oTree->get<sal_Int32>("column", -1);
            if (nCol < 0 || !IsValidColumn(static_cast<sal_uInt32>(nCol)))
            {
                SAL_WARN("sc.ui",
                         "ScCsvGrid: selectcolumn rejected - invalid column " << nCol);
                return true;
            }
            sal_uInt16 nModifier = 0;
            if (oTree->get<bool>("ctrl", false))
                nModifier |= KEY_MOD1;
            if (oTree->get<bool>("shift", false))
                nModifier |= KEY_SHIFT;
            DoSelectAction(static_cast<sal_uInt32>(nCol), nModifier);
            Execute(CSVCMD_EXPORTCOLUMNTYPE);
            return true;
        }
        case CsvEvent::SetColumnType:
        {
            // data: JSON string { "column": N, "type": T }
            auto oTree = parseEventJson(rData);
            if (!oTree)
                return true;
            const sal_Int32 nCol = oTree->get<sal_Int32>("column", -1);
            if (nCol < 0 || !IsValidColumn(static_cast<sal_uInt32>(nCol)))
            {
                SAL_WARN("sc.ui",
                         "ScCsvGrid: setcolumntype rejected - invalid column " << nCol);
                return true;
            }
            const sal_Int32 nType = oTree->get<sal_Int32>("type", CSV_TYPE_DEFAULT);
            if (!IsSelected(static_cast<sal_uInt32>(nCol)))
            {
                ImplClearSelection();
                Select(static_cast<sal_uInt32>(nCol));
            }
            Execute(CSVCMD_SETCOLUMNTYPE, nType);
            return true;
        }
        case CsvEvent::ToggleSplit:
        {
            if (mpTableBox && !mpTableBox->IsFixedWidthMode())
                return true;
            const sal_Int32 nPos = rData.toInt32();
            Execute(CSVCMD_TOGGLESPLIT, nPos);
            return true;
        }
        case CsvEvent::Unknown:
        default:
            SAL_WARN("sc.ui", "ScCsvGrid: unknown custom event '" << rCmd << "'");
            return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
