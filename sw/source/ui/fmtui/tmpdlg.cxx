/*************************************************************************
 *
 *  $RCSfile: tmpdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-27 08:58:18 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVSTDARR_STRINGSISORTDTOR
#define _SVSTDARR_STRINGSISORTDTOR
#include <svtools/svstdarr.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_CHARDLG_HXX //autogen
#include <svx/chardlg.hxx>
#endif
#ifndef _SVX_PARAGRPH_HXX //autogen
#include <svx/paragrph.hxx>
#endif
#ifndef _SVX_BACKGRND_HXX //autogen
#include <svx/backgrnd.hxx>
#endif
#ifndef _SVX_TABSTPGE_HXX //autogen
#include <svx/tabstpge.hxx>
#endif
#ifndef _SVX_HDFT_HXX //autogen
#include <svx/hdft2.hxx>
#endif
#ifndef _SVX_FLSTITEM_HXX //autogen
#include <svx/flstitem.hxx>
#endif
#ifndef _SVX_PAGE_HXX //autogen
#include <svx/page.hxx>
#endif
#ifndef _SVX_BORDER_HXX //autogen
#include <svx/border.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _SVX_NUMPAGES_HXX //autogen
#include <svx/numpages.hxx>
#endif
#ifndef _OFA_HTMLCFG_HXX //autogen
#include <offmgr/htmlcfg.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif

#ifndef _NUMPARA_HXX
#include <numpara.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>       // fuer Create-Methoden
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _SWWDOCSH_HXX //autogen
#include <wdocsh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _PGFNOTE_HXX
#include <pgfnote.hxx>      // der Dialog
#endif
#ifndef _TMPDLG_HXX
#include <tmpdlg.hxx>       // der Dialog
#endif
#ifndef _COLUMN_HXX
#include <column.hxx>       // Spalten
#endif
#ifndef _DRPCPS_HXX
#include <drpcps.hxx>       // Initialen
#endif
#ifndef _FRMPAGE_HXX
#include <frmpage.hxx>      // Rahmen
#endif
#ifndef _WRAP_HXX
#include <wrap.hxx>         // Rahmen
#endif
#ifndef _FRMMGR_HXX
#include <frmmgr.hxx>       // Rahmen
#endif
#ifndef _CCOLL_HXX
#include <ccoll.hxx>        // CondColl
#endif
#ifndef _DOCSTYLE_HXX
#include <docstyle.hxx>     //
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>       //
#endif
#ifndef _MACASSGN_HXX
#include <macassgn.hxx>     //
#endif
#ifndef _UIITEMS_HXX
#include <uiitems.hxx>      // inclusive der NumTabPages!
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _SHELLRES_HXX //autogen
#include <shellres.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _FMTUI_HRC
#include <fmtui.hrc>
#endif


extern SwWrtShell* GetActiveWrtShell();

/*--------------------------------------------------------------------
    Beschreibung:   Der Traeger des Dialoges
 --------------------------------------------------------------------*/

