/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
//CHINA001 #include "textdlgs.hxx"
#include "scresid.hxx"

#include "scabstdlg.hxx" //CHINA00
//------------------------------------------------------------------------

sal_Bool ScDrawTextObjectBar::ExecuteCharDlg( const SfxItemSet& rArgs,
                                                SfxItemSet& rOutSet , sal_uInt16 nSlot)
{
//CHINA001  ScCharDlg* pDlg = new ScCharDlg( pViewData->GetDialogParent(),
//CHINA001  &rArgs,
//CHINA001  pViewData->GetSfxDocShell() );
//CHINA001
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

    SfxAbstractTabDialog* pDlg = pFact->CreateScCharDlg(  pViewData->GetDialogParent(), &rArgs,
                                                        pViewData->GetSfxDocShell(),RID_SCDLG_CHAR );
    DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001
    if (nSlot == SID_CHAR_DLG_EFFECT)
    {
        pDlg->SetCurPageId(RID_SVXPAGE_CHAR_EFFECTS);
    }
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

    aNewAttr.Put( SvxHyphenZoneItem( sal_False, SID_ATTR_PARA_HYPHENZONE ) );
    aNewAttr.Put( SvxFmtBreakItem( SVX_BREAK_NONE, SID_ATTR_PARA_PAGEBREAK ) );
    aNewAttr.Put( SvxFmtSplitItem( sal_True, SID_ATTR_PARA_SPLIT)  );
    aNewAttr.Put( SvxWidowsItem( 0, SID_ATTR_PARA_WIDOWS) );
    aNewAttr.Put( SvxOrphansItem( 0, SID_ATTR_PARA_ORPHANS) );

//CHINA001  ScParagraphDlg* pDlg = new ScParagraphDlg( pViewData->GetDialogParent(),
//CHINA001  &aNewAttr );
//CHINA001
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

    SfxAbstractTabDialog* pDlg = pFact->CreateScParagraphDlg( pViewData->GetDialogParent(), &aNewAttr, RID_SCDLG_PARAGRAPH);
    DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001
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


