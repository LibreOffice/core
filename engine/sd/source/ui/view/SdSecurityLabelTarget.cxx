/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SdSecurityLabelTarget.hxx>

#include <DrawDocShell.hxx>
#include <DrawViewShell.hxx>
#include <ViewShellBase.hxx>
#include <Outliner.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>

#include <com/sun/star/frame/XModel.hpp>

#include <COKit/COKit.hxx>
#include <comphelper/kit.hxx>
#include <comphelper/scopeguard.hxx>
#include <editeng/editeng.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdotext.hxx>
#include <tools/json_writer.hxx>

#include <algorithm>
#include <set>

using namespace css;

namespace
{
// Apply rSettings-derived footer text/visibility to every standard slide.
void setFooterOnAllSlides(SdDrawDocument* pDoc, const OUString& rText, bool bVisible)
{
    if (!pDoc)
        return;

    const sal_uInt16 nCount = pDoc->GetSdPageCount(PageKind::Standard);
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        SdPage* pPage = pDoc->GetSdPage(i, PageKind::Standard);
        if (!pPage)
            continue;

        sd::HeaderFooterSettings aSettings = pPage->getHeaderFooterSettings();
        aSettings.mbFooterVisible = bVisible;
        aSettings.maFooterText = rText;
        pPage->setHeaderFooterSettings(aSettings);
    }
}

// Grow the footer placeholder of pMaster so the footer text, as it expands on pSlide, fits on
// one line. The box only wraps (growing upwards) once it spans the full usable page width.
// The box is kept horizontally centred on its old position and its bottom edge stays put.
void fitFooterToText(SdDrawDocument& rDoc, SdPage& rMaster, const SdPage& rSlide)
{
    auto pFooter = dynamic_cast<SdrTextObj*>(rMaster.GetPresObj(PresObjKind::Footer));
    if (!pFooter)
        return;

    const OutlinerParaObject* pParaObj = pFooter->GetOutlinerParaObject();
    if (!pParaObj)
        return;

    SdOutliner* pOutliner = rDoc.GetInternalOutliner();
    const OutlinerMode eOutlinerMode = pOutliner->GetOutlinerMode();
    comphelper::ScopeGuard const aOutlinerGuard([pOutliner, eOutlinerMode]() {
        pOutliner->setVisualizedPage(nullptr);
        pOutliner->Clear();
        pOutliner->Init(eOutlinerMode);
    });

    pOutliner->Init(OutlinerMode::TextObject);
    // The footer field expands to the text of the visualized slide.
    pOutliner->setVisualizedPage(&rSlide);
    pOutliner->SetText(*pParaObj);
    pOutliner->UpdateFields();

    // EditEngine can't word-wrap inside a field, so measure its expanded text as plain text,
    // keeping the attributes of the field it replaces.
    EditEngine& rEditEngine = pOutliner->GetEditEngine();
    for (sal_Int32 nPara = 0; nPara < rEditEngine.GetParagraphCount(); ++nPara)
        pOutliner->QuickInsertText(rEditEngine.GetText(nPara),
                                   ESelection(nPara, 0, nPara, rEditEngine.GetTextLen(nPara)));

    const tools::Long nHorzDist = pFooter->GetTextLeftDistance() + pFooter->GetTextRightDistance();
    const tools::Long nVertDist = pFooter->GetTextUpperDistance() + pFooter->GetTextLowerDistance();

    const tools::Rectangle aOldRect = pFooter->GetLogicRect();
    const Size aPageSize = rMaster.GetSize();
    const tools::Long nPageLeft = rMaster.GetLeftBorder();
    const tools::Long nPageRight = aPageSize.Width() - rMaster.GetRightBorder();
    const tools::Long nMaxWidth = nPageRight - nPageLeft;
    if (nMaxWidth <= nHorzDist)
        return;

    // Measure unwrapped first; only wrap when the single line can't fit the page.
    pOutliner->SetPaperSize(Size(SAL_MAX_INT32 / 2, SAL_MAX_INT32 / 2));
    pOutliner->SetUpdateLayout(true);
    Size aTextSize = pOutliner->CalcTextSize();
    tools::Long nWidth = std::max(aOldRect.GetWidth(), aTextSize.Width() + nHorzDist);
    if (nWidth > nMaxWidth)
    {
        nWidth = nMaxWidth;
        pOutliner->SetPaperSize(Size(nMaxWidth - nHorzDist, SAL_MAX_INT32 / 2));
        aTextSize = pOutliner->CalcTextSize();
    }
    pOutliner->SetUpdateLayout(false);

    tools::Long nHeight = std::max(aOldRect.GetHeight(), aTextSize.Height() + nVertDist);
    if (nWidth == aOldRect.GetWidth() && nHeight == aOldRect.GetHeight())
        return;

    // Keep the old centre, but don't push the box off either side of the page.
    tools::Long nLeft = aOldRect.Center().X() - nWidth / 2;
    nLeft = std::clamp(nLeft, nPageLeft, nPageRight - nWidth);
    const tools::Long nBottom = std::min(aOldRect.Bottom(), aPageSize.Height() - 1);
    nHeight = std::min(nHeight, nBottom + 1);

    pFooter->SetLogicRect(
        tools::Rectangle(Point(nLeft, nBottom - nHeight + 1), Size(nWidth, nHeight)));
}

