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
#include <bookmrk.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/InterimItemWindow.hxx>
#include <tools/date.hxx>
#include <svl/zforlist.hxx>

namespace
{
class SwCalendarBox final : public InterimItemWindow
{
private:
    std::unique_ptr<weld::Calendar> m_xCalendar;

public:
    SwCalendarBox(vcl::Window* pParent)
        : InterimItemWindow(pParent, "modules/swriter/ui/calendar.ui", "Calendar")
        , m_xCalendar(m_xBuilder->weld_calendar("date"))
    {
    }
    weld::Calendar& get_widget() { return *m_xCalendar; }
    virtual ~SwCalendarBox() override { disposeOnce(); }
    virtual void dispose() override
    {
        m_xCalendar.reset();
        InterimItemWindow::dispose();
    }
};

class SwDatePickerDialog : public FloatingWindow
{
private:
    VclPtr<SwCalendarBox> m_xCalendar;
    sw::mark::DateFieldmark* m_pFieldmark;
    SvNumberFormatter* m_pNumberFormatter;

    DECL_LINK(ImplSelectHdl, weld::Calendar&, void);

public:
    SwDatePickerDialog(SwEditWin* parent, sw::mark::DateFieldmark* pFieldmark,
                       SvNumberFormatter* pNumberFormatter);
    virtual ~SwDatePickerDialog() override;
    virtual void dispose() override;
};
}

SwDatePickerDialog::SwDatePickerDialog(SwEditWin* parent, sw::mark::DateFieldmark* pFieldmark,
                                       SvNumberFormatter* pNumberFormatter)
    : FloatingWindow(parent, WB_BORDER | WB_SYSTEMWINDOW | WB_NOSHADOW)
    , m_xCalendar(VclPtr<SwCalendarBox>::Create(this))
    , m_pFieldmark(pFieldmark)
    , m_pNumberFormatter(pNumberFormatter)
{
    weld::Calendar& rCalendar = m_xCalendar->get_widget();

    if (m_pFieldmark != nullptr)
    {
        std::pair<bool, double> aResult = m_pFieldmark->GetCurrentDate();
        if (aResult.first)
        {
            const Date& rNullDate = m_pNumberFormatter->GetNullDate();
            rCalendar.set_date(rNullDate + sal_Int32(aResult.second));
        }
    }

    Size lbSize(rCalendar.get_preferred_size());

    m_xCalendar->SetSizePixel(lbSize);
    rCalendar.connect_activated(LINK(this, SwDatePickerDialog, ImplSelectHdl));
    m_xCalendar->Show();

    rCalendar.grab_focus();

    SetSizePixel(lbSize);
}

SwDatePickerDialog::~SwDatePickerDialog() { disposeOnce(); }

void SwDatePickerDialog::dispose()
{
    m_xCalendar.disposeAndClear();
    FloatingWindow::dispose();
}

IMPL_LINK(SwDatePickerDialog, ImplSelectHdl, weld::Calendar&, rCalendar, void)
{
    if (m_pFieldmark != nullptr)
    {
        const Date& rNullDate = m_pNumberFormatter->GetNullDate();
        double dDate = rCalendar.get_date() - rNullDate;
        m_pFieldmark->SetCurrentDate(dDate);
    }
    EndPopupMode();
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
