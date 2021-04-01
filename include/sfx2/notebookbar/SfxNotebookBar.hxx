/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_NOTEBOOKBAR_SFXNOTEBOOKBAR_HXX
#define INCLUDED_SFX2_NOTEBOOKBAR_SFXNOTEBOOKBAR_HXX

#include <sfx2/dllapi.h>
#include <rtl/ustring.hxx>
#include <vcl/WeldedTabbedNotebookbar.hxx>

#include <map>
#include <memory>
#include <string_view>

namespace com::sun::star::frame
{
class XFrame;
}
namespace com::sun::star::uno
{
template <typename> class Reference;
}

class SfxBindings;
class SfxViewFrame;
class SfxViewShell;
class SystemWindow;
class WeldedTabbedNotebookbar;

namespace sfx2
{
/** Helpers for easier access to NotebookBar via the sfx2 infrastructure.
*/
class SFX2_DLLPUBLIC SfxNotebookBar
{
public:
    static void CloseMethod(SfxBindings& rBindings);
    static void CloseMethod(SystemWindow* pSysWindow);

    static bool IsActive();

    /// Function to be called from the sdi's ExecMethod.
    static void ExecMethod(SfxBindings& rBindings, const OUString& rUIName);

    /// Function to be called from the sdi's StateMethod.
    static bool StateMethod(SfxBindings& rBindings, std::u16string_view rUIFile,
                            bool bReloadNotebookbar = false);
    static bool StateMethod(SystemWindow* pSysWindow,
                            const css::uno::Reference<css::frame::XFrame>& xFrame,
                            std::u16string_view rUIFile, bool bReloadNotebookbar = false);

    /// Method temporarily blocks showing of the NotebookBar
    static void LockNotebookBar();
    /// Method restores normal behaviour of the Notebookbar
    static void UnlockNotebookBar();

    static void RemoveListeners(SystemWindow const* pSysWindow);

    /** Show menu bar in all frames of current application */
    static void ShowMenubar(bool bShow);
    /** Show menu bar only in current frame */
    static void ShowMenubar(SfxViewFrame const* pViewFrame, bool bShow);
    static void ToggleMenubar();
    static void ReloadNotebookBar(std::u16string_view sUIPath);

private:
    static bool m_bLock;
    static bool m_bHide;

    static std::map<const SfxViewShell*, std::shared_ptr<WeldedTabbedNotebookbar>>
        m_pNotebookBarWeldedWrapper;

    DECL_STATIC_LINK(SfxNotebookBar, VclDisposeHdl, const SfxViewShell*, void);
};

} // namespace sfx2

#endif // INCLUDED_SFX2_NOTEBOOKBAR_SFXNOTEBOOKBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
