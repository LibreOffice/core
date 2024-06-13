/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <datecontentcontrolbutton.hxx>

#include <svl/numformat.hxx>
#include <tools/date.hxx>
#include <vcl/svapp.hxx>

#include <edtwin.hxx>
#include <formatcontentcontrol.hxx>
#include <view.hxx>
#include <wrtsh.hxx>

IMPL_LINK(SwDateContentControlButton, SelectHandler, weld::Calendar&, rCalendar, void)
{
    const Date& rNullDate = m_pNumberFormatter->GetNullDate();
    double fDate = rCalendar.get_date() - rNullDate;
    m_xPopup->popdown();
    m_pContentControl->SetSelectedDate(fDate);
    SwView& rView = static_cast<SwEditWin*>(GetParent())->GetView();
    SwWrtShell& rWrtShell = rView.GetWrtShell();
    rWrtShell.GotoContentControl(*m_pContentControl->GetFormatContentControl());
}

SwDateContentControlButton::SwDateContentControlButton(
    SwEditWin* pEditWin, const std::shared_ptr<SwContentControl>& pContentControl,
    SvNumberFormatter* pNumberFormatter)
    : SwContentControlButton(pEditWin, pContentControl)
    , m_pNumberFormatter(pNumberFormatter)
{
}

SwDateContentControlButton::~SwDateContentControlButton() { disposeOnce(); }

void SwDateContentControlButton::LaunchPopup()
{
    m_xPopupBuilder = Application::CreateBuilder(
        GetFrameWeld(), u"modules/swriter/ui/contentcontrolcalendar.ui"_ustr);
    m_xPopup = m_xPopupBuilder->weld_popover(u"Calendar"_ustr);
    m_xCalendar = m_xPopupBuilder->weld_calendar(u"date"_ustr);

    // Read the doc model.
    if (m_pContentControl)
    {
        const Date& rNullDate = m_pNumberFormatter->GetNullDate();
        double fCurrentDate = m_pContentControl->GetCurrentDateValue();
        if (fCurrentDate != 0)
        {
            m_xCalendar->set_date(rNullDate + sal_Int32(fCurrentDate));
        }
    }

    m_xCalendar->connect_activated(LINK(this, SwDateContentControlButton, SelectHandler));
    SwContentControlButton::LaunchPopup();
    m_xCalendar->grab_focus();
}

void SwDateContentControlButton::DestroyPopup()
{
    m_xCalendar.reset();
    SwContentControlButton::DestroyPopup();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
