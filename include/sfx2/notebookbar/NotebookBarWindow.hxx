/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_NOTEBOOKBAR_NOTEBOOKBARWINDOW_HXX
#define INCLUDED_SFX2_NOTEBOOKBAR_NOTEBOOKBARWINDOW_HXX

#include <vcl/builder.hxx>
#include <vcl/ctrl.hxx>
#include <sfx2/dockwin.hxx>

#include <sfx2/notebookbar/NotebookBarChildWindow.hxx>

/// This implements Widget Layout-based notebook-like menu bar.
class NotebookBarWindow : public SfxDockingWindow
{
public:
    NotebookBarWindow(Window* pParent,
                      sfx2::notebookbar::NotebookBarChildWindow& rChildWindow,
                      SfxBindings* pBindings,
                      const OString& rID,
                      const OUString& rUIXMLDescription);
    virtual ~NotebookBarWindow();
};

#endif // INCLUDED_SFX2_NOTEBOOKBAR_NOTEBOOKBARWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
