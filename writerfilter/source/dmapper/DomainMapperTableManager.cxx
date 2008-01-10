/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DomainMapperTableManager.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:37:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include <DomainMapperTableManager.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <BorderHandler.hxx>
#include <CellColorHandler.hxx>
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
#define DEFAULT_CELL_MARGIN 108 //default cell margin, not documented
/*-- 23.04.2007 14:57:49---------------------------------------------------

  -----------------------------------------------------------------------*/
DomainMapperTableManager::DomainMapperTableManager(bool bOOXML) :
    m_nRow(0),
    m_nCell(0),
    m_nCellBorderIndex(0),
    m_nHeaderRepeat(0),
    m_nGapHalf(DEFAULT_CELL_MARGIN),
    m_nLeftMargin(0),
    m_nTableWidth(0),
    m_bFullWidth(false),
    m_bOOXML( bOOXML )
{
}
/*-- 23.04.2007 14:57:49---------------------------------------------------

  -----------------------------------------------------------------------*/
DomainMapperTableManager::~DomainMapperTableManager()
{
}
/*-- 23.04.2007 15:25:37---------------------------------------------------

  -----------------------------------------------------------------------*/
bool DomainMapperTableManager::sprm(Sprm & rSprm)
{
    bool bRet = DomainMapperTableManager_Base_t::sprm(rSprm);
    if( !bRet )
    {
        bRet = true;
        sal_uInt32 nSprmId = rSprm.getId();
        Value::Pointer_t pValue = rSprm.getValue();
        sal_Int32 nIntValue = ((pValue.get() != NULL) ? pValue->getInt() : 0);
        /* WRITERFILTERSTATUS: table: table_sprmdata */
        switch( nSprmId )
        {
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TrPrBase_jc: //90706
            case 0x5400: // sprmTJc
            if( !m_bFullWidth )
            {
                //table justification 0: left, 1: center, 2: right
                sal_Int16 nOrient = ConversionHelper::convertTableJustification( nIntValue );
                PropertyMapPtr pTableMap( new PropertyMap );
                pTableMap->Insert( PROP_HORI_ORIENT, false, uno::makeAny( nOrient ) );
                insertTableProps( pTableMap );
            }
            break;
            /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
            case 0x9601: // sprmTDxaLeft
            break;
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case 0x9602: // sprmTDxaGapHalf
            {
                m_nGapHalf = ConversionHelper::convertTwipToMM100( nIntValue );
                PropertyMapPtr pPropMap( new PropertyMap );
                pPropMap->Insert( PROP_LEFT_MARGIN, false, uno::makeAny( m_nLeftMargin - m_nGapHalf ));
                if( m_bFullWidth && m_nLeftMargin > 0 )
                    pPropMap->Insert( PROP_HORI_ORIENT, false, uno::makeAny( text::HoriOrientation::RIGHT ) );
                insertTableProps(pPropMap);
            }
            break;
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
                    PropertyMapPtr pPropMap( new PropertyMap );
                    if( nSprmId == 0xf661 || nSprmId == sal_uInt32(NS_ooxml::LN_CT_TblPrBase_tblInd ))
                    {
                        m_nLeftMargin = pMeasureHandler->getMeasureValue();
                        pPropMap->Insert( PROP_LEFT_MARGIN, false, uno::makeAny( m_nLeftMargin - m_nGapHalf ));
                        if( m_bFullWidth && m_nLeftMargin > 0 )
                            pPropMap->Insert( PROP_HORI_ORIENT, false, uno::makeAny( text::HoriOrientation::RIGHT ));

                    }
                    else
                    {
                        m_nTableWidth = pMeasureHandler->getMeasureValue();
                        if( m_nTableWidth )
                            pPropMap->Insert( PROP_WIDTH, false, uno::makeAny( m_nTableWidth ));
                        if( pMeasureHandler->isAutoWidth() )
                        {
                            pPropMap->Insert( PROP_HORI_ORIENT, false,
                                uno::makeAny( m_nLeftMargin > 0 ? text::HoriOrientation::RIGHT : text::HoriOrientation::FULL ) );
                            m_bFullWidth = true;
                        }
                    }
                    insertTableProps(pPropMap);
                }
            }
            break;
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TrPrBase_trHeight: //90703
            {
                //contains unit and value
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {   //contains attributes x2902 (LN_unit) and x17e2 (LN_trleft)
                    MeasureHandlerPtr pMeasureHandler( new MeasureHandler );
                    pProperties->resolve(*pMeasureHandler);
                    PropertyMapPtr pPropMap( new PropertyMap );
                    pPropMap->Insert( PROP_SIZE_TYPE, false, uno::makeAny( pMeasureHandler->GetRowHeightSizeType() ));
                    pPropMap->Insert( PROP_HEIGHT, false, uno::makeAny(pMeasureHandler->getMeasureValue() ));
                    insertRowProps(pPropMap);
                }
            }
            break;
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case 0x3403: // sprmTFCantSplit
            case NS_sprm::LN_TCantSplit: // 0x3644
            {
                //row can't break across pages if nIntValue == 1
                PropertyMapPtr pPropMap( new PropertyMap );
                pPropMap->Insert( PROP_IS_SPLIT_ALLOWED, false, uno::makeAny(sal_Bool( nIntValue == 1 ? sal_False : sal_True ) ));
                insertRowProps(pPropMap);
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
                    PropertyMapPtr pPropMap( new PropertyMap );
                    pPropMap->Insert( PROP_HEADER_ROW_COUNT, false, uno::makeAny( m_nHeaderRepeat ));
                    insertTableProps(pPropMap);
                }
                else
                    m_nHeaderRepeat = -1;
            break;
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case 0x9407: // sprmTDyaRowHeight
            {
                // table row height - negative values indicate 'exact height' - positive 'at least'
                PropertyMapPtr pPropMap( new PropertyMap );
                bool bMinHeight = true;
                sal_Int16 nHeight = static_cast<sal_Int16>( nIntValue );
                if( nHeight < 0 )
                {
                    bMinHeight = false;
                    nHeight *= -1;
                }
                pPropMap->Insert( PROP_SIZE_TYPE, false, uno::makeAny(bMinHeight ? text::SizeType::MIN : text::SizeType::FIX ));
                pPropMap->Insert( PROP_HEIGHT, false, uno::makeAny(ConversionHelper::convertTwipToMM100( nHeight )));
                insertRowProps(pPropMap);
            }
            break;
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case 0xd608: // TDefTable
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    TDefTableHandlerPtr pTDefTableHandler( new TDefTableHandler(m_bOOXML) );
                    pProperties->resolve( *pTDefTableHandler );
                    insertRowProps( pTDefTableHandler->getRowProperties() );
                    if( !m_nTableWidth )
                    {
                        m_nTableWidth= pTDefTableHandler->getTableWidth();
                        if( m_nTableWidth )
                        {
                            PropertyMapPtr pPropMap( new PropertyMap );
                            pPropMap->Insert( PROP_WIDTH, false, uno::makeAny( m_nTableWidth ));
                            insertTableProps(pPropMap);
                        }
                    }
                    for( size_t nCell = 0; nCell < pTDefTableHandler->getCellCount(); ++nCell )
                        cellPropsByCell( nCell, pTDefTableHandler->getCellProperties( nCell ) );
                }
            }
            break;
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TcPrBase_vAlign://90694
            {
                sal_Int16 nVertOrient = text::VertOrientation::NONE;
                switch( nIntValue ) //0 - top 1 - center 3 - bottom
                {
                    case 1: nVertOrient = text::VertOrientation::CENTER; break;
                    case 3: nVertOrient = text::VertOrientation::BOTTOM; break;
                    default:;
                };
                PropertyMapPtr pPropMap( new PropertyMap() );
                pPropMap->Insert( PROP_VERT_ORIENT, false, uno::makeAny( nVertOrient ) );
                //todo: in ooxml import the value of m_ncell is wrong
                cellProps( pPropMap );
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
                    cellPropsByCell( m_nCellBorderIndex, PropertyMapPtr( pBorderHandler->getProperties()) );
                    ++m_nCellBorderIndex;
                }
            }
            break;
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TblPrBase_tblBorders: //table borders, might be defined in table style
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    BorderHandlerPtr pBorderHandler( new BorderHandler(m_bOOXML) );
                    pProperties->resolve(*pBorderHandler);
                    insertTableProps( pBorderHandler->getProperties() );
                }
            }
            break;
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TcPrBase_tcBorders ://cell borders
            //contains CT_TcBorders_left, right, top, bottom
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    //in OOXML there's one set of borders at each cell (if there is any)
                    TDefTableHandlerPtr pTDefTableHandler( new TDefTableHandler( m_bOOXML ));
                    pProperties->resolve( *pTDefTableHandler );
                    cellProps( pTDefTableHandler->getCellProperties( 0 ) );
                }
            }
            break;
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case 0xd61a : // sprmTCellTopColor
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case 0xd61b : // sprmTCellLeftColor
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case 0xd61c : // sprmTCellBottomColor
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case 0xd61d : // sprmTCellRightColor
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TcPrBase_shd:
            {
                // each color sprm contains as much colors as cells are in a row
                //LN_CT_TcPrBase_shd: cell shading contains: LN_CT_Shd_val, LN_CT_Shd_fill, LN_CT_Shd_color
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    CellColorHandlerPtr pCellColorHandler( new CellColorHandler );
                    pProperties->resolve( *pCellColorHandler );
                    cellProps( pCellColorHandler->getProperties());
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
//OOXML table properties
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TblPrBase_tblStyle: //table style name
            {
                m_sTableStyleName = pValue->getString();
                PropertyMapPtr pPropMap( new PropertyMap );
                pPropMap->Insert( META_PROP_TABLE_STYLE_NAME, false, uno::makeAny( m_sTableStyleName ));
                insertTableProps(pPropMap);
            }
            break;
            //case NS_ooxml::LN_CT_TcPrBase_tcW: //90684 - column width values
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TblGridBase_gridCol:

            if(!m_nRow)
            {
                m_aTableGrid.push_back( ConversionHelper::convertTwipToMM100( nIntValue ) );
            }
            break;
            /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TblPrBase_tblCellMar: //cell margins
            {
                //contains LN_CT_TblCellMar_top, LN_CT_TblCellMar_left, LN_CT_TblCellMar_bottom, LN_CT_TblCellMar_right
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
//                    CellMarHandler pCellMarHandler( new CellMarHandler );
//                    pProperties->resolve( *pCellMarHandler );
//                    cellPropsByCell(0, PropertyMapPtr( pColorHandler->getProperties()) );
                }
            }
            break;
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TcPrBase_vMerge : //vertical merge
            {
                // values can be: LN_Value_ST_Merge_restart, LN_Value_ST_Merge_continue, in reality the second one is a 0
                PropertyMapPtr pMergeProps( new PropertyMap );
                pMergeProps->Insert( PROP_VERTICAL_MERGE, false, uno::makeAny( bool( sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_ST_Merge_restart )) );
                cellProps( pMergeProps);
            }
            break;
            /* WRITERFILTERSTATUS: done: 1, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TcPrBase_gridSpan: //number of grid positions spanned by this cell
            {
                //the cell width is determined by its position in the table grid
                //it takes 'gridSpan' grid elements
                if( m_aGridSpans.size() < m_nCell)
                {
                    //fill missing elements with '1'
                    m_aGridSpans.insert( m_aGridSpans.end(), m_nCell - m_aGridSpans.size(), 1 );
                }
                m_aGridSpans.push_back( nIntValue );
            }
            break;
            /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TblPrBase_tblLook: break; //todo: table look specifier
            default: bRet = false;
        }
    }
    return bRet;
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
    if(!m_nRow && !m_nTableWidth && m_aTableGrid.size())
    {
        ::std::vector<sal_Int32>::const_iterator aCellIter = m_aTableGrid.begin();
        while( aCellIter != m_aTableGrid.end() )
             m_nTableWidth += *aCellIter++;
        if( m_nTableWidth > 0)
        {
            PropertyMapPtr pPropMap( new PropertyMap );
            pPropMap->Insert( PROP_WIDTH, false, uno::makeAny( m_nTableWidth ));
            insertTableProps(pPropMap);
        }
    }
    if( m_aGridSpans.size() < m_nCell)
    {
        //fill missing elements with '1'
        m_aGridSpans.insert( m_aGridSpans.end(), m_nCell - m_aGridSpans.size(), 1 );
    }
    //calculate number of used grids - it has to match the size of m_aTableGrid
    size_t nGrids = 0;
    ::std::vector<sal_Int32>::const_iterator aGridSpanIter = m_aGridSpans.begin();
    for( ; aGridSpanIter != m_aGridSpans.end(); ++aGridSpanIter)
        nGrids += *aGridSpanIter;

    if( m_aTableGrid.size() == nGrids )
    {
        //determine table width
        double nFullWidth = m_nTableWidth;
        //the positions have to be distibuted in a range of 10000
        const double nFullWidthRelative = 10000.;
        uno::Sequence< text::TableColumnSeparator > aSeparators( m_nCell - 1 );
        text::TableColumnSeparator* pSeparators = aSeparators.getArray();
        sal_Int16 nLastRelPos = 0;
        sal_uInt32 nBorderGridIndex = 0;
        for( sal_uInt32 nBorder = 0; nBorder < m_nCell - 1; ++nBorder )
        {
            sal_Int32 nGridCount = m_aGridSpans[nBorder];
            double fGridWidth = 0.;
            do
            {
                fGridWidth += m_aTableGrid[nBorderGridIndex++];
            }while( --nGridCount );

            sal_Int16 nRelPos =
                sal::static_int_cast< sal_Int16 >(fGridWidth * nFullWidthRelative / nFullWidth );

            pSeparators[nBorder].Position =  nRelPos + nLastRelPos;
            pSeparators[nBorder].IsVisible = sal_True;
            nLastRelPos = nLastRelPos + nRelPos;
        }
        PropertyMapPtr pPropMap( new PropertyMap );
        pPropMap->Insert( PROP_TABLE_COLUMN_SEPARATORS, false, uno::makeAny( aSeparators ) );
        insertRowProps(pPropMap);
    }

    ++m_nRow;
    m_nCell = 0;
    m_nCellBorderIndex = 0;
    m_aGridSpans.clear();
}
/*-- 18.06.2007 10:34:37---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapperTableManager::clearData()
{
    m_nRow = m_nCell = m_nCellBorderIndex = m_nHeaderRepeat = m_nGapHalf = m_nLeftMargin = m_nTableWidth = 0;
    m_bFullWidth = false;
    m_aTableGrid.clear();
    m_aGridSpans.clear();
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
        const StyleSheetEntry* pParentStyleSheet =
            pStyleSheetTable->FindStyleSheetByISTD(pStyleSheetEntry->sBaseStyleIdentifier);
        OSL_ENSURE( pParentStyleSheet, "table style not found" );
        lcl_CopyTextProperties( pToFill, pParentStyleSheet, pStyleSheetTable);
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
        const StyleSheetEntry* pStyleSheetEntry = pStyleSheetTable->FindStyleSheetByISTD(
                                                        m_sTableStyleName);
        OSL_ENSURE( pStyleSheetEntry, "table style not found" );
        lcl_CopyTextProperties(m_pTableStyleTextProperies, pStyleSheetEntry, pStyleSheetTable);
    }
    pContext->insert( m_pTableStyleTextProperies );
}
}}
