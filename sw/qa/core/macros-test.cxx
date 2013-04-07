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
#include <unotest/macros_test.hxx>
#include <test/bootstrapfixture.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include <com/sun/star/frame/XDesktop.hpp>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/intitem.hxx>
#include <comphelper/processfactory.hxx>

#include <basic/sbxdef.hxx>

#include <doc.hxx>
#include "docsh.hxx"

SV_DECL_REF(SwDocShell)
SV_IMPL_REF(SwDocShell)

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Implementation of Macros test */

class SwMacrosTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    SwMacrosTest();

    SwDocShellRef load(const OUString &rFilter, const OUString &rURL,
        const OUString &rUserData, const OUString& rTypeName, sal_uLong nFormatType=0);

    void createFileURL(const OUString& aFileBase, const OUString& aFileExtension, OUString& rFilePath);

    virtual void setUp();
    virtual void tearDown();

    //void testStarBasic();
    void testVba();
    void testFdo55289();
    CPPUNIT_TEST_SUITE(SwMacrosTest);
#if !defined(MACOSX) && !defined(WNT)
    //enable this test if you want to play with star basic macros in unit tests
    //works but does nothing useful yet
    //CPPUNIT_TEST(testStarBasic);
    CPPUNIT_TEST(testVba);
#endif
    CPPUNIT_TEST(testFdo55289);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XInterface> m_xWriterComponent;
    OUString m_aBaseString;
};

void SwMacrosTest::createFileURL(const OUString& aFileBase, const OUString& aFileExtension, OUString& rFilePath)
{
    OUString aSep("/");
    OUStringBuffer aBuffer( getSrcRootURL() );
    aBuffer.append(m_aBaseString).append(aSep).append(aFileExtension);
    aBuffer.append(aSep).append(aFileBase).append(aFileExtension);
    rFilePath = aBuffer.makeStringAndClear();
}

#if 0

void SwMacrosTest::testStarBasic()
{
    const OUString aFileNameBase("StarBasic.");
    OUString aFileExtension(aFileFormats[0].pName, strlen(aFileFormats[0].pName), RTL_TEXTENCODING_UTF8 );
    OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    uno::Reference< com::sun::star::lang::XComponent > xComponent = loadFromDesktop(aFileName, "com.sun.star.text.TextDocument");

    CPPUNIT_ASSERT_MESSAGE("Failed to load StarBasic.ods", xComponent.is());

    OUString aURL("vnd.sun.Star.script:Standard.Module1.Macro1?language=Basic&location=document");
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

void SwMacrosTest::testVba()
{
    TestMacroInfo testInfo[] = {
        {
            OUString("testVba."),
            OUString("vnd.sun.Star.script:Project.NewMacros.Macro1?language=Basic&location=document")
        }
    };
    OUString aFileExtension( "doc" );
    for ( sal_uInt32  i=0; i<SAL_N_ELEMENTS( testInfo ); ++i )
    {
        OUString aFileName;
        createFileURL(testInfo[i].sFileBaseName, aFileExtension, aFileName);
        uno::Reference< com::sun::star::lang::XComponent > xComponent = loadFromDesktop(aFileName, "com.sun.star.text.TextDocument");
        OUStringBuffer sMsg( OUString("Failed to load ") );
        sMsg.append ( aFileName );
        CPPUNIT_ASSERT_MESSAGE( OUStringToOString( sMsg.makeStringAndClear(), RTL_TEXTENCODING_UTF8 ).getStr(), xComponent.is() );

        String sUrl = testInfo[i].sMacroUrl;
        Any aRet;
        Sequence< sal_Int16 > aOutParamIndex;
        Sequence< Any > aOutParam;
        Sequence< uno::Any > aParams;

        SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);

        CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
        pFoundShell->CallXScript(xComponent, sUrl, aParams, aRet, aOutParamIndex,aOutParam);
        OUString aStringRes;
        aRet >>= aStringRes;
        std::cout << "value of Ret " << OUStringToOString( aStringRes, RTL_TEXTENCODING_UTF8 ).getStr() << std::endl;
        //CPPUNIT_ASSERT_MESSAGE( "script reported failure",aStringRes == "OK" );
        pFoundShell->DoClose();
    }
}

void SwMacrosTest::testFdo55289()
{
    SwDoc *const pDoc = new SwDoc;
    SwDocShellRef pDocShell = new SwDocShell(pDoc, SFX_CREATE_MODE_EMBEDDED);
    // this needs to run with no layout to tickle the bugs in the special
    // cases in SwXShape re-anchoring
    assert(!pDoc->GetCurrentLayout());

    uno::Reference<frame::XModel> const xModel(pDocShell->GetModel());
    uno::Reference<drawing::XDrawPageSupplier> const xDPS(xModel, UNO_QUERY);
    uno::Reference<drawing::XShapes> const xShapes(xDPS->getDrawPage(),
            UNO_QUERY);
    uno::Reference<beans::XPropertySet> const xShape(
        uno::Reference<lang::XMultiServiceFactory>(xModel, UNO_QUERY)->
            createInstance("com.sun.star.drawing.GraphicObjectShape"),
        UNO_QUERY);
    xShape->setPropertyValue("AnchorType",
            makeAny(text::TextContentAnchorType_AT_PAGE));
    xShapes->add(uno::Reference<drawing::XShape>(xShape, UNO_QUERY));
    xShape->setPropertyValue("AnchorType",
            makeAny(text::TextContentAnchorType_AT_CHARACTER));
    xShape->setPropertyValue("AnchorType",
            makeAny(text::TextContentAnchorType_AS_CHARACTER));
    xShape->setPropertyValue("AnchorType",
            makeAny(text::TextContentAnchorType_AT_CHARACTER));
    xShape->setPropertyValue("AnchorType",
            makeAny(text::TextContentAnchorType_AS_CHARACTER));
    uno::Reference<text::XTextRange> const xEnd =
        uno::Reference<text::XTextDocument>(xModel, UNO_QUERY)->getText()->getEnd();
    uno::Reference<text::XTextContent> const xShapeContent(xShape, UNO_QUERY);
    xShapeContent->attach(xEnd);
}

SwMacrosTest::SwMacrosTest()
      : m_aBaseString("/sw/qa/core/data")
{
}

void SwMacrosTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that SwGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xWriterComponent =
        getMultiServiceFactory()->createInstance(OUString(
        "com.sun.star.comp.Writer.TextDocument"));
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xWriterComponent.is());
    mxDesktop = com::sun::star::frame::Desktop::create( comphelper::getComponentContext(getMultiServiceFactory()) );
}

void SwMacrosTest::tearDown()
{
    uno::Reference< lang::XComponent >( m_xWriterComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwMacrosTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
