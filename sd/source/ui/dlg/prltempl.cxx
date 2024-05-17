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

#define IS_OUTLINE(x) (x >= PresentationObjects::Outline_1 && x <= PresentationObjects::Outline_9)

/**
 * Constructor of Tab dialog: appends pages to the dialog
 */
SdPresLayoutTemplateDlg::SdPresLayoutTemplateDlg(SfxObjectShell const * pDocSh,
                                weld::Window* pParent,
                                bool bBackground,
                                SfxStyleSheetBase& rStyleBase,
                                PresentationObjects _ePO,
                                SfxStyleSheetBasePool* pSSPool)
    : SfxTabDialogController(pParent, u"modules/sdraw/ui/drawprtldialog.ui"_ustr, u"DrawPRTLDialog"_ustr)
    , mpDocShell(pDocSh)
    , ePO(_ePO)
    , aInputSet(*rStyleBase.GetItemSet().GetPool(), svl::Items<SID_PARAM_NUM_PRESET, SID_PARAM_CUR_NUM_LEVEL>)
{
    const SfxItemSet* pOrgSet(&rStyleBase.GetItemSet());

    if( IS_OUTLINE(ePO))
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
            aInputSet.MergeRange( p1, p2 );
        }

        aInputSet.Put( rStyleBase.GetItemSet() );

        // need parent-relationship
        const SfxItemSet* pParentItemSet = rStyleBase.GetItemSet().GetParent();
        if( pParentItemSet )
            aInputSet.SetParent( pParentItemSet );

        pOutSet.reset( new SfxItemSet( rStyleBase.GetItemSet() ) );
        pOutSet->ClearItem();

        // If there is no bullet item in this stylesheet, we get it
        // from 'Outline 1' style sheet.
        const SfxPoolItem *pItem = nullptr;
        if( SfxItemState::SET != aInputSet.GetItemState(EE_PARA_NUMBULLET, false, &pItem ))
        {
            OUString aStyleName(SdResId(STR_PSEUDOSHEET_OUTLINE) + " 1");
            SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( aStyleName, SfxStyleFamily::Pseudo);

            if(pFirstStyleSheet)
                if( SfxItemState::SET == pFirstStyleSheet->GetItemSet().GetItemState(EE_PARA_NUMBULLET, false, &pItem) )
                    aInputSet.Put( *pItem );
        }

        // preselect selected layer in dialog
        aInputSet.Put( SfxUInt16Item( SID_PARAM_CUR_NUM_LEVEL, 1<<GetOutlineLevel()));

        SetInputSet(&aInputSet);
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

    pColorTab = pColorListItem->GetColorList();
    pDashList = pDashListItem->GetDashList();
    pLineEndList = pLineEndListItem->GetLineEndList();
    pGradientList = pGradientListItem->GetGradientList();
    pHatchingList = pHatchListItem->GetHatchList();
    pBitmapList = pBitmapListItem->GetBitmapList();
    pPatternList = pPatternListItem->GetPatternList();

    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();

    AddTabPage( u"RID_SVXPAGE_LINE"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_LINE ), nullptr );
    AddTabPage( u"RID_SVXPAGE_AREA"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_AREA ), nullptr );
    AddTabPage( u"RID_SVXPAGE_SHADOW"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_SHADOW ), nullptr );
    AddTabPage( u"RID_SVXPAGE_TRANSPARENCE"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TRANSPARENCE ), nullptr );
    AddTabPage( u"RID_SVXPAGE_CHAR_NAME"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), nullptr );
    AddTabPage( u"RID_SVXPAGE_CHAR_EFFECTS"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), nullptr );
    AddTabPage( u"RID_SVXPAGE_STD_PARAGRAPH"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_STD_PARAGRAPH ), nullptr );
    AddTabPage( u"RID_SVXPAGE_TEXTATTR"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TEXTATTR ), nullptr );
    AddTabPage( u"RID_SVXPAGE_PICK_BULLET"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PICK_BULLET ), nullptr );
    AddTabPage( u"RID_SVXPAGE_PICK_SINGLE_NUM"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PICK_SINGLE_NUM ), nullptr );
    AddTabPage( u"RID_SVXPAGE_PICK_BMP"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PICK_BMP ), nullptr );
    AddTabPage( u"RID_SVXPAGE_NUM_OPTIONS"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUM_OPTIONS ), nullptr );
    AddTabPage( u"RID_SVXPAGE_TABULATOR"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TABULATOR ), nullptr );
    AddTabPage( u"RID_SVXPAGE_PARA_ASIAN"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PARA_ASIAN ), nullptr );
    AddTabPage( u"RID_SVXPAGE_ALIGN_PARAGRAPH"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGN_PARAGRAPH ), nullptr );
    AddTabPage( u"RID_SVXPAGE_BKG"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), nullptr);

    if (!SvtCJKOptions::IsAsianTypographyEnabled() || bBackground)
        RemoveTabPage( u"RID_SVXPAGE_PARA_ASIAN"_ustr );

    if (bBackground)
    {
        RemoveTabPage( u"RID_SVXPAGE_LINE"_ustr);

        RemoveTabPage( u"RID_SVXPAGE_SHADOW"_ustr);
        RemoveTabPage( u"RID_SVXPAGE_TRANSPARENCE"_ustr);
        RemoveTabPage( u"RID_SVXPAGE_CHAR_NAME"_ustr);
        RemoveTabPage( u"RID_SVXPAGE_CHAR_EFFECTS"_ustr);
        RemoveTabPage( u"RID_SVXPAGE_STD_PARAGRAPH"_ustr);
        RemoveTabPage( u"RID_SVXPAGE_TEXTATTR"_ustr);
        RemoveTabPage( u"RID_SVXPAGE_PICK_BULLET"_ustr);
        RemoveTabPage( u"RID_SVXPAGE_PICK_SINGLE_NUM"_ustr);
        RemoveTabPage( u"RID_SVXPAGE_PICK_BMP"_ustr);
        RemoveTabPage( u"RID_SVXPAGE_NUM_OPTIONS"_ustr);
        RemoveTabPage( u"RID_SVXPAGE_TABULATOR"_ustr);
        RemoveTabPage( u"RID_SVXPAGE_ALIGN_PARAGRAPH"_ustr);
        RemoveTabPage( u"RID_SVXPAGE_BKG"_ustr );
    }

    // set title and add corresponding pages to dialog
    OUString aTitle;

    switch( ePO )
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
            aTitle = SdResId(STR_PSEUDOSHEET_OUTLINE) + " " +
                OUString::number( static_cast<int>(ePO) - static_cast<int>(PresentationObjects::Outline_1) + 1 );
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
    SfxAllItemSet aSet(*(aInputSet.GetPool()));

    if (rId == "RID_SVXPAGE_LINE")
    {
        aSet.Put (SvxColorListItem(pColorTab,SID_COLOR_TABLE));
        aSet.Put (SvxDashListItem(pDashList,SID_DASH_LIST));
        aSet.Put (SvxLineEndListItem(pLineEndList,SID_LINEEND_LIST));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,1));
        rPage.PageCreated(aSet);
    }
    else if (rId == "RID_SVXPAGE_AREA")
    {
        aSet.Put (SvxColorListItem(pColorTab,SID_COLOR_TABLE));
        aSet.Put (SvxGradientListItem(pGradientList,SID_GRADIENT_LIST));
        aSet.Put (SvxHatchListItem(pHatchingList,SID_HATCH_LIST));
        aSet.Put (SvxBitmapListItem(pBitmapList,SID_BITMAP_LIST));
        aSet.Put (SvxPatternListItem(pPatternList,SID_PATTERN_LIST));
        aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,0));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,1));
        aSet.Put (SfxUInt16Item(SID_TABPAGE_POS,0));
        rPage.PageCreated(aSet);
    }
    else if (rId == "RID_SVXPAGE_SHADOW")
    {
        aSet.Put (SvxColorListItem(pColorTab,SID_COLOR_TABLE));
        aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,0));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,1));
        rPage.PageCreated(aSet);
    }
    else if (rId == "RID_SVXPAGE_TRANSPARENCE")
    {
        aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,0));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,1));
        rPage.PageCreated(aSet);
    }
    else if (rId == "RID_SVXPAGE_CHAR_NAME")
    {
        SvxFontListItem aItem(*static_cast<const SvxFontListItem*>(mpDocShell->GetItem( SID_ATTR_CHAR_FONTLIST) ) );
        aSet.Put (SvxFontListItem( aItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
        rPage.PageCreated(aSet);
    }
    else if (rId == "RID_SVXPAGE_CHAR_EFFECTS")
    {
        rPage.PageCreated(aSet);
    }
    else if (rId == "RID_SVXPAGE_TEXTATTR")
    {
        aSet.Put(CntUInt16Item(SID_SVXTEXTATTRPAGE_OBJKIND, static_cast<sal_uInt16>(SdrObjKind::Text)));
        rPage.PageCreated(aSet);
    }
    else if (rId == "RID_SVXPAGE_BKG")
    {
        aSet.Put(SfxUInt32Item(SID_FLAG_TYPE,static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR)));
        rPage.PageCreated(aSet);
    }
}

const SfxItemSet* SdPresLayoutTemplateDlg::GetOutputItemSet() const
{
    if (pOutSet)
    {
        pOutSet->Put(*SfxTabDialogController::GetOutputItemSet());

        const SvxNumBulletItem *pSvxNumBulletItem = pOutSet->GetItemIfSet(EE_PARA_NUMBULLET, false);
        if (pSvxNumBulletItem)
            SdBulletMapper::MapFontsInNumRule( const_cast<SvxNumRule&>(pSvxNumBulletItem->GetNumRule()), *pOutSet );
        return pOutSet.get();
    }
    else
        return SfxTabDialogController::GetOutputItemSet();
}

sal_uInt16 SdPresLayoutTemplateDlg::GetOutlineLevel() const
{
    switch( ePO )
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
