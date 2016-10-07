/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config_test.h"

#ifdef MACOSX
#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
#include <premac.h>
#include <AppKit/AppKit.h>
#include <postmac.h>
#endif

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <unotools/fltrcfg.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/propertysequence.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text")
    {
    }

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return (OString(filename).endsWith(".docx"));
    }

    /// Copy&paste helper.
    bool paste(const OUString& rFilename, const uno::Reference<text::XTextRange>& xTextRange)
    {
        uno::Reference<document::XFilter> xFilter(m_xSFactory->createInstance("com.sun.star.comp.Writer.WriterFilter"), uno::UNO_QUERY_THROW);
        uno::Reference<document::XImporter> xImporter(xFilter, uno::UNO_QUERY_THROW);
        xImporter->setTargetDocument(mxComponent);
        SvStream* pStream = utl::UcbStreamHelper::CreateStream(m_directories.getURLFromSrc("/sw/qa/extras/ooxmlexport/data/") + rFilename, StreamMode::READ);
        uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStream));
        uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence(
        {
            {"InputStream", uno::makeAny(xStream)},
            {"InputMode", uno::makeAny(true)},
            {"TextInsertModeRange", uno::makeAny(xTextRange)},
        }));
        return xFilter->filter(aDescriptor);
    }
};

class FailTest : public Test
{
public:
    // UGLY: hacky manual override of MacrosTest::loadFromDesktop
    void executeImportTest(const char* filename)
    {
        header();
        preTest(filename);
        {
            if (mxComponent.is())
                mxComponent->dispose();
            std::cout << filename << ",";
            mnStartTime = osl_getGlobalTimer();
            {
                OUString aURL(m_directories.getURLFromSrc(mpTestDocumentPath) + OUString::createFromAscii(filename));
                CPPUNIT_ASSERT_MESSAGE("no desktop", mxDesktop.is());
                uno::Reference<frame::XComponentLoader> xLoader(mxDesktop, uno::UNO_QUERY);
                CPPUNIT_ASSERT_MESSAGE("no loader", xLoader.is());
                uno::Sequence<beans::PropertyValue> args(1);
                args[0].Name = "DocumentService";
                args[0].Handle = -1;
                args[0].Value <<= OUString("com.sun.star.text.TextDocument");
                args[0].State = beans::PropertyState_DIRECT_VALUE;

                uno::Reference<lang::XComponent> xComponent = xLoader->loadComponentFromURL(aURL, "_default", 0, args);
                OUString sMessage = "loading succeeded: " + aURL;
                CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sMessage, RTL_TEXTENCODING_UTF8).getStr(), !xComponent.is());
            }
        }
        verify();
        finish();
    }
};



CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
