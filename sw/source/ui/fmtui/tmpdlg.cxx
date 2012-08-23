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

#include <hintids.hxx>

#include <vcl/msgbox.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/hdft.hxx>
#include <editeng/flstitem.hxx>
#include <svx/htmlmode.hxx>
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
#include <fmtui.hrc>
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

SwTemplateDlg::SwTemplateDlg(Window*            pParent,
                             SfxStyleSheetBase& rBase,
                             sal_uInt16             nRegion,
                             sal_uInt16             nPageId,
                             SwWrtShell*        pActShell,
                             sal_Bool               bNew ) :
    SfxStyleDialog( pParent,
                    SW_RES(DLG_TEMPLATE_BASE + nRegion),
                    rBase,
                    sal_False,
                    0 ),
    nType( nRegion ),
    pWrtShell(pActShell),
    bNewStyle(bNew)
{
    FreeResource();

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
            AddTabPage(TP_CHAR_STD, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ) );

            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ) , "GetTabPageRangesFunc fail!");
            AddTabPage(TP_CHAR_EXT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ) );

            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ) , "GetTabPageRangesFunc fail!");
            AddTabPage(TP_CHAR_POS, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ) );

            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ) , "GetTabPageRangesFunc fail!");
            AddTabPage(TP_CHAR_TWOLN, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ) );

            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) , "GetTabPageRangesFunc fail!");
            AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) );

            SvtCJKOptions aCJKOptions;
            if(nHtmlMode & HTMLMODE_ON || !aCJKOptions.IsDoubleLinesEnabled())
                RemoveTabPage(TP_CHAR_TWOLN);
        }
        break;
        // paragraph styles
        case SFX_STYLE_FAMILY_PARA:
        {
                OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageRangesFunc fail!");
                AddTabPage( TP_PARA_STD,    pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH) );

                OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageRangesFunc fail!");
                AddTabPage( TP_PARA_ALIGN,  pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH),      pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH) );

                OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageRangesFunc fail!");
                AddTabPage( TP_PARA_EXT,    pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH) );

                OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageRangesFunc fail!");
                AddTabPage( TP_PARA_ASIAN,  pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),       pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN) );

                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageRangesFunc fail!");
                AddTabPage(TP_CHAR_STD, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ) );

                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageRangesFunc fail!");
                AddTabPage(TP_CHAR_EXT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ) );

                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ) , "GetTabPageRangesFunc fail!");
                AddTabPage(TP_CHAR_POS, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ) );

                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ) , "GetTabPageRangesFunc fail!");
                AddTabPage(TP_CHAR_TWOLN, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ) );


            OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR), "GetTabPageRangesFunc fail!");
            AddTabPage( TP_TABULATOR,   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR) );


            AddTabPage(TP_NUMPARA,   SwParagraphNumTabPage::Create,
                                    SwParagraphNumTabPage::GetRanges);
            AddTabPage(TP_DROPCAPS,     SwDropCapsPage::Create,
                                        SwDropCapsPage::GetRanges );
            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");
            AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) );
            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
            AddTabPage(TP_BORDER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );

            AddTabPage( TP_CONDCOLL,    SwCondCollPage::Create,
                                        SwCondCollPage::GetRanges );
            if( (!bNewStyle && RES_CONDTXTFMTCOLL != ((SwDocStyleSheet&)rBase).GetCollection()->Which())
            || nHtmlMode & HTMLMODE_ON )
                RemoveTabPage(TP_CONDCOLL);

            SvtCJKOptions aCJKOptions;
            if(nHtmlMode & HTMLMODE_ON)
            {
                SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
                if (!rHtmlOpt.IsPrintLayoutExtension())
                    RemoveTabPage(TP_PARA_EXT);
                RemoveTabPage(TP_PARA_ASIAN);
                RemoveTabPage(TP_TABULATOR);
                RemoveTabPage(TP_NUMPARA);
                RemoveTabPage(TP_CHAR_TWOLN);
                if(!(nHtmlMode & HTMLMODE_FULL_STYLES))
                {
                    RemoveTabPage(TP_BACKGROUND);
                    RemoveTabPage(TP_DROPCAPS);
                }
            }
            else
            {
                if(!aCJKOptions.IsAsianTypographyEnabled())
                    RemoveTabPage(TP_PARA_ASIAN);
                if(!aCJKOptions.IsDoubleLinesEnabled())
                    RemoveTabPage(TP_CHAR_TWOLN);
            }
        }
        break;
        // frame styles
        case SFX_STYLE_FAMILY_FRAME:
        {
            AddTabPage(TP_FRM_STD,      SwFrmPage::Create,
                                        SwFrmPage::GetRanges );
            AddTabPage(TP_FRM_ADD,      SwFrmAddPage::Create,
                                        SwFrmAddPage::GetRanges );
            AddTabPage(TP_FRM_WRAP,     SwWrapTabPage::Create,
                                        SwWrapTabPage::GetRanges );
            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");
            AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) );
            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
            AddTabPage(TP_BORDER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );

            AddTabPage(TP_COLUMN,       SwColumnPage::Create,
                                        SwColumnPage::GetRanges );

            AddTabPage( TP_MACRO_ASSIGN, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_MACROASSIGN), 0);

        break;
        }
        // page styles
        case SFX_STYLE_FAMILY_PAGE:
        {
            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");
            AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) );
            AddTabPage(TP_HEADER_PAGE,      String(SW_RES(STR_PAGE_HEADER)),
                                            SvxHeaderPage::Create,
                                            SvxHeaderPage::GetRanges );
            AddTabPage(TP_FOOTER_PAGE,      String(SW_RES(STR_PAGE_FOOTER)),
                                            SvxFooterPage::Create,
                                            SvxFooterPage::GetRanges );
            if(nPageId)
                SetCurPageId(nPageId);

            OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), "GetTabPageCreatorFunc fail!");
            OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_PAGE ), "GetTabPageRangesFunc fail!");
            AddTabPage(TP_PAGE_STD,         String(SW_RES(STR_PAGE_STD)),
                                            pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ),
                                            pFact->GetTabPageRangesFunc( RID_SVXPAGE_PAGE ),
                                            sal_False,
                                            1 ); // after the manage-page
            if(!pActShell || 0 == ::GetHtmlMode(pWrtShell->GetView().GetDocShell()))
            {
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
                AddTabPage(TP_BORDER,           String(SW_RES(STR_PAGE_BORDER)),
                                pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );
                AddTabPage(TP_COLUMN,           String(SW_RES(STR_PAGE_COLUMN)),
                                                SwColumnPage::Create,
                                                SwColumnPage::GetRanges );
                AddTabPage(TP_FOOTNOTE_PAGE,    String(SW_RES(STR_PAGE_FOOTNOTE)),
                                                SwFootNotePage::Create,
                                                SwFootNotePage::GetRanges );
                AddTabPage(TP_TEXTGRID_PAGE,    String(SW_RES(STR_PAGE_TEXTGRID)),
                                                SwTextGridPage::Create,
                                                SwTextGridPage::GetRanges );
                SvtCJKOptions aCJKOptions;
                if(!aCJKOptions.IsAsianTypographyEnabled())
                    RemoveTabPage(TP_TEXTGRID_PAGE);
            }


        }
        break;
        // numbering styles
        case SFX_STYLE_FAMILY_PSEUDO:
        {
            AddTabPage( RID_SVXPAGE_PICK_SINGLE_NUM );
            AddTabPage( RID_SVXPAGE_PICK_BULLET );
            AddTabPage( RID_SVXPAGE_PICK_NUM );
            AddTabPage( RID_SVXPAGE_PICK_BMP);
            AddTabPage( RID_SVXPAGE_NUM_OPTIONS );
            AddTabPage( RID_SVXPAGE_NUM_POSITION );

        }
        break;

        default:
            OSL_ENSURE(!this, "wrong family");

    }
    EnableApplyButton( true );
    SetApplyHandler( LINK(this, SwTemplateDlg, ApplyHdl ) );
}

