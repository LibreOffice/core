/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "sdmodeltestbase.hxx"
#include <sdpage.hxx>
#include <drawdoc.hxx>
#include <CustomAnimationEffect.hxx>
#include <anminfo.hxx>
#include <xmloff/SoundReference.hxx>

#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/XAudio.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>

#include <sfx2/linkmgr.hxx>
#include <sfx2/lnkbase.hxx>

using namespace css;

namespace
{
class SdExportTest2 : public SdModelTestBase
{
public:
    SdExportTest2()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

// The sound fixtures below contain nothing but their sound, so a registered
// entry in the link manager means that sound was recognised as an external
// link, the way the document tracks it.
bool lcl_hasSoundLink(SdDrawDocument& rDoc)
{
    const sfx2::LinkManager* pLinkManager = rDoc.GetLinkManager();
    return pLinkManager && !pLinkManager->GetLinks().empty();
}

SdAnimationInfo* lcl_getClickActionSoundInfo(SdDrawDocument& rDoc)
{
    for (sal_uInt16 nPage = 0; nPage < rDoc.GetSdPageCount(PageKind::Standard); ++nPage)
    {
        SdPage* pPage = rDoc.GetSdPage(nPage, PageKind::Standard);
        for (size_t nObj = 0; pPage && nObj < pPage->GetObjCount(); ++nObj)
        {
            SdrObject* pObj = pPage->GetObj(nObj);
            SdAnimationInfo* pInfo = pObj ? SdDrawDocument::GetShapeUserData(*pObj) : nullptr;
            if (pInfo && pInfo->meClickAction == css::presentation::ClickAction_SOUND)
                return pInfo;
        }
    }
    return nullptr;
}

css::uno::Reference<css::animations::XAnimationNode>
lcl_getPageAnimationNode(const css::uno::Reference<css::lang::XComponent>& xComponent,
                         sal_Int32 nPage)
{
    css::uno::Reference<css::drawing::XDrawPagesSupplier> xSupplier(xComponent,
                                                                    css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::drawing::XDrawPage> xPage(xSupplier->getDrawPages()->getByIndex(nPage),
                                                       css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::animations::XAnimationNodeSupplier> xNodeSupplier(
        xPage, css::uno::UNO_QUERY_THROW);
    return xNodeSupplier->getAnimationNode();
}

css::uno::Reference<css::animations::XAnimationNode>
lcl_findAudioNode(const css::uno::Reference<css::animations::XAnimationNode>& xNode)
{
    if (!xNode.is())
        return nullptr;
    if (xNode->getType() == css::animations::AnimationNodeType::AUDIO)
        return xNode;
    css::uno::Reference<css::container::XEnumerationAccess> xEnumAccess(xNode, css::uno::UNO_QUERY);
    if (xEnumAccess.is())
    {
        css::uno::Reference<css::container::XEnumeration> xEnum = xEnumAccess->createEnumeration();
        while (xEnum.is() && xEnum->hasMoreElements())
        {
            css::uno::Reference<css::animations::XAnimationNode> xChild(xEnum->nextElement(),
                                                                        css::uno::UNO_QUERY);
            if (auto xFound = lcl_findAudioNode(xChild))
                return xFound;
        }
    }
    return nullptr;
}
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testTransitionSoundLinkDetected)
{
    // A slide-transition sound that references an external file is recognised
    // as a link on load, so it takes part in the link-update decision.
    createSdImpressDoc("odp/transition-sound-link.fodp");
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testAnimationSoundLinkDetected)
{
    createSdImpressDoc("odp/animation-sound-link.fodp");
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testTransitionSoundLinkSurvivesODFRoundtrip)
{
    // The transition sound source must come back after an ODF save and reload,
    // not be dropped because the page "Sound" property now carries an object.
    createSdImpressDoc("odp/transition-sound-link.fodp");
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
    saveAndReload(TestFilter::ODP);
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testClickActionSoundLinkDetected)
{
    createSdImpressDoc("odp/clickaction-sound-link.fodp");
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testSoundLinkAllowedPerLink)
{
    // Allowing one external transition sound through link management marks that
    // page alone allowed and leaves the others blocked.
    createSdImpressDoc("odp/two-sound-links.fodp");
    SdDrawDocument* pDoc = getSdDocShell()->GetDoc();

    SdPage* pPage0 = pDoc->GetSdPage(0, PageKind::Standard);
    SdPage* pPage1 = pDoc->GetSdPage(1, PageKind::Standard);
    CPPUNIT_ASSERT(pPage0);
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(pPage0->GetSoundLink().isExternalLink());
    CPPUNIT_ASSERT(pPage1->GetSoundLink().isExternalLink());

    // both transition sounds are registered as links, so they appear in link
    // management as two separate entries
    sfx2::LinkManager* pLinkMgr = pDoc->GetLinkManager();
    CPPUNIT_ASSERT(pLinkMgr);
    const sfx2::SvBaseLinks& rLinks = pLinkMgr->GetLinks();
    CPPUNIT_ASSERT_EQUAL(size_t(2), rLinks.size());

    // neither page is allowed before the user acts
    CPPUNIT_ASSERT(!pPage0->GetSoundLink().isAllowed());
    CPPUNIT_ASSERT(!pPage1->GetSoundLink().isAllowed());

    // updating the link whose source is page 0's sound is the per-link allow
    // action; only page 0 ends up allowed
    for (const tools::SvRef<sfx2::SvBaseLink>& rLink : rLinks)
    {
        OUString aURL;
        sfx2::LinkManager::GetDisplayNames(rLink.get(), nullptr, &aURL);
        if (aURL == pPage0->GetSoundFile())
            rLink->Update();
    }
    CPPUNIT_ASSERT(pPage0->GetSoundLink().isAllowed());
    CPPUNIT_ASSERT(!pPage1->GetSoundLink().isAllowed());
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testClickActionSoundLinkAllowedPerLink)
{
    // Allowing the click-action sound through link management marks the shape's
    // own SdAnimationInfo allowed, not through a by-URL lookup.
    createSdImpressDoc("odp/clickaction-sound-link.fodp");
    SdDrawDocument* pDoc = getSdDocShell()->GetDoc();

    SdAnimationInfo* pInfo = lcl_getClickActionSoundInfo(*pDoc);
    CPPUNIT_ASSERT(pInfo);

    // the click sound is registered as a link that can be allowed on its own
    sfx2::LinkManager* pLinkMgr = pDoc->GetLinkManager();
    CPPUNIT_ASSERT(pLinkMgr);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pLinkMgr->GetLinks().size());

    // not allowed before the user acts
    CPPUNIT_ASSERT(!pInfo->mbClickSoundAllowed);

    // updating the link is the per-link allow action
    pLinkMgr->GetLinks()[0]->Update();
    CPPUNIT_ASSERT(pInfo->mbClickSoundAllowed);
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testTransitionSoundRemoteNotFetched)
{
    // A slide-transition sound pointing outside the document is detected on
    // load, so the link-update infobar is raised. Exporting to PPT without the
    // user allowing the document's links must not fetch the sound. The URL is
    // non-routable, so a fetch attempt would hang this test.
    createSdImpressDoc("odp/transition-sound-remote.fodp");
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
    saveAndReload(TestFilter::PPT);
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testAnimationSoundRemoteNotFetched)
{
    // An animation sound likewise: detected on load, not fetched when
    // exporting without allowing the document's links.
    createSdImpressDoc("odp/animation-sound-remote.fodp");
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
    saveAndReload(TestFilter::PPT);
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testClickActionSoundRemoteNotFetched)
{
    // A click-action sound likewise: detected on load, not fetched when
    // exporting without allowing the document's links.
    createSdImpressDoc("odp/clickaction-sound-remote.fodp");
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
    saveAndReload(TestFilter::PPT);
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testTransitionSoundRemoteNotFetchedOOXML)
{
    // The same must hold on OOXML export, not only binary PPT.
    createSdImpressDoc("odp/transition-sound-remote.fodp");
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
    saveAndReload(TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testAnimationSoundRemoteNotFetchedOOXML)
{
    createSdImpressDoc("odp/animation-sound-remote.fodp");
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
    saveAndReload(TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testAnimationSoundAllowedForPlayback)
{
    // Allowing the animation-effect sound through link management marks its own
    // audio node's source allowed, so the sound player only plays it once
    // allowed.
    createSdImpressDoc("odp/animation-sound-remote.fodp");
    SdDrawDocument* pDoc = getSdDocShell()->GetDoc();
    css::uno::Reference<css::animations::XAudio> xAudio(
        lcl_findAudioNode(lcl_getPageAnimationNode(mxComponent, 0)), css::uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAudio.is());

    // the external animation sound is registered as a link that can be allowed
    sfx2::LinkManager* pLinkMgr = pDoc->GetLinkManager();
    CPPUNIT_ASSERT(pLinkMgr);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pLinkMgr->GetLinks().size());

    // not allowed before the user acts
    CPPUNIT_ASSERT(!xmloff::getSoundAllowed(xAudio->getSource()));

    // updating the link marks the node's own source allowed
    pLinkMgr->GetLinks()[0]->Update();
    CPPUNIT_ASSERT(xmloff::getSoundAllowed(xAudio->getSource()));
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testAllowedSoundNotPersisted)
{
    // Allowing an external sound lasts for the session only. After a save and
    // reload the sound is not allowed and its link is registered again, so
    // each session asks the user anew.
    createSdImpressDoc("odp/animation-sound-remote.fodp");
    sfx2::LinkManager* pLinkMgr = getSdDocShell()->GetDoc()->GetLinkManager();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pLinkMgr->GetLinks().size());
    pLinkMgr->GetLinks()[0]->Update();
    {
        css::uno::Reference<css::animations::XAudio> xAudio(
            lcl_findAudioNode(lcl_getPageAnimationNode(mxComponent, 0)), css::uno::UNO_QUERY);
        CPPUNIT_ASSERT(xAudio.is());
        CPPUNIT_ASSERT(xmloff::getSoundAllowed(xAudio->getSource()));
    }

    saveAndReload(TestFilter::ODP);

    css::uno::Reference<css::animations::XAudio> xAudio(
        lcl_findAudioNode(lcl_getPageAnimationNode(mxComponent, 0)), css::uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAudio.is());
    CPPUNIT_ASSERT(!xmloff::getSoundAllowed(xAudio->getSource()));
    CPPUNIT_ASSERT(lcl_hasSoundLink(*getSdDocShell()->GetDoc()));
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testAnimationSoundInPackageAllowed)
{
    // A sound that lives inside the document package needs no allowing, so its
    // audio node plays without the user acting and it is not a link.
    createSdImpressDoc("odp/animation-sound-package.fodp");
    SdDrawDocument* pDoc = getSdDocShell()->GetDoc();
    css::uno::Reference<css::animations::XAudio> xAudio(
        lcl_findAudioNode(lcl_getPageAnimationNode(mxComponent, 0)), css::uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAudio.is());

    CPPUNIT_ASSERT(xmloff::getSoundAllowed(xAudio->getSource()));

    // a package sound is not an external reference, so it joins no link
    CPPUNIT_ASSERT(!lcl_hasSoundLink(*pDoc));
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testAnimationSoundExternalImportPPTX)
{
    // A PPTX animation sound whose relationship points outside the package is
    // imported as an external reference, joins link management like its ODF
    // counterpart, and plays only once allowed. The URL is non-routable, so a
    // fetch attempt would hang this test.
    createSdImpressDoc("pptx/animation-sound-external.pptx");
    SdDrawDocument* pDoc = getSdDocShell()->GetDoc();
    css::uno::Reference<css::animations::XAudio> xAudio(
        lcl_findAudioNode(lcl_getPageAnimationNode(mxComponent, 0)), css::uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAudio.is());

    sfx2::LinkManager* pLinkMgr = pDoc->GetLinkManager();
    CPPUNIT_ASSERT(pLinkMgr);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pLinkMgr->GetLinks().size());

