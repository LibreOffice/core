/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <IDocumentDrawModelAccess.hxx>
#include <docsh.hxx>
#include <drawdoc.hxx>
#include <edtwin.hxx>
#include <swmodeltestbase.hxx>
#include <view.hxx>
#include <wrtsh.hxx>

#include <comphelper/configuration.hxx>
#include <officecfg/Office/Writer.hxx>
#include <svx/svdobjkind.hxx>
#include <svx/svdpage.hxx>
#include <vcl/event.hxx>
#include <vcl/scheduler.hxx>

namespace
{
class SwSnapToGridTest : public SwModelTestBase
{
public:
    SwSnapToGridTest()
        : SwModelTestBase(u"/sw/qa/extras/snaptogrid/data/"_ustr)
    {
    }

protected:
    enum class DragPoint
    {
        TopLeft,
        TopCenter,
        TopRight,
        LeftCenter,
        RightCenter,
        BottomLeft,
        BottomCenter,
        BottomRight
    };

    void applyGridOptions(const bool bSnapToGrid, const sal_Int32 nResolutionXMm100,
                          const sal_Int32 nResolutionYMm100, const sal_Int32 nSubdivisionX,
                          const sal_Int32 nSubdivisionY);

    SdrObject* getShapeObject(const size_t nObjectIndex);
    void moveShape(const SdrObject* pObject, const tools::Long nMoveXMm100,
                   const tools::Long nMoveYMm100);
    void resizeShape(const SdrObject* pObject, const DragPoint eDragPoint,
                     const tools::Long nMoveXMm100, const tools::Long nMoveYMm100);

