/*************************************************************************
 *
 *  $RCSfile: StatisticsItemConverter.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-12-09 16:27:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "StatisticsItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ItemPropertyMap.hxx"
#include "RegressionCurveHelper.hxx"

#include "GraphicPropertyItemConverter.hxx"
#include "CharacterPropertyItemConverter.hxx"

#ifndef _SVX_CHRTITEM_HXX
#define ITEMID_DOUBLE SCHATTR_STAT_PERCENT
#include <svx/chrtitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_DATAPOINTLABEL_HPP_
#include <drafts/com/sun/star/chart2/DataPointLabel.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XREGRESSIONCURVECONTAINER_HPP_
#include <drafts/com/sun/star/chart2/XRegressionCurveContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif

#include <functional>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

namespace
{
::comphelper::ItemPropertyMapType & lcl_GetDataPointPropertyMap()
{
    static ::comphelper::ItemPropertyMapType aDataPointPropertyMap(
        ::comphelper::MakeItemPropertyMap
        ( CHATTR_PIE_SEGMENT_OFFSET,      C2U( "Offset" ))
        /*
          SCHATTR_STAT_AVERAGE                    SfxBoolItem
          SCHATTR_STAT_KIND_ERROR                 SfxInt32Item
          SCHATTR_STAT_PERCENT                    SvxDoubleItem
          SCHATTR_STAT_BIGERROR                   SvxDoubleItem
          SCHATTR_STAT_CONSTPLUS                  SvxDoubleItem
          SCHATTR_STAT_CONSTMINUS                 SvxDoubleItem
          SCHATTR_STAT_REGRESSTYPE                SfxInt32Item
          SCHATTR_STAT_INDICATE                   SfxInt32Item
        */
        );

    return aDataPointPropertyMap;
};

