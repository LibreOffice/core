/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <config_features.h>
#include <config_fonts.h>

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/XFontMappingUse.hpp>
#include <com/sun/star/document/FontsDisallowEditingRequest.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>

#include <comphelper/compbase.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <rtl/ref.hxx>

#include <docsh.hxx>

namespace
{
// Tests for embedded fonts in Writer documents
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/embedded_fonts/data/"_ustr)
    {
    }
};

// A custom interaction handler specifically handling FontsDisallowEditingRequest, allowing to test
// what was requested, and how many times; and to define the answer in tests
class FontInteractionHandler : public comphelper::WeakImplHelper<task::XInteractionHandler>
{
public:
    FontInteractionHandler(bool bApprove)
        : mbApprove(bApprove)
    {
    }

    int getRequestCount() const { return mnRequestCount; }
    OUString getRequestedFontName() const { return maRequestedFontName; }

    virtual void SAL_CALL handle(uno::Reference<task::XInteractionRequest> const& rRequest) override
    {
        const auto aContinuations = rRequest->getContinuations();

        if (handleRestrictedFontRequest(aContinuations, rRequest->getRequest()))
            return;

        for (auto const& continuation : aContinuations)
        {
            if (auto xApprove = continuation.query<task::XInteractionApprove>())
            {
                xApprove->select();
                break;
            }
        }
    }

    bool handleRestrictedFontRequest(
        const uno::Sequence<uno::Reference<task::XInteractionContinuation>>& rContinuations,
        const uno::Any& rRequest)
    {
        document::FontsDisallowEditingRequest aRequest;
        if (!(rRequest >>= aRequest))
            return false;

        ++mnRequestCount;
        maRequestedFontName += aRequest.aFontNames;

        for (auto const& continuation : rContinuations)
        {
            if (mbApprove)
            {
                if (auto xApprove = continuation.query<task::XInteractionApprove>())
                {
                    xApprove->select();
                    break;
                }
            }
            else
            {
                if (auto xDisapprove = continuation.query<task::XInteractionDisapprove>())
                {
                    xDisapprove->select();
                    break;
                }
            }
        }
        return true;
    }

private:
    bool mbApprove; // Approve the request or not
    int mnRequestCount = 0; // How many times had restricted font request happened
    OUString maRequestedFontName;
};

// A wrapped over XFontMappingUse, simplifying checks if some fonts were actually used in tests,
// and if their use required a substitution or not
class FontMappingUseListener
{
public:
    FontMappingUseListener()
        : mxFontMappingUse(awt::Toolkit::create(comphelper::getProcessComponentContext())
                               .queryThrow<awt::XFontMappingUse>())
    {
        mxFontMappingUse->startTrackingFontMappingUse();
    }
    ~FontMappingUseListener() { mxFontMappingUse->finishTrackingFontMappingUse(); }

    void checkpoint()
    {
        maFontMappingUseData = mxFontMappingUse->finishTrackingFontMappingUse();
        mxFontMappingUse->startTrackingFontMappingUse();
    }

    bool wasUsed(std::u16string_view font) const
    {
        for (const auto& element : maFontMappingUseData)
        {
            if (element.originalFont == font)
                return true;
        }
        return false;
    }

    bool wasSubstituted(std::u16string_view font) const
    {
        for (const auto& element : maFontMappingUseData)
        {
            if (element.originalFont != font)
                continue;

            for (const auto& used : element.usedFonts)
            {
                std::u16string_view rest;
                if (!used.startsWith(element.originalFont, &rest)
                    || (!rest.empty() && !rest.starts_with('/')))
                {
                    SAL_INFO("test", element.originalFont << " substituted with " << used);
                    return true;
                }
            }
        }
        return false;
    }

private:
    uno::Reference<awt::XFontMappingUse> mxFontMappingUse;
    uno::Sequence<awt::XFontMappingUseItem> maFontMappingUseData;
};

