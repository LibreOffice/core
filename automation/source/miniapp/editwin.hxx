/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: editwin.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:19:12 $
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
#ifndef _editwin
#define _editwin

#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif

#ifndef _SV_FLOATWIN_HXX
#include <vcl/floatwin.hxx>
#endif
#ifndef _SVEDIT_HXX
#include <svtools/svmedit.hxx>
#endif

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

