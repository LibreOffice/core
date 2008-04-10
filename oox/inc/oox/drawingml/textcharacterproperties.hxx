/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: textcharacterproperties.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
