/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <sal/types.h>
#include <sfx2/app.hxx>
#include <unotools/tempfile.hxx>
#include <svx/xtable.hxx>
#include <vcl/bitmapex.hxx>
#include <svx/XPropertyTable.hxx>

#include <xmlxtexp.hxx>

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

using namespace css;

class XTableImportExportTest : public CppUnit::TestFixture
{
public:
    void testImportExport();

    virtual void setUp() override
    {
        CppUnit::TestFixture::setUp();
        SfxApplication::GetOrCreate();
    }

    CPPUNIT_TEST_SUITE(XTableImportExportTest);
    CPPUNIT_TEST(testImportExport);
    CPPUNIT_TEST_SUITE_END();
};

void XTableImportExportTest::testImportExport()
{
    utl::TempFile aTempFile(nullptr, true);
    aTempFile.EnableKillingFile();
    OUString aTempURL = aTempFile.GetURL();
    BitmapChecksum aChecksum(0);

    {
        XBitmapList xBitmapList(aTempURL, "REF");
        uno::Reference<container::XNameContainer> xNameContainer(xBitmapList.createInstance());
        CPPUNIT_ASSERT(xNameContainer.is());

        Bitmap aBitmap(Size(5, 5), 24);
        aBitmap.Erase(COL_RED);
        BitmapEx aBitmapEx(aBitmap);
        Graphic aGraphic(aBitmapEx);
        uno::Reference<awt::XBitmap> xBitmap(aGraphic.GetXGraphic(), css::uno::UNO_QUERY);

        xNameContainer->insertByName("SomeBitmap", uno::makeAny(xBitmap));
        xBitmapList.Save();

        aChecksum = aBitmap.GetChecksum();
    }

    {
        XBitmapList xBitmapList(aTempURL, "REF");
        bool bResult = xBitmapList.Load();
        CPPUNIT_ASSERT(bResult);
        uno::Reference<container::XNameContainer> xNameContainer(xBitmapList.createInstance());
        CPPUNIT_ASSERT(xNameContainer.is());

        uno::Any aAny = xNameContainer->getByName("SomeBitmap");
        CPPUNIT_ASSERT(aAny.has<uno::Reference<awt::XBitmap>>());
        auto xBitmap = aAny.get<uno::Reference<awt::XBitmap>>();
        CPPUNIT_ASSERT(xBitmap.is());
        uno::Reference<graphic::XGraphic> xGraphic(xBitmap, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xGraphic.is());
        Graphic aGraphic(xGraphic);
        CPPUNIT_ASSERT(aGraphic);
        Bitmap aBitmap = aGraphic.GetBitmapEx().GetBitmap();
        CPPUNIT_ASSERT_EQUAL(aChecksum, aBitmap.GetChecksum());
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(XTableImportExportTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
