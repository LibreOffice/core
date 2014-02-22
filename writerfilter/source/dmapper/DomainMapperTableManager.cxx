/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
            case 0xf661: 
            case 0xf614: 
            case NS_ooxml::LN_CT_TblPrBase_tblW:  
            case NS_ooxml::LN_CT_TblPrBase_tblInd: 
            {
                
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {   
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
                            IntVectorPtr pCellWidths = getCurrentCellWidths();
                            
                            for (std::vector<sal_Int32>::const_iterator aValIter = pCellWidths->begin(); aValIter != pCellWidths->end(); ++aValIter)
                            {
                                if (*aValIter == -1)
                                {
                                    bFixed = false;
                                    break;
                                }
                                
                                nRowFixedWidth += (*aValIter);
                            }

                            
                            if (bFixed )
                            {
                                
                                if (m_nMaxFixedWidth < nRowFixedWidth)
                                    m_nMaxFixedWidth = nRowFixedWidth;

                                pPropMap->setValue( TablePropertyMap::TABLE_WIDTH_TYPE, text::SizeType::FIX );
                                pPropMap->setValue( TablePropertyMap::TABLE_WIDTH, m_nMaxFixedWidth );
                            }
                            else
                            {
                                
                                pPropMap->setValue( TablePropertyMap::TABLE_WIDTH_TYPE, text::SizeType::VARIABLE );
                                pPropMap->setValue( TablePropertyMap::TABLE_WIDTH, 0 );
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
            case 0x3404:
            case NS_ooxml::LN_CT_TrPrBase_tblHeader: 
                
                
                if( nIntValue > 0 && m_nHeaderRepeat >= 0 )
                {
                    ++m_nHeaderRepeat;
                    TablePropertyMapPtr pPropMap( new TablePropertyMap );
                    pPropMap->Insert( PROP_HEADER_ROW_COUNT, uno::makeAny( m_nHeaderRepeat ));
                    insertTableProps(pPropMap);
                }
                else
                    m_nHeaderRepeat = -1;
                if (nIntValue)
                {
                    
                    TablePropertyMapPtr pPropMap( new TablePropertyMap );
                    pPropMap->Insert( PROP_TBL_HEADER, uno::makeAny(nIntValue));
                    insertRowProps(pPropMap);
                }
            break;
            case 0xd608: 
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
            case 0xD605: 
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
            case 0xd632 : 
            case 0xd634 : 
                
            break;
            case 0xd613: 
                
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
            case 0x740a : 
                
            break;
            case 0x6816 : 
            case 0x3466 : 
            case 0x3615 : 
            case 0x646b : 
            case 0x7479 : 
            case 0xf617 : 
            case 0xf618 : 
                bRet = false;
            break;
            case NS_ooxml::LN_CT_TblPrBase_tblStyle: 
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
            case NS_ooxml::LN_CT_TcPrBase_vMerge : 
            {
                
                TablePropertyMapPtr pMergeProps( new TablePropertyMap );
                pMergeProps->Insert( PROP_VERTICAL_MERGE, uno::makeAny( bool( sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_ST_Merge_restart )) );
                cellProps( pMergeProps);
            }
            break;
            case NS_ooxml::LN_CT_TcPrBase_hMerge:
            {
                
                TablePropertyMapPtr pMergeProps(new TablePropertyMap());
                pMergeProps->Insert(PROP_HORIZONTAL_MERGE, uno::makeAny(bool(sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_ST_Merge_restart)));
                cellProps(pMergeProps);
            }
            break;
            case NS_ooxml::LN_CT_TcPrBase_gridSpan: 
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
                    case 1:  
                    
                    
                        pPropMap->Insert( PROP_FRM_DIRECTION, uno::makeAny( text::WritingMode2::TB_RL ));
                        SAL_INFO( "writerfilter", "Have inserted textDirection " << nIntValue );
                        break;
                    case 3:  
                        {
                        
                         pPropMap->Insert( PROP_FRM_DIRECTION, uno::makeAny( text::WritingMode2::LR_TB ));
                         pPropMap->Insert( PROP_CHAR_ROTATION, uno::makeAny( sal_Int16( 900 ) ));
                        SAL_INFO( "writerfilter", "Have inserted textDirection " << nIntValue );

                        
                        TablePropertyMapPtr pRowPropMap( new TablePropertyMap );
                        pRowPropMap->Insert(PROP_SIZE_TYPE, uno::makeAny(text::SizeType::FIX));
                        m_bRowSizeTypeInserted = true;
                        insertRowProps(pRowPropMap);
                        }
                        break;
                    case 4: 
                        pPropMap->Insert( PROP_FRM_DIRECTION, uno::makeAny( text::WritingMode2::LR_TB ));
                        break;
                    case 5: 
                        pPropMap->Insert( PROP_FRM_DIRECTION, uno::makeAny( text::WritingMode2::TB_RL ));
                        break;
                    case 0: 
                    case NS_ooxml::LN_Value_ST_TextDirection_tbLrV:
                    default:
                       
                       bInsertCellProps = false;
                       break;
                }
                if ( bInsertCellProps )
                    cellProps( pPropMap );
                break;
            }
            case NS_ooxml::LN_CT_TcPrBase_tcW:
                {
                    
                    
                    
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
                break;  
            case NS_ooxml::LN_CT_PPrBase_cnfStyle:
                
                break;
            case NS_ooxml::LN_CT_TcPrBase_cnfStyle:
                break;  
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

    
    if (oCurrentWidth)
        m_aCellWidths.back()->push_back(*oCurrentWidth);
}

void DomainMapperTableManager::endLevel( )
{
    m_aTableGrid.pop_back( );
    m_aGridSpans.pop_back( );

    
    boost::optional<sal_Int32> oCurrentWidth;
    if (m_bPushCurrentWidth && !m_aCellWidths.empty() && !m_aCellWidths.back()->empty())
        oCurrentWidth.reset(m_aCellWidths.back()->back());
    m_aCellWidths.pop_back( );
    
    if (oCurrentWidth)
        m_aCellWidths.back()->push_back(*oCurrentWidth);

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

    
    
    
    TablePositionHandlerPtr pTmpPosition = m_aTmpPosition.back();
    TablePropertyMapPtr pTmpTableProperties = m_aTmpTableProperties.back( );
    TablePositionHandlerPtr pCurrentPosition = m_aTablePositions.back();
    bool bSamePosition = ( pTmpPosition == pCurrentPosition ) ||
                         ( pTmpPosition && pCurrentPosition && *pTmpPosition == *pCurrentPosition );
    if ( !bSamePosition && m_nRow > 0 )
    {
        
        IntVectorPtr pTmpTableGrid = m_aTableGrid.back();
        IntVectorPtr pTmpGridSpans = m_aGridSpans.back();
        IntVectorPtr pTmpCellWidths = m_aCellWidths.back();

        
        
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

    
    size_t nGrids = 0;
    ::std::vector<sal_Int32>::const_iterator aGridSpanIter = pCurrentSpans->begin();
    for( ; aGridSpanIter != pCurrentSpans->end(); ++aGridSpanIter)
        nGrids += *aGridSpanIter;

    
    
    
    
    int nFullWidthRelative = 0;
    for (unsigned int i = 0 ; i < (*pTableGrid.get()).size(); i++ )
        nFullWidthRelative += (*pTableGrid.get())[ i ];

    if( pTableGrid->size() == ( m_nGridBefore + nGrids + m_nGridAfter ) && m_nCell.back( ) > 0 )
    {
        /*
         * If table width property set earlier is smaller than the current table width,
         * then replace the TABLE_WIDTH property, set earlier.
         */
        TablePropertyMapPtr propMap = m_aTmpTableProperties.back();
        sal_Int32 nTableWidth;
        sal_Int32 nTableWidthType;
        propMap->getValue( TablePropertyMap::TABLE_WIDTH, nTableWidth );
        propMap->getValue( TablePropertyMap::TABLE_WIDTH_TYPE, nTableWidthType );
        if ((nTableWidthType == text::SizeType::FIX) && (nTableWidth < m_nTableWidth))
        {
            propMap->setValue( TablePropertyMap::TABLE_WIDTH, m_nTableWidth );
        }
        if (nTableWidthType == text::SizeType::VARIABLE )
        {
            if(nTableWidth > 100 || nTableWidth <= 0)
            {
                propMap->setValue( TablePropertyMap::TABLE_WIDTH, m_nTableWidth);
                propMap->setValue( TablePropertyMap::TABLE_WIDTH_TYPE, text::SizeType::FIX);
            }
        }
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
        
        
        
        
        
        uno::Sequence< text::TableColumnSeparator > aSeparators(pCellWidths->size() - 1);
        text::TableColumnSeparator* pSeparators = aSeparators.getArray();
        sal_Int16 nSum = 0;
        sal_uInt32 nPos = 0;
        
        if( nFullWidthRelative == 0 )
            for (sal_uInt32 i = 0; i < pCellWidths->size(); ++i)
                nFullWidthRelative += (*pCellWidths.get())[i];

        for (sal_uInt32 i = 0; i < pCellWidths->size() - 1; ++i)
        {
            nSum += (*pCellWidths.get())[i];
            pSeparators[nPos].Position = (nSum * 10000) / nFullWidthRelative; 
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
