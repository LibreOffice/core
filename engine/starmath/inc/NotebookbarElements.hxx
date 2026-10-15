/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/frame/XFrame.hpp>
#include <svl/undo.hxx>

#include <functional>

class SfxViewShell;

namespace sm::notebookbar
{
/** Whether the view carries the Math elements in its notebookbar Formula tab.

    A formula edited in place in such a view has its elements there, so it
    leaves the sidebar of that view alone; anywhere else the elements are in
    the sidebar Elements deck. */
bool HostsElements(const SfxViewShell* pView);

/// Called by the notebookbar elements section as it comes and goes with its view.
void SetHostsElements(ViewShellId nViewId, bool bHosts);

/** The element category last shown for the formulas of the view, in its
    Formula tab or in its sidebar Elements panel, so that either one opens on
    the category the other was left on. 0 until one is chosen. */
int GetCategory(ViewShellId nViewId);

/// Remembers the category for the view and passes it on to its followers.
void SetCategory(ViewShellId nViewId, int nCategory);

/// An elements panel that is kept, hidden, while the Formula tab is in use
/// follows the category chosen there. pOwner identifies it for UnfollowCategory.
void FollowCategory(const void* pOwner, ViewShellId nViewId, std::function<void(int)> aFollow);
void UnfollowCategory(const void* pOwner);

/// The view of the document a formula is edited in place in, given the
/// frame the formula is edited in; nullptr when it is not edited in place.
SfxViewShell* GetHostView(const cpo::uno::Reference<css::frame::XFrame>& xFrame);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