SwTemplateDlg::SwTemplateDlg(Window*            pParent,
                             SfxStyleSheetBase& rBase,
                             USHORT             nRegion,
                             BOOL               bColumn,
                             SwWrtShell*        pActShell,
                             BOOL               bNew ) :
    SfxStyleDialog( pParent,
                    SW_RES(DLG_TEMPLATE_BASE + nRegion),
                    rBase,
                    FALSE,
                    0 ),
    nType( nRegion ),
    pWrtShell(pActShell),
    bNewStyle(bNew)
{
    FreeResource();

    nHtmlMode = ::GetHtmlMode(pWrtShell->GetView().GetDocShell());
    // TabPages zusammenfieseln
    switch( nRegion )
    {
        // Zeichenvorlagen
        case SFX_STYLE_FAMILY_CHAR:
        {
            AddTabPage(TP_CHAR_STD,     SvxCharNamePage::Create,
                                        SvxCharNamePage::GetRanges );
            AddTabPage(TP_CHAR_EXT,     SvxCharEffectsPage::Create,
                                        SvxCharEffectsPage::GetRanges );
            AddTabPage(TP_CHAR_POS,     SvxCharPositionPage::Create,
                                        SvxCharPositionPage::GetRanges );
            AddTabPage(TP_CHAR_TWOLN,   SvxCharTwoLinesPage::Create,
                                        SvxCharTwoLinesPage::GetRanges );
            AddTabPage(TP_BACKGROUND,   SvxBackgroundTabPage::Create,
                                        SvxBackgroundTabPage::GetRanges );
            break;
        }
        // Absatzvorlagen
        case SFX_STYLE_FAMILY_PARA:
        {
            AddTabPage(TP_PARA_STD,     SvxStdParagraphTabPage::Create,
                                        SvxStdParagraphTabPage::GetRanges );
            AddTabPage(TP_PARA_ALIGN,   SvxParaAlignTabPage::Create,
                                        SvxParaAlignTabPage::GetRanges );

            AddTabPage(TP_PARA_EXT,     SvxExtParagraphTabPage::Create,
                                        SvxExtParagraphTabPage::GetRanges );

            AddTabPage(TP_CHAR_STD,     SvxCharNamePage::Create,
                                        SvxCharNamePage::GetRanges );
            AddTabPage(TP_CHAR_EXT,     SvxCharEffectsPage::Create,
                                        SvxCharEffectsPage::GetRanges );
            AddTabPage(TP_CHAR_POS,     SvxCharPositionPage::Create,
                                        SvxCharPositionPage::GetRanges );
            AddTabPage(TP_CHAR_TWOLN,   SvxCharTwoLinesPage::Create,
                                        SvxCharTwoLinesPage::GetRanges );

            AddTabPage(TP_TABULATOR,    SvxTabulatorTabPage::Create,
                                        SvxTabulatorTabPage::GetRanges );

            AddTabPage(TP_NUMPARA,   SwParagraphNumTabPage::Create,
                                    SwParagraphNumTabPage::GetRanges);
            AddTabPage(TP_DROPCAPS,     SwDropCapsPage::Create,
                                        SwDropCapsPage::GetRanges );

            AddTabPage(TP_BACKGROUND,   SvxBackgroundTabPage::Create,
                                        SvxBackgroundTabPage::GetRanges );
            AddTabPage(TP_BORDER,       SvxBorderTabPage::Create,
                                        SvxBorderTabPage::GetRanges );

            AddTabPage( TP_CONDCOLL,    SwCondCollPage::Create,
                                        SwCondCollPage::GetRanges );
            if( (!bNewStyle && RES_CONDTXTFMTCOLL != ((SwDocStyleSheet&)rBase).GetCollection()->Which())
            || nHtmlMode & HTMLMODE_ON )
                RemoveTabPage(TP_CONDCOLL);

            if(nHtmlMode & HTMLMODE_ON)
            {
                OfaHtmlOptions* pHtmlOpt = OFF_APP()->GetHtmlOptions();
                if (!pHtmlOpt->IsPrintLayoutExtension())
                    RemoveTabPage(TP_PARA_EXT);
                RemoveTabPage(TP_TABULATOR);
                RemoveTabPage(TP_NUMPARA);
                RemoveTabPage(TP_CHAR_TWOLN);
                if(!(nHtmlMode & HTMLMODE_FULL_STYLES))
                {
                    RemoveTabPage(TP_BACKGROUND);
                    RemoveTabPage(TP_DROPCAPS);
                }
                if(!(nHtmlMode & HTMLMODE_PARA_BORDER))
                    RemoveTabPage(TP_BORDER);
            }
        }
        break;
        // Rahmenvorlagen
        case SFX_STYLE_FAMILY_FRAME:
        {
            AddTabPage(TP_FRM_STD,      SwFrmPage::Create,
                                        SwFrmPage::GetRanges );
            AddTabPage(TP_FRM_ADD,      SwFrmAddPage::Create,
                                        SwFrmAddPage::GetRanges );
            AddTabPage(TP_FRM_WRAP,     SwWrapTabPage::Create,
                                        SwWrapTabPage::GetRanges );

            AddTabPage(TP_BACKGROUND,   SvxBackgroundTabPage::Create,
                                        SvxBackgroundTabPage::GetRanges );
            AddTabPage(TP_BORDER,       SvxBorderTabPage::Create,
                                        SvxBorderTabPage::GetRanges );

            AddTabPage(TP_COLUMN,       SwColumnPage::Create,
                                        SwColumnPage::GetRanges );

            AddTabPage( TP_MACRO_ASSIGN, SfxSmallMacroTabPage::Create, 0);

            // Auskommentiert wegen Bug #45776 (per default keine Breite&Groesse in Rahmenvorlagen)
/*          SwFmtFrmSize aSize( (const SwFmtFrmSize&)rBase.
                                            GetItemSet().Get(RES_FRM_SIZE));
            if( !aSize.GetWidth() )
            {
                aSize.SetWidth( DFLT_WIDTH );
                aSize.SetHeight( DFLT_HEIGHT );
                rBase.GetItemSet().Put( aSize );
            }*/
        break;
        }
        // Seitenvorlagen
        case SFX_STYLE_FAMILY_PAGE:
        {
            AddTabPage(TP_BACKGROUND,       SvxBackgroundTabPage::Create,
                                            SvxBackgroundTabPage::GetRanges );
            AddTabPage(TP_HEADER_PAGE,      String(SW_RES(STR_PAGE_HEADER)),
                                            SvxHeaderPage::Create,
                                            SvxHeaderPage::GetRanges );
            AddTabPage(TP_FOOTER_PAGE,      String(SW_RES(STR_PAGE_FOOTER)),
                                            SvxFooterPage::Create,
                                            SvxFooterPage::GetRanges );
            if(bColumn)
                SetCurPageId(TP_COLUMN);

            AddTabPage(TP_PAGE_STD,         String(SW_RES(STR_PAGE_STD)),
                                            SvxPageDescPage::Create,
                                            SvxPageDescPage::GetRanges,
                                            FALSE,
                                            1 ); // nach der Verwalten-Page
            if(!pActShell || 0 == ::GetHtmlMode(pWrtShell->GetView().GetDocShell()))
            {
                AddTabPage(TP_BORDER,           String(SW_RES(STR_PAGE_BORDER)),
                                                SvxBorderTabPage::Create,
                                                SvxBorderTabPage::GetRanges );
                AddTabPage(TP_COLUMN,           String(SW_RES(STR_PAGE_COLUMN)),
                                                SwColumnPage::Create,
                                                SwColumnPage::GetRanges );
                AddTabPage(TP_FOOTNOTE_PAGE,    String(SW_RES(STR_PAGE_FOOTNOTE)),
                                                SwFootNotePage::Create,
                                                SwFootNotePage::GetRanges );
            }


        }
        break;
        // Numerierungsvorlagen
        case SFX_STYLE_FAMILY_PSEUDO:
        {
            AddTabPage(RID_SVXPAGE_PICK_SINGLE_NUM,     &SvxSingleNumPickTabPage::Create, 0);
            AddTabPage(RID_SVXPAGE_PICK_BULLET, &SvxBulletPickTabPage::Create, 0);
            AddTabPage(RID_SVXPAGE_PICK_NUM,    &SvxNumPickTabPage::Create, 0);
            AddTabPage(RID_SVXPAGE_PICK_BMP,    &SvxBitmapPickTabPage::Create, 0);
            AddTabPage(RID_SVXPAGE_NUM_OPTIONS, &SvxNumOptionsTabPage::Create, 0);
            AddTabPage(RID_SVXPAGE_NUM_POSITION,&SvxNumPositionTabPage::Create, 0);

/*          const SfxPoolItem* pItem;
            if(SFX_ITEM_SET == rBase.GetItemSet().GetItemState(
                FN_PARAM_ACT_NUMBER, FALSE, &pItem ))
            {
//              pActNum = (ActNumberFormat*)((const SwPtrItem*)pItem)->GetValue();
            }
*/
        }
        break;

        default:
            ASSERT(!this, "Falsche Familie");

    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwTemplateDlg::~SwTemplateDlg()
{
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

short SwTemplateDlg::Ok()
{
    short nRet = SfxTabDialog::Ok();
    if( RET_OK == nRet )
    {
        const SfxPoolItem *pOutItem, *pExItem;
        if( SFX_ITEM_SET == pExampleSet->GetItemState(
            SID_ATTR_NUMBERING_RULE, FALSE, &pExItem ) &&
            ( !GetOutputItemSet() ||
            SFX_ITEM_SET != GetOutputItemSet()->GetItemState(
            SID_ATTR_NUMBERING_RULE, FALSE, &pOutItem ) ||
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
        // das ist der Ok - Handler also muss auf OK defaultet werden!
        nRet = RET_OK;
    return nRet;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

const SfxItemSet* SwTemplateDlg::GetRefreshedSet()
{
    delete GetInputSetImpl();
    return new SfxItemSet(GetStyleSheet().GetItemSet());
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwTemplateDlg::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    //Namen der Vorlagen und Metric setzen
    String sNumCharFmt; GetDocPoolNm( RES_POOLCHR_NUM_LEVEL, sNumCharFmt);
    String sBulletCharFmt; GetDocPoolNm( RES_POOLCHR_BUL_LEVEL, sBulletCharFmt);

    switch( nId )
    {
        case TP_CHAR_STD:
            {
                ASSERT(::GetActiveView(), "keine View aktiv");

                SvxFontListItem aFontListItem( *( (SvxFontListItem*)::GetActiveView()->
                    GetDocShell()->GetItem( SID_ATTR_CHAR_FONTLIST ) ) );

                ((SvxCharNamePage&)rPage).SetFontList(aFontListItem);

                if(rPage.GetItemSet().GetParent() && 0 == (nHtmlMode & HTMLMODE_ON ))
                    ((SvxCharNamePage&)rPage).EnableRelativeMode();
            }
            break;

        case TP_CHAR_EXT:
            ((SvxCharEffectsPage&)rPage).EnableFlash();
            break;

        case TP_PARA_STD:
            if( rPage.GetItemSet().GetParent() )
            {
                ((SvxStdParagraphTabPage&)rPage).EnableRelativeMode();
                ((SvxStdParagraphTabPage&)rPage).EnableRegisterMode();
                ((SvxStdParagraphTabPage&)rPage).EnableAutoFirstLine();
                ((SvxStdParagraphTabPage&)rPage).EnableAbsLineDist(MM50/2);
            }
            break;
        case TP_NUMPARA:
            {
                ListBox & rBox = ((SwParagraphNumTabPage&)rPage).GetStyleBox();
                SfxStyleSheetBasePool* pPool = pWrtShell->GetView().GetDocShell()->GetStyleSheetPool();
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
        break;
        case TP_PARA_ALIGN:
            ((SvxParaAlignTabPage&)rPage).EnableJustifyExt();
            break;

        case TP_FRM_STD:
            ((SwFrmPage&)rPage).SetNewFrame( TRUE );
            ((SwFrmPage&)rPage).SetFormatUsed( TRUE );
            break;

        case TP_FRM_ADD:
            ((SwFrmAddPage&)rPage).SetFormatUsed(TRUE);
            ((SwFrmAddPage&)rPage).SetNewFrame(TRUE);
            break;

        case TP_FRM_WRAP:
            ((SwWrapTabPage&)rPage).SetFormatUsed( TRUE, FALSE );
            break;

        case TP_COLUMN:
            if( nType == SFX_STYLE_FAMILY_FRAME )
                ((SwColumnPage&)rPage).SetFrmMode(TRUE);
            ((SwColumnPage&)rPage).SetFormatUsed( TRUE );
            break;

        case TP_BACKGROUND:
            if( SFX_STYLE_FAMILY_PARA == nType )
                ((SvxBackgroundTabPage&)rPage).ShowParaControl();
            if( SFX_STYLE_FAMILY_CHAR != nType )
                ((SvxBackgroundTabPage&)rPage).ShowSelector();
            break;

        case TP_CONDCOLL:
            ((SwCondCollPage&)rPage).SetCollection(
                ((SwDocStyleSheet&)GetStyleSheet()).GetCollection(), bNewStyle );
            break;

        case TP_PAGE_STD:
            if(0 == (nHtmlMode & HTMLMODE_ON ))
            {
                List aList;
                String* pNew = new String;
                GetDocPoolNm( RES_POOLCOLL_TEXT, *pNew );
                aList.Insert( pNew, (ULONG)0 );
                if( pWrtShell )
                {
                    SfxStyleSheetBasePool* pStyleSheetPool = pWrtShell->
                                GetView().GetDocShell()->GetStyleSheetPool();
                    pStyleSheetPool->SetSearchMask(SFX_STYLE_FAMILY_PARA);
                    SfxStyleSheetBase *pStyle = pStyleSheetPool->First();
                    while(pStyle)
                    {
                        aList.Insert( new String(pStyle->GetName()),
                                        aList.Count());
                        pStyle = pStyleSheetPool->Next();
                    }
                }
                ((SvxPageDescPage&)rPage).SetCollectionList(&aList);
                for( USHORT i = (USHORT)aList.Count(); i; --i )
                    delete (String*)aList.Remove(i);
            }
            break;

        case TP_MACRO_ASSIGN:
            SwMacroAssignDlg::AddEvents( (SfxSmallMacroTabPage&)rPage, MACASSGN_ALLFRM);
            break;

        case RID_SVXPAGE_PICK_NUM:
            ((SvxNumPickTabPage&)rPage).SetCharFmtNames(sNumCharFmt, sBulletCharFmt);
        break;
        case RID_SVXPAGE_NUM_OPTIONS:
        {
            ((SvxNumOptionsTabPage&)rPage).SetCharFmts(sNumCharFmt, sBulletCharFmt);
            ListBox& rCharFmtLB = ((SvxNumOptionsTabPage&)rPage).GetCharFmtListBox();
            // Zeichenvorlagen sammeln
            rCharFmtLB.Clear();
            rCharFmtLB.InsertEntry( ViewShell::GetShellRes()->aStrNone );
            SwDocShell* pDocShell = ::GetActiveWrtShell()->GetView().GetDocShell();
            ::FillCharStyleListBox(rCharFmtLB,  pDocShell);
            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebDocShell, pDocShell));
            ((SvxNumOptionsTabPage&)rPage).SetMetric(eMetric);
        }
        break;
        case RID_SVXPAGE_NUM_POSITION:
        {
            SwDocShell* pDocShell = ::GetActiveWrtShell()->GetView().GetDocShell();
            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebDocShell, pDocShell));
            ((SvxNumPositionTabPage&)rPage).SetMetric(eMetric);
        }
        break;
        case  RID_SVXPAGE_PICK_BULLET :
            ((SvxBulletPickTabPage&)rPage).SetCharFmtName(sBulletCharFmt);
        break;
    }
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/18 17:14:37  hr
    initial import

    Revision 1.88  2000/09/18 16:05:31  willem.vandorp
    OpenOffice header added.

    Revision 1.87  2000/07/27 21:15:53  jp
    opt: get template names direct from the doc and don't load it from the resource

    Revision 1.86  2000/02/11 14:47:00  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.85  1999/01/05 09:11:44  OS
    #60606# #52654# Numerierungsregel an Styles wieder korrekt


      Rev 1.84   05 Jan 1999 10:11:44   OS
   #60606# #52654# Numerierungsregel an Styles wieder korrekt

      Rev 1.83   17 Nov 1998 10:51:44   OS
   #58263# Numerierungs-Tabseiten aus dem Svx

      Rev 1.82   17 Jul 1998 10:18:28   OS
   SfxxSmallMacroTabPage einsetzen #52809#

      Rev 1.81   01 Jul 1998 17:41:16   OS
   keine rel. FontHeight im Html #51938#

      Rev 1.80   24 Apr 1998 11:06:24   OS
   Format-Flag fuer Spalten-Page setzen

      Rev 1.79   22 Apr 1998 13:35:04   OS
   PrintingExtensions im HTML

      Rev 1.78   16 Apr 1998 15:56:40   OS
   kein RegisterMode im HTML

      Rev 1.77   31 Mar 1998 10:20:56   OM
   #45776 Per default kein Size-Item in Rahmenvorlagen

------------------------------------------------------------------------*/


