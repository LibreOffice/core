/*************************************************************************
 *
 *  $RCSfile: pivotsh.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:09 $
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

//------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/srchitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <svtools/whiter.hxx>

#define GLOBALOVERFLOW
#include "segmentc.hxx"


#include "pivotsh.hxx"
#include "tabvwsh.hxx"
#include "scresid.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "sc.hrc"

//------------------------------------------------------------------------

#define ScPivotShell
#include "scslots.hxx"

//------------------------------------------------------------------------

SEG_EOFGLOBALS()


#pragma SEG_SEGCLASS(SFXMACROS_SEG,STARTWORK_CODE)


TYPEINIT1( ScPivotShell, SfxShell );

SFX_IMPL_INTERFACE(ScPivotShell, SfxShell, ScResId(SCSTR_PIVOTSHELL))
{
    SFX_POPUPMENU_REGISTRATION( ScResId(RID_POPUP_PIVOT) );
}


//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pivotsh_01)

ScPivotShell::ScPivotShell( ScTabViewShell* pViewSh ) :
    SfxShell(pViewSh),
    pViewShell( pViewSh )
{
    SetPool( &pViewSh->GetPool() );
    SetUndoManager( pViewSh->GetViewData()->GetSfxDocShell()->GetUndoManager() );
    SetHelpId( HID_SCSHELL_PIVOTSH );
    SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Pivot")));
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pivotsh_02)

ScPivotShell::~ScPivotShell()
{
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pivotsh_03)

void ScPivotShell::Execute( SfxRequest& rReq )
{
    switch ( rReq.GetSlot() )
    {
        case SID_PIVOT_RECALC:
            pViewShell->RecalcPivotTable();
            break;

        case SID_PIVOT_KILL:
            pViewShell->DeletePivotTable();
            break;
    }
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pivotsh_04)

void __EXPORT ScPivotShell::GetState( SfxItemSet& rSet )
{
    ScDocShell* pDocSh = pViewShell->GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();

    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        switch (nWhich)
        {
            case SID_PIVOT_RECALC:
            case SID_PIVOT_KILL:
                {
                    //! move ReadOnly check to idl flags

                    if ( pDocSh->IsReadOnly() || pDoc->GetChangeTrack()!=NULL )
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.18  2000/09/17 14:09:34  willem.vandorp
    OpenOffice header added.

    Revision 1.17  2000/08/31 16:38:46  willem.vandorp
    Header and footer replaced

    Revision 1.16  2000/05/09 18:30:15  nn
    use IMPL_INTERFACE macro without IDL

    Revision 1.15  2000/04/14 08:31:38  nn
    unicode changes

    Revision 1.14  2000/03/16 15:47:42  nn
    #74017# check ReadOnly state

    Revision 1.13  1999/06/02 19:42:10  ANK
    #66547# SubShells


      Rev 1.12   02 Jun 1999 21:42:10   ANK
   #66547# SubShells

      Rev 1.11   04 Sep 1998 18:24:48   ANK
   #55978# Slots disablen bei Redlining

      Rev 1.10   24 Nov 1997 20:04:54   NN
   includes

      Rev 1.9   04 Sep 1997 19:48:30   RG
   change header

      Rev 1.8   05 Aug 1997 14:31:18   TJ
   include svx/srchitem.hxx

      Rev 1.7   29 Oct 1996 13:36:00   NN
   ueberall ScResId statt ResId

      Rev 1.6   09 Aug 1996 20:34:46   NN
   Svx-Includes aus scitems.hxx raus

      Rev 1.5   16 Oct 1995 09:56:32   JN
   HelpIds fuer Shells

      Rev 1.4   01 Aug 1995 15:30:36   NN
   SetUndoManager

      Rev 1.3   03 Jul 1995 10:19:28   STE
   GetState exported

      Rev 1.2   20 Jun 1995 16:24:22   NN
   nochmal Segmentierung

      Rev 1.1   13 Jun 1995 18:43:52   MO
   seg-pragmas korrigiert

      Rev 1.0   30 May 1995 14:59:28   MO
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE


