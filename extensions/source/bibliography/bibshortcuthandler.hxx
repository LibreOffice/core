/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bibshortcuthandler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:15:04 $
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

#ifndef _BIBSHORTCUTHANDLER_HXX
#define _BIBSHORTCUTHANDLER_HXX

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _SV_SPLITWIN_HXX
#include <vcl/splitwin.hxx>
#endif
#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif

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
