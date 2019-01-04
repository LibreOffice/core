/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/AutoTextContainer.hpp>
#include <com/sun/star/text/XAutoTextGroup.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/URIs.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XToolkit.hpp>
#include <comphelper/propertyvalue.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <wrtsh.hxx>
#include <ndtxt.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;

namespace
{
char const DATA_DIRECTORY[] = "/sw/qa/extras/unowriter/data/";
}

/// Test to assert UNO API call results of Writer.
class SwUnoWriter : public SwModelTestBase
{
public:
    SwUnoWriter()
        : SwModelTestBase(DATA_DIRECTORY, "writer8")
    {
    }
};

/**
 * Macro to declare a new test with preloaded file
 * (similar to DECLARE_SW_ROUNDTRIP_TEST)
 */
#define DECLARE_UNOAPI_TEST_FILE(TestName, filename)                                               \
    class TestName : public SwUnoWriter                                                            \
    {                                                                                              \
    protected:                                                                                     \
        virtual OUString getTestName() override { return OUString(#TestName); }                    \
                                                                                                   \
    public:                                                                                        \
        CPPUNIT_TEST_SUITE(TestName);                                                              \
        CPPUNIT_TEST(loadAndTest);                                                                 \
        CPPUNIT_TEST_SUITE_END();                                                                  \
        void loadAndTest()                                                                         \
        {                                                                                          \
            load(mpTestDocumentPath, filename);                                                    \
            runTest();                                                                             \
        }                                                                                          \
        void runTest();                                                                            \
    };                                                                                             \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName);                                                     \
    void TestName::runTest()

/**
 * Macro to declare a new test without loading any files
 */
#define DECLARE_UNOAPI_TEST(TestName)                                                              \
    class TestName : public SwUnoWriter                                                            \
    {                                                                                              \
    protected:                                                                                     \
        virtual OUString getTestName() override { return OUString(#TestName); }                    \
                                                                                                   \
    public:                                                                                        \
        CPPUNIT_TEST_SUITE(TestName);                                                              \
        CPPUNIT_TEST(runTest);                                                                     \
        CPPUNIT_TEST_SUITE_END();                                                                  \
        void runTest();                                                                            \
    };                                                                                             \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName);                                                     \
    void TestName::runTest()

DECLARE_UNOAPI_TEST_FILE(testRenderablePagePosition, "renderable-page-position.odt")
{
    // Make sure that the document has 2 pages.
    uno::Reference<view::XRenderable> xRenderable(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Any aSelection = uno::makeAny(mxComponent);

    uno::Reference<awt::XToolkit> xToolkit = VCLUnoHelper::CreateToolkit();
    uno::Reference<awt::XDevice> xDevice(xToolkit->createScreenCompatibleDevice(32, 32));

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<frame::XController> xController = xModel->getCurrentController();

    beans::PropertyValues aRenderOptions = {
        comphelper::makePropertyValue("IsPrinter", true),
        comphelper::makePropertyValue("RenderDevice", xDevice),
        comphelper::makePropertyValue("View", xController),
        comphelper::makePropertyValue("RenderToGraphic", true),
    };

    sal_Int32 nPages = xRenderable->getRendererCount(aSelection, aRenderOptions);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), nPages);

    // Make sure that the first page has some offset.
    comphelper::SequenceAsHashMap aRenderer1(
        xRenderable->getRenderer(0, aSelection, aRenderOptions));
    // Without the accompanying fix in place, this test would have failed: i.e.
    // there was no PagePos key in this map.
    awt::Point aPosition1 = aRenderer1["PagePos"].get<awt::Point>();
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aPosition1.X);
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aPosition1.Y);

    // Make sure that the second page is below the first one.
    comphelper::SequenceAsHashMap aRenderer2(
        xRenderable->getRenderer(1, aSelection, aRenderOptions));
    awt::Point aPosition2 = aRenderer2["PagePos"].get<awt::Point>();
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aPosition2.X);
    CPPUNIT_ASSERT_GREATER(aPosition1.Y, aPosition2.Y);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
