/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bastype4.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:01:19 $
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
#ifndef _BASTYPE4_HXX
#define _BASTYPE4_HXX


#ifndef _TABBAR_HXX //autogen
#include <svtools/tabbar.hxx>
#endif

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


