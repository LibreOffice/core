/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: splittbl.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 23:21:04 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif



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
    aSplitFL(               this, ResId(FL_SPLIT    )),
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



