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

#include <sal/log.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxids.hrc>
#include <editeng/flstitem.hxx>
#include <svx/drawitem.hxx>
#include <svl/style.hxx>
#include <svx/svdobjkind.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/numitem.hxx>
#include <svl/cjkoptions.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxdlg.hxx>

#include <strings.hrc>
#include <sdresid.hxx>
#include <prltempl.hxx>
#include <bulmaper.hxx>
#include <svl/intitem.hxx>
#include <svx/flagsdef.hxx>

#include <vcl/tabs.hrc>
#include <vcl/weld/Dialog.hxx>

namespace
{
bool lcl_isOutline(PresentationObjects eType)
{
    return eType >= PresentationObjects::Outline_1 && eType <= PresentationObjects::Outline_9;
}
}

/**
 * Constructor of Tab dialog: appends pages to the dialog
 */
SdPresLayoutTemplateDlg::SdPresLayoutTemplateDlg(SfxObjectShell const* pDocSh,
                                                 weld::Window* pParent, bool bBackground,
                                                 SfxStyleSheetBase& rStyleBase,
                                                 PresentationObjects _ePO,
                                                 SfxStyleSheetBasePool* pSSPool)
    : SfxTabDialogController(pParent, u"modules/sdraw/ui/drawprtldialog.ui"_ustr,
                             u"DrawPRTLDialog"_ustr)
    , mpDocShell(pDocSh)
    , m_ePO(_ePO)
    , m_aInputSet(*rStyleBase.GetItemSet().GetPool(),
                  svl::Items<SID_PARAM_NUM_PRESET, SID_PARAM_CUR_NUM_LEVEL>)
{
    const SfxItemSet* pOrgSet(&rStyleBase.GetItemSet());

    if (lcl_isOutline(m_ePO))
    {
        // Unfortunately, the Itemsets of our style sheets are not discrete...
        const WhichRangesContainer& pPtr = pOrgSet->GetRanges();
        sal_uInt16 p1, p2;
        for( sal_Int32 i = 0; i < pPtr.size(); ++i )
        {
            p1 = pPtr[i].first;
            p2 = pPtr[i].second;

            // first, we make it discrete
            while(i < pPtr.size() - 1 && (pPtr[i+1].first - p2 == 1))
            {
                p2 = pPtr[i+1].second;
                ++i;
            }
            m_aInputSet.MergeRange(p1, p2);
        }

        m_aInputSet.Put(rStyleBase.GetItemSet());

        // need parent-relationship
        const SfxItemSet* pParentItemSet = rStyleBase.GetItemSet().GetParent();
        if( pParentItemSet )
            m_aInputSet.SetParent(pParentItemSet);

        m_pOutSet.reset(new SfxItemSet(rStyleBase.GetItemSet()));
        m_pOutSet->ClearItem();

        // If there is no bullet item in this stylesheet, we get it
        // from 'Outline 1' style sheet.
        const SfxPoolItem *pItem = nullptr;
        if (SfxItemState::SET != m_aInputSet.GetItemState(EE_PARA_NUMBULLET, false, &pItem))
        {
            OUString aStyleName(SdResId(STR_PSEUDOSHEET_OUTLINE) + " 1");
            SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( aStyleName, SfxStyleFamily::Pseudo);

            if(pFirstStyleSheet)
                if( SfxItemState::SET == pFirstStyleSheet->GetItemSet().GetItemState(EE_PARA_NUMBULLET, false, &pItem) )
                    m_aInputSet.Put(*pItem);
        }

        // preselect selected layer in dialog
        m_aInputSet.Put(SfxUInt16Item(SID_PARAM_CUR_NUM_LEVEL, 1 << GetOutlineLevel()));

        SetInputSet(&m_aInputSet);
    }
    else {
        SetInputSet(pOrgSet);
    }

    SvxColorListItem const *pColorListItem = mpDocShell->GetItem( SID_COLOR_TABLE );
    SvxGradientListItem const *pGradientListItem = mpDocShell->GetItem( SID_GRADIENT_LIST );
    SvxBitmapListItem const *pBitmapListItem = mpDocShell->GetItem( SID_BITMAP_LIST );
    SvxPatternListItem const *pPatternListItem = mpDocShell->GetItem( SID_PATTERN_LIST );
    SvxHatchListItem const *pHatchListItem = mpDocShell->GetItem( SID_HATCH_LIST );
    SvxDashListItem const *pDashListItem = mpDocShell->GetItem( SID_DASH_LIST );
    SvxLineEndListItem const *pLineEndListItem = mpDocShell->GetItem( SID_LINEEND_LIST );

    m_pColorTab = pColorListItem->GetColorList();
    m_pDashList = pDashListItem->GetDashList();
    m_pLineEndList = pLineEndListItem->GetLineEndList();
    m_pGradientList = pGradientListItem->GetGradientList();
    m_pHatchingList = pHatchListItem->GetHatchList();
    m_pBitmapList = pBitmapListItem->GetBitmapList();
    m_pPatternList = pPatternListItem->GetPatternList();

    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();

    if (!bBackground)
        AddTabPage(u"line"_ustr, TabResId(RID_TAB_SDPRESSTYLE_LINE),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_LINE), RID_M + RID_TAB_LINE_IMG);
    AddTabPage(u"area"_ustr, TabResId(RID_TAB_SDPRESSTYLE_AREA),
               pFact->GetTabPageCreatorFunc(RID_SVXPAGE_AREA), RID_M + RID_TAB_AREA_IMG);
    if (!bBackground)
    {
        AddTabPage(u"shadowing"_ustr, TabResId(RID_TAB_SDPRESSTYLE_SHADOW),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_SHADOW),
                   RID_M + RID_TAB_SHADOW_IMG);
        AddTabPage(u"transparency"_ustr, TabResId(RID_TAB_SDPRESSTYLE_TRANSPARENCE),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TRANSPARENCE),
                   RID_M + RID_TAB_TRANSPARENCE_IMG);
        AddTabPage(u"font"_ustr, TabResId(RID_TAB_SDPRESSTYLE_FONT),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_NAME),
                   RID_M + RID_TAB_FONT_IMG);
        AddTabPage(u"fonteffect"_ustr, TabResId(RID_TAB_SDPRESSTYLE_FONTEFFECTS),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_EFFECTS),
                   RID_M + RID_TAB_FONTEFFECTS_IMG);
        AddTabPage(u"indents"_ustr, TabResId(RID_TAB_SDPRESSTYLE_INDENTS),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH),
                   RID_M + RID_TAB_INDENTS_IMG);
        AddTabPage(u"text"_ustr, TabResId(RID_TAB_SDPRESSTYLE_TEXT),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TEXTATTR),
                   RID_M + RID_TAB_TEXT_IMG);
        AddTabPage(u"bullets"_ustr, TabResId(RID_TAB_SDPRESSTYLE_BULLETS),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PICK_BULLET),
                   RID_M + RID_TAB_BULLETS_IMG);
        AddTabPage(u"numbering"_ustr, TabResId(RID_TAB_SDPRESSTYLE_NUMBERING),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PICK_SINGLE_NUM),
                   RID_M + RID_TAB_NUMBERING_IMG);
        AddTabPage(u"image"_ustr, TabResId(RID_TAB_SDPRESSTYLE_IMAGE),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PICK_BMP),
                   RID_M + RID_TAB_IMAGE_IMG);
        AddTabPage(u"customize"_ustr, TabResId(RID_TAB_SDPRESSTYLE_CUSTOMIZE),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_NUM_OPTIONS),
                   RID_M + RID_TAB_CUSTOMIZE_IMG);
        AddTabPage(u"alignment"_ustr, TabResId(RID_TAB_SDPRESSTYLE_ALIGNMENT),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH),
                   RID_M + RID_TAB_ALIGNMENT_IMG);
        if (SvtCJKOptions::IsAsianTypographyEnabled())
            AddTabPage(u"asiantypo"_ustr, TabResId(RID_TAB_SDPRESSTYLE_ASIANTYPO),
                       pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),
                       RID_M + RID_TAB_ASIANTYPO_IMG);
        AddTabPage(u"tabs"_ustr, TabResId(RID_TAB_SDPRESSTYLE_TABS),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR),
                   RID_M + RID_TAB_TABS_IMG);
        AddTabPage(u"background"_ustr, TabResId(RID_TAB_SDPRESSTYLE_HIGHLIGHTING),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BKG),
                   RID_M + RID_TAB_HIGHLIGHTING_IMG);
    }

    // set title and add corresponding pages to dialog
    OUString aTitle;

    switch (m_ePO)
    {
        case PresentationObjects::Title:
            aTitle = SdResId(STR_PSEUDOSHEET_TITLE);
        break;

        case PresentationObjects::Subtitle:
            aTitle = SdResId(STR_PSEUDOSHEET_SUBTITLE);
        break;

        case PresentationObjects::Background:
            aTitle = SdResId(STR_PSEUDOSHEET_BACKGROUND);
        break;

        case PresentationObjects::BackgroundObjects:
            aTitle = SdResId(STR_PSEUDOSHEET_BACKGROUNDOBJECTS);
        break;

        case PresentationObjects::Outline_1:
        case PresentationObjects::Outline_2:
        case PresentationObjects::Outline_3:
        case PresentationObjects::Outline_4:
        case PresentationObjects::Outline_5:
        case PresentationObjects::Outline_6:
        case PresentationObjects::Outline_7:
        case PresentationObjects::Outline_8:
        case PresentationObjects::Outline_9:
            aTitle = SdResId(STR_PSEUDOSHEET_OUTLINE) + " "
                     + OUString::number(static_cast<int>(m_ePO)
                                        - static_cast<int>(PresentationObjects::Outline_1) + 1);
            break;

        case PresentationObjects::Notes:
            aTitle = SdResId(STR_PSEUDOSHEET_NOTES);
        break;
    }
    m_xDialog->set_title(aTitle);
}

