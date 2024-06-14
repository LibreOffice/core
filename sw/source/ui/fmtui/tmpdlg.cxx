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
#include <osl/diagnose.h>
#include <sfx2/htmlmode.hxx>
#include <sfx2/sfxdlg.hxx>
#include <svl/cjkoptions.hxx>
#include <vcl/svapp.hxx>
#include <numpara.hxx>
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
#include <swuiccoll.hxx>
#include <docstyle.hxx>
#include <fmtcol.hxx>
#include <macassgn.hxx>
#include <poolfmt.hxx>
#include <uitool.hxx>
#include <shellres.hxx>
#include <strings.hrc>

#include <cmdid.h>
#include <SwStyleNameMapper.hxx>
#include <svl/stritem.hxx>
#include <svl/slstitm.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>
#include <officecfg/Office/Common.hxx>

// the dialog's carrier
SwTemplateDlgController::SwTemplateDlgController(weld::Window* pParent,
                                                 SfxStyleSheetBase& rBase,
                                                 SfxStyleFamily nRegion,
                                                 const OUString& sPage,
                                                 SwWrtShell* pActShell,
                                                 bool bNew)
    : SfxStyleDialogController(pParent,
                               "modules/swriter/ui/templatedialog" +
                                   OUString::number(static_cast<sal_uInt16>(nRegion)) + ".ui",
                               "TemplateDialog" + OUString::number(static_cast<sal_uInt16>(nRegion)),
                               rBase)
    , m_nType(nRegion)
    , m_pWrtShell(pActShell)
    , m_bNewStyle(bNew)
{
    m_nHtmlMode = ::GetHtmlMode(m_pWrtShell->GetView().GetDocShell());
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();

    GetStandardButton()->set_label(SwResId(STR_STANDARD_LABEL));
    GetStandardButton()->set_tooltip_text(SwResId(STR_STANDARD_TOOLTIP));
    GetStandardButton()->set_accessible_description(SwResId(STR_STANDARD_EXTENDEDTIP));

    GetApplyButton()->set_label(SwResId(STR_APPLY_LABEL));
    GetApplyButton()->set_tooltip_text(SwResId(STR_APPLY_TOOLTIP));
    GetApplyButton()->set_accessible_description(SwResId(STR_APPLY_EXTENDEDTIP));

    GetResetButton()->set_label(SwResId(STR_RESET_LABEL));
    GetResetButton()->set_tooltip_text(SwResId(STR_RESET_TOOLTIP));
    GetResetButton()->set_accessible_description(SwResId(STR_RESET_EXTENDEDTIP));

    // stitch TabPages together
    switch( nRegion )
    {
        // character styles
        case SfxStyleFamily::Char:
        {
            AddTabPage(u"font"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ));
            AddTabPage(u"fonteffect"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ));
            AddTabPage(u"position"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ));
            AddTabPage(u"asianlayout"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ));
            AddTabPage(u"background"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BKG ));
            AddTabPage(u"borders"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ));
            if(m_nHtmlMode & HTMLMODE_ON || !SvtCJKOptions::IsDoubleLinesEnabled())
                RemoveTabPage(u"asianlayout"_ustr);
        }
        break;
        // paragraph styles
        case SfxStyleFamily::Para:
        {
            AddTabPage(u"indents"_ustr, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH), pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH));

            AddTabPage(u"alignment"_ustr, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH));

            AddTabPage(u"textflow"_ustr, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH) );

            AddTabPage(u"asiantypo"_ustr,  pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),       pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN) );

            AddTabPage(u"font"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ) );

            AddTabPage(u"fonteffect"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ) );

            AddTabPage(u"position"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ) );

            AddTabPage(u"asianlayout"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ) );

            AddTabPage(u"highlighting"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BKG ));

            AddTabPage(u"tabs"_ustr, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR) );

            AddTabPage(u"outline"_ustr, SwParagraphNumTabPage::Create, SwParagraphNumTabPage::GetRanges);
            AddTabPage(u"dropcaps"_ustr, SwDropCapsPage::Create, SwDropCapsPage::GetRanges );

            // add Area and Transparence TabPages
            AddTabPage(u"area"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_AREA ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_AREA ));
            AddTabPage(u"transparence"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TRANSPARENCE ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_TRANSPARENCE ) );

            AddTabPage(u"borders"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );

            AddTabPage(u"condition"_ustr, SwCondCollPage::Create,
                                        SwCondCollPage::GetRanges );
            if( (!m_bNewStyle && RES_CONDTXTFMTCOLL != static_cast<SwDocStyleSheet&>(rBase).GetCollection()->Which())
            || m_nHtmlMode & HTMLMODE_ON )
                RemoveTabPage(u"condition"_ustr);

            if(m_nHtmlMode & HTMLMODE_ON)
            {
                if (!officecfg::Office::Common::Filter::HTML::Export::PrintLayout::get())
                    RemoveTabPage(u"textflow"_ustr);
                RemoveTabPage(u"asiantypo"_ustr);
                RemoveTabPage(u"tabs"_ustr);
                RemoveTabPage(u"outline"_ustr);
                RemoveTabPage(u"asianlayout"_ustr);
                if(!(m_nHtmlMode & HTMLMODE_FULL_STYLES))
                {
                    RemoveTabPage(u"background"_ustr);
                    RemoveTabPage(u"dropcaps"_ustr);
                }
            }
            else
            {
                if(!SvtCJKOptions::IsAsianTypographyEnabled())
                    RemoveTabPage(u"asiantypo"_ustr);
                if(!SvtCJKOptions::IsDoubleLinesEnabled())
                    RemoveTabPage(u"asianlayout"_ustr);
            }
        }
        break;
        // page styles
        case SfxStyleFamily::Page:
        {
            // add Area and Transparence TabPages
            AddTabPage(u"area"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_AREA ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_AREA ));
            AddTabPage(u"transparence"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TRANSPARENCE ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_TRANSPARENCE ) );
            AddTabPage(u"header"_ustr,  SvxHeaderPage::Create, SvxHeaderPage::GetRanges);
            AddTabPage(u"footer"_ustr, SvxFooterPage::Create, SvxFooterPage::GetRanges);
            AddTabPage(u"page"_ustr, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PAGE), pFact->GetTabPageRangesFunc(RID_SVXPAGE_PAGE));
            if (0 == ::GetHtmlMode(m_pWrtShell->GetView().GetDocShell()))
            {
                AddTabPage(u"borders"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );
                AddTabPage(u"columns"_ustr, SwColumnPage::Create, SwColumnPage::GetRanges );
                AddTabPage(u"footnotes"_ustr, SwFootNotePage::Create, SwFootNotePage::GetRanges );
                AddTabPage(u"textgrid"_ustr, SwTextGridPage::Create, SwTextGridPage::GetRanges );
                if(!SvtCJKOptions::IsAsianTypographyEnabled())
                    RemoveTabPage(u"textgrid"_ustr);
            }
            else
            {
                RemoveTabPage(u"borders"_ustr);
                RemoveTabPage(u"columns"_ustr);
                RemoveTabPage(u"footnotes"_ustr);
                RemoveTabPage(u"textgrid"_ustr);
            }
        }
        break;
        // numbering styles
        case SfxStyleFamily::Pseudo:
        {
            AddTabPage(u"numbering"_ustr, RID_SVXPAGE_PICK_SINGLE_NUM);
            AddTabPage(u"bullets"_ustr, RID_SVXPAGE_PICK_BULLET);
            AddTabPage(u"outline"_ustr, RID_SVXPAGE_PICK_NUM);
            AddTabPage(u"graphics"_ustr, RID_SVXPAGE_PICK_BMP);
            AddTabPage(u"customize"_ustr, RID_SVXPAGE_NUM_OPTIONS );
            AddTabPage(u"position"_ustr, RID_SVXPAGE_NUM_POSITION );
        }
        break;
        case SfxStyleFamily::Frame:
        {
            AddTabPage(u"type"_ustr, SwFramePage::Create, SwFramePage::GetRanges);
            AddTabPage(u"options"_ustr, SwFrameAddPage::Create, SwFrameAddPage::GetRanges);
            AddTabPage(u"wrap"_ustr, SwWrapTabPage::Create, SwWrapTabPage::GetRanges);

            // add Area and Transparence TabPages
            AddTabPage(u"area"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_AREA ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_AREA ));
            AddTabPage(u"transparence"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TRANSPARENCE ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_TRANSPARENCE ) );

            AddTabPage(u"borders"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );

            AddTabPage(u"columns"_ustr, SwColumnPage::Create, SwColumnPage::GetRanges );

            AddTabPage(u"macros"_ustr, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_MACROASSIGN), nullptr);
        }
        break;
        default:
            OSL_ENSURE(false, "wrong family");
    }

    if (bNew)
        SetCurPageId(u"organizer"_ustr);
    else if (!sPage.isEmpty())
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

