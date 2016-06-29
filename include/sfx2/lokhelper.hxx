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
#include <cstddef>
#include <rtl/string.hxx>

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
    static std::size_t getViews();

    /// Invoke the LOK callback of all views except pThisView, with a payload of rKey-rPayload.
    static void notifyOtherViews(SfxViewShell* pThisView, int nType, const OString& rKey, const OString& rPayload);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
