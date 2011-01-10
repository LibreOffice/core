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

#ifndef _SOLDEPTBOX_HXX
#define _SOLDEPTBOX_HXX

#include <vcl/toolbox.hxx>
#include <vcl/menu.hxx>

class SoldepToolBox : public ToolBox
{
protected:
//  sal_uInt16          nBuildServerToolBoxId;
    sal_Bool            bDockable;
    sal_Bool            bCloseMode;
    sal_Bool            bOldFloatMode;
    sal_Bool            bBoxIsVisible;
    sal_Bool            bPin;
    sal_Bool            bPinable;

    PopupMenu       aMenu;

    Rectangle       aOutRect;
    Rectangle       aInRect;

    Link            aResizeHdl;
    Link            aMouseDownHdl;

    Bitmap          aPinedBitmap;
    Bitmap          aUnpinedBitmap;

    void            InitContextMenu();

public:
                    SoldepToolBox( Window* pParent, const ResId& aId, sal_Bool bDAble = sal_True );
                    ~SoldepToolBox();

    virtual void    Command( const CommandEvent& rCEvt);
    virtual void    CallContextMenu( Window *pWin, Point aPos );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    MouseButtonDown(const MouseEvent& rEvent);
    virtual void    MouseButtonUp(const MouseEvent& rEvent);
    virtual void    MouseMove(const MouseEvent& rEvent);

    virtual void    ToggleFloatingMode();
//  void            SetFloatingWindow( FloatingWindow* pFW) { ToolBox::mpFloatWin = pFW; }
    virtual void    StartDocking();
    virtual void    EndDocking( const Rectangle& rRect, sal_Bool bFloatMode );
    virtual sal_Bool    Close();
    virtual void    CloseDockingMode();

                    //Called when toolbar droped
    virtual void    Tracking( const TrackingEvent &rTEvt ) { Invalidate(); ToolBox::Tracking( rTEvt );}

    virtual void    Move();

    using           DockingWindow::SetPosSizePixel;
    void            SetPosSizePixel( const Point& rNewPos,
                                         const Size& rNewSize );

    void            SetDockingRects( const Rectangle& rOutRect,
                                         const Rectangle& rInRect );

    void            SetMouseDownHdl(const Link& rLink) { aMouseDownHdl = rLink; }
    Link            GetMouseDownHdl() { return aMouseDownHdl; }
    void            SetResizeHdl(const Link& rLink) { aResizeHdl = rLink; }

    sal_Bool            IsBoxVisible() { return bBoxIsVisible; }

    void            EnablePin( sal_Bool bEnable = sal_True ) { bPinable = bEnable; Invalidate(); }
    sal_Bool            GetPin();
    void            TogglePin();
    void            SetPin(sal_Bool bP);
    PopupMenu       *GetContextMenu();
    DECL_LINK( MenuSelectHdl, Menu * );
};

#endif
