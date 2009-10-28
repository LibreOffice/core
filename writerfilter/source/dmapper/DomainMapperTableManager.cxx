/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DomainMapperTableManager.cxx,v $
 * $Revision: 1.21 $
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
#include <ooxml/resourceids.hxx>
#include <doctok/sprmids.hxx>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;
using namespace ::std;
/*-- 23.04.2007 14:57:49---------------------------------------------------

  -----------------------------------------------------------------------*/
DomainMapperTableManager::DomainMapperTableManager(bool bOOXML) :
    m_nRow(0),
    m_nCell(0),
    m_nCellBorderIndex(0),
    m_nHeaderRepeat(0),
    m_nTableWidth(0),
    m_bOOXML( bOOXML ),
    m_pTablePropsHandler( new TablePropertiesHandler( bOOXML ) )
{
    m_pTablePropsHandler->SetTableManager( this );
}
/*-- 23.04.2007 14:57:49---------------------------------------------------

  -----------------------------------------------------------------------*/
DomainMapperTableManager::~DomainMapperTableManager()
{
    if ( m_pTablePropsHandler )
        delete m_pTablePropsHandler, m_pTablePropsHandler = NULL;
}
/*-- 23.04.2007 15:25:37---------------------------------------------------

  -----------------------------------------------------------------------*/
bool DomainMapperTableManager::sprm(Sprm & rSprm)
{
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
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case 0xf661: //sprmTTRLeft left table indent
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case 0xf614: // sprmTTPreferredWidth - preferred table width
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TblPrBase_tblW:  //90722;
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
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
                    insertTableProps(pPropMap);
                }
            }
            break;
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
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
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case 0xd608: // TDefTable
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    TDefTableHandlerPtr pTDefTableHandler( new TDefTableHandler(m_bOOXML) );
                    pProperties->resolve( *pTDefTableHandler );

                    TablePropertyMapPtr pRowPropMap( new TablePropertyMap );
                    pRowPropMap->insert( pTDefTableHandler->getRowProperties() );
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
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case 0xD605: // sprmTTableBorders
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    BorderHandlerPtr pBorderHandler( new BorderHandler(m_bOOXML) );
                    pProperties->resolve(*pBorderHandler);
                    TablePropertyMapPtr pCellPropMap( new TablePropertyMap() );
                    pCellPropMap->insert( pBorderHandler->getProperties() );
                    cellPropsByCell( m_nCellBorderIndex, pCellPropMap );
                    ++m_nCellBorderIndex;
                }
            }
            break;
            /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
            case 0xd632 : //sprmTNewSpacing
            /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
            case 0xd634 : //sprmTNewSpacing
                //TODO: sprms contain default (TNew) and actual border spacing of cells - not resolvable yet
            break;
            /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
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
            /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
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
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TblPrBase_tblStyle: //table style name
            {
                m_sTableStyleName = pValue->getString();
                TablePropertyMapPtr pPropMap( new TablePropertyMap );
                pPropMap->Insert( META_PROP_TABLE_STYLE_NAME, false, uno::makeAny( m_sTableStyleName ));
                insertTableProps(pPropMap);
            }
            break;
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TblGridBase_gridCol:
            {
                getCurrentGrid()->push_back( ConversionHelper::convertTwipToMM100( nIntValue ) );
            }
            break;
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TcPrBase_vMerge : //vertical merge
            {
                // values can be: LN_Value_ST_Merge_restart, LN_Value_ST_Merge_continue, in reality the second one is a 0
                TablePropertyMapPtr pMergeProps( new TablePropertyMap );
                pMergeProps->Insert( PROP_VERTICAL_MERGE, false, uno::makeAny( bool( sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_ST_Merge_restart )) );
                cellProps( pMergeProps);
            }
            break;
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TcPrBase_gridSpan: //number of grid positions spanned by this cell
            {
#if DEBUG
                clog << "GridSpan: " << nIntValue << endl;
#endif
                //the cell width is determined by its position in the table grid
                //it takes 'gridSpan' grid elements
                IntVectorPtr pCurrentSpans = getCurrentSpans( );
                if( pCurrentSpans->size() < m_nCell)
                {
                    //fill missing elements with '1'
                    pCurrentSpans->insert( pCurrentSpans->end(), m_nCell - pCurrentSpans->size(), 1 );
                }
                pCurrentSpans->push_back( nIntValue );
            }
            break;
            /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TblPrBase_tblLook:
                break; //todo: table look specifier
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
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
            case NS_ooxml::LN_tblStart:
                {
                    startLevel( );
                }
                break;
            default:
                bRet = false;
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

