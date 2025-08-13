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
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/embedded_fonts/data/"_ustr, u"writer8"_ustr)
    {
    }
};

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
                    return true;
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
    {
        // 1. Load and do not approve the restricted font
        FontMappingUseListener fontMappingData;
        rtl::Reference xInteraction(new FontInteractionHandler(false));
        loadWithParams(createFileURL(u"embed-restricted-style+autostyle.odt"),
                       { comphelper::makePropertyValue(
                           u"InteractionHandler"_ustr,
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
        save(u"writer8"_ustr);
        xmlDocUniquePtr pXml = parseExport(u"content.xml"_ustr);
        assertXPath(pXml, "//style:font-face[@style:name='Naftalene']");
        assertXPath(pXml, "//style:font-face[@style:name='Naftalene']/svg:font-face-src", 0);

        auto xZipFile = packages::zip::ZipFileAccess::createWithURL(
            comphelper::getProcessComponentContext(), maTempFile.GetURL());
        CPPUNIT_ASSERT(xZipFile);
        for (const OUString& name : xZipFile->getElementNames())
            CPPUNIT_ASSERT(name.indexOf("Naftalene") < 0);
    }

    {
        // 2. Load and approve the restricted font
        FontMappingUseListener fontMappingData;
        rtl::Reference xInteraction(new FontInteractionHandler(true));
        loadWithParams(createFileURL(u"embed-restricted-style+autostyle.odt"),
                       { comphelper::makePropertyValue(
                           u"InteractionHandler"_ustr,
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
        save(u"writer8"_ustr);
        xmlDocUniquePtr pXml = parseExport(u"content.xml"_ustr);
        assertXPath(pXml, "//style:font-face[@style:name='Naftalene']");
        assertXPath(pXml, "//style:font-face[@style:name='Naftalene']/svg:font-face-src", 0);

        auto xZipFile = packages::zip::ZipFileAccess::createWithURL(
            comphelper::getProcessComponentContext(), maTempFile.GetURL());
        CPPUNIT_ASSERT(xZipFile);
        for (const OUString& name : xZipFile->getElementNames())
            CPPUNIT_ASSERT(name.indexOf("Naftalene") < 0);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testOpenDOCXWithRestrictedEmbeddedFont)
{
    // The DOCX has two embedded fonts, one restricted (Naftalene), one unrestricted (Unsteady
    // Oversteer). Test without interaction handler, and with handler (without and with approval).
    {
        // 1. Load without interaction handler. It must not load the restricted font;
        // unrestricted one must load.
        FontMappingUseListener fontMappingData;
        loadWithParams(createFileURL(u"embed-restricted+unrestricted.docx"), {});

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
        save(u"Office Open XML Text"_ustr);
        xmlDocUniquePtr pXml = parseExport(u"word/fontTable.xml"_ustr);
        assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']");
        assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedRegular", 0);
        assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedBold", 0);
        assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedItalic", 0);
        assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedBoldItalic", 0);
    }

    {
        // 2. Load and do not approve the restricted font. It must not load the restricted font;
        // unrestricted one must load.
        FontMappingUseListener fontMappingData;
        rtl::Reference xInteraction(new FontInteractionHandler(false));
        loadWithParams(createFileURL(u"embed-restricted+unrestricted.docx"),
                       { comphelper::makePropertyValue(
                           u"InteractionHandler"_ustr,
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
        save(u"Office Open XML Text"_ustr);
        xmlDocUniquePtr pXml = parseExport(u"word/fontTable.xml"_ustr);
        assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']");
        assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedRegular", 0);
        assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedBold", 0);
        assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedItalic", 0);
        assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedBoldItalic", 0);
    }

    {
        // 3. Load and approve the restricted font. It must load both fonts, and open in read-only
        // mode.
        FontMappingUseListener fontMappingData;
        rtl::Reference xInteraction(new FontInteractionHandler(true));
        loadWithParams(createFileURL(u"embed-restricted+unrestricted.docx"),
                       { comphelper::makePropertyValue(
                           u"InteractionHandler"_ustr,
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
        save(u"Office Open XML Text"_ustr);
        xmlDocUniquePtr pXml = parseExport(u"word/fontTable.xml"_ustr);
        assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']");
        assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedRegular", 0);
        assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedBold", 0);
        assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedItalic", 0);
        assertXPath(pXml, "/w:fonts/w:font[@w:name='Naftalene']/w:embedBoldItalic", 0);
    }
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

CPPUNIT_TEST_FIXTURE(Test, testFontEmbeddingDOCX)
{
    createSwDoc("font_used_in_header_only.fodt");
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xProps(
        xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(uno::Any(true), xProps->getPropertyValue(u"EmbedFonts"_ustr));

    save(u"Office Open XML Text"_ustr);

    xmlDocUniquePtr pXml = parseExport(u"word/fontTable.xml"_ustr);

    // Test that DejaVu Sans is embedded
    assertXPath(pXml, "/w:fonts/w:font[@w:name='DejaVu Sans']/w:embedRegular");
    assertXPath(pXml, "/w:fonts/w:font[@w:name='DejaVu Sans']/w:embedBold");
    assertXPath(pXml, "/w:fonts/w:font[@w:name='DejaVu Sans']/w:embedItalic");
// It is strange that DejaVu is different on Linux: see e.g. tdf166627 in odfexport2.cxx
#if defined(_WIN32) || defined(MACOSX)
    assertXPath(pXml, "/w:fonts/w:font[@w:name='DejaVu Sans']/w:embedBoldItalic");
#endif

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
