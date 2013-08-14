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
#include <boost/optional.hpp>
#include <DomainMapperTableManager.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <BorderHandler.hxx>
#include <CellColorHandler.hxx>
#include <CellMarginHandler.hxx>
#include <ConversionHelper.hxx>
#include <MeasureHandler.hxx>
#include <TDefTableHandler.hxx>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <ooxml/resourceids.hxx>
#include <doctok/sprmids.hxx>
#include <dmapperLoggers.hxx>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;
using namespace ::std;


DomainMapperTableManager::DomainMapperTableManager(bool bOOXML) :
    m_nRow(0),
    m_nCell(),
    m_nGridSpan(1),
    m_nGridBefore(0),
    m_nGridAfter(0),
    m_nCellBorderIndex(0),
    m_nHeaderRepeat(0),
    m_nTableWidth(0),
    m_bOOXML( bOOXML ),
    m_aTmpPosition(),
    m_aTmpTableProperties(),
    m_bPushCurrentWidth(false),
    m_bRowSizeTypeInserted(false),
    m_bTableSizeTypeInserted(false),
    m_nLayoutType(0),
    m_nMaxFixedWidth(0),
    m_pTablePropsHandler( new TablePropertiesHandler( bOOXML ) )
{
    m_pTablePropsHandler->SetTableManager( this );

#ifdef DEBUG_DOMAINMAPPER
#ifdef DEBUG_TABLE
    setTagLogger(dmapper_logger);
#endif
#endif
}


DomainMapperTableManager::~DomainMapperTableManager()
{
    if ( m_pTablePropsHandler )
        delete m_pTablePropsHandler, m_pTablePropsHandler = NULL;
}


bool DomainMapperTableManager::sprm(Sprm & rSprm)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("tablemanager.sprm");
    string sSprm = rSprm.toString();
    dmapper_logger->chars(sSprm);
    dmapper_logger->endElement();
