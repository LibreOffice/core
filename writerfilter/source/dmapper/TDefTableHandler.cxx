/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TDefTableHandler.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: os $ $Date: 2007-05-09 13:50:21 $
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
#ifndef INCLUDED_TDEFTABLEHANDLER_HXX
#include <TDefTableHandler.hxx>
#endif
#ifndef INCLUDED_DMAPPER_PROPERTYMAP_HXX
#include <PropertyMap.hxx>
#endif
#ifndef INCLUDED_DMAPPER_CONVERSIONHELPER_HXX
#include <ConversionHelper.hxx>
#endif
#ifndef INCLUDED_QNAME_HXX
#include <odiapi/qname.hxx>
#endif
#ifndef INCLUDED_RESOURCESIDS
#include <doctok/resourceids.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_TABLECOLUMNSEPARATOR_HPP_
#include <com/sun/star/text/TableColumnSeparator.hpp>
#endif

namespace dmapper {

using namespace ::com::sun::star;
using namespace ::writerfilter;

/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
TDefTableHandler::TDefTableHandler()
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
TDefTableHandler::~TDefTableHandler()
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void TDefTableHandler::attribute(doctok::Id rName, doctok::Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    (void)nIntValue;
    (void)rName;
    switch( rName )
    {
        case NS_rtf::LN_cellx:
            // nIntValue contains the vert. line position
            //TODO: nIntValue is wrong for negative values!
            if( nIntValue > 0x7fff )
                nIntValue -= 0xffff;
            m_aCellBorders.push_back( ConversionHelper::convertToMM100( nIntValue ) );
        break;
        case NS_rtf::LN_tc:
        //from LN_tc
        case NS_rtf::LN_BRCTOP:
        case NS_rtf::LN_BRCLEFT:
        case NS_rtf::LN_BRCBOTTOM:
        case NS_rtf::LN_BRCRIGHT:
        {
            doctok::Reference<doctok::Properties>::Pointer_t pProperties = rVal.getProperties();
            if( pProperties.get())
            {
                pProperties->resolve( *this );
            }
        }
        break;
        //from LN_tc
        case NS_rtf::LN_FFIRSTMERGED:
        case NS_rtf::LN_FMERGED:
        case NS_rtf::LN_FVERTICAL:
        case NS_rtf::LN_FBACKWARD:
        case NS_rtf::LN_FROTATEFONT:
        case NS_rtf::LN_FVERTMERGE:
        case NS_rtf::LN_FVERTRESTART:
        case NS_rtf::LN_VERTALIGN:
        case NS_rtf::LN_FUNUSED:
        case NS_rtf::LN_CellPrefferedSize:
        break;
        //from LN_BRCXXXX
        case NS_rtf::LN_DPTLINEWIDTH:
        case NS_rtf::LN_BRCTYPE:
        case NS_rtf::LN_ICO:
        case NS_rtf::LN_DPTSPACE:
        case NS_rtf::LN_FSHADOW:
        case NS_rtf::LN_FFRAME:
        case NS_rtf::LN_UNUSED2_15:
        break;
        default:
            OSL_ASSERT("unknown attribute");
    }
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void TDefTableHandler::sprm(doctok::Sprm & rSprm)
{
    (void)rSprm;
}
/*-- 24.04.2007 09:09:01---------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyMapPtr  TDefTableHandler::getProperties() const
{
    PropertyMapPtr pPropertyMap(new PropertyMap);

    // Writer only wants the separators, Word provides also the outer border positions
    if( m_aCellBorders.size() > 2 )
    {
        //determine table width
        double nFullWidth = m_aCellBorders[m_aCellBorders.size() - 1] - m_aCellBorders[0];
        //the positions have to be distibuted in a range of 10000
        const double nFullWidthRelative = 10000.;
        uno::Sequence< text::TableColumnSeparator > aSeparators( m_aCellBorders.size() - 2 );
        text::TableColumnSeparator* pSeparators = aSeparators.getArray();
        for( sal_uInt32 nBorder = 1; nBorder < m_aCellBorders.size() - 1; ++nBorder )
        {
            sal_Int16 nRelPos =
                sal::static_int_cast< sal_Int16 >(double(m_aCellBorders[nBorder]) * nFullWidthRelative / nFullWidth );

            pSeparators[nBorder - 1].Position =  nRelPos;
            pSeparators[nBorder - 1].IsVisible = sal_True;
        }
        pPropertyMap->Insert( PROP_TABLE_COLUMN_SEPARATORS, uno::makeAny( aSeparators ) );
    }

    return pPropertyMap;
}
/*-- 09.05.2007 13:14:17---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 TDefTableHandler::getTableWidth() const
{
    sal_Int32 nWidth = 0;
    if( m_aCellBorders.size() > 1 )
    {
        //determine table width
        nWidth = m_aCellBorders[m_aCellBorders.size() - 1] - m_aCellBorders[0];
    }
    return nWidth;
}

} //namespace dmapper
