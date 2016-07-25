/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/notebookbar.hxx>
#include <vcl/menu.hxx>
#include <sfxlocal.hrc>
#include <sfx2/sfxresid.hxx>

class NotebookBarPopupMenu : public PopupMenu
{
public:
    explicit NotebookBarPopupMenu(ResId aRes);
    void Execute(NotebookBar* pNotebookbar,
                css::uno::Reference<css::frame::XFrame>& xFrame);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
