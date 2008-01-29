/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLTableShapeResizer.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:36:14 $
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

#ifndef _SC_XMLTABLESHAPERESIZER_HXX
#define _SC_XMLTABLESHAPERESIZER_HXX

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEET_HPP_
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#endif
#include <list>

class ScXMLImport;
class ScChartListenerCollection;
class ScDocument;
class Rectangle;

struct ScMyToResizeShape
{
    com::sun::star::uno::Reference <com::sun::star::drawing::XShape> xShape;
    rtl::OUString* pRangeList;
    com::sun::star::table::CellAddress  aEndCell;
    com::sun::star::table::CellAddress  aStartCell;
    sal_Int32 nEndX;
    sal_Int32 nEndY;

    ScMyToResizeShape() : pRangeList(NULL) {}
};

typedef std::list<ScMyToResizeShape> ScMyToResizeShapes;

class ScMyShapeResizer
{
    ScXMLImport&                rImport;
    ScMyToResizeShapes          aShapes;
    ScChartListenerCollection*  pCollection;

    sal_Bool IsOLE(com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& rShape) const;
    void CreateChartListener(ScDocument* pDoc,
        const rtl::OUString& rName,
        const rtl::OUString* pRangeList);
    void GetNewShapeSizePos(ScDocument* pDoc, const Rectangle& rStartRect,
                            const com::sun::star::table::CellAddress& rEndCell,
                            com::sun::star::awt::Point& rPoint, com::sun::star::awt::Size& rSize,
                            sal_Int32& rEndX, sal_Int32& rEndY) const;
public:
    ScMyShapeResizer(ScXMLImport& rImport);
    ~ScMyShapeResizer();

    void    AddShape(com::sun::star::uno::Reference <com::sun::star::drawing::XShape>& rShape,
                    rtl::OUString* pRangeList,
                    com::sun::star::table::CellAddress& rStartAddress,
                    com::sun::star::table::CellAddress& rEndAddress,
                    sal_Int32 nEndX, sal_Int32 nEndY);
    void    ResizeShapes();
};

#endif
