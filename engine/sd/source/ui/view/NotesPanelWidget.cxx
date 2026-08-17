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

#include <NotesPanelWidget.hxx>
#include <NotesPanelView.hxx>

#include <editeng/outliner.hxx>
#include <vcl/jsdialog/executor.hxx>

namespace sd
{
NotesPanelWidget::NotesPanelWidget(vcl::Window* pParent, NotesPanelView& rNotesPanelView,
                                   sal_uInt64 nKitWindowId)
    : InterimItemWindow(pParent, u"modules/simpress/ui/notespanel.ui"_ustr, u"NotesPanel"_ustr,
                        true, nKitWindowId)
    , mrNotesPanelView(rNotesPanelView)
{
    OutlinerView* pOutlinerView = rNotesPanelView.GetOutlinerView();
    if (!pOutlinerView)
        return;

    mxController = std::make_unique<EditEngineWidgetController>(pOutlinerView->GetEditView(),
                                                                &rNotesPanelView.GetOutliner());
    mxWidgetWeld
        = std::make_unique<weld::CustomClientWeld>(*m_xBuilder, u"notesedit"_ustr, *mxController);

    mrNotesPanelView.SetContentChangedHdl(LINK(this, NotesPanelWidget, ContentChangedHdl));

    InitControlBase(mxWidgetWeld->GetWidget());
}

NotesPanelWidget::~NotesPanelWidget() { disposeOnce(); }

void NotesPanelWidget::dispose()
{
    mrNotesPanelView.SetContentChangedHdl(Link<LinkParamNone*, void>());
    mxWidgetWeld.reset();
    mxController.reset();
    InterimItemWindow::dispose();
}

IMPL_LINK_NOARG(NotesPanelWidget, ContentChangedHdl, LinkParamNone*, void)
{
    if (mxController)
        mxController->QueueUpdate();
}

void NotesPanelWidget::SendInitialUpdate()
{
    if (weld::Widget* pWidget = mxWidgetWeld ? mxWidgetWeld->GetWidget() : nullptr)
        jsdialog::SendFullUpdate(*pWidget);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
