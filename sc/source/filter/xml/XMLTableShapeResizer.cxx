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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
#include "XMLTableShapeResizer.hxx"
#include "unonames.hxx"
#include "document.hxx"
#include "xmlimprt.hxx"
#include "chartlis.hxx"
#include "XMLConverter.hxx"
#include "rangeutl.hxx"
#include "reftokenhelper.hxx"
#include <tools/debug.hxx>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <memory>
#include <vector>

using namespace ::com::sun::star;
using ::std::auto_ptr;
using ::std::vector;
using ::rtl::OUString;

ScMyShapeResizer::ScMyShapeResizer(ScXMLImport& rTempImport)
    : rImport(rTempImport),
    aShapes(),
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
    const rtl::OUString* pRangeList)
{
    if (!pDoc || !pRangeList)
        // These are minimum required.
        return;

    if (!pRangeList->getLength())
    {
        pDoc->AddOLEObjectToCollection(rName);
        return;
    }

    OUString aRangeStr;
    ScRangeStringConverter::GetStringFromXMLRangeString(aRangeStr, *pRangeList, pDoc);
    if (!aRangeStr.getLength())
    {
        pDoc->AddOLEObjectToCollection(rName);
        return;
    }

    if (!pCollection)
        pCollection = pDoc->GetChartListenerCollection();

    if (!pCollection)
        return;

    auto_ptr< vector<ScSharedTokenRef> > pRefTokens(new vector<ScSharedTokenRef>);
    ScRefTokenHelper::compileRangeRepresentation(*pRefTokens, aRangeStr, pDoc);
    if (!pRefTokens->empty())
    {
        ScChartListener* pCL(new ScChartListener(rName, pDoc, pRefTokens.release()));

        //for loading binary files e.g.
        //if we have the flat filter we need to set the dirty flag thus the visible charts get repainted
        //otherwise the charts keep their first visual representation which was created at a moment where the calc itself was not loaded completly and is incorect therefor
        if( (rImport.getImportFlags() & IMPORT_ALL) == IMPORT_ALL )
            pCL->SetDirty( sal_True );
        else
        {
            // #i104899# If a formula cell is already dirty, further changes aren't propagated.
            // This can happen easily now that row heights aren't updated for all sheets.
            pDoc->InterpretDirtyCells( *pCL->GetRangeList() );
        }

        pCollection->Insert( pCL );
        pCL->StartListeningTo();
    }
}

void ScMyShapeResizer::AddShape(uno::Reference <drawing::XShape>& rShape,
    rtl::OUString* pRangeList,
    table::CellAddress& rStartAddress, table::CellAddress& rEndAddress,
    sal_Int32 nEndX, sal_Int32 nEndY)
{
    ScMyToResizeShape aShape;
    aShape.xShape.set(rShape);
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
    sal_Bool bNegativePage(pDoc->IsNegativePage(rEndCell.Sheet));
    if (bNegativePage)
        aRefPoint.X = rStartRect.Right();
    else
        aRefPoint.X = rStartRect.Left();
    aRefPoint.Y = rStartRect.Top();
    Rectangle aRect(pDoc->GetMMRect(
        static_cast<SCCOL>(rEndCell.Column), static_cast<SCROW>(rEndCell.Row),
        static_cast<SCCOL>(rEndCell.Column), static_cast<SCROW>(rEndCell.Row), rEndCell.Sheet ));
    if (bNegativePage)
        rEndX = -rEndX + aRect.Right();
    else
        rEndX += aRect.Left();
    rEndY += aRect.Top();
    rPoint.X += aRefPoint.X;
    if (bNegativePage)
    {
        if (rPoint.X < rStartRect.Left())
            rPoint.X = rStartRect.Left() + 2; // increment by 2 100th_mm because the cellwidth is internal in twips
    }
    else
    {
        if (rPoint.X > rStartRect.Right())
            rPoint.X = rStartRect.Right() - 2; // decrement by 2 100th_mm because the cellwidth is internal in twips
    }
    rPoint.Y += aRefPoint.Y;
    if (rPoint.Y > rStartRect.Bottom())
        rPoint.Y = rStartRect.Bottom() - 2; // decrement by 2 100th_mm because the cellheight is internal in twips
    if (bNegativePage)
    {
        rSize.Width = -(rEndX - rPoint.X);
    }
    else
        rSize.Width = rEndX - rPoint.X;
    rSize.Height = rEndY - rPoint.Y;
}