    // not allowed before the user acts
    CPPUNIT_ASSERT(!xmloff::getSoundAllowed(xAudio->getSource()));

    // updating the link marks the node's own source allowed
    pLinkMgr->GetLinks()[0]->Update();
    CPPUNIT_ASSERT(xmloff::getSoundAllowed(xAudio->getSource()));
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testAnimationSoundEmbeddedImportPPTX)
{
    // A PPTX animation sound embedded in the package becomes package content,
    // so it needs no allowing and is not a link.
    createSdImpressDoc("pptx/animation-sound-embedded.pptx");
    SdDrawDocument* pDoc = getSdDocShell()->GetDoc();
    css::uno::Reference<css::animations::XAudio> xAudio(
        lcl_findAudioNode(lcl_getPageAnimationNode(mxComponent, 0)), css::uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAudio.is());

    CPPUNIT_ASSERT(xmloff::getSoundAllowed(xAudio->getSource()));
    CPPUNIT_ASSERT(!lcl_hasSoundLink(*pDoc));
}

CPPUNIT_TEST_FIXTURE(SdExportTest2, testLegacyAnimationSoundNotFetched)
{
    // A legacy animation-effect sound, set through the deprecated shape Sound
    // property the way binary PPT import and the old API populate it, migrates
    // into an audio node whose source is not allowed. So exporting without
    // allowing the links leaves it alone. The URL is non-routable, a fetch
    // attempt would hang.
    createSdImpressDoc();

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShapes> xShapes(xDrawPage, uno::UNO_QUERY_THROW);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.TextShape"_ustr), uno::UNO_QUERY_THROW);
    xShapes->add(xShape);

    // an animation effect gives the legacy sound an effect to migrate onto
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY_THROW);
    xShapeProps->setPropertyValue(u"Effect"_ustr,
                                  uno::Any(presentation::AnimationEffect_FADE_FROM_LEFT));
    xShapeProps->setPropertyValue(u"SoundOn"_ustr, uno::Any(true));
    xShapeProps->setPropertyValue(u"Sound"_ustr,
                                  uno::Any(u"http://192.0.2.1:12345/animation.wav"_ustr));

    // setting the effect schedules the animation tree on a debounced rebuild
    // timer. Force the export-able representation now, the way the document
    // load path does, so the result does not depend on timer timing.
    SdDrawDocument* pDoc = getSdDocShell()->GetDoc();
    pDoc->GetSdPage(0, PageKind::Standard)->getMainSequence()->getRootNode();

    uno::Reference<css::animations::XAnimationNodeSupplier> xNodeSupplier(xDrawPage,
                                                                          uno::UNO_QUERY_THROW);
    uno::Reference<css::animations::XAudio> xAudio(
        lcl_findAudioNode(xNodeSupplier->getAnimationNode()), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAudio.is());

    // the migrated sound is not allowed, so playback and export leave it alone
    // until the user allows it
    CPPUNIT_ASSERT(!xmloff::getSoundAllowed(xAudio->getSource()));

    saveAndReload(TestFilter::PPT);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
