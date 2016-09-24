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
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
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

    void test();

    CPPUNIT_TEST_SUITE(DialogSaveTest);
// Should we disable this test on MOX and WNT?
// #if !defined(MACOSX) && !defined(_WIN32)
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
    // UnoApiTest::setUp (via InitVCL) puts each test under a locked SolarMutex,
    // but at least the below xDocCloseable->close call could lead to a deadlock
    // then, and it looks like none of the code here requires the SolarMutex to
    // be locked anyway:
    SolarMutexReleaser rel;

    const OUString sStandard("Standard");
    const OUString aFileName(m_directories.getURLFromWorkdir("CppunitTest/testNolibSave.odb"));
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
        xBasLib->loadLibrary(sStandard);
        xDlgLib->loadLibrary(sStandard);
        CPPUNIT_ASSERT(xBasLib->isLibraryLoaded(sStandard));
        CPPUNIT_ASSERT(xDlgLib->isLibraryLoaded(sStandard));

        xBasLib->removeLibrary(sStandard);
        xDlgLib->removeLibrary(sStandard);

        // uno::Reference< util::XModifiable > xDlgMod(xDlgLib, UNO_QUERY_THROW);
        // CPPUNIT_ASSERT(xDlgMod.is());
        // xDlgMod->setModified(sal_True);

        // uno::Reference< util::XModifiable > xScrMod(xDocScr, UNO_QUERY_THROW);
        // CPPUNIT_ASSERT(xScrMod.is());
        // xScrMod->setModified(sal_True);

        // uno::Reference< util::XModifiable > xDocMod(xComponent, UNO_QUERY_THROW);
        // CPPUNIT_ASSERT(xDocMod.is());
        // std::cerr << "** Modified: " << static_cast<bool>(xDocMod->isModified()) << std::endl;
        // xDocMod->setModified(sal_True);
        // std::cerr << "** Modified: " << static_cast<bool>(xDocMod->isModified()) << std::endl;
        // CPPUNIT_ASSERT(xDocMod->isModified());

        // now save; the code path to exercise in this test is the "store to same location"
        //           do *not* change to store(As|To|URL)!
        xDocStorable->store();

        // close
        uno::Reference< util::XCloseable > xDocCloseable(xComponent, UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xDocCloseable.is());
        xDocCloseable->close(false);

        // All our uno::References are (should?) be invalid now -> let them go out of scope
    }
    {
        uno::Sequence<uno::Any> args(1);
        args[0] <<= aFileName;
        Reference<container::XHierarchicalNameAccess> xHNA(getMultiServiceFactory()->createInstanceWithArguments("com.sun.star.packages.Package", args), UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xHNA.is());
        CPPUNIT_ASSERT(!xHNA->hasByHierarchicalName("Basic/Standard"));
        CPPUNIT_ASSERT(!xHNA->hasByHierarchicalName("Dialogs/Standard"));
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(DialogSaveTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
