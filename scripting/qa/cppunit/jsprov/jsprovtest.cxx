/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/script/provider/ScriptURIHelper.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <test/unoapi_test.hxx>

namespace
{
class JsProvTest : public UnoApiTest
{
public:
    JsProvTest()
        : UnoApiTest(u"/scripting/qa/extras"_ustr)
    {
    }

    void setUp() override;
    void tearDown() override;

private:
    css::uno::Reference<css::ucb::XSimpleFileAccess3> m_xFileAccess;
    css::uno::Reference<css::script::provider::XScriptURIHelper> m_xUriHelper;
    css::uno::Reference<css::script::provider::XScriptProvider> m_xScriptProvider;

    css::uno::Reference<css::script::provider::XScriptProvider> createScriptProvider();

    void testEditableCreatable();
    void testCreate();
    void testRun();
    void testUnrelatedFile();
    void testFolder();

    CPPUNIT_TEST_SUITE(JsProvTest);
    CPPUNIT_TEST(testEditableCreatable);
    CPPUNIT_TEST(testCreate);
    CPPUNIT_TEST(testRun);
    CPPUNIT_TEST(testUnrelatedFile);
    CPPUNIT_TEST(testFolder);
    CPPUNIT_TEST_SUITE_END();
};

void JsProvTest::setUp()
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
                                                                  u"JavaScript"_ustr, u"user"_ustr);

    m_xScriptProvider = createScriptProvider();
}

void JsProvTest::tearDown()
{
    m_xScriptProvider.clear();

    // Delete the JavaScript scripting directory so that each test can have a clean slate
    if (m_xFileAccess->isFolder(m_xUriHelper->getRootStorageURI()))
        m_xFileAccess->kill(m_xUriHelper->getRootStorageURI());

    m_xFileAccess.clear();
    m_xUriHelper.clear();

    UnoApiTest::tearDown();
}