CPPUNIT_TEST_FIXTURE(Test, testOpenODTWithRestrictedEmbeddedFont)
{
    // The ODT has a restricted embedded font, referenced both from styles.xml and content.xml.
    // Test its loading without and with approval; and check that there are no double requests
    // 1. Load and do not approve the restricted font
    FontMappingUseListener fontMappingData;
    rtl::Reference xInteraction(new FontInteractionHandler(false));
    createSwDoc(
        "embed-restricted-style+autostyle.odt",
        { comphelper::makePropertyValue(u"InteractionHandler"_ustr,
                                        uno::Reference<task::XInteractionHandler>(xInteraction)) });

    // It asked exactly once, even though both styles.xml and content.xml requested the font:
    CPPUNIT_ASSERT_EQUAL(1, xInteraction->getRequestCount());
    // It requested the expected font
    CPPUNIT_ASSERT_EQUAL(u"Naftalene"_ustr, xInteraction->getRequestedFontName().trim());
    // The document is editable:
    CPPUNIT_ASSERT(!getSwDocShell()->IsReadOnly());

    fontMappingData.checkpoint();
    // The request was disapproved, and the font didn't load; so it was substituted:
    CPPUNIT_ASSERT(fontMappingData.wasUsed(u"Naftalene"));
    CPPUNIT_ASSERT(fontMappingData.wasSubstituted(u"Naftalene"));

    // Make sure that saving doesn't somehow embed the font
    save(TestFilter::ODT);
    xmlDocUniquePtr pXml = parseExport(u"content.xml"_ustr);
    assertXPath(pXml, "//style:font-face[@style:name='Naftalene']");
    assertXPath(pXml, "//style:font-face[@style:name='Naftalene']/svg:font-face-src", 0);

    auto xZipFile = packages::zip::ZipFileAccess::createWithURL(
        comphelper::getProcessComponentContext(), maTempFile.GetURL());
    CPPUNIT_ASSERT(xZipFile);
    for (const OUString& name : xZipFile->getElementNames())
        CPPUNIT_ASSERT(name.indexOf("Naftalene") < 0);
}

CPPUNIT_TEST_FIXTURE(Test, testOpenODTWithRestrictedEmbeddedFont2)
{
    // 2. Load and approve the restricted font
    FontMappingUseListener fontMappingData;
    rtl::Reference xInteraction(new FontInteractionHandler(true));
    createSwDoc(
        "embed-restricted-style+autostyle.odt",
        { comphelper::makePropertyValue(u"InteractionHandler"_ustr,
                                        uno::Reference<task::XInteractionHandler>(xInteraction)) });

    // It asked exactly once, even though both styles.xml and content.xml requested the font:
    CPPUNIT_ASSERT_EQUAL(1, xInteraction->getRequestCount());
    // It requested the expected font
    CPPUNIT_ASSERT_EQUAL(u"Naftalene"_ustr, xInteraction->getRequestedFontName().trim());
    // The document loaded read-only:
    CPPUNIT_ASSERT(getSwDocShell()->IsReadOnly());

    fontMappingData.checkpoint();
    // The request was approved, and the font loaded; no substitution happened:
    CPPUNIT_ASSERT(fontMappingData.wasUsed(u"Naftalene"));
    CPPUNIT_ASSERT(!fontMappingData.wasSubstituted(u"Naftalene"));

    // Make sure that saving doesn't somehow embed the font
    save(TestFilter::ODT);
    xmlDocUniquePtr pXml = parseExport(u"content.xml"_ustr);
    assertXPath(pXml, "//style:font-face[@style:name='Naftalene']");
    assertXPath(pXml, "//style:font-face[@style:name='Naftalene']/svg:font-face-src", 0);

    auto xZipFile = packages::zip::ZipFileAccess::createWithURL(
        comphelper::getProcessComponentContext(), maTempFile.GetURL());
    CPPUNIT_ASSERT(xZipFile);
    for (const OUString& name : xZipFile->getElementNames())
        CPPUNIT_ASSERT(name.indexOf("Naftalene") < 0);
}

