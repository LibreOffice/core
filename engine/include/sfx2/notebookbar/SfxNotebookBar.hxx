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
namespace cpo::uno
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

    /// Creates the online notebookbar for the current view.
    static void ExecMethod(SfxBindings& rBindings);

    /// Creates the online notebookbar for the current view.
    static bool StateMethod(SfxBindings& rBindings);
    static bool StateMethod(SystemWindow* pSysWindow,
                            const cpo::uno::Reference<css::frame::XFrame>& xFrame);

    /// Creates the online notebookbar for the view when it has none yet. Returns whether the view
    /// has one afterwards. Only a desktop or tablet view gets one.
    static bool CreateForView(SfxViewShell& rViewShell);

    /// Creates the online notebookbar for the view from the main loop, once the work that is
    /// running now has finished. A view that has one already, or whose creation is pending, is
    /// left as it is.
    static void CreateForViewWhenIdle(SfxViewShell& rViewShell);

    /** Show menu bar in all frames of current application */
    static void ShowMenubar(bool bShow);
    static void ToggleMenubar();

private:
    static bool m_bLock;

    static void RemoveCurrentKitWrapper();

    DECL_DLLPRIVATE_STATIC_LINK(SfxNotebookBar, VclDisposeHdl, const SfxViewShell*, void);
    DECL_DLLPRIVATE_STATIC_LINK(SfxNotebookBar, CreateForViewHdl, void*, void);
};

} // namespace sfx2

#endif // INCLUDED_SFX2_NOTEBOOKBAR_SFXNOTEBOOKBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
