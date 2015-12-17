/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VCL_SOURCE_WINDOW_NOTEBOOKBARTAB_HXX
#define VCL_SOURCE_WINDOW_NOTEBOOKBARTAB_HXX

#include "menuwindow.hxx"

#include <vcl/tabpage.hxx>

/// This implements Widget Layout-based notebook-like toolbar.
class NotebookBarTab : public TabPage
{
public:
    NotebookBarTab(Window* pParent);

    OUString msTitle;
    OUString msId;
    OUString msHelpURL;
    // ContextList maContextList;
    bool mbIsEnabled;
    sal_Int32 mnOrderIndex;
    bool mbExperimental;
};

#endif // VCL_SOURCE_WINDOW_NOTEBOOKBARTAB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
