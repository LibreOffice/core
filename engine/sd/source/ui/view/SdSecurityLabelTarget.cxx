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
#include <drawdoc.hxx>
#include <sdpage.hxx>

#include <com/sun/star/frame/XModel.hpp>

#include <COKit/COKit.hxx>
#include <comphelper/kit.hxx>
#include <tools/json_writer.hxx>

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
}

uno::Reference<frame::XModel> SdSecurityLabelTarget::getModel() const
{
    sd::DrawDocShell* pDocShell = m_rViewShell.GetDocSh();
    if (!pDocShell)
        return {};
    return pDocShell->GetModel();
}

void SdSecurityLabelTarget::applyMarking(const svx::seclabel::LabelPlacement& rPlacement)
{
    // The marking becomes the slide footer. Colour/cover/portion/watermark
    // placements do not apply to Impress.
    setFooterOnAllSlides(m_rViewShell.GetDoc(), rPlacement.aMarking, true);
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
