/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bastype4.hxx,v $
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
#ifndef _BASTYPE4_HXX
#define _BASTYPE4_HXX


#include <svtools/tabbar.hxx>

class EditEngine;
class EditView;

class ExtendedTabBar : public TabBar
{
    EditEngine*     pEditEngine;
    EditView*       pEditView;
    BOOL            bIsInKeyInput;
#if _SOLAR__PRIVATE
    void            ImpCheckEditEngine( BOOL bKeepNewText );
#endif
protected:
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    LoseFocus();
    virtual void    KeyInput( const KeyEvent& rKEvent );
    virtual void    Paint( const Rectangle& );

    virtual BOOL    StartRenamingTab( USHORT nCurId );
    virtual BOOL    AllowRenamingTab( USHORT nCurId, const String& rNewName );
    virtual void    TabRenamed( USHORT nCurId, const String& rNewName );

public:
                    ExtendedTabBar( Window* pParent, WinBits nStyle );
                    ~ExtendedTabBar();

    void            RenameSelectedTab();
    BOOL            IsInEditMode() const { return pEditEngine ? TRUE : FALSE; }
    void            StopEditMode( BOOL bKeepCurText = FALSE );
};

#endif  //_BASTYPE4_HXX