// Fit the footer placeholder of every master used by a standard slide to the footer text.
void fitFootersToText(SdDrawDocument* pDoc)
{
    if (!pDoc)
        return;

    std::set<SdrPage*> aDoneMasters;
    const sal_uInt16 nCount = pDoc->GetSdPageCount(PageKind::Standard);
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        SdPage* pPage = pDoc->GetSdPage(i, PageKind::Standard);
        if (!pPage || !pPage->TRG_HasMasterPage())
            continue;

        auto pMaster = dynamic_cast<SdPage*>(&pPage->TRG_GetMasterPage());
        if (pMaster && aDoneMasters.insert(pMaster).second)
            fitFooterToText(*pDoc, *pMaster, *pPage);
    }
}
}

cpo::uno::Reference<frame::XModel> SdSecurityLabelTarget::getModel() const
{
    sd::DrawDocShell* pDocShell = m_rViewShell.GetDocSh();
    if (!pDocShell)
        return {};
    return pDocShell->GetModel();
}

void SdSecurityLabelTarget::applyMarking(const svx::seclabel::LabelPlacement& rPlacement)
{
    // The marking becomes the visible slide footer on every standard slide. Its colour
    // follows the master's footer placeholder (HeaderFooterSettings carries only text +
    // visibility, no colour). Cover/portion placements and the watermark are Writer-only
    // (Impress has no native watermark; a master-slide text object is the deferred option).
    setFooterOnAllSlides(m_rViewShell.GetDoc(), rPlacement.aMarking, true);
    // The master's footer placeholder is sized for a short footer; a long marking would
    // otherwise wrap and spill off the bottom of the slide.
    fitFootersToText(m_rViewShell.GetDoc());
}

void SdSecurityLabelTarget::clearMarkings()
{
    setFooterOnAllSlides(m_rViewShell.GetDoc(), OUString(), false);
}

void SdSecurityLabelTarget::notify(const svx::seclabel::LabelChange& rChange)
{
    if (!comphelper::COKit::isActive())
        return;

    // The enriched .uno:SecurityLabel statechanged payload: the browser banner reads
    // state.marking (empty => hide) and fires a Security_Label_Changed postMessage from
    // state.action + old/new. wsd broadcasts this to every session.
    tools::JsonWriter aJson;
    aJson.put("commandName", ".uno:SecurityLabel");
    {
        auto aState = aJson.startNode("state");
        aJson.put("action", rChange.aAction);
        aJson.put("marking", rChange.aMarking);
        aJson.put("classification", rChange.aClassification);
        aJson.put("oldMarking", rChange.aOldMarking);
        aJson.put("oldClassification", rChange.aOldClassification);
    }
    m_rViewShell.GetViewShellBase().viewCallback(COKitCallbackType::STATE_CHANGED,
                                                 aJson.finishAndGetAsOString());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
