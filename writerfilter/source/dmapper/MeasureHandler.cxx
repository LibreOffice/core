
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MeasureHandler.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:52:05 $
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
#include <MeasureHandler.hxx>
#include <PropertyMap.hxx>
#include <doctok/resourceids.hxx>
#include <ConversionHelper.hxx>
#include <ooxml/resourceids.hxx>
#include <com/sun/star/text/SizeType.hpp>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;

/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
MeasureHandler::MeasureHandler() :
    m_nMeasureValue( 0 ),
    m_nUnit( -1 ),
    m_nRowHeightSizeType( text::SizeType::MIN )
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
MeasureHandler::~MeasureHandler()
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void MeasureHandler::attribute(Id rName, Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    (void)rName;
    /* WRITERFILTERSTATUS: table: MeasureHandler_attributedata */
    switch( rName )
    {
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_unit:
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_TblWidth_type:// = 90668;
            //can be: NS_ooxml::LN_Value_ST_TblWidth_nil, NS_ooxml::LN_Value_ST_TblWidth_pct,
            //        NS_ooxml::LN_Value_ST_TblWidth_dxa, NS_ooxml::LN_Value_ST_TblWidth_auto;
            m_nUnit = nIntValue;
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Height_hRule: // 90666;
        {
            ::rtl::OUString sHeightType = rVal.getString();
            if( sHeightType.equalsAscii( "exact" ) )
                m_nRowHeightSizeType = text::SizeType::FIX;
        }
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_trleft:
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_preferredWidth:
        case NS_ooxml::LN_CT_TblWidth_w:// = 90667;
            m_nMeasureValue = nIntValue;
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Height_val: // 90665 -- a string value
        {
            m_nUnit = NS_ooxml::LN_Value_ST_TblWidth_dxa;
            ::rtl::OUString sHeight = rVal.getString();
            m_nMeasureValue = sHeight.toInt32();
        }
        break;
        default:
            OSL_ENSURE( false, "unknown attribute");
    }
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void MeasureHandler::sprm(Sprm & rSprm)
{
    (void)rSprm;
}
/*-- 24.04.2007 09:09:01---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 MeasureHandler::getMeasureValue() const
{
    sal_Int32 nRet = 0;
    if( m_nMeasureValue != 0 && m_nUnit >= 0 )
    {
        // TODO m_nUnit 3 - twip, other values unknown :-(
        if( m_nUnit == 3 || sal::static_int_cast<Id>(m_nUnit) == NS_ooxml::LN_Value_ST_TblWidth_dxa)
            nRet = ConversionHelper::convertTwipToMM100( m_nMeasureValue );
        //todo: handle additional width types:
        //NS_ooxml::LN_Value_ST_TblWidth_nil, NS_ooxml::LN_Value_ST_TblWidth_pct,
        //NS_ooxml::LN_Value_ST_TblWidth_dxa, NS_ooxml::LN_Value_ST_TblWidth_auto;
    }
    return nRet;
}
/*-- 18.06.2007 10:24:26---------------------------------------------------

  -----------------------------------------------------------------------*/
bool MeasureHandler::isAutoWidth() const
{
    return sal::static_int_cast<Id>(m_nUnit) == NS_ooxml::LN_Value_ST_TblWidth_auto;
}

} //namespace dmapper
} //namespace writerfilter
