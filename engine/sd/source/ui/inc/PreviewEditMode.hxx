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

#include <sddllapi.h>
#include <svl/lstner.hxx>
#include <unotools/weakref.hxx>

class SdrObject;
class SdrPageObj;

namespace sd
{
class DrawViewShell;

/** Per-view state for editing one slide through its preview on the canvas page.

    Holds the entered preview and at most one selected shape on the slide the preview shows,
    both as weak references so that deletion by any view leaves the mode in a safe, empty
    state. Listens on the document so that removal of either object, or a geometry change of
    the selected shape, is reported to the owning view at once.
*/
class PreviewEditMode final : public SfxListener
{
public:
    PreviewEditMode(DrawViewShell& rShell, SdrPageObj& rPreview);
    virtual ~PreviewEditMode() override;

    SdrPageObj* GetEnteredPreview() const;
    SD_DLLPUBLIC SdrObject* GetSelectedShape() const; // export for unit test
    SD_DLLPUBLIC void SetSelectedShape(SdrObject* pShape); // export for unit test

    void SendModeState(bool bEntered) const;
    void SendSelectionState() const;

    virtual void Notify(SfxBroadcaster& rBroadcaster, const SfxHint& rHint) override;

private:
    DrawViewShell& mrShell;
    ::unotools::WeakReference<SdrObject> mxPreview;
    ::unotools::WeakReference<SdrObject> mxSelectedShape;
};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