SwTemplateDlg::~SwTemplateDlg()
{
}

IMPL_LINK( SwTemplateDlg, ApplyHdl, void*, pVoid )
{
    (void)pVoid; //unused
    if ( OK_Impl() )
    {
        Ok();
        EndDialog( RET_APPLY_TEMPLATE );
    }
    return 0;
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
    String sNumCharFmt, sBulletCharFmt;
    SwStyleNameMapper::FillUIName( RES_POOLCHR_NUM_LEVEL, sNumCharFmt);
    SwStyleNameMapper::FillUIName( RES_POOLCHR_BUL_LEVEL, sBulletCharFmt);
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));

    switch( nId )
    {
        case TP_CHAR_STD:
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
            break;

        case TP_CHAR_EXT:
            {
            sal_uInt32 nFlags = SVX_ENABLE_FLASH;
            if( SFX_STYLE_FAMILY_CHAR == nType )
                nFlags = nFlags|SVX_PREVIEW_CHARACTER;
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, nFlags));
            rPage.PageCreated(aSet);
            }
            break;

        case TP_CHAR_POS:
            if( SFX_STYLE_FAMILY_CHAR == nType )
            {
                aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_PREVIEW_CHARACTER));
                rPage.PageCreated(aSet);
            }
        break;

        case TP_CHAR_TWOLN:
            if( SFX_STYLE_FAMILY_CHAR == nType )
            {
                aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_PREVIEW_CHARACTER));
                rPage.PageCreated(aSet);
            }
        break;

        case TP_PARA_STD:
            {
                if( rPage.GetItemSet().GetParent() )
                {
                    aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_ABSLINEDIST,MM50/10));
                    aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_FLAGSET,0x000F));
                    rPage.PageCreated(aSet);
                }

            }
            break;
        case TP_NUMPARA:
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
        break;
        case TP_PARA_ALIGN:
            {
                aSet.Put(SfxBoolItem(SID_SVXPARAALIGNTABPAGE_ENABLEJUSTIFYEXT,sal_True));
                rPage.PageCreated(aSet);
            }
            break;

        case TP_FRM_STD:
            ((SwFrmPage&)rPage).SetNewFrame( sal_True );
            ((SwFrmPage&)rPage).SetFormatUsed( sal_True );
            break;

        case TP_FRM_ADD:
            ((SwFrmAddPage&)rPage).SetFormatUsed(sal_True);
            ((SwFrmAddPage&)rPage).SetNewFrame(sal_True);
            break;

        case TP_FRM_WRAP:
            ((SwWrapTabPage&)rPage).SetFormatUsed( sal_True, sal_False );
            break;

        case TP_COLUMN:
            if( nType == SFX_STYLE_FAMILY_FRAME )
                ((SwColumnPage&)rPage).SetFrmMode(sal_True);
            ((SwColumnPage&)rPage).SetFormatUsed( sal_True );
            break;

        case TP_BACKGROUND:
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
        break;
        case TP_CONDCOLL:
            ((SwCondCollPage&)rPage).SetCollection(
                ((SwDocStyleSheet&)GetStyleSheet()).GetCollection(), bNewStyle );
            break;

        case TP_PAGE_STD:
            if(0 == (nHtmlMode & HTMLMODE_ON ))
            {
                std::vector<String> aList;
                String aNew;
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
            break;

        case TP_MACRO_ASSIGN:
        {
            SfxAllItemSet aNewSet(*aSet.GetPool());
            aNewSet.Put( SwMacroAssignDlg::AddEvents(MACASSGN_ALLFRM) );
            if ( pWrtShell )
                rPage.SetFrame( pWrtShell->GetView().GetViewFrame()->GetFrame().GetFrameInterface() );
            rPage.PageCreated(aNewSet);
            break;
        }

        case RID_SVXPAGE_PICK_NUM:
            {
                aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFmt));
                aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFmt));
                rPage.PageCreated(aSet);
            }
        break;
        case RID_SVXPAGE_NUM_OPTIONS:
        {

            aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFmt));
            aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFmt));
            // collect character styles
            ListBox rCharFmtLB(this);
            rCharFmtLB.Clear();
            rCharFmtLB.InsertEntry( ViewShell::GetShellRes()->aStrNone );
            SwDocShell* pDocShell = ::GetActiveWrtShell()->GetView().GetDocShell();
            ::FillCharStyleListBox(rCharFmtLB,  pDocShell);

            std::vector<String> aList;
            for(sal_uInt16 j = 0; j < rCharFmtLB.GetEntryCount(); j++)
                 aList.push_back( rCharFmtLB.GetEntry(j) );

            aSet.Put( SfxStringListItem( SID_CHAR_FMT_LIST_BOX,&aList ) ) ;
            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebDocShell, pDocShell));
            aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM, static_cast< sal_uInt16 >(eMetric)));
            rPage.PageCreated(aSet);
        }
        break;
        case RID_SVXPAGE_NUM_POSITION:
        {
            SwDocShell* pDocShell = ::GetActiveWrtShell()->GetView().GetDocShell();
            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebDocShell, pDocShell));

            aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM, static_cast< sal_uInt16 >(eMetric)));
            rPage.PageCreated(aSet);
        }
        break;
        case  RID_SVXPAGE_PICK_BULLET :
            {
                aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFmt));
                rPage.PageCreated(aSet);
            }
        break;
        case  TP_HEADER_PAGE:
            if(0 == (nHtmlMode & HTMLMODE_ON ))
                ((SvxHeaderPage&)rPage).EnableDynamicSpacing();
        break;
        case  TP_FOOTER_PAGE:
            if(0 == (nHtmlMode & HTMLMODE_ON ))
                ((SvxFooterPage&)rPage).EnableDynamicSpacing();
        break;
        case TP_BORDER :
            if( SFX_STYLE_FAMILY_PARA == nType )
            {
                aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,SW_BORDER_MODE_PARA));
            }
            else if( SFX_STYLE_FAMILY_FRAME == nType )
            {
                aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,SW_BORDER_MODE_FRAME));
            }
            rPage.PageCreated(aSet);

        break;
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
