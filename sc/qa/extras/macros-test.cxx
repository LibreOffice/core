/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2010 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
 *  (initial developer)
 * Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <sal/config.h>
#include <test/unoapi_test.hxx>
#include <unotest/macros_test.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/intitem.hxx>
#include <com/sun/star/frame/Desktop.hpp>

#include <basic/sbxdef.hxx>

#include "docsh.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "document.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Implementation of Macros test */

class ScMacrosTest : public UnoApiTest
{
public:
    ScMacrosTest();

    void testStarBasic();
    void testVba();
    CPPUNIT_TEST_SUITE(ScMacrosTest);
#if !defined(MACOSX) && !defined(WNT)
    //enable this test if you want to play with star basic macros in unit tests
    //works but does nothing useful yet
    CPPUNIT_TEST(testStarBasic);
    CPPUNIT_TEST(testVba);
#endif

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XInterface> m_xCalcComponent;
};

void ScMacrosTest::testStarBasic()
{
    const OUString aFileNameBase("StarBasic.ods");
    rtl::OUString aFileName;
    createFileURL(aFileNameBase, aFileName);
    std::cout << "StarBasic test" << std::endl;
    uno::Reference< com::sun::star::lang::XComponent > xComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    CPPUNIT_ASSERT_MESSAGE("Failed to load StarBasic.ods", xComponent.is());

    rtl::OUString aURL(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:Standard.Module1.Macro1?language=Basic&location=document"));
    String sUrl = aURL;
    Any aRet;
    Sequence< sal_Int16 > aOutParamIndex;
    Sequence< Any > aOutParam;
    Sequence< uno::Any > aParams;

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* xDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument* pDoc = xDocSh->GetDocument();

    pFoundShell->CallXScript(xComponent, sUrl, aParams, aRet, aOutParamIndex,aOutParam);
    double aValue;
    pDoc->GetValue(0,0,0,aValue);
    std::cout << "returned value = " << aValue << std::endl;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("script did not change the value of Sheet1.A1",2.0, aValue, 0.00001);
    xDocSh->DoClose();
}

void ScMacrosTest::testVba()
{
    TestMacroInfo testInfo[] = {
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TestAddress.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document"))
        },
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vba.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:VBAProject.Modul1.Modul1?language=Basic&location=document")),
        },
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MiscRangeTests.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document"))
        },
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bytearraystring.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:VBAProject.testMacro.test?language=Basic&location=document"))
        },
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AutoFilter.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document"))
        },
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CalcFont.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document"))
        },
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TestIntersection.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document"))
        },
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TestUnion.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document"))
        },
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("range-4.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document"))
        },
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Ranges-3.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document"))
        },
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TestCalc_Rangetest.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document"))
        },
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TestCalc_Rangetest2.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document"))
        },
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Ranges-2.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document"))
        },
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("pagesetup.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document"))
        },
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Window.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document"))
        },
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("window2.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document"))
        },
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PageBreaks.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document"))
        },
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Shapes.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document"))
        },
#if VBA_TEST_WORKING
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Ranges.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document"))
        },
#endif
    };

    for ( sal_uInt32  i=0; i<SAL_N_ELEMENTS( testInfo ); ++i )
    {
        rtl::OUString aFileName;
        createFileURL(testInfo[i].sFileBaseName + "xls", aFileName);
        uno::Reference< com::sun::star::lang::XComponent > xComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");
        rtl::OUString sMsg( "Failed to load " + aFileName );
        CPPUNIT_ASSERT_MESSAGE( rtl::OUStringToOString( sMsg, RTL_TEXTENCODING_UTF8 ).getStr(), xComponent.is() );

        String sUrl = testInfo[i].sMacroUrl;
        Any aRet;
        Sequence< sal_Int16 > aOutParamIndex;
        Sequence< Any > aOutParam;
        Sequence< uno::Any > aParams;

        SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);

        CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
        std::cout << "about to invoke vba test in " << rtl::OUStringToOString( aFileName, RTL_TEXTENCODING_UTF8 ).getStr() << std::endl;

        pFoundShell->CallXScript(xComponent, sUrl, aParams, aRet, aOutParamIndex,aOutParam);
        rtl::OUString aStringRes;
        aRet >>= aStringRes;
        std::cout << "value of Ret " << rtl::OUStringToOString( aStringRes, RTL_TEXTENCODING_UTF8 ).getStr() << std::endl;
        CPPUNIT_ASSERT_MESSAGE( "script reported failure",aStringRes == "OK" );
        pFoundShell->DoClose();
    }
}

ScMacrosTest::ScMacrosTest()
      : UnoApiTest("/sc/qa/extras/testdocuments")
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScMacrosTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
