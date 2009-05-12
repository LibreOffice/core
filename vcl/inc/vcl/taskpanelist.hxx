/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: taskpanelist.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_TASKPANELIST_HXX
#define _SV_TASKPANELIST_HXX

#include <vcl/dllapi.h>

#include <vector>
#include <vcl/window.hxx>

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