void SwTemplateDlgController::PageCreated(const OUString& rId, SfxTabPage &rPage )
{
    // set style's and metric's names
    OUString sNumCharFormat, sBulletCharFormat;
    SwStyleNameMapper::FillUIName( RES_POOLCHR_NUM_LEVEL, sNumCharFormat);
    SwStyleNameMapper::FillUIName( RES_POOLCHR_BULLET_LEVEL, sBulletCharFormat);
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));

    if (rId == "font")
    {
        if (SwView* pView = GetActiveView())
        {
            SvxFontListItem aFontListItem( *static_cast<const SvxFontListItem*>(pView->
                GetDocShell()->GetItem( SID_ATTR_CHAR_FONTLIST ) ) );

            aSet.Put (SvxFontListItem( aFontListItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
            sal_uInt32 nFlags = 0;
            if(rPage.GetItemSet().GetParent() && 0 == (m_nHtmlMode & HTMLMODE_ON ))
                nFlags = SVX_RELATIVE_MODE;
            if( SfxStyleFamily::Char == m_nType )
                nFlags = nFlags|SVX_PREVIEW_CHARACTER;
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, nFlags));
            rPage.PageCreated(aSet);
        }
    }
    else if (rId == "fonteffect")
    {
        sal_uInt32 nFlags = SVX_ENABLE_CHAR_TRANSPARENCY;
        if( SfxStyleFamily::Char == m_nType )
            nFlags = nFlags|SVX_PREVIEW_CHARACTER;
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, nFlags));
        rPage.PageCreated(aSet);
    }
    else if (rId == "position")
    {
        if( SfxStyleFamily::Char == m_nType )
        {
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_PREVIEW_CHARACTER));
            rPage.PageCreated(aSet);
        }
        else if (SfxStyleFamily::Pseudo == m_nType)
        {
            if (SwWrtShell* pSh = ::GetActiveWrtShell())
            {
                SwDocShell* pDocShell = pSh->GetView().GetDocShell();
                FieldUnit eMetric = ::GetDfltMetric(dynamic_cast<SwWebDocShell*>( pDocShell) !=  nullptr );

                aSet.Put ( SfxUInt16Item(SID_METRIC_ITEM, static_cast< sal_uInt16 >(eMetric)));
                rPage.PageCreated(aSet);
            }
        }
    }
    else if (rId == "columns")
    {
        if( m_nType == SfxStyleFamily::Frame )
            static_cast<SwColumnPage&>(rPage).SetFrameMode(true);
        static_cast<SwColumnPage&>(rPage).SetFormatUsed( true );
    }
    // do not remove; many other style dialog combinations still use the SfxTabPage
    // for the SvxBrushItem (see RID_SVXPAGE_BKG)
    else if (rId == "background" || rId == "highlighting")
    {
        SvxBackgroundTabFlags nFlagType = SvxBackgroundTabFlags::NONE;
        if( SfxStyleFamily::Char == m_nType || SfxStyleFamily::Para == m_nType )
            nFlagType |= SvxBackgroundTabFlags::SHOW_HIGHLIGHTING;
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, static_cast<sal_uInt32>(nFlagType)));
        rPage.PageCreated(aSet);
    }
    else if (rId == "condition")
    {
        static_cast<SwCondCollPage&>(rPage).SetCollection(
            static_cast<SwDocStyleSheet&>(GetStyleSheet()).GetCollection() );
    }
    else if (rId == "page")
    {
        if(0 == (m_nHtmlMode & HTMLMODE_ON ))
        {
            std::vector<OUString> aList;
            OUString aNew;
            SwStyleNameMapper::FillUIName( RES_POOLCOLL_TEXT, aNew );
            aList.push_back( aNew );
            if( m_pWrtShell )
            {
                SfxStyleSheetBasePool* pStyleSheetPool = m_pWrtShell->
                            GetView().GetDocShell()->GetStyleSheetPool();
                SfxStyleSheetBase *pFirstStyle = pStyleSheetPool->First(SfxStyleFamily::Para);
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
        if(0 == (m_nHtmlMode & HTMLMODE_ON ))
        {
            static_cast<SvxHeaderPage&>(rPage).EnableDynamicSpacing();
        }

        // set DrawingLayer FillStyles active
        aSet.Put(SfxBoolItem(SID_DRAWINGLAYER_FILLSTYLES, true));
        rPage.PageCreated(aSet);
    }
    else if (rId == "footer")
    {
        if(0 == (m_nHtmlMode & HTMLMODE_ON ))
        {
            static_cast<SvxFooterPage&>(rPage).EnableDynamicSpacing();
        }

        // set DrawingLayer FillStyles active
        aSet.Put(SfxBoolItem(SID_DRAWINGLAYER_FILLSTYLES, true));
        rPage.PageCreated(aSet);
    }
    else if (rId == "border")
    {
        if( SfxStyleFamily::Para == m_nType )
        {
            aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,static_cast<sal_uInt16>(SwBorderModes::PARA)));
        }
        else if( SfxStyleFamily::Frame == m_nType )
        {
            aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,static_cast<sal_uInt16>(SwBorderModes::FRAME)));
        }
        rPage.PageCreated(aSet);
    }
    else if (rId == "borders")
    {
        if( SfxStyleFamily::Para == m_nType )
        {
            aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,static_cast<sal_uInt16>(SwBorderModes::PARA)));
        }
        else if( SfxStyleFamily::Frame == m_nType )
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
        if (SfxStyleFamily::Pseudo == m_nType)
        {
            aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFormat));
            aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFormat));
            rPage.PageCreated(aSet);
        }
        else if (SfxStyleFamily::Para == m_nType)
        {
            //  handle if the current paragraph style is assigned to a list level of outline style,
            SwTextFormatColl* pTmpColl = m_pWrtShell->FindTextFormatCollByName( GetStyleSheet().GetName() );
            if( pTmpColl && pTmpColl->IsAssignedToListLevelOfOutlineStyle() )
            {
                static_cast<SwParagraphNumTabPage&>(rPage).DisableOutline() ;
                static_cast<SwParagraphNumTabPage&>(rPage).DisableNumbering();
            }//<-end
            weld::ComboBox& rBox = static_cast<SwParagraphNumTabPage&>(rPage).GetStyleBox();
            SfxStyleSheetBasePool* pPool = m_pWrtShell->GetView().GetDocShell()->GetStyleSheetPool();
            const SfxStyleSheetBase* pBase = pPool->First(SfxStyleFamily::Pseudo);
            std::set<OUString> aNames;
            while(pBase)
            {
                aNames.insert(pBase->GetName());
                pBase = pPool->Next();
            }
            aNames.erase(SwResId(STR_POOLNUMRULE_NOLIST));
            for(std::set<OUString>::const_iterator it = aNames.begin(); it != aNames.end(); ++it)
                rBox.append_text(*it);
        }
    }
    else if (rId == "customize")
    {
        aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFormat));
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFormat));

        // collect character styles
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(nullptr, u"modules/swriter/ui/comboboxfragment.ui"_ustr));
        std::unique_ptr<weld::ComboBox> xCharFormatLB(xBuilder->weld_combo_box(u"combobox"_ustr));
        xCharFormatLB->clear();
        xCharFormatLB->append_text(SwViewShell::GetShellRes()->aStrNone);
        if (SwWrtShell* pSh = ::GetActiveWrtShell())
        {
            SwDocShell* pDocShell = pSh->GetView().GetDocShell();
            ::FillCharStyleListBox(*xCharFormatLB,  pDocShell);

            std::vector<OUString> aList;
            aList.reserve(xCharFormatLB->get_count());
            for (sal_Int32 j = 0; j < xCharFormatLB->get_count(); j++)
                aList.push_back(xCharFormatLB->get_text(j));

            aSet.Put( SfxStringListItem( SID_CHAR_FMT_LIST_BOX,&aList ) ) ;
            FieldUnit eMetric = ::GetDfltMetric(dynamic_cast< const SwWebDocShell *>( pDocShell ) !=  nullptr);
            aSet.Put ( SfxUInt16Item(SID_METRIC_ITEM, static_cast< sal_uInt16 >(eMetric)));
            rPage.PageCreated(aSet);
        }
    }
    else if (rId == "indents")
    {
        if( rPage.GetItemSet().GetParent() )
        {
            constexpr tools::Long constTwips_0_5mm = o3tl::toTwips(5, o3tl::Length::mm10);
            aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_ABSLINEDIST, constTwips_0_5mm));
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
        if( SfxStyleFamily::Char == m_nType )
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
        if ( m_pWrtShell )
            rPage.SetFrame( m_pWrtShell->GetView().GetViewFrame().GetFrame().GetFrameInterface() );
        rPage.PageCreated(aNewSet);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
