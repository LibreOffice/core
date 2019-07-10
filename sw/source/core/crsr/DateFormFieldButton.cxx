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
#include <svx/numfmtsh.hxx>

class SwDatePickerDialog : public FloatingWindow
{
private:
    VclPtr<Calendar> m_pCalendar;
    sw::mark::DateFieldmark* m_pFieldmark;
    SvNumberFormatter* m_pNumberFormatter;

    DECL_LINK(ImplSelectHdl, Calendar*, void);

public:
    SwDatePickerDialog(SwEditWin* parent, sw::mark::DateFieldmark* pFieldmark,
                       SvNumberFormatter* pNumberFormatter);
    virtual ~SwDatePickerDialog() override;
    virtual void dispose() override;
};

SwDatePickerDialog::SwDatePickerDialog(SwEditWin* parent, sw::mark::DateFieldmark* pFieldmark,
                                       SvNumberFormatter* pNumberFormatter)
    : FloatingWindow(parent, WB_BORDER | WB_SYSTEMWINDOW | WB_NOSHADOW)
    , m_pCalendar(VclPtr<Calendar>::Create(this, WB_TABSTOP))
    , m_pFieldmark(pFieldmark)
    , m_pNumberFormatter(pNumberFormatter)
{
    if (m_pFieldmark != nullptr)
    {
        std::pair<bool, double> aResult = m_pFieldmark->GetCurrentDate();
        if (aResult.first)
        {
            const Date& rNullDate = m_pNumberFormatter->GetNullDate();
            m_pCalendar->SetCurDate(rNullDate + sal_Int32(aResult.second));
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
            const Date& rNullDate = m_pNumberFormatter->GetNullDate();
            double dDate = pCalendar->GetFirstSelectedDate() - rNullDate;
            m_pFieldmark->SetCurrentDate(dDate);
        }
        EndPopupMode();
    }
}

DateFormFieldButton::DateFormFieldButton(SwEditWin* pEditWin, sw::mark::DateFieldmark& rFieldmark,
                                         SvNumberFormatter* pNumberFormatter)
    : FormFieldButton(pEditWin, rFieldmark)
    , m_pNumberFormatter(pNumberFormatter)
{
}

DateFormFieldButton::~DateFormFieldButton() { disposeOnce(); }

void DateFormFieldButton::InitPopup()
{
    sw::mark::DateFieldmark* pDateFieldmark = dynamic_cast<sw::mark::DateFieldmark*>(&m_rFieldmark);
    m_pFieldPopup = VclPtr<SwDatePickerDialog>::Create(static_cast<SwEditWin*>(GetParent()),
                                                       pDateFieldmark, m_pNumberFormatter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