CPPUNIT_TEST_FIXTURE(Test, testOpenDOCXWithRestrictedEmbeddedFont)
{
    // The DOCX has two embedded fonts, one restricted (Naftalene), one unrestricted (Unsteady
    // Oversteer). Test without interaction handler, and with handler (without and with approval).
    // 1. Load without interaction handler. It must not load the restricted font;
    // unrestricted one must load.
    FontMappingUseListener fontMappingData;
    loadFromFile(u"embed-restricted+unrestricted.docx");

    // The document is editable:
    CPPUNIT_ASSERT(!getSwDocShell()->IsReadOnly());

    fontMappingData.checkpoint();

    // Interaction handler was absent, and the restricted font didn't load; it was substituted:
    CPPUNIT_ASSERT(fontMappingData.wasUsed(u"Naftalene"));
    CPPUNIT_ASSERT(fontMappingData.wasSubstituted(u"Naftalene"));

    // Unrestricted font was loaded and used without substitution:
    CPPUNIT_ASSERT(fontMappingData.wasUsed(u"Unsteady Oversteer"));
    CPPUNIT_ASSERT(!fontMappingData.wasSubstituted(u"Unsteady Oversteer"));

    // Make sure that saving doesn't somehow embed the font
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXml = parseExport(u"word/fontTable.xml"_ustr);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']");
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedRegular", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedBold", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedItalic", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedBoldItalic", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testOpenDOCXWithRestrictedEmbeddedFont2)
{
    // 2. Load and do not approve the restricted font. It must not load the restricted font;
    // unrestricted one must load.
    FontMappingUseListener fontMappingData;
    rtl::Reference xInteraction(new FontInteractionHandler(false));
    createSwDoc(
        "embed-restricted+unrestricted.docx",
        { comphelper::makePropertyValue(u"InteractionHandler"_ustr,
                                        uno::Reference<task::XInteractionHandler>(xInteraction)) });

    CPPUNIT_ASSERT_EQUAL(1, xInteraction->getRequestCount());
    // It requested only the expected font (no requests for 'Unsteady Oversteer')
    CPPUNIT_ASSERT_EQUAL(u"Naftalene"_ustr, xInteraction->getRequestedFontName().trim());
    // The document is editable:
    CPPUNIT_ASSERT(!getSwDocShell()->IsReadOnly());

    fontMappingData.checkpoint();

    // The request was disapproved, and the font didn't load; so it was substituted:
    CPPUNIT_ASSERT(fontMappingData.wasUsed(u"Naftalene"));
    CPPUNIT_ASSERT(fontMappingData.wasSubstituted(u"Naftalene"));

    // Unrestricted font was loaded and used without substitution:
    CPPUNIT_ASSERT(fontMappingData.wasUsed(u"Unsteady Oversteer"));
    CPPUNIT_ASSERT(!fontMappingData.wasSubstituted(u"Unsteady Oversteer"));

    // Make sure that saving doesn't somehow embed the font
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXml = parseExport(u"word/fontTable.xml"_ustr);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']");
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedRegular", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedBold", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedItalic", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedBoldItalic", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testOpenDOCXWithRestrictedEmbeddedFont3)
{
    // 3. Load and approve the restricted font. It must load both fonts, and open in read-only
    // mode.
    FontMappingUseListener fontMappingData;
    rtl::Reference xInteraction(new FontInteractionHandler(true));
    createSwDoc(
        "embed-restricted+unrestricted.docx",
        { comphelper::makePropertyValue(u"InteractionHandler"_ustr,
                                        uno::Reference<task::XInteractionHandler>(xInteraction)) });

    CPPUNIT_ASSERT_EQUAL(1, xInteraction->getRequestCount());
    // It requested the expected font
    CPPUNIT_ASSERT_EQUAL(u"Naftalene"_ustr, xInteraction->getRequestedFontName().trim());
    // The document loaded read-only:
    CPPUNIT_ASSERT(getSwDocShell()->IsReadOnly());

    fontMappingData.checkpoint();

    // The request was approved, and the restricted font loaded; no substitution:
    CPPUNIT_ASSERT(fontMappingData.wasUsed(u"Naftalene"));
    CPPUNIT_ASSERT(!fontMappingData.wasSubstituted(u"Naftalene"));

    // Unrestricted font was loaded and used without substitution:
    CPPUNIT_ASSERT(fontMappingData.wasUsed(u"Unsteady Oversteer"));
    CPPUNIT_ASSERT(!fontMappingData.wasSubstituted(u"Unsteady Oversteer"));

    // Make sure that saving doesn't somehow embed the font
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXml = parseExport(u"word/fontTable.xml"_ustr);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']");
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedRegular", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedBold", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedItalic", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedBoldItalic", 0);
}

