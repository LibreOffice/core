/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DomainMapperTableManager.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-29 15:29:11 $
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
#include <doctok/WW8ResourceModel.hxx>
#include <BorderHandler.hxx>
#include <CellColorHandler.hxx>
#include <ConversionHelper.hxx>
#include <MeasureHandler.hxx>
#include <TDefTableHandler.hxx>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <ooxml/resourceids.hxx>
#include <resourcemodel/sprmids.hxx>

using namespace ::writerfilter;
namespace dmapper {

using namespace ::com::sun::star;
using namespace ::std;

/*-- 23.04.2007 14:57:49---------------------------------------------------

  -----------------------------------------------------------------------*/
DomainMapperTableManager::DomainMapperTableManager() :
    m_nRow(0),
    m_nCell(0),
    m_nCellBorderIndex(0),
    m_nHeaderRepeat(0),
    m_nGapHalf(0),
    m_nLeftMargin(0),
    m_nTableWidth(0),
    m_bFullWidth(false)
{
}
/*-- 23.04.2007 14:57:49---------------------------------------------------

  -----------------------------------------------------------------------*/
DomainMapperTableManager::~DomainMapperTableManager()
{
}
/*-- 23.04.2007 15:25:37---------------------------------------------------

  -----------------------------------------------------------------------*/
bool DomainMapperTableManager::sprm(doctok::Sprm & rSprm)
{
    bool bRet = DomainMapperTableManager_Base_t::sprm(rSprm);
    if( !bRet )
    {
        bRet = true;
        sal_uInt32 nSprmId = rSprm.getId();
        doctok::Value::Pointer_t pValue = rSprm.getValue();
        sal_Int32 nIntValue = ((pValue.get() != NULL) ? pValue->getInt() : 0);
        /* WRITERFILTERSTATUS: table: table_sprmdata */
        switch( nSprmId )
        {
            case NS_ooxml::LN_CT_TrPrBase_jc: //90706
            case 0x5400: // sprmTJc
            if( !m_bFullWidth )
            {
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
                //table justification 0: left, 1: center, 2: right
                sal_Int16 nOrient = text::HoriOrientation::LEFT_AND_WIDTH;
                switch( nIntValue )
                {
                    case 1 : nOrient = text::HoriOrientation::CENTER; break;
                    case 2 : nOrient = text::HoriOrientation::RIGHT; break;
                    case 0 :
                    //no break
                    default:;

                }
                PropertyMapPtr pTableMap( new PropertyMap );
                pTableMap->Insert( PROP_HORI_ORIENT, false, uno::makeAny( nOrient ) );
                insertTableProps( pTableMap );
            }
            break;
            case 0x9601: // sprmTDxaLeft
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
            break;
            case 0x9602: // sprmTDxaGapHalf
            {
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
                m_nGapHalf = ConversionHelper::convertToMM100( nIntValue );
                PropertyMapPtr pPropMap( new PropertyMap );
                pPropMap->Insert( PROP_LEFT_MARGIN, false, uno::makeAny( m_nLeftMargin - m_nGapHalf ));
                if( m_bFullWidth && m_nLeftMargin > 0 )
                    pPropMap->Insert( PROP_HORI_ORIENT, false, uno::makeAny( text::HoriOrientation::RIGHT ) );
                insertTableProps(pPropMap);
            }
            break;
            case 0xf661: //sprmTTRLeft left table indent
            case 0xf614: // sprmTTPreferredWidth - preferred table width
            case NS_ooxml::LN_CT_TblPrBase_tblW:  //90722;
            case NS_ooxml::LN_CT_TblPrBase_tblInd: //90725
            {
                //contains unit and value
                doctok::Reference<doctok::Properties>::Pointer_t pProperties = rSprm.getProps();
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
            case NS_ooxml::LN_CT_TrPrBase_trHeight: //90703
            {
                //contains unit and value
                doctok::Reference<doctok::Properties>::Pointer_t pProperties = rSprm.getProps();
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
            case 0x3403: // sprmTFCantSplit
            case NS_sprm::LN_TCantSplit: // 0x3644
            {
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0.5 */
                //row can't break across pages if nIntValue == 1
                PropertyMapPtr pPropMap( new PropertyMap );
                pPropMap->Insert( PROP_IS_SPLIT_ALLOWED, false, uno::makeAny(sal_Bool( nIntValue == 1 ? sal_False : sal_True ) ));
                insertRowProps(pPropMap);
            }
            break;
            case 0x3404:// sprmTTableHeader
            case NS_ooxml::LN_CT_TrPrBase_tblHeader: //90704
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
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
            case 0x9407: // sprmTDyaRowHeight
            {
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0.5 */
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
                pPropMap->Insert( PROP_HEIGHT, false, uno::makeAny(ConversionHelper::convertToMM100( nHeight )));
                insertRowProps(pPropMap);
            }
            break;
            case 0xd608: // TDefTable
            {
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
                doctok::Reference<doctok::Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    TDefTableHandlerPtr pTDefTableHandler( new TDefTableHandler );
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
            case NS_ooxml::LN_CT_TcPrBase_vAlign://90694
            {
                sal_Int16 nVertOrient = text::VertOrientation::NONE;
                switch( nIntValue ) //0 - top 1 - center 2 - bottom
                {
                    case 1: nVertOrient = text::VertOrientation::CENTER; break;
                    case 2: nVertOrient = text::VertOrientation::BOTTOM; break;
                    default:;
                };
                PropertyMapPtr pPropMap( new PropertyMap() );
                pPropMap->Insert( PROP_VERT_ORIENT, false, uno::makeAny( nVertOrient ) );
                //todo: in ooxml import the value of m_ncell is wrong
                cellProps( pPropMap );
            }
            break;
            case 0xD605: // sprmTTableBorders
            {
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
                doctok::Reference<doctok::Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    BorderHandlerPtr pBorderHandler( new BorderHandler );
                    pProperties->resolve(*pBorderHandler);
                    cellPropsByCell( m_nCellBorderIndex, PropertyMapPtr( pBorderHandler->getProperties()) );
                    ++m_nCellBorderIndex;
                }
            }
            break;
            case 0xd61a : // sprmTCellTopColor
            case 0xd61b : // sprmTCellLeftColor
            case 0xd61c : // sprmTCellBottomColor
            case 0xd61d : // sprmTCellRightColor
            {
                // each color sprm contains as much colors as cells are in a row
                doctok::Reference<doctok::Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    CellColorHandlerPtr pCellColorHandler( new CellColorHandler );
                    pProperties->resolve( *pCellColorHandler );
//                    cellPropsByCell(0, PropertyMapPtr( pColorHandler->getProperties()) );
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
//OOXML table properties
            case NS_ooxml::LN_CT_TblPrBase_tblStyle: //table style name
            {
                m_sTableStyleName = pValue->getString();
                PropertyMapPtr pPropMap( new PropertyMap );
                pPropMap->Insert( META_PROP_TABLE_STYLE_NAME, false, uno::makeAny( m_sTableStyleName ));
                insertTableProps(pPropMap);
            }
            break;
            case NS_ooxml::LN_CT_TcPrBase_tcW: //90684 - column width values
            //TODO: this is wrong - it should be something like CT_TblGridBase
            if(!m_nRow)
            {
                doctok::Reference<doctok::Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    MeasureHandlerPtr pMeasureHandler( new MeasureHandler );
                    pProperties->resolve(*pMeasureHandler);
                    sal_Int32 nColumnWidth = pMeasureHandler->getMeasureValue();
                    m_aCellWidths.push_back( nColumnWidth );
                }
            }
            break;
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
    if(!m_nRow && !m_nTableWidth && m_aCellWidths.size())
    {
        ::std::vector<sal_Int32>::const_iterator aCellIter = m_aCellWidths.begin();
        while( aCellIter != m_aCellWidths.end() )
             m_nTableWidth += *aCellIter++;
        if( m_nTableWidth > 0)
        {
            PropertyMapPtr pPropMap( new PropertyMap );
            pPropMap->Insert( PROP_WIDTH, false, uno::makeAny( m_nTableWidth ));
            insertTableProps(pPropMap);
        }
    }
    ++m_nRow;
    m_nCell = 0;
    m_nCellBorderIndex = 0;
}
/*-- 18.06.2007 10:34:37---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapperTableManager::clearData()
{
    m_nRow = m_nCell = m_nCellBorderIndex = m_nHeaderRepeat = m_nGapHalf = m_nLeftMargin = m_nTableWidth = 0;
    m_bFullWidth = false;
    m_aCellWidths.clear();
    m_sTableStyleName = ::rtl::OUString();
}

}
