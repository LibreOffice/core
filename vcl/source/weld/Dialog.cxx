/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <strings.hrc>
#include <svdata.hxx>

#include <officecfg/Office/Common.hxx>
#include <vcl/abstdlg.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/Dialog.hxx>
#include <vcl/weld/Menu.hxx>

namespace weld
{
bool Dialog::signal_command(const CommandEvent& rCEvt)
{
    if (CommandEventId::ContextMenu == rCEvt.GetCommand()
        && officecfg::Office::Common::Misc::ScreenshotMode::get())
    {
        // show menu to allow opening the screenshot annotation dialog
        std::unique_ptr<weld::Builder> xBuilder(
            Application::CreateBuilder(this, u"vcl/ui/screenshotmenu.ui"_ustr));
        std::unique_ptr<weld::Menu> pMenu = xBuilder->weld_menu(u"menu"_ustr);
        static constexpr OUString sMenuItemId = u"screenshot"_ustr;
        pMenu->append(sMenuItemId, VclResId(SV_BUTTONTEXT_SCREENSHOT));
        // set tooltip if extended tips are enabled
        if (ImplGetSVHelpData().mbBalloonHelp)
            pMenu->set_tooltip_text(sMenuItemId, VclResId(SV_HELPTEXT_SCREENSHOT));
        pMenu->set_item_help_id(sMenuItemId, u"InteractiveScreenshotMode"_ustr);

        if (pMenu->popup_at_rect(this, tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1, 1)))
            == sMenuItemId)
        {
            VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
            ScopedVclPtr<AbstractScreenshotAnnotationDlg> pDialog
                = pFact->CreateScreenshotAnnotationDlg(*this);
            assert(pDialog);
            pDialog->Execute();
        }

        return true;
    }

    return Window::signal_command(rCEvt);
}

void Dialog::set_default_response(int nResponse)
{
    std::unique_ptr<weld::Button> pButton = weld_button_for_response(nResponse);
    change_default_button(nullptr, pButton.get());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
