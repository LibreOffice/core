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
#include <DomainMapperTableManager.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <BorderHandler.hxx>
#include <CellColorHandler.hxx>
#include <CellMarginHandler.hxx>
#include <ConversionHelper.hxx>
#include <MeasureHandler.hxx>
#include <TablePositionHandler.hxx>
#include <TDefTableHandler.hxx>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <ooxml/resourceids.hxx>
#include <doctok/sprmids.hxx>
#include <dmapperLoggers.hxx>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;
using namespace ::std;


DomainMapperTableManager::DomainMapperTableManager(bool bOOXML, bool bImplicitMerges) :
    m_nRow(0),
    m_nCell(),
    m_nGridSpan(1),
    m_nGridBefore(0),
    m_nGridAfter(0),
    m_nCellBorderIndex(0),
    m_nHeaderRepeat(0),
    m_nTableWidth(0),
    m_bOOXML( bOOXML ),
    m_bImplicitMerges(bImplicitMerges),
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
                            pPropMap->setValue( TablePropertyMap::TABLE_WIDTH, m_nTableWidth );
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
                    pPropMap->Insert( PROP_HEADER_ROW_COUNT, false, uno::makeAny( m_nHeaderRepeat ));
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
                pPropMap->Insert( META_PROP_TABLE_STYLE_NAME, false, uno::makeAny( m_sTableStyleName ));
                insertTableProps(pPropMap);
            }
            break;
            case NS_ooxml::LN_CT_TblGridBase_gridCol:
            {
                getCurrentGrid()->push_back( ConversionHelper::convertTwipToMM100( nIntValue ) );
            }
            break;
            case NS_ooxml::LN_CT_TcPrBase_vMerge : //vertical merge
            {
                // values can be: LN_Value_ST_Merge_restart, LN_Value_ST_Merge_continue, in reality the second one is a 0
                TablePropertyMapPtr pMergeProps( new TablePropertyMap );
                pMergeProps->Insert( PROP_VERTICAL_MERGE, false, uno::makeAny( bool( sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_ST_Merge_restart )) );
                cellProps( pMergeProps);
            }
            break;
            case NS_ooxml::LN_CT_TcPrBase_gridSpan: //number of grid positions spanned by this cell
            {
#if DEBUG_DOMAINMAPPER
                dmapper_logger->startElement("tablemanager.GridSpan");
                dmapper_logger->attribute("gridSpan", nIntValue);
                dmapper_logger->endElement();
#endif
                m_nGridSpan = nIntValue;
            }
            break;
            case NS_ooxml::LN_CT_TblPrBase_tblLook:
                break; //todo: table look specifier
            case NS_ooxml::LN_CT_TcPrBase_textDirection:
            {
                TablePropertyMapPtr pPropMap( new TablePropertyMap );
                const sal_Int16 HORI_LEFT_TOP = 0;
                const sal_Int16 VERT_TOP_RIGHT = 2;
                bool bInsertCellProps = true;
                switch ( nIntValue )
                {
                    case 1:  // tbRl
                    // Binary filter takes BiDirection into account ( but I have no idea about that here )
                    // or even what it is. But... here's where to handle it if it becomes an issue
                        pPropMap->Insert( PROP_FRM_DIRECTION, false, uno::makeAny( VERT_TOP_RIGHT ));
                        SAL_INFO( "writerfilter", "Have inserted textDirection " << nIntValue );
                        break;
                    case 3:  // btLr
                        // We have to fake this text direction
                         pPropMap->Insert( PROP_FRM_DIRECTION, false, uno::makeAny( HORI_LEFT_TOP ));
                         pPropMap->Insert( PROP_CHAR_ROTATION, false, uno::makeAny( sal_Int16( 900 ) ));
                        SAL_INFO( "writerfilter", "Have inserted textDirection " << nIntValue );
                        break;
                    case 4: // lrTbV
                        pPropMap->Insert( PROP_FRM_DIRECTION, false, uno::makeAny( HORI_LEFT_TOP ));
                        break;
                    case 5: // tbRlV
                        pPropMap->Insert( PROP_FRM_DIRECTION, false, uno::makeAny( VERT_TOP_RIGHT ));
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
                break; //fixed column width is not supported
            case NS_ooxml::LN_CT_TrPrBase_cnfStyle:
                {
                    TablePropertyMapPtr pProps( new TablePropertyMap );
                    pProps->Insert( PROP_CNF_STYLE, true, uno::makeAny( pValue->getString( ) ) );
                    insertRowProps( pProps );
                }
                break;
            case NS_ooxml::LN_CT_PPrBase_cnfStyle:
                // TODO cnfStyle on a paragraph
                break;
            case NS_ooxml::LN_CT_TcPrBase_cnfStyle:
                {
                    TablePropertyMapPtr pProps( new TablePropertyMap );
                    pProps->Insert( PROP_CNF_STYLE, true, uno::makeAny( pValue->getString( ) ) );
                    cellProps( pProps );
                }
                break;
            case NS_ooxml::LN_CT_TblPrBase_tblpPr:
                {
                    writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                    if (pProperties.get())
                    {
                        TablePositionHandlerPtr pHandler( new TablePositionHandler );
                        pProperties->resolve(*pHandler);
                        m_sTableVertAnchor = pHandler->getVertAnchor();
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

const OUString& DomainMapperTableManager::getTableVertAnchor() const
{
    return m_sTableVertAnchor;
}

void DomainMapperTableManager::startLevel( )
{
    DomainMapperTableManager_Base_t::startLevel( );

    IntVectorPtr pNewGrid( new vector<sal_Int32> );
    IntVectorPtr pNewSpans( new vector<sal_Int32> );
    m_aTableGrid.push_back( pNewGrid );
    m_aGridSpans.push_back( pNewSpans );
    m_nCell.push_back( 0 );
    m_nTableWidth = 0;
}

void DomainMapperTableManager::endLevel( )
{
    m_aTableGrid.pop_back( );
    m_aGridSpans.pop_back( );
    m_nCell.pop_back( );
    m_nTableWidth = 0;

    DomainMapperTableManager_Base_t::endLevel( );
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("dmappertablemanager.endLevel");
    PropertyMapPtr pProps = getTableProps();
    if (pProps.get() != NULL)
        getTableProps()->dumpXml( dmapper_logger );

    dmapper_logger->endElement();
#endif
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

    IntVectorPtr pTableGrid = getCurrentGrid( );
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

        if( m_nTableWidth > 0)
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

    //determine table width
    double nFullWidth = m_nTableWidth;
    //the positions have to be distibuted in a range of 10000
    const double nFullWidthRelative = 10000.;
    if( pTableGrid->size() == ( m_nGridBefore + nGrids + m_nGridAfter ) && m_nCell.back( ) > 0 )
    {
        uno::Sequence< text::TableColumnSeparator > aSeparators( m_nCell.back( ) - 1 );
        text::TableColumnSeparator* pSeparators = aSeparators.getArray();
        sal_Int16 nLastRelPos = 0;
        sal_uInt32 nBorderGridIndex = m_nGridBefore;

        ::std::vector< sal_Int32 >::const_iterator aSpansIter = pCurrentSpans->begin( );
        for( sal_uInt32 nBorder = 0; nBorder < m_nCell.back( ) - 1; ++nBorder )
        {
            sal_Int32 nGridCount = *aSpansIter;
            double fGridWidth = 0.;
            do
            {
                fGridWidth += (*pTableGrid.get())[nBorderGridIndex++];
            }while( --nGridCount );

            sal_Int16 nRelPos =
                sal::static_int_cast< sal_Int16 >(fGridWidth * nFullWidthRelative / nFullWidth );

            pSeparators[nBorder].Position =  nRelPos + nLastRelPos;
            pSeparators[nBorder].IsVisible = sal_True;
            nLastRelPos = nLastRelPos + nRelPos;
            ++aSpansIter;
        }
        TablePropertyMapPtr pPropMap( new TablePropertyMap );
        pPropMap->Insert( PROP_TABLE_COLUMN_SEPARATORS, false, uno::makeAny( aSeparators ) );

#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->startElement("rowProperties");
        pPropMap->dumpXml( dmapper_logger );
        dmapper_logger->endElement();
#endif
        insertRowProps(pPropMap);
    }
    else if (m_bImplicitMerges && pTableGrid->size())
    {
        // More grid than cells definitions? Then take the last ones.
        // This feature is used by the RTF implicit horizontal cell merges.
        uno::Sequence< text::TableColumnSeparator > aSeparators(m_nCell.back( ) - 1);
        text::TableColumnSeparator* pSeparators = aSeparators.getArray();

        sal_Int16 nSum = 0;
        sal_uInt32 nPos = 0;
        sal_uInt32 nSizeTableGrid = pTableGrid->size();
        // Ignoring the i=0 case means we assume that the width of the last cell matches the table width
        for (sal_uInt32 i = m_nCell.back( ); i > 1 && nSizeTableGrid >= i; i--)
        {
            nSum += (*pTableGrid.get())[pTableGrid->size() - i]; // Size of the current cell
            pSeparators[nPos].Position = nSum * nFullWidthRelative / nFullWidth; // Relative position
            pSeparators[nPos].IsVisible = sal_True;
            nPos++;
        }

        TablePropertyMapPtr pPropMap( new TablePropertyMap );
        pPropMap->Insert( PROP_TABLE_COLUMN_SEPARATORS, false, uno::makeAny( aSeparators ) );
#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->startElement("rowProperties");
        pPropMap->dumpXml( dmapper_logger );
        dmapper_logger->endElement();
#endif
        insertRowProps(pPropMap);
    }

    ++m_nRow;
    m_nCell.back( ) = 0;
    m_nCellBorderIndex = 0;
    pCurrentSpans->clear();

    m_nGridBefore = m_nGridAfter = 0;

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement();
#endif
}


void DomainMapperTableManager::clearData()
{
    m_nRow = m_nCellBorderIndex = m_nHeaderRepeat = m_nTableWidth = 0;
    m_sTableStyleName = OUString();
    m_sTableVertAnchor = OUString();
    m_pTableStyleTextProperies.reset();
}


void lcl_CopyTextProperties(PropertyMapPtr pToFill,
            const StyleSheetEntry* pStyleSheetEntry, StyleSheetTablePtr pStyleSheetTable)
{
    if( !pStyleSheetEntry )
        return;
    //fill base style properties first, recursively
    if( !pStyleSheetEntry->sBaseStyleIdentifier.isEmpty())
    {
        const StyleSheetEntryPtr pParentStyleSheet =
            pStyleSheetTable->FindStyleSheetByISTD(pStyleSheetEntry->sBaseStyleIdentifier);
        OSL_ENSURE( pParentStyleSheet, "table style not found" );
        lcl_CopyTextProperties( pToFill, pParentStyleSheet.get( ), pStyleSheetTable);
    }

    PropertyMap::const_iterator aPropIter = pStyleSheetEntry->pProperties->begin();
    while(aPropIter != pStyleSheetEntry->pProperties->end())
    {
        //copy all text properties form the table style to the current run attributes
        if( aPropIter->first.bIsTextProperty )
            pToFill->insert(*aPropIter);
        ++aPropIter;
    }
}
void DomainMapperTableManager::CopyTextProperties(PropertyMapPtr pContext, StyleSheetTablePtr pStyleSheetTable)
{
    if( !m_pTableStyleTextProperies.get())
    {
        m_pTableStyleTextProperies.reset( new PropertyMap );
        const StyleSheetEntryPtr pStyleSheetEntry = pStyleSheetTable->FindStyleSheetByISTD(
                                                        m_sTableStyleName);
        OSL_ENSURE( pStyleSheetEntry, "table style not found" );
        lcl_CopyTextProperties(m_pTableStyleTextProperies, pStyleSheetEntry.get( ), pStyleSheetTable);
    }
    pContext->InsertProps(m_pTableStyleTextProperies);
}


}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
