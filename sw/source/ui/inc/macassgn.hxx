/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macassgn.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:39:07 $
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
#ifndef _MACASSGN_HXX
#define _MACASSGN_HXX

#ifndef _MACROPG_HXX //autogen
#include <sfx2/macropg.hxx>
#endif

class SwWrtShell;
class SvxMacroItem;

enum DlgEventType
{
    MACASSGN_TEXTBAUST,
    MACASSGN_GRAPHIC,
    MACASSGN_OLE,
    MACASSGN_FRMURL,
    MACASSGN_INETFMT,
    MACASSGN_ALLFRM
};

class SwMacroAssignDlg : public SfxMacroAssignDlg
{
public:
    SwMacroAssignDlg( Window* pParent, SfxItemSet& rSet, DlgEventType eType );
    virtual ~SwMacroAssignDlg();

    static void AddEvents( SfxMacroTabPage& rPg, DlgEventType eType );
    static BOOL INetFmtDlg( Window* pParent, SwWrtShell& rSh,
                            SvxMacroItem*& rpINetItem );
};




#endif

