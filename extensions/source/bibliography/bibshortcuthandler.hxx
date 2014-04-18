/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_EXTENSIONS_SOURCE_BIBLIOGRAPHY_BIBSHORTCUTHANDLER_HXX
#define INCLUDED_EXTENSIONS_SOURCE_BIBLIOGRAPHY_BIBSHORTCUTHANDLER_HXX

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
    virtual sal_Bool            HandleShortCutKey( const KeyEvent& rKeyEvent ); // returns true, if key was handled

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
