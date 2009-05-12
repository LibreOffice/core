/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dlgedfunc.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _BASCTL_DLGEDFUNC_HXX
#define _BASCTL_DLGEDFUNC_HXX

#include <vcl/timer.hxx>

class DlgEditor;
class Timer;
class MouseEvent;
class Point;

//============================================================================
// DlgEdFunc
//============================================================================

class DlgEdFunc /* : public LinkHdl */
{
protected:
    DlgEditor* pParent;
    Timer        aScrollTimer;

    DECL_LINK( ScrollTimeout, Timer * );
    void    ForceScroll( const Point& rPos );

public:
    DlgEdFunc( DlgEditor* pParent );
    virtual ~DlgEdFunc();

    virtual BOOL MouseButtonDown( const MouseEvent& rMEvt );
    virtual BOOL MouseButtonUp( const MouseEvent& rMEvt );
    virtual BOOL MouseMove( const MouseEvent& rMEvt );
    virtual BOOL KeyInput( const KeyEvent& rKEvt );
};

//============================================================================
// DlgEdFuncInsert
//============================================================================

class DlgEdFuncInsert : public DlgEdFunc
{
public:
    DlgEdFuncInsert( DlgEditor* pParent );
    ~DlgEdFuncInsert();

    virtual BOOL MouseButtonDown( const MouseEvent& rMEvt );
    virtual BOOL MouseButtonUp( const MouseEvent& rMEvt );
    virtual BOOL MouseMove( const MouseEvent& rMEvt );
};

//============================================================================
// DlgEdFuncSelect
//============================================================================

class DlgEdFuncSelect : public DlgEdFunc
{
protected:
    BOOL    bMarkAction;

public:
    DlgEdFuncSelect( DlgEditor* pParent );
    ~DlgEdFuncSelect();

    virtual BOOL MouseButtonDown( const MouseEvent& rMEvt );
    virtual BOOL MouseButtonUp( const MouseEvent& rMEvt );
    virtual BOOL MouseMove( const MouseEvent& rMEvt );
};


#endif //_BASCTL_DLGEDFUNC_HXX
