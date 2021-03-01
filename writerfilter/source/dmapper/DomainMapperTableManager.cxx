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
#include <optional>
#include "DomainMapperTableManager.hxx"
#include "ConversionHelper.hxx"
#include "MeasureHandler.hxx"
#include "TagLogger.hxx"
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <o3tl/numeric.hxx>
#include <o3tl/safeint.hxx>
#include <ooxml/resourceids.hxx>
#include <rtl/math.hxx>
#include <sal/log.hxx>
#include <numeric>

namespace writerfilter::dmapper {

using namespace ::com::sun::star;
using namespace ::std;

DomainMapperTableManager::DomainMapperTableManager() :
    m_nRow(0),
    m_nCell(),
    m_nGridSpan(1),
    m_nHeaderRepeat(0),
    m_nTableWidth(0),
    m_bIsInShape(false),
    m_aTmpPosition(),
    m_aTmpTableProperties(),
    m_bPushCurrentWidth(false),
    m_bTableSizeTypeInserted(false),
    m_nLayoutType(0),
    m_aParagraphsToEndTable(),
    m_pTablePropsHandler(new TablePropertiesHandler())
{
    m_pTablePropsHandler->SetTableManager( this );
}


DomainMapperTableManager::~DomainMapperTableManager()
{
}

bool DomainMapperTableManager::attribute(Id nName, Value const & rValue)
{
    bool bRet = true;

    switch (nName)
    {
    case NS_ooxml::LN_CT_TblLook_val:
    {
        TablePropertyMapPtr pPropMap(new TablePropertyMap());
        pPropMap->Insert(PROP_TBL_LOOK, uno::makeAny<sal_Int32>(rValue.getInt()));
        insertTableProps(pPropMap);
        m_aTableLook["val"] <<= static_cast<sal_Int32>(rValue.getInt());
    }
    break;
    case NS_ooxml::LN_CT_TblLook_noVBand:
        m_aTableLook["noVBand"] <<= static_cast<sal_Int32>(rValue.getInt());
    break;
    case NS_ooxml::LN_CT_TblLook_noHBand:
        m_aTableLook["noHBand"] <<= static_cast<sal_Int32>(rValue.getInt());
    break;
    case NS_ooxml::LN_CT_TblLook_lastColumn:
        m_aTableLook["lastColumn"] <<= static_cast<sal_Int32>(rValue.getInt());
    break;
    case NS_ooxml::LN_CT_TblLook_lastRow:
        m_aTableLook["lastRow"] <<= static_cast<sal_Int32>(rValue.getInt());
    break;
    case NS_ooxml::LN_CT_TblLook_firstColumn:
        m_aTableLook["firstColumn"] <<= static_cast<sal_Int32>(rValue.getInt());
    break;
    case NS_ooxml::LN_CT_TblLook_firstRow:
        m_aTableLook["firstRow"] <<= static_cast<sal_Int32>(rValue.getInt());
    break;
    default:
        bRet = false;
    }

    return bRet;
}

void DomainMapperTableManager::finishTableLook()
{
    TablePropertyMapPtr pPropMap(new TablePropertyMap());
    pPropMap->Insert(META_PROP_TABLE_LOOK, uno::makeAny(m_aTableLook.getAsConstPropertyValueList()));
    m_aTableLook.clear();
    insertTableProps(pPropMap);
}

bool DomainMapperTableManager::sprm(Sprm & rSprm)
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("tablemanager.sprm");
    string sSprm = rSprm.toString();
    TagLogger::getInstance().chars(sSprm);
    TagLogger::getInstance().endElement();
#endif
    bool bRet = TableManager::sprm(rSprm);
    if( !bRet )
    {
        bRet = m_pTablePropsHandler->sprm( rSprm );
    }

