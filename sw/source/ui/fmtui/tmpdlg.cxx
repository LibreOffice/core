/*************************************************************************
 *
 *  $RCSfile: tmpdlg.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 16:04:54 $
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

#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVSTDARR_STRINGSSORTDTOR
#define _SVSTDARR_STRINGSSORTDTOR
#include <svtools/svstdarr.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
//CHINA001 #ifndef _SVX_CHARDLG_HXX //autogen
//CHINA001 #include <svx/chardlg.hxx>
//CHINA001 #endif
//CHINA001 #ifndef _SVX_PARAGRPH_HXX //autogen
//CHINA001 #include <svx/paragrph.hxx>
//CHINA001 #endif
//CHINA001 #ifndef _SVX_BACKGRND_HXX //autogen
//CHINA001 #include <svx/backgrnd.hxx>
//CHINA001 #endif
//CHINA001 #ifndef _SVX_TABSTPGE_HXX //autogen
//CHINA001 #include <svx/tabstpge.hxx>
//CHINA001 #endif
#ifndef _SVX_HDFT_HXX //autogen
#include <svx/hdft2.hxx>
#endif
#ifndef _SVX_FLSTITEM_HXX //autogen
#include <svx/flstitem.hxx>
#endif
//CHINA001 #ifndef _SVX_PAGE_HXX //autogen
//CHINA001 #include <svx/page.hxx>
//CHINA001 #endif
//CHINA001 #ifndef _SVX_BORDER_HXX //autogen
//CHINA001 #include <svx/border.hxx>
//CHINA001 #endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
//CHINA001 #ifndef _SVX_NUMPAGES_HXX //autogen
//CHINA001 #include <svx/numpages.hxx>
//CHINA001 #endif
#include <svx/htmlcfg.hxx>
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
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
#include <pgfnote.hxx>
#endif
#ifndef _PGFGRID_HXX
#include <pggrid.hxx>
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
#include <swuiccoll.hxx> //CHINA001
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
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
#include <svx/svxids.hrc> //add CHINA001
#include <svtools/stritem.hxx>//add CHINA001
#include <svtools/aeitem.hxx> //add CHINA001
#include <svtools/slstitm.hxx> //add CHINA001
#include <svtools/eitem.hxx> //add CHINA001
#include <svtools/intitem.hxx> //add CHINA001
#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001
#include <svx/flagsdef.hxx> //CHINA001
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
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
    // TabPages zusammenfieseln
    switch( nRegion )
    {
        // Zeichenvorlagen
        case SFX_STYLE_FAMILY_CHAR:
        {
            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageCreatorFunc fail!");//CHINA001
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ) , "GetTabPageRangesFunc fail!");//CHINA001
            AddTabPage(TP_CHAR_STD, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ) );  //CHINA001 AddTabPage(TP_CHAR_STD,  SvxCharNamePage::Create,
                                        //CHINA001 SvxCharNamePage::GetRanges );
            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageCreatorFunc fail!");//CHINA001
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ) , "GetTabPageRangesFunc fail!");//CHINA001
            AddTabPage(TP_CHAR_EXT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ) );  //CHINA001 AddTabPage(TP_CHAR_EXT,    SvxCharEffectsPage::Create,
                                        //CHINA001 SvxCharEffectsPage::GetRanges );
            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), "GetTabPageCreatorFunc fail!");//CHINA001
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ) , "GetTabPageRangesFunc fail!");//CHINA001
            AddTabPage(TP_CHAR_POS, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ) );  //CHINA001 AddTabPage(TP_CHAR_POS,  SvxCharPositionPage::Create,
                                        //CHINA001 SvxCharPositionPage::GetRanges );
            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), "GetTabPageCreatorFunc fail!");//CHINA001
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ) , "GetTabPageRangesFunc fail!");//CHINA001
            AddTabPage(TP_CHAR_TWOLN, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ) );  //CHINA001 AddTabPage(TP_CHAR_TWOLN,  SvxCharTwoLinesPage::Create,
                                        //CHINA001 SvxCharTwoLinesPage::GetRanges );
            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");//CHINA001
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) , "GetTabPageRangesFunc fail!");//CHINA001
            AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) ); //CHINA001 AddTabPage(TP_BACKGROUND,     SvxBackgroundTabPage::Create,
                                        //CHINA001 SvxBackgroundTabPage::GetRanges );
            SvtCJKOptions aCJKOptions;
            if(nHtmlMode & HTMLMODE_ON || !aCJKOptions.IsDoubleLinesEnabled())
                RemoveTabPage(TP_CHAR_TWOLN);
        }
        break;
        // Absatzvorlagen
        case SFX_STYLE_FAMILY_PARA:
        {
                //CHINA001  AddTabPage(TP_PARA_STD,     SvxStdParagraphTabPage::Create,
                //CHINA001                              SvxStdParagraphTabPage::GetRanges );
                DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageCreatorFunc fail!");//CHINA001
                DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageRangesFunc fail!");//CHINA001
                AddTabPage( TP_PARA_STD,    pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH) );
            //CHINA001 AddTabPage(TP_PARA_ALIGN,    SvxParaAlignTabPage::Create,
            //CHINA001                          SvxParaAlignTabPage::GetRanges );

                DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageCreatorFunc fail!");//CHINA001
                DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageRangesFunc fail!");//CHINA001
                AddTabPage( TP_PARA_ALIGN,  pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH),      pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH) );
            //CHINA001 AddTabPage(TP_PARA_EXT,      SvxExtParagraphTabPage::Create,
            //CHINA001                          SvxExtParagraphTabPage::GetRanges );

                DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageCreatorFunc fail!");//CHINA001
                DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageRangesFunc fail!");//CHINA001
                AddTabPage( TP_PARA_EXT,    pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH) );


            //CHINA001 AddTabPage(TP_PARA_ASIAN,    SvxAsianTabPage::Create,
            //CHINA001                          SvxAsianTabPage::GetRanges);

                DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageCreatorFunc fail!");//CHINA001
                DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageRangesFunc fail!");//CHINA001
                AddTabPage( TP_PARA_ASIAN,  pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),       pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN) );


                DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageCreatorFunc fail!");//CHINA001
                DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageRangesFunc fail!");//CHINA001
                AddTabPage(TP_CHAR_STD, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ) ); //CHINA001 AddTabPage(TP_CHAR_STD,   SvxCharNamePage::Create,
                                        //CHINA001 SvxCharNamePage::GetRanges );
                DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageCreatorFunc fail!");//CHINA001
                DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageRangesFunc fail!");//CHINA001
                AddTabPage(TP_CHAR_EXT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ) ); //CHINA001 AddTabPage(TP_CHAR_EXT,     SvxCharEffectsPage::Create,
                                        //CHINA001 SvxCharEffectsPage::GetRanges );
                DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), "GetTabPageCreatorFunc fail!");//CHINA001
                DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ) , "GetTabPageRangesFunc fail!");//CHINA001
                AddTabPage(TP_CHAR_POS, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ) ); //CHINA001 AddTabPage(TP_CHAR_POS,   SvxCharPositionPage::Create,
                                        //CHINA001 SvxCharPositionPage::GetRanges );
                DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), "GetTabPageCreatorFunc fail!");//CHINA001
                DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ) , "GetTabPageRangesFunc fail!");//CHINA001
                AddTabPage(TP_CHAR_TWOLN, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ) ); //CHINA001 AddTabPage(TP_CHAR_TWOLN,   SvxCharTwoLinesPage::Create,
                                        //CHINA001 SvxCharTwoLinesPage::GetRanges );


            //CHINA001 AddTabPage(TP_TABULATOR,     SvxTabulatorTabPage::Create,
            //CHINA001                          SvxTabulatorTabPage::GetRanges );
            DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR), "GetTabPageCreatorFunc fail!");//CHINA001
            DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR), "GetTabPageRangesFunc fail!");//CHINA001
            AddTabPage( TP_TABULATOR,   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR),        pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR) );


            AddTabPage(TP_NUMPARA,   SwParagraphNumTabPage::Create,
                                    SwParagraphNumTabPage::GetRanges);
            AddTabPage(TP_DROPCAPS,     SwDropCapsPage::Create,
                                        SwDropCapsPage::GetRanges );
            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");//CHINA001
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");//CHINA001
            AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) ); //CHINA001 AddTabPage(TP_BACKGROUND,     SvxBackgroundTabPage::Create,
                                        //CHINA001 SvxBackgroundTabPage::GetRanges );
            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");//CHINA001
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");//CHINA001
            AddTabPage(TP_BORDER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );//CHINA001 AddTabPage(TP_BORDER,      SvxBorderTabPage::Create,
                                        //CHINA001 SvxBorderTabPage::GetRanges );

            AddTabPage( TP_CONDCOLL,    SwCondCollPage::Create,
                                        SwCondCollPage::GetRanges );
            if( (!bNewStyle && RES_CONDTXTFMTCOLL != ((SwDocStyleSheet&)rBase).GetCollection()->Which())
            || nHtmlMode & HTMLMODE_ON )
                RemoveTabPage(TP_CONDCOLL);

            SvtCJKOptions aCJKOptions;
            if(nHtmlMode & HTMLMODE_ON)
            {
                SvxHtmlOptions* pHtmlOpt = SvxHtmlOptions::Get();
                if (!pHtmlOpt->IsPrintLayoutExtension())
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
                if(!(nHtmlMode & HTMLMODE_PARA_BORDER))
                    RemoveTabPage(TP_BORDER);
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
        // Rahmenvorlagen
        case SFX_STYLE_FAMILY_FRAME:
        {
            AddTabPage(TP_FRM_STD,      SwFrmPage::Create,
                                        SwFrmPage::GetRanges );
            AddTabPage(TP_FRM_ADD,      SwFrmAddPage::Create,
                                        SwFrmAddPage::GetRanges );
            AddTabPage(TP_FRM_WRAP,     SwWrapTabPage::Create,
                                        SwWrapTabPage::GetRanges );
            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");//CHINA001
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");//CHINA001
            AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) ); //CHINA001 AddTabPage(TP_BACKGROUND, SvxBackgroundTabPage::Create,
                                        //CHINA001 SvxBackgroundTabPage::GetRanges );
            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");//CHINA001
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");//CHINA001
            AddTabPage(TP_BORDER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) ); //CHINA001 AddTabPage(TP_BORDER,         SvxBorderTabPage::Create,
                                        //CHINA001 SvxBorderTabPage::GetRanges );

            AddTabPage(TP_COLUMN,       SwColumnPage::Create,
                                        SwColumnPage::GetRanges );

            AddTabPage( TP_MACRO_ASSIGN, SfxMacroTabPage::Create, 0);

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
            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");//CHINA001
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");//CHINA001
            AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) ); //CHINA001 AddTabPage(TP_BACKGROUND,         SvxBackgroundTabPage::Create,
                                            //CHINA001 SvxBackgroundTabPage::GetRanges );
            AddTabPage(TP_HEADER_PAGE,      String(SW_RES(STR_PAGE_HEADER)),
                                            SvxHeaderPage::Create,
                                            SvxHeaderPage::GetRanges );
            AddTabPage(TP_FOOTER_PAGE,      String(SW_RES(STR_PAGE_FOOTER)),
                                            SvxFooterPage::Create,
                                            SvxFooterPage::GetRanges );
            if(bColumn)
                SetCurPageId(TP_COLUMN);

            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), "GetTabPageCreatorFunc fail!");//CHINA001
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_PAGE ), "GetTabPageRangesFunc fail!");//CHINA001
            AddTabPage(TP_PAGE_STD,         String(SW_RES(STR_PAGE_STD)),
                                            pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), //CHINA001 SvxPageDescPage::Create,
                                            pFact->GetTabPageRangesFunc( RID_SVXPAGE_PAGE ), //CHINA001 SvxPageDescPage::GetRanges,
                                            FALSE,
                                            1 ); // nach der Verwalten-Page
            if(!pActShell || 0 == ::GetHtmlMode(pWrtShell->GetView().GetDocShell()))
            {
                DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");//CHINA001
                DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");//CHINA001
                AddTabPage(TP_BORDER,           String(SW_RES(STR_PAGE_BORDER)),
                                pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );
                                                //CHINA001 SvxBorderTabPage::Create,
                                                //CHINA001 SvxBorderTabPage::GetRanges );
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
        // Numerierungsvorlagen
        case SFX_STYLE_FAMILY_PSEUDO:
        {
            AddTabPage( RID_SVXPAGE_PICK_SINGLE_NUM );//CHINA001 AddTabPage(RID_SVXPAGE_PICK_SINGLE_NUM,    &SvxSingleNumPickTabPage::Create, 0);
            AddTabPage( RID_SVXPAGE_PICK_BULLET );//CHINA001 AddTabPage(RID_SVXPAGE_PICK_BULLET, &SvxBulletPickTabPage::Create, 0);
            AddTabPage( RID_SVXPAGE_PICK_NUM );//CHINA001 AddTabPage(RID_SVXPAGE_PICK_NUM,  &SvxNumPickTabPage::Create, 0);
            AddTabPage( RID_SVXPAGE_PICK_BMP);//CHINA001 AddTabPage(RID_SVXPAGE_PICK_BMP,   &SvxBitmapPickTabPage::Create, 0);
            AddTabPage( RID_SVXPAGE_NUM_OPTIONS );//CHINA001 AddTabPage(RID_SVXPAGE_NUM_OPTIONS, &SvxNumOptionsTabPage::Create, 0);
            AddTabPage( RID_SVXPAGE_NUM_POSITION );//CHINA001 AddTabPage(RID_SVXPAGE_NUM_POSITION,&SvxNumPositionTabPage::Create, 0);


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
    SfxItemSet* pInSet = GetInputSetImpl();
    pInSet->ClearItem();
    pInSet->SetParent( &GetStyleSheet().GetItemSet() );
    return pInSet;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwTemplateDlg::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    //Namen der Vorlagen und Metric setzen
    String sNumCharFmt, sBulletCharFmt;
    SwStyleNameMapper::FillUIName( RES_POOLCHR_NUM_LEVEL, sNumCharFmt);
    SwStyleNameMapper::FillUIName( RES_POOLCHR_BUL_LEVEL, sBulletCharFmt);
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool())); //CHINA001

    switch( nId )
    {
        case TP_CHAR_STD:
            {
                ASSERT(::GetActiveView(), "keine View aktiv");

                SvxFontListItem aFontListItem( *( (SvxFontListItem*)::GetActiveView()->
                    GetDocShell()->GetItem( SID_ATTR_CHAR_FONTLIST ) ) );

                //CHINA001 ((SvxCharNamePage&)rPage).SetFontList(aFontListItem);
                aSet.Put (SvxFontListItem( aFontListItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
                UINT32 nFlags = 0;
                if(rPage.GetItemSet().GetParent() && 0 == (nHtmlMode & HTMLMODE_ON ))
                    nFlags = SVX_RELATIVE_MODE;
                    //CHINA001 ((SvxCharNamePage&)rPage).EnableRelativeMode();
                if( SFX_STYLE_FAMILY_CHAR == nType )
                    nFlags = nFlags|SVX_PREVIEW_CHARACTER;
                    //CHINA001 ((SvxCharNamePage&)rPage).SetPreviewBackgroundToCharacter();
                aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, nFlags));
                rPage.PageCreated(aSet);
            }
            break;

        case TP_CHAR_EXT:
            {
            //CHINA001 ((SvxCharEffectsPage&)rPage).EnableFlash();
            //CHINA001 if( SFX_STYLE_FAMILY_CHAR == nType )
                //CHINA001 ((SvxCharEffectsPage&)rPage).SetPreviewBackgroundToCharacter();
            UINT32 nFlags = SVX_ENABLE_FLASH;
            if( SFX_STYLE_FAMILY_CHAR == nType )
                nFlags = nFlags|SVX_PREVIEW_CHARACTER;
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, nFlags));
            rPage.PageCreated(aSet);
            }
            break;

        case TP_CHAR_POS:
            if( SFX_STYLE_FAMILY_CHAR == nType )
                //CHINA001 ((SvxCharPositionPage&)rPage).SetPreviewBackgroundToCharacter();
            {
                aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_PREVIEW_CHARACTER));
                rPage.PageCreated(aSet);
            }
        break;

        case TP_CHAR_TWOLN:
            if( SFX_STYLE_FAMILY_CHAR == nType )
                //CHINA001 ((SvxCharTwoLinesPage&)rPage).SetPreviewBackgroundToCharacter();
            {
                aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_PREVIEW_CHARACTER));
                rPage.PageCreated(aSet);
            }
        break;

        case TP_PARA_STD:
            {
                if( rPage.GetItemSet().GetParent() )
                {
                    /* CHINA001 different bit represent call to different method of SvxStdParagraphTabPage
                        0x0001 --->EnableRelativeMode()
                        0x0002 --->EnableRegisterMode()
                        0x0004 --->EnableAutoFirstLine()
                        0x0008 --->EnableNegativeMode()


                    */
                    //CHINA001 ((SvxStdParagraphTabPage&)rPage).EnableRelativeMode();
                    //CHINA001 ((SvxStdParagraphTabPage&)rPage).EnableRegisterMode();
                    //CHINA001 ((SvxStdParagraphTabPage&)rPage).EnableAutoFirstLine();
                    //CHINA001 ((SvxStdParagraphTabPage&)rPage).EnableAbsLineDist(MM50/2);
                    //CHINA001 ((SvxStdParagraphTabPage&)rPage).EnableNegativeMode();

                    aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_ABSLINEDIST,MM50/2));
                    aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_FLAGSET,0x000F));
                    rPage.PageCreated(aSet);//add CHINA001
                }

            }
            break;
        case TP_NUMPARA:
            {
                ListBox & rBox = ((SwParagraphNumTabPage&)rPage).GetStyleBox();
                SfxStyleSheetBasePool* pPool = pWrtShell->GetView().GetDocShell()->GetStyleSheetPool();
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
        break;
        case TP_PARA_ALIGN:
            {
                //CHINA001 ((SvxParaAlignTabPage&)rPage).EnableJustifyExt();

                aSet.Put(SfxBoolItem(SID_SVXPARAALIGNTABPAGE_ENABLEJUSTIFYEXT,TRUE));
                rPage.PageCreated(aSet);//add CHINA001
            }
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
                List aList;
                String* pNew = new String;
                SwStyleNameMapper::FillUIName( RES_POOLCOLL_TEXT, *pNew );
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
                //CHINA001 ((SvxPageDescPage&)rPage).SetCollectionList(&aList);
                aSet.Put (SfxStringListItem(SID_COLLECT_LIST, &aList)); //CHINA001
                rPage.PageCreated(aSet); //CHINA001
                for( USHORT i = (USHORT)aList.Count(); i; --i )
                    delete (String*)aList.Remove(i);
            }
            break;

        case TP_MACRO_ASSIGN:
            SwMacroAssignDlg::AddEvents( (SfxMacroTabPage&)rPage, MACASSGN_ALLFRM);
            break;

        case RID_SVXPAGE_PICK_NUM:
            {

                //CHINA001 ((SvxNumPickTabPage&)rPage).SetCharFmtNames(sNumCharFmt, sBulletCharFmt);
                aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFmt));//add CHINA001
                aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFmt)); //add CHINA001
                rPage.PageCreated(aSet);//add CHINA001
            }
        break;
        case RID_SVXPAGE_NUM_OPTIONS:
        {

            //CHINA001 ((SvxNumOptionsTabPage&)rPage).SetCharFmts(sNumCharFmt, sBulletCharFmt);
            aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFmt));//add CHINA001
            aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFmt)); //add CHINA001
            //CHINA001 ListBox& rCharFmtLB = ((SvxNumOptionsTabPage&)rPage).GetCharFmtListBox();
            // Zeichenvorlagen sammeln
            ListBox rCharFmtLB(this); //add CHINA001
            rCharFmtLB.Clear();
            rCharFmtLB.InsertEntry( ViewShell::GetShellRes()->aStrNone );
            SwDocShell* pDocShell = ::GetActiveWrtShell()->GetView().GetDocShell();
            ::FillCharStyleListBox(rCharFmtLB,  pDocShell);
            //add CHINA001 begin
            List aList;
            for(USHORT j = 0; j < rCharFmtLB.GetEntryCount(); j++)
            {

                 aList.Insert( new XubString(rCharFmtLB.GetEntry(j)), LIST_APPEND );
            }
            aSet.Put( SfxStringListItem( SID_CHAR_FMT_LIST_BOX,&aList ) ) ;
            //add end of CHINA001
            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebDocShell, pDocShell));
            //CHINA001 ((SvxNumOptionsTabPage&)rPage).SetMetric(eMetric);
            aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM,eMetric));//add CHINA001
            rPage.PageCreated(aSet);//add CHINA001
            for( USHORT i = (USHORT)aList.Count(); i; --i )
                    delete (XubString*)aList.Remove(i);
            aList.Clear();
        }
        break;
        case RID_SVXPAGE_NUM_POSITION:
        {
            SwDocShell* pDocShell = ::GetActiveWrtShell()->GetView().GetDocShell();
            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebDocShell, pDocShell));
            //CHINA001 ((SvxNumPositionTabPage&)rPage).SetMetric(eMetric);

            aSet.Put ( SfxAllEnumItem(SID_METRIC_ITEM,eMetric));//add CHINA001
            rPage.PageCreated(aSet);//add CHINA001
        }
        break;
        case  RID_SVXPAGE_PICK_BULLET :
            {

                //CHINA001 ((SvxBulletPickTabPage&)rPage).SetCharFmtName(sBulletCharFmt);
                aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFmt)); //add CHINA001
                rPage.PageCreated(aSet);//add CHINA001
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
             //CHINA001   ((SvxBorderTabPage&)rPage).SetSWMode(SW_BORDER_MODE_PARA);
                aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,SW_BORDER_MODE_PARA));
            }
            else if( SFX_STYLE_FAMILY_FRAME == nType )
            {
               //CHINA001 ((SvxBorderTabPage&)rPage).SetSWMode(SW_BORDER_MODE_FRAME);
                aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,SW_BORDER_MODE_FRAME));
            }
            rPage.PageCreated(aSet);

        break;
    }
}



