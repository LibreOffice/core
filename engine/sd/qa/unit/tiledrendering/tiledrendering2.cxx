/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_pdfimport.h>
#include <sdtiledrenderingtest.hxx>

#include <sfx2/sidebar/Sidebar.hxx>
#include <vcl/scheduler.hxx>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <svl/cryptosign.hxx>
#include <sfx2/linkmgr.hxx>

#include <vcl/virdev.hxx>

#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <drawdoc.hxx>
#include <unomodel.hxx>

using namespace css;

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSidebarSwitchDeck)
{
    // Given an impress document, with a visible sidebar (ModifyPage deck):
    createDoc("dummy.odp");
    SdTestViewCallback aView;
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

#if ENABLE_PDFIMPORT
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
    pViewShell->GetViewShell()->SetKitAuthor("myauthor");
    SdTestViewCallback aView;

    // When inserting a signature line for electronic (external) signing:
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
    // Also verify that this is exposed at a COKit level:
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
#endif

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testNotesViewInvalidations)
{
    // Given a document with 2 slides.
    SdXImpressDocument* pXImpressDocument = createDoc("NotesView.odp");
    SdTestViewCallback aView;
    CPPUNIT_ASSERT_EQUAL(2, pXImpressDocument->getParts());

    // Switching to the second slide.
    pXImpressDocument->setPart(1);

    Scheduler::ProcessEventsToIdle();

    aView.invalidatedAll = false;

    // Switching to notes view.
    dispatchCommand(mxComponent, ".uno:NotesMode", uno::Sequence<beans::PropertyValue>());

    CPPUNIT_ASSERT_EQUAL(true, aView.invalidatedAll);
    CPPUNIT_ASSERT_EQUAL(1, aView.partOfInvalidation);
    CPPUNIT_ASSERT_EQUAL(2, aView.editModeOfInvalidation);
}

#if ENABLE_PDFIMPORT
CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testPdfiumLinks)
{
    // Given a pdf file with links:
    SdXImpressDocument* pDoc = createDoc("link_2p.pdf");
    SdTestViewCallback aView;

    auto get_links_array = [&aView]() -> boost::property_tree::ptree {
        auto it = aView.m_aStateChanges.find("PageLinks");
        CPPUNIT_ASSERT(it != aView.m_aStateChanges.end());
        boost::property_tree::ptree pTree = it->second;
        auto it2 = pTree.find("state");
        CPPUNIT_ASSERT(it2 != pTree.not_found());
        auto it3 = it2->second.find("links");
        CPPUNIT_ASSERT(it3 != pTree.not_found());
        return it3->second;
    };

    // First page has a link
    pDoc->setPart(0);
    auto links = get_links_array();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), links.size());
    // auto pLinkInfo = links.get_child("");
    auto pLinkInfo = links.begin()->second;
    CPPUNIT_ASSERT_EQUAL(std::string("767.28x292.999@(1133.86,2013)"),
                         pLinkInfo.get_child("rectangle").get_value<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("http://cidac.pt/"),
                         pLinkInfo.get_child("uri").get_value<std::string>());

    // Second doesn't
    pDoc->setPart(1);
    links = get_links_array();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), links.size());
}
#endif

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideBackgroundRemoteNotFetched)
{
    // Slide background fill image with a remote URL must not fetch
    // the URL during paint when link updates are not allowed.
    // The assert in createNewSdrFillGraphicAttribute will fire if
    // a remote fetch is attempted.
    uno::Sequence<beans::PropertyValue> aParams = {
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr,
                                      sal_Int16(css::document::UpdateDocMode::NO_UPDATE)),
    };
    loadWithParams(createFileURL(u"slide-background-link.fodp"), aParams);
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);

    // The slide's deferred remote background fill bitmap is registered as an
    // external link as the page is imported, so it appears in Edit, Links to
    // External Files and can be updated or broken.
    sfx2::LinkManager* pLinkMgr = pImpressDocument->GetDoc()->GetLinkManager();
    CPPUNIT_ASSERT(pLinkMgr);
    CPPUNIT_ASSERT_MESSAGE("slide background fill bitmap link should be registered",
                           !pLinkMgr->GetLinks().empty());

    pImpressDocument->initializeForTiledRendering({});

    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetOutputSizePixel(Size(1024, 768));
    pImpressDocument->paintTile(*pDevice, 1024, 768, 0, 0, 15360, 7680);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testMasterPageBackgroundRemoteNotFetched)
{
    // A master page background fill image with a remote URL must not fetch
    // the URL during paint when link updates are not allowed. The master
    // background lands on the master's presentation style sheet, not on the
    // page item set, so the link is registered through the SfxStyleSheet path
    // rather than the SdrPage one.
    uno::Sequence<beans::PropertyValue> aParams = {
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr,
                                      sal_Int16(css::document::UpdateDocMode::NO_UPDATE)),
    };
    loadWithParams(createFileURL(u"master-page-background-link.fodp"), aParams);
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);

    sfx2::LinkManager* pLinkMgr = pImpressDocument->GetDoc()->GetLinkManager();
    CPPUNIT_ASSERT(pLinkMgr);
    CPPUNIT_ASSERT_MESSAGE("master page background fill bitmap link should be registered",
                           !pLinkMgr->GetLinks().empty());

    pImpressDocument->initializeForTiledRendering({});

    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetOutputSizePixel(Size(1024, 768));
    pImpressDocument->paintTile(*pDevice, 1024, 768, 0, 0, 15360, 7680);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testBulletImageRemoteNotFetched)
{
    // text:list-level-style-image with a remote URL must not fetch
    // the URL during paint when link updates are not allowed.
    // Currently the editeng rendering path silently skips unresolved
    // GraphicExternalLink graphics (GraphicType::Default is not handled
    // by create2DDecompositionOfGraphic). If someone adds fetching here,
    // this test should catch it.
    uno::Sequence<beans::PropertyValue> aParams = {
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr,
                                      sal_Int16(css::document::UpdateDocMode::NO_UPDATE)),
    };
    loadWithParams(createFileURL(u"bullet-image-link.fodp"), aParams);
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);
    pImpressDocument->initializeForTiledRendering({});

    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetOutputSizePixel(Size(1024, 768));
    pImpressDocument->paintTile(*pDevice, 1024, 768, 0, 0, 15360, 7680);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testShapeFillRemoteNotFetched)
{
    // draw:fill-image with a remote URL on a shape must not fetch
    // the URL during paint when link updates are not allowed.
    // The assert in createNewSdrFillGraphicAttribute will fire if
    // a remote fetch is attempted.
    uno::Sequence<beans::PropertyValue> aParams = {
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr,
                                      sal_Int16(css::document::UpdateDocMode::NO_UPDATE)),
    };
    loadWithParams(createFileURL(u"shape-fill-link.fodp"), aParams);
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);
    pImpressDocument->initializeForTiledRendering({});

    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetOutputSizePixel(Size(1024, 768));
    pImpressDocument->paintTile(*pDevice, 1024, 768, 0, 0, 15360, 7680);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testFormImageRemoteNotFetched)
{
    // Form image button with a remote ImageURL must not fetch the
    // URL during import when link updates are not allowed.
    uno::Sequence<beans::PropertyValue> aParams = {
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr,
                                      sal_Int16(css::document::UpdateDocMode::NO_UPDATE)),
    };
    loadWithParams(createFileURL(u"form-image-link.fodp"), aParams);
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);
    pImpressDocument->initializeForTiledRendering({});

    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetOutputSizePixel(Size(1024, 768));
    pImpressDocument->paintTile(*pDevice, 1024, 768, 0, 0, 15360, 7680);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