void ScMyShapeResizer::ResizeShapes()
{
    if (!aShapes.empty() && rImport.GetModel().is())
    {
        rtl::OUString sRowHeight(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_CELLHGT));
        rtl::OUString sPersistName (RTL_CONSTASCII_USTRINGPARAM("PersistName"));
        rtl::OUString sCaptionPoint( RTL_CONSTASCII_USTRINGPARAM( "CaptionPoint" ));
        rtl::OUString sConnectorShape( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ConnectorShape") );
        rtl::OUString sCaptionShape( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.CaptionShape") );
        rtl::OUString sStartShape(RTL_CONSTASCII_USTRINGPARAM("StartShape"));
        rtl::OUString sEndShape(RTL_CONSTASCII_USTRINGPARAM("EndShape"));
        rtl::OUString sStartPosition(RTL_CONSTASCII_USTRINGPARAM("StartPosition"));
        rtl::OUString sEndPosition(RTL_CONSTASCII_USTRINGPARAM("EndPosition"));
        uno::Reference<table::XCellRange> xTableRow;
        uno::Reference<sheet::XSpreadsheet> xSheet;
        uno::Reference<table::XTableRows> xTableRows;
        sal_Int32 nOldRow(-1);
        sal_Int32 nOldSheet(-1);
        ScMyToResizeShapes::iterator aItr(aShapes.begin());
        ScMyToResizeShapes::iterator aEndItr(aShapes.end());
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( rImport.GetModel(), uno::UNO_QUERY );
        if ( xSpreadDoc.is() )
        {
            uno::Reference<container::XIndexAccess> xIndex( xSpreadDoc->getSheets(), uno::UNO_QUERY );
            ScDocument* pDoc(rImport.GetDocument());
            if ( pDoc && xIndex.is() )
            {
                rImport.LockSolarMutex();
                while (aItr != aEndItr)
                {
                    // #i78086# invalid cell position is used to call CreateChartListener only
                    if ( aItr->aEndCell.Sheet >= 0 )
                    {
                        if ((nOldSheet != aItr->aEndCell.Sheet) || !xSheet.is())
                        {
                            nOldSheet = aItr->aEndCell.Sheet;
                            xSheet.set(xIndex->getByIndex(nOldSheet), uno::UNO_QUERY);
                            if (xSheet.is())
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
                                xTableRows->getByIndex(nOldRow) >>= xTableRow;
                            }
                            if (xTableRow.is())
                            {
                                uno::Reference <beans::XPropertySet> xRowProperties(xTableRow, uno::UNO_QUERY);
                                if (xRowProperties.is())
                                {
                                    sal_Int32 nHeight;
                                    if (xRowProperties->getPropertyValue(sRowHeight) >>= nHeight)
                                    {
                                        Rectangle aRec = pDoc->GetMMRect(static_cast<SCCOL>(aItr->aStartCell.Column), static_cast<SCROW>(aItr->aStartCell.Row),
                                            static_cast<SCCOL>(aItr->aStartCell.Column), static_cast<SCROW>(aItr->aStartCell.Row), aItr->aStartCell.Sheet);
                                        awt::Point aPoint(aItr->xShape->getPosition());
                                        awt::Size aSize(aItr->xShape->getSize());
                                        if (pDoc->IsNegativePage(static_cast<SCTAB>(nOldSheet)))
                                            aPoint.X += aSize.Width;
                                        if (aItr->nEndY >= 0 && aItr->nEndX >= 0)
                                        {
                                            if (aItr->xShape->getShapeType().equals(sConnectorShape))
                                            {
                                                //#103122#; handle connected Connectorshapes
                                                uno::Reference<beans::XPropertySet> xShapeProps (aItr->xShape, uno::UNO_QUERY);
                                                if(xShapeProps.is())
                                                {
                                                    uno::Reference<drawing::XShape> xStartShape(xShapeProps->getPropertyValue(  sStartShape ), uno::UNO_QUERY);
                                                    uno::Reference<drawing::XShape> xEndShape(xShapeProps->getPropertyValue( sEndShape ), uno::UNO_QUERY);
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

                                                        rtl::OUString sProperty;
                                                        if (xStartShape.is())
                                                        {
                                                            awt::Point aEndPoint;
                                                            xShapeProps->getPropertyValue(sEndPosition) >>= aEndPoint;
                                                            aPoint.X = aRec.Left() + aEndPoint.X;
                                                            aPoint.Y = aRec.Top() + aEndPoint.Y;
                                                            sProperty = sEndPosition;
                                                        }
                                                        else
                                                        {
                                                            awt::Point aStartPoint;
                                                            xShapeProps->getPropertyValue(sStartPosition) >>= aStartPoint;
                                                            aPoint.X = aRec.Left() + aStartPoint.X;
                                                            aPoint.Y = aRec.Top() + aStartPoint.Y;
                                                            sProperty = sStartPosition;
                                                        }
                                                        xShapeProps->setPropertyValue(sProperty, uno::makeAny(aPoint));
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                awt::Size aOldSize(aSize);
                                                GetNewShapeSizePos(pDoc, aRec, aItr->aEndCell, aPoint, aSize, aItr->nEndX, aItr->nEndY);
                                                if (pDoc->IsNegativePage(static_cast<SCTAB>(nOldSheet)))
                                                    aPoint.X -= aSize.Width;
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
                    {
                            DBG_ERROR("something wents wrong");
                    }
                    }
                    // #i78086# call CreateChartListener also for invalid position (anchored to sheet)
                    if (IsOLE(aItr->xShape))
                    {
                        uno::Reference < beans::XPropertySet > xShapeProps ( aItr->xShape, uno::UNO_QUERY );
                        uno::Reference < beans::XPropertySetInfo > xShapeInfo(xShapeProps->getPropertySetInfo());
                        rtl::OUString sName;
                        if (xShapeProps.is() && xShapeInfo.is() && xShapeInfo->hasPropertyByName(sPersistName) &&
                            (xShapeProps->getPropertyValue(sPersistName) >>= sName))
                            CreateChartListener(pDoc, sName, aItr->pRangeList);
                    }
                    if (aItr->pRangeList)
                        delete aItr->pRangeList;
                    aItr = aShapes.erase(aItr);
                }
                rImport.UnlockSolarMutex();
//              if (pCollection)
//                  pDoc->SetChartListenerCollection(pCollection);
            }
        }
    }
}
