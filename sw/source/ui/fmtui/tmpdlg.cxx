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

#include <vcl/tabs.hrc>

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
            AddTabPage(u"font"_ustr, TabResId(RID_TAB_FONT.aLabel), RID_SVXPAGE_CHAR_NAME,
                       RID_M + RID_TAB_FONT.sIconName);
            AddTabPage(u"fonteffect"_ustr, TabResId(RID_TAB_FONTEFFECTS.aLabel),
                       RID_SVXPAGE_CHAR_EFFECTS, RID_M + RID_TAB_FONTEFFECTS.sIconName);
            AddTabPage(u"position"_ustr, TabResId(RID_TAB_POSITION.aLabel),
                       RID_SVXPAGE_CHAR_POSITION, RID_M + RID_TAB_POSITION.sIconName);
            AddTabPage(u"asianlayout"_ustr, TabResId(RID_TAB_ASIANLAYOUT.aLabel),
                       RID_SVXPAGE_CHAR_TWOLINES, RID_M + RID_TAB_ASIANLAYOUT.sIconName);
            AddTabPage(u"background"_ustr, TabResId(RID_TAB_HIGHLIGHTING.aLabel), RID_SVXPAGE_BKG,
                       RID_M + RID_TAB_HIGHLIGHTING.sIconName);
            AddTabPage(u"borders"_ustr, TabResId(RID_TAB_BORDER.aLabel), RID_SVXPAGE_BORDER,
                       RID_M + RID_TAB_BORDER.sIconName);

            if(m_nHtmlMode & HTMLMODE_ON || !SvtCJKOptions::IsDoubleLinesEnabled())
                RemoveTabPage(u"asianlayout"_ustr);
        }
        break;
        // paragraph styles
        case SfxStyleFamily::Para:
        {
            AddTabPage(u"font"_ustr, TabResId(RID_TAB_FONT.aLabel), RID_SVXPAGE_CHAR_NAME,
                       RID_M + RID_TAB_FONT.sIconName);
            AddTabPage(u"fonteffect"_ustr, TabResId(RID_TAB_FONTEFFECTS.aLabel),
                       RID_SVXPAGE_CHAR_EFFECTS, RID_M + RID_TAB_FONTEFFECTS.sIconName);
            AddTabPage(u"textflow"_ustr, TabResId(RID_TAB_TEXTFLOW.aLabel),
                       RID_SVXPAGE_EXT_PARAGRAPH, RID_M + RID_TAB_TEXTFLOW.sIconName);
            AddTabPage(u"alignment"_ustr, TabResId(RID_TAB_ALIGNMENT.aLabel),
                       RID_SVXPAGE_ALIGN_PARAGRAPH, RID_M + RID_TAB_ALIGNMENT.sIconName);
            AddTabPage(u"indents"_ustr, TabResId(RID_TAB_INDENTS.aLabel), RID_SVXPAGE_STD_PARAGRAPH,
                       RID_M + RID_TAB_INDENTS.sIconName);
            AddTabPage(u"position"_ustr, TabResId(RID_TAB_POSITION.aLabel),
                       RID_SVXPAGE_CHAR_POSITION, RID_M + RID_TAB_POSITION.sIconName);
            AddTabPage(u"dropcaps"_ustr, TabResId(RID_TAB_DROPCAPS.aLabel), SwDropCapsPage::Create,
                       SwDropCapsPage::GetRanges, RID_M + RID_TAB_DROPCAPS.sIconName);
            AddTabPage(u"highlighting"_ustr, TabResId(RID_TAB_HIGHLIGHTING.aLabel), RID_SVXPAGE_BKG,
                       RID_M + RID_TAB_HIGHLIGHTING.sIconName);
            AddTabPage(u"area"_ustr, TabResId(RID_TAB_AREA.aLabel), RID_SVXPAGE_AREA,
                       RID_M + RID_TAB_AREA.sIconName);
            AddTabPage(u"transparence"_ustr, TabResId(RID_TAB_TRANSPARENCE.aLabel),
                       RID_SVXPAGE_TRANSPARENCE, RID_M + RID_TAB_TRANSPARENCE.sIconName);
            AddTabPage(u"borders"_ustr, TabResId(RID_TAB_BORDER.aLabel), RID_SVXPAGE_BORDER,
                       RID_M + RID_TAB_BORDER.sIconName);
            AddTabPage(u"tabs"_ustr, TabResId(RID_TAB_TABS.aLabel), RID_SVXPAGE_TABULATOR,
                       RID_M + RID_TAB_TABS.sIconName);
            AddTabPage(u"outline"_ustr, TabResId(RID_TAB_OUTLINELIST.aLabel),
                       SwParagraphNumTabPage::Create, SwParagraphNumTabPage::GetRanges,
                       RID_M + RID_TAB_OUTLINELIST.sIconName);
            AddTabPage(u"condition"_ustr, TabResId(RID_TAB_CONDITION.aLabel),
                       SwCondCollPage::Create, SwCondCollPage::GetRanges,
                       RID_M + RID_TAB_CONDITION.sIconName);
            AddTabPage(u"asiantypo"_ustr, TabResId(RID_TAB_ASIANTYPO.aLabel),
                       RID_SVXPAGE_PARA_ASIAN, RID_M + RID_TAB_ASIANTYPO.sIconName);
            AddTabPage(u"asianlayout"_ustr, TabResId(RID_TAB_ASIANLAYOUT.aLabel),
                       RID_SVXPAGE_CHAR_TWOLINES, RID_M + RID_TAB_ASIANLAYOUT.sIconName);

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
            AddTabPage(u"page"_ustr, TabResId(RID_TAB_PAGE.aLabel), RID_SVXPAGE_PAGE,
                       RID_M + RID_TAB_PAGE.sIconName);
            AddTabPage(u"area"_ustr, TabResId(RID_TAB_AREA.aLabel), RID_SVXPAGE_AREA,
                       RID_M + RID_TAB_AREA.sIconName);
            AddTabPage(u"transparence"_ustr, TabResId(RID_TAB_TRANSPARENCE.aLabel),
                       RID_SVXPAGE_TRANSPARENCE, RID_M + RID_TAB_TRANSPARENCE.sIconName);
            AddTabPage(u"header"_ustr, TabResId(RID_TAB_HEADER.aLabel), SvxHeaderPage::Create,
                       SvxHeaderPage::GetRanges, RID_M + RID_TAB_HEADER.sIconName);
            AddTabPage(u"footer"_ustr, TabResId(RID_TAB_FOOTER.aLabel), SvxFooterPage::Create,
                       SvxFooterPage::GetRanges, RID_M + RID_TAB_FOOTER.sIconName);
            if (0 == ::GetHtmlMode(m_pWrtShell->GetView().GetDocShell()))
            {
                AddTabPage(u"borders"_ustr, TabResId(RID_TAB_BORDER.aLabel), RID_SVXPAGE_BORDER,
                           RID_M + RID_TAB_BORDER.sIconName);
                AddTabPage(u"columns"_ustr, TabResId(RID_TAB_COLUMNS.aLabel), SwColumnPage::Create,
                           SwColumnPage::GetRanges, RID_M + RID_TAB_COLUMNS.sIconName);
                AddTabPage(u"footnotes"_ustr, TabResId(RID_TAB_FOOTNOTES.aLabel),
                           SwFootNotePage::Create, SwFootNotePage::GetRanges,
                           RID_M + RID_TAB_FOOTNOTES.sIconName);
                if (SvtCJKOptions::IsAsianTypographyEnabled())
                    AddTabPage(u"textgrid"_ustr, TabResId(RID_TAB_TEXTGRID.aLabel),
                               SwTextGridPage::Create, SwTextGridPage::GetRanges,
                               RID_M + RID_TAB_TEXTGRID.sIconName);
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
            AddTabPage(u"bullets"_ustr, TabResId(RID_TAB_UNOORDERED.aLabel),
                       RID_SVXPAGE_PICK_BULLET, RID_M + RID_TAB_UNOORDERED.sIconName);
            AddTabPage(u"numbering"_ustr, TabResId(RID_TAB_ORDERED.aLabel),
                       RID_SVXPAGE_PICK_SINGLE_NUM, RID_M + RID_TAB_ORDERED.sIconName);
            AddTabPage(u"outline"_ustr, TabResId(RID_TAB_OUTLINE.aLabel), RID_SVXPAGE_PICK_NUM,
                       RID_M + RID_TAB_OUTLINE.sIconName);
            AddTabPage(u"graphics"_ustr, TabResId(RID_TAB_IMAGE.aLabel), RID_SVXPAGE_PICK_BMP,
                       RID_M + RID_TAB_IMAGE.sIconName);
            AddTabPage(u"position"_ustr, TabResId(RID_TAB_LIST_POSITION.aLabel),
                       RID_SVXPAGE_NUM_POSITION, RID_M + RID_TAB_LIST_POSITION.sIconName);
            AddTabPage(u"customize"_ustr, TabResId(RID_TAB_CUSTOMIZE.aLabel),
                       RID_SVXPAGE_NUM_OPTIONS, RID_M + RID_TAB_CUSTOMIZE.sIconName);
        }
        break;
        case SfxStyleFamily::Frame:
        {
            AddTabPage(u"type"_ustr, TabResId(RID_TAB_FRAME_TYPE.aLabel), SwFramePage::Create,
                       SwFramePage::GetRanges, RID_M + RID_TAB_FRAME_TYPE.sIconName);
            AddTabPage(u"options"_ustr, TabResId(RID_TAB_FRAME_OPTIONS.aLabel),
                       SwFrameAddPage::Create, SwFrameAddPage::GetRanges,
                       RID_M + RID_TAB_FRAME_OPTIONS.sIconName);
            AddTabPage(u"wrap"_ustr, TabResId(RID_TAB_FRAME_WRAP.aLabel), SwWrapTabPage::Create,
                       SwWrapTabPage::GetRanges, RID_M + RID_TAB_FRAME_WRAP.sIconName);
            AddTabPage(u"area"_ustr, TabResId(RID_TAB_AREA.aLabel), RID_SVXPAGE_AREA,
                       RID_M + RID_TAB_AREA.sIconName);
            AddTabPage(u"transparence"_ustr, TabResId(RID_TAB_TRANSPARENCE.aLabel),
                       RID_SVXPAGE_TRANSPARENCE, RID_M + RID_TAB_TRANSPARENCE.sIconName);
            AddTabPage(u"borders"_ustr, TabResId(RID_TAB_BORDER.aLabel), RID_SVXPAGE_BORDER,
                       RID_M + RID_TAB_BORDER.sIconName);
            AddTabPage(u"columns"_ustr, TabResId(RID_TAB_COLUMNS.aLabel), SwColumnPage::Create,
                       SwColumnPage::GetRanges, RID_M + RID_TAB_COLUMNS.sIconName);
            AddTabPage(u"macros"_ustr, TabResId(RID_TAB_MACRO.aLabel), RID_SVXPAGE_MACROASSIGN,
                       RID_M + RID_TAB_MACRO.sIconName);
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
    UIName sNumCharFormat, sBulletCharFormat;
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
            UIName aNew;
            SwStyleNameMapper::FillUIName( RES_POOLCOLL_TEXT, aNew );
            aList.push_back( aNew.toString() );
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
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFormat.toString()));
        rPage.PageCreated(aSet);
    }
    else if (rId == "outline")
    {
        if (SfxStyleFamily::Pseudo == m_nType)
        {
            aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFormat.toString()));
            aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFormat.toString()));
            rPage.PageCreated(aSet);
        }
        else if (SfxStyleFamily::Para == m_nType)
        {
            //  handle if the current paragraph style is assigned to a list level of outline style,
            SwTextFormatColl* pTmpColl = m_pWrtShell->FindTextFormatCollByName( UIName(GetStyleSheet().GetName()) );
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
        aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFormat.toString()));
        aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFormat.toString()));

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
