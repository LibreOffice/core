/*************************************************************************
 *
 *  $RCSfile: splittbl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:48 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _WRTSH_HXX //autogen
#include <wrtsh.hxx>
#endif
#ifndef _SPLITTBL_HXX
#include <splittbl.hxx>
#endif
#include <splittbl.hrc>
#include <table.hrc>
#ifndef _TBLENUM_HXX
#include <tblenum.hxx>
#endif
/*-----------------17.03.98 10:56-------------------

--------------------------------------------------*/
SwSplitTblDlg::SwSplitTblDlg( Window *pParent, SwWrtShell &rSh ) :
    SvxStandardDialog(pParent, SW_RES(DLG_SPLIT_TABLE)),
    aOKPB(                  this, ResId(PB_OK       )),
    aCancelPB(              this, ResId(PB_CANCEL   )),
    aHelpPB(                this, ResId(PB_HELP     )),
    aSplitGB(               this, ResId(GB_SPLIT    )),
    aCntntCopyRB(           this, ResId(RB_CNTNT    )),
    aBoxAttrCopyWithParaRB( this, ResId(RB_BOX_PARA )),
    aBoxAttrCopyNoParaRB(   this, ResId(RB_BOX_NOPARA)),
    aBorderCopyRB(          this, ResId(RB_BORDER   )),
    rShell(rSh)
{
    FreeResource();
    aCntntCopyRB.Check();
}

/*-----------------17.03.98 10:56-------------------

--------------------------------------------------*/
void SwSplitTblDlg::Apply()
{
    USHORT nSplit = HEADLINE_CNTNTCOPY;
    if(aBoxAttrCopyWithParaRB.IsChecked())
        nSplit = HEADLINE_BOXATRCOLLCOPY;
    if(aBoxAttrCopyNoParaRB.IsChecked())
        nSplit = HEADLINE_BOXATTRCOPY;
    else if(aBorderCopyRB.IsChecked())
        nSplit = HEADLINE_BORDERCOPY;

    rShell.SplitTable( nSplit );

}


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.4  2000/09/18 16:06:09  willem.vandorp
    OpenOffice header added.

    Revision 1.3  1998/03/20 12:39:40  OS
    verdrehter enum


      Rev 1.2   20 Mar 1998 13:39:40   OS
   verdrehter enum

      Rev 1.1   19 Mar 1998 11:15:38   OS
   Neuer Parameter: mit Vorlage

      Rev 1.0   17 Mar 1998 16:16:52   OS
   Initial revision.

------------------------------------------------------------------------*/