#endif
    bool bRet = DomainMapperTableManager_Base_t::sprm(rSprm);
    if( !bRet )
    {
        bRet = m_pTablePropsHandler->sprm( rSprm );
    }

    if ( !bRet )
    {
        bRet = true;
        sal_uInt32 nSprmId = rSprm.getId();
        Value::Pointer_t pValue = rSprm.getValue();
        sal_Int32 nIntValue = ((pValue.get() != NULL) ? pValue->getInt() : 0);
        switch ( nSprmId )
        {
            case 0xf661: //sprmTTRLeft left table indent
            case 0xf614: // sprmTTPreferredWidth - preferred table width
            case NS_ooxml::LN_CT_TblPrBase_tblW:  //90722;
            case NS_ooxml::LN_CT_TblPrBase_tblInd: //90725
            {
                //contains unit and value
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {   //contains attributes x2902 (LN_unit) and x17e2 (LN_trleft)
                    MeasureHandlerPtr pMeasureHandler( new MeasureHandler );
                    pProperties->resolve(*pMeasureHandler);
                    TablePropertyMapPtr pPropMap( new TablePropertyMap );
                    if( nSprmId == 0xf661 || nSprmId == sal_uInt32(NS_ooxml::LN_CT_TblPrBase_tblInd ))
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
                        }
                        else if( sal::static_int_cast<Id>(pMeasureHandler->getUnit()) == NS_ooxml::LN_Value_ST_TblWidth_pct )
                        {
                            sal_Int32 nPercent = pMeasureHandler->getValue() / 50;
                            if(nPercent > 100)
                                nPercent = 100;
                            pPropMap->setValue( TablePropertyMap::TABLE_WIDTH_TYPE, text::SizeType::VARIABLE );
                            pPropMap->setValue( TablePropertyMap::TABLE_WIDTH, nPercent );
                        }
                        else if( sal::static_int_cast<Id>(pMeasureHandler->getUnit()) == NS_ooxml::LN_Value_ST_TblWidth_auto )
                        {
                            /*
                            This attribute specifies the width type of table. This is used as part of the table layout
                            algorithm specified by the tblLayout element.(See 17.4.64 and 17.4.65 of the ISO/IEC 29500-1:2011.)
                            If this valus is 'auto', the table layout has to uses the preferred widths on the table items to generate
                            the final sizing of the table, but then must use the contents of each cell to determine final column widths.
                            (See 17.18.87 of the ISO/IEC 29500-1:2011.)
                            */
                            bool bFixed = true;
                            sal_Int32 nRowFixedWidth = 0;
                            if (!m_aCellWidths.empty())
                            {
                                // Step 1. Check whether all cells have fixed widths in the given row of table.
                                ::std::vector< IntVectorPtr >::iterator itr;
                                for (itr = m_aCellWidths.begin(); itr != m_aCellWidths.end(); ++itr )
                                {
                                    IntVectorPtr itrVal = (*itr);
                                    for (std::vector<sal_Int32>::const_iterator aValIter = itrVal->begin(); aValIter != itrVal->end(); ++aValIter)
                                    {
                                        if (*aValIter == -1)
                                        {
                                            bFixed = false;
                                            break;
                                        }
                                        // Sum the width of cells to find the total width of given row
                                        nRowFixedWidth += (*aValIter);
                                    }
                                }
                            }

                            // Check whether the total width of given row is compared with the maximum value of rows (m_nMaxFixedWidth).
                            if (bFixed )
                            {
                                // Check if total width
                                if (m_nMaxFixedWidth < nRowFixedWidth)
                                    m_nMaxFixedWidth = nRowFixedWidth;

                                pPropMap->setValue( TablePropertyMap::TABLE_WIDTH_TYPE, text::SizeType::FIX );
                                pPropMap->setValue( TablePropertyMap::TABLE_WIDTH, m_nMaxFixedWidth );
                            }
                            else
                            {
                                // Set the width type of table with 'Auto' and set the width value to 100(%)
                                pPropMap->setValue( TablePropertyMap::TABLE_WIDTH_TYPE, text::SizeType::VARIABLE );
                                pPropMap->setValue( TablePropertyMap::TABLE_WIDTH, 100 );
                            }
                        }
                        m_bTableSizeTypeInserted = true;
                    }
#ifdef DEBUG_DOMAINMAPPER
                    pPropMap->dumpXml( dmapper_logger );
#endif
                    insertTableProps(pPropMap);
                }
            }
            break;
            case 0x3404:// sprmTTableHeader
            case NS_ooxml::LN_CT_TrPrBase_tblHeader: //90704
                // if nIntValue == 1 then the row is a repeated header line
                // to prevent later rows from increasing the repeating m_nHeaderRepeat is set to NULL when repeating stops
                if( nIntValue > 0 && m_nHeaderRepeat >= 0 )
                {
                    ++m_nHeaderRepeat;
                    TablePropertyMapPtr pPropMap( new TablePropertyMap );
                    pPropMap->Insert( PROP_HEADER_ROW_COUNT, uno::makeAny( m_nHeaderRepeat ));
                    insertTableProps(pPropMap);
                }
                else
                    m_nHeaderRepeat = -1;
            break;
            case 0xd608: // TDefTable
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    TDefTableHandlerPtr pTDefTableHandler( new TDefTableHandler(m_bOOXML) );
                    pProperties->resolve( *pTDefTableHandler );

                    TablePropertyMapPtr pRowPropMap( new TablePropertyMap );
                    pRowPropMap->InsertProps(pTDefTableHandler->getRowProperties());
                    insertRowProps( pRowPropMap );
                    if( !m_nTableWidth )
                    {
                        m_nTableWidth= pTDefTableHandler->getTableWidth();
                        if( m_nTableWidth )
                        {
                            TablePropertyMapPtr pPropMap( new TablePropertyMap );
                            pPropMap->setValue( TablePropertyMap::TABLE_WIDTH, m_nTableWidth );
                            insertTableProps(pPropMap);
                        }
                    }
                    for( size_t nCell = 0; nCell < pTDefTableHandler->getCellCount(); ++nCell )
                    {
                        TablePropertyMapPtr pCellPropMap( new TablePropertyMap );
                        pTDefTableHandler->fillCellProperties( nCell, pCellPropMap );
                        cellPropsByCell( nCell, pCellPropMap );
                    }
                }
            }
            break;
            case 0xD605: // sprmTTableBorders
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    BorderHandlerPtr pBorderHandler( new BorderHandler(m_bOOXML) );
                    pProperties->resolve(*pBorderHandler);
                    TablePropertyMapPtr pCellPropMap( new TablePropertyMap() );
                    pCellPropMap->InsertProps(pBorderHandler->getProperties());
                    cellPropsByCell( m_nCellBorderIndex, pCellPropMap );
                    ++m_nCellBorderIndex;
                }
            }
            break;
            case 0xd632 : //sprmTNewSpacing
            case 0xd634 : //sprmTNewSpacing
                //TODO: sprms contain default (TNew) and actual border spacing of cells - not resolvable yet
            break;
            case 0xd613: //sprmTGridLineProps
                // TODO: needs a handler
                /*contains:
                 GridLineProps">
                    rtf:LINEPROPSTOP
                    rtf:LINEPROPSLEFT
                    rtf:LINEPROPSBOTTOM
                    rtf:LINEPROPSRIGHT
                    rtf:LINEPROPSHORIZONTAL
                    rtf:LINEPROPSVERTICAL
                        rtf:LINECOLOR
                        rtf:LINEWIDTH
                        rtf:LINETYPE

                */
            break;
            case 0x740a : //sprmTTlp
                //TODO: Table look specifier
            break;
            case 0x6816 : //unknown
            case 0x3466 : //unknown
            case 0x3615 : //unknown
            case 0x646b : //unknown - expandable sprm - see ww8scan.cxx
            case 0x7479 : //unknown
            case 0xf617 : //unknown
            case 0xf618 : //unknown
                bRet = false;
            break;
            case NS_ooxml::LN_CT_TblPrBase_tblStyle: //table style name
            {
                m_sTableStyleName = pValue->getString();
                TablePropertyMapPtr pPropMap( new TablePropertyMap );
                pPropMap->Insert( META_PROP_TABLE_STYLE_NAME, uno::makeAny( m_sTableStyleName ));
                insertTableProps(pPropMap);
            }
            break;
            case NS_ooxml::LN_CT_TblGridBase_gridCol:
            {
                if (nIntValue == -1)
                    getCurrentGrid()->clear();
                else
                    getCurrentGrid()->push_back( ConversionHelper::convertTwipToMM100( nIntValue ) );
            }
            break;
            case NS_ooxml::LN_CT_TcPrBase_vMerge : //vertical merge
            {
                // values can be: LN_Value_ST_Merge_restart, LN_Value_ST_Merge_continue, in reality the second one is a 0
                TablePropertyMapPtr pMergeProps( new TablePropertyMap );
                pMergeProps->Insert( PROP_VERTICAL_MERGE, uno::makeAny( bool( sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_ST_Merge_restart )) );
                cellProps( pMergeProps);
            }
            break;
            case NS_ooxml::LN_CT_TcPrBase_gridSpan: //number of grid positions spanned by this cell
            {
#ifdef DEBUG_DOMAINMAPPER
                dmapper_logger->startElement("tablemanager.GridSpan");
                dmapper_logger->attribute("gridSpan", nIntValue);
                dmapper_logger->endElement();
#endif
                m_nGridSpan = nIntValue;
            }
            break;
            case NS_ooxml::LN_CT_TblPrBase_tblLook:
            {
                TablePropertyMapPtr pPropMap( new TablePropertyMap );
                pPropMap->Insert( PROP_TBL_LOOK, uno::makeAny( nIntValue ));
                insertTableProps(pPropMap);
            }
            break;
            case NS_ooxml::LN_CT_TcPrBase_textDirection:
            {
                TablePropertyMapPtr pPropMap( new TablePropertyMap );
                bool bInsertCellProps = true;
                switch ( nIntValue )
                {
                    case 1:  // tbRl
                    // Binary filter takes BiDirection into account ( but I have no idea about that here )
                    // or even what it is. But... here's where to handle it if it becomes an issue
                        pPropMap->Insert( PROP_FRM_DIRECTION, uno::makeAny( text::WritingMode2::TB_RL ));
                        SAL_INFO( "writerfilter", "Have inserted textDirection " << nIntValue );
                        break;
                    case 3:  // btLr
                        {
                        // We have to fake this text direction
                         pPropMap->Insert( PROP_FRM_DIRECTION, uno::makeAny( text::WritingMode2::LR_TB ));
                         pPropMap->Insert( PROP_CHAR_ROTATION, uno::makeAny( sal_Int16( 900 ) ));
                        SAL_INFO( "writerfilter", "Have inserted textDirection " << nIntValue );

                        // We're faking a text direction, so don't allow multiple lines.
                        TablePropertyMapPtr pRowPropMap( new TablePropertyMap );
                        pRowPropMap->Insert(PROP_SIZE_TYPE, uno::makeAny(text::SizeType::FIX));
                        m_bRowSizeTypeInserted = true;
                        insertRowProps(pRowPropMap);
                        }
                        break;
                    case 4: // lrTbV
                        pPropMap->Insert( PROP_FRM_DIRECTION, uno::makeAny( text::WritingMode2::LR_TB ));
                        break;
                    case 5: // tbRlV
                        pPropMap->Insert( PROP_FRM_DIRECTION, uno::makeAny( text::WritingMode2::TB_RL ));
                        break;
                    case 0: // lrTb
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
                    if( pProperties.get())
                    {
                        MeasureHandlerPtr pMeasureHandler(new MeasureHandler());
                        pProperties->resolve(*pMeasureHandler);
                        if (sal::static_int_cast<Id>(pMeasureHandler->getUnit()) == NS_ooxml::LN_Value_ST_TblWidth_auto)
                            getCurrentCellWidths()->push_back(sal_Int32(-1));
                        else
                            getCurrentCellWidths()->push_back(pMeasureHandler->getMeasureValue());
                        if (getTableDepthDifference() > 0)
                            m_bPushCurrentWidth = true;
                    }
                }
                break;
            case NS_ooxml::LN_CT_TrPrBase_cnfStyle:
                break;  // the cnfStyle doesn't matter, instead the tblLook property is used to specify conditional styles that are to be used
            case NS_ooxml::LN_CT_PPrBase_cnfStyle:
                // TODO cnfStyle on a paragraph
                break;
            case NS_ooxml::LN_CT_TcPrBase_cnfStyle:
                break;  // the cnfStyle doesn't matter, instead the tblLook property is used to specify conditional styles that are to be used
            case NS_ooxml::LN_CT_TblPrBase_tblpPr:
                {
                    writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                    if (pProperties.get())
                    {
                        TablePositionHandlerPtr pHandler = m_aTmpPosition.back();
                        if ( !pHandler )
                        {
                            m_aTmpPosition.pop_back();
                            pHandler.reset( new TablePositionHandler );
                            m_aTmpPosition.push_back( pHandler );
                        }
                        pProperties->resolve(*m_aTmpPosition.back());
                    }
                }
                break;
            case NS_ooxml::LN_CT_TrPrBase_gridBefore:
                m_nGridBefore = nIntValue;
                break;
            case NS_ooxml::LN_CT_TrPrBase_gridAfter:
                m_nGridAfter = nIntValue;
                break;
            default:
                bRet = false;

#ifdef DEBUG_DOMAINMAPPER
                dmapper_logger->element("unhandled");
#endif
        }
    }
    return bRet;
}

boost::shared_ptr< vector<sal_Int32> > DomainMapperTableManager::getCurrentGrid( )
{
    return m_aTableGrid.back( );
}

boost::shared_ptr< vector< sal_Int32 > > DomainMapperTableManager::getCurrentSpans( )
{
    return m_aGridSpans.back( );
}

boost::shared_ptr< vector< sal_Int32 > > DomainMapperTableManager::getCurrentCellWidths( )
{
    return m_aCellWidths.back( );
}

const uno::Sequence<beans::PropertyValue> DomainMapperTableManager::getCurrentTablePosition( )
{
    if ( !m_aTablePositions.empty( ) && m_aTablePositions.back() )
        return m_aTablePositions.back( )->getTablePosition();
    else
        return uno::Sequence< beans::PropertyValue >( 0 );
}

void DomainMapperTableManager::startLevel( )
{
    DomainMapperTableManager_Base_t::startLevel( );

    // If requested, pop the value that was pushed too early.
    boost::optional<sal_Int32> oCurrentWidth;
    if (m_bPushCurrentWidth && !m_aCellWidths.empty() && !m_aCellWidths.back()->empty())
    {
        oCurrentWidth.reset(m_aCellWidths.back()->back());
        m_aCellWidths.back()->pop_back();
    }

    IntVectorPtr pNewGrid( new vector<sal_Int32> );
    IntVectorPtr pNewSpans( new vector<sal_Int32> );
    IntVectorPtr pNewCellWidths( new vector<sal_Int32> );
    TablePositionHandlerPtr pNewPositionHandler;
    m_aTableGrid.push_back( pNewGrid );
    m_aGridSpans.push_back( pNewSpans );
    m_aCellWidths.push_back( pNewCellWidths );
    m_aTablePositions.push_back( pNewPositionHandler );

    TablePositionHandlerPtr pTmpPosition;
    TablePropertyMapPtr pTmpProperties( new TablePropertyMap( ) );
    m_aTmpPosition.push_back( pTmpPosition );
    m_aTmpTableProperties.push_back( pTmpProperties );
    m_nCell.push_back( 0 );
    m_nTableWidth = 0;
    m_nLayoutType = 0;
    m_nMaxFixedWidth = 0;

    // And push it back to the right level.
    if (oCurrentWidth)
        m_aCellWidths.back()->push_back(*oCurrentWidth);
}

void DomainMapperTableManager::endLevel( )
{
    m_aTableGrid.pop_back( );
    m_aGridSpans.pop_back( );
    m_aCellWidths.pop_back( );
    m_nCell.pop_back( );
    m_nTableWidth = 0;
    m_nLayoutType = 0;
    m_nMaxFixedWidth = 0;

    m_aTmpPosition.pop_back( );
    m_aTmpTableProperties.pop_back( );

    DomainMapperTableManager_Base_t::endLevel( );
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("dmappertablemanager.endLevel");
    PropertyMapPtr pProps = getTableProps();
    if (pProps.get() != NULL)
        getTableProps()->dumpXml( dmapper_logger );

    dmapper_logger->endElement();
#endif

    // Pop back the table position after endLevel as it's used
    // in the endTable method called in endLevel.
    m_aTablePositions.pop_back();
}



void DomainMapperTableManager::endOfCellAction()
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->element("endOFCellAction");
#endif

    getCurrentSpans()->push_back(m_nGridSpan);
    m_nGridSpan = 1;
    ++m_nCell.back( );
}


void DomainMapperTableManager::endOfRowAction()
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("endOfRowAction");
#endif

    // Compare the table position with the previous ones. We may need to split
    // into two tables if those are different. We surely don't want to do anything
    // if we don't have any row yet.
    TablePositionHandlerPtr pTmpPosition = m_aTmpPosition.back();
    TablePropertyMapPtr pTmpTableProperties = m_aTmpTableProperties.back( );
    TablePositionHandlerPtr pCurrentPosition = m_aTablePositions.back();
    bool bSamePosition = ( pTmpPosition == pCurrentPosition ) ||
                         ( pTmpPosition && pCurrentPosition && *pTmpPosition == *pCurrentPosition );
    if ( !bSamePosition && m_nRow > 0 )
    {
        // Save the grid infos to have them survive the end/start level
        IntVectorPtr pTmpTableGrid = m_aTableGrid.back();
        IntVectorPtr pTmpGridSpans = m_aGridSpans.back();
        IntVectorPtr pTmpCellWidths = m_aCellWidths.back();

        // endLevel and startLevel are taking care of the non finished row
        // to carry it over to the next table
        setKeepUnfinishedRow( true );
        endLevel();
        setKeepUnfinishedRow( false );
        startLevel();

        m_aTableGrid.pop_back();
        m_aGridSpans.pop_back();
        m_aCellWidths.pop_back();
        m_aTableGrid.push_back(pTmpTableGrid);
        m_aGridSpans.push_back(pTmpGridSpans);
        m_aCellWidths.push_back(pTmpCellWidths);
    }

    // Push the tmp position now that we compared it
    m_aTablePositions.pop_back();
    m_aTablePositions.push_back( pTmpPosition );
    m_aTmpPosition.back().reset( );


    IntVectorPtr pTableGrid = getCurrentGrid( );
    IntVectorPtr pCellWidths = getCurrentCellWidths( );
    if(!m_nTableWidth && pTableGrid->size())
    {
        ::std::vector<sal_Int32>::const_iterator aCellIter = pTableGrid->begin();

#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->startElement("tableWidth");
#endif

        while( aCellIter != pTableGrid->end() )
        {
#ifdef DEBUG_DOMAINMAPPER
            dmapper_logger->startElement("col");
            dmapper_logger->attribute("width", *aCellIter);
            dmapper_logger->endElement();
#endif

             m_nTableWidth += *aCellIter++;
        }

        if (m_nTableWidth > 0 && !m_bTableSizeTypeInserted)
        {
            TablePropertyMapPtr pPropMap( new TablePropertyMap );
            pPropMap->setValue( TablePropertyMap::TABLE_WIDTH, m_nTableWidth );
            insertTableProps(pPropMap);
        }

#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->endElement();
#endif
    }

    IntVectorPtr pCurrentSpans = getCurrentSpans( );
    if( pCurrentSpans->size() < m_nCell.back( ) )
    {
        //fill missing elements with '1'
        pCurrentSpans->insert( pCurrentSpans->end( ), m_nCell.back( ) - pCurrentSpans->size(), 1 );
    }

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("gridSpans");
    {
        ::std::vector<sal_Int32>::const_iterator aGridSpanIter = pCurrentSpans->begin();
        ::std::vector<sal_Int32>::const_iterator aGridSpanIterEnd = pCurrentSpans->end();

        while (aGridSpanIter != aGridSpanIterEnd)
        {
            dmapper_logger->startElement("gridSpan");
            dmapper_logger->attribute("span", *aGridSpanIter);
            dmapper_logger->endElement();

            ++aGridSpanIter;
        }
    }
    dmapper_logger->endElement();
#endif

    //calculate number of used grids - it has to match the size of m_aTableGrid
    size_t nGrids = 0;
    ::std::vector<sal_Int32>::const_iterator aGridSpanIter = pCurrentSpans->begin();
    for( ; aGridSpanIter != pCurrentSpans->end(); ++aGridSpanIter)
        nGrids += *aGridSpanIter;

    // sj: the grid is having no units... they is containing only relative values.
    // a table with a grid of "1:2:1" looks identical as if the table is having
    // a grid of "20:40:20" and it doesn't have to do something with the tableWidth
    // -> so we have get the sum of each grid entry for the fullWidthRelative:
    int nFullWidthRelative = 0;
    for (unsigned int i = 0 ; i < (*pTableGrid.get()).size(); i++ )
        nFullWidthRelative += (*pTableGrid.get())[ i ];

    if( pTableGrid->size() == ( m_nGridBefore + nGrids + m_nGridAfter ) && m_nCell.back( ) > 0 )
    {
        uno::Sequence< text::TableColumnSeparator > aSeparators( m_nCell.back( ) - 1 );
        text::TableColumnSeparator* pSeparators = aSeparators.getArray();
        sal_Int16 nLastRelPos = 0;
        sal_uInt32 nBorderGridIndex = m_nGridBefore;

        ::std::vector< sal_Int32 >::const_iterator aSpansIter = pCurrentSpans->begin( );
        for( sal_uInt32 nBorder = 0; nBorder < m_nCell.back( ) - 1; ++nBorder )
        {
            double fGridWidth = 0.;
            for ( sal_Int32 nGridCount = *aSpansIter; nGridCount > 0; --nGridCount )
                fGridWidth += (*pTableGrid.get())[nBorderGridIndex++];

            sal_Int16 nRelPos =
                sal::static_int_cast< sal_Int16 >((fGridWidth * 10000) / nFullWidthRelative);

            pSeparators[nBorder].Position =  nRelPos + nLastRelPos;
            pSeparators[nBorder].IsVisible = sal_True;
            nLastRelPos = nLastRelPos + nRelPos;
            ++aSpansIter;
        }
        TablePropertyMapPtr pPropMap( new TablePropertyMap );
        pPropMap->Insert( PROP_TABLE_COLUMN_SEPARATORS, uno::makeAny( aSeparators ) );

#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->startElement("rowProperties");
        pPropMap->dumpXml( dmapper_logger );
        dmapper_logger->endElement();
#endif
        insertRowProps(pPropMap);
    }
    else if (pCellWidths->size() > 0 && m_nLayoutType == NS_ooxml::LN_Value_wordprocessingml_ST_TblLayout_fixed)
    {
        // If we're here, then the number of cells does not equal to the amount
        // defined by the grid, even after taking care of
        // gridSpan/gridBefore/gridAfter. Handle this by ignoring the grid and
        // providing the separators based on the provided cell widths, as long
        // as we have a fixed layout.
        uno::Sequence< text::TableColumnSeparator > aSeparators(pCellWidths->size() - 1);
        text::TableColumnSeparator* pSeparators = aSeparators.getArray();
        sal_Int16 nSum = 0;
        sal_uInt32 nPos = 0;
        // Avoid divide by zero (if there's no grid, position using cell widths).
        if( nFullWidthRelative == 0 )
            for (sal_uInt32 i = 0; i < pCellWidths->size(); ++i)
                nFullWidthRelative += (*pCellWidths.get())[i];

        for (sal_uInt32 i = 0; i < pCellWidths->size() - 1; ++i)
        {
            nSum += (*pCellWidths.get())[i];
            pSeparators[nPos].Position = (nSum * 10000) / nFullWidthRelative; // Relative position
            pSeparators[nPos].IsVisible = sal_True;
            nPos++;
        }

        TablePropertyMapPtr pPropMap( new TablePropertyMap );
        pPropMap->Insert( PROP_TABLE_COLUMN_SEPARATORS, uno::makeAny( aSeparators ) );
#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->startElement("rowProperties");
        pPropMap->dumpXml( dmapper_logger );
        dmapper_logger->endElement();
#endif
        insertRowProps(pPropMap);
    }

    // Now that potentially opened table is closed, save the table properties
    DomainMapperTableManager_Base_t::insertTableProps( pTmpTableProperties );

    m_aTmpTableProperties.pop_back();
    TablePropertyMapPtr pEmptyTableProps( new TablePropertyMap() );
    m_aTmpTableProperties.push_back( pEmptyTableProps );

    ++m_nRow;
    m_nCell.back( ) = 0;
    m_nCellBorderIndex = 0;
    getCurrentGrid()->clear();
    pCurrentSpans->clear();
    pCellWidths->clear();

    m_nGridBefore = m_nGridAfter = 0;
    m_bRowSizeTypeInserted = false;
    m_bTableSizeTypeInserted = false;

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement();
#endif
}


void DomainMapperTableManager::clearData()
{
    m_nRow = m_nCellBorderIndex = m_nHeaderRepeat = m_nTableWidth = m_nLayoutType = 0;
    m_sTableStyleName = OUString();
    m_pTableStyleTextProperies.reset();
}


}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
