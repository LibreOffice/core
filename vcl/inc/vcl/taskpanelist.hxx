/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: taskpanelist.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:14:20 $
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

#ifndef _SV_TASKPANELIST_HXX
#define _SV_TASKPANELIST_HXX

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#include <vector>

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

class VCL_DLLPUBLIC TaskPaneList
{
    ::std::vector<Window *> mTaskPanes;
    Window *FindNextPane( Window *pWindow, BOOL bForward = TRUE );
    Window *FindNextFloat( Window *pWindow, BOOL bForward = TRUE );
    Window *FindNextSplitter( Window *pWindow, BOOL bForward = TRUE );

//#if 0 // _SOLAR__PRIVATE
public:
    BOOL IsInList( Window *pWindow );
//#endif

public:
    TaskPaneList();
    ~TaskPaneList();

    void AddWindow( Window *pWindow );
    void RemoveWindow( Window *pWindow );
    BOOL HandleKeyEvent( KeyEvent aKeyEvent );
};

#endif
