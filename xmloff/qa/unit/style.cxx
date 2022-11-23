/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <test/unoapixml_test.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>

#include <officecfg/Office/Common.hxx>
#include <rtl/character.hxx>

using namespace ::com::sun::star;

/// Covers xmloff/source/style/ fixes.
class XmloffStyleTest : public UnoApiXmlTest
{
public:
    XmloffStyleTest();
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
};

XmloffStyleTest::XmloffStyleTest()
    : UnoApiXmlTest("/xmloff/qa/unit/data/")
{
}

void XmloffStyleTest::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    XmlTestTools::registerODFNamespaces(pXmlXpathCtx);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testFillImageBase64)
{
    // Load a flat ODG that has base64-encoded bitmap as a fill style.
    loadFromURL(u"fill-image-base64.fodg");
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameContainer> xBitmaps(
        xFactory->createInstance("com.sun.star.drawing.BitmapTable"), uno::UNO_QUERY);

    // Without the accompanying fix in place, this test would have failed, as the base64 stream was
    // not considered when parsing the fill-image style.
    CPPUNIT_ASSERT(xBitmaps->hasByName("libreoffice_0"));
}

namespace
{
struct XmlFont
{
    OString aName;
    OString aFontFamilyGeneric;
    bool operator<(const XmlFont& rOther) const
    {
        sal_Int32 nRet = aName.compareTo(rOther.aName);
        if (nRet != 0)
        {
            return nRet < 0;
        }

        return aFontFamilyGeneric.compareTo(rOther.aFontFamilyGeneric) < 0;
    }
};
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testFontSorting)
{
    // Given an empty document with default fonts (Liberation Sans, Lucida Sans, etc):
    mxComponent = loadFromDesktop("private:factory/swriter");

    // When saving that document to ODT:
    save("writer8");

    // Then make sure <style:font-face> elements are sorted (by style:name="..."):
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    xmlXPathObjectPtr pXPath
        = getXPathNode(pXmlDoc, "/office:document-content/office:font-face-decls/style:font-face");
    xmlNodeSetPtr pXmlNodes = pXPath->nodesetval;
    int nNodeCount = xmlXPathNodeSetGetLength(pXmlNodes);
    std::vector<XmlFont> aXMLFonts;
    std::vector<XmlFont> aSortedFonts;
    for (int i = 0; i < nNodeCount; ++i)
    {
        xmlNodePtr pXmlNode = pXmlNodes->nodeTab[i];
        xmlChar* pName = xmlGetProp(pXmlNode, BAD_CAST("name"));
        OString aName(reinterpret_cast<char const*>(pName));

        // Ignore numbers at the end, those are just appended to make all names unique.
        while (rtl::isAsciiDigit(static_cast<sal_uInt32>(aName[aName.getLength() - 1])))
        {
            aName = aName.copy(0, aName.getLength() - 1);
        }

        xmlChar* pFontFamilyGeneric = xmlGetProp(pXmlNode, BAD_CAST("font-family-generic"));
        OString aFontFamilyGeneric;
        if (pFontFamilyGeneric)
        {
            aFontFamilyGeneric = OString(reinterpret_cast<char const*>(pFontFamilyGeneric));
        }

        aXMLFonts.push_back(XmlFont{ aName, aFontFamilyGeneric });
        aSortedFonts.push_back(XmlFont{ aName, aFontFamilyGeneric });
        xmlFree(pName);
    }
    std::sort(aSortedFonts.begin(), aSortedFonts.end());
    size_t nIndex = 0;
    for (const auto& rFont : aSortedFonts)
    {
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: Liberation Sans
        // - Actual  : Lucida Sans1
        // i.e. the output was not lexicographically sorted, "u" was before "i".
        CPPUNIT_ASSERT_EQUAL(rFont.aName, aXMLFonts[nIndex].aName);
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: swiss
        // - Actual  : system
        // i.e. the output was not lexicographically sorted when style:name was the same, but
        // style:font-family-generic was not the same.
        CPPUNIT_ASSERT_EQUAL(rFont.aFontFamilyGeneric, aXMLFonts[nIndex].aFontFamilyGeneric);
        ++nIndex;
    }
    xmlXPathFreeObject(pXPath);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testRtlGutter)
{
    // Given a document with a gutter margin and an RTL writing mode:
    // When loading that document from ODF:
    loadFromURL(u"rtl-gutter.fodt");

    // Then make sure the page style's RtlGutter property is true.
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("PageStyles"),
                                                        uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStandard(xStyleFamily->getByName("Standard"),
                                                  uno::UNO_QUERY);
    bool bRtlGutter{};
    xStandard->getPropertyValue("RtlGutter") >>= bRtlGutter;
    // Without the accompanying fix in place, this test would have failed as
    // <style:page-layout-properties>'s style:writing-mode="..." did not affect RtlGutter.
    CPPUNIT_ASSERT(bRtlGutter);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testWritingModeBTLR)
{
    // Load document. It has a frame style with writing-mode bt-lr.
    // In ODF 1.3 extended it is written as loext:writing-mode="bt-lr".
    // In ODF 1.3 strict, there must not be an attribute at all.
    loadFromURL(u"tdf150407_WritingModeBTLR_style.odt");

    Resetter _([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(3, pBatch);
        return pBatch->commit();
    });

    // Save to ODF 1.3 extended. Adapt 3 (=ODFVER_LATEST) to a to be ODFVER_013_EXTENDED when
    // attribute value "bt-lr" is included in ODF strict.
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(3, pBatch);
        pBatch->commit();
        save("writer8");

        // With applied fix for tdf150407 still loext:writing-mode="bt-lr" has to be written.
        xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
        assertXPath(pXmlDoc,
                    "/office:document-styles/office:styles/style:style[@style:name='FrameBTLR']/"
                    "style:graphic-properties[@loext:writing-mode]");
        assertXPath(pXmlDoc,
                    "/office:document-styles/office:styles/style:style[@style:name='FrameBTLR']/"
                    "style:graphic-properties",
                    "writing-mode", "bt-lr");
    }

    loadFromURL(u"tdf150407_WritingModeBTLR_style.odt");
    // Save to ODF 1.3 strict.
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(10, pBatch);
        pBatch->commit();
        save("writer8");

        // Without the fix an faulty 'writing-mode="bt-lr"' attribute was written in productive build.
        // A debug build fails assertion in SvXMLNamespaceMap::GetQNameByKey().
        xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
        assertXPathNoAttribute(pXmlDoc,
                               "/office:document-styles/office:styles/"
                               "style:style[@style:name='FrameBTLR']/style:graphic-properties",
                               "writing-mode");
    }
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testPosRelBottomMargin)
{
    // Load document. It has a frame position with vertical position relative to bottom margin.
    // In ODF 1.3 extended it is written as loext:vertical-rel="page-content-bottom".
    // In ODF 1.3 strict, there must not be an attribute at all.
    loadFromURL(u"tdf150407_PosRelBottomMargin.docx");

    Resetter _([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(3, pBatch);
        return pBatch->commit();
    });

    // Save to ODF 1.3 extended. Adapt 3 (=ODFVER_LATEST) to a to be ODFVER_013_EXTENDED when
    // attribute value "page-content-bottom" is included in ODF strict.
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(3, pBatch);
        pBatch->commit();
        save("writer8");

        // With applied fix for tdf150407 still loext:vertical-rel="page-content-bottom" has to be
        // written.
        xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
        assertXPath(
            pXmlDoc,
            "/office:document-content/office:automatic-styles/style:style[@style:name='gr1']/"
            "style:graphic-properties[@loext:vertical-rel]");
        assertXPath(
            pXmlDoc,
            "/office:document-content/office:automatic-styles/style:style[@style:name='gr1']/"
            "style:graphic-properties",
            "vertical-rel", "page-content-bottom");
    }

    loadFromURL(u"tdf150407_PosRelBottomMargin.docx");
    // Save to ODF 1.3 strict.
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(10, pBatch);
        pBatch->commit();
        save("writer8");

        // Without the fix an faulty 'vertical-rel="page-content-bottom"' attribute was written in
        // productive build. A debug build fails assertion in SvXMLNamespaceMap::GetQNameByKey().
        xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
        assertXPathNoAttribute(pXmlDoc,
                               "/office:document-content/office:automatic-styles/"
                               "style:style[@style:name='gr1']/style:graphic-properties",
                               "vertical-rel");
    }
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testPosRelTopMargin)
{
    // Load document. It has a frame position with vertical position relative to top margin.
    // In ODF 1.3 extended it is written as loext:vertical-rel="page-content-top".
    // In ODF 1.3 strict, there must not be an attribute at all.
    loadFromURL(u"tdf150407_PosRelTopMargin.docx");

    Resetter _([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(3, pBatch);
        return pBatch->commit();
    });

    // Save to ODF 1.3 extended. Adapt 3 (=ODFVER_LATEST) to a to be ODFVER_013_EXTENDED when
    // attribute value "page-content-top" is included in ODF strict.
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(3, pBatch);
        pBatch->commit();
        save("writer8");

        // With applied fix for tdf150407 still loext:vertical-rel="page-content-top has to be
        // written.
        xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
        assertXPath(
            pXmlDoc,
            "/office:document-content/office:automatic-styles/style:style[@style:name='gr1']/"
            "style:graphic-properties[@loext:vertical-rel]");
        assertXPath(
            pXmlDoc,
            "/office:document-content/office:automatic-styles/style:style[@style:name='gr1']/"
            "style:graphic-properties",
            "vertical-rel", "page-content-top");
    }

    loadFromURL(u"tdf150407_PosRelTopMargin.docx");
    // Save to ODF 1.3 strict.
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(10, pBatch);
        pBatch->commit();
        save("writer8");

        // Without the fix an faulty 'vertical-rel="page-content-top"' attribute was written in
        // productive build. A debug build fails assertion in SvXMLNamespaceMap::GetQNameByKey().
        xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
        assertXPathNoAttribute(pXmlDoc,
                               "/office:document-content/office:automatic-styles/"
                               "style:style[@style:name='gr1']/style:graphic-properties",
                               "vertical-rel");
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
