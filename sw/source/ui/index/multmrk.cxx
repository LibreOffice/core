/*************************************************************************
 *
 *  $RCSfile: multmrk.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:33:45 $
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

#include "swtypes.hxx"

#include "multmrk.hxx"
#include "toxmgr.hxx"

#include "index.hrc"
#include "multmrk.hrc"


SwMultiTOXMarkDlg::SwMultiTOXMarkDlg( Window* pParent, SwTOXMgr& rTOXMgr ) :

    SvxStandardDialog(pParent, SW_RES(DLG_MULTMRK)),

    aEntryFT(this,  SW_RES(FT_ENTRY)),
    aTextFT(this,   SW_RES(FT_TEXT)),
    aTOXFT(this,    SW_RES(FT_TOX)),
    aOkBT(this,     SW_RES(OK_BT)),
    aCancelBT(this, SW_RES(CANCEL_BT)),
    aTOXLB(this,    SW_RES(LB_TOX)),
    aTOXFL(this,    SW_RES(FL_TOX)),
    rMgr( rTOXMgr ),
    nPos(0)
{
    aTOXLB.SetSelectHdl(LINK(this, SwMultiTOXMarkDlg, SelectHdl));

    USHORT nSize = rMgr.GetTOXMarkCount();
    for(USHORT i=0; i < nSize; ++i)
        aTOXLB.InsertEntry(rMgr.GetTOXMark(i)->GetText());

    aTOXLB.SelectEntryPos(0);
    aTextFT.SetText(rMgr.GetTOXMark(0)->GetTOXType()->GetTypeName());

    FreeResource();
}


IMPL_LINK_INLINE_START( SwMultiTOXMarkDlg, SelectHdl, ListBox *, pBox )
{
    if(pBox->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND)
    {   SwTOXMark* pMark = rMgr.GetTOXMark(pBox->GetSelectEntryPos());
        aTextFT.SetText(pMark->GetTOXType()->GetTypeName());
        nPos = pBox->GetSelectEntryPos();
    }
    return 0;
}
IMPL_LINK_INLINE_END( SwMultiTOXMarkDlg, SelectHdl, ListBox *, pBox )


void SwMultiTOXMarkDlg::Apply()
{
    rMgr.SetCurTOXMark(nPos);
}

/*-----------------25.02.94 22:06-------------------
 dtor ueberladen
--------------------------------------------------*/


SwMultiTOXMarkDlg::~SwMultiTOXMarkDlg() {}



