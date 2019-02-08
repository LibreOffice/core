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

#include <sal/log.hxx>
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
SwTemplateDlgController::SwTemplateDlgController(weld::Window* pParent,
                                                 SfxStyleSheetBase& rBase,
                                                 SfxStyleFamily nRegion,
                                                 const OString& sPage,
                                                 SwWrtShell* pActShell,
                                                 bool bNew)
    : SfxStyleDialogController(pParent,
                               "modules/swriter/ui/templatedialog" +
                                   OUString::number(static_cast<sal_uInt16>(nRegion)) + ".ui",
                               "TemplateDialog" + OString::number(static_cast<sal_uInt16>(nRegion)),
                               rBase)
    , nType(nRegion)
    , pWrtShell(pActShell)
    , bNewStyle(bNew)
{
    nHtmlMode = ::GetHtmlMode(pWrtShell->GetView().GetDocShell());
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    // stitch TabPages together
    switch( nRegion )
    {
        // character styles
        case SfxStyleFamily::Char:
        {
            AddTabPage("font", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ));
            AddTabPage("fonteffect", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ));
            AddTabPage("position", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ));
            AddTabPage("asianlayout", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ));
            AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ));
            AddTabPage("borders", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ));
            SvtCJKOptions aCJKOptions;
            if(nHtmlMode & HTMLMODE_ON || !aCJKOptions.IsDoubleLinesEnabled())
                RemoveTabPage("asianlayout");
        }
        break;
        // paragraph styles
        case SfxStyleFamily::Para:
        {
            AddTabPage("indents", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH), pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH));

            AddTabPage("alignment", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH));

            AddTabPage("textflow", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH) );

            AddTabPage("asiantypo",  pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),       pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN) );

            AddTabPage("font", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ) );

            AddTabPage("fonteffect", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ) );

            AddTabPage("position", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ) );

            AddTabPage("asianlayout", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ) );

            AddTabPage("highlighting", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ));

            AddTabPage("tabs", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR) );

            AddTabPage("outline", SwParagraphNumTabPage::Create, SwParagraphNumTabPage::GetRanges);
            AddTabPage("dropcaps", SwDropCapsPage::Create, SwDropCapsPage::GetRanges );

            // add Area and Transparence TabPages
            AddTabPage("area", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_AREA ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_AREA ));
            AddTabPage("transparence", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TRANSPARENCE ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_TRANSPARENCE ) );

            AddTabPage("borders", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );

            AddTabPage("condition", SwCondCollPage::Create,
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
        // page styles
        case SfxStyleFamily::Page:
        {
            // add Area and Transparence TabPages
            AddTabPage("area", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_AREA ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_AREA ));
            AddTabPage("transparence", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TRANSPARENCE ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_TRANSPARENCE ) );
            AddTabPage("header",  SvxHeaderPage::Create, SvxHeaderPage::GetRanges);
            AddTabPage("footer", SvxFooterPage::Create, SvxFooterPage::GetRanges);
            AddTabPage("page", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PAGE), pFact->GetTabPageRangesFunc(RID_SVXPAGE_PAGE));
            if (0 == ::GetHtmlMode(pWrtShell->GetView().GetDocShell()))
            {
                AddTabPage("borders", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );
                AddTabPage("columns", SwColumnPage::Create, SwColumnPage::GetRanges );
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
            AddTabPage("bullets", RID_SVXPAGE_PICK_BULLET);
            AddTabPage("outline", RID_SVXPAGE_PICK_NUM);
            AddTabPage("graphics", RID_SVXPAGE_PICK_BMP);
            AddTabPage("customize", RID_SVXPAGE_NUM_OPTIONS );
            AddTabPage("position", RID_SVXPAGE_NUM_POSITION );
        }
        break;
        case SfxStyleFamily::Frame:
        {
            AddTabPage("type", SwFramePage::Create, SwFramePage::GetRanges);
            AddTabPage("options", SwFrameAddPage::Create, SwFrameAddPage::GetRanges);
            AddTabPage("wrap", SwWrapTabPage::Create, SwWrapTabPage::GetRanges);

            // add Area and Transparence TabPages
            AddTabPage("area", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_AREA ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_AREA ));
            AddTabPage("transparence", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TRANSPARENCE ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_TRANSPARENCE ) );

            AddTabPage("borders", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );

            AddTabPage("columns", SwColumnPage::Create, SwColumnPage::GetRanges );

            AddTabPage("macros", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_MACROASSIGN), nullptr);
        }
        break;
        default:
            OSL_ENSURE(false, "wrong family");
    }

    if (!sPage.isEmpty())
        SetCurPageId(sPage);
}

short SwTemplateDlgController::Ok()
{
    short nRet = SfxTabDialogController::Ok();
    if( RET_OK == nRet )
    {
        const SfxPoolItem *pOutItem, *pExItem;
        if( SfxItemState::SET == m_xExampleSet->GetItemState(
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
    {
        //JP 09.01.98 Bug #46446#:
        // that's the Ok-Handler, so OK has to be default!
        nRet = RET_OK;
    }
    return nRet;
}

void SwTemplateDlgController::RefreshInputSet()
{
    SfxItemSet* pInSet = GetInputSetImpl();
    pInSet->ClearItem();
    pInSet->SetParent( &GetStyleSheet().GetItemSet() );
}

void SwTemplateDlgController::PageCreated(const OString& rId, SfxTabPage &rPage )
{
    // set style's and metric's names
    OUString sNumCharFormat, sBulletCharFormat;
    SwStyleNameMapper::FillUIName( RES_POOLCHR_NUM_LEVEL, sNumCharFormat);
    SwStyleNameMapper::FillUIName( RES_POOLCHR_BUL_LEVEL, sBulletCharFormat);
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));

    if (rId == "font")
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
    else if (rId == "fonteffect")
    {
        sal_uInt32 nFlags = SVX_ENABLE_FLASH;
        if( SfxStyleFamily::Char == nType )
            nFlags = nFlags|SVX_PREVIEW_CHARACTER;
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, nFlags));
        rPage.PageCreated(aSet);
    }
    else if (rId == "position")
    {
        if( SfxStyleFamily::Char == nType )
        {
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_PREVIEW_CHARACTER));
            rPage.PageCreated(aSet);
        }
        else if (SfxStyleFamily::Pseudo == nType)
        {
            SwDocShell* pDocShell = ::GetActiveWrtShell()->GetView().GetDocShell();
            FieldUnit eMetric = ::GetDfltMetric(dynamic_cast<SwWebDocShell*>( pDocShell) !=  nullptr );

            aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM, static_cast< sal_uInt16 >(eMetric)));
            rPage.PageCreated(aSet);
        }
    }
    else if (rId == "columns")
    {
        if( nType == SfxStyleFamily::Frame )
            static_cast<SwColumnPage&>(rPage).SetFrameMode(true);
        static_cast<SwColumnPage&>(rPage).SetFormatUsed( true );
    }
    // do not remove; many other style dialog combinations still use the SfxTabPage
    // for the SvxBrushItem (see RID_SVXPAGE_BACKGROUND)
    else if (rId == "background" || rId == "highlighting")
    {
        SvxBackgroundTabFlags nFlagType = SvxBackgroundTabFlags::NONE;
        if( SfxStyleFamily::Char == nType || SfxStyleFamily::Para == nType )
            nFlagType |= SvxBackgroundTabFlags::SHOW_HIGHLIGHTING;
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, static_cast<sal_uInt32>(nFlagType)));
        rPage.PageCreated(aSet);
    }
    else if (rId == "condition")
    {
        static_cast<SwCondCollPage&>(rPage).SetCollection(
            static_cast<SwDocStyleSheet&>(GetStyleSheet()).GetCollection(), bNewStyle );
    }
    else if (rId == "page")
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
    else if (rId == "header")
    {
        if(0 == (nHtmlMode & HTMLMODE_ON ))
        {
            static_cast<SvxHeaderPage&>(rPage).EnableDynamicSpacing();
        }

        // set DrawingLayer FillStyles active
        aSet.Put(SfxBoolItem(SID_DRAWINGLAYER_FILLSTYLES, true));
        rPage.PageCreated(aSet);
    }
    else if (rId == "footer")
    {
        if(0 == (nHtmlMode & HTMLMODE_ON ))
        {
            static_cast<SvxFooterPage&>(rPage).EnableDynamicSpacing();
        }

        // set DrawingLayer FillStyles active
        aSet.Put(SfxBoolItem(SID_DRAWINGLAYER_FILLSTYLES, true));
        rPage.PageCreated(aSet);
    }
    else if (rId == "border")
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
    else if (rId == "borders")
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
    else if (rId == "area")
    {
        aSet.Put(GetStyleSheet().GetItemSet());

        // add flag for direct graphic content selection
        aSet.Put(SfxBoolItem(SID_OFFER_IMPORT, true));

        rPage.PageCreated(aSet);
    }
    else if (rId == "transparence")
    {
        rPage.PageCreated(GetStyleSheet().GetItemSet());
    }
    else if (rId == "bullets")
    {
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFormat));
        rPage.PageCreated(aSet);
    }
    else if (rId == "outline")
    {
        if (SfxStyleFamily::Pseudo == nType)
        {
            aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFormat));
            aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFormat));
            rPage.PageCreated(aSet);
        }
        else if (SfxStyleFamily::Para == nType)
        {
            //  handle if the current paragraph style is assigned to a list level of outline style,
            SwTextFormatColl* pTmpColl = pWrtShell->FindTextFormatCollByName( GetStyleSheet().GetName() );
            if( pTmpColl && pTmpColl->IsAssignedToListLevelOfOutlineStyle() )
            {
                static_cast<SwParagraphNumTabPage&>(rPage).DisableOutline() ;
                static_cast<SwParagraphNumTabPage&>(rPage).DisableNumbering();
            }//<-end
            weld::ComboBox& rBox = static_cast<SwParagraphNumTabPage&>(rPage).GetStyleBox();
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
    }
    else if (rId == "customize")
    {
        aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFormat));
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFormat));

        // collect character styles
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(nullptr, "modules/swriter/ui/comboboxfragment.ui"));
        std::unique_ptr<weld::ComboBox> xCharFormatLB(xBuilder->weld_combo_box("combobox"));
        xCharFormatLB->clear();
        xCharFormatLB->append_text(SwViewShell::GetShellRes()->aStrNone);
        SwDocShell* pDocShell = ::GetActiveWrtShell()->GetView().GetDocShell();
        ::FillCharStyleListBox(*xCharFormatLB,  pDocShell);

        std::vector<OUString> aList;
        aList.reserve(xCharFormatLB->get_count());
        for (sal_Int32 j = 0; j < xCharFormatLB->get_count(); j++)
            aList.push_back(xCharFormatLB->get_text(j));

        aSet.Put( SfxStringListItem( SID_CHAR_FMT_LIST_BOX,&aList ) ) ;
        FieldUnit eMetric = ::GetDfltMetric(dynamic_cast< const SwWebDocShell *>( pDocShell ) !=  nullptr);
        aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM, static_cast< sal_uInt16 >(eMetric)));
        rPage.PageCreated(aSet);
    }
    else if (rId == "indents")
    {
        if( rPage.GetItemSet().GetParent() )
        {
            aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_ABSLINEDIST,MM50/10));
            aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_FLAGSET,0x000F));
            rPage.PageCreated(aSet);
        }
    }
    else if (rId == "alignment")
    {
        aSet.Put(SfxBoolItem(SID_SVXPARAALIGNTABPAGE_ENABLEJUSTIFYEXT,true));
        rPage.PageCreated(aSet);
    }
    else if (rId == "asianlayout")
    {
        if( SfxStyleFamily::Char == nType )
        {
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_PREVIEW_CHARACTER));
            rPage.PageCreated(aSet);
        }
    }
    else if (rId == "type")
    {
        static_cast<SwFramePage&>(rPage).SetNewFrame( true );
        static_cast<SwFramePage&>(rPage).SetFormatUsed( true );
    }
    else if (rId == "options")
    {
        static_cast<SwFrameAddPage&>(rPage).SetFormatUsed(true);
        static_cast<SwFrameAddPage&>(rPage).SetNewFrame(true);
    }
    else if (rId == "wrap")
    {
        static_cast<SwWrapTabPage&>(rPage).SetFormatUsed( true, false );
    }
    else if (rId == "macros")
    {
        SfxAllItemSet aNewSet(*aSet.GetPool());
        aNewSet.Put( SwMacroAssignDlg::AddEvents(MACASSGN_ALLFRM) );
        if ( pWrtShell )
            rPage.SetFrame( pWrtShell->GetView().GetViewFrame()->GetFrame().GetFrameInterface() );
        rPage.PageCreated(aNewSet);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
