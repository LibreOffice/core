/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/autocorrectdialog.hxx>

#include <memory>
#include <optional>
#include <utility>

#include <comphelper/kit.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/viewsh.hxx>
#include <tools/lazydelete.hxx>
#include <vcl/abstdlg.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

namespace sfx2
{
namespace
{
struct OpenDialog
{
    sal_uInt32 nGeneration;
    /// The view it was opened in; it goes away without closing the dialog
    /// when that view does.
    ViewShellId nViewShellId;
    VclPtr<VclAbstractDialog> pDialog;
    std::function<void()> aPutBack;
};

// Let go of, if still there, when VCL is, not later at exit.
std::optional<OpenDialog>* getOpenDialog()
{
    static tools::DeleteOnDeinit<std::optional<OpenDialog>> s_aOpenDialog{};
    return s_aOpenDialog.get();
}

sal_uInt32 g_nGeneration = 0;

bool isViewAlive(ViewShellId nViewShellId)
{
    for (SfxViewShell* pViewShell = SfxViewShell::GetFirst(false); pViewShell;
         pViewShell = SfxViewShell::GetNext(*pViewShell, false))
    {
        if (pViewShell->GetViewShellId() == nViewShellId)
            return true;
    }
    return false;
}
}

bool AutoCorrectDialog::MayChangeSettings(weld::Window* pParent)
{
    if (comphelper::COKit::mayChangeUserSettings())
        return true;

    // The settings have an owner, and it is someone else.
    std::shared_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(
        pParent, VclMessageType::Info, VclButtonsType::Ok,
        SfxResId(comphelper::COKit::getUserSettingsRole()
                         == comphelper::COKit::UserSettingsRole::OtherUser
                     ? STR_AUTOCORRECT_SETTINGS_OF_ANOTHER_USER
                     : STR_AUTOCORRECT_SETTINGS_NEED_SIGN_IN)));
    xInfoBox->runAsync(xInfoBox, [](sal_Int32) {});
    return false;
}

bool AutoCorrectDialog::MayOpen(weld::Window* pParent)
{
    if (!MayChangeSettings(pParent))
        return false;

    std::optional<OpenDialog>* pOpenDialog = getOpenDialog();
    if (!pOpenDialog || !*pOpenDialog)
        return true;

    if (isViewAlive((*pOpenDialog)->nViewShellId))
    {
        std::shared_ptr<weld::MessageDialog> xInfoBox(
            Application::CreateMessageDialog(pParent, VclMessageType::Info, VclButtonsType::Ok,
                                             SfxResId(STR_AUTOCORRECT_DIALOG_ALREADY_OPEN)));
        xInfoBox->runAsync(xInfoBox, [](sal_Int32) {});
        return false;
    }

    OpenDialog aGivenUp = std::move(**pOpenDialog);
    pOpenDialog->reset();
    if (aGivenUp.aPutBack)
        aGivenUp.aPutBack();
    // Disposing it lets go of its callback, and with that of the dialog itself.
    aGivenUp.pDialog->disposeOnce();
    return true;
}

sal_uInt32 AutoCorrectDialog::Opened(const VclPtr<VclAbstractDialog>& pDialog,
                                     std::function<void()> aPutBack)
{
    const SfxViewShell* pViewShell = SfxViewShell::Current();
    ++g_nGeneration;
    if (std::optional<OpenDialog>* pOpenDialog = getOpenDialog())
        *pOpenDialog = OpenDialog{ g_nGeneration,
                                   pViewShell ? pViewShell->GetViewShellId() : ViewShellId(-1),
                                   pDialog, std::move(aPutBack) };
    return g_nGeneration;
}

bool AutoCorrectDialog::Closed(sal_uInt32 nGeneration)
{
    std::optional<OpenDialog>* pOpenDialog = getOpenDialog();
    if (!pOpenDialog || !*pOpenDialog || (*pOpenDialog)->nGeneration != nGeneration)
        return false;

    pOpenDialog->reset();
    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
