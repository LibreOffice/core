/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <GraphicSizeCheck.hxx>
#include <svx/strings.hrc>
#include <svx/svdobj.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>

#include <ModelTraverser.hxx>
#include <ndgrf.hxx>
#include <IDocumentSettingAccess.hxx>
#include <fmtfsize.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <cmdid.h>

using namespace css;

namespace sw
{
GraphicSizeViolation::GraphicSizeViolation(sal_Int32 nDPI, const SwGrfNode* pGraphicNode)
    : m_pGraphicNode(pGraphicNode)
{
    constexpr double fLowPercentage = 110;
    constexpr double fHighPercentage = 50;

    m_nLowDPILimit = sal_Int32(100.0 / fLowPercentage * nDPI);
    m_nHighDPILimit = sal_Int32(100.0 / fHighPercentage * nDPI);
}

bool GraphicSizeViolation::check()
{
    auto pFrameFormat = m_pGraphicNode->GetFlyFormat();
    Graphic aGraphic = m_pGraphicNode->GetGraphic();
    Size aSizePixel = aGraphic.GetSizePixel();
    Size aFrameSize(pFrameFormat->GetFrameSize().GetSize());

    double nSizeXInch
        = o3tl::convert(double(aFrameSize.Width()), o3tl::Length::twip, o3tl::Length::in);
    double nSizeYInch
        = o3tl::convert(double(aFrameSize.Height()), o3tl::Length::twip, o3tl::Length::in);

    m_nDPIX = sal_Int32(aSizePixel.Width() / nSizeXInch);
    m_nDPIY = sal_Int32(aSizePixel.Height() / nSizeYInch);

    return isDPITooLow() || isDPITooHigh();
}

const UIName& GraphicSizeViolation::getGraphicName()
{
    return m_pGraphicNode->GetFlyFormat()->GetName();
}

namespace
{
class GraphicSizeCheckHandler : public ModelTraverseHandler
{
private:
    sal_Int32 m_nDPI;
    std::vector<std::unique_ptr<GraphicSizeViolation>>& m_rGraphicSizeViolationList;

public:
    GraphicSizeCheckHandler(
        sal_Int32 nDPI,
        std::vector<std::unique_ptr<GraphicSizeViolation>>& rGraphicSizeViolationList)
        : m_nDPI(nDPI)
        , m_rGraphicSizeViolationList(rGraphicSizeViolationList)
    {
    }

    void handleNode(SwNode* pNode) override
    {
        if (!pNode->IsGrfNode())
            return;

        auto pEntry = std::make_unique<GraphicSizeViolation>(m_nDPI, pNode->GetGrfNode());
        if (pEntry->check())
        {
            m_rGraphicSizeViolationList.push_back(std::move(pEntry));
        }
    }

    void handleSdrObject(SdrObject* /*pObject*/) override {}
};

} // end anonymous namespace

void GraphicSizeCheck::check()
{
    sal_Int32 nDPI = m_pDocument->getIDocumentSettingAccess().getImagePreferredDPI();
    if (nDPI == 0)
        return;

    auto pHandler = std::make_shared<GraphicSizeCheckHandler>(nDPI, m_aGraphicSizeViolationList);
    ModelTraverser aModelTraverser(m_pDocument);
    aModelTraverser.addNodeHandler(pHandler);
    aModelTraverser.traverse();
}

OUString GraphicSizeCheckGUIEntry::getText()
{
    OUString sText;

    if (m_pViolation->isDPITooLow())
    {
        sText = SwResId(STR_WARNING_GRAPHIC_PIXEL_COUNT_LOW);
    }
    else if (m_pViolation->isDPITooHigh())
    {
        sText = SwResId(STR_WARNING_GRAPHIC_PIXEL_COUNT_HIGH);
    }

    sText = sText.replaceAll("%NAME%", m_pViolation->getGraphicName().toString());
    sText = sText.replaceAll("%DPIX%", OUString::number(m_pViolation->getDPIX()));
    sText = sText.replaceAll("%DPIY%", OUString::number(m_pViolation->getDPIY()));

    return sText;
}

void GraphicSizeCheckGUIEntry::markObject()
{
    SwWrtShell* pWrtShell = m_pDocument->GetDocShell()->GetWrtShell();
    pWrtShell->GotoFly(m_pViolation->getGraphicName(), FLYCNTTYPE_ALL, true);
}

void GraphicSizeCheckGUIEntry::runProperties()
{
    markObject();
    SwWrtShell* pWrtShell = m_pDocument->GetDocShell()->GetWrtShell();
    pWrtShell->GetView().GetViewFrame().GetDispatcher()->Execute(FN_FORMAT_GRAFIC_DLG,
                                                                 SfxCallMode::SYNCHRON);
}

GraphicSizeCheckGUIResult::GraphicSizeCheckGUIResult(SwDoc* pDocument)
{
    GraphicSizeCheck aCheck(pDocument);
    aCheck.check();

    auto& rCollection = getCollection();
    for (auto& rpViolation : aCheck.getViolationList())
    {
        auto rGUIEntry
            = std::make_unique<GraphicSizeCheckGUIEntry>(pDocument, std::move(rpViolation));
        rCollection.push_back(std::move(rGUIEntry));
    }
}

OUString GraphicSizeCheckGUIResult::getTitle()
{
    return SwResId(STR_GRAPHIC_SIZE_CHECK_DIALOG_TITLE);
}

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