css::uno::Reference<css::script::provider::XScriptProvider> JsProvTest::createScriptProvider()
{
    // Create the script provider by searching for a factory with the right implementation in case
    // there are multiple JavaScript providers
    css::uno::Reference<css::container::XContentEnumerationAccess> xEnumAccess(
        m_xFactory, css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::container::XEnumeration> xEnum = xEnumAccess->createContentEnumeration(
        "com.sun.star.script.provider.ScriptProviderForJavaScript");

    while (xEnum->hasMoreElements())
    {
        css::uno::Reference<css::lang::XSingleComponentFactory> xFactory(xEnum->nextElement(),
                                                                         css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::lang::XServiceInfo> xServiceInfo(xFactory,
                                                                  css::uno::UNO_QUERY_THROW);

        if (xServiceInfo->getImplementationName()
            != "com.sun.star.singleprov.ScriptProviderForJavaScript")
            continue;

        css::uno::Sequence<css::uno::Any> aArgs(1);
        aArgs.getArray()[0] <<= u"user"_ustr;

        css::uno::Reference<css::script::provider::XScriptProvider> xProvider(
            xFactory->createInstanceWithArgumentsAndContext(aArgs, m_xContext),
            css::uno::UNO_QUERY_THROW);

        return xProvider;
    }

    CPPUNIT_FAIL("Couldn’t find com.sun.star.singleprov.ScriptProviderForJavaScript "
                 "implementation");
}

bool getBooleanProperty(const css::uno::Reference<css::uno::XInterface>& xInterface,
                        const OUString& sPropertyName)
{
    css::uno::Reference<css::beans::XPropertySet> xPropertySet(xInterface,
                                                               css::uno::UNO_QUERY_THROW);
    css::uno::Any xAnyResult = xPropertySet->getPropertyValue(sPropertyName);

    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_BOOLEAN, xAnyResult.getValueTypeClass());

    bool bBoolResult = false;
    bool bConversionResult = xAnyResult >>= bBoolResult;

    CPPUNIT_ASSERT(bConversionResult);

    return bBoolResult;
}

void JsProvTest::testEditableCreatable()
{
    // Create a dummy macro in the script directory
    m_xFileAccess->openFileWrite(m_xUriHelper->getRootStorageURI() + "/MyScript.js");

    // The root node should be creatable but not editable
    CPPUNIT_ASSERT(getBooleanProperty(m_xScriptProvider, "Creatable"));
    CPPUNIT_ASSERT(!getBooleanProperty(m_xScriptProvider, "Editable"));
    css::uno::Reference<css::script::XInvocation> xInvocation(m_xScriptProvider,
                                                              css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xInvocation->hasMethod("Creatable"));
    CPPUNIT_ASSERT(!xInvocation->hasMethod("Editable"));

    css::uno::Reference<css::script::browse::XBrowseNode> xBrowseNode(m_xScriptProvider,
                                                                      css::uno::UNO_QUERY_THROW);

    // The root node should be a container
    CPPUNIT_ASSERT_EQUAL(css::script::browse::BrowseNodeTypes::CONTAINER, xBrowseNode->getType());

    CPPUNIT_ASSERT_EQUAL(u"JavaScript"_ustr, xBrowseNode->getName());

    // The root node should have exactly one child which is the script we created above
    CPPUNIT_ASSERT(xBrowseNode->hasChildNodes());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xBrowseNode->getChildNodes().getLength());

    xBrowseNode = xBrowseNode->getChildNodes()[0];

    // The next level down should be a container node to represent the module
    CPPUNIT_ASSERT_EQUAL(css::script::browse::BrowseNodeTypes::CONTAINER, xBrowseNode->getType());

    CPPUNIT_ASSERT_EQUAL(u"MyScript"_ustr, xBrowseNode->getName());

    // The module should be editable but not creatable
    CPPUNIT_ASSERT(!getBooleanProperty(xBrowseNode, "Creatable"));
    CPPUNIT_ASSERT(getBooleanProperty(xBrowseNode, "Editable"));
    xInvocation.set(xBrowseNode, css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(!xInvocation->hasMethod("Creatable"));
    CPPUNIT_ASSERT(xInvocation->hasMethod("Editable"));

    // The module should have exactly one child to represent the macro
    CPPUNIT_ASSERT(xBrowseNode->hasChildNodes());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xBrowseNode->getChildNodes().getLength());

    xBrowseNode = xBrowseNode->getChildNodes()[0];

    // Finally we should be on a node representing the actual macro
    CPPUNIT_ASSERT_EQUAL(css::script::browse::BrowseNodeTypes::SCRIPT, xBrowseNode->getType());

    CPPUNIT_ASSERT_EQUAL(u"MyScript"_ustr, xBrowseNode->getName());

    // The macro should be editable but not creatable
    CPPUNIT_ASSERT(!getBooleanProperty(xBrowseNode, "Creatable"));
    CPPUNIT_ASSERT(getBooleanProperty(xBrowseNode, "Editable"));
    xInvocation.set(xBrowseNode, css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(!xInvocation->hasMethod("Creatable"));
    CPPUNIT_ASSERT(xInvocation->hasMethod("Editable"));

    // The macro should have no children
    CPPUNIT_ASSERT(!xBrowseNode->hasChildNodes());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xBrowseNode->getChildNodes().getLength());

    // Test the macro’s URL
    css::uno::Reference<css::beans::XPropertySet> xPropertySet(xBrowseNode,
                                                               css::uno::UNO_QUERY_THROW);
    OUString sMacroUri;
    bool bConvertResult = xPropertySet->getPropertyValue("URI") >>= sMacroUri;
    CPPUNIT_ASSERT(bConvertResult);
    CPPUNIT_ASSERT_EQUAL(u"vnd.sun.star.script:MyScript.js?language=JavaScript&location=user"_ustr,
                         sMacroUri);
}

void JsProvTest::testCreate()
{
    css::uno::Reference<css::script::browse::XBrowseNode> xBrowseNode(m_xScriptProvider,
                                                                      css::uno::UNO_QUERY_THROW);
    // The root node should start by having no children
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xBrowseNode->getChildNodes().getLength());

    // The root node should be creatable
    CPPUNIT_ASSERT(getBooleanProperty(xBrowseNode, "Creatable"));

    // and have the Creatable method
    css::uno::Reference<css::script::XInvocation> xInvocation(m_xScriptProvider,
                                                              css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xInvocation->hasMethod("Creatable"));

    // Invoke the method
    css::uno::Sequence<css::uno::Any> aInArgs(1);
    aInArgs.getArray()[0] <<= u"My Script"_ustr;
    css::uno::Sequence<sal_Int16> aOutParamIndex;
    css::uno::Sequence<css::uno::Any> aOutParam;
    xInvocation->invoke("Creatable", aInArgs, aOutParamIndex, aOutParam);

    // That should have created the file
    CPPUNIT_ASSERT(m_xFileAccess->exists(m_xUriHelper->getRootStorageURI() + "/My Script.js"));

    // The file should now appear as a child of the root node
    CPPUNIT_ASSERT(xBrowseNode->hasChildNodes());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xBrowseNode->getChildNodes().getLength());

    CPPUNIT_ASSERT_EQUAL(u"My Script"_ustr, xBrowseNode->getChildNodes()[0]->getName());
}

