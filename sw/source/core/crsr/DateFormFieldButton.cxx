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
#include <bookmark.hxx>
#include <tools/date.hxx>
#include <svl/numformat.hxx>
#include <vcl/svapp.hxx>

IMPL_LINK(DateFormFieldButton, ImplSelectHdl, weld::Calendar&, rCalendar, void)
{
    if (m_pDateFieldmark)
    {
        const Date& rNullDate = m_pNumberFormatter->GetNullDate();
        double dDate = rCalendar.get_date() - rNullDate;
        m_pDateFieldmark->SetCurrentDate(dDate);
    }
    m_xFieldPopup->popdown();
}

DateFormFieldButton::DateFormFieldButton(SwEditWin* pEditWin, sw::mark::DateFieldmark& rFieldmark,
                                         SvNumberFormatter* pNumberFormatter)
    : FormFieldButton(pEditWin, rFieldmark)
    , m_pNumberFormatter(pNumberFormatter)
    , m_pDateFieldmark(dynamic_cast<sw::mark::DateFieldmark*>(&m_rFieldmark))
{
}

DateFormFieldButton::~DateFormFieldButton() { disposeOnce(); }

void DateFormFieldButton::LaunchPopup()
{
    m_xFieldPopupBuilder
        = Application::CreateBuilder(GetFrameWeld(), u"modules/swriter/ui/calendar.ui"_ustr);
    m_xFieldPopup = m_xFieldPopupBuilder->weld_popover(u"Calendar"_ustr);
    m_xCalendar = m_xFieldPopupBuilder->weld_calendar(u"date"_ustr);
    if (m_pDateFieldmark)
    {
        std::pair<bool, double> aResult = m_pDateFieldmark->GetCurrentDate();
        if (aResult.first)
        {
            const Date& rNullDate = m_pNumberFormatter->GetNullDate();
            m_xCalendar->set_date(rNullDate + sal_Int32(aResult.second));
        }
    }
    m_xCalendar->connect_activated(LINK(this, DateFormFieldButton, ImplSelectHdl));
    FormFieldButton::LaunchPopup();
    m_xCalendar->grab_focus();
}

void DateFormFieldButton::DestroyPopup()
{
    m_xCalendar.reset();
    FormFieldButton::DestroyPopup();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
