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
#include <com/sun/star/frame/XUntitledNumbers.hpp>

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/WindowArrange.hpp>
#include <com/sun/star/frame/TerminationVetoException.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/frame/XWindowArranger.hpp>
#include <com/sun/star/frame/XTask.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XFrames.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/frame/XTasksSupplier.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/frame/XDispatchRecorderSupplier.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/intitem.hxx>

#include <basic/sbxdef.hxx>

#include "docsh.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "document.hxx"

#define ODS_FORMAT_TYPE 50331943
#define XLS_FORMAT_TYPE 318767171
#define XLSX_FORMAT_TYPE 268959811

#define ODS     0
#define XLS     1
#define XLSX    2

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {

struct FileFormat {
    const char* pName; const char* pFilterName; const char* pTypeName; sal_uLong nFormatType;
};

FileFormat aFileFormats[] = {
    { "ods" , "calc8", "", ODS_FORMAT_TYPE },
    { "xls" , "MS Excel 97", "calc_MS_EXCEL_97", XLS_FORMAT_TYPE },
    { "xlsx", "Calc MS Excel 2007 XML" , "MS Excel 2007 XML", XLSX_FORMAT_TYPE }
};

}

/* Implementation of Filters test */

class ScMacrosTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:
    ScMacrosTest();

    virtual bool load(const rtl::OUString &rFilter, const rtl::OUString &rURL, const rtl::OUString &rUserData);
    ScDocShellRef load(const rtl::OUString &rFilter, const rtl::OUString &rURL,
        const rtl::OUString &rUserData, const rtl::OUString& rTypeName, sal_uLong nFormatType=0);
    uno::Reference< com::sun::star::frame::XModel > loadFromDesktop(const rtl::OUString& rURL);

    void createFileURL(const rtl::OUString& aFileBase, const rtl::OUString& aFileExtension, rtl::OUString& rFilePath);

    virtual void setUp();
    virtual void tearDown();

    void testStarBasic();
    void testVba();
    CPPUNIT_TEST_SUITE(ScMacrosTest);
#if 0
#ifndef MACOSX
    //enable this test if you want to play with star basic macros in unit tests
    //works but does nothing useful yet
    CPPUNIT_TEST(testStarBasic);
    //enable if you want to hack vba support for unit tests
    //does not work, still problems during loading
    CPPUNIT_TEST(testVba);
#endif
#endif

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XInterface> m_xCalcComponent;
    uno::Reference<frame::XDesktop> mxDesktop;
    ::rtl::OUString m_aBaseString;
};

ScDocShellRef ScMacrosTest::load(const rtl::OUString &rFilter, const rtl::OUString &rURL,
    const rtl::OUString &rUserData, const rtl::OUString& rTypeName, sal_uLong nFormatType)
{
    sal_uInt32 nFormat = 0;
    if (nFormatType)
        nFormat = SFX_FILTER_IMPORT | SFX_FILTER_USESOPTIONS;
    SfxFilter* aFilter = new SfxFilter(
        rFilter,
        rtl::OUString(), nFormatType, nFormat, rTypeName, 0, rtl::OUString(),
        rUserData, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/scalc*")) );
    aFilter->SetVersion(SOFFICE_FILEFORMAT_CURRENT);

    ScDocShellRef xDocShRef = new ScDocShell;
    SfxMedium* pSrcMed = new SfxMedium(rURL, STREAM_STD_READWRITE, true);
    pSrcMed->SetFilter(aFilter);
    if (!xDocShRef->DoLoad(pSrcMed))
    {
        xDocShRef->DoClose();
        // load failed.
        xDocShRef.Clear();
    }
    else if (nFormatType)
    {
        pSrcMed->GetItemSet()->Put( SfxUInt16Item( SID_MACROEXECMODE, 4));
        SfxObjectShell::SetCurrentComponent( xDocShRef->GetModel() );
    }

    return xDocShRef;
}

uno::Reference< com::sun::star::frame::XModel > ScMacrosTest::loadFromDesktop(const rtl::OUString& rURL)
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
    uno::Reference< com::sun::star::frame::XModel > xModel( xComponent, UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE("", xComponent.is());
    return xModel;
}


bool ScMacrosTest::load(const rtl::OUString &rFilter, const rtl::OUString &rURL,
    const rtl::OUString &rUserData)
{
    ScDocShellRef xDocShRef = load(rFilter, rURL, rUserData, rtl::OUString());
    bool bLoaded = xDocShRef.Is();
    //reference counting of ScDocShellRef is very confused.
    if (bLoaded)
        xDocShRef->DoClose();
    return bLoaded;
}

void ScMacrosTest::createFileURL(const rtl::OUString& aFileBase, const rtl::OUString& aFileExtension, rtl::OUString& rFilePath)
{
    rtl::OUString aSep(RTL_CONSTASCII_USTRINGPARAM("/"));
    rtl::OUStringBuffer aBuffer( getSrcRootURL() );
    aBuffer.append(m_aBaseString).append(aSep).append(aFileExtension);
    aBuffer.append(aSep).append(aFileBase).append(aFileExtension);
    rFilePath = aBuffer.makeStringAndClear();
}

void ScMacrosTest::testStarBasic()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("StarBasic."));
    rtl::OUString aFileExtension(aFileFormats[0].pName, strlen(aFileFormats[0].pName), RTL_TEXTENCODING_UTF8 );
    rtl::OUString aFilterName(aFileFormats[0].pFilterName, strlen(aFileFormats[0].pFilterName), RTL_TEXTENCODING_UTF8) ;
    rtl::OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    rtl::OUString aFilterType(aFileFormats[0].pTypeName, strlen(aFileFormats[0].pTypeName), RTL_TEXTENCODING_UTF8);
    std::cout << aFileFormats[0].pName << " Test" << std::endl;
    uno::Reference< com::sun::star::frame::XModel > xModel = loadFromDesktop(aFileName);

    CPPUNIT_ASSERT_MESSAGE("Failed to load StarBasic.ods", xModel.is());

    rtl::OUString aURL(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.Star.script:Standard.Module1.Macro1?language=Basic&location=document"));
    String sUrl = aURL;
    Any aRet;
    Sequence< sal_Int16 > aOutParamIndex;
    Sequence< Any > aOutParam;
    Sequence< uno::Any > aParams;

    com::sun::star::uno::Reference< com::sun::star::lang::XUnoTunnel >  xObjShellTunnel( xModel,com::sun::star::uno:: UNO_QUERY_THROW );
    SfxObjectShell* pFoundShell = reinterpret_cast<SfxObjectShell*>( xObjShellTunnel->getSomething(SfxObjectShell::getUnoTunnelId()));
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* xDocSh = ( ScDocShell*)pFoundShell;
    ScDocument* pDoc = xDocSh->GetDocument();

    pFoundShell->CallXScript(xModel, sUrl, aParams, aRet, aOutParamIndex,aOutParam);
    double aValue;
    pDoc->GetValue(0,0,0,aValue);
    std::cout << "returned value = " << aValue << std::endl;
    CPPUNIT_ASSERT_MESSAGE("script did not change the value of Sheet1.A1",aValue==2);
    xDocSh->DoClose();
}