#if !defined(MACOSX)
CPPUNIT_TEST_FIXTURE(Test, testTdf167849)
{
    // Given two documents with embedded fonts, that will not require substitution, if present:

    FontMappingUseListener fontMappingData;

    // Load the first document
    createSwDoc("embed-unrestricted1.odt");
    // At this point, 'Manbow Solid' embedded font is loaded
    std::swap(mxComponent, mxComponent2); // keep it from unloading upon the next load

    fontMappingData.checkpoint();
    CPPUNIT_ASSERT(fontMappingData.wasUsed(u"Manbow Solid"));
    CPPUNIT_ASSERT(!fontMappingData.wasSubstituted(u"Manbow Solid"));

    // Load the second document
    createSwDoc("embed-unrestricted2.odt");
    // At this point, 'Unsteady Oversteer' font is also loaded

    fontMappingData.checkpoint();
    CPPUNIT_ASSERT(fontMappingData.wasUsed(u"Unsteady Oversteer"));
    CPPUNIT_ASSERT(!fontMappingData.wasSubstituted(u"Unsteady Oversteer"));

    // Re-layout both documents; both fonts must still be loaded
    calcLayout(true);
    std::swap(mxComponent, mxComponent2);
    calcLayout(true);

    fontMappingData.checkpoint();
    CPPUNIT_ASSERT(fontMappingData.wasUsed(u"Manbow Solid"));
    // Without the fix, it would fail, because loading the second document unregistered
    // the embedded font from the first one.
    CPPUNIT_ASSERT(!fontMappingData.wasSubstituted(u"Manbow Solid"));
    CPPUNIT_ASSERT(fontMappingData.wasUsed(u"Unsteady Oversteer"));
    CPPUNIT_ASSERT(!fontMappingData.wasSubstituted(u"Unsteady Oversteer"));
}
#endif