    void checkShapeOpenSize(SdrObject* pObject, const sal_Int32 nWidthMm100,
                            const sal_Int32 nHeightMm100);
    void checkShapePosition(SdrObject* pObject, const sal_Int32 nXMm100, const sal_Int32 nYMm100);
};

void SwSnapToGridTest::applyGridOptions(const bool bSnapToGrid, const sal_Int32 nResolutionXMm100,
                                        const sal_Int32 nResolutionYMm100,
                                        const sal_Int32 nSubdivisionX,
                                        const sal_Int32 nSubdivisionY)
{
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Writer::Grid::Option::SnapToGrid::set(bSnapToGrid, pBatch);
    officecfg::Office::Writer::Grid::Resolution::XAxis::set(nResolutionXMm100, pBatch);
    officecfg::Office::Writer::Grid::Resolution::YAxis::set(nResolutionYMm100, pBatch);
    officecfg::Office::Writer::Grid::Subdivision::XAxis::set(nSubdivisionX, pBatch);
    officecfg::Office::Writer::Grid::Subdivision::YAxis::set(nSubdivisionY, pBatch);
    pBatch->commit();
}

SdrObject* SwSnapToGridTest::getShapeObject(const size_t nObjectIndex)
{
    const SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

    const SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    CPPUNIT_ASSERT(pPage);

    SdrObject* pObject = pPage->GetObj(nObjectIndex);
    CPPUNIT_ASSERT(pObject);

    return pObject;
}

void SwSnapToGridTest::moveShape(const SdrObject* pObject, const tools::Long nMoveXMm100,
                                 const tools::Long nMoveYMm100)
{
    CPPUNIT_ASSERT(pObject);

    const tools::Long nMoveXTwips = o3tl::toTwips(nMoveXMm100, o3tl::Length::mm100);
    const tools::Long nMoveYTwips = o3tl::toTwips(nMoveYMm100, o3tl::Length::mm100);

    const tools::Rectangle& rBoundRect = pObject->GetCurrentBoundRect();
    Point aFromTwips;
    if (pObject->GetObjIdentifier() == SdrObjKind::SwFlyDrawObjIdentifier)
    {
        // Grab at the top for text frames.
        aFromTwips = Point(rBoundRect.TopCenter().X(), rBoundRect.TopCenter().Y() + 50);
    }
    else
    {
        // Use center for other objects.
        aFromTwips = rBoundRect.Center();
    }
    const Point aToTwips(aFromTwips.X() + nMoveXTwips, aFromTwips.Y() + nMoveYTwips);

    vcl::Window& rEditWin = getSwDocShell()->GetView()->GetEditWin();
    const Point aFromPixels = rEditWin.LogicToPixel(aFromTwips);
    const Point aToPixels = rEditWin.LogicToPixel(aToTwips);

    const MouseEvent aClickEvent(aFromPixels, 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT);
    rEditWin.MouseButtonDown(aClickEvent);
    const MouseEvent aMoveEvent(aToPixels, 0, MouseEventModifiers::SIMPLEMOVE, MOUSE_LEFT);
    rEditWin.MouseMove(aMoveEvent);
    rEditWin.MouseMove(aMoveEvent);
    const MouseEvent aReleaseEvent(aToPixels, 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT);
    rEditWin.MouseButtonUp(aReleaseEvent);
    Scheduler::ProcessEventsToIdle();
}

void SwSnapToGridTest::resizeShape(const SdrObject* pObject, const DragPoint eDragPoint,
                                   const tools::Long nMoveXMm100, const tools::Long nMoveYMm100)
{
    CPPUNIT_ASSERT(pObject);

    Point aDragPoint;
    switch (eDragPoint)
    {
        case DragPoint::TopLeft:
            aDragPoint = pObject->GetCurrentBoundRect().TopLeft();
            break;
        case DragPoint::TopCenter:
            aDragPoint = pObject->GetCurrentBoundRect().TopCenter();
            break;
        case DragPoint::TopRight:
            aDragPoint = pObject->GetCurrentBoundRect().TopRight();
            break;
        case DragPoint::LeftCenter:
            aDragPoint = pObject->GetCurrentBoundRect().LeftCenter();
            break;
        case DragPoint::RightCenter:
            aDragPoint = pObject->GetCurrentBoundRect().RightCenter();
            break;
        case DragPoint::BottomLeft:
            aDragPoint = pObject->GetCurrentBoundRect().BottomLeft();
            break;
        case DragPoint::BottomCenter:
            aDragPoint = pObject->GetCurrentBoundRect().BottomCenter();
            break;
        case DragPoint::BottomRight:
            aDragPoint = pObject->GetCurrentBoundRect().BottomRight();
            break;
    }

    const tools::Long nMoveXTwips = o3tl::toTwips(nMoveXMm100, o3tl::Length::mm100);
    const tools::Long nMoveYTwips = o3tl::toTwips(nMoveYMm100, o3tl::Length::mm100);

    const Point aFromTwips(aDragPoint);
    const Point aToTwips(aDragPoint.X() + nMoveXTwips, aDragPoint.Y() + nMoveYTwips);

    vcl::Window& rEditWin = getSwDocShell()->GetView()->GetEditWin();
    const Point aFromPixels = rEditWin.LogicToPixel(aFromTwips);
    const Point aToPixels = rEditWin.LogicToPixel(aToTwips);

    const MouseEvent aClickEvent(aFromPixels, 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT);
    rEditWin.MouseButtonDown(aClickEvent);
    const MouseEvent aMoveEvent(aToPixels, 0, MouseEventModifiers::SIMPLEMOVE, MOUSE_LEFT);
    rEditWin.MouseMove(aMoveEvent);
    rEditWin.MouseMove(aMoveEvent);
    const MouseEvent aReleaseEvent(aToPixels, 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT);
    rEditWin.MouseButtonUp(aReleaseEvent);
    Scheduler::ProcessEventsToIdle();
}

void SwSnapToGridTest::checkShapeOpenSize(SdrObject* pObject, const sal_Int32 nWidthMm100,
                                          const sal_Int32 nHeightMm100)
{
    CPPUNIT_ASSERT(pObject);

    css::uno::Reference<css::drawing::XShape> xShape = pObject->getUnoShape();
    CPPUNIT_ASSERT(xShape.is());

    if (pObject->GetObjIdentifier() == SdrObjKind::SwFlyDrawObjIdentifier)
    {
        // Writer objects return with "closed" size values (i.e.  width = right - left + 1)
        const awt::Size aSize = xShape->getSize();
        const sal_Int32 nSizeDiff = o3tl::convert(1, o3tl::Length::twip, o3tl::Length::mm100);
        CPPUNIT_ASSERT_EQUAL(nWidthMm100, aSize.Width - nSizeDiff);
        CPPUNIT_ASSERT_EQUAL(nHeightMm100, aSize.Height - nSizeDiff);
    }
    else
    {
        // General shapes return with "open" size values (i.e.  width = right - left)
        const awt::Size aSize = xShape->getSize();
        CPPUNIT_ASSERT_EQUAL(nWidthMm100, aSize.Width);
        CPPUNIT_ASSERT_EQUAL(nHeightMm100, aSize.Height);
    }
}

void SwSnapToGridTest::checkShapePosition(SdrObject* pObject, const sal_Int32 nXMm100,
                                          const sal_Int32 nYMm100)
{
    CPPUNIT_ASSERT(pObject);

    css::uno::Reference<css::drawing::XShape> xShape = pObject->getUnoShape();
    CPPUNIT_ASSERT(xShape.is());

    CPPUNIT_ASSERT_EQUAL(nXMm100, getProperty<sal_Int32>(xShape, u"HoriOrientPosition"_ustr));
    CPPUNIT_ASSERT_EQUAL(nYMm100, getProperty<sal_Int32>(xShape, u"VertOrientPosition"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwSnapToGridTest, testMoveShapeOnGrid)
{
    // Apply grid options
    const sal_Int32 nGridSize = 1016;
    applyGridOptions(true, nGridSize, nGridSize, 0, 0);

    // Load a document with a single shape
    createSwDoc("shapeSnappedToGrid.fodt");

    // Check the original position and size of the shape (it is aligned to the grid)
    const awt::Point aStartPosition{ 1016, 1016 };
    const awt::Size aStartSize{ 1016, 1016 };
    SdrObject* pObject = getShapeObject(0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    const tools::Long nMove = 2 * nGridSize - 10;

    // Move the shape two grid positions to the right
    moveShape(pObject, nMove, 0);
    checkShapePosition(pObject, aStartPosition.X + 2 * nGridSize, aStartPosition.Y);

    // Move the shape two grid positions down
    moveShape(pObject, 0, nMove);
    checkShapePosition(pObject, aStartPosition.X + 2 * nGridSize, aStartPosition.Y + 2 * nGridSize);

    // Move the shape two grid positions to the left
    moveShape(pObject, -nMove, 0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y + 2 * nGridSize);

    // Move the shape two grid positions up
    moveShape(pObject, 0, -nMove);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);

    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);
}

CPPUNIT_TEST_FIXTURE(SwSnapToGridTest, testTryToMoveShape)
{
    // Apply grid options
    const sal_Int32 nGridSize = 1016;
    applyGridOptions(true, nGridSize, nGridSize, 0, 0);

    // Load a document with a single shape
    createSwDoc("shapeSnappedToGrid.fodt");

    // Check the original position and size of the shape (it is aligned to the grid)
    const awt::Point aStartPosition{ 1016, 1016 };
    const awt::Size aStartSize{ 1016, 1016 };
    SdrObject* pObject = getShapeObject(0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    const tools::Long nMove = nGridSize / 4; // Move distance is too small.

    // Try to move the shape to the right
    moveShape(pObject, nMove, 0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);

    // Try to move the shape down
    moveShape(pObject, 0, nMove);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);

    // Try to move the shape to the left
    moveShape(pObject, -nMove, 0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);

    // Try to move the shape up
    moveShape(pObject, 0, -nMove);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);

    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);
}

