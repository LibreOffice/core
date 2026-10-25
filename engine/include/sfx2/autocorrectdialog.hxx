/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <functional>

#include <sfx2/dllapi.h>
#include <vcl/vclptr.hxx>

class VclAbstractDialog;
namespace weld
{
class Window;
}

namespace sfx2
{
/// The AutoCorrect Options dialog, which Writer opens on its own and Calc and
/// Impress through the application, works on process-wide settings: the
/// AutoCorrect configuration and lists, and in Writer the word completion list
/// and the AutoFormat flags pointing at a copy of it. Two of them open at a time,
/// as two views of the kit could, would overwrite each other on OK, so there is
/// one at a time.
class SFX2_DLLPUBLIC AutoCorrectDialog
{
public:
    /// Whether a new dialog may open now. While one is open in a view that is
    /// still there, tells the user so, with pParent as the parent of the
    /// message, and returns false. One whose view went away without closing it,
    /// so that its callback will not run, is given up: what it took is put
    /// back, and it is disposed.
    static bool MayOpen(weld::Window* pParent);

    /// Records pDialog, just created in the current view, as the open one, and
    /// aPutBack as what puts back what it took, should it be given up. Returns
    /// what tells it from a later one, for Closed().
    static sal_uInt32 Opened(const VclPtr<VclAbstractDialog>& pDialog,
                             std::function<void()> aPutBack);

    /// To be called first thing in the dialog's callback, and when it did not
    /// start. False when it has been given up: the callback is then only to
    /// dispose it.
    static bool Closed(sal_uInt32 nGeneration);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
