/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: annotsh.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-19 13:55:58 $
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
#ifndef _SWANNOTSH_HXX
#define _SWANNOTSH_HXX

#include <sfx2/shell.hxx>
#include "shellid.hxx"
#include "swmodule.hxx"

class SwView;
class SwAnnotationShell: public SfxShell
{
    SwView&     rView;

public:
    SFX_DECL_INTERFACE(SW_ANNOTATIONSHELL)
    TYPEINFO();

                SwAnnotationShell(SwView&);
    virtual     ~SwAnnotationShell();

    void        StateDisableItems(SfxItemSet &);
    void        Exec(SfxRequest &);

    void        GetState(SfxItemSet &);
    void        StateInsert(SfxItemSet &rSet);

    void        NoteExec(SfxRequest &);
    void        GetNoteState(SfxItemSet &);

    void        ExecLingu(SfxRequest &rReq);
    void        GetLinguState(SfxItemSet &);

    void        ExecClpbrd(SfxRequest &rReq);
    void        StateClpbrd(SfxItemSet &rSet);

    void        ExecTransliteration(SfxRequest &);

    void        ExecUndo(SfxRequest &rReq);
    void        StateUndo(SfxItemSet &rSet);

    void        InsertSymbol(SfxRequest& rReq);

    virtual SfxUndoManager*     GetUndoManager();
};

#endif
