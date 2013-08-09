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

#include <hintids.hxx>

#include <vcl/msgbox.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/hdft.hxx>
#include <editeng/flstitem.hxx>
#include <sfx2/htmlmode.hxx>
#include <svtools/htmlcfg.hxx>
#include <svl/cjkoptions.hxx>
#include <numpara.hxx>
#include <fmtclds.hxx>
#include <swtypes.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <wdocsh.hxx>
#include <viewopt.hxx>
#include <pgfnote.hxx>
#include <pggrid.hxx>
#include <tmpdlg.hxx>       // the dialog
#include <column.hxx>       // columns
#include <drpcps.hxx>       // initials
#include <frmpage.hxx>      // frames
#include <wrap.hxx>         // frames
#include <frmmgr.hxx>       // frames
#include <ccoll.hxx>        // CondColl
#include <swuiccoll.hxx>
#include <docstyle.hxx>     //
#include <fmtcol.hxx>       //
#include <macassgn.hxx>     //
#include <poolfmt.hxx>
#include <uitool.hxx>
#include <shellres.hxx>
#include <swabstdlg.hxx>

#include <cmdid.h>
#include <helpid.h>
#include <globals.hrc>
#include <SwStyleNameMapper.hxx>
#include <svx/svxids.hrc>
#include <svl/stritem.hxx>
#include <svl/aeitem.hxx>
#include <svl/slstitm.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>

extern SW_DLLPUBLIC SwWrtShell* GetActiveWrtShell();

/*--------------------------------------------------------------------
    Description:    the dialog's carrier
 --------------------------------------------------------------------*/

