/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <memory>

#include <o3tl/string_view.hxx>
#include <svtools/colorcfg.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/BitmapReadAccess.hxx>

#include <docsh.hxx>
#include <wrtsh.hxx>
#include <ndtxt.hxx>
#include <swmodule.hxx>
#include <swdll.hxx>

namespace
{
/// Covers sw/source/core/text/itrpaint.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/text/data/"_ustr)
    {
    }
};

/// #RRGGBB -> HSL lightness.
sal_Int16 GetColorLightness(std::u16string_view rRGB)
{
    Color aColor(o3tl::toInt32(rRGB.substr(1, 2), 16), o3tl::toInt32(rRGB.substr(3, 2), 16),
                 o3tl::toInt32(rRGB.substr(5, 2), 16));
    sal_uInt16 nHue;
    sal_uInt16 nSaturation;
    sal_uInt16 nBrightness;
    aColor.RGBtoHSB(nHue, nSaturation, nBrightness);
    return nBrightness;
}

int GetColorHue(std::u16string_view rRGB)
{
    Color aColor(o3tl::toInt32(rRGB.substr(1, 2), 16), o3tl::toInt32(rRGB.substr(3, 2), 16),
                 o3tl::toInt32(rRGB.substr(5, 2), 16));
    sal_uInt16 nHue;
    sal_uInt16 nSaturation;
    sal_uInt16 nBrightness;
    aColor.RGBtoHSB(nHue, nSaturation, nBrightness);
    return nHue;
}

