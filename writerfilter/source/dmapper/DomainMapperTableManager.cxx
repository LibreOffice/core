/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DomainMapperTableManager.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: os $ $Date: 2007-05-07 12:05:13 $
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
#ifndef INCLUDED_BORDERHANDLER_HXX
#include <BorderHandler.hxx>
#endif
#ifndef INCLUDED_CELLCOLORHANDLER_HXX
#include <CellColorHandler.hxx>
#endif
#ifndef INCLUDED_TABLELEFTINDENTHANDLER_HXX
#include <TableLeftIndentHandler.hxx>
#endif
#ifndef INCLUDED_DMAPPER_CONVERSIONHELPER_HXX
#include <ConversionHelper.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HDL_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_SIZETYPE_HDL_
#include <com/sun/star/text/SizeType.hpp>
#endif

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
    m_nGapHalf(0)
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
        sal_uInt32 nId = rSprm.getId();
        doctok::Value::Pointer_t pValue = rSprm.getValue();
        sal_Int32 nIntValue = ((pValue.get() != NULL) ? pValue->getInt() : 0);
        /* WRITERFILTERSTATUS: table: table_sprmdata */
        switch( nId )
        {
            case 0x5400: // sprmTJc
            {
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
                //table justification 0: left, 1: center, 2: right
                sal_Int16 nOrient = text::HoriOrientation::LEFT;
                switch( nIntValue )
                {
                    case 1 : nOrient = text::HoriOrientation::CENTER; break;
                    case 2 : nOrient = text::HoriOrientation::RIGHT; break;
                    case 0 :
                    //no break
                    default:;

                }
                PropertyMapPtr pTableMap( new PropertyMap );
                pTableMap->Insert( PROP_HORI_ORIENT, uno::makeAny( nOrient ) );
                insertTableProps( pTableMap );
            }
            break;
            case 0x9601: // sprmTDxaLeft
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
            break;
            case 0x9602: // sprmTDxaGapHalf
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
                m_nGapHalf = nIntValue;
            break;
            case 0xf661 : //sprmTTRLeft - contains unit and value of left table indent
                {
                doctok::Reference<doctok::Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {   //contains attributes x2902 (LN_unit) and x17e2 (LN_trleft)
                    TableLeftIndentHandlerPtr pTableLeftIndentHandler( new TableLeftIndentHandler );
                    pProperties->resolve(*pTableLeftIndentHandler);
                    insertTableProps(pTableLeftIndentHandler->getProperties());
                }
            }
            break;
            case 0x3403: // sprmTFCantSplit
            {
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0.5 */
                //row can't break across pages if nIntValue == 1
                PropertyMapPtr pPropMap( new PropertyMap );
                pPropMap->Insert( PROP_IS_SPLIT_ALLOWED, uno::makeAny(sal_Bool( nIntValue == 1 ? sal_False : sal_True ) ));
                insertRowProps(pPropMap);
            }
            break;
            case 0x3404:// sprmTTableHeader
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
                // if nIntValue == 1 then the row is a repeated header line
                // to prevent later rows from increasing the repeating m_nHeaderRepeat is set to NULL when repeating stops
                if( nIntValue > 0 && m_nHeaderRepeat >= 0 )
                {
                    ++m_nHeaderRepeat;
                    PropertyMapPtr pPropMap( new PropertyMap );
                    pPropMap->Insert( PROP_HEADER_ROW_COUNT, uno::makeAny( m_nHeaderRepeat ));
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
                pPropMap->Insert( PROP_SIZE_TYPE, uno::makeAny(bMinHeight ? text::SizeType::MIN : text::SizeType::FIX ));
                pPropMap->Insert( PROP_HEIGHT, uno::makeAny(ConversionHelper::convertToMM100( nHeight )));
                insertRowProps(pPropMap);
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
            default: bRet = false;
        }
    }
    return bRet;
}
/*-- 26.04.2007 08:20:08---------------------------------------------------

  -----------------------------------------------------------------------*/
//void DomainMapperTableManager::attribute(doctok::Id nName, doctok::Value & rVal)
//{
//    sal_Int32 nIntValue = rVal.getInt();
//    rtl::OUString sStringValue = val.getString();
//    /* WRITERFILTERSTATUS: table: tablemanager_attributedata */
//    switch( nName )
//    {
//        case NS_rtf::LN_cellTopColor:
//            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
//            break;
//        case NS_rtf::LN_cellLeftColor:
//            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
//            break;
//        case NS_rtf::LN_cellBottomColor:
//            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
//            break;
//        case NS_rtf::LN_cellRightColor:
//            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
//            break;
//    }

/*-- 02.05.2007 14:36:26---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapperTableManager::endCell()
{
    DomainMapperTableManager_Base_t::endCell();
    ++m_nCell;
}
/*-- 02.05.2007 14:36:26---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapperTableManager::endRow()
{
    DomainMapperTableManager_Base_t::endRow();
    ++m_nRow;
    m_nCell = 0;
    m_nCellBorderIndex = 0;
}


}
