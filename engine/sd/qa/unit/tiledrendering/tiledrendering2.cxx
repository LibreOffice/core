/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_pdfimport.h>
#include <unotools/tempfile.hxx>
#include <sdtiledrenderingtest.hxx>

#include <sfx2/sidebar/Sidebar.hxx>
#include <vcl/scheduler.hxx>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <rtl/ustrbuf.hxx>
#include <svl/cryptosign.hxx>
#include <svl/undo.hxx>
#include <svx/svdotext.hxx>
#include <sfx2/linkmgr.hxx>
#include <tools/json_writer.hxx>

#include <vcl/virdev.hxx>

#include <tools/UnitConversion.hxx>

#include <DrawDocShell.hxx>
#include <SlideSectionManager.hxx>
#include <View.hxx>
#include <ViewShell.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
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
    cpo::uno::Sequence<beans::PropertyValue> aArgs
        = { comphelper::makePropertyValue(u"ReadOnly"_ustr, true) };
    createTempCopy(u"empty.pdf");
    loadWithParams(maTempFile.GetURL(), aArgs);
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    pImpressDocument->initializeForTiledRendering({});
    sd::ViewShell* pViewShell = pImpressDocument->GetDocShell()->GetViewShell();
    sd::View* pView = pViewShell->GetView();
    pViewShell->GetViewShell()->SetKitAuthor(u"myauthor"_ustr);
    SdTestViewCallback aView;

    // When inserting a signature line for electronic (external) signing:
    aArgs = {
        comphelper::makePropertyValue(u"External"_ustr, true),
    };
    // Without the accompanying fix in place, this test would hang here in the certificate chooser
    // dialog.
    dispatchCommand(mxComponent, u".uno:InsertSignatureLine"_ustr, aArgs);
    // Signature line is selected right after inserting:
    CPPUNIT_ASSERT(pViewShell->GetViewShell()->GetSignPDFCertificate().Is());

    // Then make sure the shape is marked as a signature line:
    std::vector<SdrObject*> aMarkedObjects = pView->GetMarkedObjects();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aMarkedObjects.size());
    cpo::uno::Any aAny;
    aMarkedObjects[0]->GetGrabBagItem(aAny);
    comphelper::SequenceAsHashMap aMap(aAny);
    CPPUNIT_ASSERT(aMap.contains(u"SignatureCertificate"_ustr));
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
    dispatchCommand(mxComponent, u".uno:NotesMode"_ustr,
                    cpo::uno::Sequence<beans::PropertyValue>());

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
    cpo::uno::Sequence<beans::PropertyValue> aParams = {
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
    cpo::uno::Sequence<beans::PropertyValue> aParams = {
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
    cpo::uno::Sequence<beans::PropertyValue> aParams = {
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
    cpo::uno::Sequence<beans::PropertyValue> aParams = {
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
    cpo::uno::Sequence<beans::PropertyValue> aParams = {
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

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testInsertChartAtClientVisibleCenter)
{
    // Given a Draw document, and a client looking at the lower part of the page:
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odg");
    const ::tools::Rectangle aVisibleArea(Point(1000, 6000), Size(9000, 4000));
    pXImpressDocument->setClientVisibleArea(aVisibleArea);

    // When inserting a chart:
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    dispatchCommand(mxComponent, u".uno:InsertObjectChart"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    // Then it lands where the client looks. Without the accompanying fix in place,
    // this test would have failed: the chart was centred on the kit window's own
    // geometry, a fixed spot of the page outside the client's view.
    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
    const ::tools::Rectangle aObject = pPage->GetObj(0)->GetLogicRect();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(double(convertTwipToMm100(aVisibleArea.Center().X())),
                                 double(aObject.Center().X()), 2.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(double(convertTwipToMm100(aVisibleArea.Center().Y())),
                                 double(aObject.Center().Y()), 2.0);
}

// The slide-import fixtures live in the shared sd data directory, next to
// the ones the misc-tests suite uses, rather than being copied per suite.
constexpr OUString gSlideImportDataDir = u"/sd/qa/unit/data/"_ustr;

namespace
{
/// The links rDocument reports, parsed.
boost::property_tree::ptree readLinks(SdXImpressDocument& rDocument)
{
    tools::JsonWriter aJsonWriter;
    const bool bWritten = rDocument.getSlideLinks(aJsonWriter);
    const OString aLinks = aJsonWriter.finishAndGetAsOString();
    CPPUNIT_ASSERT(bWritten);
    std::stringstream aStream((std::string(aLinks)));
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    return aTree;
}

/// The text the slide at nIndex of rDoc shows, taking its shapes in order.
OUString getSlideText(SdDrawDocument& rDoc, sal_uInt16 nIndex)
{
    SdPage* pPage = rDoc.GetSdPage(nIndex, PageKind::Standard);
    CPPUNIT_ASSERT(pPage);
    OUStringBuffer aText;
    for (size_t nObject = 0; nObject < pPage->GetObjCount(); ++nObject)
    {
        auto* pTextObject = dynamic_cast<SdrTextObj*>(pPage->GetObj(nObject));
        if (pTextObject && !pTextObject->IsEmptyPresObj() && pTextObject->GetOutlinerParaObject())
            aText.append(pTextObject->GetOutlinerParaObject()->GetTextObject().GetText(0));
    }
    return aText.makeStringAndClear();
}
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkList)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // A document whose slides are all its own reports no links at all.
    CPPUNIT_ASSERT(readLinks(*pXImpressDocument).get_child("links").empty());

    // Two slides of one source and one of another are inserted as links, and one slide is inserted
    // as a plain copy.
    const OUString aFirstUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aFirstUrl, "{\"slides\":[0,1],\"at\":0,\"link\":true,\"source\":\"Q3 #1 100%.odp\"}"_ostr));
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aFirstUrl, "{\"slides\":[0],\"at\":0,\"source\":\"Q3 #1 100%.odp\"}"_ostr));

    const OUString aSecondUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-sections.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSecondUrl, "{\"slides\":[0],\"link\":true,\"source\":\"Support deck.odp\"}"_ostr));

    // Each source is reported once, under the name the pages were linked to, with the pages it
    // holds in document order. The plain copy belongs to no source and is left out.
    const boost::property_tree::ptree aLinks = readLinks(*pXImpressDocument);
    std::vector<boost::property_tree::ptree> aSources;
    for (const auto& rSource : aLinks.get_child("links"))
        aSources.push_back(rSource.second);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aSources.size());
    CPPUNIT_ASSERT_EQUAL(std::string("Q3 #1 100%.odp"), aSources[0].get<std::string>("source"));
    CPPUNIT_ASSERT_EQUAL(std::string("Support deck.odp"), aSources[1].get<std::string>("source"));

    std::vector<boost::property_tree::ptree> aSlides;
    for (const auto& rSlide : aSources[0].get_child("slides"))
        aSlides.push_back(rSlide.second);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aSlides.size());
    CPPUNIT_ASSERT_EQUAL(std::string("SourceA"), aSlides[0].get<std::string>("name"));
    CPPUNIT_ASSERT_EQUAL(std::string("TargetOne"), aSlides[1].get<std::string>("name"));

    // A page is named by the identifier it keeps for the whole session, not by its position. The
    // plain copy landed before the linked pages, so the first of them is the second slide.
    const OString aPartId = pDoc->GetSdPage(1, PageKind::Standard)->GetGuid().getString();
    CPPUNIT_ASSERT_EQUAL(std::string(aPartId.getStr(), aPartId.getLength()),
                         aSlides[0].get<std::string>("part"));
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideImportLink)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // The file the slides are read from is one thing and the source document
    // they belong to is another: here the file stands in for a document the
    // user knows by a name that carries characters a reference has to escape.
    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp");

    // A slide inserted as a link reports the source document, the slide of it that it came
    // from, and the time that source was last modified.
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0],\"at\":1,\"keepDesign\":false,\"link\":true,"
                    "\"source\":\"Q3 #1 100%.odp\",\"lastModifiedTime\":\"2020-09-13T12:26:40Z\"}"_ostr));
    SdPage* pLinked = pDoc->GetSdPage(1, PageKind::Standard);
    CPPUNIT_ASSERT(pLinked);
    CPPUNIT_ASSERT_EQUAL(u"vnd.collabora.slide-source:Q3%20%231%20100%25.odp"_ustr,
                         pLinked->GetFileName());
    CPPUNIT_ASSERT_EQUAL(u"SourceA"_ustr, pLinked->GetBookmarkName());
    CPPUNIT_ASSERT_EQUAL(u"2020-09-13T12:26:40Z"_ustr, pLinked->GetSourceModifiedTime());

    // A slide inserted from the same file without asking for a link is a plain copy, and
    // names no source and no modified time.
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl,
        "{\"slides\":[0],\"at\":2,\"keepDesign\":false,\"source\":\"Q3 #1 100%.odp\"}"_ostr));
    SdPage* pCopied = pDoc->GetSdPage(2, PageKind::Standard);
    CPPUNIT_ASSERT(pCopied);
    CPPUNIT_ASSERT_EQUAL(OUString(), pCopied->GetFileName());
    CPPUNIT_ASSERT_EQUAL(OUString(), pCopied->GetBookmarkName());
    CPPUNIT_ASSERT_EQUAL(OUString(), pCopied->GetSourceModifiedTime());

    // An insert that names no source document has nothing for a link to record, so one that
    // asks for links inserts nothing at all.
    const int nPartsBefore = pXImpressDocument->getParts();
    CPPUNIT_ASSERT(
        !pXImpressDocument->insertPagesFromFile(aSourceUrl, "{\"slides\":[0],\"link\":true}"_ostr));
    CPPUNIT_ASSERT_EQUAL(nPartsBefore, pXImpressDocument->getParts());

    // A name holding a path names a location rather than a document, so an insert that asks
    // for links to it is in the same position as one that names no source at all.
    CPPUNIT_ASSERT(!pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0],\"link\":true,\"source\":\"/tmp/staged/Q3.odp\"}"_ostr));
    CPPUNIT_ASSERT_EQUAL(nPartsBefore, pXImpressDocument->getParts());
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkModifiedTimeRoundtrip)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0],\"at\":1,\"link\":true,\"source\":\"Q3 deck.odp\","
                    "\"lastModifiedTime\":\"2020-09-13T12:26:40Z\"}"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"2020-09-13T12:26:40Z"_ustr,
                         pDoc->GetSdPage(1, PageKind::Standard)->GetSourceModifiedTime());

    // The time the source was last modified survives a save to ODF and the load back.
    saveAndReload(TestFilter::ODP);
    SdXImpressDocument* pReloaded = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pReloaded);
    SdDrawDocument* pReloadedDoc = pReloaded->GetDoc();
    CPPUNIT_ASSERT_EQUAL(u"2020-09-13T12:26:40Z"_ustr,
                         pReloadedDoc->GetSdPage(1, PageKind::Standard)->GetSourceModifiedTime());
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkRefresh)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // One slide of the source deck is inserted as a link and the same slide as a plain copy.
    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0],\"at\":1,\"link\":true,\"source\":\"Q3 deck.odp\"}"_ostr));
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0],\"at\":2,\"source\":\"Q3 deck.odp\"}"_ostr));

    // The linked slide is given a name of its own, so that keeping the name of the slide in this
    // document rather than the name of the slide it came from is what the refresh has to do.
    pDoc->GetSdPage(1, PageKind::Standard)->SetName(u"Q3 numbers"_ustr);
    const int nParts = pXImpressDocument->getParts();
    CPPUNIT_ASSERT_EQUAL(u"Source title"_ustr, getSlideText(*pDoc, 1));

    SfxUndoManager* pUndoManager = pDoc->GetDocSh()->GetUndoManager();
    const size_t nUndoActions = pUndoManager->GetUndoActionCount();

    // The source deck changed, and the file staged for the refresh holds the changed slides.
    const OUString aChangedUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-link-source-changed.odp");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         pXImpressDocument->refreshSlideLinks(u"Q3 deck.odp"_ustr, aChangedUrl));

    // The linked slide shows the new content, and it kept its position, its name and its link.
    CPPUNIT_ASSERT_EQUAL(u"Source title, revised"_ustr, getSlideText(*pDoc, 1));
    CPPUNIT_ASSERT_EQUAL(nParts, pXImpressDocument->getParts());
    SdPage* pLinked = pDoc->GetSdPage(1, PageKind::Standard);
    CPPUNIT_ASSERT(pLinked);
    CPPUNIT_ASSERT_EQUAL(u"Q3 numbers"_ustr, pLinked->GetName());
    CPPUNIT_ASSERT_EQUAL(u"vnd.collabora.slide-source:Q3%20deck.odp"_ustr, pLinked->GetFileName());
    CPPUNIT_ASSERT_EQUAL(u"SourceA"_ustr, pLinked->GetBookmarkName());

    // The plain copy of the same slide belongs to no source and keeps what it holds.
    CPPUNIT_ASSERT_EQUAL(u"Source title"_ustr, getSlideText(*pDoc, 2));

    // The whole refresh is one undo step, and undoing it brings the old content back with the link.
    CPPUNIT_ASSERT_EQUAL(nUndoActions + 1, pUndoManager->GetUndoActionCount());
    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL(u"Source title"_ustr, getSlideText(*pDoc, 1));
    CPPUNIT_ASSERT_EQUAL(nParts, pXImpressDocument->getParts());
    pLinked = pDoc->GetSdPage(1, PageKind::Standard);
    CPPUNIT_ASSERT(pLinked);
    CPPUNIT_ASSERT_EQUAL(u"Q3 numbers"_ustr, pLinked->GetName());
    CPPUNIT_ASSERT_EQUAL(u"vnd.collabora.slide-source:Q3%20deck.odp"_ustr, pLinked->GetFileName());
    CPPUNIT_ASSERT_EQUAL(u"SourceA"_ustr, pLinked->GetBookmarkName());

    // A source no page of this document is linked to refreshes nothing at all.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         pXImpressDocument->refreshSlideLinks(u"Other deck.odp"_ustr, aChangedUrl));

    // The slides are read from a local file, so a location that would have to be fetched is
    // refused.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         pXImpressDocument->refreshSlideLinks(
                             u"Q3 deck.odp"_ustr, u"https://example.com/deck.odp"_ustr));

    // A file that holds no slide of the recorded name leaves the linked slide as it is, and a
    // refresh that changes nothing is no undo step of its own.
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        pXImpressDocument->refreshSlideLinks(
            u"Q3 deck.odp"_ustr,
            m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp")));
    CPPUNIT_ASSERT_EQUAL(u"Source title"_ustr, getSlideText(*pDoc, 1));
    CPPUNIT_ASSERT_EQUAL(nUndoActions, pUndoManager->GetUndoActionCount());
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkRefreshKeepsSections)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // Two slides of one source are inserted as links, so a refresh reads them as one run of pages.
    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0,1],\"at\":0,\"link\":true,\"source\":\"Q3 deck.odp\"}"_ostr));

    // A section starts on each of the two linked slides.
    sd::SlideSectionManager& rSections = pDoc->GetSectionManager();
    rSections.AddSection(0, u"Opening"_ustr);
    rSections.AddSection(1, u"Details"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rSections.GetSectionCount());

    // Both linked slides are refreshed together, and every section still starts on the slide it
    // did.
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(2),
        pXImpressDocument->refreshSlideLinks(
            u"Q3 deck.odp"_ustr,
            m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-link-source-changed.odp")));
    CPPUNIT_ASSERT_EQUAL(u"Source title, revised"_ustr, getSlideText(*pDoc, 0));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rSections.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"Opening"_ustr, rSections.GetSection(0).maName);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rSections.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(u"Details"_ustr, rSections.GetSection(1).maName);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rSections.GetSection(1).mnStartIndex);
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkSourceNotFetched)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0],\"at\":1,\"link\":true,\"source\":\"Q3 deck.odp\"}"_ostr));

    // A page holds its link with the sfx2 link manager once the document is loaded again, so the
    // test registers it here to reach the same state in one session.
    pDoc->GetSdPage(1, PageKind::Standard)->ConnectLink();
    sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();
    CPPUNIT_ASSERT(pLinkManager);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pLinkManager->GetLinks().size());

    // Updating the links reads the file staged for each source document, so a source that has no
    // staged file leaves its page the content it holds, with its link.
    pLinkManager->UpdateAllLinks(/*bAskUpdate=*/false, u""_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Source title"_ustr, getSlideText(*pDoc, 1));
    SdPage* pLinked = pDoc->GetSdPage(1, PageKind::Standard);
    CPPUNIT_ASSERT(pLinked);
    CPPUNIT_ASSERT_EQUAL(u"vnd.collabora.slide-source:Q3%20deck.odp"_ustr, pLinked->GetFileName());
    CPPUNIT_ASSERT_EQUAL(u"SourceA"_ustr, pLinked->GetBookmarkName());
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkBreak)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // Two slides of one source are inserted as links, so that taking the source off one of them
    // leaves the other linked.
    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0,1],\"at\":0,\"link\":true,\"source\":\"Q3 deck.odp\"}"_ostr));

    SfxUndoManager* pUndoManager = pDoc->GetDocSh()->GetUndoManager();
    const size_t nUndoActions = pUndoManager->GetUndoActionCount();

    // The first of the two keeps the content it holds and belongs to no source any more, and the
    // second is still linked to the source both came from.
    CPPUNIT_ASSERT(pXImpressDocument->breakSlideLink(0));
    CPPUNIT_ASSERT_EQUAL(u"Source title"_ustr, getSlideText(*pDoc, 0));
    SdPage* pBroken = pDoc->GetSdPage(0, PageKind::Standard);
    CPPUNIT_ASSERT(pBroken);
    CPPUNIT_ASSERT_EQUAL(OUString(), pBroken->GetFileName());
    CPPUNIT_ASSERT_EQUAL(OUString(), pBroken->GetBookmarkName());
    CPPUNIT_ASSERT_EQUAL(nUndoActions + 1, pUndoManager->GetUndoActionCount());

    const boost::property_tree::ptree aLinks = readLinks(*pXImpressDocument);
    std::vector<boost::property_tree::ptree> aSlides;
    for (const auto& rSource : aLinks.get_child("links"))
        for (const auto& rSlide : rSource.second.get_child("slides"))
            aSlides.push_back(rSlide.second);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aSlides.size());
    CPPUNIT_ASSERT_EQUAL(std::string("TargetOne"), aSlides[0].get<std::string>("name"));

    // A page that belongs to no source, and an index that names no page at all, leave the document
    // as it is.
    CPPUNIT_ASSERT(!pXImpressDocument->breakSlideLink(0));
    CPPUNIT_ASSERT(!pXImpressDocument->breakSlideLink(-1));
    CPPUNIT_ASSERT(!pXImpressDocument->breakSlideLink(pXImpressDocument->getParts()));

    // A refresh of the source reads the slide of the page that is still linked and leaves the other
    // one alone.
    const OUString aChangedUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-link-source-changed.odp");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         pXImpressDocument->refreshSlideLinks(u"Q3 deck.odp"_ustr, aChangedUrl));
    CPPUNIT_ASSERT_EQUAL(u"Source title"_ustr, getSlideText(*pDoc, 0));

    // Undoing the refresh and then the break gives the page the source and the slide it recorded
    // back.
    pUndoManager->Undo();
    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL(nUndoActions, pUndoManager->GetUndoActionCount());
    pBroken = pDoc->GetSdPage(0, PageKind::Standard);
    CPPUNIT_ASSERT(pBroken);
    CPPUNIT_ASSERT_EQUAL(u"vnd.collabora.slide-source:Q3%20deck.odp"_ustr, pBroken->GetFileName());
    CPPUNIT_ASSERT_EQUAL(u"SourceA"_ustr, pBroken->GetBookmarkName());
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testSlideLinkBreakOnEdit)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-target.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // One slide of the source deck is inserted as a link, and the whole insertion leaves it linked.
    const OUString aSourceUrl
        = m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp");
    CPPUNIT_ASSERT(pXImpressDocument->insertPagesFromFile(
        aSourceUrl, "{\"slides\":[0],\"at\":1,\"link\":true,\"source\":\"Q3 deck.odp\"}"_ostr));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1),
                         readLinks(*pXImpressDocument).get_child("links").size());

    SfxUndoManager* pUndoManager = pDoc->GetDocSh()->GetUndoManager();
    const size_t nUndoActions = pUndoManager->GetUndoActionCount();

    sd::ViewShell* pViewShell = pDoc->GetDocSh()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);
    sd::View* pView = pViewShell->GetView();
    SdPage* pLinked = pDoc->GetSdPage(1, PageKind::Standard);
    CPPUNIT_ASSERT(pLinked);
    CPPUNIT_ASSERT(pLinked->GetObjCount() > 0);
    SdrObject* pObject = pLinked->GetObj(0);

    // A change with no undo step in hand is the engine's own work on the page, the shape a link
    // update or an embedded object gives it, so the page keeps its source.
    pObject->Move(Size(100, 100));
    CPPUNIT_ASSERT_EQUAL(u"vnd.collabora.slide-source:Q3%20deck.odp"_ustr, pLinked->GetFileName());
    CPPUNIT_ASSERT_EQUAL(nUndoActions, pUndoManager->GetUndoActionCount());

    // A shape of the linked slide is moved by somebody editing it.
    pXImpressDocument->setPart(1);
    const Point aStart = pObject->GetSnapRect().TopLeft();
    pView->MarkObj(pObject, pView->GetSdrPageView());
    pView->MoveMarkedObj(Size(500, 500));

    // The slide became this document's own, and a refresh of the source it came from has no page
    // left to read.
    CPPUNIT_ASSERT_EQUAL(OUString(), pLinked->GetFileName());
    CPPUNIT_ASSERT_EQUAL(OUString(), pLinked->GetBookmarkName());
    CPPUNIT_ASSERT(readLinks(*pXImpressDocument).get_child("links").empty());
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(-1),
        pXImpressDocument->refreshSlideLinks(
            u"Q3 deck.odp"_ustr,
            m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-link-source-changed.odp")));

    // The edit and the source it took off are one undo step, so one undo puts the shape back where
    // it was and the slide back with its source.
    CPPUNIT_ASSERT_EQUAL(nUndoActions + 1, pUndoManager->GetUndoActionCount());
    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL(nUndoActions, pUndoManager->GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(aStart, pObject->GetSnapRect().TopLeft());
    CPPUNIT_ASSERT_EQUAL(u"vnd.collabora.slide-source:Q3%20deck.odp"_ustr, pLinked->GetFileName());
    CPPUNIT_ASSERT_EQUAL(u"SourceA"_ustr, pLinked->GetBookmarkName());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1),
                         readLinks(*pXImpressDocument).get_child("links").size());

    // A page linked to a file by its path belongs to the older linked-page feature, which no
    // refresh of a source document reaches, so an edit of it leaves what it records alone.
    SdPage* pPathLinked = pDoc->GetSdPage(0, PageKind::Standard);
    CPPUNIT_ASSERT(pPathLinked);
    CPPUNIT_ASSERT(pPathLinked->GetObjCount() > 0);
    pPathLinked->SetFileName(u"file:///decks/q3.odp"_ustr);
    pPathLinked->SetBookmarkName(u"Slide 1"_ustr);
    pXImpressDocument->setPart(0);
    pView->MarkObj(pPathLinked->GetObj(0), pView->GetSdrPageView());
    pView->MoveMarkedObj(Size(500, 500));
    CPPUNIT_ASSERT_EQUAL(u"file:///decks/q3.odp"_ustr, pPathLinked->GetFileName());
    CPPUNIT_ASSERT_EQUAL(u"Slide 1"_ustr, pPathLinked->GetBookmarkName());
}

