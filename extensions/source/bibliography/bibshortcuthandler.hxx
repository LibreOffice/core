/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bibshortcuthandler.hxx,v $
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

#ifndef _BIBSHORTCUTHANDLER_HXX
#define _BIBSHORTCUTHANDLER_HXX

#include <vcl/window.hxx>
#include <vcl/splitwin.hxx>
#include <vcl/tabpage.hxx>

// additional classes to handle shortcuts
// code in bibcont.cxx


class BibShortCutHandler
{
private:
    Window*                 pBaseClass;     // in cases, where BibShortCutHandler also has to be a window

protected:
    inline                  BibShortCutHandler( Window* pBaseClass );

public:
    virtual                 ~BibShortCutHandler();
    virtual BOOL            HandleShortCutKey( const KeyEvent& rKeyEvent ); // returns true, if key was handled

    inline Window*          GetWindow( void );
};

inline BibShortCutHandler::BibShortCutHandler( Window* _pBaseClass ) : pBaseClass( _pBaseClass )
{
}

inline Window* BibShortCutHandler::GetWindow( void )
{
    return pBaseClass;
}


class BibWindow : public Window, public BibShortCutHandler
{
public:
                            BibWindow( Window* pParent,WinBits nStyle = WB_3DLOOK);
    virtual                 ~BibWindow();
};


class BibSplitWindow : public SplitWindow, public BibShortCutHandler
{
public:
                            BibSplitWindow( Window* pParent,WinBits nStyle = WB_3DLOOK);
    virtual                 ~BibSplitWindow();
};


class BibTabPage : public TabPage, public BibShortCutHandler
{
public:
                            BibTabPage( Window* pParent, const ResId& rResId );
    virtual                 ~BibTabPage();
};

#endif
