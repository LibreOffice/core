/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/util/XTheme.hpp>

#include <test/xmldocptr.hxx>
#include <docmodel/uno/UnoTheme.hxx>
#include <docmodel/theme/Theme.hxx>

#include <pptcrypt.hxx>

#include <vector>

using namespace ::com::sun::star;

namespace
{
struct PptCryptTestData
{
    std::vector<sal_uInt8> aSalt;
    std::vector<sal_uInt8> aEncryptedVerifier;
    std::vector<sal_uInt8> aEncryptedHash;
    sal_uInt32 nKeyBits;
};

bool verifyPptPassword(const OUString& rPassword, const PptCryptTestData& rData)
{
    return PptCrypt::verifyPassword(rPassword, rData.aSalt, rData.aEncryptedVerifier,
                                    rData.aEncryptedHash, rData.nKeyBits);
}

const PptCryptTestData aPptCrypt40Bits
    = { { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
          0x0f },
        { 0xd9, 0xdf, 0xcb, 0xaa, 0x50, 0x88, 0x16, 0xba, 0xf4, 0xe2, 0x43, 0x24, 0xdb, 0x19, 0xdf,
          0x56 },
        { 0x37, 0x83, 0x84, 0x93, 0x3d, 0x6b, 0xc7, 0x79, 0x62, 0x29,
          0xad, 0x52, 0x39, 0x63, 0x99, 0xeb, 0x02, 0x21, 0x6f, 0x01 },
        40 };

const PptCryptTestData aPptCrypt56Bits
    = { { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
          0x0f },
        { 0xcb, 0xa2, 0x65, 0xac, 0xd0, 0x11, 0x96, 0x74, 0x7e, 0xc2, 0x74, 0xff, 0x91, 0xb6, 0x66,
          0xf3 },
        { 0x92, 0x17, 0xd7, 0x41, 0xdb, 0xd4, 0x91, 0x87, 0x6e, 0x89,
          0x79, 0x2c, 0x4f, 0xd9, 0x19, 0xeb, 0x72, 0xb3, 0xc2, 0xed },
        56 };

const PptCryptTestData aPptCrypt128Bits
    = { { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
          0x0f },
        { 0x1b, 0x6c, 0x92, 0x22, 0xe7, 0xb6, 0x9a, 0x25, 0xfb, 0x3c, 0xe4, 0xa6, 0x49, 0x90, 0x8a,
          0xa4 },
        { 0x1a, 0x4e, 0x78, 0x6e, 0xd5, 0xc6, 0xe7, 0xd6, 0x0c, 0x74,
          0x7b, 0xb6, 0xce, 0x51, 0x31, 0xdd, 0x5a, 0x3c, 0xb3, 0xa2 },
        128 };

/// Covers sd/source/filter/eppt/ fixes.
class Test : public UnoApiTest
{
public:
    Test();
};

Test::Test()
    : UnoApiTest(u"/sd/qa/filter/eppt/data/"_ustr)
{
}

CPPUNIT_TEST_FIXTURE(Test, testOOXMLCustomShapeBitmapFill)
{
    // Save the bugdoc to PPT.
    loadFromFile(u"custom-shape-bitmap-fill.pptx");
    saveAndReload(TestFilter::PPT);

    // Check if the bitmap shape was lost.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: com.sun.star.drawing.GraphicObjectShape
    // - Actual  : com.sun.star.drawing.CustomShape
    // i.e. the custom shape geometry was kept, but the actual bitmap was lost.
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.GraphicObjectShape"_ustr, xShape->getShapeType());
}

CPPUNIT_TEST_FIXTURE(Test, testThemeExport)
{
    // Given a document with a master slide and a theme, lt1 is set to 0x000002:
    loadFromURL(u"private:factory/simpress"_ustr);
    {
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<drawing::XMasterPageTarget> xDrawPage(
            xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xMasterPage(xDrawPage->getMasterPage(), uno::UNO_QUERY);

        auto pTheme = std::make_shared<model::Theme>("mytheme");
        auto pColorSet = std::make_shared<model::ColorSet>("mycolorscheme");
        pColorSet->add(model::ThemeColorType::Dark1, 0x111111);
        pColorSet->add(model::ThemeColorType::Light1, 0x222222);
        pColorSet->add(model::ThemeColorType::Dark2, 0x333333);
        pColorSet->add(model::ThemeColorType::Light2, 0x444444);
        pColorSet->add(model::ThemeColorType::Accent1, 0x555555);
        pColorSet->add(model::ThemeColorType::Accent2, 0x666666);
        pColorSet->add(model::ThemeColorType::Accent3, 0x777777);
        pColorSet->add(model::ThemeColorType::Accent4, 0x888888);
        pColorSet->add(model::ThemeColorType::Accent5, 0x999999);
        pColorSet->add(model::ThemeColorType::Accent6, 0xaaaaaa);
        pColorSet->add(model::ThemeColorType::Hyperlink, 0xbbbbbb);
        pColorSet->add(model::ThemeColorType::FollowedHyperlink, 0xcccccc);
        pTheme->setColorSet(pColorSet);

        xMasterPage->setPropertyValue(u"Theme"_ustr, uno::Any(model::theme::createXTheme(pTheme)));
    }

    // Export to PPTX and load again:
    saveAndReload(TestFilter::PPTX);

    // Verify that this color is not lost:
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/theme/theme1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:clrScheme/a:lt1/a:srgbClr", "val",
                u"222222"); // expected color 22-22-22

    // Check the theme after loading again
    {
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<drawing::XMasterPageTarget> xDrawPage(
            xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xMasterPage(xDrawPage->getMasterPage(), uno::UNO_QUERY);
        uno::Reference<util::XTheme> xTheme(xMasterPage->getPropertyValue(u"Theme"_ustr),
                                            uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(true, xTheme.is());

        auto* pUnoTheme = dynamic_cast<UnoTheme*>(xTheme.get());
        CPPUNIT_ASSERT(pUnoTheme);
        auto pTheme = pUnoTheme->getTheme();

        CPPUNIT_ASSERT_EQUAL(u"mytheme"_ustr, pTheme->GetName());
        CPPUNIT_ASSERT_EQUAL(u"mycolorscheme"_ustr, pTheme->getColorSet()->getName());
        CPPUNIT_ASSERT_EQUAL(u"Office"_ustr, pTheme->getFontScheme().getName());
        CPPUNIT_ASSERT_EQUAL(u""_ustr, pTheme->getFormatScheme().getName());
    }
}

CPPUNIT_TEST_FIXTURE(Test, testLoopingFromAnimation)
{
    // Given a media shape that has an animation that specifies looping for the video:
    loadFromFile(u"video-loop.pptx");

    // When exporting that to PPTX:
    save(TestFilter::PPTX);

    // Then make sure that the "infinite" repeat count is written:
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    // Without the fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - In <>, XPath '//p:cMediaNode/p:cTn' number of nodes is incorrect
    // i.e. the media node was lost on export, the video no longer looped.
    assertXPath(pXmlDoc, "//p:cMediaNode/p:cTn", "repeatCount", u"indefinite");
}

CPPUNIT_TEST_FIXTURE(Test, testPptCryptAcceptsCorrectPassword)
{
    CPPUNIT_ASSERT(verifyPptPassword(u"LibreOffice-PPT-Test"_ustr, aPptCrypt40Bits));
}

CPPUNIT_TEST_FIXTURE(Test, testPptCryptRejectsWrongPassword)
{
    CPPUNIT_ASSERT(!verifyPptPassword(u"WrongPassword"_ustr, aPptCrypt40Bits));
}

CPPUNIT_TEST_FIXTURE(Test, testPptCryptAcceptsZeroKeySizeAs40Bits)
{
    PptCryptTestData aData = aPptCrypt40Bits;
    aData.nKeyBits = 0;

    CPPUNIT_ASSERT(verifyPptPassword(u"LibreOffice-PPT-Test"_ustr, aData));
}

CPPUNIT_TEST_FIXTURE(Test, testPptCryptAcceptsCorrectPassword56Bits)
{
    CPPUNIT_ASSERT(verifyPptPassword(u"LibreOffice-PPT-Test"_ustr, aPptCrypt56Bits));
}

CPPUNIT_TEST_FIXTURE(Test, testPptCryptRejectsWrongPassword56Bits)
{
    CPPUNIT_ASSERT(!verifyPptPassword(u"WrongPassword"_ustr, aPptCrypt56Bits));
}

CPPUNIT_TEST_FIXTURE(Test, testPptCryptAcceptsCorrectPassword128Bits)
{
    CPPUNIT_ASSERT(verifyPptPassword(u"LibreOffice-PPT-Test"_ustr, aPptCrypt128Bits));
}

CPPUNIT_TEST_FIXTURE(Test, testPptCryptRejectsUnsupportedKeySize)
{
    PptCryptTestData aData = aPptCrypt40Bits;
    aData.nKeyBits = 64;

    CPPUNIT_ASSERT(!verifyPptPassword(u"LibreOffice-PPT-Test"_ustr, aData));
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