CPPUNIT_TEST_FIXTURE(SwSnapToGridTest, testResizeShapeOnGrid)
{
    // Apply grid options
    const sal_Int32 nGridSize = 1016;
    applyGridOptions(true, nGridSize, nGridSize, 0, 0);

    // Load a document with a single shape
    createSwDoc("shapeSnappedToGrid.fodt");

    // Check the original position and size of the shape (it is aligned to the grid)
    const awt::Point aStartPosition{ 1016, 1016 };
    const awt::Size aStartSize{ 1016, 1016 };
    SdrObject* pObject = getShapeObject(0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    const tools::Long nMove = nGridSize - 20;

    // Drag & resize the shape to the right and then resize it back
    resizeShape(pObject, DragPoint::RightCenter, nMove, 0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width + nGridSize, aStartSize.Height);

    resizeShape(pObject, DragPoint::RightCenter, -nMove, 0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    // Drag & resize the shape to the right and then resize it back
    resizeShape(pObject, DragPoint::LeftCenter, -nMove, 0);
    checkShapePosition(pObject, aStartPosition.X - nGridSize, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width + nGridSize, aStartSize.Height);

    resizeShape(pObject, DragPoint::LeftCenter, nMove, 0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    // Drag & resize the shape downward and then resize it back
    resizeShape(pObject, DragPoint::BottomCenter, 0, nMove);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height + nGridSize);

    resizeShape(pObject, DragPoint::BottomCenter, 0, -nMove);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    // Drag & resize the shape upward and then resize it back
    resizeShape(pObject, DragPoint::TopCenter, 0, -nMove);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y - nGridSize);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height + nGridSize);

    resizeShape(pObject, DragPoint::TopCenter, 0, nMove);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    // Drag & resize the shape to the right and upward and then resize it back
    resizeShape(pObject, DragPoint::TopRight, nMove, -nMove);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y - nGridSize);
    checkShapeOpenSize(pObject, aStartSize.Width + nGridSize, aStartSize.Height + nGridSize);

    resizeShape(pObject, DragPoint::TopRight, -nMove, nMove);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    // Drag & resize the shape to the right and downward and then resize it back
    resizeShape(pObject, DragPoint::BottomRight, nMove, nMove);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width + nGridSize, aStartSize.Height + nGridSize);

    resizeShape(pObject, DragPoint::BottomRight, -nMove, -nMove);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    // Drag & resize the shape to the left and upward and then resize it back
    resizeShape(pObject, DragPoint::TopLeft, -nMove, -nMove);
    checkShapePosition(pObject, aStartPosition.X - nGridSize, aStartPosition.Y - nGridSize);
    checkShapeOpenSize(pObject, aStartSize.Width + nGridSize, aStartSize.Height + nGridSize);

    resizeShape(pObject, DragPoint::TopLeft, nMove, nMove);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    // Drag & resize the shape to the left and downward and then resize it back
    resizeShape(pObject, DragPoint::BottomLeft, -nMove, nMove);
    checkShapePosition(pObject, aStartPosition.X - nGridSize, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width + nGridSize, aStartSize.Height + nGridSize);

    resizeShape(pObject, DragPoint::BottomLeft, nMove, -nMove);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);
}

CPPUNIT_TEST_FIXTURE(SwSnapToGridTest, testFoldShapeWithResize)
{
    // Apply grid options
    const sal_Int32 nGridSize = 1016;
    applyGridOptions(true, nGridSize, nGridSize, 0, 0);

    // Load a document with a single shape
    createSwDoc("shapeSnappedToGrid.fodt");

    // Check the original position and size of the shape (it is aligned to the grid)
    const awt::Point aStartPosition{ 1016, 1016 };
    const awt::Size aStartSize{ 1016, 1016 };
    SdrObject* pObject = getShapeObject(0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    const tools::Long nMove = nGridSize;

    // Fold the shape into a "line" by dragging its right side onto its left side
    resizeShape(pObject, DragPoint::RightCenter, -nMove, 0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, 2, aStartSize.Height);

    // Continue dragging its right side until it becomes its left side
    resizeShape(pObject, DragPoint::RightCenter, -nMove, 0);
    checkShapePosition(pObject, aStartPosition.X - nGridSize, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    // Fold the shape into a "point" by dragging its top-left corner onto its bottom-right corner
    resizeShape(pObject, DragPoint::TopLeft, nMove, nMove);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y + nGridSize);
    checkShapeOpenSize(pObject, 2, 2);

    // Continue dragging its top-left corner, until its top-left corner becomes its bottom-right corner
    resizeShape(pObject, DragPoint::TopLeft, nMove, nMove);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y + nGridSize);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);
}

CPPUNIT_TEST_FIXTURE(SwSnapToGridTest, testResizeShapeOnUnequalGrid)
{
    // Apply grid options
    const sal_Int32 nGridSizeX = 1016;
    const sal_Int32 nGridSizeY = 900;
    applyGridOptions(true, nGridSizeX, nGridSizeY, 0, 0);

    // Load a document with a shape at grid origin
    createSwDoc("shapeAtGridOrigin.fodt");

    // Check original position and size of the shape
    const awt::Point aStartPosition{ 0, 0 };
    const awt::Size aStartSize{ 642, 526 };
    SdrObject* pObject = getShapeObject(0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    const tools::Long nMove = std::max(nGridSizeX / 2, nGridSizeY / 2);

    // Drag & resize the shape to the right and downward to align it with the grid
    resizeShape(pObject, DragPoint::BottomRight, nMove, nMove);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, nGridSizeX, nGridSizeY);
}

CPPUNIT_TEST_FIXTURE(SwSnapToGridTest, testResizeShapeOnGridWithSubdivision)
{
    // Apply grid options
    const sal_Int32 nGridSize = 3048;
    applyGridOptions(true, nGridSize, nGridSize, 2, 1);

    // Load a document with a shape at grid origin
    createSwDoc("shapeAtGridOrigin.fodt");

    // Check original position and size of the shape
    const awt::Point aStartPosition{ 0, 0 };
    const awt::Size aStartSize{ 642, 526 };
    SdrObject* pObject = getShapeObject(0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    // Drag & resize the shape to the right and downward to align it with the grid
    {
        const tools::Long nMove = nGridSize / 3 / 2;
        resizeShape(pObject, DragPoint::BottomRight, nMove, nMove);
        checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
        checkShapeOpenSize(pObject, nGridSize / 3, nGridSize / 2);
    }

    // Drag & resize the shape to the right and downward to the next grid point
    {
        const tools::Long nMove = nGridSize / 3;
        resizeShape(pObject, DragPoint::BottomRight, nMove, nMove);
        checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
        checkShapeOpenSize(pObject, nGridSize / 3 * 2, nGridSize);
    }

    // Drag & resize the shape to the right and downward to the next grid point
    {
        const tools::Long nMove = nGridSize / 3;
        resizeShape(pObject, DragPoint::BottomRight, nMove, nMove);
        checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
        checkShapeOpenSize(pObject, nGridSize, nGridSize / 2 * 3);
    }
}

CPPUNIT_TEST_FIXTURE(SwSnapToGridTest, testSnapToGridByResize)
{
    // Apply grid options
    const sal_Int32 nGridSize = 1016;
    applyGridOptions(true, nGridSize, nGridSize, 0, 0);

    // Load a document with a shape off grid
    createSwDoc("shapeOffGrid.fodt");

    // Check original position and size of the shape
    const awt::Point aStartPosition{ 1132, 1117 };
    const awt::Size aStartSize{ 1016, 1016 };
    SdrObject* pObject = getShapeObject(0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    const tools::Long nMove = nGridSize / 2;

    // Drag & resize the shape's top edge to align with the grid
    resizeShape(pObject, DragPoint::TopCenter, 0, -nMove);
    checkShapePosition(pObject, aStartPosition.X, nGridSize);
    checkShapeOpenSize(pObject, aStartSize.Width, 1117);

    // Drag & resize the shape's left edge to align with the grid
    resizeShape(pObject, DragPoint::LeftCenter, -nMove, 0);
    checkShapePosition(pObject, nGridSize, nGridSize);
    checkShapeOpenSize(pObject, 1132, 1117);

    // Drag & resize the shape's bottom edge to align with the grid
    resizeShape(pObject, DragPoint::BottomCenter, 0, -nMove);
    checkShapePosition(pObject, nGridSize, nGridSize);
    checkShapeOpenSize(pObject, 1132, nGridSize);

    // Drag & resize the shape's right edge to align with the grid
    resizeShape(pObject, DragPoint::RightCenter, -nMove, 0);
    checkShapePosition(pObject, nGridSize, nGridSize);
    checkShapeOpenSize(pObject, nGridSize, nGridSize);
}

CPPUNIT_TEST_FIXTURE(SwSnapToGridTest, testMoveShapeOutsideOfGridArea)
{
    // Apply grid options
    const sal_Int32 nGridSize = 1016;
    applyGridOptions(true, nGridSize, nGridSize, 0, 0);

    // Load a document with a single shape
    createSwDoc("shapeSnappedToGrid.fodt");

    // Check the original position and size of the shape (it is aligned to the grid)
    const awt::Point aStartPosition{ 1016, 1016 };
    const awt::Size aStartSize{ 1016, 1016 };
    SdrObject* pObject = getShapeObject(0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    const tools::Long nMove = 2 * nGridSize + 10;

    // Move the shape out of the grid area (shape still follows the grid)
    moveShape(pObject, -nMove, 0);
    checkShapePosition(pObject, -nGridSize, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);
}

CPPUNIT_TEST_FIXTURE(SwSnapToGridTest, testSnapLineToGrid)
{
    // Apply grid options
    const sal_Int32 nGridSize = 1016;
    applyGridOptions(true, nGridSize, nGridSize, 0, 0);

    // Load a document with a line off grid
    createSwDoc("lineOffGrid.fodt");

    // Check original position and size of the line
    const awt::Point aStartPosition{ 4313, 1221 };
    const awt::Size aStartSize{ 1901, 1090 };
    SdrObject* pObject = getShapeObject(0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    // Drag & resize the line's left end to align with the grid
    {
        const tools::Long nMove = nGridSize / 2;
        resizeShape(pObject, DragPoint::TopLeft, -nMove, -nMove);
        checkShapePosition(pObject, 4 * nGridSize, nGridSize);
        checkShapeOpenSize(pObject, 2150, 1295);
    }

    // Drag & resize the line's right end to align with the grid
    {
        const tools::Long nMove = nGridSize / 2;
        resizeShape(pObject, DragPoint::BottomRight, nMove, nMove);
        checkShapePosition(pObject, 4 * nGridSize, nGridSize);
        checkShapeOpenSize(pObject, 3 * nGridSize, 2 * nGridSize);
    }

    // Move the line on the grid
    {
        const tools::Long nMove = nGridSize - 10;
        moveShape(pObject, nMove, 0);
        checkShapePosition(pObject, 5 * nGridSize, nGridSize);
        checkShapeOpenSize(pObject, 3 * nGridSize, 2 * nGridSize);
    }

    // Resize the line on the grid
    {
        const tools::Long nMove = 2 * nGridSize - 10;
        resizeShape(pObject, DragPoint::BottomRight, 0, -nMove);
        checkShapePosition(pObject, 5 * nGridSize, nGridSize);
        checkShapeOpenSize(pObject, 3 * nGridSize, 0);
    }
}

CPPUNIT_TEST_FIXTURE(SwSnapToGridTest, testSnapTextBoxToGrid)
{
    // Apply grid options
    const sal_Int32 nGridSize = 1016;
    applyGridOptions(true, nGridSize, nGridSize, 0, 0);

    // Load a document with a textbox off grid
    createSwDoc("testBoxOffGrid.fodt");

    // Check original position and size of the textbox
    const awt::Point aStartPosition{ 2609, 561 };
    const awt::Size aStartSize{ 1753, 2436 };
    SdrObject* pObject = getShapeObject(0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    // Drag & resize the textbox's top-left corner to align with the grid
    {
        const tools::Long nMove = nGridSize / 2;
        resizeShape(pObject, DragPoint::TopLeft, -nMove, -nMove);
        checkShapePosition(pObject, 2 * nGridSize, 0);
        checkShapeOpenSize(pObject, 2330, 2997);
    }

    // Drag & resize the textbox's bottom-right corner to align with the grid
    {
        const tools::Long nMove = nGridSize / 2;
        resizeShape(pObject, DragPoint::BottomRight, nMove, nMove);
        checkShapePosition(pObject, 2 * nGridSize, 0);
        checkShapeOpenSize(pObject, 3 * nGridSize, 3 * nGridSize);
    }

    // Move the textbox on the grid
    {
        const tools::Long nMove = 2 * nGridSize - 10;
        moveShape(pObject, 0, nMove);
        checkShapePosition(pObject, 2 * nGridSize, 2 * nGridSize);
        checkShapeOpenSize(pObject, 3 * nGridSize, 3 * nGridSize);
    }

    // Resize the textbox on the grid
    {
        const tools::Long nMove = 2 * nGridSize - 10;
        resizeShape(pObject, DragPoint::RightCenter, -nMove, 0);
        checkShapePosition(pObject, 2 * nGridSize, 2 * nGridSize);
        // Since the text content does not fit in the new size of the textbox,
        // its height is increased, but it is not snapped to the grid.
        checkShapeOpenSize(pObject, nGridSize, 3896);
    }
}

CPPUNIT_TEST_FIXTURE(SwSnapToGridTest, testMoveShapeBetweenPages)
{
    // Apply grid options
    const sal_Int32 nGridSize = 1016;
    applyGridOptions(true, nGridSize, nGridSize, 0, 0);

    // Load a document with a shape on the second page
    createSwDoc("shapeOnSecondPage.fodt");

    // Check original position and size of the shape
    const awt::Point aStartPosition{ 3 * nGridSize, nGridSize };
    const awt::Size aStartSize{ 2 * nGridSize, 2 * nGridSize };
    SdrObject* pObject = getShapeObject(0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    // Move the shape onto the first page
    const tools::Long nMove = 10 * nGridSize;
    moveShape(pObject, 0, -nMove);
    checkShapePosition(pObject, 2 * nGridSize, 18 * nGridSize);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);
}

CPPUNIT_TEST_FIXTURE(SwSnapToGridTest, testMoveShapeAnchoredAsCharacter)
{
    // Apply grid options
    const sal_Int32 nGridSize = 1016;
    applyGridOptions(true, nGridSize, nGridSize, 0, 0);

    // Load a document with a shape anchored as character
    createSwDoc("shapeAnchoredAsCharacter.fodt");

    // Check original position and size of the shape
    const awt::Point aStartPosition{ 0, 1543 };
    const awt::Size aStartSize{ nGridSize, nGridSize };
    SdrObject* pObject = getShapeObject(0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    // Try to move the shape upward and to the right, but it snaps to the left margin
    {
        const tools::Long nMove = 2 * nGridSize - 10;
        moveShape(pObject, nMove, -nMove);
        checkShapePosition(pObject, aStartPosition.X, 0);
        checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);
    }

    // Try to move the shape downward and to the right, but it snaps to the left margin
    {
        const tools::Long nMove = 2 * nGridSize - 10;
        moveShape(pObject, nMove, nMove);
        checkShapePosition(pObject, aStartPosition.X, 2 * nGridSize);
        checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);
    }
}

CPPUNIT_TEST_FIXTURE(SwSnapToGridTest, testSnapTextFrameToGridByResize)
{
    // Apply grid options
    const sal_Int32 nGridSize = 1016;
    applyGridOptions(true, nGridSize, nGridSize, 0, 0);

    // Load a document with a text frame off grid
    createSwDoc("textFrameOffGrid.fodt");

    // Check original position and size of the frame
    const awt::Point aStartPosition{ 2568, 1314 };
    const awt::Size aStartSize{ 681, 652 };
    SdrObject* pObject = getShapeObject(0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    const tools::Long nMove = nGridSize / 2;

    // Drag & resize the frame's top edge to align with the grid
    resizeShape(pObject, DragPoint::TopCenter, 0, -nMove);
    checkShapePosition(pObject, aStartPosition.X, nGridSize);
    checkShapeOpenSize(pObject, aStartSize.Width, 1086);

    // Drag & resize the frame's left edge to align with the grid
    resizeShape(pObject, DragPoint::LeftCenter, -nMove, 0);
    checkShapePosition(pObject, 2 * nGridSize, nGridSize);
    checkShapeOpenSize(pObject, 1217, 1086);

    // Drag & resize the frame's bottom edge to align with the grid
    resizeShape(pObject, DragPoint::BottomCenter, 0, nMove);
    checkShapePosition(pObject, 2 * nGridSize, nGridSize);
    checkShapeOpenSize(pObject, 1217, 2 * nGridSize);

    // Drag & resize the frame's right edge to align with the grid
    resizeShape(pObject, DragPoint::RightCenter, nMove, 0);
    checkShapePosition(pObject, 2 * nGridSize, nGridSize);
    checkShapeOpenSize(pObject, 2 * nGridSize, 2 * nGridSize);
}

CPPUNIT_TEST_FIXTURE(SwSnapToGridTest, testMoveTextFrameOnGrid)
{
    // Apply grid options
    const sal_Int32 nGridSize = 1016;
    applyGridOptions(true, nGridSize, nGridSize, 0, 0);

    // Load a document with a text frame
    createSwDoc("textFrameSnappedToGrid.fodt");

    // Check the original position and size of the frame (it is aligned to the grid)
    const awt::Point aStartPosition{ 2032, 2032 };
    const awt::Size aStartSize{ 1016, 1016 };
    SdrObject* pObject = getShapeObject(0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    const tools::Long nMove = 2 * nGridSize - 10;

    // Move the frame two grid positions to the right
    moveShape(pObject, nMove, 0);
    checkShapePosition(pObject, aStartPosition.X + 2 * nGridSize, aStartPosition.Y);

    // Move the frame two grid positions down
    moveShape(pObject, 0, nMove);
    checkShapePosition(pObject, aStartPosition.X + 2 * nGridSize, aStartPosition.Y + 2 * nGridSize);

    // Move the frame two grid positions to the left
    moveShape(pObject, -nMove, 0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y + 2 * nGridSize);

    // Move the frame two grid positions up
    moveShape(pObject, 0, -nMove);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);

    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);
}

CPPUNIT_TEST_FIXTURE(SwSnapToGridTest, testSnapImageToGridByResize)
{
    // Apply grid options
    const sal_Int32 nGridSize = 1016;
    applyGridOptions(true, nGridSize, nGridSize, 0, 0);

    // Load a document with an image off grid
    createSwDoc("imageOffGrid.fodt");

    // Check original position and size of the image
    const awt::Point aStartPosition{ 1799, 894 };
    const awt::Size aStartSize{ 905, 905 };
    SdrObject* pObject = getShapeObject(0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    const tools::Long nMove = nGridSize / 2;

    // Drag & resize the image's top edge to align with the grid
    resizeShape(pObject, DragPoint::TopCenter, 0, -nMove);
    checkShapePosition(pObject, aStartPosition.X, 0 * nGridSize);
    checkShapeOpenSize(pObject, aStartSize.Width, 1799);

    // Drag & resize the image's left edge to align with the grid
    resizeShape(pObject, DragPoint::LeftCenter, -nMove, 0);
    checkShapePosition(pObject, nGridSize, 0 * nGridSize);
    checkShapeOpenSize(pObject, 1688, 1799);

    // Drag & resize the image's bottom edge to align with the grid
    resizeShape(pObject, DragPoint::BottomCenter, 0, nMove);
    checkShapePosition(pObject, nGridSize, 0 * nGridSize);
    checkShapeOpenSize(pObject, 1688, 2 * nGridSize);

    // Drag & resize the image's right edge to align with the grid
    resizeShape(pObject, DragPoint::RightCenter, nMove, 0);
    checkShapePosition(pObject, nGridSize, 0 * nGridSize);
    checkShapeOpenSize(pObject, 2 * nGridSize, 2 * nGridSize);
}

CPPUNIT_TEST_FIXTURE(SwSnapToGridTest, testMoveImageOnGrid)
{
    // Apply grid options
    const sal_Int32 nGridSize = 1016;
    applyGridOptions(true, nGridSize, nGridSize, 0, 0);

    // Load a document with an image
    createSwDoc("imageSnappedToGrid.fodt");

    // Check the original position and size of the image (it is aligned to the grid)
    const awt::Point aStartPosition{ 4064, 1016 };
    const awt::Size aStartSize{ 1016, 1016 };
    SdrObject* pObject = getShapeObject(0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);
    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);

    const tools::Long nMove = 2 * nGridSize - 10;

    // Move the image two grid positions to the right
    moveShape(pObject, nMove, 0);
    checkShapePosition(pObject, aStartPosition.X + 2 * nGridSize, aStartPosition.Y);

    // Move the image two grid positions down
    moveShape(pObject, 0, nMove);
    checkShapePosition(pObject, aStartPosition.X + 2 * nGridSize, aStartPosition.Y + 2 * nGridSize);

    // Move the image two grid positions to the left
    moveShape(pObject, -nMove, 0);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y + 2 * nGridSize);

    // Move the image two grid positions up
    moveShape(pObject, 0, -nMove);
    checkShapePosition(pObject, aStartPosition.X, aStartPosition.Y);

    checkShapeOpenSize(pObject, aStartSize.Width, aStartSize.Height);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
