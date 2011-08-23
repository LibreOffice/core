/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "unonames.hxx"
#include "document.hxx"
#include "xmlimprt.hxx"
#include "chartlis.hxx"
#include "XMLConverter.hxx"

#include <tools/debug.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
namespace binfilter {

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
    return rShape->getShapeType().equals(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.OLE2Shape")));
}

void ScMyShapeResizer::CreateChartListener(ScDocument* pDoc,
    const ::rtl::OUString& rName,
    const ::rtl::OUString* pRangeList)
{
    if(pDoc)
    {
        if (pRangeList)
        {
            if (pRangeList->getLength())
            {
                if (!pCollection)
                    pCollection = pDoc->GetChartListenerCollection();//new ScChartListenerCollection(pDoc);
                if (pCollection)
                {
                    ScRangeListRef aRangeListRef = new ScRangeList();
                    ScXMLConverter::GetRangeListFromString(*aRangeListRef, *pRangeList, pDoc);
                    if (aRangeListRef->Count())
                    {
                        ScChartListener* pCL = new ScChartListener(
                                            rName, pDoc, aRangeListRef );
                        pCollection->Insert( pCL );
                        pCL->StartListeningTo();
                    }
                }
            }
            else
            {
                pDoc->AddOLEObjectToCollection(rName);
            }
        }
    }
}

void ScMyShapeResizer::AddShape(uno::Reference <drawing::XShape>& rShape,
    ::rtl::OUString* pRangeList,
    table::CellAddress& rStartAddress, table::CellAddress& rEndAddress,
    sal_Int32 nEndX, sal_Int32 nEndY)
{
    ScMyToResizeShape aShape;
    aShape.xShape = rShape;
    aShape.pRangeList = pRangeList;
    aShape.aEndCell = rEndAddress;
    aShape.aStartCell = rStartAddress;
    aShape.nEndY = nEndY;
    aShape.nEndX = nEndX;
    aShapes.push_back(aShape);
}

void ScMyShapeResizer::GetNewShapeSizePos(ScDocument* pDoc, const Rectangle& rStartRect, 
                                          const table::CellAddress& rEndCell, 
                                          awt::Point& rPoint, awt::Size& rSize,
                                          sal_Int32& rEndX, sal_Int32& rEndY) const
{
    awt::Point aRefPoint;
    aRefPoint.X = rStartRect.Left();
    aRefPoint.Y = rStartRect.Top();
    Rectangle* pRect = new Rectangle(pDoc->GetMMRect(
        static_cast<USHORT>(rEndCell.Column), static_cast<USHORT>(rEndCell.Row),
        static_cast<USHORT>(rEndCell.Column), static_cast<USHORT>(rEndCell.Row), rEndCell.Sheet ));
    rEndX += pRect->Left();
    rEndY += pRect->Top();
    rPoint.X += aRefPoint.X;
    if (rPoint.X > rStartRect.Right())
        rPoint.X = rStartRect.Right() - 2; // decrement by 2 100th_mm because the cellheight is internal in twips
    rPoint.Y += aRefPoint.Y;
    if (rPoint.Y > rStartRect.Bottom())
        rPoint.Y = rStartRect.Bottom() - 2; // decrement by 2 100th_mm because the cellheight is internal in twips
    rSize.Width = rEndX - rPoint.X;
    rSize.Height = rEndY - rPoint.Y;
    delete pRect;
}

void ScMyShapeResizer::ResizeShapes()
{
    if (!aShapes.empty() && rImport.GetModel().is())
    {
        ::rtl::OUString sRowHeight(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_CELLHGT));
        ::rtl::OUString sPersistName (RTL_CONSTASCII_USTRINGPARAM("PersistName"));
        ::rtl::OUString sCaptionPoint( RTL_CONSTASCII_USTRINGPARAM( "CaptionPoint" ));
        ::rtl::OUString sConnectorShape( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ConnectorShape") );
        ::rtl::OUString sCaptionShape( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.CaptionShape") );
        ::rtl::OUString sStartShape(RTL_CONSTASCII_USTRINGPARAM("StartShape"));
        ::rtl::OUString sEndShape(RTL_CONSTASCII_USTRINGPARAM("EndShape"));
        ::rtl::OUString sStartPosition(RTL_CONSTASCII_USTRINGPARAM("StartPosition"));
        ::rtl::OUString sEndPosition(RTL_CONSTASCII_USTRINGPARAM("EndPosition"));
        uno::Reference<table::XCellRange> xTableRow;
        uno::Reference<sheet::XSpreadsheet> xSheet;
        uno::Reference<table::XTableRows> xTableRows;
        sal_Int32 nOldRow(-1);
        sal_Int32 nOldSheet(-1);
        ScMyToResizeShapes::iterator aItr = aShapes.begin();
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( rImport.GetModel(), uno::UNO_QUERY );
        if ( xSpreadDoc.is() )
        {
            uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
            uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
            ScDocument* pDoc = rImport.GetDocument();
            if ( pDoc && xIndex.is() )
            {
                rImport.LockSolarMutex();
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
                                    Rectangle aRec = pDoc->GetMMRect(static_cast<USHORT>(aItr->aStartCell.Column), static_cast<USHORT>(aItr->aStartCell.Row),
                                        static_cast<USHORT>(aItr->aStartCell.Column), static_cast<USHORT>(aItr->aStartCell.Row), aItr->aStartCell.Sheet);
                                    awt::Point aPoint(aItr->xShape->getPosition());
                                    awt::Size aSize(aItr->xShape->getSize());
                                    if (aItr->nEndY >= 0 && aItr->nEndX >= 0)
                                    {
                                        if (aItr->xShape->getShapeType().equals(sConnectorShape))
                                        {
                                            //#103122#; handle connected Connectorshapes
                                            uno::Reference<beans::XPropertySet> xShapeProps (aItr->xShape, uno::UNO_QUERY);
                                            if(xShapeProps.is())
                                            {
                                                uno::Any aAny = xShapeProps->getPropertyValue(	sStartShape );
                                                uno::Reference<drawing::XShape> xStartShape;
                                                aAny >>= xStartShape;
                                                aAny = xShapeProps->getPropertyValue( sEndShape );
                                                uno::Reference<drawing::XShape> xEndShape;
                                                aAny >>= xEndShape;
                                                if (!xStartShape.is() && !xEndShape.is())
                                                {
                                                    awt::Size aOldSize(aSize);
                                                    GetNewShapeSizePos(pDoc, aRec, aItr->aEndCell, aPoint, aSize, aItr->nEndX, aItr->nEndY);
                                                    aItr->xShape->setPosition(aPoint);
                                                    if( (aSize.Width != aOldSize.Width) ||
                                                        (aSize.Height != aOldSize.Height) )
                                                        aItr->xShape->setSize(aSize);
                                                }
                                                else if (xStartShape.is() && xEndShape.is())
                                                {
                                                    // do nothing, because they are connected
                                                }
                                                else
                                                {
                                                    // only one point is connected, the other should be moved

                                                    ::rtl::OUString sProperty;
                                                    if (xStartShape.is())
                                                    {
                                                        awt::Point aEndPoint;
                                                        uno::Any aAny = xShapeProps->getPropertyValue(sEndPosition);
                                                        aAny >>= aEndPoint;
                                                        aPoint.X = aRec.Left() + aEndPoint.X;
                                                        aPoint.Y = aRec.Top() + aEndPoint.Y;
                                                        sProperty = sEndPosition;
                                                    }
                                                    else
                                                    {
                                                        awt::Point aStartPoint;
                                                        uno::Any aAny = xShapeProps->getPropertyValue(sStartPosition);
                                                        aAny >>= aStartPoint;
                                                        aPoint.X = aRec.Left() + aStartPoint.X;
                                                        aPoint.Y = aRec.Top() + aStartPoint.Y;
                                                        sProperty = sStartPosition;
                                                    }
                                                    uno::Any aAny;
                                                    aAny <<= aPoint;
                                                    xShapeProps->setPropertyValue(sProperty, aAny);
                                                }
                                            }
                                        }
                                        else
                                        {
                                            awt::Size aOldSize(aSize);
                                            GetNewShapeSizePos(pDoc, aRec, aItr->aEndCell, aPoint, aSize, aItr->nEndX, aItr->nEndY);
                                            aItr->xShape->setPosition(aPoint);
                                            if( (aSize.Width != aOldSize.Width) ||
                                                (aSize.Height != aOldSize.Height) )
                                                aItr->xShape->setSize(aSize);
                                        }
                                    }
                                    else
                                    {
                                        if (aItr->xShape->getShapeType().equals(sCaptionShape))
                                        {
                                            Rectangle aRectangle(aPoint.X, aPoint.Y, aPoint.X + aSize.Width, aPoint.Y + aSize.Height);

                                            awt::Point aCaptionPoint;
                                            uno::Reference< beans::XPropertySet > xShapeProps(aItr->xShape, uno::UNO_QUERY);
                                            if (xShapeProps.is())
                                            {
                                                try
                                                {
                                                    xShapeProps->getPropertyValue( sCaptionPoint ) >>= aCaptionPoint;
                                                }
                                                catch ( uno::Exception& )
                                                {
                                                    DBG_ERROR("This Captionshape has no CaptionPoint property.");
                                                }
                                            }
                                            Point aCorePoint(aPoint.X, aPoint.Y);
                                            Point aCoreCaptionPoint(aCaptionPoint.X, aCaptionPoint.Y);
                                            aCoreCaptionPoint += aCorePoint;
                                            aRectangle.Union(Rectangle(aCoreCaptionPoint, aCoreCaptionPoint));

                                            Point aBeforeRightBottomPoint(aRectangle.BottomRight());

                                            aRectangle += aRec.TopLeft();
                                            if (aRectangle.Left() > aRec.Right())
                                                aRectangle -= (Point(aRectangle.Left() - aRec.Right() + 2, 0));
                                            if (aRectangle.Top() > aRec.Bottom())
                                                aRectangle -= (Point(0, aRectangle.Top() - aRec.Bottom() + 2));

                                            Point aDifferencePoint(aRectangle.BottomRight() - aBeforeRightBottomPoint);
                                            aPoint.X += aDifferencePoint.X();
                                            aPoint.Y += aDifferencePoint.Y();

                                            aItr->xShape->setPosition(aPoint);
                                        }
                                        else
                                        {
                                            // #96159# it is possible, that shapes have a negative position
                                            // this is now handled here
                                            DBG_ERROR("no or negative end address of this shape");
                                            awt::Point aRefPoint;
                                            aRefPoint.X = aRec.Left();
                                            aRefPoint.Y = aRec.Top();
                                            aPoint.X += aRefPoint.X;
                                            if (aPoint.X > aRec.Right())
                                                aPoint.X = aRec.Right() - 2; // decrement by 2 100th_mm because the cellheight is internal in twips
                                            aPoint.Y += aRefPoint.Y;
                                            if (aPoint.Y > aRec.Bottom())
                                                aPoint.Y = aRec.Bottom() - 2; // decrement by 2 100th_mm because the cellheight is internal in twips
                                            aItr->xShape->setPosition(aPoint);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                        DBG_ERROR("something wents wrong");
                    if (IsOLE(aItr->xShape))
                    {
                        uno::Reference < beans::XPropertySet > xShapeProps ( aItr->xShape, uno::UNO_QUERY );
                        uno::Reference < beans::XPropertySetInfo > xShapeInfo = xShapeProps->getPropertySetInfo();
                        if (xShapeProps.is() && xShapeInfo.is())
                        {
                            if (xShapeInfo->hasPropertyByName(sPersistName))
                            {
                                uno::Any aAny = xShapeProps->getPropertyValue(sPersistName);
                                ::rtl::OUString sName;
                                if (aAny >>= sName)
                                    CreateChartListener(pDoc, sName, aItr->pRangeList);
                            }
                        }
                    }
                    if (aItr->pRangeList)
                        delete aItr->pRangeList;
                    aItr = aShapes.erase(aItr);
                }
                rImport.UnlockSolarMutex();
//				if (pCollection)
//					pDoc->SetChartListenerCollection(pCollection);
            }
        }
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
