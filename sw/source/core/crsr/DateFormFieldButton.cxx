/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <DateFormFieldButton.hxx>
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
#include <vcl/calendar.hxx>
#include <tools/date.hxx>

class SwDatePickerDialog : public FloatingWindow
{
private:
    VclPtr<Calendar> m_pCalendar;
    sw::mark::IFieldmark* m_pFieldmark;

    DECL_LINK(ImplSelectHdl, Calendar*, void);

public:
    SwDatePickerDialog(SwEditWin* parent, sw::mark::IFieldmark* pFieldmark);
    virtual ~SwDatePickerDialog() override;
    virtual void dispose() override;
};

SwDatePickerDialog::SwDatePickerDialog(SwEditWin* parent, sw::mark::IFieldmark* pFieldmark)
    : FloatingWindow(parent, WB_BORDER | WB_SYSTEMWINDOW | WB_NOSHADOW)
    , m_pCalendar(VclPtr<Calendar>::Create(this, WB_TABSTOP))
    , m_pFieldmark(pFieldmark)
{
    if (m_pFieldmark != nullptr)
    {
        sw::mark::IFieldmark::parameter_map_t* pParameters = m_pFieldmark->GetParameters();
        auto pResult = pParameters->find(ODF_FORMDATE_CURRENTDATE);
        if (pResult != pParameters->end())
        {
            sal_Int32 nCurrentDate = 0;
            pResult->second >>= nCurrentDate;
            m_pCalendar->SetCurDate(Date(nCurrentDate));
        }
    }
    m_pCalendar->SetSelectHdl(LINK(this, SwDatePickerDialog, ImplSelectHdl));
    m_pCalendar->SetOutputSizePixel(m_pCalendar->CalcWindowSizePixel());
    m_pCalendar->Show();
    SetOutputSizePixel(m_pCalendar->GetSizePixel());
}

SwDatePickerDialog::~SwDatePickerDialog() { disposeOnce(); }

void SwDatePickerDialog::dispose()
{
    m_pCalendar.clear();
    FloatingWindow::dispose();
}

IMPL_LINK(SwDatePickerDialog, ImplSelectHdl, Calendar*, pCalendar, void)
{
    if (!pCalendar->IsTravelSelect())
    {
        if (m_pFieldmark != nullptr)
        {
            sw::mark::IFieldmark::parameter_map_t* pParameters = m_pFieldmark->GetParameters();
            (*pParameters)[ODF_FORMDATE_CURRENTDATE]
                <<= pCalendar->GetFirstSelectedDate().GetDate();
        }
        EndPopupMode();
    }
}

DateFormFieldButton::DateFormFieldButton(SwEditWin* pEditWin, sw::mark::DateFieldmark& rFieldmark)
    : FormFieldButton(pEditWin, rFieldmark)
{
}

DateFormFieldButton::~DateFormFieldButton() { disposeOnce(); }

void DateFormFieldButton::InitPopup()
{
    m_pFieldPopup
        = VclPtr<SwDatePickerDialog>::Create(static_cast<SwEditWin*>(GetParent()), &m_rFieldmark);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