struct TestMacroInfo
{
    rtl::OUString sFileBaseName;
    rtl::OUString sMacroUrl;
};
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
        }
    };

    rtl::OUString aFileExtension(aFileFormats[1].pName, strlen(aFileFormats[1].pName), RTL_TEXTENCODING_UTF8 );
    rtl::OUString aFilterName(aFileFormats[1].pFilterName, strlen(aFileFormats[1].pFilterName), RTL_TEXTENCODING_UTF8) ;
    rtl::OUString aFilterType(aFileFormats[1].pTypeName, strlen(aFileFormats[1].pTypeName), RTL_TEXTENCODING_UTF8);
    std::cout << aFileFormats[1].pName << " Test" << std::endl;
    for ( sal_uInt32  i=0; i<SAL_N_ELEMENTS( testInfo ); ++i )
    {
        rtl::OUString aFileName;
        createFileURL(testInfo[i].sFileBaseName, aFileExtension, aFileName);
        uno::Reference< com::sun::star::frame::XModel > xModel = loadFromDesktop(aFileName);
        rtl::OUString sMsg( RTL_CONSTASCII_USTRINGPARAM("Failed to load ") );
        sMsg.concat( aFileName );
        CPPUNIT_ASSERT_MESSAGE( rtl::OUStringToOString( sMsg, RTL_TEXTENCODING_UTF8 ).getStr(), xModel.is() );

        //is it really the right way to call a vba macro through CallXScript?
        //it seems that the basic ide does it differently, but then we would need to init all parts ourself
        //the problem is that CallXScript inits the basic part
        ////BasicIDE::RunMethod takes an SbMethod as parametre
        String sUrl = testInfo[i].sMacroUrl;
        Any aRet;
        Sequence< sal_Int16 > aOutParamIndex;
        Sequence< Any > aOutParam;
        Sequence< uno::Any > aParams;

        com::sun::star::uno::Reference< com::sun::star::lang::XUnoTunnel >  xObjShellTunnel( xModel,com::sun::star::uno:: UNO_QUERY_THROW );
        SfxObjectShell* pFoundShell = reinterpret_cast<SfxObjectShell*>( xObjShellTunnel->getSomething(SfxObjectShell::getUnoTunnelId()));

        CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
        pFoundShell->CallXScript(xModel, sUrl, aParams, aRet, aOutParamIndex,aOutParam);
        rtl::OUString aStringRes;
        aRet >>= aStringRes;
        std::cout << "value of Ret " << rtl::OUStringToOString( aStringRes, RTL_TEXTENCODING_UTF8 ).getStr() << std::endl;
        CPPUNIT_ASSERT_MESSAGE("script reported failure",aStringRes.equals( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("OK") )) );
        pFoundShell->DoClose();
    }
}

ScMacrosTest::ScMacrosTest()
      : m_aBaseString(RTL_CONSTASCII_USTRINGPARAM("/sc/qa/unit/data"))
{
}

void ScMacrosTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent =
        getMultiServiceFactory()->createInstance(rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.SpreadsheetDocument")));
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
    mxDesktop = Reference<com::sun::star::frame::XDesktop>( getMultiServiceFactory()->createInstance(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ))), UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE("", mxDesktop.is());
}

void ScMacrosTest::tearDown()
{
    uno::Reference< lang::XComponent >( m_xCalcComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScMacrosTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
