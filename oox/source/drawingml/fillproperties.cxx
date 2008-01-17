/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fillproperties.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:51 $
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
#include "oox/drawingml/fillproperties.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/XGraphicTransformer.hpp>

using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::graphic;

namespace oox { namespace drawingml {

FillProperties::FillProperties( sal_Int32 nContext )
: mnContext( nContext )
, maFillColor( new Color() )
{
}
FillProperties::~FillProperties()
{
}

void FillProperties::apply( const FillPropertiesPtr& rSourceFillProperties )
{
    maFillProperties.insert( rSourceFillProperties->maFillProperties.begin(), rSourceFillProperties->maFillProperties.end() );
    if ( rSourceFillProperties->maFillColor->isUsed() )
        maFillColor = rSourceFillProperties->maFillColor;
}

void FillProperties::pushToPropSet( const ::oox::core::XmlFilterBase& rFilterBase,
    const Reference < XPropertySet >& xPropSet ) const
{
    PropertySet aPropSet( xPropSet );
    Sequence< OUString > aNames;
    Sequence< Any > aValues;

    maFillProperties.makeSequence( aNames, aValues );
    aPropSet.setProperties( aNames, aValues );
    if ( maFillColor->isUsed() )
    {
        const rtl::OUString sFillColor( OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "FillColor" ) ) );
        xPropSet->setPropertyValue( sFillColor, Any( maFillColor->getColor( rFilterBase ) ) );
    }
    createTransformedGraphic( rFilterBase, xPropSet );
}

void FillProperties::createTransformedGraphic( const oox::core::XmlFilterBase& rFilterBase, const Reference < XPropertySet >& xPropSet ) const
{
    if( mxGraphic.is() )
    {
        if ( mnContext == XML_pic )
        {
            Reference< XGraphic > xGraphic( mxGraphic );
            if ( maColorChangeFrom.get() && maColorChangeTo.get() )
            {
                sal_Int32 nClrChangeFrom = maColorChangeFrom->getColor( rFilterBase );
                sal_Int32 nClrChangeTo = maColorChangeTo->getColor( rFilterBase );
                sal_Int32 nAlphaTo = maColorChangeTo->getAlpha();
                if ( ( nClrChangeFrom != nClrChangeTo ) || ( maColorChangeTo->hasAlpha() && ( nAlphaTo != 1000000 ) ) )
                {
                    Reference< XGraphicTransformer > xTransformer( xGraphic, UNO_QUERY );
                    if ( xTransformer.is() )
                        xGraphic = xTransformer->colorChange( xGraphic, nClrChangeFrom, 9, nClrChangeTo, static_cast< sal_Int8 >( ( nAlphaTo / 39062 ) ) );
                }
            }
            static const OUString sGraphic( CREATE_OUSTRING( "Graphic" ) );
            xPropSet->setPropertyValue( sGraphic, Any( xGraphic ) );
        }
        else if ( mnContext == XML_spPr )
        {
            static const OUString sFillBitmap( CREATE_OUSTRING( "FillBitmap" ) );
            xPropSet->setPropertyValue( sFillBitmap, Any( mxGraphic ) );
        }
    }
}

} }
