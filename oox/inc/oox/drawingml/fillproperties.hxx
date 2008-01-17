/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fillproperties.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:45 $
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

#ifndef OOX_DRAWINGML_FILLPROPERTIES_HXX
#define OOX_DRAWINGML_FILLPROPERTIES_HXX

#include "oox/helper/propertymap.hxx"
#include "oox/drawingml/color.hxx"
#include <com/sun/star/graphic/XGraphic.hpp>
#include <boost/shared_ptr.hpp>
#include "tokens.hxx"
#include <vector>
#include <map>

namespace oox { namespace drawingml {

class FillProperties;
class BlipFillPropertiesContext;

typedef boost::shared_ptr< FillProperties > FillPropertiesPtr;

class FillProperties
{
    friend class BlipFillPropertiesContext;

public:

    FillProperties( sal_Int32 nContext = XML_spPr );
    virtual ~FillProperties();

    void apply( const FillPropertiesPtr& );
    void pushToPropSet( const ::oox::core::XmlFilterBase& rFilterBase,
            const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > & xPropSet ) const;

    PropertyMap&                getFillPropertyMap(){ return maFillProperties; };
    ::oox::drawingml::ColorPtr& getFillColor(){ return maFillColor; };
    ::oox::drawingml::ColorPtr& getColorChangeFrom(){ return maColorChangeFrom; };
    ::oox::drawingml::ColorPtr& getColorChangeTo(){ return maColorChangeTo; };

private:

    // creates the graphic for a graphic object and its corresponding properties
    void createTransformedGraphic( const oox::core::XmlFilterBase& rFilterBase,
        const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > & xPropSet ) const;

    sal_Int32                                                               mnContext;
    PropertyMap                                                             maFillProperties;
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > mxGraphic;
    ::oox::drawingml::ColorPtr                                              maFillColor;
    ::oox::drawingml::ColorPtr                                              maColorChangeFrom;
    ::oox::drawingml::ColorPtr                                              maColorChangeTo;
};

} }

#endif  //  OOX_DRAWINGML_FILLPROPERTIES_HXX
