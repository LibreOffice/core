/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

class ShapeInsertionUndoAction : public cppu::WeakImplHelper<XUndoAction>
{
public:
    ShapeInsertionUndoAction(const Reference<XShape>& i_shape,
                             const Reference<XShapes>& i_shapeCollection)
    {
        m_shape = i_shape;
        m_shapeCollection = i_shapeCollection;
    }

    OUString getTitle() override { return u"insert shape"_ustr; }

    virtual void undo() override { m_shapeCollection->remove(m_shape); }

    virtual void redo() override { m_shapeCollection->add(m_shape); }

private:
    Reference<XShape> m_shape;
    Reference<XShapes> m_shapeCollection;
};

/**
 * implements the {@link DocumentTest} interface on top of a drawing document
 */
class DrawingOrPresentationDocumentTest : public DocumentTestBase
{
public:
    DrawingOrPresentationDocumentTest(const Reference<XComponent>& rxDoc)
        : DocumentTestBase(rxDoc)
    {
    }

    virtual void initializeDocument() override
    {
        // remove all shapes - Impress has two default shapes in a new doc; just get rid of them
        Reference<XShapes> firstPageShapes = getFirstPageShapes();
        while (firstPageShapes->getCount() > 0)
            firstPageShapes->remove(Reference<XShape>(firstPageShapes->getByIndex(0), UNO_QUERY));
    }

    virtual void doSingleModification() override
    {
        // add a simple centered shape to the first page
        Rectangle pagePlayground = impl_getFirstPagePlayground();
        impl_createCircleShape((pagePlayground.X + (pagePlayground.Width - BIG_CIRCLE_SIZE) / 2),
                               (pagePlayground.Y + (pagePlayground.Height - BIG_CIRCLE_SIZE) / 2),
                               BIG_CIRCLE_SIZE, FILL_COLOR);
    }

    virtual void verifyInitialDocumentState() override
    {
        Reference<XShapes> firstPageShapes = getFirstPageShapes();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("there should be no shapes at all", sal_Int32(0),
                                     firstPageShapes->getCount());
    }

    virtual void verifySingleModificationDocumentState() override
    {
        Reference<XShapes> firstPageShapes = getFirstPageShapes();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("there should be one shape, not more, not less", sal_Int32(1),
                                     firstPageShapes->getCount());

        Any shape = firstPageShapes->getByIndex(0);
        verifyShapeGeometry(shape, BIG_CIRCLE_SIZE, BIG_CIRCLE_SIZE);
        Reference<XPropertySet> shapeProps(shape, UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "wrong circle type", CIRCLE_TYPE,
            shapeProps->getPropertyValue(u"CircleKind"_ustr).get<CircleKind>());
        //assertEquals( "wrong circle fill color", FILL_COLOR, ((Integer)shapeProps.getPropertyValue( "FillColor" )).intValue() );
        // disable this particular check: A bug in the drawing layer API restores the FillColor to its
        // default value upon re-insertion. This is issue #i115080#
    }

    virtual int doMultipleModifications() override
    {
        // add a simple centered shape to the first page
        Rectangle pagePlayground = impl_getFirstPagePlayground();
        impl_createCircleShape(pagePlayground.X, pagePlayground.Y, SMALL_CIRCLE_SIZE,
                               ALTERNATE_FILL_COLOR);
        impl_createCircleShape(pagePlayground.X + pagePlayground.Width - SMALL_CIRCLE_SIZE,
                               pagePlayground.Y, SMALL_CIRCLE_SIZE, ALTERNATE_FILL_COLOR);
        impl_createCircleShape(pagePlayground.X,
                               pagePlayground.Y + pagePlayground.Height - SMALL_CIRCLE_SIZE,
                               SMALL_CIRCLE_SIZE, ALTERNATE_FILL_COLOR);
        impl_createCircleShape(pagePlayground.X + pagePlayground.Width - SMALL_CIRCLE_SIZE,
                               pagePlayground.Y + pagePlayground.Height - SMALL_CIRCLE_SIZE,
                               SMALL_CIRCLE_SIZE, ALTERNATE_FILL_COLOR);
        return 4;
    }

private:
    void impl_createCircleShape(int i_x, int i_y, int i_size, int i_color)
    {
        Reference<XMultiServiceFactory> xMSF(getDocument(), UNO_QUERY_THROW);
        Reference<XPropertySet> shapeProps(
            xMSF->createInstance(u"com.sun.star.drawing.EllipseShape"_ustr), UNO_QUERY);
        shapeProps->setPropertyValue(u"CircleKind"_ustr, Any(CIRCLE_TYPE));
        shapeProps->setPropertyValue(u"FillColor"_ustr, Any(sal_Int32(i_color)));

        Reference<XShape> shape(shapeProps, UNO_QUERY);
        css::awt::Size shapeSize(i_size, i_size);
        shape->setSize(shapeSize);
        css::awt::Point shapePos(i_x, i_y);
        shape->setPosition(shapePos);

        Reference<XShapes> pageShapes(getFirstPageShapes());
        pageShapes->add(shape);

        // Sadly, Draw/Impress currently do not create Undo actions for programmatic changes to the document.
        // Which renders the test here slightly useless ... unless we fake the Undo actions ourself.
        Reference<XUndoManagerSupplier> suppUndoManager(m_document, UNO_QUERY);
        Reference<XUndoManager> undoManager = suppUndoManager->getUndoManager();
        undoManager->addUndoAction(new ShapeInsertionUndoAction(shape, pageShapes));
    }

    Rectangle impl_getFirstPagePlayground()
    {
        Reference<XShapes> firstPageShapes = getFirstPageShapes();
        Reference<XPropertySet> firstPageProps(firstPageShapes, UNO_QUERY);
        sal_Int32 pageWidth = firstPageProps->getPropertyValue(u"Width"_ustr).get<sal_Int32>();
        sal_Int32 pageHeight = firstPageProps->getPropertyValue(u"Height"_ustr).get<sal_Int32>();
        sal_Int32 borderLeft
            = firstPageProps->getPropertyValue(u"BorderLeft"_ustr).get<sal_Int32>();
        sal_Int32 borderTop = firstPageProps->getPropertyValue(u"BorderTop"_ustr).get<sal_Int32>();
        sal_Int32 borderRight
            = firstPageProps->getPropertyValue(u"BorderRight"_ustr).get<sal_Int32>();
        sal_Int32 borderBottom
            = firstPageProps->getPropertyValue(u"BorderBottom"_ustr).get<sal_Int32>();
        return Rectangle(borderLeft, borderTop, pageWidth - borderLeft - borderRight,
                         pageHeight - borderTop - borderBottom);
    }

    /**
     * returns the XShapes interface of the first page of our drawing document
     */
    Reference<XShapes> getFirstPageShapes()
    {
        Reference<XDrawPagesSupplier> suppPages(getDocument(), UNO_QUERY);
        Reference<XDrawPages> pages = suppPages->getDrawPages();
        return Reference<XShapes>(pages->getByIndex(0), UNO_QUERY);
    }

    /**
     * verifies the given shape has the given size
     */
    static void verifyShapeGeometry(const Any& i_shapeObject, sal_Int32 i_expectedWidth,
                                    sal_Int32 i_expectedHeight)
    {
        Reference<XShape> shape(i_shapeObject, UNO_QUERY);
        css::awt::Size shapeSize = shape->getSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected shape width", i_expectedWidth, shapeSize.Width);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected shape height", i_expectedHeight, shapeSize.Height);
    }

    static constexpr CircleKind CIRCLE_TYPE = CircleKind_FULL;
    static constexpr int FILL_COLOR = 0xCC2244;
    static constexpr int ALTERNATE_FILL_COLOR = 0x44CC22;
    static constexpr int BIG_CIRCLE_SIZE = 5000;
    static constexpr int SMALL_CIRCLE_SIZE = 2000;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
