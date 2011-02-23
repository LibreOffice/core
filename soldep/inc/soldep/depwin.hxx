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

#ifndef _FMRWRK_DEPWIN_HXX
#define _FMRWRK_DEPWIN_HXX

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#include <vcl/menu.hxx>
#include <svtools/scrwin.hxx>
#include <soldep/connctr.hxx>

class Depper;

class DepWin : public Window
{
private:
    ConnectorList   ConList;

    ObjectWin*      mpNewConWin;
    sal_Bool            mbStartNewCon;
    Point           maNewConStart;
    Point           maNewConEnd;
    ObjectWin*      mpSelectedProject;

public:
    PopupMenu*      mpPopup;
//  Depper*         mpDepperDontuseme;

                    DepWin( Window* pParent, WinBits nWinStyle );
                    ~DepWin();
    void            AddConnector( Connector* pNewCon );
    void            RemoveConnector( Connector* piOldCon );
    void            NewConnector( ObjectWin* pWin );
    ConnectorList*  GetConnectorList();
    void            ClearConnectorList() { ConList.Clear();}
    void            Paint( const Rectangle& rRect );
    void            MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    Command( const CommandEvent& rEvent);
//  void            Resize();
    void            MouseMove( const MouseEvent& rMEvt );
    sal_Bool            IsStartNewCon() { return mbStartNewCon; };
    void            SetPopupHdl( void* pHdl );
    void            SetSelectedProject( ObjectWin* object ) { mpSelectedProject = object; };
    ObjectWin*      GetSelectedProject() { return mpSelectedProject; };
//  DECL_LINK( PopupSelected, PopupMenu* );
    void    DrawOutput( OutputDevice* pDevice, const Point& rOffset  );
};

#endif
