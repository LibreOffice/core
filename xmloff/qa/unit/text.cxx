/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#include <comphelper/propertysequence.hxx>
#include <unotools/tempfile.hxx>

using namespace ::com::sun::star;

constexpr OUStringLiteral DATA_DIRECTORY = u"/xmloff/qa/unit/data/";

/// Covers xmloff/source/text/ fixes.
class XmloffStyleTest : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
};

void XmloffStyleTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void XmloffStyleTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testMailMergeInEditeng)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "mail-merge-editeng.odt";
    getComponent() = loadFromDesktop(aURL);
    // Without the accompanying fix in place, this test would have failed, as unexpected
    // <text:database-display> in editeng text aborted the whole import process.
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testCommentResolved)
{
    getComponent() = loadFromDesktop("private:factory/swriter");
    uno::Sequence<beans::PropertyValue> aCommentProps = comphelper::InitPropertySequence({
        { "Text", uno::makeAny(OUString("comment")) },
    });
    dispatchCommand(getComponent(), ".uno:InsertAnnotation", aCommentProps);
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortionEnum = xPara->createEnumeration();
    uno::Reference<beans::XPropertySet> xPortion(xPortionEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(xPortion->getPropertyValue("TextField"),
                                               uno::UNO_QUERY);
    xField->setPropertyValue("Resolved", uno::makeAny(true));

    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProps = comphelper::InitPropertySequence({
        { "FilterName", uno::makeAny(OUString("writer8")) },
    });
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aStoreProps);
    getComponent()->dispose();

    getComponent() = loadFromDesktop(aTempFile.GetURL());
    xTextDocument.set(getComponent(), uno::UNO_QUERY);
    xParaEnumAccess.set(xTextDocument->getText(), uno::UNO_QUERY);
    xParaEnum = xParaEnumAccess->createEnumeration();
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPortionEnum = xPara->createEnumeration();
    xPortion.set(xPortionEnum->nextElement(), uno::UNO_QUERY);
    xField.set(xPortion->getPropertyValue("TextField"), uno::UNO_QUERY);
    bool bResolved = false;
    xField->getPropertyValue("Resolved") >>= bResolved;
    // Without the accompanying fix in place, this test would have failed, as the resolved state was
    // not saved for non-range comments.
    CPPUNIT_ASSERT(bResolved);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
