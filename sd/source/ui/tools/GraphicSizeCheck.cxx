/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <memory>
#include <tools/GraphicSizeCheck.hxx>
#include <ModelTraverser.hxx>
#include <svx/strings.hrc>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/svxids.hrc>
#include <sfx2/dispatch.hxx>

#include <sdresid.hxx>
#include <DrawDocShell.hxx>
#include <ViewShell.hxx>

namespace sd
{
GraphicSizeViolation::GraphicSizeViolation(sal_Int32 nDPI, SdrGrafObj* pGraphicObject)
    : m_pGraphicObject(pGraphicObject)
{
    constexpr double fLowPercentage = 110;
    constexpr double fHighPercentage = 50;

    m_nLowDPILimit = sal_Int32(100.0 / fLowPercentage * nDPI);
    m_nHighDPILimit = sal_Int32(100.0 / fHighPercentage * nDPI);
}

bool GraphicSizeViolation::check()
{
    Graphic aGraphic = m_pGraphicObject->GetGraphic();
    Size aSizePixel = aGraphic.GetSizePixel();
    Size aGraphicSize = m_pGraphicObject->GetLogicRect().GetSize();

    double nSizeXInch
        = o3tl::convert(double(aGraphicSize.Width()), o3tl::Length::mm100, o3tl::Length::in);
    double nSizeYInch
        = o3tl::convert(double(aGraphicSize.Height()), o3tl::Length::mm100, o3tl::Length::in);

    m_nDPIX = sal_Int32(aSizePixel.Width() / nSizeXInch);
    m_nDPIY = sal_Int32(aSizePixel.Height() / nSizeYInch);

    return isDPITooLow() || isDPITooHigh();
}

const OUString& GraphicSizeViolation::getGraphicName() { return m_pGraphicObject->GetName(); }

namespace
{
class GraphicSizeCheckHandler : public ModelTraverseHandler
{
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

    void handleSdrObject(SdrObject* pObject) override
    {
        auto* pGraphicObject = dynamic_cast<SdrGrafObj*>(pObject);
        if (!pGraphicObject)
            return;

        auto pEntry = std::make_unique<GraphicSizeViolation>(m_nDPI, pGraphicObject);
        if (pEntry->check())
        {
            m_rGraphicSizeViolationList.push_back(std::move(pEntry));
        }
    }
};

} // end anonymous namespace

void GraphicSizeCheck::check()
{
    if (!m_pDocument)
        return;

    sal_Int32 nDPI = m_pDocument->getImagePreferredDPI();
    if (nDPI == 0)
        return;

    auto pHandler = std::make_shared<GraphicSizeCheckHandler>(nDPI, m_aGraphicSizeViolationList);

    ModelTraverser aModelTraverser(m_pDocument, { .mbPages = true, .mbMasterPages = false });
    aModelTraverser.addNodeHandler(pHandler);
    aModelTraverser.traverse();
}

OUString GraphicSizeCheckGUIEntry::getText()
{
    OUString sText;

    if (m_pViolation->isDPITooLow())
    {
        sText = SdResId(STR_WARNING_GRAPHIC_PIXEL_COUNT_LOW);
    }
    else if (m_pViolation->isDPITooHigh())
    {
        sText = SdResId(STR_WARNING_GRAPHIC_PIXEL_COUNT_HIGH);
    }

    sText = sText.replaceAll("%NAME%", m_pViolation->getGraphicName());
    sText = sText.replaceAll("%DPIX%", OUString::number(m_pViolation->getDPIX()));
    sText = sText.replaceAll("%DPIY%", OUString::number(m_pViolation->getDPIY()));

    return sText;
}

void GraphicSizeCheckGUIEntry::markObject()
{
    sd::ViewShell* pViewShell = m_pDocument->GetDocSh()->GetViewShell();
    SdrView* pView = pViewShell->GetView();
    pView->ShowSdrPage(m_pViolation->getObject()->getSdrPageFromSdrObject());
    pView->UnmarkAll();
    pView->MarkObj(m_pViolation->getObject(), pView->GetSdrPageView());
}

void GraphicSizeCheckGUIEntry::runProperties()
{
    markObject();
    sd::ViewShell* pViewShell = m_pDocument->GetDocSh()->GetViewShell();
    pViewShell->GetDispatcher()->Execute(SID_ATTR_GRAF_CROP, SfxCallMode::SYNCHRON);
}

GraphicSizeCheckGUIResult::GraphicSizeCheckGUIResult(SdDrawDocument* pDocument)
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
    return SdResId(STR_GRAPHIC_SIZE_CHECK_DIALOG_TITLE);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
