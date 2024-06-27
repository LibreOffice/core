/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <comphelper/propertyvalue.hxx>
#include <unotools/streamwrap.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

using namespace com::sun::star;

namespace
{
/// Tests for sw/source/writerfilter/dmapper/NumberingManager.cxx.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"/sw/qa/writerfilter/dmapper/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testPasteBulletListStyleName)
{
    // Given a document with a WWNum1 list style:
    mxComponent
        = loadFromDesktop(u"private:factory/swriter"_ustr, u"com.sun.star.text.TextDocument"_ustr);
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameContainer> xStyles;
    xStyleFamilies->getByName(u"NumberingStyles"_ustr) >>= xStyles;
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<uno::XInterface> xStyle
        = xFactory->createInstance("com.sun.star.style.NumberingStyle");
    xStyles->insertByName(u"WWNum1"_ustr, uno::Any(xStyle));

    // When pasting bullets to that document:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText = xTextDocument->getText();
    uno::Reference<text::XTextRange> xBodyEnd = xText->getEnd();
    uno::Reference<document::XFilter> xFilter(
        m_xSFactory->createInstance(u"com.sun.star.comp.Writer.RtfFilter"_ustr), uno::UNO_QUERY);
    uno::Reference<document::XImporter> xImporter(xFilter, uno::UNO_QUERY);
    xImporter->setTargetDocument(mxComponent);
    std::unique_ptr<SvStream> pStream(
        new SvFileStream(createFileURL(u"clipboard-bullets.rtf"), StreamMode::READ));
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(std::move(pStream)));
    uno::Sequence aDescriptor{ comphelper::makePropertyValue(u"InputStream"_ustr, xStream),
                               comphelper::makePropertyValue(u"InsertMode"_ustr, true),
                               comphelper::makePropertyValue(u"TextInsertModeRange"_ustr,
                                                             xBodyEnd) };
    CPPUNIT_ASSERT(xFilter->filter(aDescriptor));

    // Then make sure we don't create new list styles, but reuse the existing ones:
    // Without the accompanying fix in place, this test would have failed, new character styles were
    // created again and again on each paste.
    CPPUNIT_ASSERT(!xStyles->hasByName(u"WWNum1a"_ustr));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
