/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: shape.hxx,v $
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

#ifndef OOX_VML_SHAPE_HXX
#define OOX_VML_SHAPE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include "com/sun/star/drawing/XShape.hpp"
#include "com/sun/star/drawing/XShapes.hpp"
#include <boost/shared_ptr.hpp>
#include <map>
#include <rtl/ustring.hxx>

namespace oox { namespace core {
    class XmlFilterBase;
} }

namespace oox { namespace vml {

struct Shape
{
    Shape();
    ~Shape();

    void applyAttributes( const Shape& rSource );

    rtl::OUString   msId;
    rtl::OUString   msType;
    sal_Int32       mnShapeType;

    sal_Int32       mnCoordWidth;
    sal_Int32       mnCoordHeight;
    sal_Int32       mnStroked;
    sal_Int32       mnFilled;

    rtl::OUString   msGraphicURL;
    rtl::OUString   msImageTitle;
    rtl::OUString   msPath;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > mxShape;

    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& getXShape() const { return mxShape; };

    // addShape is creating and inserting the corresponding XShape.
    void addShape( const ::oox::core::XmlFilterBase& rFilterBase,
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes );

};

typedef boost::shared_ptr< Shape > ShapePtr;

} }

#endif  //  OOX_VML_SHAPE_HXX
