/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: appedit.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _APPEDIT_HXX
#define _APPEDIT_HXX

#ifndef _SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif

#include "appwin.hxx"
#include "textedit.hxx"

class BasicFrame;

class AppEdit : public AppWin    {  // Editor window
using Window::Scroll;

public:
    ScrollBar *pVScroll;
    ScrollBar *pHScroll;
    void SetScrollBarRanges();
    ULONG nCurTextWidth;
private:
    void InitScrollBars();
protected:
    DECL_LINK( Scroll, ScrollBar* );
public:
    TYPEINFO();
    AppEdit( BasicFrame* );
    ~AppEdit();
    USHORT GetLineNr();                                 // Current line number
    FileType GetFileType();                             // Returns the file type
    virtual long InitMenu( Menu* );                     // Inits the menu
    virtual long DeInitMenu( Menu* );                   // Reset to enable all Shortcuts
    virtual void Command( const CommandEvent& rCEvt );  // Command Handler
    void Resize();
    void PostLoad();
    void PostSaveAs();
    void Mark( short, short, short );                   // Select text
    void Highlight( USHORT nLine, USHORT nCol1, USHORT nCol2 );
    virtual BOOL ReloadAllowed(){ return !StarBASIC::IsRunning(); }
    virtual void LoadIniFile();     // (re)load ini file after change
};

#endif