SwTemplateDlg::SwTemplateDlg(Window* pParent,
                             SfxStyleSheetBase& rBase,
                             sal_uInt16 nRegion,
                             OString sPage,
                             SwWrtShell* pActShell,
                             bool bNew)
    : SfxStyleDialog(pParent,
                    OString("TemplateDialog") + OString::number(nRegion),
                    OUString("modules/swriter/ui/templatedialog") +
                        OUString::number(nRegion) + OUString(".ui"),
                    rBase)
    , nType(nRegion)
    , pWrtShell(pActShell)
    , bNewStyle(bNew)
    , m_nIndentsId(0)
    , m_nAlignId(0)
    , m_nTextFlowId(0)
    , m_nAsianTypo(0)
    , m_nFontId(0)
    , m_nFontEffectId(0)
    , m_nPositionId(0)
    , m_nAsianLayoutId(0)
    , m_nTabId(0)
    , m_nOutlineId(0)
    , m_nDropCapsId(0)
    , m_nBackgroundId(0)
    , m_nBorderId(0)
    , m_nConditionId(0)
    , m_nTypeId(0)
    , m_nOptionsId(0)
    , m_nWrapId(0)
    , m_nColumnId(0)
    , m_nMacroId(0)
    , m_nHeaderId(0)
    , m_nFooterId(0)
    , m_nPageId(0)
    , m_nFootNoteId(0)
    , m_nTextGridId(0)
    , m_nSingleId(0)
    , m_nBulletId(0)
    , m_nNumId(0)
    , m_nBmpId(0)
    , m_nNumOptId(0)
    , m_nNumPosId(0)
{
    nHtmlMode = ::GetHtmlMode(pWrtShell->GetView().GetDocShell());
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");
    // tinker TabPages together
    switch( nRegion )
    {
        // character styles
        case SFX_STYLE_FAMILY_CHAR:
        {
            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ) , "GetTabPageRangesFunc fail!");
            m_nFontId = AddTabPage("font", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ));

            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ) , "GetTabPageRangesFunc fail!");
            m_nFontEffectId = AddTabPage("fonteffect", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ));

            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ) , "GetTabPageRangesFunc fail!");
            m_nPositionId = AddTabPage("position", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ));

            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ) , "GetTabPageRangesFunc fail!");
            m_nAsianLayoutId = AddTabPage("asianlayout", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ));

            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) , "GetTabPageRangesFunc fail!");
            m_nBackgroundId = AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ));

            SAL_WARN_IF(!pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "sw.ui", "GetTabPageCreatorFunc fail!");
            SAL_WARN_IF(!pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "sw.ui", "GetTabPageRangesFunc fail!");
            m_nBorderId = AddTabPage("borders", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ));

            SvtCJKOptions aCJKOptions;
            if(nHtmlMode & HTMLMODE_ON || !aCJKOptions.IsDoubleLinesEnabled())
                RemoveTabPage("asianlayout");
        }
        break;
        // paragraph styles
        case SFX_STYLE_FAMILY_PARA:
        {
            OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageRangesFunc fail!");
            m_nIndentsId = AddTabPage("indents", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH) );

            OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageRangesFunc fail!");
            m_nAlignId = AddTabPage("alignment", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH),      pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH) );

            OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageRangesFunc fail!");
            m_nTextFlowId = AddTabPage("textflow", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH) );

            OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageRangesFunc fail!");
            m_nAsianTypo = AddTabPage("asiantypo",  pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),       pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN) );

            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageRangesFunc fail!");
            m_nFontId = AddTabPage("font", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ) );

            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageRangesFunc fail!");
            m_nFontEffectId = AddTabPage("fonteffect", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ) );

            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ) , "GetTabPageRangesFunc fail!");
            m_nPositionId = AddTabPage("position", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ) );

            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ) , "GetTabPageRangesFunc fail!");
            m_nAsianLayoutId = AddTabPage("asianlayout", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ) );

            OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR), "GetTabPageRangesFunc fail!");
            m_nTabId = AddTabPage("tabs", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR) );

            m_nOutlineId = AddTabPage("outline", SwParagraphNumTabPage::Create,
                                    SwParagraphNumTabPage::GetRanges);
            m_nDropCapsId = AddTabPage("dropcaps", SwDropCapsPage::Create,
                                        SwDropCapsPage::GetRanges );
            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");
            m_nBackgroundId = AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) );
            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
            m_nBorderId = AddTabPage("borders", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );

            m_nConditionId = AddTabPage("condition", SwCondCollPage::Create,
                                        SwCondCollPage::GetRanges );
            if( (!bNewStyle && RES_CONDTXTFMTCOLL != ((SwDocStyleSheet&)rBase).GetCollection()->Which())
            || nHtmlMode & HTMLMODE_ON )
                RemoveTabPage("condition");

            SvtCJKOptions aCJKOptions;
            if(nHtmlMode & HTMLMODE_ON)
            {
                SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
                if (!rHtmlOpt.IsPrintLayoutExtension())
                    RemoveTabPage("textflow");
                RemoveTabPage("asiantypo");
                RemoveTabPage("tabs");
                RemoveTabPage("outline");
                RemoveTabPage("asianlayout");
                if(!(nHtmlMode & HTMLMODE_FULL_STYLES))
                {
                    RemoveTabPage("background");
                    RemoveTabPage("dropcaps");
                }
            }
            else
            {
                if(!aCJKOptions.IsAsianTypographyEnabled())
                    RemoveTabPage("asiantypo");
                if(!aCJKOptions.IsDoubleLinesEnabled())
                    RemoveTabPage("asianlayout");
            }
        }
        break;
        // frame styles
        case SFX_STYLE_FAMILY_FRAME:
        {
            m_nTypeId = AddTabPage("type", SwFrmPage::Create,
                                        SwFrmPage::GetRanges);
            m_nOptionsId = AddTabPage("options", SwFrmAddPage::Create,
                                        SwFrmAddPage::GetRanges);
            m_nWrapId = AddTabPage("wrap", SwWrapTabPage::Create,
                                        SwWrapTabPage::GetRanges);
            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");
            m_nBackgroundId = AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) );
            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
            m_nBorderId = AddTabPage("borders", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );

            m_nColumnId = AddTabPage("columns", SwColumnPage::Create,
                                        SwColumnPage::GetRanges );

            m_nMacroId = AddTabPage("macros", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_MACROASSIGN), 0);

        break;
        }
        // page styles
        case SFX_STYLE_FAMILY_PAGE:
        {
            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");
            m_nBackgroundId = AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) );
            m_nHeaderId = AddTabPage("header",  SvxHeaderPage::Create, SvxHeaderPage::GetRanges);
            m_nFooterId = AddTabPage("footer", SvxFooterPage::Create, SvxFooterPage::GetRanges);

            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_PAGE ), "GetTabPageRangesFunc fail!");
            m_nPageId = AddTabPage("page",
                                            pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ),
                                            pFact->GetTabPageRangesFunc( RID_SVXPAGE_PAGE ) );
            if(!pActShell || 0 == ::GetHtmlMode(pWrtShell->GetView().GetDocShell()))
            {
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
                m_nBorderId = AddTabPage("borders",
                                pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );
                m_nColumnId = AddTabPage("columns", SwColumnPage::Create, SwColumnPage::GetRanges );
                m_nFootNoteId = AddTabPage("footnotes",
                                                SwFootNotePage::Create,
                                                SwFootNotePage::GetRanges );
                m_nTextGridId = AddTabPage("textgrid",
                                                SwTextGridPage::Create,
                                                SwTextGridPage::GetRanges );
                SvtCJKOptions aCJKOptions;
                if(!aCJKOptions.IsAsianTypographyEnabled())
                    RemoveTabPage("textgrid");
            }
        }
        break;
        // numbering styles
        case SFX_STYLE_FAMILY_PSEUDO:
        {
            m_nSingleId = AddTabPage("numbering", RID_SVXPAGE_PICK_SINGLE_NUM);
            m_nBulletId = AddTabPage("bullets", RID_SVXPAGE_PICK_BULLET);
            m_nNumId = AddTabPage("outline", RID_SVXPAGE_PICK_NUM);
            m_nBmpId = AddTabPage("graphics", RID_SVXPAGE_PICK_BMP);
            m_nNumOptId = AddTabPage("options", RID_SVXPAGE_NUM_OPTIONS );
            m_nNumPosId = AddTabPage("position", RID_SVXPAGE_NUM_POSITION );
        }
        break;

        default:
            OSL_ENSURE(!this, "wrong family");
    }

    if (!sPage.isEmpty())
        SetCurPageId(sPage);
}

