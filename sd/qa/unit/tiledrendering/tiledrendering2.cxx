/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "tiledrenderingmodeltestbase.cxx"

#include <sfx2/sidebar/Sidebar.hxx>
#include <vcl/scheduler.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <svl/cryptosign.hxx>

#include <DrawDocShell.hxx>
#include <ViewShell.hxx>

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSidebarSwitchDeck)
{
    // Given an impress document, with a visible sidebar (ModifyPage deck):
    createDoc("dummy.odp");
    ViewCallback aView;
    sfx2::sidebar::Sidebar::Setup(u"");
    Scheduler::ProcessEventsToIdle();
    aView.m_aStateChanges.clear();

    // When switching to the MasterSlidesPanel deck:
    dispatchCommand(mxComponent, u".uno:MasterSlidesPanel"_ustr, {});

    // Then make sure notifications are sent for both the old and the new decks:
    auto it = aView.m_aStateChanges.find(".uno:ModifyPage");
    // Without the accompanying fix in place, this test would have failed, the notification for the
    // old deck was missing.
    CPPUNIT_ASSERT(it != aView.m_aStateChanges.end());
    boost::property_tree::ptree aTree = it->second;
    CPPUNIT_ASSERT(aTree.get_child_optional("state").has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("false"), aTree.get_child("state").get_value<std::string>());
    it = aView.m_aStateChanges.find(".uno:MasterSlidesPanel");
    CPPUNIT_ASSERT(it != aView.m_aStateChanges.end());
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testInsertSignatureLineExternal)
{
    // Given a PDF to be signed:
    uno::Sequence<beans::PropertyValue> aArgs = { comphelper::makePropertyValue("ReadOnly", true) };
    createTempCopy(u"empty.pdf");
    loadWithParams(maTempFile.GetURL(), aArgs);
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    pImpressDocument->initializeForTiledRendering({});
    sd::ViewShell* pViewShell = pImpressDocument->GetDocShell()->GetViewShell();
    sd::View* pView = pViewShell->GetView();
    pView->SetAuthor("myauthor");
    ViewCallback aView;

    // When insrerting a signature line for electronic (extrenal) signing:
    aArgs = {
        comphelper::makePropertyValue("External", true),
    };
    // Without the accompanying fix in place, this test would hang here in the certificate chooser
    // dialog.
    dispatchCommand(mxComponent, ".uno:InsertSignatureLine", aArgs);
    // Signature line is selected right after inserting:
    CPPUNIT_ASSERT(pViewShell->GetViewShell()->GetSignPDFCertificate().Is());

    // Then make sure the shape is marked as a signature line:
    std::vector<SdrObject*> aMarkedObjects = pView->GetMarkedObjects();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aMarkedObjects.size());
    uno::Any aAny;
    aMarkedObjects[0]->GetGrabBagItem(aAny);
    comphelper::SequenceAsHashMap aMap(aAny);
    CPPUNIT_ASSERT(aMap.contains("SignatureCertificate"));
    // Also verify that this is exposed at a LOK level:
    OString aShapeSelection = "[" + aView.m_ShapeSelection + "]";
    const char* pShapeSelectionStr = aShapeSelection.getStr();
    std::stringstream aStream(pShapeSelectionStr);
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    int nCount = 0;
    bool bSignature = false;
    for (const auto& i : aTree)
    {
        ++nCount;
        if (nCount <= 5)
        {
            // x, y, w, h, part
            continue;
        }
        boost::property_tree::ptree aProps = i.second;
        // Without the accompanying fix in place, this test would have failed with:
        // - No such node (isSignature)
        bSignature = aProps.get<bool>("isSignature");
    }
    CPPUNIT_ASSERT(bSignature);

    // Make sure there is no leaked selection after signing is finished:
    OUString aSigUrl = createFileURL(u"signature.pkcs7");
    SvFileStream aSigStream(aSigUrl, StreamMode::READ);
    auto aSigValue
        = OUString::fromUtf8(read_uInt8s_ToOString(aSigStream, aSigStream.remainingSize()));
    aArgs = {
        comphelper::makePropertyValue(u"SignatureTime"_ustr, u"1643201995722"_ustr),
        comphelper::makePropertyValue(u"SignatureValue"_ustr, aSigValue),
    };
    dispatchCommand(mxComponent, u".uno:Signature"_ustr, aArgs);
    // Signature line is not selected after finishing signing:
    CPPUNIT_ASSERT(!pViewShell->GetViewShell()->GetSignPDFCertificate().Is());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
