/*************************************************************************
 *
 *  $RCSfile: textsh.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-14 18:32:27 $
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

#define _SW_FRMVALID_HXX
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif


#ifndef __RSC //autogen
#include <tools/errinf.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _PLUGIN_HXX //autogen wg. SvPlugInObject
#include <so3/plugin.hxx>
#endif
#ifndef _SFXPTITEM_HXX //autogen
#include <svtools/ptitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef _SFXMSG_HXX //autogen
#include <sfx2/msg.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX //autogen
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SVX_HLNKITEM_HXX //autogen
#include <svx/hlnkitem.hxx>
#endif
#ifndef _SBASLTID_HRC //autogen
#include <offmgr/sbasltid.hrc>
#endif
#ifndef _SVX_SRCHITEM_HXX //autogen
#include <svx/srchitem.hxx>
#endif
#ifndef _SVX_CHARMAP_HXX //autogen
#include <svx/charmap.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <svtools/sbxvar.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <svx/opaqitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX //autogen
#include <svx/sizeitem.hxx>
#endif
#ifndef _MySVXACORR_HXX
#include <svx/svxacorr.hxx>
#endif
#ifndef _FILTER_HXX //autogen
#include <svtools/filter.hxx>
#endif
#ifndef SMDLL_HXX
#include <starmath/smdll0.hxx>
#endif
#ifndef _SCHDLL0_HXX
#include <sch/schdll0.hxx>
#endif
#ifndef _SCH_DLL_HXX
#include <sch/schdll.hxx>
#endif
#ifndef _SCH_MEMCHRT_HXX
#include <sch/memchrt.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif
#ifndef _SVX_PFILEDLG_HXX //autogen wg. SvxPluginFileDlg
#include <svx/pfiledlg.hxx>
#endif
#ifndef _OFA_HTMLCFG_HXX //autogen
#include <offmgr/htmlcfg.hxx>
#endif
#ifndef _OFAACCFG_HXX //autogen
#include <offmgr/ofaaccfg.hxx>
#endif

#ifndef _SWWDOCSH_HXX //autogen
#include <wdocsh.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _FRMMGR_HXX
#include <frmmgr.hxx>
#endif
#ifndef _TEXTSH_HXX
#include <textsh.hxx>
#endif
#ifndef _INSTABLE_HXX
#include <instable.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _TABLEMGR_HXX
#include <tablemgr.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>       // fuer Undo-IDs
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _FRMDLG_HXX
#include <frmdlg.hxx>
#endif
#ifndef _USRPREF_HXX
#include <usrpref.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _TBLAFMT_HXX
#include <tblafmt.hxx>
#endif
#ifndef _INSRULE_HXX
#include <insrule.hxx>
#endif
#ifndef _CAPTION_HXX
#include <caption.hxx>
#endif
#ifndef _IDXMRK_HXX
#include <idxmrk.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif

#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif
#ifndef _POPUP_HRC
#include <popup.hrc>
#endif
#ifndef _SWERROR_H
#include <swerror.h>
#endif
#ifndef _MODOPT_HXX //autogen
#include <modcfg.hxx>
#endif
#ifndef _COLUMN_HXX
#include <column.hxx>
#endif

#define SwTextShell
#define Paragraph
#define HyphenZone
#define TextFont
#define DropCap
#ifndef _ITEMDEF_HXX
#include <itemdef.hxx>
#endif
#ifndef _SWSLOTS_HXX
#include <swslots.hxx>
#endif

#define C2S(cChar) UniString::CreateFromAscii(cChar)

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SFX_IMPL_INTERFACE(SwTextShell, SwBaseShell, SW_RES(STR_SHELLNAME_TEXT))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_TEXT_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_TEXT_TOOLBOX));
    SFX_OBJECTMENU_REGISTRATION(SID_OBJECTMENU0, SW_RES(MN_OBJECTMENU_TEXT));
    SFX_CHILDWINDOW_REGISTRATION(FN_EDIT_FORMULA);
    SFX_CHILDWINDOW_REGISTRATION(SID_INSERT_DIAGRAM);
    SFX_CHILDWINDOW_REGISTRATION(FN_INSERT_FIELD);
    SFX_CHILDWINDOW_REGISTRATION(FN_INSERT_IDX_ENTRY_DLG);
    SFX_CHILDWINDOW_REGISTRATION(FN_INSERT_AUTH_ENTRY_DLG);
}



TYPEINIT1(SwTextShell,SwBaseShell)



void SwTextShell::ExecInsert(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();

    ASSERT( !rSh.IsObjSelected() && !rSh.IsFrmSelected(),
            "Falsche Shell auf dem Dispatcher" );

    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = 0;
    USHORT nSlot = rReq.GetSlot();
    if(pArgs)
        pArgs->GetItemState(nSlot, FALSE, &pItem );

    switch( nSlot )
    {
    case FN_INSERT_SOFT_HYPHEN:
        if( CHAR_SOFTHYPHEN != rSh.SwCrsrShell::GetChar( TRUE, 0 ) &&
            CHAR_SOFTHYPHEN != rSh.SwCrsrShell::GetChar( TRUE, -1 ))
            rSh.Insert( String( CHAR_SOFTHYPHEN ) );
        break;

    case FN_INSERT_HARDHYPHEN:
    case FN_INSERT_HARD_SPACE:
        {
            sal_Unicode cIns = FN_INSERT_HARD_SPACE == nSlot ? CHAR_HARDBLANK
                                                             : CHAR_HARDHYPHEN;

            SvxAutoCorrect* pACorr = OFF_APP()->GetAutoCorrConfig()->GetAutoCorrect();
            if( pACorr && pACorr->IsAutoCorrFlag( CptlSttSntnc | CptlSttWrd |
                                ChgFractionSymbol | ChgOrdinalNumber |
                                ChgToEnEmDash | SetINetAttr | Autocorrect ))
                rSh.AutoCorrect( *pACorr, cIns );
            else
                rSh.Insert( String( cIns ) );
        }
        break;

    case FN_INSERT_BREAK:
        rSh.SplitNode();
    break;
    case FN_INSERT_PAGEBREAK:
        rSh.InsertPageBreak();
    break;
    case FN_INSERT_LINEBREAK:
        rSh.InsertLineBreak();
    break;
    case FN_INSERT_COLUMN_BREAK:
        rSh.InsertColumnBreak();
    break;
    case SID_HYPERLINK_SETLINK:
        if (pItem)
            InsertHyperlink(*((const SvxHyperlinkItem *)pItem));
        break;
    case  SID_INSERT_SOUND:
    case  SID_INSERT_VIDEO:
    {
        SvxPluginFileDlg aDlg( &GetView().GetViewFrame()->GetWindow(), nSlot );

        if ( RET_OK == aDlg.Execute() )
        {
            // URL ermitteln
            String aStrURL( aDlg.GetPath() );
            aStrURL = URIHelper::SmartRelToAbs( aStrURL );

            INetURLObject* pURL = new INetURLObject();
            pURL->SetSmartProtocol( INET_PROT_FILE );

            if ( pURL->SetURL( aStrURL, INetURLObject::WAS_ENCODED ) )
            {
                SvFactory * pFactory = SvFactory::GetDefaultPlugInFactory();
                SvStorageRef aStor = new SvStorage( aEmptyStr, STREAM_STD_READWRITE );
                SvPlugInObjectRef xObj = &pFactory->CreateAndInit( *pFactory, aStor );
                xObj->SetPlugInMode( (USHORT)PLUGIN_EMBEDED );
                xObj->SetURL( *pURL );
                SvPlugInObject* pObj = &xObj;
                SvInPlaceObjectRef *pxIns = new SvInPlaceObjectRef(pObj);
                rSh.Insert( pxIns, 0, TRUE, nSlot);
            }
        }
    }
    break;
    case SID_INSERT_OBJECT:
    case SID_INSERT_PLUGIN:
    case SID_INSERT_APPLET:
    case SID_INSERT_FLOATINGFRAME:
        rSh.Insert( (SvInPlaceObjectRef*)0, 0, TRUE, nSlot);
        break;

    case SID_INSERT_DIAGRAM:
        {
            SvtModuleOptions aMOpt;
            if ( !aMOpt.IsChart() )
                break;
            if(!rReq.IsAPI())
            {
                SfxViewFrame* pVFrame = GetView().GetViewFrame();
                pVFrame->ToggleChildWindow( SID_INSERT_DIAGRAM );
            }
            else
            {
                const SwFrmFmt* pTFmt = rSh.GetTableFmt();
                if( pTFmt && !rSh.IsTblComplexForChart() )
                {
                    SchMemChart* pData = 0;
                    rSh.UpdateChartData( pTFmt->GetName(), pData );

                    SwTableFUNC( &rSh, FALSE ).InsertChart( *pData );
                    rSh.LaunchOLEObj();

                    delete pData;
                }
                else
                {
                    rSh.Insert( 0, SCH_MOD()->pSchChartDocShellFactory );
                }
                SvInPlaceObjectRef xOLE = rSh.GetOLEObj();
                if(pItem && xOLE.Is())
                {
                    Size aSize(((SvxSizeItem*)pItem)->GetSize());
                    aSize = OutputDevice::LogicToLogic
                                    ( aSize, MapMode( MAP_TWIP ), MapMode( MAP_100TH_MM ) );

                    if(aSize.Width() > MINLAY&& aSize.Height()> MINLAY)
                    {
                        Rectangle aRect = xOLE->GetVisArea();
                        aRect.SetSize(aSize);
                        xOLE->SetVisArea( aRect );
                    }
                }
            }
        }
        break;

    case FN_INSERT_SMA:
        {
            rSh.Insert( 0, SM_MOD()->pSmDocShellFactory );
        }
        break;

    case FN_INSERT_TABLE:
        if( !rSh.GetTableFmt() && !(rSh.GetFrmType(0,TRUE) & FRMTYPE_FOOTNOTE))
        {
            SwView &rView = GetView(); // Da GetView() nach Shellwechsel nicht mehr geht
            BOOL bHTMLMode = 0 != (::GetHtmlMode(rView.GetDocShell())&HTMLMODE_ON);
            BOOL bCallEndUndo = FALSE;

            if( !pArgs && rSh.IsSelection() && !rSh.IsInClickToEdit() )
            {
                const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
                USHORT nInsTblFlags = pModOpt->GetInsTblFlags(bHTMLMode);

                rSh.StartUndo(UNDO_INSTABLE);
                bCallEndUndo = TRUE;

                BOOL bInserted = rSh.TextToTable('\t', HORI_FULL, nInsTblFlags);
                rSh.EnterStdMode();
                if (bInserted)
                    rView.AutoCaption(TABLE_CAP);
            }
            else
            {
                USHORT nCols = 0;
                USHORT nRows = 0;
                USHORT nInsTblFlags = ALL_TBL_INS_ATTR;
                String aTableName;
                SwTableAutoFmt* pTAFmt = 0;

                if( pArgs && pArgs->Count() >= 2 )
                {
                    nCols = ((SfxUInt16Item &)pArgs->Get(SID_ATTR_TABLE_COLUMN)).GetValue();
                    nRows = ((SfxUInt16Item &)pArgs->Get(SID_ATTR_TABLE_ROW)).GetValue();

                    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
                    nInsTblFlags = pModOpt->GetInsTblFlags(bHTMLMode);
                }

                if( !nCols || !nRows )
                {
                    SwInsTableDlg *pDlg = new SwInsTableDlg(rView);
                    if( RET_OK == pDlg->Execute() )
                    {
                        pDlg->GetValues( aTableName, nRows, nCols, nInsTblFlags, pTAFmt );
                    }
                    delete pDlg;
                }

                if( nCols && nRows )
                {
                    rSh.StartUndo(UNDO_INSTABLE);
                    bCallEndUndo = TRUE;

                    rSh.StartAllAction();
                    if( rSh.HasSelection() )
                        rSh.DelRight();

                    rSh.InsertTable( nRows, nCols, HORI_FULL, nInsTblFlags, pTAFmt );
                    rSh.MoveTable( fnTablePrev, fnTableStart );

                    if( aTableName.Len() && !rSh.GetTblStyle( aTableName ) )
                        rSh.GetTableFmt()->SetName( aTableName );

                    rSh.EndAllAction();
                    rView.AutoCaption(TABLE_CAP);
                }
                delete pTAFmt;
            }
            if( bCallEndUndo )
                rSh.EndUndo(UNDO_INSTABLE); // wegen moegl. Shellwechsel
        }
        break;

    case FN_INSERT_FRAME_INTERACT_NOCOL:
    case FN_INSERT_FRAME_INTERACT:
    {
        USHORT nCols = 1;
        if(FN_INSERT_FRAME_INTERACT_NOCOL != nSlot &&
            pArgs &&pArgs->GetItemState(SID_ATTR_COLUMNS, FALSE, &pItem) == SFX_ITEM_SET)
            nCols = ((SfxUInt16Item *)pItem)->GetValue();
        GetView().InsFrmMode(nCols);
    }
    break;
    case FN_INSERT_FRAME:
    {
        const int nSel = rSh.GetSelectionType();

        BOOL bSingleCol = FALSE;
        if( 0!= PTR_CAST(SwWebDocShell, GetView().GetDocShell()) )
        {
            OfaHtmlOptions* pHtmlOpt = OFF_APP()->GetHtmlOptions();
            USHORT nExport = pHtmlOpt->GetExportMode();
            if( HTML_CFG_MSIE == nExport ||
                HTML_CFG_HTML32 == nExport ||
                HTML_CFG_MSIE_40 == nExport ||
                HTML_CFG_HTML32 == nExport )
            {
                bSingleCol = TRUE;
            }

        }
        // Rahmen neu anlegen
        SwFlyFrmAttrMgr aMgr( TRUE, GetShellPtr(), FRMMGR_TYPE_TEXT );
        static USHORT __READONLY_DATA aFrmAttrRange[] =
        {
            RES_FRMATR_BEGIN,       RES_FRMATR_END-1,
            SID_ATTR_BORDER_INNER,  SID_ATTR_BORDER_INNER,
            FN_GET_PRINT_AREA,      FN_GET_PRINT_AREA,
            SID_ATTR_PAGE_SIZE,     SID_ATTR_PAGE_SIZE,
            FN_SET_FRM_NAME,        FN_SET_FRM_NAME,
            SID_HTML_MODE,          SID_HTML_MODE,
            0
        };

        SfxItemSet aSet(GetPool(), aFrmAttrRange );
        aSet.Put(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(GetView().GetDocShell())));
        const SwRect &rPg = GetShell().GetAnyCurRect(RECT_PAGE);
        SwFmtFrmSize aFrmSize(ATT_VAR_SIZE, rPg.Width(), rPg.Height());
        aFrmSize.SetWhich(GetPool().GetWhich(SID_ATTR_PAGE_SIZE));
        aSet.Put(aFrmSize);

        const SwRect &rPr = GetShell().GetAnyCurRect(RECT_PAGE_PRT);
        SwFmtFrmSize aPrtSize(ATT_VAR_SIZE, rPr.Width(), rPr.Height());
        aPrtSize.SetWhich(GetPool().GetWhich(FN_GET_PRINT_AREA));
        aSet.Put(aPrtSize);

        aSet.Put(aMgr.GetAttrSet());
        aSet.SetParent( aMgr.GetAttrSet().GetParent() );

        // Minimalgroesse in Spalten l”schen
        SvxBoxInfoItem aBoxInfo((SvxBoxInfoItem &)aSet.Get(SID_ATTR_BORDER_INNER));
        aBoxInfo.SetMinDist(FALSE);
        aSet.Put(aBoxInfo);

        SwFrmDlg* pDlg = new SwFrmDlg(GetView().GetViewFrame(), &GetView().GetViewFrame()->GetWindow(), aSet, TRUE);

        if(pDlg->Execute() && pDlg->GetOutputItemSet())
        {
            GetShell().StartAllAction();
            GetShell().StartUndo(UNDO_INSERT);

            const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
            aMgr.SetAttrSet(*pOutSet);

            // beim ClickToEditFeld erst die Selektion loeschen
            if( GetShell().IsInClickToEdit() )
                GetShell().DelRight();

            aMgr.InsertFlyFrm();


            GetView().AutoCaption(FRAME_CAP);
            GetShell().EndUndo(UNDO_INSERT);
            GetShell().EndAllAction();
        }

        DELETEZ(pDlg);
        break;
    }
    case FN_INSERT_HRULER:
    {
        String sPath;
        BOOL bSimpleLine = FALSE;
        BOOL bRet = FALSE;
        Window* pParent = GetView().GetWindow();
        SwInsertGrfRulerDlg* pDlg = new SwInsertGrfRulerDlg(
            pParent ,
            GetView().GetDocShell());
        // MessageBox fuer fehlende Grafiken
        if(!pDlg->HasImages())
            InfoBox( pParent, SW_RES(MSG_NO_RULER)).Execute();
        if(RET_OK == pDlg->Execute())
        {
            sPath = pDlg->GetGraphicName();
            bSimpleLine = pDlg->IsSimpleLine();
        }
        delete pDlg;
        rSh.StartAllAction();
        rSh.StartUndo(UIUNDO_INSERT_RULER);
        if(bSimpleLine)
        {
            if(!(rSh.IsSttOfPara() && rSh.IsEndOfPara())) // kein leerer Absatz?
                rSh.SplitNode( FALSE, FALSE ); // dann Platz schaffen
            rSh.SplitNode( FALSE, FALSE );
            rSh.Left();
            rSh.SetTxtFmtColl( rSh.GetTxtCollFromPool( RES_POOLCOLL_HTML_HR ));
            rSh.Right();
            bRet = TRUE;
        }
        else if(sPath.Len())
        {
            if (!pFrmMgr)
                pFrmMgr = new SwFlyFrmAttrMgr( TRUE, &rSh, FRMMGR_TYPE_GRF );
            // am FrmMgr muessen die richtigen Parameter eingestellt werden

            pFrmMgr->SetAnchor(FLY_IN_CNTNT);

            rSh.SplitNode( FALSE, FALSE );
            rSh.SplitNode( FALSE, FALSE );
            rSh.Left();
            rSh.SetAttr(SvxAdjustItem(SVX_ADJUST_CENTER,RES_PARATR_ADJUST ));
            if(GRFILTER_OK == InsertGraphic(sPath, aEmptyStr, TRUE, 0, 0 ))
                bRet = TRUE;
            rSh.EnterStdMode();
            rSh.Right();
            DELETEZ(pFrmMgr);
        }
        rSh.EndAllAction();
        rSh.EndUndo(UIUNDO_INSERT_RULER);
        rReq.SetReturnValue(SfxBoolItem(nSlot, bRet));
    }
    break;
    case FN_FORMAT_COLUMN :
    {
        SwColumnDlg* pColDlg = new SwColumnDlg(GetView().GetWindow(), rSh);
        pColDlg->Execute();
        delete pColDlg;
    }
    break;

    case FN_SECTION_APPENDNODE:
        rReq.SetReturnValue( SfxBoolItem( nSlot, rSh.AppendNodeInSection() ));
        break;

    default:
        ASSERT( !this, "falscher Dispatcher" );
        return;
    }
}



void SwTextShell::StateInsert( SfxItemSet &rSet )
{
    USHORT nHtmlMode = ::GetHtmlMode(GetView().GetDocShell());
    SfxWhichIter aIter( rSet );
    SwWrtShell &rSh = GetShell();
    USHORT nWhich = aIter.FirstWhich();
    SvtModuleOptions aMOpt;
    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_INSERT_SOUND:
            case SID_INSERT_VIDEO:
                if ( GetShell().IsSelFrmMode() ||
                     !SvxPluginFileDlg::IsAvailable( nWhich ) ||
                     SFX_CREATE_MODE_EMBEDDED == GetView().GetDocShell()->GetCreateMode() )
                {
                    rSet.DisableItem( nWhich );
                }
                break;

            case SID_INSERT_DIAGRAM:
                if( !aMOpt.IsChart() )
                {
                    rSet.DisableItem( nWhich );
                    break;
                }

            case FN_INSERT_SMA:
                if( FN_INSERT_SMA == nWhich && !aMOpt.IsMath() )
                {
                    rSet.DisableItem( nWhich );
                    break;
                }

            case SID_INSERT_FLOATINGFRAME:
            case SID_INSERT_OBJECT:
            case SID_INSERT_PLUGIN:
            case SID_INSERT_APPLET:
            {
                SfxObjectCreateMode eMode =
                                    GetView().GetDocShell()->GetCreateMode();

                if(
#ifndef SOLAR_JAVA
                    nWhich == SID_INSERT_APPLET ||
#endif
                    eMode == SFX_CREATE_MODE_EMBEDDED ||
                    (nWhich == SID_INSERT_DIAGRAM && rSh.IsCrsrInTbl() &&
                     rSh.IsTblComplexForChart() &&
                     !GetView().GetViewFrame()->GetChildWindow(
                            SID_INSERT_DIAGRAM)) )
                    rSet.DisableItem( nWhich );
                else if( GetShell().IsSelFrmMode())
                    rSet.DisableItem( nWhich );
                else if(SID_INSERT_FLOATINGFRAME == nWhich && nHtmlMode&HTMLMODE_ON)
                {
                    OfaHtmlOptions* pHtmlOpt = OFF_APP()->GetHtmlOptions();
                    USHORT nExport = pHtmlOpt->GetExportMode();
                    if(HTML_CFG_MSIE_40 != nExport && HTML_CFG_WRITER != nExport )
                        rSet.DisableItem(nWhich);
                }
            }
            break;
            case FN_INSERT_FRAME_INTERACT_NOCOL :
            case FN_INSERT_FRAME_INTERACT:
            {
                if ( GetShell().IsSelFrmMode() ||
                    (0 != (nHtmlMode & HTMLMODE_ON) && 0 == (nHtmlMode & HTMLMODE_SOME_ABS_POS)))
                    rSet.DisableItem(nWhich);
            }
            break;
            case SID_HYPERLINK_GETLINK:
                {
                    SfxItemSet aSet(GetPool(), RES_TXTATR_INETFMT, RES_TXTATR_INETFMT);
                    rSh.GetAttr( aSet );

                    SvxHyperlinkItem aHLinkItem;
                    const SfxPoolItem* pItem;
                    if(SFX_ITEM_SET == aSet.GetItemState(RES_TXTATR_INETFMT, FALSE, &pItem))
                    {
                        const SwFmtINetFmt* pINetFmt = (const SwFmtINetFmt*)pItem;
                        aHLinkItem.SetURL(pINetFmt->GetValue());
                        aHLinkItem.SetTargetFrame(pINetFmt->GetTargetFrame());
                        aHLinkItem.SetIntName(pINetFmt->GetName());
                        const SvxMacro *pMacro = pINetFmt->GetMacro( SFX_EVENT_MOUSEOVER_OBJECT );
                        if( pMacro )
                            aHLinkItem.SetMacro(HYPERDLG_EVENT_MOUSEOVER_OBJECT, *pMacro);

                        pMacro = pINetFmt->GetMacro( SFX_EVENT_MOUSECLICK_OBJECT );
                        if( pMacro )
                            aHLinkItem.SetMacro(HYPERDLG_EVENT_MOUSECLICK_OBJECT, *pMacro);

                        pMacro = pINetFmt->GetMacro( SFX_EVENT_MOUSEOUT_OBJECT );
                        if( pMacro )
                            aHLinkItem.SetMacro(HYPERDLG_EVENT_MOUSEOUT_OBJECT, *pMacro);

                        // Text des Links besorgen
                        rSh.StartAction();
                        rSh.CreateCrsr();
                        rSh.SwCrsrShell::SelectTxtAttr(RES_TXTATR_INETFMT,TRUE);
                        String sLinkName = rSh.GetSelTxt();
                        aHLinkItem.SetName(sLinkName);
                        aHLinkItem.SetInsertMode(HLINK_FIELD);
                        rSh.DestroyCrsr();
                        rSh.EndAction();
                    }
                    else
                    {
                        String sReturn = rSh.GetSelTxt();
                        sReturn.Erase(255);
                        sReturn.EraseTrailingChars();
                        aHLinkItem.SetName(sReturn);
                    }

                    aHLinkItem.SetInsertMode((SvxLinkInsertMode)(aHLinkItem.GetInsertMode() |
                        ((nHtmlMode & HTMLMODE_ON) != 0 ? HLINK_HTMLMODE : 0)));
                    aHLinkItem.SetMacroEvents ( HYPERDLG_EVENT_MOUSEOVER_OBJECT|
                                HYPERDLG_EVENT_MOUSECLICK_OBJECT | HYPERDLG_EVENT_MOUSEOUT_OBJECT );

                    rSet.Put(aHLinkItem);
                }
                break;

            case FN_INSERT_FRAME:
                if(rSh.IsSelFrmMode())
                {
                    const int nSel = rSh.GetSelectionType();
                    if( (SwWrtShell::SEL_GRF | SwWrtShell::SEL_OLE ) & nSel )
                        rSet.DisableItem(nWhich);
                }
            break;
            case FN_INSERT_HRULER :
                if(rSh.IsReadOnlyAvailable() && rSh.HasReadonlySel() )
                    rSet.DisableItem(nWhich);
            break;

            case FN_SECTION_APPENDNODE:
                if( !rSh.CanInsertNodeAtEndOfSection() )
                    rSet.DisableItem(nWhich);
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void  SwTextShell::ExecDelete(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    switch( rReq.GetSlot() )
    {
        case FN_DELETE_SENT:
            if( rSh.IsTableMode() )
            {
                rSh.DeleteRow();
                rSh.EnterStdMode();
            }
            else
                rSh.DelToEndOfSentence();
            break;
        case FN_DELETE_BACK_SENT:
            rSh.DelToStartOfSentence();
            break;
        case FN_DELETE_WORD:
            rSh.DelNxtWord();
            break;
        case FN_DELETE_BACK_WORD:
            rSh.DelPrvWord();
            break;
        case FN_DELETE_LINE:
            rSh.DelToEndOfLine();
            break;
        case FN_DELETE_BACK_LINE:
            rSh.DelToStartOfLine();
            break;
        case FN_DELETE_PARA:
            rSh.DelToEndOfPara();
            break;
        case FN_DELETE_BACK_PARA:
            rSh.DelToStartOfPara();
            break;
        case FN_DELETE_WHOLE_LINE:
            rSh.DelLine();
            break;
        default:
            ASSERT(!this, "falscher Dispatcher");
            return;
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



SwTextShell::SwTextShell(SwView &rView) :
    SwBaseShell(rView), pPostItFldMgr( 0 ), pInsGrfSetPtr( 0 )
{
    SetName(String::CreateFromAscii("Text"));
    SetHelpId(SW_TEXTSHELL);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwTextShell::~SwTextShell()
{
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwTextShell::InsertSymbol(const String& rChars, const String& rFontName)
{
    SwWrtShell &rSh = GetShell();
    String aChars( rChars );

    SfxItemSet aSet( GetPool(), RES_CHRATR_FONT, RES_CHRATR_FONT );
    rSh.GetAttr( aSet );
    SvxFontItem aFont( (SvxFontItem &) aSet.Get( RES_CHRATR_FONT ));
    Font aNewFont(rFontName, Size(1,1)); // Size nur wg. CTOR

    if( !rChars.Len() )
    {
        // Eingestellten Font als Default
        SvxCharacterMap* pDlg = new SvxCharacterMap(&GetView().GetViewFrame()->GetWindow(), FALSE);

        Font aDlgFont( pDlg->GetCharFont() );
        SwViewOption aOpt(*GetShell().GetViewOptions());
        String sSymbolFont = aOpt.GetSymbolFont();
        if(!rFontName.Len() && sSymbolFont.Len())
            aDlgFont.SetName(sSymbolFont);
        else
            aDlgFont.SetName( aFont.GetFamilyName() );

        pDlg->SetCharFont( aDlgFont );

        if( RET_OK == pDlg->Execute() )
        {
            aChars = pDlg->GetCharacters();
            aNewFont = pDlg->GetCharFont();
            aOpt.SetSymbolFont(aNewFont.GetName());
            SW_MOD()->ApplyUsrPref(aOpt, &GetView());
        }
        delete pDlg;
    }

    if( aChars.Len() )
    {
        rSh.StartAllAction();

        // Selektierten Inhalt loeschen
        rSh.StartUndo( UNDO_INSERT );
        if ( rSh.HasSelection() )
        {
            rSh.DelRight();
            aSet.ClearItem();
            rSh.GetAttr( aSet );
            aFont = (SvxFontItem &) aSet.Get( RES_CHRATR_FONT );
        }

        // Zeichen einfuegen
        rSh.Insert( aChars );

        // Muss der Font geaendert werden
        if( aNewFont.GetName() != aFont.GetFamilyName() )
        {
            SvxFontItem aNewFontItem( aFont );
            aNewFontItem.GetFamilyName() = aNewFont.GetName();
            aNewFontItem.GetFamily()     = aNewFont.GetFamily();
            aNewFontItem.GetPitch()      = aNewFont.GetPitch();
            aNewFontItem.GetCharSet()    = aNewFont.GetCharSet();
            aSet.Put(aNewFontItem);

            rSh.SetMark();
            rSh.ExtendSelection( FALSE, aChars.Len() );
            rSh.SetAttr( aSet, SETATTR_DONTEXPAND | SETATTR_NOFORMATATTR );
            if( !rSh.IsCrsrPtAtEnd() )
                rSh.SwapPam();
            rSh.ClearMark();
            rSh.UpdateAttr();
        }

        rSh.EndAllAction();
        rSh.EndUndo( UNDO_INSERT );
    }
}


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.2  2000/10/05 12:27:50  jp
    should change: remove image

    Revision 1.1.1.1  2000/09/18 17:14:47  hr
    initial import

    Revision 1.356  2000/09/18 16:06:06  willem.vandorp
    OpenOffice header added.

    Revision 1.355  2000/09/08 08:12:52  os
    Change: Set/Toggle/Has/Knows/Show/GetChildWindow

    Revision 1.354  2000/08/17 11:40:16  jp
    remove the SW graphicmanager

    Revision 1.353  2000/08/07 20:40:21  jp
    ExectInsert: don't insert a softhyphen behind/befor another sofhyphen

    Revision 1.352  2000/08/04 10:54:23  jp
    Soft-/HardHyphens & HardBlanks changed from attribute to unicode character; remove unused methods

    Revision 1.351  2000/07/26 16:27:20  jp
    Bug #77158#: remove the unused code

    Revision 1.350  2000/07/07 13:24:57  jp
    must changes VCL

    Revision 1.349  2000/06/27 17:43:21  jp
    Bug #76332#: internal SplitNode don't insert a new node before a table

    Revision 1.348  2000/06/26 13:17:56  os
    INetURLObject::SmartRelToAbs removed

    Revision 1.347  2000/06/20 14:49:23  os
    #70060# less occurences of columns in HTML

    Revision 1.346  2000/05/26 07:21:33  os
    old SW Basic API Slots removed

    Revision 1.345  2000/05/10 11:53:02  os
    Basic API removed

    Revision 1.344  2000/05/09 14:41:35  os
    BASIC interface partially removed

------------------------------------------------------------------------*/


