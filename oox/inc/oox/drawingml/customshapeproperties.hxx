/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: customshapeproperties.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:39:22 $
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

#ifndef OOX_DRAWINGML_CUSTOMSHAPEPROPERTIES_HXX
#define OOX_DRAWINGML_CUSTOMSHAPEPROPERTIES_HXX

#include "oox/helper/propertymap.hxx"
#include "oox/drawingml/color.hxx"
#include <com/sun/star/graphic/XGraphic.hpp>
#include <boost/shared_ptr.hpp>
#include "tokens.hxx"
#include <vector>
#include <map>

#ifndef _COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEPARAMETERPAIR_HPP_
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEPARAMETERTYPE_HPP_
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPESEGMENT_HPP_
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEGLUEPOINTTYPE_HPP_
#include <com/sun/star/drawing/EnhancedCustomShapeGluePointType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPESEGMENTCOMMAND_HPP_
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPETEXTFRAME_HPP_
#include <com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEADJUSTMENTVALUE_HPP_
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPETEXTPATHMODE_HPP_
#include <com/sun/star/drawing/EnhancedCustomShapeTextPathMode.hpp>
#endif
#ifndef __com_sun_star_beans_PropertyValues_hpp__
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_PROJECTIONMODE_HPP_
#include <com/sun/star/drawing/ProjectionMode.hpp>
#endif
#include <com/sun/star/drawing/XShape.hpp>

namespace oox { namespace drawingml {

class CustomShapeProperties;

typedef boost::shared_ptr< CustomShapeProperties > CustomShapePropertiesPtr;

struct CustomShapeGuide
{
    rtl::OUString   maName;
    rtl::OUString   maFormula;
};

class CustomShapeProperties
{
public:

    CustomShapeProperties();
    virtual ~CustomShapeProperties();

    void apply( const CustomShapePropertiesPtr& );
    void pushToPropSet( const ::oox::core::XmlFilterBase& rFilterBase,
            const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > & xPropSet,
                        const ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShape > & xShape) const;

    void setShapePresetType( const rtl::OUString& rShapePresetType ){ maShapePresetType = rShapePresetType; };
    std::vector< CustomShapeGuide >& getAdjustmentValues(){ return maAdjustmentValues; };

    double getValue( const std::vector< CustomShapeGuide >&, sal_uInt32 nIndex ) const;

private:

    rtl::OUString                   maShapePresetType;
    std::vector< CustomShapeGuide > maAdjustmentValues;
};

} }

#endif  //  OOX_DRAWINGML_CUSTOMSHAPEPROPERTIES_HXX
