/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: lineproperties.hxx,v $
 * $Revision: 1.4 $
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

#ifndef OOX_DRAWINGML_LINEPROPERTIES_HXX
#define OOX_DRAWINGML_LINEPROPERTIES_HXX

#include "oox/helper/propertymap.hxx"
#include "oox/drawingml/color.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <vector>
#include <map>

namespace oox { namespace drawingml {

class LineProperties;

typedef boost::shared_ptr< LineProperties > LinePropertiesPtr;

class LineProperties
{
public:

    LineProperties();
    virtual ~LineProperties();

    void apply( const LineProperties& );
    void pushToPropSet( const ::oox::core::XmlFilterBase& rFilterBase,
            const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > & xPropSet ) const;

    PropertyMap&                    getLinePropertyMap(){ return maLineProperties; };
    ::oox::drawingml::ColorPtr&     getLineColor(){ return maLineColor; };
    boost::optional< sal_Int32 >&   getLineWidth(){ return moLineWidth; };
    boost::optional< sal_Int32 >&   getStartArrow(){ return moStartArrow; };
    boost::optional< sal_Int32 >&   getStartArrowWidth(){ return moStartArrowWidth; };
    boost::optional< sal_Int32 >&   getStartArrowLength(){ return moStartArrowLength; };
    boost::optional< sal_Int32 >&   getEndArrow(){ return moEndArrow; };
    boost::optional< sal_Int32 >&   getEndArrowWidth(){ return moEndArrowWidth; };
    boost::optional< sal_Int32 >&   getEndArrowLength(){ return moEndArrowLength; };
    boost::optional< sal_Int32 >&   getPresetDash(){ return moPresetDash; };
    boost::optional< sal_Int32 >&   getLineCap(){ return moLineCap; };

private:

    PropertyMap                     maLineProperties;
    ::oox::drawingml::ColorPtr      maLineColor;

    boost::optional< sal_Int32 >    moLineWidth;
    boost::optional< sal_Int32 >    moStartArrow;
    boost::optional< sal_Int32 >    moStartArrowWidth;
    boost::optional< sal_Int32 >    moStartArrowLength;
    boost::optional< sal_Int32 >    moEndArrow;
    boost::optional< sal_Int32 >    moEndArrowWidth;
    boost::optional< sal_Int32 >    moEndArrowLength;
    boost::optional< sal_Int32 >    moPresetDash;
    boost::optional< sal_Int32 >    moLineCap;
};

} }

#endif  //  OOX_DRAWINGML_LINEPROPERTIES_HXX
