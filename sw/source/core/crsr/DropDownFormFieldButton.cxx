/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <DropDownFormFieldButton.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <edtwin.hxx>
#include <basegfx/color/bcolortools.hxx>
#include <viewopt.hxx>
#include <bookmrk.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/event.hxx>
#include <vcl/lstbox.hxx>
#include <xmloff/odffields.hxx>
#include <IMark.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <strings.hrc>

/**
 * Popup dialog for drop-down form field showing the list items of the field.
 * The user can select the item using this popup while filling in a form.
 */
class SwFieldDialog : public FloatingWindow
{
private:
    VclPtr<ListBox> aListBox;
    sw::mark::IFieldmark* pFieldmark;

    DECL_LINK(MyListBoxHandler, ListBox&, void);

public:
    SwFieldDialog(SwEditWin* parent, sw::mark::IFieldmark* fieldBM, long nMinListWidth);
    virtual ~SwFieldDialog() override;
    virtual void dispose() override;
};

SwFieldDialog::SwFieldDialog(SwEditWin* parent, sw::mark::IFieldmark* fieldBM, long nMinListWidth)
    : FloatingWindow(parent, WB_BORDER | WB_SYSTEMWINDOW)
    , aListBox(VclPtr<ListBox>::Create(this))
    , pFieldmark(fieldBM)
{
    if (fieldBM != nullptr)
    {
        const sw::mark::IFieldmark::parameter_map_t* const pParameters = fieldBM->GetParameters();

        OUString sListKey = ODF_FORMDROPDOWN_LISTENTRY;
        sw::mark::IFieldmark::parameter_map_t::const_iterator pListEntries
            = pParameters->find(sListKey);
        css::uno::Sequence<OUString> vListEntries;
        if (pListEntries != pParameters->end())
        {
            pListEntries->second >>= vListEntries;
            for (OUString const& i : vListEntries)
                aListBox->InsertEntry(i);
        }

        if (vListEntries.getLength() == 0)
        {
            aListBox->InsertEntry(SwResId(STR_DROP_DOWN_EMPTY_LIST));
        }

        // Select the current one
        OUString sResultKey = ODF_FORMDROPDOWN_RESULT;
        sw::mark::IFieldmark::parameter_map_t::const_iterator pResult
            = pParameters->find(sResultKey);
        if (pResult != pParameters->end())
        {
            sal_Int32 nSelection = -1;
            pResult->second >>= nSelection;
            aListBox->SelectEntryPos(nSelection);
        }
    }

    Size lbSize(aListBox->GetOptimalSize());
    lbSize.AdjustWidth(50);
    lbSize.AdjustHeight(20);
    lbSize.setWidth(std::max(lbSize.Width(), nMinListWidth));
    aListBox->SetSizePixel(lbSize);
    aListBox->SetSelectHdl(LINK(this, SwFieldDialog, MyListBoxHandler));
    aListBox->Show();

    SetSizePixel(lbSize);
}

SwFieldDialog::~SwFieldDialog() { disposeOnce(); }

void SwFieldDialog::dispose()
{
    aListBox.disposeAndClear();
    FloatingWindow::dispose();
}

IMPL_LINK(SwFieldDialog, MyListBoxHandler, ListBox&, rBox, void)
{
    if (!rBox.IsTravelSelect())
    {
        OUString sSelection = rBox.GetSelectedEntry();
        if (sSelection == SwResId(STR_DROP_DOWN_EMPTY_LIST))
        {
            EndPopupMode();
            return;
        }

        sal_Int32 nSelection = rBox.GetSelectedEntryPos();
        if (nSelection >= 0)
        {
            OUString sKey = ODF_FORMDROPDOWN_RESULT;
            (*pFieldmark->GetParameters())[sKey] <<= nSelection;
            pFieldmark->Invalidate();
            SwView& rView = static_cast<SwEditWin*>(GetParent())->GetView();
            rView.GetDocShell()->SetModified();
        }

        EndPopupMode();
    }
}

DropDownFormFieldButton::DropDownFormFieldButton(SwEditWin* pEditWin,
                                                 sw::mark::DropDownFieldmark& rFieldmark)
    : MenuButton(pEditWin, WB_DIALOGCONTROL)
    , m_rFieldmark(rFieldmark)
{
    assert(GetParent());
    assert(dynamic_cast<SwEditWin*>(GetParent()));
}

DropDownFormFieldButton::~DropDownFormFieldButton() { disposeOnce(); }

void DropDownFormFieldButton::dispose()
{
    m_pFieldPopup.disposeAndClear();
    MenuButton::dispose();
}

