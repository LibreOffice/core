/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <welcomedlg.hxx>

#include <whatsnewtabpage.hxx>
#include <uitabpage.hxx>
#include "../options/appearance.hxx"

#include <comphelper/dispatchcommand.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/viewfrm.hxx>

constexpr OUString sNewsTab = u"WhatsNewTabPage"_ustr;
constexpr OUString sUITab = u"UITabPage"_ustr;
constexpr OUString sAppearanceTab = u"AppearanceTabPage"_ustr;

WelcomeDialog::WelcomeDialog(weld::Window* pParent)
    : SfxTabDialogController(pParent, u"cui/ui/welcomedialog.ui"_ustr, u"WelcomeDialog"_ustr)
    , m_xActionBtn(m_xBuilder->weld_button(u"action"_ustr)) // release notes / apply
    , m_xNextBtn(m_xBuilder->weld_button(u"next"_ustr)) // next / close
    , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr)) // hidden
    , m_xResetBtn(m_xBuilder->weld_button(u"reset"_ustr)) // hidden
    , m_xCancelBtn(m_xBuilder->weld_button(u"cancel"_ustr)) // hidden
{
    m_xDialog->set_title(SfxResId(STR_WELCOME_LINE1));

    AddTabPage(sNewsTab, WhatsNewTabPage::Create, nullptr);
    AddTabPage(sUITab, UITabPage::Create, nullptr);
    AddTabPage(sAppearanceTab, SvxAppearanceTabPage::Create, nullptr);

    m_xTabCtrl->connect_enter_page(LINK(this, WelcomeDialog, OnActivatePage));
    m_xResetBtn->set_visible(false);
    m_xOKBtn->set_visible(false);
    m_xCancelBtn->set_visible(false);

    m_xNextBtn->connect_clicked(LINK(this, WelcomeDialog, OnNextClick));
    m_xActionBtn->connect_clicked(LINK(this, WelcomeDialog, OnActionClick));

    m_xTabCtrl->set_current_page(sNewsTab);
    OnActivatePage(sNewsTab);
}

IMPL_LINK(WelcomeDialog, OnActivatePage, const OUString&, rPage, void)
{
    if (rPage == sNewsTab)
        m_xActionBtn->set_label(SfxResId(STR_CREDITS_BUTTON));
    else
        m_xActionBtn->set_label(SfxResId(STR_WELCOME_APPLY));

    if (rPage == sAppearanceTab)
        m_xNextBtn->set_label(SfxResId(STR_WELCOME_CLOSE));
    else
        m_xNextBtn->set_label(SfxResId(STR_WELCOME_NEXT));
}

IMPL_LINK_NOARG(WelcomeDialog, OnNextClick, weld::Button&, void)
{
    const int nCurrentTabPage(m_xTabCtrl->get_current_page());

    if (nCurrentTabPage < 2)
    {
        m_xTabCtrl->set_current_page(nCurrentTabPage + 1);
        OnActivatePage(m_xTabCtrl->get_page_ident(nCurrentTabPage + 1));
    }
    else
        m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(WelcomeDialog, OnActionClick, weld::Button&, void)
{
    switch (m_xTabCtrl->get_current_page())
    {
        case 0:
        {
            SfxViewFrame* pViewFrame = SfxViewFrame::Current();
            if (pViewFrame)
                pViewFrame->GetBindings().GetDispatcher()->Execute(SID_CREDITS);
        }
        break;
        case 1:
        {
            UITabPage* pUITabPage = static_cast<UITabPage*>(GetCurTabPage());
            OUString sCmd = pUITabPage->GetSelectedMode();
            comphelper::dispatchCommand(".uno:ToolbarMode?Mode:string=" + sCmd, {});
        }
        break;
        case 2:
        {
            SvxAppearanceTabPage* pAppearanceTabPage
                = static_cast<SvxAppearanceTabPage*>(GetCurTabPage());
            pAppearanceTabPage->FillItemSet(nullptr);
        }
        break;
        default:
            m_xDialog->response(RET_OK);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */