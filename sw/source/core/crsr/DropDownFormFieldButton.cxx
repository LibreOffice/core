/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <DropDownFormFieldButton.hxx>
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

        if (!vListEntries.hasElements())
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
    : FormFieldButton(pEditWin, rFieldmark)
{
}

DropDownFormFieldButton::~DropDownFormFieldButton() { disposeOnce(); }

void DropDownFormFieldButton::InitPopup()
{
    m_pFieldPopup = VclPtr<SwFieldDialog>::Create(static_cast<SwEditWin*>(GetParent()),
                                                  &m_rFieldmark, GetSizePixel().Width());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
