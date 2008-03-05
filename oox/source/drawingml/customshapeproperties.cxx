/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: customshapeproperties.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:19:11 $
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

#include <comphelper/processfactory.hxx>
#include "oox/drawingml/customshapeproperties.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/XGraphicTransformer.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>

using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::drawing;

namespace oox { namespace drawingml {

CustomShapeProperties::CustomShapeProperties()
{
}
CustomShapeProperties::~CustomShapeProperties()
{
}

void CustomShapeProperties::apply( const CustomShapePropertiesPtr& /* rSourceCustomShapeProperties */ )
{
    // not sure if this needs to be implemented
}

void CustomShapeProperties::pushToPropSet( const ::oox::core::XmlFilterBase& /* rFilterBase */,
    const Reference < XPropertySet >& xPropSet, const Reference < XShape > & xShape ) const
{
    const OUString sType = CREATE_OUSTRING( "Type" );
    if ( maShapePresetType.getLength() )
    {
        // XEnhancedCustomShapeDefaulter TODO
        //const uno::Reference < drawing::XShape > xShape( xPropSet, UNO_QUERY );
        Reference< drawing::XEnhancedCustomShapeDefaulter > xDefaulter( xShape, UNO_QUERY );
        if( xDefaulter.is() )
            xDefaulter->createCustomShapeDefaults( maShapePresetType );

        const rtl::OUString sCustomShapeGeometry( RTL_CONSTASCII_USTRINGPARAM( "CustomShapeGeometry" ) );
        uno::Any aGeoPropSet = xPropSet->getPropertyValue( sCustomShapeGeometry );
        uno::Sequence< beans::PropertyValue > aGeoPropSeq;
        if ( aGeoPropSet >>= aGeoPropSeq )
        {
            sal_Int32 i, nCount = aGeoPropSeq.getLength();
            for ( i = 0; i < nCount; i++ )
            {
                const rtl::OUString sAdjustmentValues( RTL_CONSTASCII_USTRINGPARAM( "AdjustmentValues" ) );
                if ( aGeoPropSeq[ i ].Name.equals( sAdjustmentValues ) )
                {
                    uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue > aAdjustmentSeq;
                    if ( aGeoPropSeq[ i ].Value >>= aAdjustmentSeq )
                    {
                        sal_uInt32 j, nHighest = 0;
                        for( j = 0; j < maAdjustmentValues.size(); j++ )
                        {
                            const rtl::OUString& rS( maAdjustmentValues[ j ].maName );
                            if ( rS.getLength() > 3 )
                            {
                                sal_uInt32 nVal = rS.copy( 3 ).toInt32();
                                if ( ( nVal < 10 ) && ( nVal > nHighest ) )
                                    nHighest = nVal;
                            }
                        }
                        if ( nHighest > static_cast< sal_uInt32 >( aAdjustmentSeq.getLength() ) )
                            aAdjustmentSeq.realloc( nHighest );

                        for ( j = 0; j < maAdjustmentValues.size(); j++ )
                        {
                            sal_uInt32 nVal = maAdjustmentValues[ j ].maName.copy( 3 ).toInt32();
                            if ( nVal-- )
                            {
                                double fNewAdj = getValue( maAdjustmentValues, nVal );
                                aAdjustmentSeq[ nVal ].State = beans::PropertyState_DIRECT_VALUE;
                                aAdjustmentSeq[ nVal ].Value <<= fNewAdj;
                            }
                        }
                        aGeoPropSeq[ i ].Value <<= aAdjustmentSeq;
                        xPropSet->setPropertyValue( sCustomShapeGeometry, Any( aGeoPropSeq ) );
                    }
                }
                else if ( aGeoPropSeq[ i ].Name.equals( sType ) )
                {
                    aGeoPropSeq[ i ].Value <<= maShapePresetType;
                }
            }
        }
    }
    else
    {
        PropertyMap aPropertyMap;
        OUString sShapeType( CREATE_OUSTRING( "non-primitive" ) );
        aPropertyMap[ sType ] <<= sShapeType;


        // converting the vector to a sequence
        Sequence< PropertyValue > aSeq;
        aPropertyMap.makeSequence( aSeq );
        static const rtl::OUString sCustomShapeGeometry( RTL_CONSTASCII_USTRINGPARAM( "CustomShapeGeometry" ) );
        xPropSet->setPropertyValue( sCustomShapeGeometry, Any( aSeq ) );
    }
}

double CustomShapeProperties::getValue( const std::vector< CustomShapeGuide >& rGuideList, sal_uInt32 nIndex ) const
{
    double fRet = 0.0;
    if ( nIndex < rGuideList.size() )
    {

    }
    return fRet;
}

} }
