/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textcharacterproperties.hxx,v $
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

#ifndef OOX_DRAWINGML_TEXTCHARACTERPROPERTIES_HXX
#define OOX_DRAWINGML_TEXTCHARACTERPROPERTIES_HXX

#include "oox/helper/propertymap.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/color.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <boost/shared_ptr.hpp>

namespace oox { namespace drawingml {

class TextCharacterProperties;

typedef boost::shared_ptr< TextCharacterProperties > TextCharacterPropertiesPtr;

class TextCharacterProperties
{
public:

    TextCharacterProperties();
    ~TextCharacterProperties();

    PropertyMap&                    getTextCharacterPropertyMap()   { return maTextCharacterPropertyMap; }
    PropertyMap&                    getHyperlinkPropertyMap()        { return maHyperlinkPropertyMap; }
    ::oox::drawingml::ColorPtr      getCharColor()                  { return maCharColorPtr; }
    ::oox::drawingml::ColorPtr      getUnderlineColor()             { return maUnderlineColorPtr; }
    ::oox::drawingml::ColorPtr      getHighlightColor()             { return maHighlightColorPtr; }
    ::com::sun::star::uno::Any&     getHasUnderline()               { return maHasUnderline; }
    ::com::sun::star::uno::Any&     getUnderlineLineFollowText()    { return maUnderlineLineFollowText; }
    ::com::sun::star::uno::Any&     getUnderlineFillFollowText()    { return maUnderlineFillFollowText; }


    void apply( const TextCharacterPropertiesPtr& rSource );
    void pushToPropSet( const ::oox::core::XmlFilterBase& rFilterBase, const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > & xPropSet ) const;
    void pushToUrlFieldPropSet( const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > & xPropSet ) const;
    float getCharacterSize( float fDefault ) const;

protected:
    PropertyMap                     maTextCharacterPropertyMap;
    PropertyMap                     maHyperlinkPropertyMap;
    ::oox::drawingml::ColorPtr      maCharColorPtr;
    ::oox::drawingml::ColorPtr      maUnderlineColorPtr;
    ::oox::drawingml::ColorPtr      maHighlightColorPtr;
    ::com::sun::star::uno::Any      maHasUnderline;
    ::com::sun::star::uno::Any      maUnderlineLineFollowText;
    ::com::sun::star::uno::Any      maUnderlineFillFollowText;
};

} }

#endif  //  OOX_DRAWINGML_TEXTCHARACTERPROPERTIES_HXX
