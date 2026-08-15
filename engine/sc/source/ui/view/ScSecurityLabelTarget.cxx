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

// Replace the centre text of the page style's (right/shared) header.
void setHeaderCenter(const uno::Reference<beans::XPropertySet>& xPageStyle, const OUString& rText,
                     sal_Int32 nColor)
{
    if (!xPageStyle.is())
        return;

    uno::Reference<sheet::XHeaderFooterContent> xContent;
    xPageStyle->getPropertyValue(SC_UNO_PAGE_RIGHTHDRCON) >>= xContent;
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

    xPageStyle->setPropertyValue(SC_UNO_PAGE_RIGHTHDRCON, cpo::uno::Any(xContent));
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

    // Turn the header on (shared, so a single centre text covers all pages) and
    // write the marking. Cover/portion/watermark placements do not apply to Calc.
    xPageStyle->setPropertyValue(SC_UNO_PAGE_HDRON, cpo::uno::Any(true));
    xPageStyle->setPropertyValue(SC_UNO_PAGE_HDRSHARED, cpo::uno::Any(true));
    setHeaderCenter(xPageStyle, rPlacement.aMarking, rPlacement.nColor);
}

void ScSecurityLabelTarget::clearMarkings()
{
    uno::Reference<frame::XModel> xModel = getModel();
    uno::Reference<beans::XPropertySet> xPageStyle = getActivePageStyle(m_rViewShell, xModel);
    setHeaderCenter(xPageStyle, OUString(), 0); // empty centre text
}

void ScSecurityLabelTarget::notify(const OUString& rMarking)
{
    if (!comphelper::COKit::isActive())
        return;

    // Same shape as ScModelObj::getCommandValues(".uno:SecurityLabel"): the browser
    // banner reads state.marking (empty => hide the banner).
    tools::JsonWriter aJson;
    aJson.put("commandName", ".uno:SecurityLabel");
    {
        auto aState = aJson.startNode("state");
        aJson.put("marking", rMarking);
    }
    m_rViewShell.viewCallback(COKitCallbackType::STATE_CHANGED, aJson.finishAndGetAsOString());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
