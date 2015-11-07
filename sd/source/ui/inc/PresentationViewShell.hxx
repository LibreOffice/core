/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SD_SOURCE_UI_INC_PRESENTATIONVIEWSHELL_HXX
#define INCLUDED_SD_SOURCE_UI_INC_PRESENTATIONVIEWSHELL_HXX

#include "DrawViewShell.hxx"

namespace sd {

/** This view shell is responsible for showing the presentation of an
    Impress document.
*/
class PresentationViewShell : public DrawViewShell
{
public:

    SFX_DECL_INTERFACE( SD_IF_SDPRESVIEWSHELL )

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    PresentationViewShell( SfxViewFrame* pFrame, ViewShellBase& rViewShellBase, vcl::Window* pParentWindow, FrameView* pFrameView = nullptr);
    virtual ~PresentationViewShell();

    /** This method is used by a simple class that passes some
        arguments from the creator of the new view shell to the new view
        shell object by waiting for its asynchronous creation.
        @param pFrameView
            The frame view that is typically used by the creating object and
            that shall be shared by the created view shell.
    */
    void FinishInitialization( FrameView* pFrameView );

    virtual void Resize() override;

protected:
    virtual SvxRuler* CreateHRuler(::sd::Window* pWin, bool bIsFirst) override;
    virtual SvxRuler* CreateVRuler(::sd::Window* pWin) override;

private:
    Rectangle       maOldVisArea;

    virtual void Activate (bool bIsMDIActivate) override;
    virtual void Paint (const Rectangle& rRect, ::sd::Window* pWin) override;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
