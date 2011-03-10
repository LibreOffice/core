/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//------------------------------------------------------------------------

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <svx/svxdlg.hxx>
#include <editeng/brkitem.hxx>
#include <editeng/hyznitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/outliner.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/widwitem.hxx>
#include <sot/exchange.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/transfer.hxx>

#include "sc.hrc"
#include "drtxtob.hxx"
#include "drawview.hxx"
#include "viewdata.hxx"
#include "scresid.hxx"

#include "scabstdlg.hxx"
//------------------------------------------------------------------------

sal_Bool ScDrawTextObjectBar::ExecuteCharDlg( const SfxItemSet& rArgs,
                                                SfxItemSet& rOutSet )
{
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");

    SfxAbstractTabDialog* pDlg = pFact->CreateScCharDlg(  pViewData->GetDialogParent(), &rArgs,
                                                        pViewData->GetSfxDocShell(),RID_SCDLG_CHAR );
    DBG_ASSERT(pDlg, "Dialog create fail!");
    sal_Bool bRet = ( pDlg->Execute() == RET_OK );

    if ( bRet )
    {
        const SfxItemSet* pNewAttrs = pDlg->GetOutputItemSet();
        if ( pNewAttrs )
            rOutSet.Put( *pNewAttrs );
    }
    delete pDlg;

    return bRet;
}

sal_Bool ScDrawTextObjectBar::ExecuteParaDlg( const SfxItemSet& rArgs,
                                                SfxItemSet& rOutSet )
{
    SfxItemPool* pArgPool = rArgs.GetPool();
    SfxItemSet aNewAttr( *pArgPool,
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

    aNewAttr.Put( SvxHyphenZoneItem( false, SID_ATTR_PARA_HYPHENZONE ) );
    aNewAttr.Put( SvxFmtBreakItem( SVX_BREAK_NONE, SID_ATTR_PARA_PAGEBREAK ) );
    aNewAttr.Put( SvxFmtSplitItem( sal_True, SID_ATTR_PARA_SPLIT)  );
    aNewAttr.Put( SvxWidowsItem( 0, SID_ATTR_PARA_WIDOWS) );
    aNewAttr.Put( SvxOrphansItem( 0, SID_ATTR_PARA_ORPHANS) );

    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");

    SfxAbstractTabDialog* pDlg = pFact->CreateScParagraphDlg( pViewData->GetDialogParent(), &aNewAttr, RID_SCDLG_PARAGRAPH);
    DBG_ASSERT(pDlg, "Dialog create fail!");
    sal_Bool bRet = ( pDlg->Execute() == RET_OK );

    if ( bRet )
    {
        const SfxItemSet* pNewAttrs = pDlg->GetOutputItemSet();
        if ( pNewAttrs )
            rOutSet.Put( *pNewAttrs );
    }
    delete pDlg;

    return bRet;
}

void ScDrawTextObjectBar::ExecutePasteContents( SfxRequest & /* rReq */ )
{
    SdrView* pView = pViewData->GetScDrawView();
    OutlinerView* pOutView = pView->GetTextEditOutlinerView();
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    SfxAbstractPasteDialog* pDlg = pFact->CreatePasteDialog( pViewData->GetDialogParent() );

    pDlg->Insert( SOT_FORMAT_STRING, EMPTY_STRING );
    pDlg->Insert( SOT_FORMAT_RTF,    EMPTY_STRING );

    TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pViewData->GetActiveWin() ) );

    sal_uLong nFormat = pDlg->GetFormat( aDataHelper.GetTransferable() );

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
