/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_LOKHELPER_HXX
#define INCLUDED_SFX2_LOKHELPER_HXX

#include <sfx2/dllapi.h>
#include <sfx2/viewsh.hxx>
#include <cstddef>
#include <rtl/string.hxx>
#include <comphelper/lok.hxx>

class SfxViewShell;

class SFX2_DLLPUBLIC SfxLokHelper
{
public:
    /// Create a new view shell from the current view frame.
    static int createView();
    /// Destroy a view shell from the global shell list.
    static void destroyView(int nId);
    /// Set a view shell as current one.
    static void setView(int nId);
    /// Get the currently active view.
    static int getView(SfxViewShell* pViewShell = nullptr);
    /// Get the number of views of the current object shell.
    static std::size_t getViewsCount();
    /// Get viewIds of all existing views.
    static bool getViewIds(int* pArray, size_t nSize);
    /// Iterate over any view shell, except pThisViewShell, passing it to the f function.
    template<typename ViewShellType, typename FunctionType>
    static void forEachOtherView(ViewShellType* pThisViewShell, FunctionType f);
    /// Invoke the LOK callback of all views except pThisView, with a payload of rKey-rPayload.
    static void notifyOtherViews(SfxViewShell* pThisView, int nType, const OString& rKey, const OString& rPayload);
    /// Same as notifyOtherViews(), but works on a selected "other" view, not on all of them.
    static void notifyOtherView(SfxViewShell* pThisView, SfxViewShell const* pOtherView, int nType, const OString& rKey, const OString& rPayload);
    /// Emits a LOK_CALLBACK_DIALOG
    static void notifyDialog(const OUString& rPayload, const OUString& rAction);
    /// Emits a LOK_CALLBACK_DIALOG_CHILD
    static void notifyDialogChild(const OUString& rDialogID, const OUString& rAction, const Point& rPos);
    /// Emits a LOK_CALLBACK_INVALIDATE_TILES, but tweaks it according to setOptionalFeatures() if needed.
    static void notifyInvalidation(SfxViewShell* pThisView, const OString& rPayload);
    /// A special value to signify 'infinity'.
    /// This value is chosen such that sal_Int32 will not overflow when manipulated.
    static const long MaxTwips = 1e9;
};

template<typename ViewShellType, typename FunctionType>
void SfxLokHelper::forEachOtherView(ViewShellType* pThisViewShell, FunctionType f)
{
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        auto pOtherViewShell = dynamic_cast<ViewShellType*>(pViewShell);
        if (pOtherViewShell != nullptr && pOtherViewShell != pThisViewShell)
        {
            f(pOtherViewShell);
        }
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
