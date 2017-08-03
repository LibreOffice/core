/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_IDIALOGRENDERABLE_HXX
#define INCLUDED_VCL_IDIALOGRENDERABLE_HXX

#include <tools/gen.hxx>
#include <vcl/pointr.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>

#include "IDialogRenderable.hxx"

#include <map>

namespace vcl
{

typedef OUString DialogID;

class VCL_DLLPUBLIC IDialogRenderable
{
public:
    virtual ~IDialogRenderable();

    virtual void paintDialog(const DialogID& rDialogID, VirtualDevice &rDevice,
                             int& nOutputWidth, int& nOutputHeight) = 0;

    virtual void paintActiveFloatingWindow(const DialogID& rDialogID, VirtualDevice &rDevice,
                                           int& nOutputWidth, int& nOutputHeight) = 0;

    virtual void postDialogKeyEvent(const DialogID& rDialogID, int nType,
                                    int nCharCode, int nKeyCode) = 0;

    virtual void postDialogMouseEvent(const DialogID& rDialogID, int nType, int nX, int nY,
                                      int nCount, int nButtons, int nModifier) = 0;

    // Callbacks
    virtual void notifyDialogInvalidation(const DialogID& rDialogID) = 0;

    virtual void notifyDialogChild(const DialogID& rDialogID, const OUString& rAction, const Point& rPos) = 0;
};

} // namespace vcl

#endif // INCLUDED_VCL_IDIALOGRENDERABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
