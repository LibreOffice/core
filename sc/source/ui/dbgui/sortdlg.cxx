/*************************************************************************
 *
 *  $RCSfile: sortdlg.cxx,v $
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
#include <segmentc.hxx>
#endif

#include "tpsort.hxx"
#include "sortdlg.hxx"
#include "scresid.hxx"
#include "sortdlg.hrc"

#pragma hdrstop

SEG_EOFGLOBALS()

//==================================================================
#pragma SEG_FUNCDEF(sortdlg_01)

ScSortDlg::ScSortDlg( Window*           pParent,
                      const SfxItemSet* pArgSet ) :
        SfxTabDialog( pParent,
                      ScResId( RID_SCDLG_SORT ),
                      pArgSet ),
        bIsHeaders  ( FALSE ),
        bIsByRows   ( FALSE )

{
    AddTabPage( TP_FIELDS,  ScTabPageSortFields::Create,  0 );
    AddTabPage( TP_OPTIONS, ScTabPageSortOptions::Create, 0 );
    FreeResource();
}

#pragma SEG_FUNCDEF(sortdlg_02)

__EXPORT ScSortDlg::~ScSortDlg()
{
}

// -----------------------------------------------------------------------

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.11  2000/09/17 14:08:57  willem.vandorp
    OpenOffice header added.

    Revision 1.10  2000/08/31 16:38:21  willem.vandorp
    Header and footer replaced

    Revision 1.9  1996/10/29 13:04:12  NN
    ueberall ScResId statt ResId


      Rev 1.8   29 Oct 1996 14:04:12   NN
   ueberall ScResId statt ResId

      Rev 1.7   27 Jan 1995 12:37:44   TRI
   __EXPORT bei virtuellen Methoden eingebaut

      Rev 1.6   25 Jan 1995 18:23:28   MO
   tpsort.hxx-include

      Rev 1.5   24 Jan 1995 21:04:08   MO
   TP_FIELDS/TP_OPTIONS anstatt Res-Ids der Pages

      Rev 1.4   22 Jan 1995 15:10:12   SC
   2.37 Anpassung

      Rev 1.3   18 Jan 1995 13:56:24   TRI
   Pragmas zur Segementierung eingebaut

      Rev 1.2   21 Dec 1994 18:17:52   MO
   Dialog fungiert als Vermittler zwischen den beiden TabPages.
   -> zusaetzliche Datenmember+Zugriffsfunktionen

      Rev 1.1   14 Dec 1994 13:51:58   MO
   Globale ResIds jetzt in sc.hrc

      Rev 1.0   13 Dec 1994 20:51:10   MO
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE

