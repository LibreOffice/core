/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>

#include <sfx2/objsh.hxx>
#include <vcl/scheduler.hxx>
#include <comphelper/propertyvalue.hxx>

using namespace com::sun::star;

class AutoRedactionTest : public UnoApiTest
{
public:
    AutoRedactionTest()
        : UnoApiTest(u"/sfx2/qa/cppunit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(AutoRedactionTest, testImageRedaction)
{
    // Get the images size and position from the writer
    loadFromFile(u"autoredact_img.fodt");
    CPPUNIT_ASSERT_MESSAGE("Failed to load autoredact_img.odt", mxComponent.is());

    uno::Reference<text::XTextGraphicObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xSupplier.is());
    uno::Reference<container::XIndexAccess> xGraphics(xSupplier->getGraphicObjects(),
                                                      uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGraphics.is());

    sal_Int32 nImageCount = 0, nTotalCount = 3;
    std::vector<tools::Rectangle> aImageRectangles;

    for (sal_Int32 i = 0; i < xGraphics->getCount(); i++)
    {
        uno::Reference<beans::XPropertySet> xProps;
        xGraphics->getByIndex(i) >>= xProps;
        if (!xProps.is())
            continue;

        awt::Point aPos;
        awt::Size aSize;

        xProps->getPropertyValue(u"HoriOrientPosition"_ustr) >>= aPos.X;
        xProps->getPropertyValue(u"VertOrientPosition"_ustr) >>= aPos.Y;
        xProps->getPropertyValue(u"Width"_ustr) >>= aSize.Width;
        xProps->getPropertyValue(u"Height"_ustr) >>= aSize.Height;

        aImageRectangles.push_back(
            tools::Rectangle(aPos.X, aPos.Y, aPos.X + aSize.Width, aPos.Y + aSize.Height));
        nImageCount++;
    }

    CPPUNIT_ASSERT_EQUAL(nTotalCount, nImageCount);

    uno::Sequence<beans::PropertyValue> aArgs
        = { comphelper::makePropertyValue(u"RedactImages"_ustr, true) };
    dispatchCommand(mxComponent, u".uno:AutoRedactDoc"_ustr, aArgs);

    Scheduler::ProcessEventsToIdle();

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    CPPUNIT_ASSERT(pDocSh);

    uno::Reference<frame::XModel> xDrawingDoc = pDocSh->GetModel();
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(xDrawingDoc, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDrawPagesSupplier.is());
    uno::Reference<container::XIndexAccess> xDrawPages = xDrawPagesSupplier->getDrawPages();
    CPPUNIT_ASSERT(xDrawPages.is());

    sal_Int32 nRedactRect = 0;
    for (sal_Int32 i = 0; i < xDrawPages->getCount(); i++)
    {
        uno::Reference<drawing::XDrawPage> xDrawPage;
        xDrawPages->getByIndex(i) >>= xDrawPage;
        CPPUNIT_ASSERT(xDrawPage.is());

        // Iterate through all shapes on this page
        for (sal_Int32 j = 0; j < xDrawPage->getCount(); j++)
        {
            uno::Reference<drawing::XShape> xShape;
            xDrawPage->getByIndex(j) >>= xShape;
            CPPUNIT_ASSERT(xShape.is());

            uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
            if (!xShapeProps.is())
                continue;

            if (xShapeProps->getPropertyValue(u"Name"_ustr) == u"ImageRedactionShape"_ustr)
            {
                nRedactRect++;
                awt::Point aPos = xShape->getPosition();
                awt::Size aSize = xShape->getSize();

                tools::Rectangle aRect(aPos.X, aPos.Y, aPos.X + aSize.Width, aPos.Y + aSize.Height);

                bool bMatchesImage = false;
                for (const auto& aImageRect : aImageRectangles)
                {
                    if (aRect.Overlaps(aImageRect))
                    {
                        bMatchesImage = true;
                        break;
                    }
                }

                CPPUNIT_ASSERT_MESSAGE("No proper redaction rectangle for a image", bMatchesImage);
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of redaction rectangles should match number of images",
                                 nImageCount, nRedactRect);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
