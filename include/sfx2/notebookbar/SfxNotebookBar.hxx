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
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <vcl/notebookbar.hxx>

class SfxBindings;

namespace sfx2 {

/** Helpers for easier access to NotebookBar via the sfx2 infrastructure.
*/
class SFX2_DLLPUBLIC SfxNotebookBar
{
public:
    static void CloseMethod(SfxBindings& rBindings);
    static void CloseMethod(SystemWindow* pSysWindow);

    /// Function to be called from the sdi's ExecMethod.
    static void ExecMethod(SfxBindings& rBindings);

    /// Function to be called from the sdi's StateMethod.
    static void StateMethod(SfxBindings& rBindings, const OUString& rUIFile);
    static void StateMethod(SystemWindow* pSysWindow,
                            const css::uno::Reference<css::frame::XFrame> & xFrame,
                            const OUString& rUIFile);

    static void RemoveListeners(SystemWindow* pSysWindow);

    static void ShowMenubar(bool bShow);

private:
    static bool m_bLock;
    static css::uno::Reference<css::frame::XLayoutManager> m_xLayoutManager;
    static css::uno::Reference<css::frame::XFrame> m_xFrame;

    DECL_STATIC_LINK_TYPED(SfxNotebookBar, ToggleMenubar, NotebookBar*, void);
};

} // namespace sfx2

#endif // INCLUDED_SFX2_NOTEBOOKBAR_SFXNOTEBOOKBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