    if ( !bRet )
    {
        bRet = true;
        sal_uInt32 nSprmId = rSprm.getId();
        Value::Pointer_t pValue = rSprm.getValue();
        sal_Int32 nIntValue = (pValue ? pValue->getInt() : 0);
        switch ( nSprmId )
        {
            case NS_ooxml::LN_CT_TblPrBase_tblW:
            case NS_ooxml::LN_CT_TblPrBase_tblInd:
            {
                //contains unit and value
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties )
                {
                    MeasureHandlerPtr pMeasureHandler( new MeasureHandler );
                    pProperties->resolve(*pMeasureHandler);
                    TablePropertyMapPtr pPropMap( new TablePropertyMap );
                    if (nSprmId == sal_uInt32(NS_ooxml::LN_CT_TblPrBase_tblInd))
                    {
                        pPropMap->setValue( TablePropertyMap::LEFT_MARGIN, pMeasureHandler->getMeasureValue() );
                    }
                    else
                    {
                        m_nTableWidth = pMeasureHandler->getMeasureValue();
                        if( m_nTableWidth )
                        {
                            pPropMap->setValue( TablePropertyMap::TABLE_WIDTH_TYPE, text::SizeType::FIX );
                            pPropMap->setValue( TablePropertyMap::TABLE_WIDTH, m_nTableWidth );
                            m_bTableSizeTypeInserted = true;
                        }
                        else if( sal::static_int_cast<Id>(pMeasureHandler->getUnit()) == NS_ooxml::LN_Value_ST_TblWidth_pct )
                        {
                            sal_Int32 nPercent = pMeasureHandler->getValue() / 50;
                            if(nPercent > 100)
                                nPercent = 100;
                            pPropMap->setValue( TablePropertyMap::TABLE_WIDTH_TYPE, text::SizeType::VARIABLE );
                            pPropMap->setValue( TablePropertyMap::TABLE_WIDTH, nPercent );
                            m_bTableSizeTypeInserted = true;
                        }
                        else if( sal::static_int_cast<Id>(pMeasureHandler->getUnit()) == NS_ooxml::LN_Value_ST_TblWidth_auto )
                        {
                            /*
                            This attribute specifies the width type of table. This is used as part of the table layout
                            algorithm specified by the tblLayout element.(See 17.4.64 and 17.4.65 of the ISO/IEC 29500-1:2011.)
                            If this value is 'auto', the table layout has to use the preferred widths on the table items to generate
                            the final sizing of the table, but then must use the contents of each cell to determine final column widths.
                            (See 17.18.87 of the ISO/IEC 29500-1:2011.)
                            */
                            IntVectorPtr pCellWidths = getCurrentCellWidths();
                            // Check whether all cells have fixed widths in the given row of table.
                            bool bFixed = std::find(pCellWidths->begin(), pCellWidths->end(), -1) == pCellWidths->end();
                            if (!bFixed)
                            {
                                // Set the width type of table with 'Auto' and set the width value to 0 (as per grid values)
                                pPropMap->setValue( TablePropertyMap::TABLE_WIDTH_TYPE, text::SizeType::VARIABLE );
                                pPropMap->setValue( TablePropertyMap::TABLE_WIDTH, 0 );
                                m_bTableSizeTypeInserted = true;
                            }
                            else if (getTableDepth() > 1)
                            {
                                // tdf#131819 limiting the fix for nested tables temporarily
                                // TODO revert the fix for tdf#104876 and reopen it
                                m_bTableSizeTypeInserted = true;
                            }
                        }
                    }
#ifdef DBG_UTIL
                    pPropMap->dumpXml();
#endif
                    insertTableProps(pPropMap);
                }
            }
            break;
            case NS_ooxml::LN_CT_TrPrBase_tblHeader:
                // if nIntValue == 1 then the row is a repeated header line
                // to prevent later rows from increasing the repeating m_nHeaderRepeat is set to NULL when repeating stops
                if( nIntValue > 0 && m_nHeaderRepeat == static_cast<int>(m_nRow) )
                {
                    TablePropertyMapPtr pPropMap( new TablePropertyMap );

                    // FIXME: DOCX tables with more than 10 repeating header lines imported
                    // without repeating header lines to mimic an MSO workaround for its usability bug.
                    // Explanation: it's very hard to set and modify repeating header rows in Word,
                    // often resulting tables with a special workaround: setting all table rows as
                    // repeating header, because exceeding the pages by "unlimited" header rows turns off the
                    // table headers automatically in MSO. 10-row limit is a reasonable temporary limit
                    // to handle DOCX tables with "unlimited" repeating header, till the same "turn off
                    // exceeding header" feature is ready (see tdf#88496).
#define HEADER_ROW_LIMIT_FOR_MSO_WORKAROUND 10
                    if ( m_nHeaderRepeat == HEADER_ROW_LIMIT_FOR_MSO_WORKAROUND )
                    {
                        m_nHeaderRepeat = -1;
                        pPropMap->Insert( PROP_HEADER_ROW_COUNT, uno::makeAny(sal_Int32(0)));
                    }
                    else
                    {
                        ++m_nHeaderRepeat;
                        pPropMap->Insert( PROP_HEADER_ROW_COUNT, uno::makeAny( m_nHeaderRepeat ));
                    }
                    insertTableProps(pPropMap);
                }
                else
                {
                    if ( nIntValue == 0 && m_nRow == 0 )
                    {
                        // explicit tblHeader=0 in the first row must overwrite table style
                        TablePropertyMapPtr pPropMap( new TablePropertyMap );
                        pPropMap->Insert( PROP_HEADER_ROW_COUNT, uno::makeAny(sal_Int32(0)));
                        insertTableProps(pPropMap);
                    }
                    m_nHeaderRepeat = -1;
                }
                if (nIntValue)
                {
                    // Store the info that this is a header, we'll need that when we apply table styles.
                    TablePropertyMapPtr pPropMap( new TablePropertyMap );
                    pPropMap->Insert( PROP_TBL_HEADER, uno::makeAny(nIntValue));
                    insertRowProps(pPropMap);
                }
            break;
            case NS_ooxml::LN_CT_TblPrBase_tblStyle: //table style name
            {
                TablePropertyMapPtr pPropMap( new TablePropertyMap );
                pPropMap->Insert( META_PROP_TABLE_STYLE_NAME, uno::makeAny( pValue->getString() ));
                insertTableProps(pPropMap);
            }
            break;
            case NS_ooxml::LN_CT_TblGridBase_gridCol:
            {
                if (nIntValue == -1)
                    getCurrentGrid()->clear();
                else
                    getCurrentGrid()->push_back( nIntValue );
            }
            break;
            case NS_ooxml::LN_CT_TcPrBase_vMerge : //vertical merge
            {
                // values can be: LN_Value_ST_Merge_restart, LN_Value_ST_Merge_continue, in reality the second one is a 0
                TablePropertyMapPtr pMergeProps( new TablePropertyMap );
                pMergeProps->Insert( PROP_VERTICAL_MERGE, uno::makeAny( sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_ST_Merge_restart ) );
                cellProps( pMergeProps);
            }
            break;
            case NS_ooxml::LN_CT_TcPrBase_hMerge:
            {
                // values can be: LN_Value_ST_Merge_restart, LN_Value_ST_Merge_continue, in reality the second one is a 0
                TablePropertyMapPtr pMergeProps(new TablePropertyMap());
                pMergeProps->Insert(PROP_HORIZONTAL_MERGE, uno::makeAny( sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_ST_Merge_restart ));
                cellProps(pMergeProps);
            }
            break;
            case NS_ooxml::LN_CT_TcPrBase_gridSpan: //number of grid positions spanned by this cell
            {
#ifdef DBG_UTIL
                TagLogger::getInstance().startElement("tablemanager.GridSpan");
                TagLogger::getInstance().attribute("gridSpan", nIntValue);
                TagLogger::getInstance().endElement();
#endif
                m_nGridSpan = nIntValue;
            }
            break;
            case NS_ooxml::LN_CT_TcPrBase_textDirection:
            {
                TablePropertyMapPtr pPropMap( new TablePropertyMap );
                bool bInsertCellProps = true;
                switch ( nIntValue )
                {
                    case NS_ooxml::LN_Value_ST_TextDirection_tbRl:
                    // Binary filter takes BiDirection into account ( but I have no idea about that here )
                    // or even what it is. But... here's where to handle it if it becomes an issue
                        pPropMap->Insert( PROP_FRM_DIRECTION, uno::makeAny( text::WritingMode2::TB_RL ));
                        SAL_INFO( "writerfilter", "Have inserted textDirection " << nIntValue );
                        break;
                    case NS_ooxml::LN_Value_ST_TextDirection_btLr:
                        pPropMap->Insert( PROP_FRM_DIRECTION, uno::makeAny( text::WritingMode2::BT_LR ));
                        break;
                    case NS_ooxml::LN_Value_ST_TextDirection_lrTbV:
                        pPropMap->Insert( PROP_FRM_DIRECTION, uno::makeAny( text::WritingMode2::LR_TB ));
                        break;
                    case NS_ooxml::LN_Value_ST_TextDirection_tbRlV:
                        pPropMap->Insert( PROP_FRM_DIRECTION, uno::makeAny( text::WritingMode2::TB_RL ));
                        break;
                    case NS_ooxml::LN_Value_ST_TextDirection_lrTb:
                    case NS_ooxml::LN_Value_ST_TextDirection_tbLrV:
                    default:
                       // Ignore - we can't handle these
                       bInsertCellProps = false;
                       break;
                }
                if ( bInsertCellProps )
                    cellProps( pPropMap );
                break;
            }
            case NS_ooxml::LN_CT_TcPrBase_tcW:
                {
                    // Contains unit and value, but unit is not interesting for
                    // us, later we'll just distribute these values in a
                    // 0..10000 scale.
                    writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                    if( pProperties )
                    {
                        MeasureHandlerPtr pMeasureHandler(new MeasureHandler());
                        pProperties->resolve(*pMeasureHandler);
                        if (sal::static_int_cast<Id>(pMeasureHandler->getUnit()) == NS_ooxml::LN_Value_ST_TblWidth_auto)
                            getCurrentCellWidths()->push_back(sal_Int32(-1));
                        else
                            // store the original value to limit rounding mistakes, if it's there in a recognized measure (twip)
                            getCurrentCellWidths()->push_back(pMeasureHandler->getMeasureValue() ? pMeasureHandler->getValue() : sal_Int32(0));
                        if (getTableDepthDifference() > 0)
                            m_bPushCurrentWidth = true;
                    }
                }
                break;
            case NS_ooxml::LN_CT_TblPrBase_tblpPr:
                {
                    writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                    // Ignore <w:tblpPr> in shape text, those tables should be always non-floating ones.
                    if (!m_bIsInShape && pProperties)
                    {
                        TablePositionHandlerPtr pHandler = m_aTmpPosition.back();
                        if ( !pHandler )
                        {
                            m_aTmpPosition.pop_back();
                            pHandler = new TablePositionHandler;
                            m_aTmpPosition.push_back( pHandler );
                        }
                        pProperties->resolve(*m_aTmpPosition.back());
                    }
                }
                break;
            case NS_ooxml::LN_CT_TrPrBase_gridBefore:
                setCurrentGridBefore( nIntValue );
                break;
            case NS_ooxml::LN_CT_TrPrBase_gridAfter:
                setCurrentGridAfter( nIntValue );
                break;
            case NS_ooxml::LN_CT_TblPrBase_tblCaption:
                // To-Do: Not yet preserved
                break;
            case NS_ooxml::LN_CT_TblPrBase_tblDescription:
                // To-Do: Not yet preserved
                break;
            case NS_ooxml::LN_CT_TrPrBase_tblCellSpacing:
                // To-Do: Not yet preserved
                break;
            case NS_ooxml::LN_CT_TblPrBase_tblCellSpacing:
                // To-Do: Not yet preserved
                break;
            case NS_ooxml::LN_CT_TblPrBase_bidiVisual:
            {
                TablePropertyMapPtr pPropMap(new TablePropertyMap());
                pPropMap->Insert(PROP_WRITING_MODE, uno::makeAny(sal_Int16(nIntValue ? text::WritingMode2::RL_TB : text::WritingMode2::LR_TB)));
                insertTableProps(pPropMap);
                break;
            }
            default:
                bRet = false;

#ifdef DBG_UTIL
                TagLogger::getInstance().element("unhandled");
#endif
        }
    }
    return bRet;
}

DomainMapperTableManager::IntVectorPtr const & DomainMapperTableManager::getCurrentGrid( )
{
    return m_aTableGrid.back( );
}

DomainMapperTableManager::IntVectorPtr const & DomainMapperTableManager::getCurrentCellWidths( )
{
    return m_aCellWidths.back( );
}

uno::Sequence<beans::PropertyValue> DomainMapperTableManager::getCurrentTablePosition( )
{
    if ( !m_aTablePositions.empty( ) && m_aTablePositions.back() )
        return m_aTablePositions.back( )->getTablePosition();
    else
        return uno::Sequence< beans::PropertyValue >();
}

TablePositionHandler* DomainMapperTableManager::getCurrentTableRealPosition()
{
    if ( !m_aTablePositions.empty( ) && m_aTablePositions.back() )
        return m_aTablePositions.back().get();
    else
        return nullptr;
}

TableParagraphVectorPtr DomainMapperTableManager::getCurrentParagraphs( )
{
    return m_aParagraphsToEndTable.top( );
}

void DomainMapperTableManager::setIsInShape(bool bIsInShape)
{
    m_bIsInShape = bIsInShape;
}

void DomainMapperTableManager::startLevel( )
{
    TableManager::startLevel( );

    // If requested, pop the value that was pushed too early.
    std::optional<sal_Int32> oCurrentWidth;
    if (m_bPushCurrentWidth && !m_aCellWidths.empty() && !m_aCellWidths.back()->empty())
    {
        oCurrentWidth = m_aCellWidths.back()->back();
        m_aCellWidths.back()->pop_back();
    }
    std::optional<TableParagraph> oParagraph;
    if (getTableDepthDifference() > 0 && !m_aParagraphsToEndTable.empty() && !m_aParagraphsToEndTable.top()->empty())
    {
        oParagraph = m_aParagraphsToEndTable.top()->back();
        m_aParagraphsToEndTable.top()->pop_back();
    }

    IntVectorPtr pNewGrid = std::make_shared<vector<sal_Int32>>();
    IntVectorPtr pNewCellWidths = std::make_shared<vector<sal_Int32>>();
    TablePositionHandlerPtr pNewPositionHandler;
    m_aTableGrid.push_back( pNewGrid );
    m_aCellWidths.push_back( pNewCellWidths );
    m_aTablePositions.push_back( pNewPositionHandler );
    // empty name will be replaced by the table style name, if it exists
    m_aTableStyleNames.push_back( OUString() );

    TablePositionHandlerPtr pTmpPosition;
    TablePropertyMapPtr pTmpProperties( new TablePropertyMap( ) );
    m_aTmpPosition.push_back( pTmpPosition );
    m_aTmpTableProperties.push_back( pTmpProperties );
    m_nCell.push_back( 0 );
    m_nTableWidth = 0;
    m_nLayoutType = 0;
    TableParagraphVectorPtr pNewParagraphs = std::make_shared<vector<TableParagraph>>();
    m_aParagraphsToEndTable.push( pNewParagraphs );

    // And push it back to the right level.
    if (oCurrentWidth)
        m_aCellWidths.back()->push_back(*oCurrentWidth);
    if (oParagraph)
        m_aParagraphsToEndTable.top()->push_back(*oParagraph);
}

