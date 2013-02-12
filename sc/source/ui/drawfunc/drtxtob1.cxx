/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <svx/svxdlg.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/hyphenzoneitem.hxx>
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
    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

    SfxAbstractTabDialog* pDlg = pFact->CreateScCharDlg(  pViewData->GetDialogParent(), &rArgs,
                                                        pViewData->GetSfxDocShell(),RID_SCDLG_CHAR );
    OSL_ENSURE(pDlg, "Dialog create fail!");
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
    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

    SfxAbstractTabDialog* pDlg = pFact->CreateScParagraphDlg( pViewData->GetDialogParent(), &aNewAttr, RID_SCDLG_PARAGRAPH);
    OSL_ENSURE(pDlg, "Dialog create fail!");
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
