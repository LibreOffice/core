/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SwSecurityLabelTarget.hxx>

#include <SecLabelApply.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <view.hxx>
#include <wrtsh.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>

#include <COKit/COKit.hxx>
#include <comphelper/kit.hxx>
#include <sfx2/watermarkitem.hxx>
#include <tools/color.hxx>
#include <tools/json_writer.hxx>

using namespace css;

namespace
{
// The page style of the current view cursor (consistent with PageStyles getByName).
OUString getCurrentPageStyle(const uno::Reference<frame::XModel>& xModel)
{
    OUString sName(u"Standard"_ustr);
    uno::Reference<text::XTextViewCursorSupplier> xSupplier(xModel->getCurrentController(),
                                                            uno::UNO_QUERY);
    if (xSupplier.is())
    {
        uno::Reference<beans::XPropertySet> xProps(xSupplier->getViewCursor(), uno::UNO_QUERY);
        if (xProps.is())
            xProps->getPropertyValue(u"PageStyleName"_ustr) >>= sName;
    }
    return sName;
}
}

uno::Reference<frame::XModel> SwSecurityLabelTarget::getModel() const
{
    SwDocShell* pDocShell = m_rSh.GetDoc()->GetDocShell();
    if (!pDocShell)
        return {};
    return pDocShell->GetModel();
}

void SwSecurityLabelTarget::applyMarking(const svx::seclabel::LabelPlacement& rPlacement)
{
    uno::Reference<frame::XModel> xModel = getModel();
    if (!xModel.is())
        return;

    // Header/footer marking is always applied (v1 pageTopBottom behaviour).
    sw::seclabel::applyMarking(xModel, rPlacement.aMarking, rPlacement.nColor,
                               getCurrentPageStyle(xModel));

    // Watermark is policy-driven: set it when requested, else a default
    // (empty-text) item clears any prior watermark, so a re-label can't leave a
    // stale one behind.
    SfxWatermarkItem aWatermark;
    if (rPlacement.bWatermark)
    {
        aWatermark.SetText(rPlacement.aMarking);
        aWatermark.SetColor(Color(ColorTransparency, static_cast<sal_uInt32>(rPlacement.nColor)));
    }
    m_rSh.SetWatermark(aWatermark);

    // Cover/end-page markings (also self-clearing on re-label).
    sw::seclabel::applyBodyMarkings(xModel, rPlacement.aMarking, rPlacement.nColor,
                                    rPlacement.bCoverStart, rPlacement.bCoverEnd);

    if (rPlacement.bPortion)
        sw::seclabel::applyPortionMarking(xModel, rPlacement.aMarking, rPlacement.nColor);
}

void SwSecurityLabelTarget::clearMarkings()
{
    uno::Reference<frame::XModel> xModel = getModel();
    if (!xModel.is())
        return;
    sw::seclabel::removeLabel(xModel, getCurrentPageStyle(xModel));
    m_rSh.SetWatermark(SfxWatermarkItem()); // empty text clears the watermark
}

void SwSecurityLabelTarget::notify(const svx::seclabel::LabelChange& rChange)
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
    m_rSh.GetView().viewCallback(COKitCallbackType::STATE_CHANGED, aJson.finishAndGetAsOString());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
