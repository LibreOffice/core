/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: customshapeproperties.hxx,v $
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

#ifndef OOX_DRAWINGML_CUSTOMSHAPEPROPERTIES_HXX
#define OOX_DRAWINGML_CUSTOMSHAPEPROPERTIES_HXX

#include "oox/helper/propertymap.hxx"
#include "oox/drawingml/color.hxx"
#include <com/sun/star/graphic/XGraphic.hpp>
#include <boost/shared_ptr.hpp>
#include "tokens.hxx"
#include <vector>
#include <map>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeGluePointType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextPathMode.hpp>
#ifndef __com_sun_star_beans_PropertyValues_hpp__
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#include <com/sun/star/drawing/ProjectionMode.hpp>
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
    void pushToPropSet( const ::oox::core::FilterBase& rFilterBase,
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
