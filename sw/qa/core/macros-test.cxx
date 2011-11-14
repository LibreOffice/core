/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Caolán McNamara <caolanm@redhat.com>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Caolán McNamara <caolanm@redhat.com>
 *   Markus Mohrhard <markus.mohrhard@googlemail.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <sal/config.h>
#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include <com/sun/star/frame/XDesktop.hpp>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/intitem.hxx>

#include <basic/sbxdef.hxx>

#include "docsh.hxx"

SO2_DECL_REF(SwDocShell)
SO2_IMPL_REF(SwDocShell)

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Implementation of Macros test */

class SwMacrosTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:
    SwMacrosTest();

    virtual bool load(const rtl::OUString &rFilter, const rtl::OUString &rURL, const rtl::OUString &rUserData);
    SwDocShellRef load(const rtl::OUString &rFilter, const rtl::OUString &rURL,
        const rtl::OUString &rUserData, const rtl::OUString& rTypeName, sal_uLong nFormatType=0);
    uno::Reference< com::sun::star::lang::XComponent > loadFromDesktop(const rtl::OUString& rURL);

    void createFileURL(const rtl::OUString& aFileBase, const rtl::OUString& aFileExtension, rtl::OUString& rFilePath);

    virtual void setUp();
    virtual void tearDown();

    //void testStarBasic();
    void testVba();
    CPPUNIT_TEST_SUITE(SwMacrosTest);
#if !defined(MACOSX) && !defined(WNT)
    //enable this test if you want to play with star basic macros in unit tests
    //works but does nothing useful yet
    //CPPUNIT_TEST(testStarBasic);
    CPPUNIT_TEST(testVba);
#endif

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XInterface> m_xCalcComponent;
    uno::Reference<frame::XDesktop> mxDesktop;
    ::rtl::OUString m_aBaseString;
};

uno::Reference< com::sun::star::lang::XComponent > SwMacrosTest::loadFromDesktop(const rtl::OUString& rURL)
{
    uno::Reference< com::sun::star::frame::XComponentLoader> xLoader = uno::Reference< com::sun::star::frame::XComponentLoader >( mxDesktop, UNO_QUERY );
    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > args(1);
    args[0].Name = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("MacroExecutionMode"));
    args[0].Handle = -1;
    args[0].Value <<=
        com::sun::star::document::MacroExecMode::ALWAYS_EXECUTE_NO_WARN;
    args[0].State = com::sun::star::beans::PropertyState_DIRECT_VALUE;
    uno::Reference< com::sun::star::lang::XComponent> xComponent= xLoader->loadComponentFromURL(rURL, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_default")), 0, args);
    CPPUNIT_ASSERT_MESSAGE("loading failed", xComponent.is());
    return xComponent;
}


bool SwMacrosTest::load(const rtl::OUString &rFilter, const rtl::OUString &rURL,
    const rtl::OUString &rUserData)
{
    SwDocShellRef xDocShRef;
    bool bLoaded = xDocShRef.Is();
    //reference counting of SwDocShellRef is very confused.
    if (bLoaded)
        xDocShRef->DoClose();
    return bLoaded;
}

void SwMacrosTest::createFileURL(const rtl::OUString& aFileBase, const rtl::OUString& aFileExtension, rtl::OUString& rFilePath)
{
    rtl::OUString aSep(RTL_CONSTASCII_USTRINGPARAM("/"));
    rtl::OUStringBuffer aBuffer( getSrcRootURL() );
    aBuffer.append(m_aBaseString).append(aSep).append(aFileExtension);
    aBuffer.append(aSep).append(aFileBase).append(aFileExtension);
    rFilePath = aBuffer.makeStringAndClear();
}

#if 0

void SwMacrosTest::testStarBasic()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("StarBasic."));
    rtl::OUString aFileExtension(aFileFormats[0].pName, strlen(aFileFormats[0].pName), RTL_TEXTENCODING_UTF8 );
    rtl::OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    uno::Reference< com::sun::star::lang::XComponent > xComponent = loadFromDesktop(aFileName);

    CPPUNIT_ASSERT_MESSAGE("Failed to load StarBasic.ods", xComponent.is());

    rtl::OUString aURL(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:Standard.Module1.Macro1?language=Basic&location=document"));
    String sUrl = aURL;
    Any aRet;
    Sequence< sal_Int16 > aOutParamIndex;
    Sequence< Any > aOutParam;
    Sequence< uno::Any > aParams;

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    pFoundShell->CallXScript(xComponent, sUrl, aParams, aRet, aOutParamIndex,aOutParam);
    pFoundShell->DoClose();
}

#endif

struct TestMacroInfo
{
    rtl::OUString sFileBaseName;
    rtl::OUString sMacroUrl;
};
void SwMacrosTest::testVba()
{
    TestMacroInfo testInfo[] = {
        {
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("testVba.")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:Project.NewMacros.Macro1?language=Basic&location=document"))
        }
    };
    rtl::OUString aFileExtension( RTL_CONSTASCII_USTRINGPARAM("doc") );
    for ( sal_uInt32  i=0; i<SAL_N_ELEMENTS( testInfo ); ++i )
    {
        rtl::OUString aFileName;
        createFileURL(testInfo[i].sFileBaseName, aFileExtension, aFileName);
        uno::Reference< com::sun::star::lang::XComponent > xComponent = loadFromDesktop(aFileName);
        rtl::OUStringBuffer sMsg( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Failed to load ")) );
        sMsg.append ( aFileName );
        CPPUNIT_ASSERT_MESSAGE( rtl::OUStringToOString( sMsg.makeStringAndClear(), RTL_TEXTENCODING_UTF8 ).getStr(), xComponent.is() );

        String sUrl = testInfo[i].sMacroUrl;
        Any aRet;
        Sequence< sal_Int16 > aOutParamIndex;
        Sequence< Any > aOutParam;
        Sequence< uno::Any > aParams;

        SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);

        CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
        pFoundShell->CallXScript(xComponent, sUrl, aParams, aRet, aOutParamIndex,aOutParam);
        rtl::OUString aStringRes;
        aRet >>= aStringRes;
        std::cout << "value of Ret " << rtl::OUStringToOString( aStringRes, RTL_TEXTENCODING_UTF8 ).getStr() << std::endl;
        //CPPUNIT_ASSERT_MESSAGE("script reported failure",aStringRes.equals( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("OK") )) );
        pFoundShell->DoClose();
    }
}

SwMacrosTest::SwMacrosTest()
      : m_aBaseString(RTL_CONSTASCII_USTRINGPARAM("/sw/qa/core/data"))
{
}

void SwMacrosTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that SwGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent =
        getMultiServiceFactory()->createInstance(rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Writer.TextDocument")));
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
    mxDesktop = Reference<com::sun::star::frame::XDesktop>( getMultiServiceFactory()->createInstance(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ))), UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE("", mxDesktop.is());
}

void SwMacrosTest::tearDown()
{
    uno::Reference< lang::XComponent >( m_xCalcComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwMacrosTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