SdPresLayoutTemplateDlg::~SdPresLayoutTemplateDlg()
{
}

void SdPresLayoutTemplateDlg::PageCreated(const OUString& rId, SfxTabPage &rPage)
{
    SfxAllItemSet aSet(*(m_aInputSet.GetPool()));

    if (rId == "line")
    {
        aSet.Put(SvxColorListItem(m_pColorTab, SID_COLOR_TABLE));
        aSet.Put(SvxDashListItem(m_pDashList, SID_DASH_LIST));
        aSet.Put(SvxLineEndListItem(m_pLineEndList, SID_LINEEND_LIST));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,1));
        rPage.PageCreated(aSet);
    }
    else if (rId == "area")
    {
        aSet.Put(SvxColorListItem(m_pColorTab, SID_COLOR_TABLE));
        aSet.Put(SvxGradientListItem(m_pGradientList, SID_GRADIENT_LIST));
        aSet.Put(SvxHatchListItem(m_pHatchingList, SID_HATCH_LIST));
        aSet.Put(SvxBitmapListItem(m_pBitmapList, SID_BITMAP_LIST));
        aSet.Put(SvxPatternListItem(m_pPatternList, SID_PATTERN_LIST));
        aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,0));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,1));
        aSet.Put (SfxUInt16Item(SID_TABPAGE_POS,0));
        rPage.PageCreated(aSet);
    }
    else if (rId == "shadowing")
    {
        aSet.Put(SvxColorListItem(m_pColorTab, SID_COLOR_TABLE));
        aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,0));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,1));
        rPage.PageCreated(aSet);
    }
    else if (rId == "transparency")
    {
        aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,0));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,1));
        rPage.PageCreated(aSet);
    }
    else if (rId == "font")
    {
        SvxFontListItem aItem(*mpDocShell->GetItem( SID_ATTR_CHAR_FONTLIST) );
        aSet.Put (SvxFontListItem( aItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
        rPage.PageCreated(aSet);
    }
    else if (rId == "fonteffect")
    {
        rPage.PageCreated(aSet);
    }
    else if (rId == "text")
    {
        aSet.Put(CntUInt16Item(SID_SVXTEXTATTRPAGE_OBJKIND, static_cast<sal_uInt16>(SdrObjKind::Text)));
        rPage.PageCreated(aSet);
    }
    else if (rId == "background")
    {
        aSet.Put(SfxUInt32Item(SID_FLAG_TYPE,static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR)));
        rPage.PageCreated(aSet);
    }
}