void JsProvTest::testRun()
{
    // Create a dummy macro in the script directory
    css::uno::Reference<css::io::XOutputStream> xOutput
        = m_xFileAccess->openFileWrite(m_xUriHelper->getRootStorageURI() + "/MyScript.js");

    // Make the script insert some text in all of the writer documents
    static constexpr OString sSource
        = "let context = uno.componentContext;\n"
          "let desktop = uno.idl.com.sun.star.frame.theDesktop(context);\n"
          "let enumeration = desktop.getComponents().createEnumeration();\n"
          "while (enumeration.hasMoreElements())\n"
          "{\n"
          "  let model = enumeration.nextElement();\n"
          "  model.getText().getEnd().setString(\"jsprovtest\");\n"
          "}\n"_ostr;

    css::uno::Sequence<sal_Int8> aSource(reinterpret_cast<const sal_Int8*>(sSource.getStr()),
                                         sSource.getLength());
    xOutput->writeBytes(aSource);

    xOutput->closeOutput();

    // Get the macro as a property set
    css::uno::Reference<css::script::browse::XBrowseNode> xBrowseNode(m_xScriptProvider,
                                                                      css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::beans::XPropertySet> xPropertySet(
        xBrowseNode->getChildNodes()[0]->getChildNodes()[0], css::uno::UNO_QUERY_THROW);

    // Get the XScript
    OUString sScriptUri;
    bool bConvertResult = xPropertySet->getPropertyValue("URI") >>= sScriptUri;
    CPPUNIT_ASSERT(bConvertResult);
    css::uno::Reference<css::script::provider::XScript> xScript
        = m_xScriptProvider->getScript(sScriptUri);
    CPPUNIT_ASSERT(xScript.is());

    // Create a document to run the script on
    loadFromURL(u"private:factory/swriter"_ustr);

    // Execute it
    css::uno::Sequence<css::uno::Any> aScriptParams;
    css::uno::Sequence<sal_Int16> aOutParamIndex;
    css::uno::Sequence<css::uno::Any> aOutParam;
    xScript->invoke(aScriptParams, aOutParamIndex, aOutParam);

    // Check that the text was inserted into our test document
    css::uno::Reference<css::text::XTextDocument> xTextDocument(mxComponent,
                                                                css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"jsprovtest"_ustr, xTextDocument->getText()->getString());
}

void JsProvTest::testUnrelatedFile()
{
    // Create a file with a name that shouldn’t be picked up by the JavaScript provider
    OUString sFileUri = m_xUriHelper->getRootStorageURI() + "/MyScript.py";
    m_xFileAccess->openFileWrite(sFileUri);
    CPPUNIT_ASSERT(m_xFileAccess->exists(sFileUri));

    css::uno::Reference<css::script::browse::XBrowseNode> xBrowseNode(m_xScriptProvider,
                                                                      css::uno::UNO_QUERY_THROW);

    // The root node should have no children
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xBrowseNode->getChildNodes().getLength());
}

void JsProvTest::testFolder()
{
    // Create a file that is nested in subfolders
    m_xFileAccess->openFileWrite(m_xUriHelper->getRootStorageURI()
                                 + "/very/nested/file/MyScript.js");

    css::uno::Reference<css::script::browse::XBrowseNode> xBrowseNode(m_xScriptProvider,
                                                                      css::uno::UNO_QUERY_THROW);

    std::array<OUString, 3> aParts = { u"very"_ustr, u"nested"_ustr, u"file"_ustr };

    for (const auto& sPart : aParts)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xBrowseNode->getChildNodes().getLength());

        xBrowseNode = xBrowseNode->getChildNodes()[0];

        CPPUNIT_ASSERT_EQUAL(css::script::browse::BrowseNodeTypes::CONTAINER,
                             xBrowseNode->getType());
        CPPUNIT_ASSERT_EQUAL(sPart, xBrowseNode->getName());

        // Folders should be creatable but not editable
        CPPUNIT_ASSERT(getBooleanProperty(xBrowseNode, "Creatable"));
        CPPUNIT_ASSERT(!getBooleanProperty(xBrowseNode, "Editable"));
        css::uno::Reference<css::script::XInvocation> xInvocation(xBrowseNode,
                                                                  css::uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xInvocation->hasMethod("Creatable"));
        CPPUNIT_ASSERT(!xInvocation->hasMethod("Editable"));
    }

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xBrowseNode->getChildNodes().getLength());
    xBrowseNode = xBrowseNode->getChildNodes()[0];
    CPPUNIT_ASSERT_EQUAL(u"MyScript"_ustr, xBrowseNode->getName());
}

CPPUNIT_TEST_SUITE_REGISTRATION(JsProvTest);

} // namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
