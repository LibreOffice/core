/*************************************************************************
 *
 *  $RCSfile: subtdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef PCH
#include "segmentc.hxx"
#endif

#include "tpsubt.hxx"
#include "scresid.hxx"
#include "subtdlg.hxx"
#include "subtdlg.hrc"

#pragma hdrstop

SEG_EOFGLOBALS()
//==================================================================
#pragma SEG_FUNCDEF(subtdlg_01)

ScSubTotalDlg::ScSubTotalDlg( Window*           pParent,
                              const SfxItemSet* pArgSet ) :
        SfxTabDialog( pParent,
                      ScResId( RID_SCDLG_SUBTOTALS ),
                      pArgSet ),
        aBtnRemove  ( this, ScResId( BTN_REMOVE ) )
{
    AddTabPage( PAGE_GROUP1,  ScTpSubTotalGroup1::Create,  0 );
    AddTabPage( PAGE_GROUP2,  ScTpSubTotalGroup2::Create,  0 );
    AddTabPage( PAGE_GROUP3,  ScTpSubTotalGroup3::Create,  0 );
    AddTabPage( PAGE_OPTIONS, ScTpSubTotalOptions::Create, 0 );
    aBtnRemove.SetClickHdl( LINK( this, ScSubTotalDlg, RemoveHdl ) );
    FreeResource();
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(subtdlg_02)

IMPL_LINK_INLINE_START( ScSubTotalDlg, RemoveHdl, PushButton *, pBtn )
{
    if ( pBtn == &aBtnRemove )
    {
        EndDialog( SCRET_REMOVE );
    }
    return 0;
}
IMPL_LINK_INLINE_END( ScSubTotalDlg, RemoveHdl, PushButton *, pBtn )

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.10  2000/09/17 14:08:57  willem.vandorp
    OpenOffice header added.

    Revision 1.9  2000/08/31 16:38:21  willem.vandorp
    Header and footer replaced

    Revision 1.8  1996/10/29 13:04:12  NN
    ueberall ScResId statt ResId


      Rev 1.7   29 Oct 1996 14:04:12   NN
   ueberall ScResId statt ResId

      Rev 1.6   29 Jan 1996 15:12:28   MO
   neuer Link

      Rev 1.5   08 Nov 1995 13:06:24   MO
   301-Aenderungen

      Rev 1.4   31 Jan 1995 16:46:32   TRI
   Pragmas zur Segmentierung eingebaut

      Rev 1.3   25 Jan 1995 18:28:16   MO
   tpsubt.hxx-include

      Rev 1.2   22 Jan 1995 15:10:14   SC
   2.37 Anpassung

      Rev 1.1   20 Jan 1995 18:33:24   MO
   Gruppen-TabPages 2 und 3 hinzugefuegt
   Remove-Button als Exit-Button mit Return-Wert SCRET_REMOVE

      Rev 1.0   19 Jan 1995 11:42:04   MO
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE

