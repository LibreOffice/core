/*************************************************************************
 *
 *  $RCSfile: XMLTableShapeResizer.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: sab $ $Date: 2001-07-23 15:24:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SC_XMLTABLESHAPERESIZER_HXX
#include "XMLTableShapeResizer.hxx"
#endif
#ifndef SC_UNONAMES_HXX
#include "unonames.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_XMLIMPRT_HXX
#include "xmlimprt.hxx"
#endif
#ifndef SC_CHARTLIS_HXX
#include "chartlis.hxx"
#endif
#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETDOCUMENT_HPP_
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_XCOLUMNROWRANGE_HPP_
#include <com/sun/star/table/XColumnRowRange.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

using namespace ::com::sun::star;

ScMyShapeResizer::ScMyShapeResizer(ScXMLImport& rTempImport)
    : aShapes(),
    rImport(rTempImport),
    pCollection(NULL)
{
}

ScMyShapeResizer::~ScMyShapeResizer()
{
}

sal_Bool ScMyShapeResizer::IsOLE(uno::Reference< drawing::XShape >& rShape) const
{
    return rShape->getShapeType().equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.OLE2Shape")));
}

void ScMyShapeResizer::CreateChartListener(ScDocument* pDoc,
    const rtl::OUString& rName,
    const rtl::OUString& rRangeList)
{
    if (pDoc && rRangeList.getLength())
    {
        if (!pCollection)
            pCollection = pDoc->GetChartListenerCollection();//new ScChartListenerCollection(pDoc);
        if (pCollection)
        {
            ScRangeListRef aRangeListRef = new ScRangeList();
            ScXMLConverter::GetRangeListFromString(*aRangeListRef, rRangeList, pDoc);
            if (aRangeListRef->Count())
            {
                ScChartListener* pCL = new ScChartListener(
                                    rName, pDoc, aRangeListRef );
                pCollection->Insert( pCL );
                pCL->StartListeningTo();
            }
        }
    }
}

void ScMyShapeResizer::AddShape(uno::Reference <drawing::XShape>& rShape,
    const rtl::OUString& rName, const rtl::OUString& rRangeList,
    table::CellAddress& rStartAddress, table::CellAddress& rEndAddress,
    sal_Int32 nEndX, sal_Int32 nEndY)
{
    ScMyToResizeShape aShape;
    aShape.xShape = rShape;
    aShape.sName = rName;
    aShape.sRangeList = rRangeList;
    aShape.aEndCell = rEndAddress;
    aShape.aStartCell = rStartAddress;
    aShape.nEndY = nEndY;
    aShape.nEndX = nEndX;
    aShapes.push_back(aShape);
}

void ScMyShapeResizer::ResizeShapes()
{
    if (aShapes.size())
    {
        rtl::OUString sRowHeight(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_CELLHGT));
        uno::Reference<table::XCellRange> xTableRow;
        uno::Reference<sheet::XSpreadsheet> xSheet;
        uno::Reference<table::XTableRows> xTableRows;
        sal_Int32 nOldRow(-1);
        sal_Int32 nOldSheet(-1);
        Rectangle* pRect = NULL;
        ScMyToResizeShapes::iterator aItr = aShapes.begin();
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( rImport.GetModel(), uno::UNO_QUERY );
        if ( xSpreadDoc.is() )
        {
            uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
            uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
            if ( xIndex.is() )
            {
                ScDocument* pDoc = rImport.GetDocument();
                while (aItr != aShapes.end())
                {
                    if ((nOldSheet != aItr->aEndCell.Sheet) || !xSheet.is())
                    {
                        nOldSheet = aItr->aEndCell.Sheet;
                        uno::Any aTable = xIndex->getByIndex(nOldSheet);
                        if (aTable>>=xSheet)
                        {
                            uno::Reference<table::XColumnRowRange> xColumnRowRange (xSheet, uno::UNO_QUERY);
                            if (xColumnRowRange.is())
                                xTableRows = xColumnRowRange->getRows();
                        }
                    }
                    if (xTableRows.is())
                    {
                        if (nOldRow != aItr->aEndCell.Row || !xTableRow.is())
                        {
                            nOldRow = aItr->aEndCell.Row;
                            uno::Any aRow = xTableRows->getByIndex(nOldRow);
                            aRow >>= xTableRow;
                        }
                        if (xTableRow.is())
                        {
                            uno::Reference <beans::XPropertySet> xRowProperties(xTableRow, uno::UNO_QUERY);
                            if (xRowProperties.is())
                            {
                                uno::Any aAny = xRowProperties->getPropertyValue(sRowHeight);
                                sal_Int32 nHeight;
                                if (aAny >>= nHeight)
                                {
                                    if (aItr->nEndY >= 0 && aItr->nEndX >= 0)
                                    {
                                        Rectangle aRec = rImport.GetDocument()->GetMMRect(static_cast<USHORT>(aItr->aStartCell.Column), static_cast<USHORT>(aItr->aStartCell.Row),
                                            static_cast<USHORT>(aItr->aStartCell.Column), static_cast<USHORT>(aItr->aStartCell.Row), aItr->aStartCell.Sheet);
                                        awt::Point aRefPoint;
                                        aRefPoint.X = aRec.Left();
                                        aRefPoint.Y = aRec.Top();
                                        pRect = new Rectangle(rImport.GetDocument()->GetMMRect(
                                            static_cast<USHORT>(aItr->aEndCell.Column), static_cast<USHORT>(aItr->aEndCell.Row),
                                            static_cast<USHORT>(aItr->aEndCell.Column), static_cast<USHORT>(aItr->aEndCell.Row), aItr->aEndCell.Sheet ));
                                        aItr->nEndX += pRect->Left();
                                        aItr->nEndY += pRect->Top();
                                        awt::Point aPoint = aItr->xShape->getPosition();
                                        awt::Size aOldSize = aItr->xShape->getSize();
                                        awt::Size aSize(aOldSize);
                                        aPoint.X += aRefPoint.X;
                                        aPoint.Y += aRefPoint.Y;
                                        aSize.Width = aItr->nEndX - aPoint.X;
                                        aSize.Height = aItr->nEndY - aPoint.Y;
                                        aItr->xShape->setPosition(aPoint);
                                        if( (aSize.Width != aOldSize.Width) ||
                                            (aSize.Height != aOldSize.Height) )
                                            aItr->xShape->setSize(aSize);
                                        delete pRect;
                                    }
                                    else
                                    {
                                        DBG_ASSERT(aItr->xShape->getShapeType().equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.CaptionShape"))),
                                            "no end address of this shape");
                                        Rectangle aRec = rImport.GetDocument()->GetMMRect(static_cast<USHORT>(aItr->aStartCell.Column), static_cast<USHORT>(aItr->aStartCell.Row),
                                            static_cast<USHORT>(aItr->aStartCell.Column), static_cast<USHORT>(aItr->aStartCell.Row), aItr->aStartCell.Sheet);
                                        awt::Point aRefPoint;
                                        aRefPoint.X = aRec.Left();
                                        aRefPoint.Y = aRec.Top();
                                        awt::Point aPoint = aItr->xShape->getPosition();
                                        aPoint.X += aRefPoint.X;
                                        aPoint.Y += aRefPoint.Y;
                                        aItr->xShape->setPosition(aPoint);
                                    }
                                }
                            }
                        }
                    }
                    else
                        DBG_ERROR("something wents wrong");
                    if (IsOLE(aItr->xShape))
                        CreateChartListener(pDoc, aItr->sName, aItr->sRangeList);
                    aItr = aShapes.erase(aItr);
                }
//              if (pCollection)
//                  pDoc->SetChartListenerCollection(pCollection);
            }
        }
    }
}
