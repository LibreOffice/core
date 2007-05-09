/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MeasureHandler.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: os $ $Date: 2007-05-09 13:49:12 $
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
#ifndef INCLUDED_MEASUREHANDLER_HXX
#include <MeasureHandler.hxx>
#endif
#ifndef INCLUDED_DMAPPER_PROPERTYMAP_HXX
#include <PropertyMap.hxx>
#endif
#ifndef INCLUDED_RESOURCESIDS
#include <doctok/resourceids.hxx>
#endif
#ifndef INCLUDED_DMAPPER_CONVERSIONHELPER_HXX
#include <ConversionHelper.hxx>
#endif
#ifndef INCLUDED_QNAME_HXX
#include <odiapi/qname.hxx>
#endif

using namespace ::writerfilter;
namespace dmapper {

using namespace ::com::sun::star;

/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
MeasureHandler::MeasureHandler() :
    m_nMeasureValue( 0 ),
    m_nUnit( -1 )
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
MeasureHandler::~MeasureHandler()
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void MeasureHandler::attribute(doctok::Id rName, doctok::Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    (void)rName;
    switch( rName )
    {
        case NS_rtf::LN_unit:
            m_nUnit = nIntValue;
        break;
        case NS_rtf::LN_trleft:
        case NS_rtf::LN_preferredWidth:
            m_nMeasureValue = nIntValue;
        break;
        default:
            OSL_ASSERT("unknown attribute");
    }
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void MeasureHandler::sprm(doctok::Sprm & rSprm)
{
    (void)rSprm;
}
/*-- 24.04.2007 09:09:01---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 MeasureHandler::getTwipValue() const
{
    sal_Int32 nRet = 0;
    if( m_nMeasureValue != 0 && m_nUnit >= 0 )
    {
        // TODO m_nUnit 3 - twip, other values unknown :-(
        if( m_nUnit == 3 )
            nRet = ConversionHelper::convertToMM100( m_nMeasureValue );
    }
    return nRet;
}
} //namespace dmapper
