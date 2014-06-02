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
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#include "hintids.hxx"
#include <svx/htmlmode.hxx>
#include <svl/style.hxx>

#include <svtools/htmlcfg.hxx>

#ifndef _SVSTDARR_STRINGSSORTDTOR
#define _SVSTDARR_STRINGSSORTDTOR
#include <svl/svstdarr.hxx>
#endif
#include <svl/cjkoptions.hxx>
#include "docsh.hxx"
#include "wrtsh.hxx"
#include "frmatr.hxx"
#include "view.hxx"
#include "globals.hrc"
#include "swuipardlg.hxx"
#include "pagedesc.hxx"
#include "paratr.hxx"
#include "drpcps.hxx"
#include "uitool.hxx"
#include "viewopt.hxx"
#include <numpara.hxx>
#include "chrdlg.hrc"
#include "poolfmt.hrc"
#include <svx/svxids.hrc>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>
// STATIC DATA -----------------------------------------------------------


SwParaDlg::SwParaDlg(Window *pParent,
                    SwView& rVw,
                    const SfxItemSet& rCoreSet,
                    sal_uInt8 nDialogMode,
                    const String *pTitle,
                    sal_Bool bDraw,
                    sal_uInt16 nDefPage):

    SfxTabDialog(pParent, bDraw ? SW_RES(DLG_DRAWPARA) : SW_RES(DLG_PARA),
                    &rCoreSet,  0 != pTitle),

    rView(rVw),
    nDlgMode(nDialogMode),
    bDrawParaDlg(bDraw)

{
    FreeResource();

    nHtmlMode = ::GetHtmlMode(rVw.GetDocShell());
    sal_Bool bHtmlMode = static_cast< sal_Bool >(nHtmlMode & HTMLMODE_ON);
    if(pTitle)
    {
            // Update des Titels
        String aTmp( GetText() );
        aTmp += SW_RESSTR(STR_TEXTCOLL_HEADER);
        aTmp += *pTitle;
        aTmp += ')';
        SetText(aTmp);
    }
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();

    DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageCreatorFunc fail!");
    DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageRangesFunc fail!");
    AddTabPage( TP_PARA_STD,    pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH) );

    DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageCreatorFunc fail!");
    DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageRangesFunc fail!");
    AddTabPage( TP_PARA_ALIGN,  pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH),      pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH) );

    SvxHtmlOptions* pHtmlOpt = SvxHtmlOptions::Get();
    if (!bDrawParaDlg && (!bHtmlMode || pHtmlOpt->IsPrintLayoutExtension()))
    {
        DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageCreatorFunc fail!");
        DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageRangesFunc fail!");
        AddTabPage( TP_PARA_EXT,    pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH) );

    }
    else
        RemoveTabPage(TP_PARA_EXT);

    SvtCJKOptions aCJKOptions;
    if(!bHtmlMode && aCJKOptions.IsAsianTypographyEnabled())
    {
        DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageCreatorFunc fail!");
        DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageRangesFunc fail!");
        AddTabPage( TP_PARA_ASIAN,  pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),       pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN) );
    }
    else
        RemoveTabPage(TP_PARA_ASIAN);

    sal_uInt16 nWhich(rCoreSet.GetPool()->GetWhich(SID_ATTR_LRSPACE));
    sal_Bool bLRValid = SFX_ITEM_AVAILABLE <= rCoreSet.GetItemState(nWhich);
    if(bHtmlMode || !bLRValid)
        RemoveTabPage(TP_TABULATOR);
    else
    {
        DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR), "GetTabPageCreatorFunc fail!");
        DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR), "GetTabPageRangesFunc fail!");
        AddTabPage( TP_TABULATOR,   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR) );

    }
    if (!bDrawParaDlg)
    {
        if(!(nDlgMode & DLG_ENVELOP))
            AddTabPage(TP_NUMPARA,   SwParagraphNumTabPage::Create,SwParagraphNumTabPage::GetRanges);
        else
            RemoveTabPage(TP_NUMPARA);
        if(!bHtmlMode || (nHtmlMode & HTMLMODE_FULL_STYLES))
        {
            AddTabPage(TP_DROPCAPS,  SwDropCapsPage::Create,        SwDropCapsPage::GetRanges);
        }
        else
        {
            RemoveTabPage(TP_DROPCAPS);
        }
        if(!bHtmlMode || (nHtmlMode & (HTMLMODE_SOME_STYLES|HTMLMODE_FULL_STYLES)))
        {
            //UUUU remove?
            //DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
            //DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");
            //AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) );

            //UUUU add Area and Transparence TabPages
            AddTabPage(RID_SVXPAGE_AREA);
            AddTabPage(RID_SVXPAGE_TRANSPARENCE);
        }
        else
        {
            //UUUU RemoveTabPage(TP_BACKGROUND);
            RemoveTabPage(RID_SVXPAGE_AREA);
            RemoveTabPage(RID_SVXPAGE_TRANSPARENCE);
        }
        if(!bHtmlMode || (nHtmlMode & HTMLMODE_PARA_BORDER))
        {
            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
            AddTabPage(TP_BORDER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );
        }
        else
            RemoveTabPage(TP_BORDER);
    }

    if (nDefPage)
        SetCurPageId(nDefPage);
}


