/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <helper/qahelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Implementation of Macros test */

class ScScriptForgeTest : public ScModelTestBase
{
public:
    ScScriptForgeTest();
};

CPPUNIT_TEST_FIXTURE(ScScriptForgeTest, testSetValue)
{
    createScDoc();

    // insert initial library
    css::uno::Reference<css::document::XEmbeddedScripts> xDocScr(mxComponent, UNO_QUERY_THROW);
    auto xLibs = xDocScr->getBasicLibraries();
    auto xLibrary = xLibs->createLibrary(u"TestLibrary"_ustr);
    xLibrary->insertByName(
        u"TestModule"_ustr,
        uno::Any(u"Function Main\n"
                 " GlobalScope.BasicLibraries.LoadLibrary(\"ScriptForge\")\n"
                 " Dim oDoc : oDoc = CreateScriptService(\"Calc\", ThisComponent)\n"
                 " oDoc.setValue(\"A1\", 10)\n"
                 "End Function\n"_ustr));

    executeMacro(
        u"vnd.sun.Star.script:TestLibrary.TestModule.Main?language=Basic&location=document"_ustr);

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(u"10"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScScriptForgeTest, testShowProgressBar)
{
    createScDoc();

    // insert initial library
    css::uno::Reference<css::document::XEmbeddedScripts> xDocScr(mxComponent, UNO_QUERY_THROW);
    auto xLibs = xDocScr->getBasicLibraries();
    auto xLibrary = xLibs->createLibrary(u"TestLibrary"_ustr);
    xLibrary->insertByName(u"TestModule"_ustr,
                           uno::Any(u"Function Test as String\n"
                                    " GlobalScope.BasicLibraries.LoadLibrary(\"ScriptForge\")\n"
                                    " ui = CreateScriptService(\"UI\")\n"
                                    " ui.ShowProgressBar\n"
                                    " Test = \"OK\"\n"
                                    "End Function\n"_ustr));

    Any aRet = executeMacro(
        u"vnd.sun.Star.script:TestLibrary.TestModule.Test?language=Basic&location=document"_ustr);
    OUString sResult;
    aRet >>= sResult;
    CPPUNIT_ASSERT_EQUAL(u"OK"_ustr, sResult);
}

ScScriptForgeTest::ScScriptForgeTest()
    : ScModelTestBase(u"/sc/qa/extras/testdocuments"_ustr)
{
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