CPPUNIT_TEST_FIXTURE(Test, testRedlineRenderModeOmitInsertDelete)
{
    // Reset redline author IDs to a predictable default.
    SwGlobals::ensure();
    SwModule::get()->ClearRedlineAuthors();

    // Default rendering: default, delete, insert.
    createSwDoc("redline.docx");

    SwDocShell* pDocShell = getSwDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pDocShell->GetPreviewMetaFile();

    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    OUString aContent = getXPathContent(pXmlDoc, "(//textarray)[1]/text");
    assertXPath(pXmlDoc, "//textarray", 3);
    sal_Int32 nIndex1 = getXPath(pXmlDoc, "(//textarray)[1]", "index").toInt32();
    sal_Int32 nLength1 = getXPath(pXmlDoc, "(//textarray)[1]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(u"baseline "_ustr, aContent.copy(nIndex1, nLength1));
    OUString aColor1
        = getXPath(pXmlDoc, "(//textarray)[1]/preceding-sibling::textcolor[1]", "color");
    CPPUNIT_ASSERT_EQUAL(u"#000000"_ustr, aColor1);
    sal_Int32 nIndex2 = getXPath(pXmlDoc, "(//textarray)[2]", "index").toInt32();
    sal_Int32 nLength2 = getXPath(pXmlDoc, "(//textarray)[2]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(u"oldcontent"_ustr, aContent.copy(nIndex2, nLength2));
    OUString aColor2
        = getXPath(pXmlDoc, "(//textarray)[2]/preceding-sibling::textcolor[1]", "color");
    Color aRedlineColor = SwModule::get()->GetColorConfig().GetColorValue(svtools::AUTHOR1).nColor;
    OUString aRedlineColorString = u"#"_ustr + aRedlineColor.AsRGBHexString();
    CPPUNIT_ASSERT_EQUAL(aRedlineColorString, aColor2);
    sal_Int32 nIndex3 = getXPath(pXmlDoc, "(//textarray)[3]", "index").toInt32();
    sal_Int32 nLength3 = getXPath(pXmlDoc, "(//textarray)[3]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(u"newcontent"_ustr, aContent.copy(nIndex3, nLength3));
    OUString aColor3
        = getXPath(pXmlDoc, "(//textarray)[3]/preceding-sibling::textcolor[1]", "color");
    CPPUNIT_ASSERT_EQUAL(aRedlineColorString, aColor3);

    // Omit inserts, color deletes: default, red-like, increased lightness.
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwViewOption aOpt(*pWrtShell->GetViewOptions());
    aOpt.SetRedlineRenderMode(SwRedlineRenderMode::OmitInserts);
    pWrtShell->ApplyViewOptions(aOpt);

    xMetaFile = pDocShell->GetPreviewMetaFile();

    pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    assertXPath(pXmlDoc, "//textarray", 3);
    nIndex1 = getXPath(pXmlDoc, "(//textarray)[1]", "index").toInt32();
    nLength1 = getXPath(pXmlDoc, "(//textarray)[1]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(u"baseline "_ustr, aContent.copy(nIndex1, nLength1));
    aColor1 = getXPath(pXmlDoc, "(//textarray)[1]/preceding-sibling::textcolor[1]", "color");
    CPPUNIT_ASSERT_EQUAL(u"#000000"_ustr, aColor1);
    nIndex2 = getXPath(pXmlDoc, "(//textarray)[2]", "index").toInt32();
    nLength2 = getXPath(pXmlDoc, "(//textarray)[2]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(u"oldcontent"_ustr, aContent.copy(nIndex2, nLength2));
    aColor2 = getXPath(pXmlDoc, "(//textarray)[2]/preceding-sibling::textcolor[1]", "color");
    CPPUNIT_ASSERT_EQUAL(0, GetColorHue(aColor2));
    nIndex3 = getXPath(pXmlDoc, "(//textarray)[3]", "index").toInt32();
    nLength3 = getXPath(pXmlDoc, "(//textarray)[3]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(u"newcontent"_ustr, aContent.copy(nIndex3, nLength3));
    aColor3 = getXPath(pXmlDoc, "(//textarray)[3]/preceding-sibling::textcolor[1]", "color");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater or equal than: 49
    // - Actual  : 0
    // i.e. the 3rd text portion had no increased lightness from black.
    CPPUNIT_ASSERT_GREATEREQUAL(static_cast<sal_Int16>(49), GetColorLightness(aColor3));

    // Omit deletes, color inserts: default, increased lightness, green-like.
    aOpt.SetRedlineRenderMode(SwRedlineRenderMode::OmitDeletes);
    pWrtShell->ApplyViewOptions(aOpt);

    xMetaFile = pDocShell->GetPreviewMetaFile();

    pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    assertXPath(pXmlDoc, "//textarray", 3);
    nIndex1 = getXPath(pXmlDoc, "(//textarray)[1]", "index").toInt32();
    nLength1 = getXPath(pXmlDoc, "(//textarray)[1]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(u"baseline "_ustr, aContent.copy(nIndex1, nLength1));
    aColor1 = getXPath(pXmlDoc, "(//textarray)[1]/preceding-sibling::textcolor[1]", "color");
    CPPUNIT_ASSERT_EQUAL(u"#000000"_ustr, aColor1);
    nIndex2 = getXPath(pXmlDoc, "(//textarray)[2]", "index").toInt32();
    nLength2 = getXPath(pXmlDoc, "(//textarray)[2]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(u"oldcontent"_ustr, aContent.copy(nIndex2, nLength2));
    aColor2 = getXPath(pXmlDoc, "(//textarray)[2]/preceding-sibling::textcolor[1]", "color");
    CPPUNIT_ASSERT_GREATEREQUAL(static_cast<sal_Int16>(49), GetColorLightness(aColor2));
    nIndex3 = getXPath(pXmlDoc, "(//textarray)[3]", "index").toInt32();
    nLength3 = getXPath(pXmlDoc, "(//textarray)[3]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(u"newcontent"_ustr, aContent.copy(nIndex3, nLength3));
    aColor3 = getXPath(pXmlDoc, "(//textarray)[3]/preceding-sibling::textcolor[1]", "color");
    CPPUNIT_ASSERT_EQUAL(120, GetColorHue(aColor3));
}

bool IsGrayScale(const BitmapEx& rBitmap)
{
    Bitmap aBitmap = rBitmap.GetBitmap();
    BitmapScopedReadAccess pReadAccess(aBitmap);
    Size aSize = rBitmap.GetSizePixel();
    Color aColor = pReadAccess->GetColor(aSize.getHeight() / 2, aSize.getWidth() / 2);
    return aColor.GetRed() == aColor.GetGreen() && aColor.GetRed() == aColor.GetBlue();
}

std::vector<BitmapEx> GetMetaFileImages(const GDIMetaFile& rMetaFile)
{
    std::vector<BitmapEx> aImages;
    for (size_t nAction = 0; nAction < rMetaFile.GetActionSize(); ++nAction)
    {
        MetaAction* pAction = rMetaFile.GetAction(nAction);
        if (pAction->GetType() != MetaActionType::BMPEXSCALE)
        {
            continue;
        }

        auto pAct = static_cast<MetaBmpExScaleAction*>(pAction);
        aImages.push_back(pAct->GetBitmapEx());
    }
    return aImages;
}

CPPUNIT_TEST_FIXTURE(Test, testAnchoredImageRedlineRenderModeOmitInsertDelete)
{
    // Given a document with a normal, a deleted and an inserted image:
    createSwDoc("redline-image-anchored.docx");

    // When using the standard redline render mode:
    SwDocShell* pDocShell = getSwDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pDocShell->GetPreviewMetaFile();

    // Then make sure none of the images are grayscale:
    std::vector<BitmapEx> aImages = GetMetaFileImages(*xMetaFile);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aImages.size());
    CPPUNIT_ASSERT(!IsGrayScale(aImages[0]));
    CPPUNIT_ASSERT(!IsGrayScale(aImages[1]));
    CPPUNIT_ASSERT(!IsGrayScale(aImages[2]));

    // Omit insert: default, default, grayscale.
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwViewOption aOpt(*pWrtShell->GetViewOptions());
    aOpt.SetRedlineRenderMode(SwRedlineRenderMode::OmitInserts);
    pWrtShell->ApplyViewOptions(aOpt);

    xMetaFile = pDocShell->GetPreviewMetaFile();

    aImages = GetMetaFileImages(*xMetaFile);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aImages.size());
    CPPUNIT_ASSERT(!IsGrayScale(aImages[0]));
    CPPUNIT_ASSERT(!IsGrayScale(aImages[1]));
    // Without the accompanying fix in place, this test would have failed, the image's center pixel
    // wasn't gray.
    CPPUNIT_ASSERT(IsGrayScale(aImages[2]));

    // Omit deletes: default, grayscale, default.
    aOpt.SetRedlineRenderMode(SwRedlineRenderMode::OmitDeletes);
    pWrtShell->ApplyViewOptions(aOpt);

    xMetaFile = pDocShell->GetPreviewMetaFile();

    aImages = GetMetaFileImages(*xMetaFile);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aImages.size());
    CPPUNIT_ASSERT(!IsGrayScale(aImages[0]));
    CPPUNIT_ASSERT(IsGrayScale(aImages[1]));
    CPPUNIT_ASSERT(!IsGrayScale(aImages[2]));
}

CPPUNIT_TEST_FIXTURE(Test, testInlineImageRedlineRenderModeOmitInsertDelete)
{
    // Given a document with a normal, a deleted and an inserted image:
    createSwDoc("redline-image-inline.docx");

    // When using the standard redline render mode:
    SwDocShell* pDocShell = getSwDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pDocShell->GetPreviewMetaFile();

    // Then make sure none of the images are grayscale:
    std::vector<BitmapEx> aImages = GetMetaFileImages(*xMetaFile);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aImages.size());
    CPPUNIT_ASSERT(!IsGrayScale(aImages[0]));
    CPPUNIT_ASSERT(!IsGrayScale(aImages[1]));
    CPPUNIT_ASSERT(!IsGrayScale(aImages[2]));

    // Omit insert: default, default, grayscale.
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwViewOption aOpt(*pWrtShell->GetViewOptions());
    aOpt.SetRedlineRenderMode(SwRedlineRenderMode::OmitInserts);
    pWrtShell->ApplyViewOptions(aOpt);

    xMetaFile = pDocShell->GetPreviewMetaFile();

    aImages = GetMetaFileImages(*xMetaFile);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aImages.size());
    CPPUNIT_ASSERT(!IsGrayScale(aImages[0]));
    CPPUNIT_ASSERT(!IsGrayScale(aImages[1]));
    // Without the accompanying fix in place, this test would have failed, the image's center pixel
    // wasn't gray.
    CPPUNIT_ASSERT(IsGrayScale(aImages[2]));

    // Omit deletes: default, grayscale, default.
    aOpt.SetRedlineRenderMode(SwRedlineRenderMode::OmitDeletes);
    pWrtShell->ApplyViewOptions(aOpt);

    xMetaFile = pDocShell->GetPreviewMetaFile();

    aImages = GetMetaFileImages(*xMetaFile);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aImages.size());
    CPPUNIT_ASSERT(!IsGrayScale(aImages[0]));
    CPPUNIT_ASSERT(IsGrayScale(aImages[1]));
    CPPUNIT_ASSERT(!IsGrayScale(aImages[2]));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