void DropDownFormFieldButton::CalcPosAndSize(const SwRect& rPortionPaintArea)
{
    assert(GetParent());

    Point aBoxPos = GetParent()->LogicToPixel(rPortionPaintArea.Pos());
    Size aBoxSize = GetParent()->LogicToPixel(rPortionPaintArea.SSize());

    // First calculate the size of the frame around the field
    int nPadding = aBoxSize.Height() / 4;
    aBoxPos.AdjustX(-nPadding);
    aBoxPos.AdjustY(-nPadding);
    aBoxSize.AdjustWidth(2 * nPadding);
    aBoxSize.AdjustHeight(2 * nPadding);

    m_aFieldFramePixel = tools::Rectangle(aBoxPos, aBoxSize);

    // Then extend the size with the button area
    aBoxSize.AdjustWidth(GetParent()->LogicToPixel(rPortionPaintArea.SSize()).Height());

    SetPosSizePixel(aBoxPos, aBoxSize);
}

void DropDownFormFieldButton::MouseButtonUp(const MouseEvent&)
{
    assert(GetParent());

    Point aPixPos = GetPosPixel();
    aPixPos.AdjustY(GetSizePixel().Height());

    m_pFieldPopup = VclPtr<SwFieldDialog>::Create(static_cast<SwEditWin*>(GetParent()),
                                                  &m_rFieldmark, GetSizePixel().Width());
    m_pFieldPopup->SetPopupModeEndHdl(LINK(this, DropDownFormFieldButton, FieldPopupModeEndHdl));

    tools::Rectangle aRect(GetParent()->OutputToScreenPixel(aPixPos), Size(0, 0));
    m_pFieldPopup->StartPopupMode(aRect, FloatWinPopupFlags::Down | FloatWinPopupFlags::GrabFocus);
    Invalidate();
}

IMPL_LINK_NOARG(DropDownFormFieldButton, FieldPopupModeEndHdl, FloatingWindow*, void)
{
    m_pFieldPopup.disposeAndClear();
    m_rFieldmark.Invalidate();
    // Hide the button here and make it visible later, to make transparent background work with SAL_USE_VCLPLUGIN=gen
    Show(false);
    Invalidate();
}

static basegfx::BColor lcl_GetFillColor(const basegfx::BColor& rLineColor, double aLuminance)
{
    basegfx::BColor aHslLine = basegfx::utils::rgb2hsl(rLineColor);
    aHslLine.setZ(aLuminance);
    return basegfx::utils::hsl2rgb(aHslLine);
}

void DropDownFormFieldButton::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    SetMapMode(MapMode(MapUnit::MapPixel));

    //const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    Color aLineColor = COL_BLACK;
    Color aFillColor
        = Color(lcl_GetFillColor(aLineColor.getBColor(), (m_pFieldPopup ? 0.5 : 0.75)));

    // Draw the frame around the field
    // GTK3 backend cuts down the frame's top and left border, to avoid that add a padding around the frame
    int nPadding = 1;
    Point aPos(nPadding, nPadding);
    Size aSize(m_aFieldFramePixel.GetSize().Width() - nPadding,
               m_aFieldFramePixel.GetSize().Height() - 2 * nPadding);
    const tools::Rectangle aFrameRect(tools::Rectangle(aPos, aSize));
    rRenderContext.SetLineColor(aLineColor);
    rRenderContext.SetFillColor(COL_TRANSPARENT);
    rRenderContext.DrawRect(aFrameRect);

    // Draw the button next to the frame
    Point aButtonPos(aFrameRect.TopLeft());
    aButtonPos.AdjustX(aFrameRect.GetSize().getWidth() - 1);
    Size aButtonSize(aFrameRect.GetSize());
    aButtonSize.setWidth(GetSizePixel().getWidth() - aFrameRect.getWidth() - nPadding);
    const tools::Rectangle aButtonRect(tools::Rectangle(aButtonPos, aButtonSize));

    // Background & border
    rRenderContext.SetLineColor(aLineColor);
    rRenderContext.SetFillColor(aFillColor);
    rRenderContext.DrawRect(aButtonRect);

    // the arrowhead
    rRenderContext.SetLineColor(aLineColor);
    rRenderContext.SetFillColor(aLineColor);

    Point aCenter(aButtonPos.X() + (aButtonSize.Width() / 2),
                  aButtonPos.Y() + (aButtonSize.Height() / 2));
    Size aArrowSize(aButtonSize.Width() / 4, aButtonSize.Height() / 10);

    tools::Polygon aPoly(3);
    aPoly.SetPoint(Point(aCenter.X() - aArrowSize.Width(), aCenter.Y() - aArrowSize.Height()), 0);
    aPoly.SetPoint(Point(aCenter.X() + aArrowSize.Width(), aCenter.Y() - aArrowSize.Height()), 1);
    aPoly.SetPoint(Point(aCenter.X(), aCenter.Y() + aArrowSize.Height()), 2);
    rRenderContext.DrawPolygon(aPoly);
}

WindowHitTest DropDownFormFieldButton::ImplHitTest(const Point& rFramePos)
{
    // We need to check whether the position hits the button (the frame should be mouse transparent)
    WindowHitTest aResult = MenuButton::ImplHitTest(rFramePos);
    if (aResult != WindowHitTest::Inside)
        return aResult;
    else
    {
        return rFramePos.X() >= m_aFieldFramePixel.Right() ? WindowHitTest::Inside
                                                           : WindowHitTest::Transparent;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
