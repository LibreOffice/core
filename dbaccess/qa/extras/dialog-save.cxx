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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/script/XStorageBasedLibraryContainer.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


class DialogSaveTest : public UnoApiTest
{
public:
    DialogSaveTest();

    void test();

    CPPUNIT_TEST_SUITE(DialogSaveTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

};


DialogSaveTest::DialogSaveTest()
      : UnoApiTest(u"/dbaccess/qa/extras/testdocuments"_ustr)
{
}

void DialogSaveTest::test()
{
    createTempCopy(u"testDialogSave.odb");
    {
        mxComponent = loadFromDesktop(maTempFile.GetURL());
        uno::Reference< frame::XStorable > xDocStorable(mxComponent, UNO_QUERY_THROW);
        uno::Reference< document::XEmbeddedScripts > xDocScr(mxComponent, UNO_QUERY_THROW);
        uno::Reference< script::XStorageBasedLibraryContainer > xStorBasLib(xDocScr->getBasicLibraries());
        CPPUNIT_ASSERT(xStorBasLib.is());
        uno::Reference< script::XLibraryContainer > xBasLib(xStorBasLib, UNO_QUERY_THROW);
        uno::Reference< script::XStorageBasedLibraryContainer > xStorDlgLib(xDocScr->getDialogLibraries());
        CPPUNIT_ASSERT(xStorDlgLib.is());
        uno::Reference< script::XLibraryContainer > xDlgLib(xStorDlgLib, UNO_QUERY_THROW);
        static constexpr OUString sStandard(u"Standard"_ustr);
        xBasLib->loadLibrary(sStandard);
        CPPUNIT_ASSERT(xBasLib->isLibraryLoaded(sStandard));
        // the whole point of this test is to test the "save" operation
        // when the Basic library is loaded, but not the Dialog library
        CPPUNIT_ASSERT(!xDlgLib->isLibraryLoaded(sStandard));

        // make some change to enable a save
        // uno::Reference< document::XDocumentPropertiesSupplier > xDocPropSuppl(mxComponent, UNO_QUERY_THROW);
        // uno::Reference< document::XDocumentPropertiesSupplier > xDocProps(xDocPropSuppl->getDocumentProperties());
        // CPPUNIT_ASSERT(xDocProps.is());
        // xDocProps.setTitle(xDocProps.getTitle() + " suffix");
        uno::Reference< util::XModifiable > xDocMod(mxComponent, UNO_QUERY_THROW);
        xDocMod->setModified(true);

        // now save; the code path to exercise in this test is the "store to same location"
        //           do *not* change to store(As|To|URL)!
        xDocStorable->store();

        // All our uno::References are (should?) be invalid now -> let them go out of scope
    }
    {
        uno::Sequence<uno::Any> args{ uno::Any(maTempFile.GetURL()) };
        Reference<container::XHierarchicalNameAccess> xHNA(getMultiServiceFactory()->createInstanceWithArguments(u"com.sun.star.packages.Package"_ustr, args), UNO_QUERY_THROW);
        Reference< beans::XPropertySet > xPS(xHNA->getByHierarchicalName(u"Dialogs/Standard/Dialog1.xml"_ustr), UNO_QUERY_THROW);
        sal_Int64 nSize = 0;
        CPPUNIT_ASSERT(xPS->getPropertyValue(u"Size"_ustr) >>= nSize);
        CPPUNIT_ASSERT(nSize != 0);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(DialogSaveTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