CPPUNIT_TEST_FIXTURE(Test, tdf166627)
{
    createSwDoc("font_used_in_header_only.fodt");
    saveAndReload(TestFilter::ODT);
    // DejaVu Serif wasn't embedded before fix, because it was only seen used in header

    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);

    // There should be four files embedded for the font
    assertXPath(pXmlDoc,
                "//style:font-face[@style:name='DejaVu Serif']/svg:font-face-src/svg:font-face-uri",
                4);

    auto xZipNames = packages::zip::ZipFileAccess::createWithURL(
        comphelper::getProcessComponentContext(), maTempFile.GetURL());

    OUString url = getXPath(
        pXmlDoc,
        "//style:font-face[@style:name='DejaVu Serif']/svg:font-face-src/svg:font-face-uri[1]",
        "href");
    CPPUNIT_ASSERT(xZipNames->hasByName(url));
    url = getXPath(
        pXmlDoc,
        "//style:font-face[@style:name='DejaVu Serif']/svg:font-face-src/svg:font-face-uri[2]",
        "href");
    CPPUNIT_ASSERT(xZipNames->hasByName(url));
    url = getXPath(
        pXmlDoc,
        "//style:font-face[@style:name='DejaVu Serif']/svg:font-face-src/svg:font-face-uri[3]",
        "href");
    CPPUNIT_ASSERT(xZipNames->hasByName(url));
    url = getXPath(
        pXmlDoc,
        "//style:font-face[@style:name='DejaVu Serif']/svg:font-face-src/svg:font-face-uri[4]",
        "href");
    CPPUNIT_ASSERT(xZipNames->hasByName(url));

    // also test that common fonts don't get embedded

    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testFontEmbedding)
{
#if HAVE_MORE_FONTS && !defined(MACOSX)
    createSwDoc("testFontEmbedding.odt");

    OString aContentBaseXpath("/office:document-content/office:font-face-decls"_ostr);
    OString aStylesBaseXpath("/office:document-styles/office:font-face-decls"_ostr);
    OString aSettingsBaseXpath(
        "/office:document-settings/office:settings/config:config-item-set"_ostr);

    xmlDocUniquePtr pXmlDoc;

    // Get document settings
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xProps(
        xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW);

    // Check font embedding state
    CPPUNIT_ASSERT_EQUAL(false, xProps->getPropertyValue(u"EmbedFonts"_ustr).get<bool>());
    CPPUNIT_ASSERT_EQUAL(false, xProps->getPropertyValue(u"EmbedOnlyUsedFonts"_ustr).get<bool>());
    // Font scripts should be enabled by default, however this has no effect unless "EmbedOnlyUsedFonts" is enabled
    CPPUNIT_ASSERT_EQUAL(true, xProps->getPropertyValue(u"EmbedLatinScriptFonts"_ustr).get<bool>());
    CPPUNIT_ASSERT_EQUAL(true, xProps->getPropertyValue(u"EmbedAsianScriptFonts"_ustr).get<bool>());
    CPPUNIT_ASSERT_EQUAL(true,
                         xProps->getPropertyValue(u"EmbedComplexScriptFonts"_ustr).get<bool>());

    // CASE 1 - no font embedding enabled

    // Save the document
    save(TestFilter::ODT);
    CPPUNIT_ASSERT(maTempFile.IsValid());

    // Check setting - No font embedding should be enabled
    pXmlDoc = parseExport(u"settings.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedFonts']", u"false");

    // Check styles - No font-face-src nodes should be present
    pXmlDoc = parseExport(u"styles.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, aStylesBaseXpath + "/style:font-face['CASE 1']", 5);
    for (auto fontName : { "DejaVu Sans", "DejaVu Sans Mono", "DejaVu Serif",
                           "DejaVu Serif Condensed", "DejaVu Serif Condensed1" })
    {
        OString prefix = aStylesBaseXpath + "/style:font-face[@style:name='" + fontName + "']";
        assertXPath(pXmlDoc, prefix + "['CASE 1']");
        assertXPath(pXmlDoc, prefix + "/svg:font-face-src['CASE 1']", 0);
    }

    // Check content - No font-face-src nodes should be present
    pXmlDoc = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face['CASE 1']", 5);
    for (auto fontName : { "DejaVu Sans", "DejaVu Sans Mono", "DejaVu Serif",
                           "DejaVu Serif Condensed", "DejaVu Serif Condensed1" })
    {
        OString prefix = aContentBaseXpath + "/style:font-face[@style:name='" + fontName + "']";
        assertXPath(pXmlDoc, prefix + "['CASE 1']");
        assertXPath(pXmlDoc, prefix + "/svg:font-face-src['CASE 1']", 0);
    }

    // CASE 2 - font embedding enabled, but embed used fonts disabled

    // Enable font embedding, disable embedding used font only
    xProps->setPropertyValue(u"EmbedFonts"_ustr, uno::Any(true));
    xProps->setPropertyValue(u"EmbedOnlyUsedFonts"_ustr, uno::Any(false));

    // Save the document again
    save(TestFilter::ODT);
    CPPUNIT_ASSERT(maTempFile.IsValid());

    // Check setting - font embedding should be enabled
    pXmlDoc = parseExport(u"settings.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedFonts']", u"true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedOnlyUsedFonts']",
        u"false");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedLatinScriptFonts']",
        u"true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedAsianScriptFonts']",
        u"true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedComplexScriptFonts']",
        u"true");

    // Check styles - font-face-src should be present for all fonts
    pXmlDoc = parseExport(u"styles.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, aStylesBaseXpath + "/style:font-face['CASE 2']", 5);
    for (auto fontName : { "DejaVu Sans", "DejaVu Sans Mono", "DejaVu Serif",
                           "DejaVu Serif Condensed", "DejaVu Serif Condensed1" })
    {
        OString prefix = aStylesBaseXpath + "/style:font-face[@style:name='" + fontName + "']";
        assertXPath(pXmlDoc, prefix + "['CASE 2']");
        assertXPath(pXmlDoc, prefix + "/svg:font-face-src['CASE 2']", 1);
    }

    // Check content - font-face-src should be present for all fonts
    pXmlDoc = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face['CASE 2']", 5);
    for (auto fontName : { "DejaVu Sans", "DejaVu Sans Mono", "DejaVu Serif",
                           "DejaVu Serif Condensed", "DejaVu Serif Condensed1" })
    {
        OString prefix = aContentBaseXpath + "/style:font-face[@style:name='" + fontName + "']";
        assertXPath(pXmlDoc, prefix + "['CASE 2']");
        assertXPath(pXmlDoc, prefix + "/svg:font-face-src['CASE 2']", 1);
    }

    // CASE 3 - font embedding enabled, embed only used fonts enabled

    // Enable font embedding and setting to embed used fonts only
    xProps->setPropertyValue(u"EmbedFonts"_ustr, uno::Any(true));
    xProps->setPropertyValue(u"EmbedOnlyUsedFonts"_ustr, uno::Any(true));
    xProps->setPropertyValue(u"EmbedLatinScriptFonts"_ustr, uno::Any(true));
    xProps->setPropertyValue(u"EmbedAsianScriptFonts"_ustr, uno::Any(true));
    xProps->setPropertyValue(u"EmbedComplexScriptFonts"_ustr, uno::Any(true));

    // Save the document again
    save(TestFilter::ODT);
    CPPUNIT_ASSERT(maTempFile.IsValid());

    // Check setting - font embedding should be enabled + embed only used fonts and scripts
    pXmlDoc = parseExport(u"settings.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedFonts']", u"true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedOnlyUsedFonts']",
        u"true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedLatinScriptFonts']",
        u"true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedAsianScriptFonts']",
        u"true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedComplexScriptFonts']",
        u"true");

    // Check styles - font-face-src should be present only for "DejaVu Serif"
    pXmlDoc = parseExport(u"styles.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, aStylesBaseXpath + "/style:font-face['CASE 3']", 5);
    // 'DejaVu Sans' is by default exported because of table styles
    for (auto fontName :
         { "DejaVu Sans Mono", "DejaVu Serif Condensed", "DejaVu Serif Condensed1" })
    {
        OString prefix = aStylesBaseXpath + "/style:font-face[@style:name='" + fontName + "']";
        assertXPath(pXmlDoc, prefix + "['CASE 3']");
        assertXPath(pXmlDoc, prefix + "/svg:font-face-src['CASE 3']", 0);
    }
    for (auto fontName : { "DejaVu Serif" })
    {
        OString prefix = aStylesBaseXpath + "/style:font-face[@style:name='" + fontName + "']";
        assertXPath(pXmlDoc, prefix + "['CASE 3']");
        assertXPath(pXmlDoc, prefix + "/svg:font-face-src['CASE 3']", 1);
    }

    // Check content - font-face-src should be present only for DejaVu Sans Mono and DejaVu Serif
    // Note that the used sets of fonts are different for styles.xml and content.xml
    pXmlDoc = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face['CASE 3']", 5);
    for (auto fontName : { "DejaVu Serif Condensed", "DejaVu Serif Condensed1" })
    {
        OString prefix = aContentBaseXpath + "/style:font-face[@style:name='" + fontName + "']";
        assertXPath(pXmlDoc, prefix + "['CASE 3']");
        assertXPath(pXmlDoc, prefix + "/svg:font-face-src['CASE 3']", 0);
    }
    for (auto fontName : { "DejaVu Sans Mono", "DejaVu Serif" })
    {
        OString prefix = aContentBaseXpath + "/style:font-face[@style:name='" + fontName + "']";
        assertXPath(pXmlDoc, prefix + "['CASE 3']");
        assertXPath(pXmlDoc, prefix + "/svg:font-face-src['CASE 3']", 1);
    }
#endif
}

CPPUNIT_TEST_FIXTURE(Test, testEmbeddedFontProps)
{
    createSwDoc("embedded-font-props.odt");
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xProps(
        xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW);
    xProps->setPropertyValue(u"EmbedFonts"_ustr, uno::Any(true));

    saveAndReload(TestFilter::ODT);
    CPPUNIT_ASSERT_EQUAL(1, getPages());
#if !defined(MACOSX)
    // Test that font style/weight of embedded fonts is exposed.
    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);
    // These failed, the attributes were missing.
    assertXPath(
        pXmlDoc,
        "//style:font-face[@style:name='DejaVu Serif']/svg:font-face-src/svg:font-face-uri[1]",
        "font-style", u"normal");
    assertXPath(
        pXmlDoc,
        "//style:font-face[@style:name='DejaVu Serif']/svg:font-face-src/svg:font-face-uri[1]",
        "font-weight", u"normal");
    assertXPath(
        pXmlDoc,
        "//style:font-face[@style:name='DejaVu Serif']/svg:font-face-src/svg:font-face-uri[2]",
        "font-style", u"normal");
    assertXPath(
        pXmlDoc,
        "//style:font-face[@style:name='DejaVu Serif']/svg:font-face-src/svg:font-face-uri[2]",
        "font-weight", u"bold");
    assertXPath(
        pXmlDoc,
        "//style:font-face[@style:name='DejaVu Serif']/svg:font-face-src/svg:font-face-uri[3]",
        "font-style", u"italic");
    assertXPath(
        pXmlDoc,
        "//style:font-face[@style:name='DejaVu Serif']/svg:font-face-src/svg:font-face-uri[3]",
        "font-weight", u"normal");
#if defined _WIN32
    assertXPath(
        pXmlDoc,
        "//style:font-face[@style:name='DejaVu Serif']/svg:font-face-src/svg:font-face-uri[4]",
        "font-style", u"italic");
    assertXPath(
        pXmlDoc,
        "//style:font-face[@style:name='DejaVu Serif']/svg:font-face-src/svg:font-face-uri[4]",
        "font-weight", u"bold");
#endif
#endif
}

CPPUNIT_TEST_FIXTURE(Test, testFontEmbeddingDOCX)
{
    createSwDoc("font_used_in_header_only.fodt");
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xProps(
        xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(uno::Any(true), xProps->getPropertyValue(u"EmbedFonts"_ustr));

    save(TestFilter::DOCX);

    xmlDocUniquePtr pXml = parseExport(u"word/fontTable.xml"_ustr);

    // Test that DejaVu Serif is embedded
    assertXPath(pXml, "/w:fonts/w:font[@w:name='DejaVu Serif']/w:embedRegular");
    assertXPath(pXml, "/w:fonts/w:font[@w:name='DejaVu Serif']/w:embedBold");
    assertXPath(pXml, "/w:fonts/w:font[@w:name='DejaVu Serif']/w:embedItalic");
    assertXPath(pXml, "/w:fonts/w:font[@w:name='DejaVu Serif']/w:embedBoldItalic");

    // Test that common fonts (here: Liberation Serif, Liberation Sans) are not embedded
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Serif']");
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Serif']/w:embedRegular", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Serif']/w:embedBold", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Serif']/w:embedItalic", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Serif']/w:embedBoldItalic", 0);

    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Sans']");
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Sans']/w:embedRegular", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Sans']/w:embedBold", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Sans']/w:embedItalic", 0);
    assertXPath(pXml, "/w:fonts/w:font[@w:name='Liberation Sans']/w:embedBoldItalic", 0);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
