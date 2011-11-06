/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
