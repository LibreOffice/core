/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_NOTEBOOKBAR_SFXNOTEBOOKBAR_HXX
#define INCLUDED_SFX2_NOTEBOOKBAR_SFXNOTEBOOKBAR_HXX

#include <sfx2/dllapi.h>
#include <rtl/ustring.hxx>
#include <vcl/notebookbar/notebookbar.hxx>
#include <vcl/EnumContext.hxx>

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

namespace sfx2
{
/** Helpers for easier access to NotebookBar via the sfx2 infrastructure.
*/
class SFX2_DLLPUBLIC SfxNotebookBar
{
public:
    static void CloseMethod(SfxBindings& rBindings);
    static void CloseMethod(SystemWindow* pSysWindow);

    static bool IsActive(bool bConsiderSingleToolbar = false);

    /// Stores the notebookbar UI file name for the current module and updates the notebookbar.
    static void ExecMethod(SfxBindings& rBindings, const OUString& rUIName);

    /// Creates the online notebookbar for the current view when it is active.
    static bool StateMethod(SfxBindings& rBindings);
    static bool StateMethod(SystemWindow* pSysWindow,
                            const css::uno::Reference<css::frame::XFrame>& xFrame);

    /** Show menu bar in all frames of current application */
    static void ShowMenubar(bool bShow);
    static void ToggleMenubar();

private:
    static bool m_bLock;

    static void ResetActiveToolbarModeToDefault(vcl::EnumContext::Application eApp);
    static void RemoveCurrentKitWrapper();

    DECL_DLLPRIVATE_STATIC_LINK(SfxNotebookBar, VclDisposeHdl, const SfxViewShell*, void);
};

} // namespace sfx2

#endif // INCLUDED_SFX2_NOTEBOOKBAR_SFXNOTEBOOKBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
