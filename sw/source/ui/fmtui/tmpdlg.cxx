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
#include <tmpdlg.hxx>
#include <column.hxx>
#include <drpcps.hxx>
#include <frmpage.hxx>
#include <wrap.hxx>
#include <frmmgr.hxx>
#include <ccoll.hxx>
#include <swuiccoll.hxx>
#include <docstyle.hxx>
#include <fmtcol.hxx>
#include <macassgn.hxx>
#include <poolfmt.hxx>
#include <uitool.hxx>
#include <shellres.hxx>
#include <swabstdlg.hxx>

#include <cmdid.h>
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

// the dialog's carrier
SwTemplateDlg::SwTemplateDlg(vcl::Window* pParent,
                             SfxStyleSheetBase& rBase,
                             SfxStyleFamily nRegion,
                             const OString& sPage,
                             SwWrtShell* pActShell,
                             bool bNew)
    : SfxStyleDialog(pParent,
                    "TemplateDialog" + OUString::number(static_cast<sal_uInt16>(nRegion)),
                    "modules/swriter/ui/templatedialog" +
                        OUString::number(static_cast<sal_uInt16>(nRegion)) + ".ui",
                    rBase)
    , nType(nRegion)
    , pWrtShell(pActShell)
    , bNewStyle(bNew)
    , m_nIndentsId(0)
    , m_nAlignId(0)
    , m_nFontId(0)
    , m_nFontEffectId(0)
    , m_nPositionId(0)
    , m_nAsianLayoutId(0)
    , m_nOutlineId(0)
    , m_nBackgroundId(0)
    , m_nAreaId(0)
    , m_nTransparenceId(0)
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
    , m_nBulletId(0)
    , m_nNumId(0)
    , m_nNumOptId(0)
    , m_nNumPosId(0)
{
    nHtmlMode = ::GetHtmlMode(pWrtShell->GetView().GetDocShell());
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    // tinker TabPages together
    switch( nRegion )
    {
        // character styles
        case SfxStyleFamily::Char:
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

            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) , "GetTabPageRangesFunc fail!");
            m_nBackgroundId = AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ));

            SAL_WARN_IF(!pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "sw.ui", "GetTabPageCreatorFunc fail!");
            SAL_WARN_IF(!pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "sw.ui", "GetTabPageRangesFunc fail!");
            m_nBorderId = AddTabPage("borders", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ));

            SvtCJKOptions aCJKOptions;
            if(nHtmlMode & HTMLMODE_ON || !aCJKOptions.IsDoubleLinesEnabled())
                RemoveTabPage("asianlayout");
        }
        break;
        // paragraph styles
        case SfxStyleFamily::Para:
        {
            OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageRangesFunc fail!");
            m_nIndentsId = AddTabPage("indents", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH) );

            OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageRangesFunc fail!");
            m_nAlignId = AddTabPage("alignment", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH),      pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH) );

            OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageRangesFunc fail!");
            AddTabPage("textflow", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH) );

            OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageRangesFunc fail!");
            AddTabPage("asiantypo",  pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),       pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN) );

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

            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) , "GetTabPageRangesFunc fail!");
            m_nBackgroundId = AddTabPage("highlighting", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ));

            OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR), "GetTabPageRangesFunc fail!");
            AddTabPage("tabs", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR) );

            m_nOutlineId = AddTabPage("outline", SwParagraphNumTabPage::Create, SwParagraphNumTabPage::GetRanges);
            AddTabPage("dropcaps", SwDropCapsPage::Create, SwDropCapsPage::GetRanges );

            // add Area and Transparence TabPages
            m_nAreaId = AddTabPage("area", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_AREA ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_AREA ));
            m_nTransparenceId = AddTabPage("transparence", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TRANSPARENCE ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_TRANSPARENCE ) );

            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
            m_nBorderId = AddTabPage("borders", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );

            m_nConditionId = AddTabPage("condition", SwCondCollPage::Create,
                                        SwCondCollPage::GetRanges );
            if( (!bNewStyle && RES_CONDTXTFMTCOLL != static_cast<SwDocStyleSheet&>(rBase).GetCollection()->Which())
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
        case SfxStyleFamily::Frame:
        {
            m_nTypeId = AddTabPage("type", SwFramePage::Create,
                                        SwFramePage::GetRanges);
            m_nOptionsId = AddTabPage("options", SwFrameAddPage::Create,
                                        SwFrameAddPage::GetRanges);
            m_nWrapId = AddTabPage("wrap", SwWrapTabPage::Create,
                                        SwWrapTabPage::GetRanges);

            // add Area and Transparence TabPages
            m_nAreaId = AddTabPage("area", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_AREA ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_AREA ));
            m_nTransparenceId = AddTabPage("transparence", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TRANSPARENCE ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_TRANSPARENCE ) );

            m_nBorderId = AddTabPage("borders", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );

            m_nColumnId = AddTabPage("columns", SwColumnPage::Create,
                                        SwColumnPage::GetRanges );

            m_nMacroId = AddTabPage("macros", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_MACROASSIGN), nullptr);

        break;
        }
        // page styles
        case SfxStyleFamily::Page:
        {
            // remove?
            //OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
            //OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");
            //m_nBackgroundId = AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) );

            // add Area and Transparence TabPages
            m_nAreaId = AddTabPage("area", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_AREA ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_AREA ));
            m_nTransparenceId = AddTabPage("transparence", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TRANSPARENCE ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_TRANSPARENCE ) );

            m_nHeaderId = AddTabPage("header",  SvxHeaderPage::Create, SvxHeaderPage::GetRanges);
            m_nFooterId = AddTabPage("footer", SvxFooterPage::Create, SvxFooterPage::GetRanges);

            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_PAGE ), "GetTabPageRangesFunc fail!");
            m_nPageId = AddTabPage("page",
                                            pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ),
                                            pFact->GetTabPageRangesFunc( RID_SVXPAGE_PAGE ) );
            if (0 == ::GetHtmlMode(pWrtShell->GetView().GetDocShell()))
            {
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
                m_nBorderId = AddTabPage("borders",
                                pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );
                m_nColumnId = AddTabPage("columns", SwColumnPage::Create, SwColumnPage::GetRanges );
                AddTabPage("footnotes", SwFootNotePage::Create, SwFootNotePage::GetRanges );
                AddTabPage("textgrid", SwTextGridPage::Create, SwTextGridPage::GetRanges );
                SvtCJKOptions aCJKOptions;
                if(!aCJKOptions.IsAsianTypographyEnabled())
                    RemoveTabPage("textgrid");
            }
            else
            {
                RemoveTabPage("borders");
                RemoveTabPage("columns");
                RemoveTabPage("footnotes");
                RemoveTabPage("textgrid");
            }
        }
        break;
        // numbering styles
        case SfxStyleFamily::Pseudo:
        {
            AddTabPage("numbering", RID_SVXPAGE_PICK_SINGLE_NUM);
            m_nBulletId = AddTabPage("bullets", RID_SVXPAGE_PICK_BULLET);
            m_nNumId = AddTabPage("outline", RID_SVXPAGE_PICK_NUM);
            AddTabPage("graphics", RID_SVXPAGE_PICK_BMP);
            m_nNumOptId = AddTabPage("customize", RID_SVXPAGE_NUM_OPTIONS );
            m_nNumPosId = AddTabPage("position", RID_SVXPAGE_NUM_POSITION );
        }
        break;

        default:
            OSL_ENSURE(false, "wrong family");
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
        if( SfxItemState::SET == m_pExampleSet->GetItemState(
            SID_ATTR_NUMBERING_RULE, false, &pExItem ) &&
            ( !GetOutputItemSet() ||
            SfxItemState::SET != GetOutputItemSet()->GetItemState(
            SID_ATTR_NUMBERING_RULE, false, &pOutItem ) ||
            *pExItem != *pOutItem ))
        {
            if( GetOutputItemSet() )
                const_cast<SfxItemSet*>(GetOutputItemSet())->Put( *pExItem );
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

void SwTemplateDlg::RefreshInputSet()
{
    SfxItemSet* pInSet = GetInputSetImpl();
    pInSet->ClearItem();
    pInSet->SetParent( &GetStyleSheet().GetItemSet() );
}

void SwTemplateDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    // set style's and metric's names
    OUString sNumCharFormat, sBulletCharFormat;
    SwStyleNameMapper::FillUIName( RES_POOLCHR_NUM_LEVEL, sNumCharFormat);
    SwStyleNameMapper::FillUIName( RES_POOLCHR_BUL_LEVEL, sBulletCharFormat);
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));

    if (nId == m_nFontId)
    {
        OSL_ENSURE(::GetActiveView(), "no active view");

        SvxFontListItem aFontListItem( *static_cast<const SvxFontListItem*>(::GetActiveView()->
            GetDocShell()->GetItem( SID_ATTR_CHAR_FONTLIST ) ) );

        aSet.Put (SvxFontListItem( aFontListItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
        sal_uInt32 nFlags = 0;
        if(rPage.GetItemSet().GetParent() && 0 == (nHtmlMode & HTMLMODE_ON ))
            nFlags = SVX_RELATIVE_MODE;
        if( SfxStyleFamily::Char == nType )
            nFlags = nFlags|SVX_PREVIEW_CHARACTER;
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, nFlags));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nFontEffectId)
    {
        sal_uInt32 nFlags = SVX_ENABLE_FLASH;
        if( SfxStyleFamily::Char == nType )
            nFlags = nFlags|SVX_PREVIEW_CHARACTER;
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, nFlags));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nPositionId)
    {
        if( SfxStyleFamily::Char == nType )
        {
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_PREVIEW_CHARACTER));
            rPage.PageCreated(aSet);
        }
    }
    else if (nId == m_nAsianLayoutId)
    {
        if( SfxStyleFamily::Char == nType )
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
        //  handle if the current paragraph style is assigned to a list level of outline style,
        SwTextFormatColl* pTmpColl = pWrtShell->FindTextFormatCollByName( GetStyleSheet().GetName() );
        if( pTmpColl && pTmpColl->IsAssignedToListLevelOfOutlineStyle() )
        {
            static_cast<SwParagraphNumTabPage&>(rPage).DisableOutline() ;
            static_cast<SwParagraphNumTabPage&>(rPage).DisableNumbering();
        }//<-end
        weld::ComboBoxText& rBox = static_cast<SwParagraphNumTabPage&>(rPage).GetStyleBox();
        SfxStyleSheetBasePool* pPool = pWrtShell->GetView().GetDocShell()->GetStyleSheetPool();
        pPool->SetSearchMask(SfxStyleFamily::Pseudo);
        const SfxStyleSheetBase* pBase = pPool->First();
        std::set<OUString> aNames;
        while(pBase)
        {
            aNames.insert(pBase->GetName());
            pBase = pPool->Next();
        }
        for(std::set<OUString>::const_iterator it = aNames.begin(); it != aNames.end(); ++it)
            rBox.append_text(*it);
    }
    else if (nId == m_nAlignId)
    {
        aSet.Put(SfxBoolItem(SID_SVXPARAALIGNTABPAGE_ENABLEJUSTIFYEXT,true));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nTypeId)
    {
        static_cast<SwFramePage&>(rPage).SetNewFrame( true );
        static_cast<SwFramePage&>(rPage).SetFormatUsed( true );
    }
    else if (nId == m_nOptionsId)
    {
        static_cast<SwFrameAddPage&>(rPage).SetFormatUsed(true);
        static_cast<SwFrameAddPage&>(rPage).SetNewFrame(true);
    }
    else if (nId == m_nWrapId)
    {
        static_cast<SwWrapTabPage&>(rPage).SetFormatUsed( true, false );
    }
    else if (nId == m_nColumnId)
    {
        if( nType == SfxStyleFamily::Frame )
            static_cast<SwColumnPage&>(rPage).SetFrameMode(true);
        static_cast<SwColumnPage&>(rPage).SetFormatUsed( true );
    }
    // do not remove; many other style dialog combinations still use the SfxTabPage
    // for the SvxBrushItem (see RID_SVXPAGE_BACKGROUND)
    else if (nId == m_nBackgroundId)
    {
        SvxBackgroundTabFlags nFlagType = SvxBackgroundTabFlags::NONE;
        if( SfxStyleFamily::Char == nType || SfxStyleFamily::Para == nType )
            nFlagType |= SvxBackgroundTabFlags::SHOW_HIGHLIGHTING;
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, static_cast<sal_uInt32>(nFlagType)));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nConditionId)
    {
        static_cast<SwCondCollPage&>(rPage).SetCollection(
            static_cast<SwDocStyleSheet&>(GetStyleSheet()).GetCollection(), bNewStyle );
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
                pStyleSheetPool->SetSearchMask(SfxStyleFamily::Para);
                SfxStyleSheetBase *pFirstStyle = pStyleSheetPool->First();
                while(pFirstStyle)
                {
                    aList.push_back( pFirstStyle->GetName() );
                    pFirstStyle = pStyleSheetPool->Next();
                }
            }
            // set DrawingLayer FillStyles active
            aSet.Put(SfxBoolItem(SID_DRAWINGLAYER_FILLSTYLES, true));
            aSet.Put(SfxStringListItem(SID_COLLECT_LIST, &aList));
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
        aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFormat));
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFormat));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nNumOptId)
    {

        aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFormat));
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFormat));
        // collect character styles
        ScopedVclPtrInstance< ListBox > rCharFormatLB(this);
        rCharFormatLB->Clear();
        rCharFormatLB->InsertEntry( SwViewShell::GetShellRes()->aStrNone );
        SwDocShell* pDocShell = ::GetActiveWrtShell()->GetView().GetDocShell();
        ::FillCharStyleListBox(*rCharFormatLB.get(),  pDocShell);

        std::vector<OUString> aList;
        for(sal_Int32 j = 0; j < rCharFormatLB->GetEntryCount(); j++)
             aList.push_back( rCharFormatLB->GetEntry(j) );

        aSet.Put( SfxStringListItem( SID_CHAR_FMT_LIST_BOX,&aList ) ) ;
        FieldUnit eMetric = ::GetDfltMetric(dynamic_cast< const SwWebDocShell *>( pDocShell ) !=  nullptr);
        aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM, static_cast< sal_uInt16 >(eMetric)));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nNumPosId)
    {
        SwDocShell* pDocShell = ::GetActiveWrtShell()->GetView().GetDocShell();
        FieldUnit eMetric = ::GetDfltMetric(dynamic_cast<SwWebDocShell*>( pDocShell) !=  nullptr );

        aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM, static_cast< sal_uInt16 >(eMetric)));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nBulletId)
    {
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFormat));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nHeaderId)
    {
        if(0 == (nHtmlMode & HTMLMODE_ON ))
        {
            static_cast<SvxHeaderPage&>(rPage).EnableDynamicSpacing();
        }

        // set DrawingLayer FillStyles active
        aSet.Put(SfxBoolItem(SID_DRAWINGLAYER_FILLSTYLES, true));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nFooterId)
    {
        if(0 == (nHtmlMode & HTMLMODE_ON ))
        {
            static_cast<SvxFooterPage&>(rPage).EnableDynamicSpacing();
        }

        // set DrawingLayer FillStyles active
        aSet.Put(SfxBoolItem(SID_DRAWINGLAYER_FILLSTYLES, true));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nBorderId)
    {
        if( SfxStyleFamily::Para == nType )
        {
            aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,static_cast<sal_uInt16>(SwBorderModes::PARA)));
        }
        else if( SfxStyleFamily::Frame == nType )
        {
            aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,static_cast<sal_uInt16>(SwBorderModes::FRAME)));
        }
        rPage.PageCreated(aSet);
    }
    // inits for Area and Transparency TabPages
    // The selection attribute lists (XPropertyList derivates, e.g. XColorList for
    // the color table) need to be added as items (e.g. SvxColorTableItem) to make
    // these pages find the needed attributes for fill style suggestions.
    // These are added in SwDocStyleSheet::GetItemSet() for the SfxStyleFamily::Para on
    // demand, but could also be directly added from the DrawModel.
    else if (nId == m_nAreaId)
    {
        aSet.Put(GetStyleSheet().GetItemSet());

        // add flag for direct graphic content selection
        aSet.Put(SfxBoolItem(SID_OFFER_IMPORT, true));

        rPage.PageCreated(aSet);
    }
    else if (nId == m_nTransparenceId)
    {
        rPage.PageCreated(GetStyleSheet().GetItemSet());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
