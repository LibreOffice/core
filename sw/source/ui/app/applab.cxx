/*************************************************************************
 *
 *  $RCSfile: applab.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2000-10-27 14:29:44 $
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

#if STLPORT_VERSION>=321
#include <cstdarg>
#endif

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include <hintids.hxx>

#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SBASLTID_HRC //autogen
#include <offmgr/sbasltid.hrc>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SVX_PBINITEM_HXX //autogen
#include <svx/pbinitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX
#include <svx/paperinf.hxx>
#endif
#ifndef _LINKMGR_HXX
#include <so3/linkmgr.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _GLOSHDL_HXX
#include <gloshdl.hxx>
#endif
#ifndef _GLOSSARY_HXX
#include <glossary.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _FLDMGR_HXX
#include <fldmgr.hxx>
#endif
#ifndef _LABEL_HXX
#include <label.hxx>
#endif
#ifndef _LABIMG_HXX
#include <labimg.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif

#ifndef _APP_HRC
#include <app.hrc>
#endif
#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>
#endif

using namespace ::com::sun::star;
using namespace ::rtl;

// steht im appenv.cxx
extern sal_Bool InsertLabEnvText( SwWrtShell& , SwFldMgr& , const String& );

const char __FAR_DATA MASTER_LABEL[] = "MasterLabel";

// --------------------------------------------------------------------------

const SwFrmFmt *lcl_InsertBCText( SwWrtShell& rSh, const SwLabItem& rItem,
                        SwFrmFmt &rFmt,
                        sal_uInt16 nCol, sal_uInt16 nRow, sal_Bool bPage)
{
    SfxItemSet aSet(rSh.GetAttrPool(), RES_ANCHOR, RES_ANCHOR,
                        RES_VERT_ORIENT, RES_VERT_ORIENT, RES_HORI_ORIENT, RES_HORI_ORIENT, 0 );
    sal_uInt16 nPhyPageNum, nVirtPageNum;
    rSh.GetPageNum( nPhyPageNum, nVirtPageNum );

    aSet.Put(SwFmtAnchor(bPage ? FLY_IN_CNTNT : FLY_PAGE, nPhyPageNum));
    if (!bPage)
    {
        aSet.Put(SwFmtHoriOrient(rItem.lLeft + nCol * rItem.lHDist,
                                                    HORI_NONE, REL_PG_FRAME ));
        aSet.Put(SwFmtVertOrient(rItem.lUpper + nRow * rItem.lVDist,
                                                    VERT_NONE, REL_PG_FRAME ));
    }
    const SwFrmFmt *pFmt = rSh.NewFlyFrm(aSet, sal_True,  &rFmt );  // Fly einfuegen
    ASSERT( pFmt, "Fly not inserted" );

    rSh.UnSelectFrm();  //Rahmen wurde automatisch selektiert

    rSh.SetTxtFmtColl( rSh.GetTxtCollFromPool( RES_POOLCOLL_STANDARD ) );

    //
    if(!rItem.bSynchron || !(nCol|nRow))
    {
        SwGlossaryDlg::SetActGroup(rItem.sGlossaryGroup);
        SwGlossaryHdl* pGlosHdl = rSh.GetView().GetGlosHdl();
        pGlosHdl->SetCurGroup(rItem.sGlossaryGroup, sal_True);
        pGlosHdl->InsertGlossary( rItem.sGlossaryBlockName );
    }

    return pFmt;
}

const SwFrmFmt *lcl_InsertLabText( SwWrtShell& rSh, const SwLabItem& rItem,
                        SwFrmFmt &rFmt, SwFldMgr& rFldMgr,
                        sal_uInt16 nCol, sal_uInt16 nRow, sal_Bool bLast, sal_Bool bPage)
{
    SfxItemSet aSet(rSh.GetAttrPool(), RES_ANCHOR, RES_ANCHOR,
                        RES_VERT_ORIENT, RES_VERT_ORIENT, RES_HORI_ORIENT, RES_HORI_ORIENT, 0 );
    sal_uInt16 nPhyPageNum, nVirtPageNum;
    rSh.GetPageNum( nPhyPageNum, nVirtPageNum );

    aSet.Put(SwFmtAnchor(bPage ? FLY_IN_CNTNT : FLY_PAGE, nPhyPageNum));
    if (!bPage)
    {
        aSet.Put(SwFmtHoriOrient(rItem.lLeft + nCol * rItem.lHDist,
                                                    HORI_NONE, REL_PG_FRAME ));
        aSet.Put(SwFmtVertOrient(rItem.lUpper + nRow * rItem.lVDist,
                                                    VERT_NONE, REL_PG_FRAME ));
    }
    const SwFrmFmt *pFmt = rSh.NewFlyFrm(aSet, sal_True,  &rFmt );  // Fly einfuegen
    ASSERT( pFmt, "Fly not inserted" );

    rSh.UnSelectFrm();  //Rahmen wurde automatisch selektiert

    rSh.SetTxtFmtColl( rSh.GetTxtCollFromPool( RES_POOLCOLL_STANDARD ) );

    // Ggf. "Naechster Datensatz"
    if( (!rItem.bSynchron || !(nCol|nRow)) && InsertLabEnvText( rSh, rFldMgr, rItem.aWriting ) && !bLast )
        rFldMgr.InsertFld( TYP_DBNEXTSETFLD, 0, String::CreateFromAscii("sal_True"), aEmptyStr, 0, &rSh );

    return pFmt;
}

// ----------------------------------------------------------------------------


void SwModule::InsertLab(SfxRequest& rReq, sal_Bool bLabel)
{
static sal_uInt16 nLabelTitleNo = 0;
static sal_uInt16 nBCTitleNo = 0;

    // DB-Manager anlegen
    SwNewDBMgr* pNewDBMgr = new SwNewDBMgr;

    // SwLabItem aus Config lesen
    SwLabCfgItem aLabCfg(bLabel);

    // Dialog hochfahren
    SfxItemSet aSet( GetPool(), FN_LABEL, FN_LABEL, 0 );
    aSet.Put( aLabCfg.GetItem() );

    SwLabDlg* pDlg = new SwLabDlg(0, aSet, pNewDBMgr, bLabel);

    if ( RET_OK == pDlg->Execute() )
    {
        // Dialog auslesen, Item in Config speichern
        const SwLabItem& rItem = (const SwLabItem&) pDlg->
                                            GetOutputItemSet()->Get(FN_LABEL);
        aLabCfg.GetItem() = rItem;
        pDlg->MakeConfigItem( aLabCfg.GetItem());
        aLabCfg.Commit();

        // Neues Dokument erzeugen.
        SfxObjectShellRef xDocSh( new SwDocShell( SFX_CREATE_MODE_STANDARD));
        xDocSh->DoInitNew( 0 );

        // Drucker
        Printer *pPrt = pDlg->GetPrt();
        if (pPrt)
        {
            SwDocShell *pDocSh = (SwDocShell*)(&*xDocSh);
            pDocSh->GetDoc()->SetJobsetup(pPrt->GetJobSetup());
        }

        const SfxItemSet *pArgs = rReq.GetArgs();
        DBG_ASSERT( pArgs, "no arguments in SfxRequest")
        const SfxPoolItem* pFrameItem = 0;
        if(pArgs)
            pArgs->GetItemState(SID_DOCFRAME, FALSE, &pFrameItem);
        if(!pFrameItem)
            return ;
        SfxFrame* pFr = ((const SfxFrameItem*)pFrameItem)->GetFrame();
        xDocSh->PutItem(SfxBoolItem(SID_HIDDEN, TRUE));
        pFr->InsertDocument(xDocSh);
        SfxViewFrame *pFrame = pFr->GetCurrentViewFrame();
        SwView      *pView = (SwView*) pFrame->GetViewShell();
        pView->AttrChangedNotify( &pView->GetWrtShell() );//Damit SelectShell gerufen wird.

        // Dokumenttitel setzen
        String aTmp;
        if(bLabel)
        {
            aTmp = String(SW_RES( STR_LAB_TITLE));
            aTmp += String::CreateFromInt32(++nLabelTitleNo );
        }
        else
        {
            aTmp = pDlg->GetBusinessCardStr();
            aTmp += String::CreateFromInt32( ++nBCTitleNo );
        }
        xDocSh->SetTitle( aTmp );

        pFrame->GetFrame()->Appear();

        // Shell ermitteln
        SwWrtShell *pSh = pView->GetWrtShellPtr();
        ASSERT( pSh, "missing WrtShell" );

        {   // block for locks the dispatcher!!

            SwWait aWait( (SwDocShell&)*xDocSh, sal_True );

            SET_CURR_SHELL(pSh);
            pSh->DoUndo( sal_False );
            pSh->StartAllAction();

            pSh->SetNewDoc();       // Performanceprobleme vermeiden

            SwPageDesc aDesc = pSh->GetPageDesc( 0 );
            SwFrmFmt&  rFmt  = aDesc.GetMaster();

            // Raender
            SvxLRSpaceItem aLRMargin;
            SvxULSpaceItem aULMargin;
            aLRMargin.SetLeft ((sal_uInt16) rItem.lLeft );
            aULMargin.SetUpper((sal_uInt16) rItem.lUpper);
            aLRMargin.SetRight(0);
            aULMargin.SetLower(0);
            rFmt.SetAttr(aLRMargin);
            rFmt.SetAttr(aULMargin);

            // Kopf- und Fusszeilen
            rFmt.SetAttr(SwFmtHeader(sal_Bool(sal_False)));
            aDesc.ChgHeaderShare(sal_False);
            rFmt.SetAttr(SwFmtFooter(sal_Bool(sal_False)));
            aDesc.ChgFooterShare(sal_False);


            aDesc.SetUseOn(PD_ALL);             // Seitennumerierung

            // Einstellen der Seitengroesse
            rFmt.SetAttr(SwFmtFrmSize(ATT_FIX_SIZE,
                                        rItem.lLeft  + rItem.nCols * rItem.lHDist,
                                        rItem.lUpper + rItem.nRows * rItem.lVDist));

            // Numerierungsart
            SwNumType aType;
            aType.eType = SVX_NUM_NUMBER_NONE;
            aDesc.SetNumType( aType );

            // Folgevorlage
            const SwPageDesc &rFollow = pSh->GetPageDesc( pSh->GetCurPageDesc() );
            aDesc.SetFollow( &rFollow );

            pPrt = pSh->GetPrt( sal_True );
            SvxPaperBinItem aItem;
            aItem.SetValue((sal_Int8)pPrt->GetPaperBin());
            rFmt.SetAttr(aItem);

            aDesc.SetLandscape(ORIENTATION_LANDSCAPE == pPrt->GetOrientation());

            pSh->ChgPageDesc( 0, aDesc );

            // Rahmen einfuegen
            SwFldMgr*        pFldMgr = new SwFldMgr;
            pFldMgr->SetEvalExpFlds(sal_False);

            //fix(24446): Damit der Text der Ettiketten nicht im unbedruckbaren
            //Bereich landet stellen wir entsprechende Raender ein. Um das Handling
            //so Optimal wie moeglich zu halten stellen wir zunaechst an der
            //aktuellen Absatzvorlage keinen Rand als hartes Attribut ein (Damit die
            //Formatierung wg. der Zeichengeb. Rahmen passt. Dann stellen wir die
            //Standarabsatzvorlage anhand des unbedruckbaren Bereiches ein.
            const long nMin = pPrt->GetPageOffset().X() - rItem.lLeft;
            if ( nMin > 0 )
            {
                SvxLRSpaceItem aLR;
                pSh->SetAttr( aLR );
                SwFmt *pStandard = pSh->GetTxtCollFromPool( RES_POOLCOLL_STANDARD );
                aLR.SetLeft ( sal_uInt16(nMin) );
                aLR.SetRight( sal_uInt16(nMin) );
                pStandard->SetAttr( aLR );
            }

            // Rahmenvorlage vorbereiten
            SwFrmFmt* pFmt = pSh->GetFrmFmtFromPool( RES_POOLFRM_LABEL );
            SwFmtFrmSize aFrmSize(  ATT_FIX_SIZE,
                                    rItem.lHDist - (rItem.lHDist-rItem.lWidth),
                                    rItem.lVDist - (rItem.lVDist-rItem.lHeight));
            pFmt->SetAttr(aFrmSize);

            SvxLRSpaceItem aFrmLRSpace( 0, (sal_uInt16)(rItem.lHDist - rItem.lWidth),
                                        0, 0,
                                        RES_LR_SPACE);
            pFmt->SetAttr(aFrmLRSpace);

            SvxULSpaceItem aFrmULSpace( 0, (sal_uInt16)(rItem.lVDist - rItem.lHeight),
                                        RES_UL_SPACE);
            pFmt->SetAttr(aFrmULSpace);

            const SwFrmFmt *pFirstFlyFmt = 0;
            if ( rItem.bPage )
            {
                SwFmtVertOrient aFrmVertOrient( pFmt->GetVertOrient() );
                aFrmVertOrient.SetVertOrient( VERT_TOP );
                pFmt->SetAttr(aFrmVertOrient);

                for ( sal_uInt16 i = 0; i < rItem.nRows; ++i )
                {
                    for ( sal_uInt16 j = 0; j < rItem.nCols; ++j )
                    {
                        pSh->Push();
                        const SwFrmFmt *pTmp =
                                bLabel ?
                                lcl_InsertLabText( *pSh, rItem, *pFmt, *pFldMgr, j, i,
                                    i == rItem.nRows - 1 && j == rItem.nCols - 1,
                                    sal_True ) :
                                lcl_InsertBCText(*pSh, rItem, *pFmt, j, i, sal_True);
                        if (!(i|j))
                        {
                            pFirstFlyFmt = pTmp;

                            if (rItem.bSynchron)
                            {
                                // if there is no content in the fly then
                                // dont leave the fly!!!
                                pSh->Push();
                                pSh->SttDoc();
                                sal_Bool bInFly = 0 != pSh->WizzardGetFly();
                                pSh->Pop( bInFly );

                                if( bInFly )
                                    pSh->EndDoc(sal_True);  // select all content
                                                        // in the fly
                                else
                                    pSh->SetMark();     // set only the mark

                                SwSection aSect( CONTENT_SECTION,
                                                String::CreateFromAscii(MASTER_LABEL));
                                pSh->InsertSection(aSect);
                            }
                        }
                        else if (rItem.bSynchron)
                        {
                            SwSection aSect(FILE_LINK_SECTION, pSh->GetUniqueSectionName());
                            String sLinkName(cTokenSeperator);
                            sLinkName += cTokenSeperator;
                            sLinkName += String::CreateFromAscii(MASTER_LABEL);
                            aSect.SetLinkFileName(sLinkName);
                            aSect.SetProtect();
                            pSh->Insert(aDotStr);   // Dummytext zum Zuweisen der Section
                            pSh->SttDoc();
                            pSh->EndDoc(sal_True);  // Alles im Rahmen selektieren
                            pSh->InsertSection(aSect);
                        }
                        pSh->Pop( sal_False );
                    }
                    if ( i + 1 != rItem.nRows )
                        pSh->SplitNode(); // Kleine Optimierung
                }
            }
            else
            {
                pFirstFlyFmt = bLabel ?
                    lcl_InsertLabText( *pSh, rItem, *pFmt, *pFldMgr,
                            rItem.nCol - 1, rItem.nRow - 1, sal_True, sal_False ) :
                    lcl_InsertBCText(*pSh, rItem, *pFmt, rItem.nCol - 1, rItem.nRow - 1, sal_False);
            }

            //fill the user fields
            if(!bLabel)
            {
                uno::Reference< frame::XModel >  xModel = pSh->GetView().GetDocShell()->GetBaseModel();
                SwLabDlg::UpdateFieldInformation(xModel, rItem);
            }

            pFldMgr->SetEvalExpFlds(sal_True);
            pFldMgr->EvalExpFlds(pSh);

            delete pFldMgr;

            pSh->GotoFly(pFirstFlyFmt->GetName(), FLYCNTTYPE_ALL, sal_False);

            pSh->EndAllAction();
            pSh->DoUndo( sal_True );
            pSh->SetLabelDoc(rItem.bSynchron);
        }

        if( rItem.aWriting.indexOf( '<' ) >= 0 )
        {
            // Datenbankbrowser mit zuletzt verwendeter Datenbank oeffnen
            ShowDBObj( *pSh, pSh->GetDBName() );
        }

        if( rItem.bSynchron )
        {
            SfxDispatcher* pDisp = pFrame->GetDispatcher();
            ASSERT(pDisp, "Heute kein Dispatcher am Frame?");
            pDisp->Execute(FN_SYNC_LABELS, SFX_CALLMODE_ASYNCHRON);
        }
    }
    delete pDlg;

    if( pNewDBMgr )
        delete pNewDBMgr;
}


