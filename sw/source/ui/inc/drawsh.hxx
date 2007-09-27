/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawsh.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:57:50 $
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
#ifndef _SWDRAWSH_HXX
#define _SWDRAWSH_HXX

#include "drwbassh.hxx"

class SwDrawBaseShell;

class SwDrawShell: public SwDrawBaseShell
{
public:
    SFX_DECL_INTERFACE(SW_DRAWSHELL)
    TYPEINFO();

                SwDrawShell(SwView &rView);

    void        Execute(SfxRequest &);
    void        GetState(SfxItemSet &);
    void        ExecDrawDlg(SfxRequest& rReq);
    void        ExecDrawAttrArgs(SfxRequest& rReq);
    void        GetDrawAttrState(SfxItemSet &rSet);

    void        ExecFormText(SfxRequest& rReq);
    void        GetFormTextState(SfxItemSet& rSet);
};

#endif
