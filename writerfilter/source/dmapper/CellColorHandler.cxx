/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CellColorHandler.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:49:31 $
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
#include <CellColorHandler.hxx>
#include <PropertyMap.hxx>
#include <doctok/resourceids.hxx>
#include <ConversionHelper.hxx>
#include <ooxml/resourceids.hxx>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;
using namespace ::writerfilter;
//using namespace ::std;

/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
CellColorHandler::CellColorHandler() :
    m_nShadowType( 0 ),
    m_nColor( 0xffffffff ),
    m_nFillColor( 0xffffffff ),
    m_bOOXMLColor( false ),
    m_bParagraph( false )
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
CellColorHandler::~CellColorHandler()
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void CellColorHandler::attribute(Id rName, Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    (void)nIntValue;
    (void)rName;
    /* WRITERFILTERSTATUS: table: CellColor_attributedata */
    switch( rName )
    {
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_cellTopColor:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_cellLeftColor:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_cellBottomColor:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_cellRightColor:
            // nIntValue contains the color, directly
        break;
        /* WRITERFILTERSTATUS: done: 50, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Shd_val:
        {
            //might be clear, pct5...90, some hatch types
            //TODO: The values need symbolic names!
            m_nShadowType = nIntValue; //clear == 0, solid: 1, pct5: 2, pct50:8, pct95: x3c, horzStripe:0x0e, thinVertStripe: 0x15
            m_bOOXMLColor = true;
        }
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Shd_fill:
            m_nFillColor = nIntValue;
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Shd_color:
            //color of the shading
            m_nColor = nIntValue;
        break;
//        case NS_rtf::LN_rgbrc:
//        {
//            writerfilter::Reference<Properties>::Pointer_t pProperties = rVal.getProperties();
//            if( pProperties.get())
//            {
//                pProperties->resolve(*this);
//                //
//            }
//        }
//        break;
        default:
            OSL_ENSURE( false, "unknown attribute");
    }
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void CellColorHandler::sprm(Sprm & rSprm)
{
    (void)rSprm;
}
/*-- 24.04.2007 09:09:01---------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyMapPtr  CellColorHandler::getProperties()
{
    PropertyMapPtr pPropertyMap(new PropertyMap);
    if( m_bOOXMLColor )
    {
        pPropertyMap->Insert( m_bParagraph ? PROP_PARA_BACK_COLOR : PROP_BACK_COLOR, false,
                                uno::makeAny( m_nShadowType ? m_nColor : m_nFillColor ));
    }
    return pPropertyMap;
}
} //namespace dmapper
} //namespace writerfilter
