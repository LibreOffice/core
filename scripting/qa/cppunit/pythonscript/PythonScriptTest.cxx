/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/io/TextOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/script/browse/XCopyableBrowseNode.hpp>
#include <com/sun/star/script/provider/ScriptFrameworkErrorType.hpp>
#include <com/sun/star/script/provider/ScriptURIHelper.hpp>
#include <com/sun/star/script/provider/ScriptMissingException.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>

namespace
{
class PythonScriptTest : public UnoApiTest
{
public:
    PythonScriptTest()
        : UnoApiTest(u"/scripting/qa/extras"_ustr)
    {
    }

    void setUp() override;
    void tearDown() override;

private:
    css::uno::Reference<css::ucb::XSimpleFileAccess3> m_xFileAccess;
    css::uno::Reference<css::script::provider::XScriptURIHelper> m_xUriHelper;
    css::uno::Reference<css::script::provider::XScriptProvider> m_xScriptProvider;

    void createDummyMacro();

    void testCopyable();
    void testMissingScript();

    CPPUNIT_TEST_SUITE(PythonScriptTest);
    CPPUNIT_TEST(testCopyable);
    CPPUNIT_TEST(testMissingScript);
    CPPUNIT_TEST_SUITE_END();
};

void PythonScriptTest::setUp()
{
    UnoApiTest::setUp();

    m_xFileAccess = css::ucb::SimpleFileAccess::create(m_xContext);

    // The ExpandContentProvider is needed for ScriptURIHelper to work
    css::uno::Reference<css::ucb::XUniversalContentBroker> xUcb
        = css::ucb::UniversalContentBroker::create(m_xContext);
    css::uno::Reference<css::ucb::XContentProvider> xExpandProvider(
        m_xSFactory->createInstance(u"com.sun.star.ucb.ExpandContentProvider"_ustr),
        css::uno::UNO_QUERY_THROW);
    xUcb->registerContentProvider(xExpandProvider, u"vnd.sun.star.expand"_ustr, true);

    m_xUriHelper = css::script::provider::ScriptURIHelper::create(getComponentContext(),
                                                                  u"Python"_ustr, u"user"_ustr);

    css::uno::Sequence<css::uno::Any> aArgs(1);
    aArgs.getArray()[0] <<= u"user"_ustr;

    m_xScriptProvider.set(
        m_xFactory->createInstanceWithArgumentsAndContext(
            "com.sun.star.script.provider.ScriptProviderForPython", aArgs, m_xContext),
        css::uno::UNO_QUERY_THROW);
}

void PythonScriptTest::tearDown()
{
    m_xScriptProvider.clear();

    // Delete the Python scripting directory so that each test can have a clean slate
    if (m_xFileAccess->isFolder(m_xUriHelper->getRootStorageURI()))
        m_xFileAccess->kill(m_xUriHelper->getRootStorageURI());

    m_xFileAccess.clear();
    m_xUriHelper.clear();

    UnoApiTest::tearDown();
}

void PythonScriptTest::createDummyMacro()
{
    // Create a dummy macro in the script directory
    css::uno::Reference<css::io::XOutputStream> xOutput = m_xFileAccess->openFileWrite(
        m_xUriHelper->getRootStorageURI() + "/MyLibrary/MyScript.py");
    css::uno::Reference<css::io::XTextOutputStream> xTextOutput
        = css::io::TextOutputStream::create(m_xContext);
    xTextOutput->setEncoding(u"UTF-8"_ustr);

    css::uno::Reference<css::io::XActiveDataSource> xActiveDataSource(xTextOutput,
                                                                      css::uno::UNO_QUERY_THROW);
    xActiveDataSource->setOutputStream(xOutput);

    xTextOutput->writeString(u"def MyMacro():\n    pass\n"_ustr);
    xTextOutput->closeOutput();
}

bool isCopyableNode(const css::uno::Reference<css::script::browse::XBrowseNode>& xNode)
{
    css::uno::Reference<css::script::browse::XCopyableBrowseNode> xCopyable(xNode,
                                                                            css::uno::UNO_QUERY);

    return xCopyable.is() && xCopyable->isCopyableNode();
}

void PythonScriptTest::testCopyable()
{
    createDummyMacro();

    css::uno::Reference<css::script::browse::XBrowseNode> xRootNode(m_xScriptProvider,
                                                                    css::uno::UNO_QUERY_THROW);

    // The root node shouldn’t be copyable
    CPPUNIT_ASSERT(!isCopyableNode(xRootNode));

    css::uno::Reference<css::script::browse::XBrowseNode> xLibrary = xRootNode->getChildNodes()[0];
    CPPUNIT_ASSERT_EQUAL(u"MyLibrary"_ustr, xLibrary->getName());

    // The library shouldn’t be copyable
    CPPUNIT_ASSERT(!isCopyableNode(xLibrary));

    css::uno::Reference<css::script::browse::XBrowseNode> xFile = xLibrary->getChildNodes()[0];
    CPPUNIT_ASSERT_EQUAL(u"MyScript"_ustr, xFile->getName());

    // File nodes are copyable
    CPPUNIT_ASSERT(isCopyableNode(xFile));

    css::uno::Reference<css::script::browse::XBrowseNode> xMacro = xFile->getChildNodes()[0];
    CPPUNIT_ASSERT_EQUAL(u"MyMacro"_ustr, xMacro->getName());

    // Macro nodes are not copyable
    CPPUNIT_ASSERT(!isCopyableNode(xMacro));

    css::uno::Reference<css::script::browse::XCopyableBrowseNode> xCopyable(
        xFile, css::uno::UNO_QUERY_THROW);

    // We can copy to the root level of the Python hierarchy
    CPPUNIT_ASSERT(xCopyable->nodeCanBeCopiedTo(xRootNode));
    // We can’t copy to the same directory that the file is already in
    CPPUNIT_ASSERT(!xCopyable->nodeCanBeCopiedTo(xLibrary));
    // We can’t copy to a file
    CPPUNIT_ASSERT(!xCopyable->nodeCanBeCopiedTo(xFile));
    // or a macro
    CPPUNIT_ASSERT(!xCopyable->nodeCanBeCopiedTo(xMacro));

    // Make a new library that we can copy to
    m_xFileAccess->createFolder(m_xUriHelper->getRootStorageURI() + "/MyOtherLibrary");
    css::uno::Reference<css::script::browse::XBrowseNode> xOtherLibrary;
    for (const auto& xChild : xRootNode->getChildNodes())
    {
        if (xChild->getName() == u"MyOtherLibrary"_ustr)
        {
            xOtherLibrary = xChild;
            break;
        }
    }
    CPPUNIT_ASSERT(xOtherLibrary.is());

    // We should be able to copy into the new library
    CPPUNIT_ASSERT(xCopyable->nodeCanBeCopiedTo(xOtherLibrary));
    css::uno::Reference<css::script::browse::XBrowseNode> xCopiedNode
        = xCopyable->copyNode(xOtherLibrary);

    CPPUNIT_ASSERT_EQUAL(u"MyScript"_ustr, xCopiedNode->getName());

    css::uno::Reference<css::script::browse::XBrowseNode> xCopiedMacro
        = xCopiedNode->getChildNodes()[0];
    CPPUNIT_ASSERT_EQUAL(u"MyMacro"_ustr, xCopiedMacro->getName());

    // Get the URI of the copied macro
    css::uno::Reference<css::beans::XPropertySet> xPropertySet(xCopiedMacro,
                                                               css::uno::UNO_QUERY_THROW);
    css::uno::Any xURI = xPropertySet->getPropertyValue(u"URI"_ustr);
    OUString sURI;
    CPPUNIT_ASSERT(xURI >>= sURI);

    // The URI should contain the new library name
    CPPUNIT_ASSERT(sURI.indexOf(":MyOtherLibrary|") != -1);

    // Check that we can load the script. This should also end up checking that the contents of the
    // file contains the macro.
    CPPUNIT_ASSERT(m_xScriptProvider->getScript(sURI).is());
}

void PythonScriptTest::testMissingScript()
{
    static constexpr OUString sDummyMacroUri = u"vnd.sun.star.script:MyLibrary|MyScript.py$MyMacro?"
                                               "language=Python&location=user"_ustr;

    // Try opening the macro before it exists. This should throw the missing script exception
    try
    {
        m_xScriptProvider->getScript(sDummyMacroUri);
    }
    catch (const css::script::provider::ScriptMissingException& e)
    {
        CPPUNIT_ASSERT_EQUAL(sDummyMacroUri, e.scriptName);
        CPPUNIT_ASSERT_EQUAL(u"Python"_ustr, e.language);
        CPPUNIT_ASSERT_EQUAL(css::script::provider::ScriptFrameworkErrorType::NO_SUCH_SCRIPT,
                             e.errorType);
        CPPUNIT_ASSERT_EQUAL(u"user"_ustr, e.location);
        CPPUNIT_ASSERT_EQUAL(u"MyLibrary"_ustr, e.library);
        CPPUNIT_ASSERT_EQUAL(u"MyScript"_ustr, e.moduleName);
        CPPUNIT_ASSERT_EQUAL(u"MyMacro"_ustr, e.name);
        goto exception_caught;
    }

    CPPUNIT_FAIL("ScriptMissingException not thrown");

exception_caught:

    createDummyMacro();

    // After creating the macro no exception should be thrown
    CPPUNIT_ASSERT(m_xScriptProvider->getScript(sDummyMacroUri).is());

    // Try accessing a non-existant function within the script
    OUString sBadFuncUri = sDummyMacroUri.replaceFirst("MyMacro", "MyNonExistantMacro");

    try
    {
        m_xScriptProvider->getScript(sBadFuncUri);
    }
    catch (const css::script::provider::ScriptMissingException& e)
    {
        CPPUNIT_ASSERT_EQUAL(sBadFuncUri, e.scriptName);
        CPPUNIT_ASSERT_EQUAL(u"Python"_ustr, e.language);
        CPPUNIT_ASSERT_EQUAL(css::script::provider::ScriptFrameworkErrorType::NO_SUCH_SCRIPT,
                             e.errorType);
        CPPUNIT_ASSERT_EQUAL(u"user"_ustr, e.location);
        CPPUNIT_ASSERT_EQUAL(u"MyLibrary"_ustr, e.library);
        CPPUNIT_ASSERT_EQUAL(u"MyScript"_ustr, e.moduleName);
        CPPUNIT_ASSERT_EQUAL(u"MyNonExistantMacro"_ustr, e.name);
        goto exception_caught2;
    }

    CPPUNIT_FAIL("ScriptMissingException not thrown");

exception_caught2:;
}

CPPUNIT_TEST_SUITE_REGISTRATION(PythonScriptTest);

} // namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