void DomainMapperTableManager::startLevel( )
{
    DomainMapperTableManager_Base_t::startLevel( );

    IntVectorPtr pNewGrid( new vector<sal_Int32> );
    IntVectorPtr pNewSpans( new vector<sal_Int32> );
    m_aTableGrid.push_back( pNewGrid );
    m_aGridSpans.push_back( pNewSpans );
    m_nTableWidth = 0;
}

void DomainMapperTableManager::endLevel( )
{
    m_aTableGrid.pop_back( );
    m_aGridSpans.pop_back( );

    DomainMapperTableManager_Base_t::endLevel( );
}

/*-- 02.05.2007 14:36:26---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapperTableManager::endOfCellAction()
{
    ++m_nCell;
}
/*-- 02.05.2007 14:36:26---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapperTableManager::endOfRowAction()
{
    IntVectorPtr pTableGrid = getCurrentGrid( );
    if(!m_nTableWidth && pTableGrid->size())
    {
        ::std::vector<sal_Int32>::const_iterator aCellIter = pTableGrid->begin();
        while( aCellIter != pTableGrid->end() )
             m_nTableWidth += *aCellIter++;
        if( m_nTableWidth > 0)
        {
            TablePropertyMapPtr pPropMap( new TablePropertyMap );
//            pPropMap->Insert( PROP_WIDTH, false, uno::makeAny( m_nTableWidth ));
            pPropMap->setValue( TablePropertyMap::TABLE_WIDTH, m_nTableWidth );
            insertTableProps(pPropMap);
        }
    }

    IntVectorPtr pCurrentSpans = getCurrentSpans( );
    if( pCurrentSpans->size() < m_nCell)
    {
        //fill missing elements with '1'
        pCurrentSpans->insert( pCurrentSpans->end( ), m_nCell - pCurrentSpans->size(), 1 );
    }
    //calculate number of used grids - it has to match the size of m_aTableGrid
    size_t nGrids = 0;
    ::std::vector<sal_Int32>::const_iterator aGridSpanIter = pCurrentSpans->begin();
    for( ; aGridSpanIter != pCurrentSpans->end(); ++aGridSpanIter)
        nGrids += *aGridSpanIter;

    if( pTableGrid->size() == nGrids )
    {
        //determine table width
        double nFullWidth = m_nTableWidth;
        //the positions have to be distibuted in a range of 10000
        const double nFullWidthRelative = 10000.;
        uno::Sequence< text::TableColumnSeparator > aSeparators( m_nCell - 1 );
        text::TableColumnSeparator* pSeparators = aSeparators.getArray();
        sal_Int16 nLastRelPos = 0;
        sal_uInt32 nBorderGridIndex = 0;

        ::std::vector< sal_Int32 >::const_iterator aSpansIter = pCurrentSpans->begin( );
        for( sal_uInt32 nBorder = 0; nBorder < m_nCell - 1; ++nBorder )
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
            aSpansIter++;
        }
        TablePropertyMapPtr pPropMap( new TablePropertyMap );
        pPropMap->Insert( PROP_TABLE_COLUMN_SEPARATORS, false, uno::makeAny( aSeparators ) );
        insertRowProps(pPropMap);
    }

    ++m_nRow;
    m_nCell = 0;
    m_nCellBorderIndex = 0;
    pCurrentSpans->clear();
}
/*-- 18.06.2007 10:34:37---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapperTableManager::clearData()
{
    m_nRow = m_nCell = m_nCellBorderIndex = m_nHeaderRepeat = m_nTableWidth = 0;
    m_sTableStyleName = ::rtl::OUString();
    m_pTableStyleTextProperies.reset();
}
/*-- 27.06.2007 14:19:50---------------------------------------------------

  -----------------------------------------------------------------------*/
void lcl_CopyTextProperties(PropertyMapPtr pToFill,
            const StyleSheetEntry* pStyleSheetEntry, StyleSheetTablePtr pStyleSheetTable)
{
    if( !pStyleSheetEntry )
        return;
    //fill base style properties first, recursively
    if( pStyleSheetEntry->sBaseStyleIdentifier.getLength())
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
    pContext->insert( m_pTableStyleTextProperies );
}
}}
