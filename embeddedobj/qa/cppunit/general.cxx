/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>
#include <test/unoapi_test.hxx>

#include <com/sun/star/embed/XStorage.hpp>

#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/storagehelper.hxx>
#include <officecfg/Office/Common.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers embeddedobj/source/general/ fixes.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/embeddedobj/qa/cppunit/data/")
    {
    }
};
}

CPPUNIT_TEST_FIXTURE(Test, testInsertFileConfig)
{
    // Explicitly disable Word->Writer mapping for this test.
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Filter::Microsoft::Import::WinWordToWriter::set(false, pBatch);
    pBatch->commit();
    comphelper::ScopeGuard g([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatchReset(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Filter::Microsoft::Import::WinWordToWriter::set(true,
                                                                                   pBatchReset);
        pBatchReset->commit();
    });
    mxComponent.set(loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument"));

    // Insert a file as an embedded object.
    uno::Reference<embed::XStorage> xStorage = comphelper::OStorageHelper::GetTemporaryStorage();
    comphelper::EmbeddedObjectContainer aContainer(xStorage);
    OUString aFileName = createFileURL(u"insert-file-config.doc");
    uno::Sequence<beans::PropertyValue> aMedium{ comphelper::makePropertyValue("URL", aFileName) };
    OUString aName("Object 1");
    uno::Reference<embed::XEmbeddedObject> xObject
        = aContainer.InsertEmbeddedObject(aMedium, aName);

    // Make sure that the insertion fails:
    // 1) the user explicitly requested that the data is not loaded into Writer
    // 2) this is non-Windows, so OLE embedding is not an option
    // so silently still loading the data into Writer would be bad.
    CPPUNIT_ASSERT(!xObject.is());
}

CPPUNIT_TEST_FIXTURE(Test, testInsertFileConfigVsdx)
{
    // Explicitly disable Word->Writer mapping for this test.
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Filter::Microsoft::Import::VisioToDraw::set(false, pBatch);
    pBatch->commit();
    comphelper::ScopeGuard g([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatchReset(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Filter::Microsoft::Import::VisioToDraw::set(true, pBatchReset);
        pBatchReset->commit();
    });
    mxComponent.set(loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument"));

    // Insert a file as an embedded object.
    uno::Reference<embed::XStorage> xStorage = comphelper::OStorageHelper::GetTemporaryStorage();
    comphelper::EmbeddedObjectContainer aContainer(xStorage);
    OUString aFileName = createFileURL(u"insert-file-config.vsdx");
    uno::Sequence<beans::PropertyValue> aMedium{ comphelper::makePropertyValue("URL", aFileName) };
    OUString aName("Object 1");
    uno::Reference<embed::XEmbeddedObject> xObject
        = aContainer.InsertEmbeddedObject(aMedium, aName);

    // Make sure that the insertion fails:
    // 1) the user explicitly requested that the data is not loaded into Writer
    // 2) this is non-Windows, so OLE embedding is not an option
    // so silently still loading the data into Writer would be bad.
    CPPUNIT_ASSERT(!xObject.is());
}

CPPUNIT_TEST_FIXTURE(Test, testInsertFileConfigPdf)
{
    // Explicitly disable Word->Writer mapping for this test.
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Filter::Adobe::Import::PDFToDraw::set(false, pBatch);
    pBatch->commit();
    comphelper::ScopeGuard g([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatchReset(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Filter::Adobe::Import::PDFToDraw::set(true, pBatchReset);
        pBatchReset->commit();
    });
    mxComponent.set(loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument"));

#if HAVE_FEATURE_POPPLER
    // Insert a PDF file as an embedded object.
    uno::Reference<embed::XStorage> xStorage = comphelper::OStorageHelper::GetTemporaryStorage();
    comphelper::EmbeddedObjectContainer aContainer(xStorage);
    OUString aFileName = createFileURL(u"insert-file-config.pdf");
    uno::Sequence<beans::PropertyValue> aMedium{ comphelper::makePropertyValue("URL", aFileName) };
    OUString aName("Object 1");
    uno::Reference<embed::XEmbeddedObject> xObject
        = aContainer.InsertEmbeddedObject(aMedium, aName);

    // Make sure that the insertion fails:
    // 1) the user explicitly requested that the data is not loaded into Writer
    // 2) this is non-Windows, so OLE embedding is not an option
    // so silently still loading the data into Writer would be bad.
    CPPUNIT_ASSERT(!xObject.is());
#endif
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