__EXPORT SwParaDlg::~SwParaDlg()
{
}


void __EXPORT SwParaDlg::PageCreated(sal_uInt16 nId,SfxTabPage& rPage)
{
    SwWrtShell& rSh = rView.GetWrtShell();
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));

    switch(nId)
    {
        case TP_BORDER:
        {
            // Bei Tabellenumrandung kann im Writer kein Schatten eingestellt werden
            aSet.Put(SfxUInt16Item(SID_SWMODE_TYPE,SW_BORDER_MODE_PARA));
            rPage.PageCreated(aSet);
            break;
        }
        case TP_PARA_STD:
        {
            aSet.Put(SfxUInt16Item(SID_SVXSTDPARAGRAPHTABPAGE_PAGEWIDTH,
                static_cast<sal_uInt16>(rSh.GetAnyCurRect(RECT_PAGE_PRT).Width())));

            if(!bDrawParaDlg)
            {
                aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_FLAGSET,0x000E));
                aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_ABSLINEDIST,MM50 / 10));

            }
            rPage.PageCreated(aSet);
            break;
        }
        case TP_PARA_ALIGN:
        {
            if(!bDrawParaDlg)
            {
                aSet.Put(SfxBoolItem(SID_SVXPARAALIGNTABPAGE_ENABLEJUSTIFYEXT,sal_True));
                rPage.PageCreated(aSet);
            }
            break;
        }
        case TP_PARA_EXT:
        {
            // Seitenumbruch nur, wenn der Cursor im Body-Bereich und nicht in
            // einer Tabelle steht
            const sal_uInt16 eType = rSh.GetFrmType(0,sal_True);
            if(!(FRMTYPE_BODY & eType) ||
                rSh.GetSelectionType() & nsSelectionType::SEL_TBL)
            {
                aSet.Put(SfxBoolItem(SID_DISABLE_SVXEXTPARAGRAPHTABPAGE_PAGEBREAK,sal_True));
                rPage.PageCreated(aSet);
            }
            break;
        }
        case TP_DROPCAPS:
        {
            ((SwDropCapsPage&)rPage).SetFormat(sal_False);
            break;
        }
        case TP_BACKGROUND:
        {
            if(!(nHtmlMode & HTMLMODE_ON) ||
                nHtmlMode & HTMLMODE_SOME_STYLES)
            {
                aSet.Put(SfxUInt32Item(SID_FLAG_TYPE,SVX_SHOW_SELECTOR));
                rPage.PageCreated(aSet);
            }
            break;
        }
        case TP_NUMPARA:
        {
            //-->#outline level,added by zhaojianwei
            SwTxtFmtColl* pTmpColl = rSh.GetCurTxtFmtColl();
            if(pTmpColl && pTmpColl->IsAssignedToListLevelOfOutlineStyle())
            {
                ((SwParagraphNumTabPage&)rPage).DisableOutline();
            }//<-end

            ((SwParagraphNumTabPage&)rPage).EnableNewStart();
            ListBox & rBox = ((SwParagraphNumTabPage&)rPage).GetStyleBox();
            SfxStyleSheetBasePool* pPool = rView.GetDocShell()->GetStyleSheetPool();
            pPool->SetSearchMask(SFX_STYLE_FAMILY_PSEUDO,SFXSTYLEBIT_ALL);
            const SfxStyleSheetBase* pBase = pPool->First();
            SvStringsSortDtor aNames;
            while(pBase)
            {
                aNames.Insert(new String(pBase->GetName()));
                pBase = pPool->Next();
            }
            for(sal_uInt16 i = 0; i < aNames.Count(); i++)
                rBox.InsertEntry(*aNames.GetObject(i));
            break;
        }

        //UUUU inits for Area and Transparency TabPages
        // The selection attribute lists (XPropertyList derivates, e.g. XColorList for
        // the color table) need to be added as items (e.g. SvxColorTableItem) to make
        // these pages find the needed attributes for fill style suggestions.
        // These are added in SwDocStyleSheet::GetItemSet() for the SFX_STYLE_FAMILY_PARA on
        // demand, but could also be directly added from the DrawModel.
        case RID_SVXPAGE_AREA:
        {
            SfxItemSet aNew(*aSet.GetPool(),
                SID_COLOR_TABLE, SID_BITMAP_LIST,
                SID_OFFER_IMPORT, SID_OFFER_IMPORT, 0, 0);

            aNew.Put(*GetInputSetImpl());

            // add flag for direct graphic content selection
            aNew.Put(SfxBoolItem(SID_OFFER_IMPORT, true));

            rPage.PageCreated(aNew);
            break;
        }

        case RID_SVXPAGE_TRANSPARENCE:
        {
            rPage.PageCreated(*GetInputSetImpl());
            break;
        }
    }
}

// eof
