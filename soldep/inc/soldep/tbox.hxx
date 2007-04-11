/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tbox.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:40:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SOLDEPTBOX_HXX
#define _SOLDEPTBOX_HXX

#include <vcl/toolbox.hxx>
#include <vcl/menu.hxx>

class SoldepToolBox : public ToolBox
{
protected:
//  USHORT          nBuildServerToolBoxId;
    BOOL            bDockable;
    BOOL            bCloseMode;
    BOOL            bOldFloatMode;
    BOOL            bBoxIsVisible;
    BOOL            bPin;
    BOOL            bPinable;

    PopupMenu       aMenu;

    Rectangle       aOutRect;
    Rectangle       aInRect;

    Link            aResizeHdl;
    Link            aMouseDownHdl;

    Bitmap          aPinedBitmap;
    Bitmap          aUnpinedBitmap;

    void            InitContextMenu();

public:
                    SoldepToolBox( Window* pParent, const ResId& aId, BOOL bDAble = TRUE );
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
    virtual void    EndDocking( const Rectangle& rRect, BOOL bFloatMode );
    virtual BOOL    Close();
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

    BOOL            IsBoxVisible() { return bBoxIsVisible; }

    void            EnablePin( BOOL bEnable = TRUE ) { bPinable = bEnable; Invalidate(); }
    BOOL            GetPin();
    void            TogglePin();
    void            SetPin(BOOL bP);
    PopupMenu       *GetContextMenu();
    DECL_LINK( MenuSelectHdl, Menu * );
};

#endif