const SfxItemSet* SdPresLayoutTemplateDlg::GetOutputItemSet() const
{
    if (m_pOutSet)
    {
        m_pOutSet->Put(*SfxTabDialogController::GetOutputItemSet());

        const SvxNumBulletItem* pSvxNumBulletItem
            = m_pOutSet->GetItemIfSet(EE_PARA_NUMBULLET, false);
        if (pSvxNumBulletItem)
            SdBulletMapper::MapFontsInNumRule(
                const_cast<SvxNumRule&>(pSvxNumBulletItem->GetNumRule()), *m_pOutSet);
        return m_pOutSet.get();
    }
    else
        return SfxTabDialogController::GetOutputItemSet();
}

sal_uInt16 SdPresLayoutTemplateDlg::GetOutlineLevel() const
{
    switch (m_ePO)
    {
    case PresentationObjects::Outline_1: return 0;
    case PresentationObjects::Outline_2: return 1;
    case PresentationObjects::Outline_3: return 2;
    case PresentationObjects::Outline_4: return 3;
    case PresentationObjects::Outline_5: return 4;
    case PresentationObjects::Outline_6: return 5;
    case PresentationObjects::Outline_7: return 6;
    case PresentationObjects::Outline_8: return 7;
    case PresentationObjects::Outline_9: return 8;
    default:
        SAL_WARN( "sd", "Wrong Po! [CL]");
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
