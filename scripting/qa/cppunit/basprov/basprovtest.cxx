/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/script/provider/ScriptFrameworkErrorType.hpp>
#include <com/sun/star/script/provider/ScriptMissingException.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <test/unoapi_test.hxx>

namespace
{
class BasProvTest : public UnoApiTest
{
public:
    BasProvTest()
        : UnoApiTest(u"/scripting/qa/extras"_ustr)
    {
    }

private:
    void testScriptMissing();

    CPPUNIT_TEST_SUITE(BasProvTest);
    CPPUNIT_TEST(testScriptMissing);
    CPPUNIT_TEST_SUITE_END();
};

void BasProvTest::testScriptMissing()
{
    css::uno::Reference<css::script::provider::XScriptProvider> xProvider(
        m_xFactory->createInstanceWithContext("com.sun.star.script.provider.ScriptProviderForBasic",
                                              m_xContext),
        css::uno::UNO_QUERY_THROW);

    static constexpr OUString sScriptUrl = u"vnd.sun.star.script:Standard.Module1.macronotfound"
                                           "?language=Basic&location=document"_ustr;

    try
    {
        xProvider->getScript(sScriptUrl);
    }
    catch (const css::script::provider::ScriptMissingException& rException)
    {
        CPPUNIT_ASSERT_EQUAL(sScriptUrl, rException.scriptName);
        CPPUNIT_ASSERT_EQUAL(u"Basic"_ustr, rException.language);
        CPPUNIT_ASSERT_EQUAL(css::script::provider::ScriptFrameworkErrorType::NO_SUCH_SCRIPT,
                             rException.errorType);
        CPPUNIT_ASSERT_EQUAL(u"document"_ustr, rException.location);
        CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr, rException.library);
        CPPUNIT_ASSERT_EQUAL(u"Module1"_ustr, rException.moduleName);
        CPPUNIT_ASSERT_EQUAL(u"macronotfound"_ustr, rException.name);
        goto exception_caught;
    }

    CPPUNIT_FAIL("ScriptMissingException was not thrown");

exception_caught:;
}

CPPUNIT_TEST_SUITE_REGISTRATION(BasProvTest);

} // namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
