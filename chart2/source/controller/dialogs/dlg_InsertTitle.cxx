/*************************************************************************
 *
 *  $RCSfile: dlg_InsertTitle.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:25 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "dlg_InsertTitle.hxx"
#include "dlg_InsertTitle.hrc"

#include "ResId.hxx"
#include "SchSfxItemIds.hxx"

// header for class SfxBoolItem
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
// header for class SfxStringItem
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

InsertTitleDialogData::InsertTitleDialogData()
        : aPossibilityList(5)
        , aExistenceList(5)
        , aTextList(5)
{
    for(sal_Int32 nN=5;nN--;)
        aPossibilityList[nN]=true;
    for(nN=5;nN--;)
        aExistenceList[nN]=false;
}


SchTitleDlg::SchTitleDlg(Window* pWindow, const InsertTitleDialogData& rInput ) :
    ModalDialog(pWindow, SchResId(DLG_TITLE)),
    aCbxMain(this, ResId(CBX_MAINTITLE)),
    aEdtMain(this, ResId(EDT_MAINTITLE)),
    aCbxSub(this, ResId(CBX_SUBTITLE)),
    aEdtSub(this, ResId(EDT_SUBTITLE)),
    aCbxXAxis(this, ResId(CBX_TITLE_X_AXIS)),
    aEdtXAxis(this, ResId(EDT_X_AXIS)),
    aCbxYAxis(this, ResId(CBX_TITLE_Y_AXIS)),
    aEdtYAxis(this, ResId(EDT_Y_AXIS)),
    aCbxZAxis(this, ResId(CBX_TITLE_Z_AXIS)),
    aEdtZAxis(this, ResId(EDT_Z_AXIS)),
    aBtnOK(this, ResId(BTN_OK)),
    aBtnCancel(this, ResId(BTN_CANCEL)),
    aBtnHelp(this, ResId(BTN_HELP))
{
    FreeResource();

    aCbxMain.SetClickHdl(LINK(this, SchTitleDlg, EnableTitleHdl));
    aCbxSub.SetClickHdl(LINK(this, SchTitleDlg, EnableTitleHdl));
    aCbxXAxis.SetClickHdl(LINK(this, SchTitleDlg, EnableTitleHdl));
    aCbxYAxis.SetClickHdl(LINK(this, SchTitleDlg, EnableTitleHdl));
    aCbxZAxis.SetClickHdl(LINK(this, SchTitleDlg, EnableTitleHdl));

    aCbxMain.Check( rInput.aExistenceList[0] );
    aCbxSub.Check( rInput.aExistenceList[1] );
    aCbxXAxis.Check( rInput.aExistenceList[2] );
    aCbxYAxis.Check( rInput.aExistenceList[3] );
    aCbxZAxis.Check( rInput.aExistenceList[4] );

    aCbxMain.Enable( rInput.aPossibilityList[0] );
    aEdtMain.Enable( rInput.aPossibilityList[0] && aCbxMain.IsChecked());
    aCbxSub.Enable( rInput.aPossibilityList[1] );
    aEdtSub.Enable( rInput.aPossibilityList[1] && aCbxSub.IsChecked());
    aCbxXAxis.Enable( rInput.aPossibilityList[2] );
    aEdtXAxis.Enable( rInput.aPossibilityList[2] && aCbxXAxis.IsChecked());
    aCbxYAxis.Enable( rInput.aPossibilityList[3] );
    aEdtYAxis.Enable( rInput.aPossibilityList[3] && aCbxYAxis.IsChecked());
    aCbxZAxis.Enable( rInput.aPossibilityList[4] );
    aEdtZAxis.Enable( rInput.aPossibilityList[4] && aCbxZAxis.IsChecked());

    aEdtMain.SetText(rInput.aTextList[0]);
    aEdtSub.SetText(rInput.aTextList[1]);
    aEdtXAxis.SetText(rInput.aTextList[2]);
    aEdtYAxis.SetText(rInput.aTextList[3]);
    aEdtZAxis.SetText(rInput.aTextList[4]);
}

SchTitleDlg::~SchTitleDlg()
{
}

IMPL_LINK( SchTitleDlg, EnableTitleHdl, CheckBox *, pCbx )
{
    if (pCbx == &aCbxMain)
    {
        aEdtMain.Enable(aCbxMain.IsChecked());
    }
    else if (pCbx == &aCbxSub)
    {
        aEdtSub.Enable(aCbxSub.IsChecked());
    }
    else if (pCbx == &aCbxXAxis)
    {
        aEdtXAxis.Enable(aCbxXAxis.IsChecked());
    }
    else if (pCbx == &aCbxYAxis)
    {
        aEdtYAxis.Enable(aCbxYAxis.IsChecked());
    }
    else if (pCbx == &aCbxZAxis)
    {
        aEdtZAxis.Enable(aCbxZAxis.IsChecked());
    }
    return 0;
}

void SchTitleDlg::getResult( InsertTitleDialogData& rOutput )
{
    rOutput.aExistenceList[0] = aCbxMain.IsChecked()  && aEdtMain.GetText().Len();
    rOutput.aExistenceList[1] = aCbxSub.IsChecked()   && aEdtSub.GetText().Len();
    rOutput.aExistenceList[2] = aCbxXAxis.IsChecked() && aEdtXAxis.GetText().Len();
    rOutput.aExistenceList[3] = aCbxYAxis.IsChecked() && aEdtYAxis.GetText().Len();
    rOutput.aExistenceList[4] = aCbxZAxis.IsChecked() && aEdtZAxis.GetText().Len();

    rOutput.aTextList[0] = aEdtMain.GetText();
    rOutput.aTextList[1] = aEdtSub.GetText();
    rOutput.aTextList[2] = aEdtXAxis.GetText();
    rOutput.aTextList[3] = aEdtYAxis.GetText();
    rOutput.aTextList[4] = aEdtZAxis.GetText();
}

//.............................................................................
} //namespace chart
//.............................................................................

