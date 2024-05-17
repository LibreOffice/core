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

#include <sheetdatabuffer.hxx>
#include <patterncache.hxx>

#include <algorithm>
#include <com/sun/star/sheet/XArrayFormulaTokens.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <editeng/boxitem.hxx>
#include <oox/helper/containerhelper.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <addressconverter.hxx>
#include <formulaparser.hxx>
#include <sharedstringsbuffer.hxx>
#include <unitconverter.hxx>
#include <rangelst.hxx>
#include <document.hxx>
#include <scitems.hxx>
#include <docpool.hxx>
#include <paramisc.hxx>
#include <patattr.hxx>
#include <documentimport.hxx>
#include <formulabuffer.hxx>
#include <numformat.hxx>
#include <sax/tools/converter.hxx>
#include <docuno.hxx>

namespace oox::xls {

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

CellModel::CellModel() :
    mnCellType( XML_TOKEN_INVALID ),
    mnXfId( -1 ),
    mbShowPhonetic( false )
{
}

CellFormulaModel::CellFormulaModel() :
    mnFormulaType( XML_TOKEN_INVALID ),
    mnSharedId( -1 )
{
}

bool CellFormulaModel::isValidArrayRef( const ScAddress& rCellAddr )
{
    return (maFormulaRef.aStart == rCellAddr );
}

bool CellFormulaModel::isValidSharedRef( const ScAddress& rCellAddr )
{
    return
        (maFormulaRef.aStart.Tab() == rCellAddr.Tab() ) &&
        (maFormulaRef.aStart.Col() <= rCellAddr.Col() ) && (rCellAddr.Col() <= maFormulaRef.aEnd.Col()) &&
        (maFormulaRef.aStart.Row() <= rCellAddr.Row() ) && (rCellAddr.Row() <= maFormulaRef.aEnd.Row());
}

DataTableModel::DataTableModel() :
    mb2dTable( false ),
    mbRowTable( false ),
    mbRef1Deleted( false ),
    mbRef2Deleted( false )
{
}

SheetDataBuffer::SheetDataBuffer( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper ),
    mbPendingSharedFmla( false )
{
}

void SheetDataBuffer::setBlankCell( const CellModel& rModel )
{
    setCellFormat( rModel );
}

void SheetDataBuffer::setValueCell( const CellModel& rModel, double fValue )
{
    getDocImport().setNumericCell(rModel.maCellAddr, fValue);
    setCellFormat( rModel );
}

void SheetDataBuffer::setStringCell( const CellModel& rModel, const OUString& rText )
{
    if (!rText.isEmpty())
        getDocImport().setStringCell(rModel.maCellAddr, rText);

    setCellFormat( rModel );
}

void SheetDataBuffer::setStringCell( const CellModel& rModel, const RichStringRef& rxString )
{
    OSL_ENSURE( rxString, "SheetDataBuffer::setStringCell - missing rich string object" );
    const oox::xls::Font* pFirstPortionFont = getStyles().getFontFromCellXf( rModel.mnXfId ).get();
    const Xf* pXf = getStyles().getCellXf( rModel.mnXfId ).get();
    bool bSingleLine = pXf ? !pXf->getAlignment().getModel().mbWrapText : false;
    OUString aText;
    if( rxString->extractPlainString( aText, pFirstPortionFont ) )
    {
        setStringCell( rModel, aText );
    }
    else
    {
        putRichString( rModel.maCellAddr, *rxString, pFirstPortionFont, bSingleLine );
        setCellFormat( rModel );
    }
}

void SheetDataBuffer::setStringCell( const CellModel& rModel, sal_Int32 nStringId )
{
    RichStringRef xString = getSharedStrings().getString( nStringId );
    if( xString )
        setStringCell( rModel, xString );
    else
        setBlankCell( rModel );
}

void SheetDataBuffer::setDateTimeCell( const CellModel& rModel, const css::util::DateTime& rDateTime )
{
    // write serial date/time value into the cell
    double fSerial = getUnitConverter().calcSerialFromDateTime( rDateTime );
    setValueCell( rModel, fSerial );
    // set appropriate number format
    using namespace ::com::sun::star::util::NumberFormat;
    sal_Int16 nStdFmt = (fSerial < 1.0) ? TIME : (((rDateTime.Hours > 0) || (rDateTime.Minutes > 0) || (rDateTime.Seconds > 0)) ? DATETIME : DATE);
    // set number format
    try
    {
        Reference< XNumberFormatsSupplier > xNumFmtsSupp( static_cast<cppu::OWeakObject*>(getDocument().get()), UNO_QUERY_THROW );
        Reference< XNumberFormatTypes > xNumFmtTypes( xNumFmtsSupp->getNumberFormats(), UNO_QUERY_THROW );
        sal_Int32 nIndex = xNumFmtTypes->getStandardFormat( nStdFmt, Locale() );
        PropertySet aPropSet( getCell( rModel.maCellAddr ) );
        aPropSet.setProperty( PROP_NumberFormat, nIndex );
    }
    catch( Exception& )
    {
    }
}

void SheetDataBuffer::setBooleanCell( const CellModel& rModel, bool bValue )
{
    getFormulaBuffer().setCellFormula(
        rModel.maCellAddr, bValue ? u"TRUE()"_ustr : u"FALSE()"_ustr);

    // #108770# set 'Standard' number format for all Boolean cells
    setCellFormat( rModel );
}

void SheetDataBuffer::setErrorCell( const CellModel& rModel, const OUString& rErrorCode )
{
    // Using the formula compiler now we can simply pass on the error string.
    getFormulaBuffer().setCellFormula( rModel.maCellAddr, rErrorCode);
    setCellFormat( rModel );
}

void SheetDataBuffer::setErrorCell( const CellModel& rModel, sal_uInt8 nErrorCode )
{
    setErrorCell( rModel, getUnitConverter().calcErrorString( nErrorCode));
}

void SheetDataBuffer::setDateCell( const CellModel& rModel, const OUString& rDateString )
{
    css::util::DateTime aDateTime;
    if (!sax::Converter::parseDateTime( aDateTime, rDateString))
    {
        SAL_WARN("sc.filter", "SheetDataBuffer::setDateCell - could not parse: " << rDateString);
        // At least don't lose data.
        setStringCell( rModel, rDateString);
        return;
    }

    double fSerial = getUnitConverter().calcSerialFromDateTime( aDateTime);
    setValueCell( rModel, fSerial);
}

void SheetDataBuffer::createSharedFormula(const ScAddress& rAddr, const ApiTokenSequence& rTokens)
{
    BinAddress aAddr(rAddr);
    maSharedFormulas[aAddr] = rTokens;
    if( mbPendingSharedFmla )
        setCellFormula( maSharedFmlaAddr, resolveSharedFormula( maSharedBaseAddr ) );
}

void SheetDataBuffer::setFormulaCell( const CellModel& rModel, const ApiTokenSequence& rTokens )
{
    mbPendingSharedFmla = false;
    ApiTokenSequence aTokens;

    /*  Detect special token passed as placeholder for array formulas, shared
        formulas, and table operations. In BIFF, these formulas are represented
        by a single tExp resp. tTbl token. If the formula parser finds these
        tokens, it puts a single OPCODE_BAD token with the base address and
        formula type into the token sequence. This information will be
        extracted here, and in case of a shared formula, the shared formula
        buffer will generate the resulting formula token array. */
    ApiSpecialTokenInfo aTokenInfo;
    if( rTokens.hasElements() && getFormulaParser().extractSpecialTokenInfo( aTokenInfo, rTokens ) )
    {
        /*  The second member of the token info is set to true, if the formula
            represents a table operation, which will be skipped. In BIFF12 it
            is not possible to distinguish array and shared formulas
            (BIFF5/BIFF8 provide this information with a special flag in the
            FORMULA record). */
        if( !aTokenInfo.Second )
        {
            /*  Construct the token array representing the shared formula. If
                the returned sequence is empty, the definition of the shared
                formula has not been loaded yet, or the cell is part of an
                array formula. In this case, the cell will be remembered. After
                reading the formula definition it will be retried to insert the
                formula via retryPendingSharedFormulaCell(). */
            ScAddress aTokenAddr( aTokenInfo.First.Column, aTokenInfo.First.Row, aTokenInfo.First.Sheet );
            aTokens = resolveSharedFormula( aTokenAddr );
            if( !aTokens.hasElements() )
            {
                maSharedFmlaAddr = rModel.maCellAddr;
                maSharedBaseAddr = aTokenAddr;
                mbPendingSharedFmla = true;
            }
        }
    }
    else
    {
        // simple formula, use the passed token array
        aTokens = rTokens;
    }

    setCellFormula( rModel.maCellAddr, aTokens );
    setCellFormat( rModel );
}

void SheetDataBuffer::createArrayFormula( const ScRange& rRange, const ApiTokenSequence& rTokens )
{
    /*  Array formulas will be inserted later in finalizeImport(). This is
        needed to not disturb collecting all the cells, which will be put into
        the sheet in large blocks to increase performance. */
    maArrayFormulas.emplace_back( rRange, rTokens );
}

void SheetDataBuffer::createTableOperation( const ScRange& rRange, const DataTableModel& rModel )
{
    /*  Table operations will be inserted later in finalizeImport(). This is
        needed to not disturb collecting all the cells, which will be put into
        the sheet in large blocks to increase performance. */
    maTableOperations.emplace_back( rRange, rModel );
}

void SheetDataBuffer::setRowFormat( sal_Int32 nRow, sal_Int32 nXfId, bool bCustomFormat )
{
    // set row formatting
    if( bCustomFormat )
    {
        // try to expand cached row range, if formatting is equal
        if( (maXfIdRowRange.maRowRange.mnLast < 0) || !maXfIdRowRange.tryExpand( nRow, nXfId ) )
        {

            maXfIdRowRangeList[ maXfIdRowRange.mnXfId ].push_back( maXfIdRowRange.maRowRange );
            maXfIdRowRange.set( nRow, nXfId );
        }
    }
    else if( maXfIdRowRange.maRowRange.mnLast >= 0 )
    {
        // finish last cached row range
        maXfIdRowRangeList[ maXfIdRowRange.mnXfId ].push_back( maXfIdRowRange.maRowRange );
        maXfIdRowRange.set( -1, -1 );
    }
}

void SheetDataBuffer::setMergedRange( const ScRange& rRange )
{
    maMergedRanges.emplace_back( rRange );
}

typedef std::pair<sal_Int32, sal_Int32> FormatKeyPair;

static void addIfNotInMyMap( const StylesBuffer& rStyles, std::map< FormatKeyPair, ScRangeList >& rMap, sal_Int32 nXfId, sal_Int32 nFormatId, const ScRangeList& rRangeList )
{
    Xf* pXf1 = rStyles.getCellXf( nXfId ).get();
    if ( !pXf1 )
        return;

    auto it = std::find_if(rMap.begin(), rMap.end(),
        [&nFormatId, &rStyles, &pXf1](const std::pair<FormatKeyPair, ScRangeList>& rEntry) {
            if (rEntry.first.second != nFormatId)
                return false;
            Xf* pXf2 = rStyles.getCellXf( rEntry.first.first ).get();
            return *pXf1 == *pXf2;
        });
    if (it != rMap.end()) // already exists
    {
        // add ranges from the rangelist to the existing rangelist for the
        // matching style ( should we check if they overlap ? )
        it->second.insert(it->second.end(), rRangeList.begin(), rRangeList.end());
        return;
    }
    rMap[ FormatKeyPair( nXfId, nFormatId ) ] = rRangeList;
}

void SheetDataBuffer::addColXfStyles()
{
    std::map< FormatKeyPair, ScRangeList > rangeStyleListMap;
    for( const auto& [rFormatKeyPair, rRangeList] : maXfIdRangeLists )
    {
        addIfNotInMyMap( getStyles(), rangeStyleListMap, rFormatKeyPair.first, rFormatKeyPair.second, rRangeList );
    }
    // gather all ranges that have the same style and apply them in bulk
    // Collect data in unsorted vectors and sort them just once at the end
    // instead of possibly slow repeated inserts.
    TmpColStyles tmpStylesPerColumn;
    for ( const auto& [rFormatKeyPair, rRanges] : rangeStyleListMap )
    {
        for (const ScRange & rAddress : rRanges)
        {
            RowRangeStyle aStyleRows;
            aStyleRows.mnNumFmt.first = rFormatKeyPair.first;
            aStyleRows.mnNumFmt.second = rFormatKeyPair.second;
            aStyleRows.mnStartRow = rAddress.aStart.Row();
            aStyleRows.mnEndRow = rAddress.aEnd.Row();
            for ( sal_Int32 nCol = rAddress.aStart.Col(); nCol <= rAddress.aEnd.Col(); ++nCol )
               tmpStylesPerColumn[ nCol ].push_back( aStyleRows );
        }
    }
    for( auto& rowStyles : tmpStylesPerColumn )
    {
        TmpRowStyles& s = rowStyles.second;
        std::sort( s.begin(), s.end(), StyleRowRangeComp());
        s.erase( std::unique( s.begin(), s.end(),
                    [](const RowRangeStyle& lhs, const RowRangeStyle& rhs)
                        // Synthetize operator== from operator < . Do not create an actual operator==
                        // as operator< is somewhat specific (see StyleRowRangeComp).
                        { return !StyleRowRangeComp()(lhs,rhs) && !StyleRowRangeComp()(rhs,lhs); } ),
            s.end());
        // Broken documents may have overlapping ranges that cause problems, repeat once more.
        if(!std::is_sorted(s.begin(), s.end(), StyleRowRangeComp()))
        {
            std::sort( s.begin(), s.end(), StyleRowRangeComp());
            s.erase( std::unique( s.begin(), s.end(),
                        [](const RowRangeStyle& lhs, const RowRangeStyle& rhs)
                            { return !StyleRowRangeComp()(lhs,rhs) && !StyleRowRangeComp()(rhs,lhs); } ),
                s.end());
        }
        maStylesPerColumn[ rowStyles.first ].insert_sorted_unique_vector( std::move( s ));
    }
}

void SheetDataBuffer::addColXfStyleProcessRowRanges()
{
    // count the number of row-range-styles we have
    AddressConverter& rAddrConv = getAddressConverter();
    int cnt = 0;
    for ( const auto& [nXfId, rRowRangeList] : maXfIdRowRangeList )
    {
        if ( nXfId == -1 ) // it's a dud skip it
            continue;
        cnt += rRowRangeList.size();
    }
    // pre-allocate space in the sorted_vector
    for ( sal_Int32 nCol = 0; nCol <= rAddrConv.getMaxApiAddress().Col(); ++nCol )
    {
       RowStyles& rRowStyles = maStylesPerColumn[ nCol ];
       rRowStyles.reserve(rRowStyles.size() + cnt);
    }
    const auto nMaxCol = rAddrConv.getMaxApiAddress().Col();
    for ( sal_Int32 nCol = 0; nCol <= nMaxCol; ++nCol )
    {
        RowStyles& rRowStyles = maStylesPerColumn[ nCol ];
        for ( auto& [nXfId, rRowRangeList] : maXfIdRowRangeList )
        {
            if ( nXfId == -1 ) // it's a dud skip it
                continue;
            // sort the row ranges, so we spend less time moving data around
            // when we insert into aStyleRows
            std::sort(rRowRangeList.begin(), rRowRangeList.end(),
                [](const ValueRange& lhs, const ValueRange& rhs)
                {
                    return lhs.mnFirst < rhs.mnFirst;
                });
            // get all row ranges for id
            for ( const auto& rRange : rRowRangeList )
            {
                RowRangeStyle aStyleRows;
                aStyleRows.mnNumFmt.first = nXfId;
                aStyleRows.mnNumFmt.second = -1;

                // Reset row range for each column
                aStyleRows.mnStartRow = rRange.mnFirst;
                aStyleRows.mnEndRow = rRange.mnLast;

                // If aStyleRows includes rows already allocated to a style
                // in rRowStyles, then we need to split it into parts.
                // ( to occupy only rows that have no style definition)

                // Start iterating at the first element that is not completely before aStyleRows
                RowStyles::const_iterator rows_it = rRowStyles.lower_bound(aStyleRows);
                bool bAddRange = true;
                for ( ; rows_it != rRowStyles.end(); ++rows_it )
                {
                    // Add the part of aStyleRows that does not overlap with r
                    if ( aStyleRows.mnStartRow < rows_it->mnStartRow )
                    {
                        RowRangeStyle aSplit = aStyleRows;
                        aSplit.mnEndRow = std::min(aStyleRows.mnEndRow, rows_it->mnStartRow - 1);
                        rows_it = rRowStyles.insert( aSplit ).first;
                    }

                    // Done if no part of aStyleRows extends beyond r
                    if ( aStyleRows.mnEndRow <= rows_it->mnEndRow )
                    {
                        bAddRange = false;
                        break;
                    }

                    // Cut off the part aStyleRows that was handled above
                    aStyleRows.mnStartRow = rows_it->mnEndRow + 1;
                }
                if ( bAddRange )
                    rRowStyles.insert( aStyleRows );
            }
        }
    }
}

void SheetDataBuffer::finalizeImport()
{
    ScDocumentImport& rDocImport = getDocImport();

    SCTAB nStartTabInvalidatedIters(SCTAB_MAX);
    SCTAB nEndTabInvalidatedIters(0);

    // create all array formulas
    for( const auto& [rRange, rTokens] : maArrayFormulas )
    {
        finalizeArrayFormula(rRange, rTokens);

        nStartTabInvalidatedIters = std::min(rRange.aStart.Tab(), nStartTabInvalidatedIters);
        nEndTabInvalidatedIters = std::max(rRange.aEnd.Tab(), nEndTabInvalidatedIters);
    }

    for (SCTAB nTab = nStartTabInvalidatedIters; nTab <= nEndTabInvalidatedIters; ++nTab)
        rDocImport.invalidateBlockPositionSet(nTab);

    // create all table operations
    for( const auto& [rRange, rModel] : maTableOperations )
        finalizeTableOperation( rRange, rModel );

    // write default formatting of remaining row range
    maXfIdRowRangeList[ maXfIdRowRange.mnXfId ].push_back( maXfIdRowRange.maRowRange );

    addColXfStyles();

    addColXfStyleProcessRowRanges();

    ScDocument& rDoc = rDocImport.getDoc();
    StylesBuffer& rStyles = getStyles();
    ScDocumentImport::Attrs aPendingAttrParam;
    SCCOL pendingColStart = -1;
    SCCOL pendingColEnd = -1;
    ScPatternCache aPatternCache;

    for ( const auto& [rCol, rRowStyles] : maStylesPerColumn )
    {
        SCCOL nScCol = static_cast< SCCOL >( rCol );

        // tdf#91567 Get pattern from the first row without AutoFilter
        const ScPatternAttr* pDefPattern = nullptr;
        bool bAutoFilter = true;
        SCROW nScRow = 0;
        while ( bAutoFilter && nScRow < rDoc.MaxRow() )
        {
            pDefPattern = rDoc.GetPattern( nScCol, nScRow, getSheetIndex() );
            if ( pDefPattern )
            {
                const ScMergeFlagAttr* pAttr = pDefPattern->GetItemSet().GetItem( ATTR_MERGE_FLAG );
                bAutoFilter = pAttr->HasAutoFilter();
            }
            else
                break;
            nScRow++;
        }
        if ( !pDefPattern || nScRow == rDoc.MaxRow() )
            pDefPattern = &rDoc.getCellAttributeHelper().getDefaultCellAttribute();

        Xf::AttrList aAttrs(pDefPattern);
        for ( const auto& rRowStyle : rRowStyles )
        {
             Xf* pXf = rStyles.getCellXf( rRowStyle.mnNumFmt.first ).get();

             if ( pXf )
                 pXf->applyPatternToAttrList( aAttrs, rRowStyle.mnStartRow, rRowStyle.mnEndRow,
                    rRowStyle.mnNumFmt.first, rRowStyle.mnNumFmt.second, aPatternCache );
        }
        if (aAttrs.maAttrs.empty() || aAttrs.maAttrs.back().nEndRow != rDoc.MaxRow())
        {
            ScAttrEntry aEntry;
            aEntry.nEndRow = rDoc.MaxRow();
            aEntry.setScPatternAttr(pDefPattern, false);
            aAttrs.maAttrs.push_back(aEntry);

            if (!sc::NumFmtUtil::isLatinScript(*aEntry.getScPatternAttr(), rDoc))
                aAttrs.mbLatinNumFmtOnly = false;
        }

        ScDocumentImport::Attrs aAttrParam;
        aAttrParam.mvData.swap(aAttrs.maAttrs);
        aAttrParam.mbLatinNumFmtOnly = aAttrs.mbLatinNumFmtOnly;

        // Compress setting the attributes, set the same set in one call.
        if( pendingColStart != -1 && pendingColEnd == nScCol - 1 && aAttrParam == aPendingAttrParam )
            ++pendingColEnd;
        else
        {
            if( pendingColStart != -1 )
                rDocImport.setAttrEntries(getSheetIndex(), pendingColStart, pendingColEnd, std::move(aPendingAttrParam));
            pendingColStart = pendingColEnd = nScCol;
            aPendingAttrParam = std::move( aAttrParam );
        }
    }
    if( pendingColStart != -1 )
        rDocImport.setAttrEntries(getSheetIndex(), pendingColStart, pendingColEnd, std::move(aPendingAttrParam));

    // merge all cached merged ranges and update right/bottom cell borders
    for( const auto& rMergedRange : maMergedRanges )
        applyCellMerging( rMergedRange.maRange );
    for( const auto& rCenterFillRange : maCenterFillRanges )
        applyCellMerging( rCenterFillRange.maRange );
}

// private --------------------------------------------------------------------

SheetDataBuffer::XfIdRowRange::XfIdRowRange() :
    maRowRange( -1 ),
    mnXfId( -1 )
{
}

void SheetDataBuffer::XfIdRowRange::set( sal_Int32 nRow, sal_Int32 nXfId )
{
    maRowRange = ValueRange( nRow );
    mnXfId = nXfId;
}

bool SheetDataBuffer::XfIdRowRange::tryExpand( sal_Int32 nRow, sal_Int32 nXfId )
{
    if( mnXfId == nXfId )
    {
        if( maRowRange.mnLast + 1 == nRow )
        {
            ++maRowRange.mnLast;
            return true;
        }
        if( maRowRange.mnFirst == nRow + 1 )
        {
            --maRowRange.mnFirst;
            return true;
        }
    }
    return false;
}

SheetDataBuffer::MergedRange::MergedRange( const ScRange& rRange ) :
    maRange( rRange ),
    mnHorAlign( XML_TOKEN_INVALID )
{
}

SheetDataBuffer::MergedRange::MergedRange( const ScAddress& rAddress, sal_Int32 nHorAlign ) :
    maRange( rAddress, rAddress ),
    mnHorAlign( nHorAlign )
{
}

bool SheetDataBuffer::MergedRange::tryExpand( const ScAddress& rAddress, sal_Int32 nHorAlign )
{
    if( (mnHorAlign == nHorAlign) && (maRange.aStart.Row() == rAddress.Row() ) &&
        (maRange.aEnd.Row() == rAddress.Row() ) && (maRange.aEnd.Col() + 1 == rAddress.Col() ) )
    {
        maRange.aEnd.IncCol();
        return true;
    }
    return false;
}

void SheetDataBuffer::setCellFormula( const ScAddress& rCellAddr, const ApiTokenSequence& rTokens )
{
    if( rTokens.hasElements() )
    {
        putFormulaTokens( rCellAddr, rTokens );
    }
}


ApiTokenSequence SheetDataBuffer::resolveSharedFormula( const ScAddress& rAddr ) const
{
    BinAddress aAddr(rAddr);
    ApiTokenSequence aTokens = ContainerHelper::getMapElement( maSharedFormulas, aAddr, ApiTokenSequence() );
    return aTokens;
}

void SheetDataBuffer::finalizeArrayFormula( const ScRange& rRange, const ApiTokenSequence& rTokens ) const
{
    Reference< XArrayFormulaTokens > xTokens( getCellRange( rRange ), UNO_QUERY );
    OSL_ENSURE( xTokens.is(), "SheetDataBuffer::finalizeArrayFormula - missing formula token interface" );
    if( xTokens.is() )
        xTokens->setArrayTokens( rTokens );
}

void SheetDataBuffer::finalizeTableOperation( const ScRange& rRange, const DataTableModel& rModel )
{
    if (rModel.mbRef1Deleted)
        return;

    if (rModel.maRef1.isEmpty())
        return;

    if (rRange.aStart.Col() <= 0 || rRange.aStart.Row() <= 0)
        return;

    sal_Int16 nSheet = getSheetIndex();

    ScAddress aRef1( 0, 0, 0 );
    if (!getAddressConverter().convertToCellAddress(aRef1, rModel.maRef1, nSheet, true))
        return;

    ScDocumentImport& rDoc = getDocImport();
    ScTabOpParam aParam;

    ScRange aScRange(rRange);

    if (rModel.mb2dTable)
    {
        // Two-variable data table.
        if (rModel.mbRef2Deleted)
            return;

        if (rModel.maRef2.isEmpty())
            return;

        ScAddress aRef2( 0, 0, 0 );
        if (!getAddressConverter().convertToCellAddress(aRef2, rModel.maRef2, nSheet, true))
            return;

        aParam.meMode = ScTabOpParam::Both;

        aScRange.aStart.IncCol(-1);
        aScRange.aStart.IncRow(-1);

        aParam.aRefFormulaCell.Set(aScRange.aStart.Col(), aScRange.aStart.Row(), nSheet, false, false, false);
        aParam.aRefFormulaEnd = aParam.aRefFormulaCell;

        // Ref1 is row input cell and Ref2 is column input cell.
        aParam.aRefRowCell.Set(aRef1.Col(), aRef1.Row(), aRef1.Tab(), false, false, false);
        aParam.aRefColCell.Set(aRef2.Col(), aRef2.Row(), aRef2.Tab(), false, false, false);
        rDoc.setTableOpCells(aScRange, aParam);

        return;
    }

    // One-variable data table.

    if (rModel.mbRowTable)
    {
        // One-variable row input cell (horizontal).
        aParam.meMode = ScTabOpParam::Row;
        aParam.aRefRowCell.Set(aRef1.Col(), aRef1.Row(), aRef1.Tab(), false, false, false);
        aParam.aRefFormulaCell.Set(rRange.aStart.Col()-1, rRange.aStart.Row(), nSheet, false, true, false);
        aParam.aRefFormulaEnd = aParam.aRefFormulaCell;
        aScRange.aStart.IncRow(-1);
        rDoc.setTableOpCells(aScRange, aParam);
    }
    else
    {
        // One-variable column input cell (vertical).
        aParam.meMode = ScTabOpParam::Column;
        aParam.aRefColCell.Set(aRef1.Col(), aRef1.Row(), aRef1.Tab(), false, false, false);
        aParam.aRefFormulaCell.Set(rRange.aStart.Col(), rRange.aStart.Row()-1, nSheet, true, false, false);
        aParam.aRefFormulaEnd = aParam.aRefFormulaCell;
        aScRange.aStart.IncCol(-1);
        rDoc.setTableOpCells(aScRange, aParam);
    }
}

void SheetDataBuffer::setCellFormat( const CellModel& rModel )
{
    if( rModel.mnXfId < 0 )
        return;

    ScRangeList& rRangeList = maXfIdRangeLists[ XfIdNumFmtKey( rModel.mnXfId, -1 ) ];
    ScRange* pLastRange = rRangeList.empty() ? nullptr : &rRangeList.back();
    /* The xlsx sheet data contains row wise information.
     * It is sufficient to check if the row range size is one
     */
    if (!rRangeList.empty() &&
        *pLastRange == rModel.maCellAddr)
        ; // do nothing - this probably bad data
    else if (!rRangeList.empty() &&
        pLastRange->aStart.Tab() == rModel.maCellAddr.Tab() &&
        pLastRange->aStart.Row() == pLastRange->aEnd.Row() &&
        pLastRange->aStart.Row() == rModel.maCellAddr.Row() &&
        pLastRange->aEnd.Col() + 1 == rModel.maCellAddr.Col())
    {
        pLastRange->aEnd.IncCol();       // Expand Column
    }
    else
    {
        rRangeList.push_back(ScRange(rModel.maCellAddr));
        pLastRange = &rRangeList.back();
    }

    if (rRangeList.size() > 1)
    {
        for (size_t i = rRangeList.size() - 1; i != 0; --i)
        {
            ScRange& rMergeRange = rRangeList[i - 1];
            if (pLastRange->aStart.Tab() != rMergeRange.aStart.Tab())
                break;

            /* Try to merge this with the previous range */
            if (pLastRange->aStart.Row() == (rMergeRange.aEnd.Row() + 1) &&
                pLastRange->aStart.Col() == rMergeRange.aStart.Col() &&
                pLastRange->aEnd.Col() == rMergeRange.aEnd.Col())
            {
                rMergeRange.aEnd.SetRow(pLastRange->aEnd.Row());
                rRangeList.Remove(rRangeList.size() - 1);
                break;
            }
            else if (pLastRange->aStart.Row() > (rMergeRange.aEnd.Row() + 1))
                break; // Un-necessary to check with any other rows
        }
    }
    // update merged ranges for 'center across selection' and 'fill'
    const Xf* pXf = getStyles().getCellXf( rModel.mnXfId ).get();
    if( !pXf )
        return;

    sal_Int32 nHorAlign = pXf->getAlignment().getModel().mnHorAlign;
    if( (nHorAlign == XML_centerContinuous) || (nHorAlign == XML_fill) )
    {
        /*  start new merged range, if cell is not empty (#108781#),
            or try to expand last range with empty cell */
        if( rModel.mnCellType != XML_TOKEN_INVALID )
            maCenterFillRanges.emplace_back( rModel.maCellAddr, nHorAlign );
        else if( !maCenterFillRanges.empty() )
            maCenterFillRanges.rbegin()->tryExpand( rModel.maCellAddr, nHorAlign );
    }
}

static void lcl_SetBorderLine( ScDocument& rDoc, const ScRange& rRange, SCTAB nScTab, SvxBoxItemLine nLine )
{
    SCCOL nFromScCol = (nLine == SvxBoxItemLine::RIGHT) ? rRange.aEnd.Col() : rRange.aStart.Col();
    SCROW nFromScRow = (nLine == SvxBoxItemLine::BOTTOM) ? rRange.aEnd.Row() : rRange.aStart.Row();

    const SvxBoxItem* pFromItem =
        rDoc.GetAttr( nFromScCol, nFromScRow, nScTab, ATTR_BORDER );
    const SvxBoxItem* pToItem =
        rDoc.GetAttr( rRange.aStart.Col(), rRange.aStart.Row(), nScTab, ATTR_BORDER );

    SvxBoxItem aNewItem( *pToItem );
    aNewItem.SetLine( pFromItem->GetLine( nLine ), nLine );
    rDoc.ApplyAttr( rRange.aStart.Col(), rRange.aStart.Row(), nScTab, aNewItem );
}

void SheetDataBuffer::applyCellMerging( const ScRange& rRange )
{
    bool bMultiCol = rRange.aStart.Col() < rRange.aEnd.Col();
    bool bMultiRow = rRange.aStart.Row() < rRange.aEnd.Row();

    const ScAddress& rStart = rRange.aStart;
    const ScAddress& rEnd = rRange.aEnd;
    ScDocument& rDoc = getScDocument();
    // set correct right border
    if( bMultiCol )
        lcl_SetBorderLine( rDoc, rRange, getSheetIndex(), SvxBoxItemLine::RIGHT );
        // set correct lower border
    if( bMultiRow )
        lcl_SetBorderLine( rDoc, rRange, getSheetIndex(), SvxBoxItemLine::BOTTOM );
    // do merge
    if( bMultiCol || bMultiRow )
        rDoc.DoMerge( rStart.Col(), rStart.Row(), rEnd.Col(), rEnd.Row(), getSheetIndex() );
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