bool lcl_HasMeanValueLine( const uno::Reference< chart2::XRegressionCurveContainer > & xRegCnt )
{
    bool bResult = false;

    if( !xRegCnt.is())
        return bResult;

    try
    {
        uno::Sequence< uno::Reference< chart2::XRegressionCurve > > aCurves(
            xRegCnt->getRegressionCurves());
        for( sal_Int32 i = 0; i < aCurves.getLength(); ++i )
        {
            uno::Reference< lang::XServiceName > xServName( aCurves[i], uno::UNO_QUERY );
            if( xServName.is() &&
                xServName->getServiceName().equals(
                    C2U( "com.sun.star.comp.chart2.MeanValueRegressionCurve" )))
            {
                bResult = true;
                break;
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return bResult;
}

void lcl_RemoveMeanValueLine( uno::Reference< chart2::XRegressionCurveContainer > & xRegCnt )
{
    if( !xRegCnt.is())
        return;
    OSL_ASSERT( lcl_HasMeanValueLine( xRegCnt ));

    try
    {
        uno::Sequence< uno::Reference< chart2::XRegressionCurve > > aCurves(
            xRegCnt->getRegressionCurves());
        for( sal_Int32 i = 0; i < aCurves.getLength(); ++i )
        {
            uno::Reference< lang::XServiceName > xServName( aCurves[i], uno::UNO_QUERY );
            if( xServName.is() &&
                xServName->getServiceName().equals(
                    C2U( "com.sun.star.comp.chart2.MeanValueRegressionCurve" )))
            {
                // note: assume that there is only one mean-value curve
                xRegCnt->removeRegressionCurve( aCurves[i] );
                // attention: the iterator i has become invalid now
                break;
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void lcl_AddMeanValueLine( uno::Reference< chart2::XRegressionCurveContainer > & xRegCnt )
{
    if( !xRegCnt.is())
        return;
    OSL_ASSERT( ! lcl_HasMeanValueLine( xRegCnt ));

    // todo: use a valid context
    xRegCnt->addRegressionCurve( ::chart::RegressionCurveHelper::createMeanValueLine(
                                     uno::Reference< uno::XComponentContext >() ));
}

} // anonymous namespace

namespace chart
{
namespace wrapper
{

StatisticsItemConverter::StatisticsItemConverter(
    const uno::Reference<
    beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool ) :
        ItemConverter( rPropertySet, rItemPool )
{
}

StatisticsItemConverter::~StatisticsItemConverter()
{
    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ::comphelper::DeleteItemConverterPtr() );
}

void StatisticsItemConverter::FillItemSet( SfxItemSet & rOutItemSet ) const
{
    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ::comphelper::FillItemSetFunc( rOutItemSet ));

    // own items
    ItemConverter::FillItemSet( rOutItemSet );
}

bool StatisticsItemConverter::ApplyItemSet( const SfxItemSet & rItemSet )
{
    bool bResult = false;

    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ::comphelper::ApplyItemSetFunc( rItemSet, bResult ));

    // own items
    return ItemConverter::ApplyItemSet( rItemSet ) || bResult;
}

const USHORT * StatisticsItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nStatWhichPairs;
}

bool StatisticsItemConverter::GetItemPropertyName( USHORT nWhichId, ::rtl::OUString & rOutName ) const
{
    ::comphelper::ItemPropertyMapType & rMap( lcl_GetDataPointPropertyMap());
    ::comphelper::ItemPropertyMapType::const_iterator aIt( rMap.find( nWhichId ));

    if( aIt == rMap.end())
        return false;

    rOutName =(*aIt).second;
    return true;
}


bool StatisticsItemConverter::ApplySpecialItem(
    USHORT nWhichId, const SfxItemSet & rItemSet )
    throw( uno::Exception )
{
    bool bChanged = false;
    uno::Any aValue;

    switch( nWhichId )
    {
        case SCHATTR_STAT_AVERAGE:
        {
            uno::Reference< chart2::XRegressionCurveContainer > xRegCnt(
                GetPropertySet(), uno::UNO_QUERY );
            bool bOldHasMeanValueLine = lcl_HasMeanValueLine( xRegCnt );

            bool bNewHasMeanValueLine =
                reinterpret_cast< const SfxBoolItem & >( rItemSet.Get( nWhichId )).GetValue();

            if( bOldHasMeanValueLine != bNewHasMeanValueLine )
            {
                if( ! bNewHasMeanValueLine )
                    lcl_RemoveMeanValueLine( xRegCnt );
                else
                    lcl_AddMeanValueLine( xRegCnt );
                bChanged = true;
            }
        }
        break;
        case SCHATTR_STAT_KIND_ERROR:
            // SfxInt32Item
            break;
        case SCHATTR_STAT_PERCENT:
            // SvxDoubleItem
            break;
        case SCHATTR_STAT_BIGERROR:
            // SvxDoubleItem
            break;
        case SCHATTR_STAT_CONSTPLUS:
            // SvxDoubleItem
            break;
        case SCHATTR_STAT_CONSTMINUS:
            // SvxDoubleItem
            break;
        case SCHATTR_STAT_REGRESSTYPE:
            // SfxInt32Item
            break;
        case SCHATTR_STAT_INDICATE:
            // SfxInt32Item
            break;
    }

    return bChanged;
}

void StatisticsItemConverter::FillSpecialItem(
    USHORT nWhichId, SfxItemSet & rOutItemSet ) const
    throw( uno::Exception )
{
    switch( nWhichId )
    {
        case SCHATTR_STAT_AVERAGE:
            rOutItemSet.Put(
                SfxBoolItem( nWhichId,
                             lcl_HasMeanValueLine(
                                 uno::Reference< chart2::XRegressionCurveContainer >(
                                     GetPropertySet(), uno::UNO_QUERY ))));
            break;
        case SCHATTR_STAT_KIND_ERROR:
            // SfxInt32Item
            break;
        case SCHATTR_STAT_PERCENT:
            // SvxDoubleItem
            break;
        case SCHATTR_STAT_BIGERROR:
            // SvxDoubleItem
            break;
        case SCHATTR_STAT_CONSTPLUS:
            // SvxDoubleItem
            break;
        case SCHATTR_STAT_CONSTMINUS:
            // SvxDoubleItem
            break;
        case SCHATTR_STAT_REGRESSTYPE:
            // SfxInt32Item
            break;
        case SCHATTR_STAT_INDICATE:
            // SfxInt32Item
            break;
   }
}

} //  namespace wrapper
} //  namespace chart