void DomainMapperTableManager::endLevel( )
{
    if (m_aTableGrid.empty())
    {
        SAL_WARN("writerfilter.dmapper", "Table stack is empty");
        return;
    }

    m_aTableGrid.pop_back( );

    // Do the same trick as in startLevel(): pop the value that was pushed too early.
    std::optional<sal_Int32> oCurrentWidth;
    if (m_bPushCurrentWidth && !m_aCellWidths.empty() && !m_aCellWidths.back()->empty())
        oCurrentWidth = m_aCellWidths.back()->back();
    m_aCellWidths.pop_back( );
    // And push it back to the right level.
    if (oCurrentWidth && !m_aCellWidths.empty() && !m_aCellWidths.back()->empty())
        m_aCellWidths.back()->push_back(*oCurrentWidth);

    m_nCell.pop_back( );
    m_nTableWidth = 0;
    m_nLayoutType = 0;

    m_aTmpPosition.pop_back( );
    m_aTmpTableProperties.pop_back( );

    TableManager::endLevel( );
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("dmappertablemanager.endLevel");
    PropertyMapPtr pProps = getTableProps().get();
    if (pProps)
        getTableProps()->dumpXml();

    TagLogger::getInstance().endElement();
#endif

    // Pop back the table position after endLevel as it's used
    // in the endTable method called in endLevel.
    m_aTablePositions.pop_back();
    m_aTableStyleNames.pop_back();
    m_aParagraphsToEndTable.pop();
}

