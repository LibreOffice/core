/*************************************************************************
 *
 *  $RCSfile: tabpopsh.cxx,v $
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

#pragma hdrstop

#define GLOBALOVERFLOW
#include "segmentc.hxx"
#include "tabpopsh.hxx"
#include "sc.hrc"

//#define ScTabPopShell
//#include "scslots.hxx"
#undef ShellClass
#define ShellClass ScTabPopShell
SFX_SLOTMAP(ScTabPopShell)
{
    SFX_SLOT( 0,0, DummyExec, DummyState, 0, SfxVoidItem )
};
SEG_EOFGLOBALS()

#pragma SEG_SEGCLASS(SFXMACROS_SEG,STARTWORK_CODE)

TYPEINIT1(ScTabPopShell,SfxShell);

//SFX_IMPL_IDL_INTERFACE(ScTabPopShell, SfxShell, 0)
SFX_IMPL_INTERFACE(ScTabPopShell, SfxShell, ResId( 0, NULL))
{
    SFX_POPUPMENU_REGISTRATION( RID_POPUP_TAB );
}



#pragma SEG_FUNCDEF(tabpopsh_01)

ScTabPopShell::ScTabPopShell(SfxItemPool& rItemPool)
{
    SetPool( &rItemPool );
}

#pragma SEG_FUNCDEF(tabpopsh_02)

ScTabPopShell::~ScTabPopShell()
{
}

#pragma SEG_FUNCDEF(tabpopsh_03)

void ScTabPopShell::DummyExec( SfxRequest& rReq )
{
}

#pragma SEG_FUNCDEF(tabpopsh_04)

void ScTabPopShell::DummyState( SfxItemSet& rSet )
{
}


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.12  2000/09/17 14:09:34  willem.vandorp
    OpenOffice header added.

    Revision 1.11  2000/08/31 16:38:47  willem.vandorp
    Header and footer replaced

    Revision 1.10  1995/06/20 14:24:26  NN
    nochmal Segmentierung


      Rev 1.9   20 Jun 1995 16:24:26   NN
   nochmal Segmentierung

      Rev 1.8   13 Jun 1995 18:47:14   MO
   seg-pragmas korrigiert

      Rev 1.7   04 Apr 1995 19:22:16   TRI
   OS2 Anpassung

      Rev 1.6   24 Mar 1995 14:15:32   TRI
   Segmentierung

      Rev 1.5   21 Mar 1995 11:00:06   MO
   242-Aenderungen

      Rev 1.4   05 Mar 1995 05:19:46   TRI
   Mac Anpassung

      Rev 1.3   03 Mar 1995 23:02:28   NN
   2.41 Anpassung

      Rev 1.2   16 Feb 1995 10:33:10   NN
   Slotmap ohne IDL wegen OS2-Problemen

      Rev 1.1   15 Feb 1995 08:26:54   NN
   DummyExec

      Rev 1.0   14 Feb 1995 19:57:34   NN
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE


