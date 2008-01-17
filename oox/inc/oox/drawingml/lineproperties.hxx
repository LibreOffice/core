/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lineproperties.hxx,v $
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

    void apply( const LinePropertiesPtr& );
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
