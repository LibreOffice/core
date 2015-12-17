/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VCL_SOURCE_WINDOW_NOTEBOOKBARWINDOW_HXX
#define VCL_SOURCE_WINDOW_NOTEBOOKBARWINDOW_HXX

#include "menuwindow.hxx"

#include <vcl/tabctrl.hxx>

/// This implements Widget Layout-based notebook-like toolbar.
class NotebookBarWindow : public TabControl
{
public:
    NotebookBarWindow(Window* pParent);
    virtual ~NotebookBarWindow();
    virtual void dispose() SAL_OVERRIDE;

    virtual Size GetOptimalSize() const SAL_OVERRIDE;
    virtual void setPosSizePixel(long nX, long nY, long nWidth, long nHeight, PosSizeFlags nFlags = PosSizeFlags::All) SAL_OVERRIDE;

    void insertTabs();
};

#endif // VCL_SOURCE_WINDOW_NOTEBOOKBARWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
