/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
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
#include <unotools/tempfile.hxx>

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
    void testFdo68983();
    CPPUNIT_TEST_SUITE(SwMacrosTest);
#if !defined(MACOSX) && !defined(WNT)
    //enable this test if you want to play with star basic macros in unit tests
    //works but does nothing useful yet
    //CPPUNIT_TEST(testStarBasic);
    CPPUNIT_TEST(testVba);
#endif
    CPPUNIT_TEST(testFdo55289);
    CPPUNIT_TEST(testFdo68983);

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

        OUString sUrl = testInfo[i].sMacroUrl;
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

void SwMacrosTest::testFdo68983()
{
    const OUString aFileNameBase("StarBasic.");
    OUString aFileName;
    createFileURL("fdo68983.", "odt", aFileName);
    Reference< com::sun::star::lang::XComponent > xComponent =
        loadFromDesktop(aFileName, "com.sun.star.text.TextDocument");

    CPPUNIT_ASSERT_MESSAGE("Failed to load StarBasic.ods", xComponent.is());

    Reference< frame::XStorable > xDocStorable(xComponent, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDocStorable.is());

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    Sequence<beans::PropertyValue> desc(1);
    desc[0].Name = "FilterName";
    desc[0].Value <<= OUString("writer8");
    xDocStorable->storeAsURL(aTempFile.GetURL(), desc);

    Reference<util::XCloseable>(xComponent, UNO_QUERY_THROW)->close(false);

    // re-load
    xComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.text.TextDocument");

    // check that password-protected library survived store and re-load
    Reference<document::XEmbeddedScripts> xDocScr(xComponent, UNO_QUERY_THROW);
    Reference<script::XStorageBasedLibraryContainer> xStorBasLib(xDocScr->getBasicLibraries());
    Reference<script::XLibraryContainer> xBasLib(xStorBasLib, UNO_QUERY_THROW);
    Reference<script::XLibraryContainerPassword> xBasLibPwd(xStorBasLib, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xBasLibPwd->isLibraryPasswordProtected("Library1"));
    CPPUNIT_ASSERT(xBasLibPwd->verifyLibraryPassword("Library1", "foo"));
    xBasLib->loadLibrary("Library1");
    CPPUNIT_ASSERT(xBasLib->isLibraryLoaded("Library1"));

    // close
    Reference<util::XCloseable> xDocCloseable(xComponent, UNO_QUERY_THROW);
    xDocCloseable->close(false);
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
