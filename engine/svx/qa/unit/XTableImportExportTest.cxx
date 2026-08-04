/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <comphelper/storagehelper.hxx>
#include <unotools/tempfile.hxx>
#include <svx/xtable.hxx>
#include <tools/color.hxx>
#include <vcl/bitmap.hxx>

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/embed/XStorage.hpp>
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
        Graphic aGraphic(aBitmap);
        uno::Reference<awt::XBitmap> xBitmap(aGraphic.GetXGraphic(), css::uno::UNO_QUERY);

        xNameContainer->insertByName(u"SomeBitmap"_ustr, cpo::uno::Any(xBitmap));
        xBitmapList->Save();

        aChecksum = aBitmap.GetChecksum();
    }

    {
        rtl::Reference<XBitmapList> xBitmapList = new XBitmapList(aTempURL, u"REF"_ustr);
        bool bResult = xBitmapList->Load();
        CPPUNIT_ASSERT(bResult);
        uno::Reference<container::XNameContainer> xNameContainer(xBitmapList->createInstance());
        CPPUNIT_ASSERT(xNameContainer.is());

        cpo::uno::Any aAny = xNameContainer->getByName(u"SomeBitmap"_ustr);
        CPPUNIT_ASSERT(aAny.has<uno::Reference<awt::XBitmap>>());
        auto xBitmap = aAny.get<uno::Reference<awt::XBitmap>>();
        CPPUNIT_ASSERT(xBitmap.is());
        uno::Reference<graphic::XGraphic> xGraphic(xBitmap, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xGraphic.is());
        Graphic aGraphic(xGraphic);
        CPPUNIT_ASSERT(!aGraphic.IsNone());
        Bitmap aBitmap = aGraphic.GetBitmap().CreateColorBitmap();
        CPPUNIT_ASSERT_EQUAL(aChecksum, aBitmap.GetChecksum());
    }
}

CPPUNIT_TEST_FIXTURE(XTableImportExportTest, testImportExportInStorage)
{
    // A colour list kept inside a document goes into the document package under a name of its own,
    // and comes back from there with its entries and with the mark that says it lives in the
    // document.
    uno::Reference<embed::XStorage> xStorage = comphelper::OStorageHelper::GetTemporaryStorage();

    OUString aWrittenName;
    {
        rtl::Reference<XColorList> xColorList = new XColorList(u""_ustr, u""_ustr);
        // A list that still counts as dirty fills itself with the standard colours on first use,
        // which would bury the single entry this test is about.
        xColorList->SetDirty(false);
        xColorList->Insert(std::make_unique<XColorEntry>(COL_RED, u"SomeColor"_ustr));
        CPPUNIT_ASSERT(xColorList->SaveTo(xStorage, u"ColorTable"_ustr, &aWrittenName));
    }

    // A colour list is a single stream in the package, so it takes the .xml extension.
    CPPUNIT_ASSERT_EQUAL(u"ColorTable.xml"_ustr, aWrittenName);

    rtl::Reference<XColorList> xColorList = new XColorList(u""_ustr, u""_ustr);
    CPPUNIT_ASSERT(xColorList->LoadFrom(xStorage, aWrittenName, u""_ustr));
    CPPUNIT_ASSERT(xColorList->IsEmbedInDocument());
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(1), xColorList->Count());
    CPPUNIT_ASSERT_EQUAL(u"SomeColor"_ustr, xColorList->GetColor(0)->GetName());
    CPPUNIT_ASSERT_EQUAL(COL_RED, xColorList->GetColor(0)->GetColor());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
