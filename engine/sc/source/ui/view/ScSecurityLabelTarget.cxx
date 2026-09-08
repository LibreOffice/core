/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ScSecurityLabelTarget.hxx>

#include <docsh.hxx>
#include <document.hxx>
#include <tabvwsh.hxx>
#include <unonames.hxx>
#include <viewdata.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>

#include <COKit/COKit.hxx>
#include <comphelper/kit.hxx>
#include <tools/json_writer.hxx>

using namespace css;
using namespace ::cpo;

namespace
{
// The XPropertySet of the page style driving the active sheet, or empty.
uno::Reference<beans::XPropertySet> getActivePageStyle(ScTabViewShell& rViewShell,
                                                       const uno::Reference<frame::XModel>& xModel)
{
    if (!xModel.is())
        return {};

    ScViewData& rViewData = rViewShell.GetViewData();
    const OUString sStyle = rViewData.GetDocument().GetPageStyle(rViewData.GetTabNumber());

    uno::Reference<style::XStyleFamiliesSupplier> xSupplier(xModel, uno::UNO_QUERY);
    if (!xSupplier.is())
        return {};

    uno::Reference<container::XNameAccess> xFamilies(xSupplier->getStyleFamilies());
    uno::Reference<container::XNameAccess> xPageStyles;
    if (xFamilies.is())
        xFamilies->getByName(u"PageStyles"_ustr) >>= xPageStyles;

    uno::Reference<beans::XPropertySet> xPageStyle;
    if (xPageStyles.is() && xPageStyles->hasByName(sStyle))
        xPageStyles->getByName(sStyle) >>= xPageStyle;
    return xPageStyle;
}

// Replace the centre text (coloured) of one page-style header/footer content property
// (the right/shared variant). Only the centre section is touched, so the user's own
// left/right header/footer content is left intact.
void setCenterText(const uno::Reference<beans::XPropertySet>& xPageStyle,
                   const OUString& rContentProp, const OUString& rText, sal_Int32 nColor)
{
    if (!xPageStyle.is())
        return;

    uno::Reference<sheet::XHeaderFooterContent> xContent;
    xPageStyle->getPropertyValue(rContentProp) >>= xContent;
    if (!xContent.is())
        return;

    uno::Reference<text::XText> xText = xContent->getCenterText();
    if (!xText.is())
        return;

    xText->setString(rText);
    if (!rText.isEmpty())
    {
        uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
        xCursor->gotoStart(false);
        xCursor->gotoEnd(true);
        uno::Reference<beans::XPropertySet> xRun(xCursor, uno::UNO_QUERY);
        if (xRun.is())
            xRun->setPropertyValue(u"CharColor"_ustr, cpo::uno::Any(nColor));
    }

    xPageStyle->setPropertyValue(rContentProp, cpo::uno::Any(xContent));
}
}

uno::Reference<frame::XModel> ScSecurityLabelTarget::getModel() const
{
    ScDocShell* pDocShell = m_rViewShell.GetViewData().GetDocShell();
    if (!pDocShell)
        return {};
    return pDocShell->GetModel();
}

void ScSecurityLabelTarget::applyMarking(const svx::seclabel::LabelPlacement& rPlacement)
{
    uno::Reference<frame::XModel> xModel = getModel();
    uno::Reference<beans::XPropertySet> xPageStyle = getActivePageStyle(m_rViewShell, xModel);
    if (!xPageStyle.is())
        return;

    // Turn the header and footer on (shared, so a single centre text covers all pages)
    // and write the marking to both. Cover/portion placements and the watermark are
    // Writer-only (Calc has no native watermark); the on-screen marking is the banner.
    // Calc header/footer render in print / page-layout view only.
    xPageStyle->setPropertyValue(SC_UNO_PAGE_HDRON, cpo::uno::Any(true));
    xPageStyle->setPropertyValue(SC_UNO_PAGE_HDRSHARED, cpo::uno::Any(true));
    xPageStyle->setPropertyValue(SC_UNO_PAGE_FTRON, cpo::uno::Any(true));
    xPageStyle->setPropertyValue(SC_UNO_PAGE_FTRSHARED, cpo::uno::Any(true));
    setCenterText(xPageStyle, SC_UNO_PAGE_RIGHTHDRCON, rPlacement.aMarking, rPlacement.nColor);
    setCenterText(xPageStyle, SC_UNO_PAGE_RIGHTFTRCON, rPlacement.aMarking, rPlacement.nColor);
}

void ScSecurityLabelTarget::clearMarkings()
{
    uno::Reference<frame::XModel> xModel = getModel();
    uno::Reference<beans::XPropertySet> xPageStyle = getActivePageStyle(m_rViewShell, xModel);
    setCenterText(xPageStyle, SC_UNO_PAGE_RIGHTHDRCON, OUString(), 0); // empty centre text
    setCenterText(xPageStyle, SC_UNO_PAGE_RIGHTFTRCON, OUString(), 0);
}

void ScSecurityLabelTarget::notify(const svx::seclabel::LabelChange& rChange)
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
    m_rViewShell.viewCallback(COKitCallbackType::STATE_CHANGED, aJson.finishAndGetAsOString());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
