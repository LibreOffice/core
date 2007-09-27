/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drwtxtsh.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:58:51 $
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
#ifndef _SWDRWTXTSH_HXX
#define _SWDRWTXTSH_HXX

#ifndef _SFX_SHELL_HXX //autogen
#include <sfx2/shell.hxx>
#endif
#include "shellid.hxx"

class SdrView;
class SwView;
class SwWrtShell;
class SfxModule;

class SwDrawTextShell: public SfxShell
{
    SwView      &rView;

    SdrView     *pSdrView;

    BOOL        bRotate : 1;
    BOOL        bSelMove: 1;

    void        SetAttrToMarked(const SfxItemSet& rAttr);
    void        InsertSymbol(SfxRequest& rReq);
    BOOL        IsTextEdit();
public:
    SFX_DECL_INTERFACE(SW_DRWTXTSHELL)
    TYPEINFO();

    SwView     &GetView() { return rView; }
    SwWrtShell &GetShell();

                SwDrawTextShell(SwView &rView);
    virtual     ~SwDrawTextShell();

    virtual SfxUndoManager*     GetUndoManager();

    void        StateDisableItems(SfxItemSet &);

    void        Execute(SfxRequest &);
    void        ExecDraw(SfxRequest &);
    void        GetState(SfxItemSet &);
    void        GetDrawTxtCtrlState(SfxItemSet&);

    void        ExecFontWork(SfxRequest& rReq);
    void        StateFontWork(SfxItemSet& rSet);
    void        ExecFormText(SfxRequest& rReq);
    void        GetFormTextState(SfxItemSet& rSet);
    void        ExecDrawLingu(SfxRequest &rReq);
    void        ExecUndo(SfxRequest &rReq);
    void        StateUndo(SfxItemSet &rSet);
    void        ExecClpbrd(SfxRequest &rReq);
    void        StateClpbrd(SfxItemSet &rSet);
    void        StateInsert(SfxItemSet &rSet);
    void        ExecTransliteration(SfxRequest &);

    void        Init();
    void        StateStatusline(SfxItemSet &rSet);
};

#endif
