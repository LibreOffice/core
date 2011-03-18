/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SD_PRESENTATION_VIEW_SHELL_HXX
#define SD_PRESENTATION_VIEW_SHELL_HXX

#ifndef SD_DRAW_VIEW_SHELL
#include "DrawViewShell.hxx"
#endif

namespace sd {

/** This view shell is responsible for showing the presentation of an
    Impress document.
*/
class PresentationViewShell : public DrawViewShell
{
public:
    TYPEINFO();

    SFX_DECL_VIEWFACTORY(PresViewShell);
    SFX_DECL_INTERFACE( SD_IF_SDPRESVIEWSHELL )

    PresentationViewShell( SfxViewFrame* pFrame, ViewShellBase& rViewShellBase, ::Window* pParentWindow, FrameView* pFrameView = NULL);
    virtual ~PresentationViewShell (void);

    /** This method is used by a simple class that passes some
        arguments from the creator of the new view shell to the new view
        shell object by waiting for its asynchronous creation.
        @param pFrameView
            The frame view that is typically used by the creating object and
            that shall be shared by the created view shell.
    */
    void FinishInitialization( FrameView* pFrameView );

    virtual void Resize (void);

protected:
    virtual SvxRuler* CreateHRuler(::sd::Window* pWin, sal_Bool bIsFirst);
    virtual SvxRuler* CreateVRuler(::sd::Window* pWin);

private:
    Rectangle       maOldVisArea;

    virtual void Activate (sal_Bool bIsMDIActivate);
    virtual void Paint (const Rectangle& rRect, ::sd::Window* pWin);
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