void DomainMapperTableManager::endOfCellAction()
{
#ifdef DBG_UTIL
    TagLogger::getInstance().element("endOFCellAction");
#endif

    if ( !isInTable() )
        throw std::out_of_range("cell without a table");
    if ( m_nGridSpan > 1 )
        setCurrentGridSpan( m_nGridSpan );
    m_nGridSpan = 1;
    ++m_nCell.back( );
}

bool DomainMapperTableManager::shouldInsertRow(IntVectorPtr pCellWidths, IntVectorPtr pTableGrid, size_t nGrids, bool& rIsIncompleteGrid)
{
    if (pCellWidths->empty())
        return false;
    if (m_nLayoutType == NS_ooxml::LN_Value_doc_ST_TblLayout_fixed)
        return true;
    if (pCellWidths->size() == nGrids)
        return true;
    rIsIncompleteGrid = true;
    return nGrids > pTableGrid->size();
}

void DomainMapperTableManager::endOfRowAction()
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("endOfRowAction");
#endif

    // Compare the table position and style with the previous ones. We may need to split
    // into two tables if those are different. We surely don't want to do anything
    // if we don't have any row yet.
    TablePositionHandlerPtr pTmpPosition = m_aTmpPosition.back();
    TablePropertyMapPtr pTablePropMap = m_aTmpTableProperties.back( );
    TablePositionHandlerPtr pCurrentPosition = m_aTablePositions.back();
    bool bSamePosition = ( pTmpPosition == pCurrentPosition ) ||
                         ( pTmpPosition && pCurrentPosition && *pTmpPosition == *pCurrentPosition );
    bool bIsSetTableStyle = pTablePropMap->isSet(META_PROP_TABLE_STYLE_NAME);
    OUString sTableStyleName;
    bool bSameTableStyle = ( !bIsSetTableStyle && m_aTableStyleNames.back().isEmpty()) ||
            ( bIsSetTableStyle &&
              (pTablePropMap->getProperty(META_PROP_TABLE_STYLE_NAME)->second >>= sTableStyleName) &&
              sTableStyleName == m_aTableStyleNames.back() );
    if ( (!bSamePosition || !bSameTableStyle) && m_nRow > 0 )
    {
        // Save the grid infos to have them survive the end/start level
        IntVectorPtr pTmpTableGrid = m_aTableGrid.back();
        IntVectorPtr pTmpCellWidths = m_aCellWidths.back();
        sal_uInt32 nTmpCell = m_nCell.back();
        TableParagraphVectorPtr pTableParagraphs = getCurrentParagraphs();

        // endLevel and startLevel are taking care of the non finished row
        // to carry it over to the next table
        setKeepUnfinishedRow( true );
        endLevel();
        setKeepUnfinishedRow( false );
        startLevel();

        m_aTableGrid.pop_back();
        m_aCellWidths.pop_back();
        m_nCell.pop_back();
        m_aTableGrid.push_back(pTmpTableGrid);
        m_aCellWidths.push_back(pTmpCellWidths);
        m_nCell.push_back(nTmpCell);
        m_aParagraphsToEndTable.pop( );
        m_aParagraphsToEndTable.push( pTableParagraphs );
    }
    // save table style in the first row for comparison
    if ( m_nRow == 0 && pTablePropMap->isSet(META_PROP_TABLE_STYLE_NAME) )
    {
        pTablePropMap->getProperty(META_PROP_TABLE_STYLE_NAME)->second >>= sTableStyleName;
        m_aTableStyleNames.pop_back();
        m_aTableStyleNames.push_back( sTableStyleName );
    }

    // Push the tmp position now that we compared it
    m_aTablePositions.pop_back();
    m_aTablePositions.push_back( pTmpPosition );
    m_aTmpPosition.back().clear( );


    IntVectorPtr pTableGrid = getCurrentGrid( );
    IntVectorPtr pCellWidths = getCurrentCellWidths( );
    if(!m_nTableWidth && !pTableGrid->empty())
    {
#ifdef DBG_UTIL
        TagLogger::getInstance().startElement("tableWidth");
#endif

        for( const auto& rCell : *pTableGrid )
        {
#ifdef DBG_UTIL
            TagLogger::getInstance().startElement("col");
            TagLogger::getInstance().attribute("width", rCell);
            TagLogger::getInstance().endElement();
#endif

            m_nTableWidth = o3tl::saturating_add(m_nTableWidth, rCell);
        }
        if (m_nTableWidth)
            // convert sum of grid twip values to 1/100 mm with rounding up to avoid table width loss
            m_nTableWidth = static_cast<sal_Int32>(ceil(ConversionHelper::convertTwipToMM100Double(m_nTableWidth)));

        if (m_nTableWidth > 0 && !m_bTableSizeTypeInserted)
        {
            TablePropertyMapPtr pPropMap( new TablePropertyMap );
            pPropMap->setValue( TablePropertyMap::TABLE_WIDTH, m_nTableWidth );
            insertTableProps(pPropMap);
        }

#ifdef DBG_UTIL
        TagLogger::getInstance().endElement();
#endif
    }

    std::vector<sal_uInt32> rCurrentSpans = getCurrentGridSpans();

