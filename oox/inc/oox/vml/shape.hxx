/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: shape.hxx,v $
 * $Revision: 1.5 $
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

#ifndef OOX_VML_SHAPE_HXX
#define OOX_VML_SHAPE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include "com/sun/star/drawing/XShape.hpp"
#include "com/sun/star/drawing/XShapes.hpp"
#include "com/sun/star/beans/NamedValue.hpp"
#include "com/sun/star/awt/Rectangle.hpp"
#include "com/sun/star/awt/Point.hpp"
#include "com/sun/star/awt/Size.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <vector>
#include <map>
#include <rtl/ustring.hxx>

namespace oox { namespace core {
    class XmlFilterBase;
} }

namespace oox { namespace vml {

class Shape;
class Drawing;
typedef boost::shared_ptr< Shape > ShapePtr;

class Shape
{
    public :

    Shape( const rtl::OUString& rServiceName );
    ~Shape();

    void applyAttributes( const Shape& rSource );

    rtl::OUString   msServiceName;
    rtl::OUString   msId;
    rtl::OUString   msType;
    rtl::OUString   msShapeType;

    sal_Int32       mnCoordWidth;
    sal_Int32       mnCoordHeight;
    sal_Int32       mnStroked;

    ::boost::optional< sal_Int32 >      moFilled;
    ::boost::optional< rtl::OUString >  moFillColor;

    rtl::OUString   msGraphicURL;
    rtl::OUString   msImageTitle;

    com::sun::star::beans::NamedValue   maPath;

    rtl::OUString                       msPosition;             // absolute/relativ
    com::sun::star::awt::Point          maPosition;
    com::sun::star::awt::Size           maSize;

    std::vector< ShapePtr >& getChilds() { return maChilds; };

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > mxShape;

    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& getXShape() const { return mxShape; };

    // addShape is creating and inserting the corresponding XShape.
    void addShape( const ::oox::core::XmlFilterBase& rFilterBase, const ::oox::vml::Drawing& rDrawing,
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
            const ::com::sun::star::awt::Rectangle* pShapeRect );

    private :

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createAndInsert( const ::oox::core::XmlFilterBase& rFilterBase, const ::oox::vml::Shape& rShape,
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                const ::com::sun::star::awt::Rectangle* pShapeRect );

    void addChilds( const ::oox::core::XmlFilterBase& rFilterBase, const ::oox::vml::Drawing& rDrawing,
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
            const ::com::sun::star::awt::Rectangle& rClientRect );

    std::vector< ShapePtr > maChilds;

};

typedef boost::shared_ptr< Shape > ShapePtr;

} }

#endif  //  OOX_VML_SHAPE_HXX
