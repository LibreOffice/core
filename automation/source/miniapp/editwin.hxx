/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: editwin.hxx,v $
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
#ifndef _editwin
#define _editwin

#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif
#include <vcl/floatwin.hxx>
#include <svtools/svmedit.hxx>

class GHEditWindow : public FloatingWindow
{

protected:

    MultiLineEdit   aInhalt;

    virtual BOOL Close(); // derived
    void Resize();

public:

    GHEditWindow();
    GHEditWindow(Window * pParent, String aName = CUniString("Neues Fenster"), WinBits iWstyle = WB_STDWORK);

    void Clear();
    void AddText( String aNew, BOOL bMoveToEnd = TRUE);
};



class EditFileWindow : public GHEditWindow
{

    String  aFileName;
    virtual BOOL Close(); // derived
    void LoadFile();

public:
    EditFileWindow(Window * pParent, String aName = CUniString("Neue Datei"), WinBits iWstyle = WB_STDWORK);

};

#endif