#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("gridSpans");
    {
        for (const auto& rGridSpan : rCurrentSpans)
        {
            TagLogger::getInstance().startElement("gridSpan");
            TagLogger::getInstance().attribute("span", rGridSpan);
            TagLogger::getInstance().endElement();
        }
    }
    TagLogger::getInstance().endElement();
#endif

    //calculate number of used grids - it has to match the size of m_aTableGrid
    size_t nGrids = std::accumulate(rCurrentSpans.begin(), rCurrentSpans.end(), sal::static_int_cast<size_t>(0));

    // sj: the grid is having no units... it is containing only relative values.
    // a table with a grid of "1:2:1" looks identical as if the table is having
    // a grid of "20:40:20" and it doesn't have to do something with the tableWidth
    // -> so we have get the sum of each grid entry for the fullWidthRelative:
    int nFullWidthRelative = 0;
    for (int i : (*pTableGrid))
        nFullWidthRelative = o3tl::saturating_add(nFullWidthRelative, i);

    bool bIsIncompleteGrid = false;
    if( pTableGrid->size() == nGrids && m_nCell.back( ) > 0 )
    {
        /*
         * If table width property set earlier is smaller than the current table width,
         * then replace the TABLE_WIDTH property, set earlier.
         */
        sal_Int32 nTableWidth(0);
        sal_Int32 nTableWidthType(text::SizeType::VARIABLE);
        pTablePropMap->getValue(TablePropertyMap::TABLE_WIDTH, nTableWidth);
        pTablePropMap->getValue(TablePropertyMap::TABLE_WIDTH_TYPE, nTableWidthType);
        if ((nTableWidthType == text::SizeType::FIX) && (nTableWidth < m_nTableWidth))
        {
            pTablePropMap->setValue(TablePropertyMap::TABLE_WIDTH, m_nTableWidth);
        }
        if (nTableWidthType == text::SizeType::VARIABLE )
        {
            if(nTableWidth > 100 || nTableWidth <= 0)
            {
                if(getTableDepth() > 1 && !m_bTableSizeTypeInserted)
                {
                    pTablePropMap->setValue(TablePropertyMap::TABLE_WIDTH, sal_Int32(100));
                    pTablePropMap->setValue(TablePropertyMap::TABLE_WIDTH_TYPE, text::SizeType::VARIABLE);
                }
                else
                {
                    pTablePropMap->setValue(TablePropertyMap::TABLE_WIDTH, m_nTableWidth);
                    pTablePropMap->setValue(TablePropertyMap::TABLE_WIDTH_TYPE, text::SizeType::FIX);
                }
            }
        }
        uno::Sequence< text::TableColumnSeparator > aSeparators( getCurrentGridBefore() + m_nCell.back() - 1 );
        text::TableColumnSeparator* pSeparators = aSeparators.getArray();
        double nLastRelPos = 0.0;
        sal_uInt32 nBorderGridIndex = 0;

        size_t nWidthsBound = getCurrentGridBefore() + m_nCell.back() - 1;
        if (nWidthsBound)
        {
            ::std::vector< sal_uInt32 >::const_iterator aSpansIter = rCurrentSpans.begin();
            for( size_t nBorder = 0; nBorder < nWidthsBound; ++nBorder )
            {
                double nRelPos, fGridWidth = 0.;
                for ( sal_Int32 nGridCount = *aSpansIter; nGridCount > 0; --nGridCount )
                    fGridWidth += (*pTableGrid)[nBorderGridIndex++];

                if (fGridWidth == 0.)
                {
                    // allow nFullWidthRelative here, with a sane 0.0 result
                    nRelPos = 0.;
                }
                else
                {
                    if (nFullWidthRelative == 0)
                        throw o3tl::divide_by_zero();

                    nRelPos = (fGridWidth * 10000) / nFullWidthRelative;
                }

                pSeparators[nBorder].Position = rtl::math::round(nRelPos + nLastRelPos);
                pSeparators[nBorder].IsVisible = true;
                nLastRelPos = nLastRelPos + nRelPos;
                ++aSpansIter;
            }
        }
        TablePropertyMapPtr pPropMap( new TablePropertyMap );
        pPropMap->Insert( PROP_TABLE_COLUMN_SEPARATORS, uno::makeAny( aSeparators ) );

#ifdef DBG_UTIL
        TagLogger::getInstance().startElement("rowProperties");
        pPropMap->dumpXml();
        TagLogger::getInstance().endElement();
#endif
        insertRowProps(pPropMap);
    }
    else if (shouldInsertRow(pCellWidths, pTableGrid, nGrids, bIsIncompleteGrid))
    {
        // If we're here, then the number of cells does not equal to the amount
        // defined by the grid, even after taking care of
        // gridSpan/gridBefore/gridAfter. Handle this by ignoring the grid and
        // providing the separators based on the provided cell widths, as long
        // as we have a fixed layout;
        // On the other hand even if the layout is not fixed, but the cell widths
        // provided equal the total number of cells, and there are no after/before cells
        // then use the cell widths to calculate the column separators.
        // Also handle autofit tables with incomplete grids, when rows can have
        // different widths and last cells can be wider, than their values.
        uno::Sequence< text::TableColumnSeparator > aSeparators(pCellWidths->size() - 1);
        text::TableColumnSeparator* pSeparators = aSeparators.getArray();
        sal_Int16 nSum = 0;
        sal_uInt32 nPos = 0;

        if (bIsIncompleteGrid)
            nFullWidthRelative = 0;

        // Avoid divide by zero (if there's no grid, position using cell widths).
        if( nFullWidthRelative == 0 )
            for (size_t i = 0; i < pCellWidths->size(); ++i)
                nFullWidthRelative += (*pCellWidths)[i];

        if (bIsIncompleteGrid)
        {
            /*
             * If table width property set earlier is smaller than the current table row width,
             * then replace the TABLE_WIDTH property, set earlier.
             */
            sal_Int32 nFullWidth = static_cast<sal_Int32>(ceil(ConversionHelper::convertTwipToMM100Double(nFullWidthRelative)));
            sal_Int32 nTableWidth(0);
            sal_Int32 nTableWidthType(text::SizeType::VARIABLE);
            pTablePropMap->getValue(TablePropertyMap::TABLE_WIDTH, nTableWidth);
            pTablePropMap->getValue(TablePropertyMap::TABLE_WIDTH_TYPE, nTableWidthType);
            if (nTableWidth < nFullWidth)
            {
                pTablePropMap->setValue(TablePropertyMap::TABLE_WIDTH, nFullWidth);
            }
        }

        size_t nWidthsBound = pCellWidths->size() - 1;
        if (nWidthsBound)
        {
            if (nFullWidthRelative == 0)
                throw o3tl::divide_by_zero();

            // At incomplete table grids, last cell width can be smaller, than its final width.
            // Correct it based on the last but one column width and their span values.
            if ( bIsIncompleteGrid && rCurrentSpans.size()-1 == nWidthsBound )
            {
                auto aSpansIter = std::next(rCurrentSpans.begin(), nWidthsBound - 1);
                sal_Int32 nFixLastCellWidth = (*pCellWidths)[nWidthsBound-1] / *aSpansIter * *std::next(aSpansIter);
                if (nFixLastCellWidth > (*pCellWidths)[nWidthsBound])
                    nFullWidthRelative += nFixLastCellWidth - (*pCellWidths)[nWidthsBound];
            }

            for (size_t i = 0; i < nWidthsBound; ++i)
            {
                nSum += (*pCellWidths)[i];
                pSeparators[nPos].Position = (nSum * 10000) / nFullWidthRelative; // Relative position
                pSeparators[nPos].IsVisible = true;
                nPos++;
            }
        }

        TablePropertyMapPtr pPropMap( new TablePropertyMap );
        pPropMap->Insert( PROP_TABLE_COLUMN_SEPARATORS, uno::makeAny( aSeparators ) );
#ifdef DBG_UTIL
        TagLogger::getInstance().startElement("rowProperties");
        pPropMap->dumpXml();
        TagLogger::getInstance().endElement();
#endif
        insertRowProps(pPropMap);
    }

    // Now that potentially opened table is closed, save the table properties
    TableManager::insertTableProps(pTablePropMap);

    m_aTmpTableProperties.pop_back();
    TablePropertyMapPtr pEmptyTableProps( new TablePropertyMap() );
    m_aTmpTableProperties.push_back( pEmptyTableProps );

    ++m_nRow;
    m_nCell.back( ) = 0;
    getCurrentGrid()->clear();
    pCellWidths->clear();

    m_bTableSizeTypeInserted = false;

#ifdef DBG_UTIL
    TagLogger::getInstance().endElement();
#endif
}

void DomainMapperTableManager::clearData()
{
    m_nRow = m_nHeaderRepeat = m_nTableWidth = m_nLayoutType = 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
