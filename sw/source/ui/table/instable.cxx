/*************************************************************************
 *
 *  $RCSfile: instable.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:46:26 $
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

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#include "wrtsh.hxx"
#include "view.hxx"
#include "itabenum.hxx"
#include "instable.hxx"
#include "tautofmt.hxx"
#include "tblafmt.hxx"
#include "modcfg.hxx"
#include "swmodule.hxx"
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif

#include "table.hrc"
#include "instable.hrc"

#define ROW_COL_PROD 16384

void SwInsTableDlg::GetValues( String& rName, USHORT& rRow, USHORT& rCol,
                                USHORT& rInsTblFlags, String& rAutoName,
                                SwTableAutoFmt *& prTAFmt )
{
    rName = aNameEdit.GetText();
    rRow = (USHORT)aRowEdit.GetValue();
    rCol = (USHORT)aColEdit.GetValue();

    rInsTblFlags = 0;
    if (aBorderCB.IsChecked())
        rInsTblFlags |= DEFAULT_BORDER;
    if (aHeaderCB.IsChecked())
        rInsTblFlags |= HEADLINE;
    if (aRepeatHeaderCB.IsEnabled() && aRepeatHeaderCB.IsChecked())
        rInsTblFlags |= REPEAT;
    if (!aDontSplitCB.IsChecked())
        rInsTblFlags |= SPLIT_LAYOUT;
    if( pTAutoFmt )
    {
        prTAFmt = new SwTableAutoFmt( *pTAutoFmt );
        rAutoName = prTAFmt->GetName();
    }
}

// CTOR / DTOR -----------------------------------------------------------


SwInsTableDlg::SwInsTableDlg( SwView& rView )
    : SfxModalDialog( rView.GetWindow(), SW_RES(DLG_INSERT_TABLE) ),
    aNameEdit       (this, SW_RES(ED_NAME)),
    aNameFT         (this, SW_RES(FT_NAME)),
    aColLbl         (this, SW_RES(FT_COL)),
    aColEdit        (this, SW_RES(ED_COL)),
    aRowLbl         (this, SW_RES(FT_ROW)),
    aRowEdit        (this, SW_RES(ED_ROW)),
    aFL             (this, SW_RES(FL_TABLE)),
    aHeaderCB       (this, SW_RES(CB_HEADER)),
    aRepeatHeaderCB (this, SW_RES(CB_REPEAT_HEADER)),
    aDontSplitCB    (this, SW_RES(CB_DONT_SPLIT)),
    aBorderCB       (this, SW_RES(CB_BORDER)),
    aOptionsFL      (this, SW_RES(FL_OPTIONS)),
    aOkBtn          (this, SW_RES(BT_OK)),
    aCancelBtn      (this, SW_RES(BT_CANCEL)),
    aHelpBtn        (this, SW_RES(BT_HELP)),
    aAutoFmtBtn     (this, SW_RES(BT_AUTOFORMAT)),
    pTAutoFmt( 0 ),
    pShell(&rView.GetWrtShell())
{
    FreeResource();
    aNameEdit.SetText(pShell->GetUniqueTblName());
    aNameEdit.SetModifyHdl(LINK(this, SwInsTableDlg, ModifyName));
    aColEdit.SetModifyHdl(LINK(this, SwInsTableDlg, ModifyRowCol));
    aRowEdit.SetModifyHdl(LINK(this, SwInsTableDlg, ModifyRowCol));

    aRowEdit.SetMax(ROW_COL_PROD/aColEdit.GetValue());
    aColEdit.SetMax(ROW_COL_PROD/aRowEdit.GetValue());
    aAutoFmtBtn.SetClickHdl(LINK(this, SwInsTableDlg, AutoFmtHdl));

    BOOL bHTMLMode = 0 != (::GetHtmlMode(rView.GetDocShell())&HTMLMODE_ON);
    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    USHORT nInsTblFlags = pModOpt->GetInsTblFlags(bHTMLMode);

    aHeaderCB.Check(nInsTblFlags & HEADLINE);
    aRepeatHeaderCB.Check(nInsTblFlags & REPEAT);
    if(bHTMLMode)
    {
        aDontSplitCB.Hide();
        aBorderCB.SetPosPixel(aDontSplitCB.GetPosPixel());
    }
    else
    {
        aDontSplitCB.Check(!(nInsTblFlags & SPLIT_LAYOUT));
    }
    aBorderCB.Check(nInsTblFlags & DEFAULT_BORDER);

    aHeaderCB.SetClickHdl(LINK(this, SwInsTableDlg, CheckBoxHdl));
    CheckBoxHdl();
}

SwInsTableDlg::~SwInsTableDlg()
{
    delete pTAutoFmt;
}

IMPL_LINK_INLINE_START( SwInsTableDlg, ModifyName, Edit *, pEdit )
{
    String sTblName = pEdit->GetText();
    if(sTblName.Search(' ') != STRING_NOTFOUND)
    {
        sTblName.EraseAllChars( );
        pEdit->SetText(sTblName);
    }

    aOkBtn.Enable(pShell->GetTblStyle( sTblName ) == 0);
    return 0;
}
IMPL_LINK_INLINE_END( SwInsTableDlg, ModifyName, Edit *, EMPTYARG )

/*-----------------15.04.98 11:36-------------------

--------------------------------------------------*/
IMPL_LINK( SwInsTableDlg, ModifyRowCol, NumericField *, pField )
{
    if(pField == &aColEdit)
    {
        long nCol = aColEdit.GetValue();
        if(!nCol)
            nCol = 1;
        aRowEdit.SetMax(ROW_COL_PROD/nCol);
    }
    else
    {
        long nRow = aRowEdit.GetValue();
        if(!nRow)
            nRow = 1;
        aColEdit.SetMax(ROW_COL_PROD/nRow);
    }
    return 0;
}

IMPL_LINK( SwInsTableDlg, AutoFmtHdl, PushButton*, pButton )
{
    SwAutoFormatDlg aDlg( pButton, pShell, FALSE, pTAutoFmt );
    if( RET_OK == aDlg.Execute())
        aDlg.FillAutoFmtOfIndex( pTAutoFmt );
    return 0;
}

/*********************************************************************/
/*                                                                   */
/*********************************************************************/

IMPL_LINK(SwInsTableDlg, CheckBoxHdl, CheckBox*, EMPTYARG)
{
    aRepeatHeaderCB.Enable(aHeaderCB.IsChecked());

    return 0;
}




