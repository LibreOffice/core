/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drtxtob1.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 13:47:56 $
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
#include "precompiled_sc.hxx"



//------------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include "eetext.hxx"
#include <svx/svxdlg.hxx>
#include <svx/brkitem.hxx>
#include <svx/hyznitem.hxx>
#include <svx/orphitem.hxx>
#include <svx/outliner.hxx>
#include <svx/spltitem.hxx>
#include <svx/widwitem.hxx>
#include <sot/exchange.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/transfer.hxx>

#include "sc.hrc"
#include "drtxtob.hxx"
#include "drawview.hxx"
#include "viewdata.hxx"
//CHINA001 #include "textdlgs.hxx"
#include "scresid.hxx"

#include "scabstdlg.hxx" //CHINA00
//------------------------------------------------------------------------

BOOL ScDrawTextObjectBar::ExecuteCharDlg( const SfxItemSet& rArgs,
                                                SfxItemSet& rOutSet )
{
//CHINA001  ScCharDlg* pDlg = new ScCharDlg( pViewData->GetDialogParent(),
//CHINA001  &rArgs,
//CHINA001  pViewData->GetSfxDocShell() );
//CHINA001
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

    SfxAbstractTabDialog* pDlg = pFact->CreateScCharDlg(  pViewData->GetDialogParent(), &rArgs,
                                                        pViewData->GetSfxDocShell(),ResId(RID_SCDLG_CHAR) );
    DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001
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

//CHINA001  ScParagraphDlg* pDlg = new ScParagraphDlg( pViewData->GetDialogParent(),
//CHINA001  &aNewAttr );
//CHINA001
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

    SfxAbstractTabDialog* pDlg = pFact->CreateScParagraphDlg( pViewData->GetDialogParent(), &aNewAttr, ResId(RID_SCDLG_PARAGRAPH));
    DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001
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
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    SfxAbstractPasteDialog* pDlg = pFact->CreatePasteDialog( pViewData->GetDialogParent() );

    pDlg->Insert( SOT_FORMAT_STRING, EMPTY_STRING );
    pDlg->Insert( SOT_FORMAT_RTF,    EMPTY_STRING );

    TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pViewData->GetActiveWin() ) );

    ULONG nFormat = pDlg->GetFormat( aDataHelper.GetTransferable() );

    //! test if outliner view is still valid

    if (nFormat > 0)
    {
        if (nFormat == SOT_FORMAT_STRING)
            pOutView->Paste();
        else
            pOutView->PasteSpecial();
    }
    delete pDlg;
}


