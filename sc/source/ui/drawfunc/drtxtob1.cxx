/*************************************************************************
 *
 *  $RCSfile: drtxtob1.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2001-08-09 15:22:22 $
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

//------------------------------------------------------------------------

#include "scitems.hxx"
#include "eetext.hxx"
#include <svx/brkitem.hxx>
#include <svx/eeitem.hxx>
#include <svx/hyznitem.hxx>
#include <svx/orphitem.hxx>
#include <svx/outliner.hxx>
#include <svx/spltitem.hxx>
#include <svx/widwitem.hxx>
#include <so3/pastedlg.hxx>
#include <sot/exchange.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/transfer.hxx>

#include "sc.hrc"
#include "drtxtob.hxx"
#include "drawview.hxx"
#include "viewdata.hxx"
#include "textdlgs.hxx"
#include "scresid.hxx"


//------------------------------------------------------------------------

BOOL ScDrawTextObjectBar::ExecuteCharDlg( const SfxItemSet& rArgs,
                                                SfxItemSet& rOutSet )
{
    ScCharDlg* pDlg = new ScCharDlg( pViewData->GetDialogParent(),
                                     &rArgs,
                                     pViewData->GetSfxDocShell() );

    BOOL bRet = ( pDlg->Execute() == RET_OK );

    if ( bRet )
    {
        const SfxItemSet* pNewAttrs = pDlg->GetOutputItemSet();
        if ( pNewAttrs )
            rOutSet.Put( *pNewAttrs );
    }
    delete pDlg;

    return bRet;
}

BOOL ScDrawTextObjectBar::ExecuteParaDlg( const SfxItemSet& rArgs,
                                                SfxItemSet& rOutSet )
{
    SfxItemPool* pPool = rArgs.GetPool();
    SfxItemSet aNewAttr( *pPool,
                            EE_ITEMS_START, EE_ITEMS_END,
                            SID_ATTR_PARA_HYPHENZONE, SID_ATTR_PARA_HYPHENZONE,
                            SID_ATTR_PARA_PAGEBREAK, SID_ATTR_PARA_PAGEBREAK,
                            SID_ATTR_PARA_SPLIT, SID_ATTR_PARA_SPLIT,
                            SID_ATTR_PARA_WIDOWS, SID_ATTR_PARA_WIDOWS,
                            SID_ATTR_PARA_ORPHANS, SID_ATTR_PARA_ORPHANS,
                            0 );
    aNewAttr.Put( rArgs );

    // Die Werte sind erst einmal uebernommen worden, um den Dialog anzuzeigen.
    // Muss natuerlich noch geaendert werden
    // aNewAttr.Put( SvxParaDlgLimitsItem( 567 * 50, 5670) );

    aNewAttr.Put( SvxHyphenZoneItem() );
    aNewAttr.Put( SvxFmtBreakItem() );
    aNewAttr.Put( SvxFmtSplitItem() );
    aNewAttr.Put( SvxWidowsItem() );
    aNewAttr.Put( SvxOrphansItem() );

    ScParagraphDlg* pDlg = new ScParagraphDlg( pViewData->GetDialogParent(),
                                               &aNewAttr );

    BOOL bRet = ( pDlg->Execute() == RET_OK );

    if ( bRet )
    {
        const SfxItemSet* pNewAttrs = pDlg->GetOutputItemSet();
        if ( pNewAttrs )
            rOutSet.Put( *pNewAttrs );
    }
    delete pDlg;

    return bRet;
}

void ScDrawTextObjectBar::ExecutePasteContents( SfxRequest &rReq )
{
    SdrView* pView = pViewData->GetScDrawView();
    OutlinerView* pOutView = pView->GetTextEditOutlinerView();
    SvPasteObjectDialog* pDlg = new SvPasteObjectDialog;

    pDlg->Insert( SOT_FORMAT_STRING, ScResId( SCSTR_CLIP_STRING ) );
    pDlg->Insert( SOT_FORMAT_RTF,    ScResId( SCSTR_CLIP_RTF ) );

    TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pViewData->GetActiveWin() ) );

    ULONG nFormat = pDlg->Execute( pViewData->GetDialogParent(), aDataHelper.GetTransferable() );

    //! test if outliner view is still valid

    if (nFormat == SOT_FORMAT_STRING)
        pOutView->Paste();
    else
        pOutView->PasteSpecial();
    delete pDlg;
}


