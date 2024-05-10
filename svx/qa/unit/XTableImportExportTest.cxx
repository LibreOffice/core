/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <unotest/bootstrapfixturebase.hxx>

#include <sal/types.h>
#include <sfx2/app.hxx>
#include <unotools/tempfile.hxx>
#include <svx/xtable.hxx>
#include <vcl/bitmapex.hxx>

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

using namespace css;

class XTableImportExportTest : public CppUnit::TestFixture
{
public:
    virtual void setUp() override
    {
        CppUnit::TestFixture::setUp();
        SfxApplication::GetOrCreate();
    }
};

CPPUNIT_TEST_FIXTURE(XTableImportExportTest, testImportExport)
{
    utl::TempFileNamed aTempFile(nullptr, true);
    aTempFile.EnableKillingFile();
    OUString aTempURL = aTempFile.GetURL();
    BitmapChecksum aChecksum(0);

    {
        rtl::Reference<XBitmapList> xBitmapList = new XBitmapList(aTempURL, u"REF"_ustr);
        uno::Reference<container::XNameContainer> xNameContainer(xBitmapList->createInstance());
        CPPUNIT_ASSERT(xNameContainer.is());

        Bitmap aBitmap(Size(5, 5), vcl::PixelFormat::N24_BPP);
        aBitmap.Erase(COL_RED);
        BitmapEx aBitmapEx(aBitmap);
        Graphic aGraphic(aBitmapEx);
        uno::Reference<awt::XBitmap> xBitmap(aGraphic.GetXGraphic(), css::uno::UNO_QUERY);

        xNameContainer->insertByName(u"SomeBitmap"_ustr, uno::Any(xBitmap));
        xBitmapList->Save();

        aChecksum = aBitmap.GetChecksum();
    }

    {
        rtl::Reference<XBitmapList> xBitmapList = new XBitmapList(aTempURL, u"REF"_ustr);
        bool bResult = xBitmapList->Load();
        CPPUNIT_ASSERT(bResult);
        uno::Reference<container::XNameContainer> xNameContainer(xBitmapList->createInstance());
        CPPUNIT_ASSERT(xNameContainer.is());

        uno::Any aAny = xNameContainer->getByName(u"SomeBitmap"_ustr);
        CPPUNIT_ASSERT(aAny.has<uno::Reference<awt::XBitmap>>());
        auto xBitmap = aAny.get<uno::Reference<awt::XBitmap>>();
        CPPUNIT_ASSERT(xBitmap.is());
        uno::Reference<graphic::XGraphic> xGraphic(xBitmap, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xGraphic.is());
        Graphic aGraphic(xGraphic);
        CPPUNIT_ASSERT(!aGraphic.IsNone());
        Bitmap aBitmap = aGraphic.GetBitmapEx().GetBitmap();
        CPPUNIT_ASSERT_EQUAL(aChecksum, aBitmap.GetChecksum());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
