/*************************************************************************
 *
 *  $RCSfile: pardlg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:16:22 $
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

#ifndef _SVX_TABSTPGE_HXX //autogen
#include <svx/tabstpge.hxx>
#endif
#ifndef _SVX_PARAGRPH_HXX //autogen
#include <svx/paragrph.hxx>
#endif
#ifndef _SVX_BACKGRND_HXX
#include <svx/backgrnd.hxx>
#endif
#ifndef _SVX_BORDER_HXX
#include <svx/border.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _OFA_HTMLCFG_HXX //autogen
#include <offmgr/htmlcfg.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif

#ifndef _SVSTDARR_STRINGSISORTDTOR
#define _SVSTDARR_STRINGSISORTDTOR
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
#include "pardlg.hxx"
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

    AddTabPage(TP_PARA_STD,  SvxStdParagraphTabPage::Create,SvxStdParagraphTabPage::GetRanges);
    AddTabPage(TP_PARA_ALIGN,  SvxParaAlignTabPage::Create,SvxParaAlignTabPage::GetRanges);

    OfaHtmlOptions* pHtmlOpt = OFF_APP()->GetHtmlOptions();
    if (!bDrawParaDlg && (!bHtmlMode || pHtmlOpt->IsPrintLayoutExtension()))
        AddTabPage(TP_PARA_EXT,  SvxExtParagraphTabPage::Create,SvxExtParagraphTabPage::GetRanges);
    else
        RemoveTabPage(TP_PARA_EXT);

    SvtCJKOptions aCJKOptions;
    if(!bHtmlMode && aCJKOptions.IsAsianTypographyEnabled())
        AddTabPage(TP_PARA_ASIAN,  SvxAsianTabPage::Create,SvxAsianTabPage::GetRanges);
    else
        RemoveTabPage(TP_PARA_ASIAN);

    USHORT nWhich(rCoreSet.GetPool()->GetWhich(SID_ATTR_LRSPACE));
    BOOL bLRValid = SFX_ITEM_AVAILABLE <= rCoreSet.GetItemState(nWhich);
    if(bHtmlMode || !bLRValid)
        RemoveTabPage(TP_TABULATOR);
    else
        AddTabPage(TP_TABULATOR, SvxTabulatorTabPage::Create,   SvxTabulatorTabPage::GetRanges);

    if (!bDrawParaDlg)
    {
        if(!(nDlgMode & DLG_ENVELOP))
            AddTabPage(TP_NUMPARA,   SwParagraphNumTabPage::Create,SwParagraphNumTabPage::GetRanges);
        else
            RemoveTabPage(TP_NUMPARA);
        if(!bHtmlMode || (nHtmlMode & HTMLMODE_FULL_STYLES))
        {
            AddTabPage(TP_DROPCAPS,  SwDropCapsPage::Create,        SwDropCapsPage::GetRanges);
            AddTabPage(TP_BACKGROUND,SvxBackgroundTabPage::Create,  SvxBackgroundTabPage::GetRanges);
        }
        else
        {
            RemoveTabPage(TP_DROPCAPS);
            RemoveTabPage(TP_BACKGROUND);
        }
        if(!bHtmlMode || (nHtmlMode & HTMLMODE_PARA_BORDER))
            AddTabPage(TP_BORDER,    SvxBorderTabPage::Create,      SvxBorderTabPage::GetRanges);
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

    // Bei Tabellenumrandung kann im Writer kein Schatten eingestellt werden
    if (nId == TP_BORDER)
    {
        ((SvxBorderTabPage&) rPage).SetSWMode(SW_BORDER_MODE_PARA);
    }
    else if( nId == TP_PARA_STD )
    {
        ((SvxStdParagraphTabPage&)rPage).SetPageWidth(
                                            rSh.GetAnyCurRect(RECT_PAGE_PRT).Width());
        if (!bDrawParaDlg)
        {
            ((SvxStdParagraphTabPage&)rPage).EnableRegisterMode();
            ((SvxStdParagraphTabPage&)rPage).EnableAutoFirstLine();
            ((SvxStdParagraphTabPage&)rPage).EnableAbsLineDist(MM50/2);
            ((SvxStdParagraphTabPage&)rPage).EnableNegativeMode();
        }
    }
    else if( TP_PARA_ALIGN == nId)
    {
        if (!bDrawParaDlg)
            ((SvxParaAlignTabPage&)rPage).EnableJustifyExt();
    }
    else if( TP_PARA_EXT == nId )
    {
        // Seitenumbruch nur, wenn der Cursor im Body-Bereich und nicht in
        // einer Tabelle steht
        const USHORT eType = rSh.GetFrmType(0,TRUE);
        if( !(FRMTYPE_BODY & eType) ||
            rSh.GetSelectionType() & SwWrtShell::SEL_TBL )
            ((SvxExtParagraphTabPage&)rPage).DisablePageBreak();
    }
    else if( TP_DROPCAPS == nId )
    {
        ((SwDropCapsPage&)rPage).SetFormat(FALSE);
    }
    else if( TP_BACKGROUND == nId )
    {
      if(!( nHtmlMode & HTMLMODE_ON ) ||
        nHtmlMode & HTMLMODE_SOME_STYLES)
            ((SvxBackgroundTabPage&)rPage).ShowSelector();
    }
    else if( TP_NUMPARA == nId)
    {
        ((SwParagraphNumTabPage&)rPage).EnableNewStart();
        ListBox & rBox = ((SwParagraphNumTabPage&)rPage).GetStyleBox();
        SfxStyleSheetBasePool* pPool = rView.GetDocShell()->GetStyleSheetPool();
        pPool->SetSearchMask(SFX_STYLE_FAMILY_PSEUDO, SFXSTYLEBIT_ALL);
        const SfxStyleSheetBase* pBase = pPool->First();
        SvStringsISortDtor aNames;
        while(pBase)
        {
            aNames.Insert(new String(pBase->GetName()));
            pBase = pPool->Next();
        }
        for(USHORT i = 0; i < aNames.Count(); i++)
            rBox.InsertEntry(*aNames.GetObject(i));
    }

}



