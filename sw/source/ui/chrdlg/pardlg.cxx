/*************************************************************************
 *
 *  $RCSfile: pardlg.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 14:42:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#include "hintids.hxx"

//CHINA001 #ifndef _SVX_TABSTPGE_HXX //autogen
//CHINA001 #include <svx/tabstpge.hxx>
//CHINA001 #endif
//CHINA001 #ifndef _SVX_PARAGRPH_HXX //autogen
//CHINA001 #include <svx/paragrph.hxx>
//CHINA001 #endif
//CHINA001 #ifndef _SVX_BACKGRND_HXX
//CHINA001 #include <svx/backgrnd.hxx>
//CHINA001 #endif
//CHINA001 #ifndef _SVX_BORDER_HXX
//CHINA001 #include <svx/border.hxx>
//CHINA001 #endif
#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#include <svx/htmlcfg.hxx>

#ifndef _SVSTDARR_STRINGSSORTDTOR
#define _SVSTDARR_STRINGSSORTDTOR
#include <svtools/svstdarr.hxx>
#endif
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif
#include "docsh.hxx"
#include "wrtsh.hxx"
#include "frmatr.hxx"
#include "view.hxx"
#include "globals.hrc"
#include "swuipardlg.hxx" //CHINA001
#include "pagedesc.hxx"
#include "paratr.hxx"
#include "drpcps.hxx"
#include "uitool.hxx"
#include "viewopt.hxx"

#ifndef _NUMPARA_HXX
#include <numpara.hxx>
#endif
#include "chrdlg.hrc"
#include "poolfmt.hrc"
#include <svx/svxids.hrc> //add CHINA001
#include <svtools/eitem.hxx> //add CHINA001
#include <svtools/intitem.hxx> //add CHINA001
#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001
#include <svx/flagsdef.hxx> //CHINA001
// STATIC DATA -----------------------------------------------------------


SwParaDlg::SwParaDlg(Window *pParent,
                    SwView& rVw,
                    const SfxItemSet& rCoreSet,
                    BYTE nDialogMode,
                    const String *pTitle,
                    BOOL bDraw,
                    UINT16 nDefPage):

    SfxTabDialog(pParent, bDraw ? SW_RES(DLG_DRAWPARA) : SW_RES(DLG_PARA),
                    &rCoreSet,  0 != pTitle),

    rView(rVw),
    nDlgMode(nDialogMode),
    bDrawParaDlg(bDraw)

{
    FreeResource();

    nHtmlMode = ::GetHtmlMode(rVw.GetDocShell());
    BOOL bHtmlMode = nHtmlMode & HTMLMODE_ON;
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
    //CHINA001 AddTabPage(TP_PARA_STD,  SvxStdParagraphTabPage::Create,SvxStdParagraphTabPage::GetRanges);
    DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageCreatorFunc fail!");//CHINA001
    DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageRangesFunc fail!");//CHINA001
    AddTabPage( TP_PARA_STD,    pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH) );
    //CHINA001 AddTabPage(TP_PARA_ALIGN,  SvxParaAlignTabPage::Create,SvxParaAlignTabPage::GetRanges);
    DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageCreatorFunc fail!");//CHINA001
    DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageRangesFunc fail!");//CHINA001
    AddTabPage( TP_PARA_ALIGN,  pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH),      pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH) );

    SvxHtmlOptions* pHtmlOpt = SvxHtmlOptions::Get();
    if (!bDrawParaDlg && (!bHtmlMode || pHtmlOpt->IsPrintLayoutExtension()))
    {
        //CHINA001 AddTabPage(TP_PARA_EXT,  SvxExtParagraphTabPage::Create,SvxExtParagraphTabPage::GetRanges);
        DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageCreatorFunc fail!");//CHINA001
        DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageRangesFunc fail!");//CHINA001
        AddTabPage( TP_PARA_EXT,    pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH) );

    }
    else
        RemoveTabPage(TP_PARA_EXT);

    SvtCJKOptions aCJKOptions;
    if(!bHtmlMode && aCJKOptions.IsAsianTypographyEnabled())
    {
        //CHINA001 AddTabPage(TP_PARA_ASIAN,  SvxAsianTabPage::Create,SvxAsianTabPage::GetRanges);
        DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageCreatorFunc fail!");//CHINA001
        DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageRangesFunc fail!");//CHINA001
        AddTabPage( TP_PARA_ASIAN,  pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),       pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN) );
    }
    else
        RemoveTabPage(TP_PARA_ASIAN);

    USHORT nWhich(rCoreSet.GetPool()->GetWhich(SID_ATTR_LRSPACE));
    BOOL bLRValid = SFX_ITEM_AVAILABLE <= rCoreSet.GetItemState(nWhich);
    if(bHtmlMode || !bLRValid)
        RemoveTabPage(TP_TABULATOR);
    else
    {
        //CHINA001 AddTabPage(TP_TABULATOR, SvxTabulatorTabPage::Create,    SvxTabulatorTabPage::GetRanges);
        DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR), "GetTabPageCreatorFunc fail!");//CHINA001
        DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR), "GetTabPageRangesFunc fail!");//CHINA001
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
            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");//CHINA001
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");//CHINA001
            AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) ); //CHINA001 AddTabPage(TP_BACKGROUND,SvxBackgroundTabPage::Create,    SvxBackgroundTabPage::GetRanges);
        }
        else
        {
            RemoveTabPage(TP_DROPCAPS);
            RemoveTabPage(TP_BACKGROUND);
        }
        if(!bHtmlMode || (nHtmlMode & HTMLMODE_PARA_BORDER))
        {
            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");//CHINA001
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");//CHINA001
            AddTabPage(TP_BORDER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) ); //CHINA001 AddTabPage(TP_BORDER,  SvxBorderTabPage::Create,      SvxBorderTabPage::GetRanges);
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


void __EXPORT SwParaDlg::PageCreated(USHORT nId, SfxTabPage& rPage)
{
    SwWrtShell& rSh = rView.GetWrtShell();
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));//CHINA001

    // Bei Tabellenumrandung kann im Writer kein Schatten eingestellt werden
    if (nId == TP_BORDER)
    {
        //CHINA001 ((SvxBorderTabPage&) rPage).SetSWMode(SW_BORDER_MODE_PARA);
        aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,SW_BORDER_MODE_PARA));
        rPage.PageCreated(aSet);
    }
    else if( nId == TP_PARA_STD )
    {
        aSet.Put(SfxUInt16Item(SID_SVXSTDPARAGRAPHTABPAGE_PAGEWIDTH, //add CHINA001
                            rSh.GetAnyCurRect(RECT_PAGE_PRT).Width())); //add CHINA001
        //CHINA001 ((SvxStdParagraphTabPage&)rPage).SetPageWidth(
        //CHINA001                                  rSh.GetAnyCurRect(RECT_PAGE_PRT).Width());

        if (!bDrawParaDlg)
        {   /* CHINA001 different bit represent call to different method of SvxStdParagraphTabPage
                        0x0001 --->EnableRelativeMode()
                        0x0002 --->EnableRegisterMode()
                        0x0004 --->EnableAutoFirstLine()
                        0x0008 --->EnableNegativeMode()


            */
            //CHINA001 ((SvxStdParagraphTabPage&)rPage).EnableRegisterMode();
            //CHINA001 ((SvxStdParagraphTabPage&)rPage).EnableAutoFirstLine();
            //CHINA001 ((SvxStdParagraphTabPage&)rPage).EnableAbsLineDist(MM50/2);
            //CHINA001 ((SvxStdParagraphTabPage&)rPage).EnableNegativeMode();
            aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_FLAGSET,0x000E));
            aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_ABSLINEDIST, MM50/2));

        }
        rPage.PageCreated(aSet);//add CHINA001
    }
    else if( TP_PARA_ALIGN == nId)
    {
        if (!bDrawParaDlg)
        {
            //CHINA001 ((SvxParaAlignTabPage&)rPage).EnableJustifyExt();
            aSet.Put(SfxBoolItem(SID_SVXPARAALIGNTABPAGE_ENABLEJUSTIFYEXT,TRUE));
            rPage.PageCreated(aSet);//add CHINA001
        }
    }
    else if( TP_PARA_EXT == nId )
    {
        // Seitenumbruch nur, wenn der Cursor im Body-Bereich und nicht in
        // einer Tabelle steht
        const USHORT eType = rSh.GetFrmType(0,TRUE);
        if( !(FRMTYPE_BODY & eType) ||
            rSh.GetSelectionType() & SwWrtShell::SEL_TBL )
        {
            //CHINA001 ((SvxExtParagraphTabPage&)rPage).DisablePageBreak();
            aSet.Put(SfxBoolItem(SID_DISABLE_SVXEXTPARAGRAPHTABPAGE_PAGEBREAK,TRUE));
            rPage.PageCreated(aSet);//add CHINA001
        }
    }
    else if( TP_DROPCAPS == nId )
    {
        ((SwDropCapsPage&)rPage).SetFormat(FALSE);
    }
    else if( TP_BACKGROUND == nId )
    {
      if(!( nHtmlMode & HTMLMODE_ON ) ||
        nHtmlMode & HTMLMODE_SOME_STYLES)
            //CHINA001 ((SvxBackgroundTabPage&)rPage).ShowSelector();
        {   //add CHINA001
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_SHOW_SELECTOR));
            rPage.PageCreated(aSet);
        }
    }
    else if( TP_NUMPARA == nId)
    {
        ((SwParagraphNumTabPage&)rPage).EnableNewStart();
        ListBox & rBox = ((SwParagraphNumTabPage&)rPage).GetStyleBox();
        SfxStyleSheetBasePool* pPool = rView.GetDocShell()->GetStyleSheetPool();
        pPool->SetSearchMask(SFX_STYLE_FAMILY_PSEUDO, SFXSTYLEBIT_ALL);
        const SfxStyleSheetBase* pBase = pPool->First();
        SvStringsSortDtor aNames;
        while(pBase)
        {
            aNames.Insert(new String(pBase->GetName()));
            pBase = pPool->Next();
        }
        for(USHORT i = 0; i < aNames.Count(); i++)
            rBox.InsertEntry(*aNames.GetObject(i));
    }

}



