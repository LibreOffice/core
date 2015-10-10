/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/unoapi_test.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/intitem.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <unotools/tempfile.hxx>
#include <comphelper/propertysequence.hxx>

#include <basic/sbxdef.hxx>

#include "docsh.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "document.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Implementation of Macros test */

class ScVbaExport : public UnoApiTest
{
public:
    ScVbaExport();

#if !defined MACOSX
    void testVba();
#endif
    CPPUNIT_TEST_SUITE(ScVbaExport);
#if !defined(MACOSX)
    //enable this test if you want to play with star basic macros in unit tests
    //works but does nothing useful yet
    CPPUNIT_TEST(testVba);
#endif

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XInterface> m_xCalcComponent;

    OUString maServiceName;

    std::shared_ptr<utl::TempFile> reload(const OUString& rFilterName, uno::Reference<lang::XComponent>& xComponent);
};

std::shared_ptr<utl::TempFile> ScVbaExport::reload(const OUString& rFilterName, uno::Reference<lang::XComponent>& xComponent)
{
    uno::Reference<frame::XStorable> xStorable(xComponent, uno::UNO_QUERY);
    auto aArgs(::comphelper::InitPropertySequence({
        { "FilterName", makeAny(rFilterName) }
    }));
    std::shared_ptr<utl::TempFile> pTempFile = std::make_shared<utl::TempFile>();
    pTempFile->EnableKillingFile();
    xStorable->storeToURL(pTempFile->GetURL(), aArgs);
    xComponent->dispose();
    xComponent = loadFromDesktop(pTempFile->GetURL(), maServiceName);

    return pTempFile;
}

#if !defined MACOSX

void ScVbaExport::testVba()
{
    TestMacroInfo testInfo[] = {
        {
            OUString("TestAddress."),
            OUString("vnd.sun.Star.script:VBAProject.testMacros.test?language=Basic&location=document")
        },
    };
    Sequence< uno::Any > aParams;
    for ( sal_uInt32  i=0; i<SAL_N_ELEMENTS( testInfo ); ++i )
    {
        OUString aFileName;
        createFileURL(testInfo[i].sFileBaseName + "xls", aFileName);
        uno::Reference< com::sun::star::lang::XComponent > xComponent = loadFromDesktop(aFileName, maServiceName);
        OUString sMsg( "Failed to load " + aFileName );
        CPPUNIT_ASSERT_MESSAGE( OUStringToOString( sMsg, RTL_TEXTENCODING_UTF8 ).getStr(), xComponent.is() );

        reload("MS Excel 97", xComponent);

        Any aRet;
        Sequence< sal_Int16 > aOutParamIndex;
        Sequence< Any > aOutParam;

        SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);

        CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
        SAL_INFO("sc.qa", "about to invoke vba test in " << aFileName);

        SfxObjectShell::CallXScript(
            xComponent, testInfo[i].sMacroUrl, aParams, aRet, aOutParamIndex,
            aOutParam);
        OUString aStringRes;
        aRet >>= aStringRes;
        SAL_INFO("sc.qa", "value of Ret " << aStringRes);
        CPPUNIT_ASSERT_MESSAGE( "script reported failure", aStringRes == "OK" );
        pFoundShell->DoClose();
    }
}

#endif

ScVbaExport::ScVbaExport()
      : UnoApiTest("/sc/qa/extras/testdocuments"),
      maServiceName("com.sun.star.sheet.SpreadsheetDocument")
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScVbaExport);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