CPPUNIT_TEST_FIXTURE(SdTiledRenderingTest, testExportPages)
{
    loadFromURL(m_directories.getURLFromSrc(gSlideImportDataDir, u"slide-import-source.odp"));
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->initializeForTiledRendering({});
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    const sal_uInt16 nPages = pDoc->GetSdPageCount(PageKind::Standard);
    CPPUNIT_ASSERT(nPages > 1);

    // Two of the pages are written out as a presentation of their own.
    utl::TempFileNamed aWritten(u"", true, u".odp");
    aWritten.EnableKillingFile();
    CPPUNIT_ASSERT(pXImpressDocument->exportPages({ 1, 0 }, aWritten.GetURL()));

    // The document that was written holds those pages, in the order they were asked for, and
    // the document they came from is as it was.
    CPPUNIT_ASSERT_EQUAL(nPages, pDoc->GetSdPageCount(PageKind::Standard));

    uno::Reference<lang::XComponent> xWritten(loadFromDesktop(aWritten.GetURL()));
    SdXImpressDocument* pWrittenDocument = dynamic_cast<SdXImpressDocument*>(xWritten.get());
    CPPUNIT_ASSERT(pWrittenDocument);
    SdDrawDocument* pWritten = pWrittenDocument->GetDoc();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2), pWritten->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT_EQUAL(getSlideText(*pDoc, 1), getSlideText(*pWritten, 0));
    CPPUNIT_ASSERT_EQUAL(getSlideText(*pDoc, 0), getSlideText(*pWritten, 1));

    // A slide is a standard page and the notes page that belongs to it, so the written
    // document holds one notes page per slide and each slide finds its own.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2), pWritten->GetSdPageCount(PageKind::Notes));
    for (sal_uInt16 nPage = 0; nPage < 2; ++nPage)
    {
        const SdPage* pSlide = pWritten->GetSdPage(nPage, PageKind::Standard);
        const SdPage* pNotes = pWritten->GetSdPage(nPage, PageKind::Notes);
        CPPUNIT_ASSERT(pSlide);
        CPPUNIT_ASSERT(pNotes);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(pSlide->GetPageNum() + 1),
                             pNotes->GetPageNum());
    }

    xWritten->dispose();

    // A page the document does not hold is written by nobody.
    utl::TempFileNamed aRefused(u"", true, u".odp");
    aRefused.EnableKillingFile();
    CPPUNIT_ASSERT(!pXImpressDocument->exportPages({ 0, nPages }, aRefused.GetURL()));

    // An empty list writes every page.
    utl::TempFileNamed aWhole(u"", true, u".odp");
    aWhole.EnableKillingFile();
    CPPUNIT_ASSERT(pXImpressDocument->exportPages({}, aWhole.GetURL()));
    uno::Reference<lang::XComponent> xWhole(loadFromDesktop(aWhole.GetURL()));
    SdXImpressDocument* pWholeDocument = dynamic_cast<SdXImpressDocument*>(xWhole.get());
    CPPUNIT_ASSERT(pWholeDocument);
    CPPUNIT_ASSERT_EQUAL(nPages, pWholeDocument->GetDoc()->GetSdPageCount(PageKind::Standard));
    xWhole->dispose();
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
