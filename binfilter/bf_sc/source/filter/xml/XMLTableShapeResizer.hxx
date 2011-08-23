/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _SC_XMLTABLESHAPERESIZER_HXX
#define _SC_XMLTABLESHAPERESIZER_HXX

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
#ifndef __SGI_STL_LIST
#include <list>
#endif
class Rectangle;
namespace binfilter {

class ScXMLImport;
class ScChartListenerCollection;
class ScDocument;

struct ScMyToResizeShape
{
    ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShape> xShape;
    ::rtl::OUString* pRangeList;
    ::com::sun::star::table::CellAddress	aEndCell;
    ::com::sun::star::table::CellAddress	aStartCell;
    sal_Int32 nEndX;
    sal_Int32 nEndY;

    ScMyToResizeShape() : pRangeList(NULL) {}
};

typedef std::list<ScMyToResizeShape> ScMyToResizeShapes;

class ScMyShapeResizer
{
    ScXMLImport&				rImport;
    ScMyToResizeShapes			aShapes;
    ScChartListenerCollection*	pCollection;

    sal_Bool IsOLE(::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rShape) const;
    void CreateChartListener(ScDocument* pDoc,
        const ::rtl::OUString& rName,
        const ::rtl::OUString* pRangeList);
    void GetNewShapeSizePos(ScDocument* pDoc, const Rectangle& rStartRect, 
                            const ::com::sun::star::table::CellAddress& rEndCell, 
                            ::com::sun::star::awt::Point& rPoint, ::com::sun::star::awt::Size& rSize,
                            sal_Int32& rEndX, sal_Int32& rEndY) const;
public:
    ScMyShapeResizer(ScXMLImport& rImport);
    ~ScMyShapeResizer();

    void	AddShape(::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShape>& rShape,
                    ::rtl::OUString* pRangeList,
                    ::com::sun::star::table::CellAddress& rStartAddress,
                    ::com::sun::star::table::CellAddress& rEndAddress,
                    sal_Int32 nEndX, sal_Int32 nEndY);
    void	ResizeShapes();
};

} //namespace binfilter
#endif