short SwTemplateDlg::Ok()
{
    short nRet = SfxTabDialog::Ok();
    if( RET_OK == nRet )
    {
        const SfxPoolItem *pOutItem, *pExItem;
        if( SFX_ITEM_SET == pExampleSet->GetItemState(
            SID_ATTR_NUMBERING_RULE, sal_False, &pExItem ) &&
            ( !GetOutputItemSet() ||
            SFX_ITEM_SET != GetOutputItemSet()->GetItemState(
            SID_ATTR_NUMBERING_RULE, sal_False, &pOutItem ) ||
            *pExItem != *pOutItem ))
        {
            if( GetOutputItemSet() )
                ((SfxItemSet*)GetOutputItemSet())->Put( *pExItem );
            else
                nRet = RET_CANCEL;
        }
    }
    else
        //JP 09.01.98 Bug #46446#:
        // that's the Ok-Handler, so OK has to be default!
        nRet = RET_OK;
    return nRet;
}

const SfxItemSet* SwTemplateDlg::GetRefreshedSet()
{
    SfxItemSet* pInSet = GetInputSetImpl();
    pInSet->ClearItem();
    pInSet->SetParent( &GetStyleSheet().GetItemSet() );
    return pInSet;
}

void SwTemplateDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    // set style's and metric's names
    OUString sNumCharFmt, sBulletCharFmt;
    SwStyleNameMapper::FillUIName( RES_POOLCHR_NUM_LEVEL, sNumCharFmt);
    SwStyleNameMapper::FillUIName( RES_POOLCHR_BUL_LEVEL, sBulletCharFmt);
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));

    if (nId == m_nFontId)
    {
        OSL_ENSURE(::GetActiveView(), "no active view");

        SvxFontListItem aFontListItem( *( (SvxFontListItem*)::GetActiveView()->
            GetDocShell()->GetItem( SID_ATTR_CHAR_FONTLIST ) ) );

        aSet.Put (SvxFontListItem( aFontListItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
        sal_uInt32 nFlags = 0;
        if(rPage.GetItemSet().GetParent() && 0 == (nHtmlMode & HTMLMODE_ON ))
            nFlags = SVX_RELATIVE_MODE;
        if( SFX_STYLE_FAMILY_CHAR == nType )
            nFlags = nFlags|SVX_PREVIEW_CHARACTER;
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, nFlags));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nFontEffectId)
    {
        sal_uInt32 nFlags = SVX_ENABLE_FLASH;
        if( SFX_STYLE_FAMILY_CHAR == nType )
            nFlags = nFlags|SVX_PREVIEW_CHARACTER;
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, nFlags));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nPositionId)
    {
        if( SFX_STYLE_FAMILY_CHAR == nType )
        {
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_PREVIEW_CHARACTER));
            rPage.PageCreated(aSet);
        }
    }
    else if (nId == m_nAsianLayoutId)
    {
        if( SFX_STYLE_FAMILY_CHAR == nType )
        {
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_PREVIEW_CHARACTER));
            rPage.PageCreated(aSet);
        }
    }
    else if (nId == m_nIndentsId)
    {
        if( rPage.GetItemSet().GetParent() )
        {
            aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_ABSLINEDIST,MM50/10));
            aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_FLAGSET,0x000F));
            rPage.PageCreated(aSet);
        }

    }
    else if (nId == m_nOutlineId)
    {
        //-->#outlinelevel added by zhaojianwei
        //  handle if the current paragraph style is assigned to a list level of outline style,
        SwTxtFmtColl* pTmpColl = pWrtShell->FindTxtFmtCollByName( GetStyleSheet().GetName() );
        if( pTmpColl && pTmpColl->IsAssignedToListLevelOfOutlineStyle() )
        {
            ((SwParagraphNumTabPage&)rPage).DisableOutline() ;
            ((SwParagraphNumTabPage&)rPage).DisableNumbering();
        }//<-end
        ListBox & rBox = ((SwParagraphNumTabPage&)rPage).GetStyleBox();
        SfxStyleSheetBasePool* pPool = pWrtShell->GetView().GetDocShell()->GetStyleSheetPool();
        pPool->SetSearchMask(SFX_STYLE_FAMILY_PSEUDO, SFXSTYLEBIT_ALL);
        const SfxStyleSheetBase* pBase = pPool->First();
        std::set<String> aNames;
        while(pBase)
        {
            aNames.insert(pBase->GetName());
            pBase = pPool->Next();
        }
        for(std::set<String>::const_iterator it = aNames.begin(); it != aNames.end(); ++it)
            rBox.InsertEntry(*it);
    }
    else if (nId == m_nAlignId)
    {
        aSet.Put(SfxBoolItem(SID_SVXPARAALIGNTABPAGE_ENABLEJUSTIFYEXT,sal_True));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nTypeId)
    {
        ((SwFrmPage&)rPage).SetNewFrame( sal_True );
        ((SwFrmPage&)rPage).SetFormatUsed( sal_True );
    }
    else if (nId == m_nOptionsId)
    {
        ((SwFrmAddPage&)rPage).SetFormatUsed(sal_True);
        ((SwFrmAddPage&)rPage).SetNewFrame(sal_True);
    }
    else if (nId == m_nWrapId)
    {
        ((SwWrapTabPage&)rPage).SetFormatUsed( sal_True, sal_False );
    }
    else if (nId == m_nColumnId)
    {
        if( nType == SFX_STYLE_FAMILY_FRAME )
            ((SwColumnPage&)rPage).SetFrmMode(sal_True);
        ((SwColumnPage&)rPage).SetFormatUsed( sal_True );
    }
    else if (nId == m_nBackgroundId)
    {
        sal_Int32 nFlagType = 0;
        if( SFX_STYLE_FAMILY_PARA == nType )
            nFlagType |= SVX_SHOW_PARACTL;
        if( SFX_STYLE_FAMILY_CHAR != nType )
            nFlagType |= SVX_SHOW_SELECTOR;
        if( SFX_STYLE_FAMILY_FRAME == nType )
            nFlagType |= SVX_ENABLE_TRANSPARENCY;
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, nFlagType));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nConditionId)
    {
        ((SwCondCollPage&)rPage).SetCollection(
            ((SwDocStyleSheet&)GetStyleSheet()).GetCollection(), bNewStyle );
    }
    else if (nId == m_nPageId)
    {
        if(0 == (nHtmlMode & HTMLMODE_ON ))
        {
            std::vector<OUString> aList;
            OUString aNew;
            SwStyleNameMapper::FillUIName( RES_POOLCOLL_TEXT, aNew );
            aList.push_back( aNew );
            if( pWrtShell )
            {
                SfxStyleSheetBasePool* pStyleSheetPool = pWrtShell->
                            GetView().GetDocShell()->GetStyleSheetPool();
                pStyleSheetPool->SetSearchMask(SFX_STYLE_FAMILY_PARA);
                SfxStyleSheetBase *pFirstStyle = pStyleSheetPool->First();
                while(pFirstStyle)
                {
                    aList.push_back( pFirstStyle->GetName() );
                    pFirstStyle = pStyleSheetPool->Next();
                }
            }
            aSet.Put (SfxStringListItem(SID_COLLECT_LIST, &aList));
            rPage.PageCreated(aSet);
        }
    }
    else if (nId == m_nMacroId)
    {
        SfxAllItemSet aNewSet(*aSet.GetPool());
        aNewSet.Put( SwMacroAssignDlg::AddEvents(MACASSGN_ALLFRM) );
        if ( pWrtShell )
            rPage.SetFrame( pWrtShell->GetView().GetViewFrame()->GetFrame().GetFrameInterface() );
        rPage.PageCreated(aNewSet);
    }
    else if (nId == m_nNumId)
    {
        aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFmt));
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFmt));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nNumOptId)
    {

        aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFmt));
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFmt));
        // collect character styles
        ListBox rCharFmtLB(this);
        rCharFmtLB.Clear();
        rCharFmtLB.InsertEntry( ViewShell::GetShellRes()->aStrNone );
        SwDocShell* pDocShell = ::GetActiveWrtShell()->GetView().GetDocShell();
        ::FillCharStyleListBox(rCharFmtLB,  pDocShell);

        std::vector<OUString> aList;
        for(sal_uInt16 j = 0; j < rCharFmtLB.GetEntryCount(); j++)
             aList.push_back( rCharFmtLB.GetEntry(j) );

        aSet.Put( SfxStringListItem( SID_CHAR_FMT_LIST_BOX,&aList ) ) ;
        FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebDocShell, pDocShell));
        aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM, static_cast< sal_uInt16 >(eMetric)));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nNumPosId)
    {
        SwDocShell* pDocShell = ::GetActiveWrtShell()->GetView().GetDocShell();
        FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebDocShell, pDocShell));

        aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM, static_cast< sal_uInt16 >(eMetric)));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nBulletId)
    {
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFmt));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nHeaderId)
    {
        if(0 == (nHtmlMode & HTMLMODE_ON ))
            ((SvxHeaderPage&)rPage).EnableDynamicSpacing();
    }
    else if (nId == m_nFooterId)
    {
        if(0 == (nHtmlMode & HTMLMODE_ON ))
            ((SvxFooterPage&)rPage).EnableDynamicSpacing();
    }
    else if (nId == m_nBorderId)
    {
        if( SFX_STYLE_FAMILY_PARA == nType )
        {
            aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,SW_BORDER_MODE_PARA));
        }
        else if( SFX_STYLE_FAMILY_FRAME == nType )
        {
            aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,SW_BORDER_MODE_FRAME));
        }
        rPage.PageCreated(aSet);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
