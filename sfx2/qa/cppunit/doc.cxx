/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>

#include <comphelper/propertyvalue.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <osl/file.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequence.hxx>

using namespace com::sun::star;

namespace
{
/// Covers sfx2/source/doc/ fixes.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"/sfx2/qa/cppunit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testNoGrabBagShape)
{
    // Load a document and select the first shape.
    css::uno::Sequence<css::beans::PropertyValue> aArgs{ comphelper::makePropertyValue(
        u"ReadOnly"_ustr, true) };
    mxComponent = loadFromDesktop(u"private:factory/simpress"_ustr, u""_ustr, aArgs);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(xModel, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage(
        xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Any aShape = xDrawPage->getByIndex(0);
    uno::Reference<view::XSelectionSupplier> xController(xModel->getCurrentController(),
                                                         uno::UNO_QUERY);
    xController->select(aShape);

    // See if it has a signing certificate associated.
    auto pBaseModel = dynamic_cast<SfxBaseModel*>(xModel.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();

    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.beans.UnknownPropertyException
    // which was not caught later, resulting in a crash.
    pObjectShell->GetSignPDFCertificate();
}

CPPUNIT_TEST_FIXTURE(Test, testTempFilePath)
{
    // Create a test file in a directory that contains the URL-encoded "testÿ" string.
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    auto pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    OUString aTargetDir
        = m_directories.getURLFromWorkdir(u"CppunitTest/sfx2_doc.test.user/test%25C3%25Bf");
    osl::Directory::create(aTargetDir);
    OUString aTargetFile = aTargetDir + "/test.odt";
    css::uno::Sequence<css::beans::PropertyValue> aArgs{ comphelper::makePropertyValue(
        u"FilterName"_ustr, u"writer8"_ustr) };
    pBaseModel->storeAsURL(aTargetFile, aArgs);
    mxComponent->dispose();

    // Load it and export to PDF.
    mxComponent = loadFromDesktop(aTargetFile);
    pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    OUString aPdfTarget = aTargetDir + "/test.pdf";
    css::uno::Sequence<css::beans::PropertyValue> aPdfArgs{ comphelper::makePropertyValue(
        u"FilterName"_ustr, u"writer_pdf_Export"_ustr) };
    // Without the accompanying fix in place, this test would have failed on Windows with:
    // An uncaught exception of type com.sun.star.io.IOException
    // because we first tried to create a temp file next to test.odt in a directory named
    // "test%25C3%25Bf" instead of a directory named "test%C3%Bf".
    pBaseModel->storeToURL(aPdfTarget, aPdfArgs);
}

CPPUNIT_TEST_FIXTURE(Test, testSetDocumentPropertiesUpdate)
{
    // Given a document with 3 custom props, 2 Zotero ones and one other:
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    auto pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    uno::Reference<document::XDocumentProperties> xDP = pBaseModel->getDocumentProperties();
    uno::Reference<beans::XPropertyContainer> xUDP = xDP->getUserDefinedProperties();
    xUDP->addProperty(u"ZOTERO_PREF_1"_ustr, beans::PropertyAttribute::REMOVABLE,
                      uno::Any(u"foo"_ustr));
    xUDP->addProperty(u"ZOTERO_PREF_2"_ustr, beans::PropertyAttribute::REMOVABLE,
                      uno::Any(u"bar"_ustr));
    xUDP->addProperty(u"OTHER"_ustr, beans::PropertyAttribute::REMOVABLE, uno::Any(u"baz"_ustr));

    // When updating the Zotero ones (1 update, 1 removal):
    std::vector<beans::PropertyValue> aArgsVec = comphelper::JsonToPropertyValues(R"json(
{
    "UpdatedProperties": {
        "type": "[]com.sun.star.beans.PropertyValue",
        "value": {
            "NamePrefix": {
                "type": "string",
                "value": "ZOTERO_PREF_"
            },
            "UserDefinedProperties": {
                "type": "[]com.sun.star.beans.PropertyValue",
                "value": {
                    "ZOTERO_PREF_1": {
                        "type": "string",
                        "value": "test"
                    }
                }
            }
        }
    }
}
)json"_ostr);
    uno::Sequence<beans::PropertyValue> aArgs = comphelper::containerToSequence(aArgsVec);
    dispatchCommand(mxComponent, u".uno:SetDocumentProperties"_ustr, aArgs);

    // Then make sure that OTHER is still there and that ZOTERO_PREF_1 + ZOTERO_PREF_2 gets updated
    // to the new value of a single ZOTERO_PREF_1:
    uno::Reference<beans::XPropertyAccess> xUDPAccess(xUDP, uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aMap(xUDPAccess->getPropertyValues());
    auto it = aMap.find(u"ZOTERO_PREF_1"_ustr);
    CPPUNIT_ASSERT(it != aMap.end());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: test
    // - Actual  : foo
    // i.e. ZOTERO_PREF_1 was not updated.
    CPPUNIT_ASSERT_EQUAL(u"test"_ustr, it->second.get<OUString>());
    CPPUNIT_ASSERT(bool(aMap.find(u"ZOTERO_PREF_2"_ustr) == aMap.end()));
    CPPUNIT_ASSERT(aMap.find(u"OTHER"_ustr) != aMap.end());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
