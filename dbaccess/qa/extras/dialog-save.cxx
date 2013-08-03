/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>
#include <test/unoapi_test.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/script/XStorageBasedLibraryContainer.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <com/sun/star/embed/XExtendedStorageStream.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/intitem.hxx>
#include <comphelper/processfactory.hxx>

#include <basic/sbxdef.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


class DialogSaveTest : public UnoApiTest
{
public:
    DialogSaveTest();

    virtual void setUp();
    virtual void tearDown();

    void test();

    CPPUNIT_TEST_SUITE(DialogSaveTest);
// Should we disable this test on MOX and WNT?
// #if !defined(MACOSX) && !defined(WNT)
    CPPUNIT_TEST(test);
// #endif
    CPPUNIT_TEST_SUITE_END();

};


DialogSaveTest::DialogSaveTest()
      : UnoApiTest("/dbaccess/qa/extras/testdocuments")
{
}

void DialogSaveTest::test()
{
    const OUString aFileNameBase("testDialogSave.odb");
    OUString aFileName;
    createFileURL(aFileNameBase, aFileName);
    {
        uno::Reference< lang::XComponent > xComponent = loadFromDesktop(aFileName);
        CPPUNIT_ASSERT(xComponent.is());
        uno::Reference< frame::XStorable > xDocStorable(xComponent, UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xDocStorable.is());
        uno::Reference< document::XEmbeddedScripts > xDocScr(xComponent, UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xDocScr.is());
        uno::Reference< script::XStorageBasedLibraryContainer > xStorBasLib(xDocScr->getBasicLibraries());
        CPPUNIT_ASSERT(xStorBasLib.is());
        uno::Reference< script::XLibraryContainer > xBasLib(xStorBasLib, UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xBasLib.is());
        uno::Reference< script::XStorageBasedLibraryContainer > xStorDlgLib(xDocScr->getDialogLibraries());
        CPPUNIT_ASSERT(xStorDlgLib.is());
        uno::Reference< script::XLibraryContainer > xDlgLib(xStorDlgLib, UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xDlgLib.is());
        xBasLib->loadLibrary("Standard");
        CPPUNIT_ASSERT(xBasLib->isLibraryLoaded("Standard"));
        // the whole point of this test is to test the "save" operation
        // when the Basic library is loaded, but not the Dialog library
        CPPUNIT_ASSERT(!xDlgLib->isLibraryLoaded("Standard"));

        // make some change to enable a save
        // uno::Reference< document::XDocumentPropertiesSupplier > xDocPropSuppl(xComponent, UNO_QUERY_THROW);
        // CPPUNIT_ASSERT(xDocPropSuppl.is());
        // uno::Reference< document::XDocumentPropertiesSupplier > xDocProps(xDocPropSuppl->getDocumentProperties());
        // CPPUNIT_ASSERT(xDocProps.is());
        // xDocProps.setTitle(xDocProps.getTitle() + " suffix");
        uno::Reference< util::XModifiable > xDocMod(xComponent, UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xDocMod.is());
        xDocMod->setModified(sal_True);

        // now save; the code path to exercise in this test is the "store to same location"
        //           do *not* change to store(As|To|URL!
        xDocStorable->store();

        // close
        SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
        CPPUNIT_ASSERT(pFoundShell != NULL);
        pFoundShell->DoClose();
        // All our uno::References are (should?) be invalid now -> let them go out of scope
    }
    {
        uno::Reference< lang::XComponent > xComponent = loadFromDesktop(aFileName);
        CPPUNIT_ASSERT(xComponent.is());
        uno::Reference< embed::XHierarchicalStorageAccess > xDocHSA(xComponent, UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xDocHSA.is());
        uno::Reference< embed::XExtendedStorageStream > xDlg1ESS(xDocHSA->openStreamElementByHierarchicalName("Dialogs/Standard/Dialog1.xml", embed::ElementModes::READ));
        CPPUNIT_ASSERT(xDlg1ESS.is());
        uno::Reference< io::XStream > xDlg1Stream(xDlg1ESS, UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xDlg1Stream.is());
        uno::Reference< io::XInputStream > xDlg1InputStream(xDlg1Stream->getInputStream(), UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xDlg1InputStream.is());
        Sequence<sal_Int8> data(8);
        // should exist and be non-empty
        CPPUNIT_ASSERT(xDlg1InputStream->readBytes(data, 1) > 0);
    }
}

void DialogSaveTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    mxDesktop = com::sun::star::frame::Desktop::create( comphelper::getComponentContext(getMultiServiceFactory()) );
    CPPUNIT_ASSERT(mxDesktop.is());
}

void DialogSaveTest::tearDown()
{
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(DialogSaveTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
