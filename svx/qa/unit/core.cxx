/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>

#include <comphelper/storagehelper.hxx>

#include <svx/graphichelper.hxx>
#include <svx/xmlgrhlp.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for svx/source/core/ code.
class Test : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
};

void Test::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void Test::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

constexpr OUStringLiteral DATA_DIRECTORY = u"/svx/qa/unit/data/";

CPPUNIT_TEST_FIXTURE(Test, testGraphicObjectResolver)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "GraphicObjectResolverTest.zip";
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(ZIP_STORAGE_FORMAT_STRING, aURL,
                                                                embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());

    rtl::Reference<SvXMLGraphicHelper> xGraphicHelper
        = SvXMLGraphicHelper::Create(xStorage, SvXMLGraphicHelperMode::Read);
    CPPUNIT_ASSERT(xGraphicHelper.is());

    // Test name in root folder
    {
        uno::Reference<graphic::XGraphic> xGraphic = xGraphicHelper->loadGraphic("SomeImage.png");
        CPPUNIT_ASSERT_EQUAL(true, xGraphic.is());
    }

    // Test name in sub-folder
    {
        uno::Reference<graphic::XGraphic> xGraphic
            = xGraphicHelper->loadGraphic("Pictures/SomeOtherImage.png");
        CPPUNIT_ASSERT_EQUAL(true, xGraphic.is());
    }

    // Test non-existent name
    {
        uno::Reference<graphic::XGraphic> xGraphic;
        try
        {
            xGraphic = xGraphicHelper->loadGraphic("NoneExistent.png");
        }
        catch (const uno::Exception&)
        {
        }
        CPPUNIT_ASSERT_EQUAL(false, xGraphic.is());
    }
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
