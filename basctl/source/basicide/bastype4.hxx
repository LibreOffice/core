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
#ifndef _BASTYPE4_HXX
#define _BASTYPE4_HXX


#include <svtools/tabbar.hxx>

class EditEngine;
class EditView;

class ExtendedTabBar : public TabBar
{
    EditEngine*     pEditEngine;
    EditView*       pEditView;
    sal_Bool            bIsInKeyInput;
#if _SOLAR__PRIVATE
    void            ImpCheckEditEngine( sal_Bool bKeepNewText );
#endif
protected:
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    LoseFocus();
    virtual void    KeyInput( const KeyEvent& rKEvent );
    virtual void    Paint( const Rectangle& );

    virtual sal_Bool    StartRenamingTab( sal_uInt16 nCurId );
    virtual sal_Bool    AllowRenamingTab( sal_uInt16 nCurId, const String& rNewName );
    virtual void    TabRenamed( sal_uInt16 nCurId, const String& rNewName );

public:
                    ExtendedTabBar( Window* pParent, WinBits nStyle );
                    ~ExtendedTabBar();

    void            RenameSelectedTab();
    sal_Bool            IsInEditMode() const { return pEditEngine ? sal_True : sal_False; }
    void            StopEditMode( sal_Bool bKeepCurText = sal_False );
};

#endif  //_BASTYPE4_HXX


