/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_XMLTABLESHAPERESIZER_HXX
#define SC_XMLTABLESHAPERESIZER_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
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
