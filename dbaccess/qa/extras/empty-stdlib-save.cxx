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
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/script/XStorageBasedLibraryContainer.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
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
        xDlgLib->loadLibrary(sStandard);
        CPPUNIT_ASSERT(xBasLib->isLibraryLoaded(sStandard));
        CPPUNIT_ASSERT(xDlgLib->isLibraryLoaded(sStandard));

        Any a;
        uno::Reference< container::XNameContainer > xI;

        a = xBasLib->getByName(sStandard);
        a >>= xI;
        CPPUNIT_ASSERT(xI.is());
        xI->removeByName(u"Raralix"_ustr);

        a = xDlgLib->getByName(sStandard);
        a >>= xI;
        CPPUNIT_ASSERT(xI.is());
        xI->removeByName(u"Dialog1"_ustr);

        // uno::Reference< util::XModifiable > xDlgMod(xDlgLib, UNO_QUERY_THROW);
        // xDlgMod->setModified(sal_True);

        // uno::Reference< util::XModifiable > xScrMod(xDocScr, UNO_QUERY_THROW);
        // xScrMod->setModified(sal_True);

        // uno::Reference< util::XModifiable > xDocMod(mxComponent, UNO_QUERY_THROW);
        // std::cerr << "** Modified: " << static_cast<bool>(xDocMod->isModified()) << std::endl;
        // xDocMod->setModified(sal_True);
        // std::cerr << "** Modified: " << static_cast<bool>(xDocMod->isModified()) << std::endl;
        // CPPUNIT_ASSERT(xDocMod->isModified());

        // now save; the code path to exercise in this test is the "store to same location"
        //           do *not* change to store(As|To|URL)!
        xDocStorable->store();

        // All our uno::References are (should?) be invalid now -> let them go out of scope
    }
    {
        uno::Sequence<uno::Any> args{ uno::Any(maTempFile.GetURL()) };
        Reference<container::XHierarchicalNameAccess> xHNA(getMultiServiceFactory()->createInstanceWithArguments(u"com.sun.star.packages.Package"_ustr, args), UNO_QUERY_THROW);
        CPPUNIT_ASSERT(!xHNA->hasByHierarchicalName(u"Basic/Standard"_ustr));
        CPPUNIT_ASSERT(!xHNA->hasByHierarchicalName(u"Dialogs/Standard"_ustr));
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(DialogSaveTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
