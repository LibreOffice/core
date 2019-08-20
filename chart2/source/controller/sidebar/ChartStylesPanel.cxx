/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/svapp.hxx>
#include <iostream>
#include "ChartStylesPanel.hxx"
#include <ChartStyle.hxx>
#include <ChartModel.hxx>
#include <CloneHelper.hxx>
#include <com/sun/star/chart2/ChartObjectType.hpp>
#include <com/sun/star/chart2/XChartStyled.hpp>

using namespace css;
using namespace css::uno;

namespace
{
class QueryString : public weld::GenericDialogController
{
private:
    OUString& m_rReturnValue;

    std::unique_ptr<weld::Button> m_xOKButton;
    std::unique_ptr<weld::Label> m_xFixedText;
    std::unique_ptr<weld::Entry> m_xEdit;

    DECL_LINK(ClickBtnHdl, weld::Button&, void);

public:
    // parent window, Query text, initial value
    QueryString(weld::Window*, OUString const&, OUString&);
};

/*
     *  QueryString
     */
QueryString::QueryString(weld::Window* pParent, OUString const& rQuery, OUString& rRet)
    : GenericDialogController(pParent, "vcl/ui/querydialog.ui", "QueryDialog")
    , m_rReturnValue(rRet)
    , m_xOKButton(m_xBuilder->weld_button("ok"))
    , m_xFixedText(m_xBuilder->weld_label("label"))
    , m_xEdit(m_xBuilder->weld_entry("entry"))
{
    m_xOKButton->connect_clicked(LINK(this, QueryString, ClickBtnHdl));
    m_xFixedText->set_label(rQuery);
    m_xEdit->set_text(m_rReturnValue);
    m_xDialog->set_title(rQuery);
}

IMPL_LINK(QueryString, ClickBtnHdl, weld::Button&, rButton, void)
{
    if (&rButton == m_xOKButton.get())
    {
        m_rReturnValue = m_xEdit->get_text();
        m_xDialog->response(RET_OK);
    }
    else
        m_xDialog->response(RET_CANCEL);
}
}

namespace chart
{
namespace sidebar
{
namespace
{
ChartModel* getChartModel(const css::uno::Reference<css::frame::XModel>& xModel)
{
    ChartModel* pModel = dynamic_cast<ChartModel*>(xModel.get());
    return pModel;
}

css::uno::Reference<css::chart2::XChartStyle>
getCurrentStyle(const css::uno::Reference<css::frame::XModel>& xModel)
{
    return getChartModel(xModel)->getChartStyle();
}

} // namespace anonymous

VclPtr<vcl::Window>
ChartStylesPanel::Create(vcl::Window* pParent,
                         const css::uno::Reference<css::frame::XFrame>& rxFrame,
                         const css::uno::Reference<css::frame::XController>& pController)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to ChartStylesPanel::Create",
                                             nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to ChartStylesPanel::Create", nullptr,
                                             1);

    return VclPtr<ChartStylesPanel>::Create(pParent, rxFrame, pController);
}

ChartStylesPanel::ChartStylesPanel(vcl::Window* pParent,
                                   const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                   const css::uno::Reference<css::frame::XController>& pController)
    : PanelLayout(pParent, "ChartStylesPanel", "modules/schart/ui/sidebarstyle.ui", rxFrame)
    , m_xModel(css::uno::Reference<css::chart2::XChartDocument>(pController->getModel(),
                                                                css::uno::UNO_QUERY))
    , m_xChartStyles(::chart::getChartStyles())
{
    get(maStyleList, "lb_styles");
    get(maCreateStyleLabel, "label_create_from_chart");

    maStyleList->SetSelectHdl(LINK(this, ChartStylesPanel, SelHdl));

    Initialize();
}

ChartStylesPanel::~ChartStylesPanel() { disposeOnce(); }

void ChartStylesPanel::dispose()
{
    maStyleList.clear();
    maCreateStyleLabel.clear();

    PanelLayout::dispose();
}

void ChartStylesPanel::createNewStyle(const OUString& sName)
{
    css::uno::Reference<css::chart2::XChartStyle> xCurrentStyle = getCurrentStyle(m_xModel);

    css::uno::Reference<css::chart2::XChartStyle> xNewStyle
        = CloneHelper::CreateRefClone<css::chart2::XChartStyle>()(xCurrentStyle);
    xNewStyle->updateChartStyle(m_xModel);
    css::uno::Reference<css::style::XStyle> xStyle(xNewStyle, css::uno::UNO_QUERY);
    xStyle->setName(sName);
    m_xChartStyles->insertByName(sName, css::uno::Any(xNewStyle));

    UpdateList();
}

void ChartStylesPanel::UpdateList()
{
    maStyleList->Clear();
    css::uno::Sequence<OUString> aStyleNames = m_xChartStyles->getElementNames();
    for (auto& rStyle : aStyleNames)
    {
        maStyleList->InsertEntry(rStyle);
    }

    maStyleList->InsertEntry(maCreateStyleLabel->GetText());

    OUString aCurrentChartStyleName = css::uno::Reference<css::style::XStyle>(
                                          getCurrentStyle(m_xModel), css::uno::UNO_QUERY_THROW)
                                          ->getName();
    maStyleList->SelectEntry(aCurrentChartStyleName);
}

void ChartStylesPanel::Initialize()
{
    maStyleList->Enable();
    maStyleList->Show();
    UpdateList();
}

IMPL_LINK(ChartStylesPanel, SelHdl, ListBox&, rBox, void)
{
    OUString aSelectedEntry = rBox.GetSelectedEntry();

    if (aSelectedEntry == maCreateStyleLabel->GetText())
    {
        OUString aNewStyleName;
        vcl::Window* pWin = Application::GetDefDialogParent();
        QueryString aQuery(pWin ? pWin->GetFrameWeld() : nullptr, "Select name for new chart style:", aNewStyleName);

        int nReturnCode = aQuery.run();
        if (nReturnCode == RET_OK)
        {
            if (aNewStyleName.isEmpty())
                UpdateList();
            else
                createNewStyle(aNewStyleName);
        }
        else
            UpdateList();
    }
    else
    {
        css::uno::Any aAny = m_xChartStyles->getByName(aSelectedEntry);
        css::uno::Reference<css::chart2::XChartStyle> xChartStyle;
        aAny >>= xChartStyle;

        css::uno::Reference<css::chart2::XChartStyled> xChartModelStyled(m_xModel,
                                                                         css::uno::UNO_QUERY);
        xChartModelStyled->setChartStyle(xChartStyle);
    }
}

} // namespace sidebar
} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
