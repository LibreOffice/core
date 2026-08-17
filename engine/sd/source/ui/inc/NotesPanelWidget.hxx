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

#include <editeng/editenginewidget.hxx>
#include <vcl/InterimItemWindow.hxx>

#include <memory>

namespace sd
{
class NotesPanelView;

/**
 * Hosts the speaker notes editor as a client-rendered custom widget.
 *
 * The window itself is never painted. It exists so that the jsdialog builder has a container to
 * hang the "notesedit" custom widget on, and it carries the "notespanel" JSON type that tells the
 * client which surface the messages belong to.
 */
class NotesPanelWidget final : public InterimItemWindow
{
private:
    NotesPanelView& mrNotesPanelView;
    std::unique_ptr<EditEngineWidgetController> mxController;
    std::unique_ptr<weld::CustomClientWeld> mxWidgetWeld;

    DECL_LINK(ContentChangedHdl, LinkParamNone*, void);

public:
    NotesPanelWidget(vcl::Window* pParent, NotesPanelView& rNotesPanelView,
                     sal_uInt64 nKitWindowId);
    virtual ~NotesPanelWidget() override;
    virtual void dispose() override;

    /// Sends the whole widget tree, which the client needs before it can accept single updates.
    void SendInitialUpdate();
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
