/*************************************************************************
 *
 *  $RCSfile: newstyle.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:31 $
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

// INCLUDE ---------------------------------------------------------------

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#pragma hdrstop

#include "newstyle.hxx"
#include "dialog.hrc"
#include "newstyle.hrc"
#include "sfxresid.hxx"
#include "segmentc.hxx"

// PRIVATE METHODES ------------------------------------------------------

IMPL_LINK( SfxNewStyleDlg, OKHdl, Control *, pControl )
{
    const String aName(aColBox.GetText());
    SfxStyleSheetBase *pStyle = rPool.Find(aName, rPool.GetSearchFamily(), SFXSTYLEBIT_ALL);
    if(pStyle) {
        if(!pStyle->IsUserDefined()) {
            InfoBox(this, SfxResId(MSG_POOL_STYLE_NAME)).Execute();
            return 0;
        }
        if(RET_YES == aQueryOverwriteBox.Execute()) {
            EndDialog(RET_OK);
        }
    }
    else {
        EndDialog(RET_OK);
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxNewStyleDlg, ModifyHdl, ComboBox *, pBox )
{
    aOKBtn.Enable( pBox->GetText().EraseAllChars().Len() > 0 );
    return 0;
}
IMPL_LINK_INLINE_END( SfxNewStyleDlg, ModifyHdl, ComboBox *, pBox )

// CTOR / DTOR -----------------------------------------------------------

SfxNewStyleDlg::SfxNewStyleDlg(Window *pParent, SfxStyleSheetBasePool &pool)
    : ModalDialog(pParent, SfxResId(DLG_NEW_STYLE_BY_EXAMPLE)),
    aColBox(this, ResId(LB_COL)),
    aColGrp(this, ResId(GB_COL)),
    aOKBtn(this, ResId(BT_OK)),
    aCancelBtn(this, ResId(BT_CANCEL)),
    aQueryOverwriteBox(this, ResId(MSG_OVERWRITE)),
    rPool(pool)
{
    FreeResource();
    aOKBtn.SetClickHdl(LINK(this, SfxNewStyleDlg, OKHdl));
    aColBox.SetModifyHdl(LINK(this, SfxNewStyleDlg, ModifyHdl));
    aColBox.SetDoubleClickHdl(LINK(this, SfxNewStyleDlg, OKHdl));

    SfxStyleSheetBase *pStyle = rPool.First();
    while(pStyle) {
        aColBox.InsertEntry(pStyle->GetName());
        pStyle = rPool.Next();
    }
}

// -----------------------------------------------------------------------

__EXPORT SfxNewStyleDlg::~SfxNewStyleDlg()
{
}


