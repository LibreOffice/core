/*************************************************************************
 *
 *  $RCSfile: wrtsh1.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-25 12:06:30 $
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
#include <math.h>   // prevent conflict between exception and std::exception
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SOT_FACTORY_HXX
#include <sot/factory.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif
#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _INSDLG_HXX //autogen
#include <so3/insdlg.hxx>
#endif
#ifndef _FRAMEOBJ_HXX //autogen
#include <sfx2/frameobj.hxx>
#endif
#ifndef _EHDL_HXX //autogen
#include <svtools/ehdl.hxx>
#endif
#ifndef _IPENV_HXX //autogen
#include <so3/ipenv.hxx>
#endif
#ifndef _SOERR_HXX //autogen
#include <so3/soerr.hxx>
#endif
#ifndef _CACHESTR_HXX //autogen
#include <tools/cachestr.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef SMDLL0_HXX //autogen
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
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SBAITEMS_HRC //autogen
#include <offmgr/sbaitems.hrc>
#endif
#ifndef _SV_GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX
#include <svx/impgrf.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif

#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _BASESH_HXX
#include <basesh.hxx>
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
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _FRMMGR_HXX
#include <frmmgr.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _UINUMS_HXX
#include <uinums.hxx>  // fuer Anwenden einer
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>  // fuer Undo-Ids
#endif
#ifndef _SWCLI_HXX
#include <swcli.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _CAPTION_HXX
#include <caption.hxx>
#endif
#ifndef _VISCRS_HXX //autogen wg. SwSelPaintRects
#include <viscrs.hxx>
#endif

#ifndef _SWERROR_H
#include <swerror.h>
#endif
#ifndef _WRTSH_HRC
#include <wrtsh.hrc>
#endif

#define COMMON_INI_LIST \
        rView(rShell),\
        ePageMove(MV_NO),\
        pCrsrStack(0),  \
        fnLeaveSelect(&SwWrtShell::SttLeaveSelect),\
        fnDrag(&SwWrtShell::BeginDrag),\
        fnEndDrag(&SwWrtShell::EndDrag),\
        fnSetCrsr(&SwWrtShell::SetCrsr),\
        fnKillSel(&SwWrtShell::Ignore),\
        bDestOnStack(FALSE),\
        pModeStack(0)

#define BITFLD_INI_LIST \
        bClearMark = \
        bIns = TRUE;\
        bAddMode = \
        bExtMode = \
        bInSelect = \
        bCopy = \
        bLayoutMode = \
        bNoEdit = \
        bSelWrd = \
        bSelLn = \
        bIsInClickToEdit = FALSE;


void SwWrtShell::NoEdit(BOOL bHideCrsr)
{
    if(bHideCrsr)
        HideCrsr();
    bNoEdit = TRUE;
}



void SwWrtShell::Edit()
{
    if (CanInsert())
    {
        ShowCrsr();
        bNoEdit = FALSE;
    }
}



BOOL SwWrtShell::IsEndWrd()
{
    MV_KONTEXT(this);
    if(IsEndPara() && !IsSttPara())
        return TRUE;

    return IsEndWord();
}


/*------------------------------------------------------------------------
 Beschreibung:  Abfrage, ob Einfuegen moeglich ist; gfs. Beep
------------------------------------------------------------------------*/



BOOL SwWrtShell::_CanInsert()
{
    if(!CanInsert())
    {
        Sound::Beep();
        return FALSE;
    }
    return TRUE;
}
/*------------------------------------------------------------------------
 Beschreibung:  String einfuegen
------------------------------------------------------------------------*/

void SwWrtShell::InsertByWord( const String & rStr)
{
    if( rStr.Len() )
    {
        BOOL bDelim = WordSelection::IsNormalChar( rStr.GetChar(0) );
        xub_StrLen nPos = 0, nStt = 0;
        for( ; nPos < rStr.Len(); nPos++ )
           {
            BOOL bTmpDelim = WordSelection::IsNormalChar( rStr.GetChar( nPos ) );
            if( bTmpDelim != bDelim )
            {
                Insert( rStr.Copy( nStt, nPos - nStt ));
                nStt = nPos;
            }
        }
        if( nStt != nPos )
            Insert( rStr.Copy( nStt, nPos - nStt ));
    }
}


void SwWrtShell::Insert( const String &rStr )
{
    ResetCursorStack();
    if( !_CanInsert() )
        return;

    BOOL bStarted = FALSE, bHasSel = HasSelection(),
        bCallIns = bIns /*|| bHasSel*/;

    // Notify abschalten
    SwChgLinkFlag *pChgFlg = bCallIns ? new SwChgLinkFlag( *this ) : 0;

    if( bHasSel )
    {
            // nur hier klammern, da das normale Insert schon an der
            // Editshell geklammert ist
        StartAllAction();
        StartUndo(UNDO_INSERT);
        bStarted = TRUE;
        DelRight();
    }
/*
JP 21.01.98: Ueberschreiben ueberschreibt nur die Selektion, nicht das
            naechste Zeichen.
    if( bHasSel && !bIns && 1 < rStr.Len() )
    {
        // falls mehrere Zeichen anstehen, nur das erste einfuegen,
        // der Rest muss dann aber Ueberschrieben werden.
        SwEditShell::Insert( rStr.GetChar( 0 ) );
        SwEditShell::Overwrite( rStr.Copy( 1 ) );
    }
    else
*/
        bCallIns ? SwEditShell::Insert( rStr ) : SwEditShell::Overwrite( rStr );


    if( bStarted )
    {
        EndAllAction();
        EndUndo(UNDO_INSERT);
    }
    delete pChgFlg;
}

/* Begrenzung auf maximale Hoehe geht nicht, da die maximale Hoehe
 * des aktuellen Frames nicht erfragt werden kann. */



void SwWrtShell::Insert( const String &rPath, const String &rFilter,
                         const Graphic &rGrf, SwFlyFrmAttrMgr *pFrmMgr,
                         BOOL bRule )
{
    ResetCursorStack();
    if ( !_CanInsert() )
        return;

    StartAllAction();
    StartUndo(UNDO_INSERT);

    if ( HasSelection() )
        DelRight();
        // eingefuegte Grafik in eigenen Absatz, falls am Ende
        // eines nichtleeren Absatzes
    if ( IsEndPara() && !IsSttPara() )
        SwFEShell::SplitNode();

    EnterSelFrmMode();

    BOOL bSetGrfSize = TRUE;
    BOOL bOwnMgr     = FALSE;

    if ( !pFrmMgr )
    {
        bOwnMgr = TRUE;
        pFrmMgr = new SwFlyFrmAttrMgr( TRUE, this, FRMMGR_TYPE_GRF );

        // VORSICHT
        // GetAttrSet nimmt einen Abgleich vor
        // Beim Einfuegen ist eine SwFrmSize vorhanden wegen der
        // DEF-Rahmengroesse
        // Diese muss fuer die optimale Groesse explizit entfernt werden
        pFrmMgr->DelAttr(RES_FRM_SIZE);
    }
    else
    {
        Size aSz( pFrmMgr->GetSize() );
        if ( !aSz.Width() || !aSz.Height() )
        {
            aSz.Width() = aSz.Height() = 567;
            pFrmMgr->SetSize( aSz );
        }
        else if ( aSz.Width() != DFLT_WIDTH && aSz.Height() != DFLT_HEIGHT )
            bSetGrfSize = FALSE;

        pFrmMgr->SetSizeType(ATT_FIX_SIZE);

    }

    // Einfuegen der Grafik
    SwFEShell::Insert(rPath, rFilter, &rGrf, &pFrmMgr->GetAttrSet());
    if ( bOwnMgr )
        pFrmMgr->UpdateAttrMgr();

    if( bSetGrfSize && !bRule )
    {
        Size aGrfSize, aBound = GetGraphicDefaultSize();
        GetGrfSize( aGrfSize );

        //Die GrafikSize noch um die Randattribute vergroessern, denn die
        //Zaehlen beim Rahmen mit.
        aGrfSize.Width() += pFrmMgr->CalcWidthBorder();
        aGrfSize.Height()+= pFrmMgr->CalcHeightBorder();

        const BigInt aTempWidth( aGrfSize.Width() );
        const BigInt aTempHeight( aGrfSize.Height());

        // ggf. Breite anpassen, Hoehe dann proportional verkleinern
        if( aGrfSize.Width() > aBound.Width() )
        {
            aGrfSize.Width()  = aBound.Width();
            aGrfSize.Height() = ((BigInt)aBound.Width()) * aTempHeight / aTempWidth;
        }
        // ggf. Hoehe anpassen, Breite dann proportional verkleinern
        if( aGrfSize.Height() > aBound.Height() )
        {
            aGrfSize.Height() = aBound.Height();
            aGrfSize.Width() =  ((BigInt)aBound.Height()) * aTempWidth / aTempHeight;
        }
        pFrmMgr->SetSize( aGrfSize );
        pFrmMgr->UpdateFlyFrm();
    }
    if ( bOwnMgr )
        delete pFrmMgr;

    EndUndo(UNDO_INSERT);
    EndAllAction();
}


/*------------------------------------------------------------------------
   Beschreibung: Fuegt ein OLE-Objekt in die CORE ein.
                 Wenn kein Object uebergeben wird, so wird eins erzeugt.
------------------------------------------------------------------------*/


void SwWrtShell::Insert( SvInPlaceObjectRef *pRef, SvGlobalName *pName, BOOL bActivate, USHORT nSlotId)
{
    ResetCursorStack();
    if( !_CanInsert() )
    {
        delete pRef;
        return;
    }

    if( !pRef )
    {
        //Wir bauen uns ein neues OLE-Objekt, entweder per Dialog oder direkt
        //ueber den Namen.
        SvInPlaceObjectRef xIPObj;
        BOOL bDoVerb = TRUE;
        if ( pName )
        {
            SvStorageRef aStor = new SvStorage( aEmptyStr );
            xIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                                                                *pName,aStor );
        }
        else
        {
            SvStorageRef aStor = new SvStorage( aEmptyStr, STREAM_STD_READWRITE);

            switch (nSlotId)
            {
                case SID_INSERT_OBJECT:
                {
                    SvInsertOleObjectDialog aDlg;
                    aDlg.SetHelpId(nSlotId);

                    //Wir wollen uns nicht selbst servieren.
                    SvObjectServerList aServerList;
                    aDlg.FillObjectServerList( &aServerList );
                    aServerList.Remove( *SwDocShell::ClassFactory() );

                    xIPObj = aDlg.Execute( GetWin(), aStor, &aServerList);
                    bDoVerb = aDlg.IsCreateNew();
                }
                break;

                case SID_INSERT_PLUGIN:
                {
                    SvInsertPlugInDialog aDlg;
                    aDlg.SetHelpId(nSlotId);

                    xIPObj = aDlg.Execute( GetWin(), aStor);
                    bDoVerb = FALSE;
                }
                break;

                case SID_INSERT_APPLET:
                {
                    SvInsertAppletDialog aDlg;
                    aDlg.SetHelpId(nSlotId);

                    xIPObj = aDlg.Execute( GetWin(), aStor);
                    bDoVerb = FALSE;
                }
                break;

                case SID_INSERT_FLOATINGFRAME:
                {
                    SfxInsertFloatingFrameDialog aDlg( GetWin() );
                    xIPObj = aDlg.Execute( aStor );
                    bDoVerb = FALSE;
                }

                default:
                    break;
            }
        }

        if ( xIPObj.Is() )
        {
            if( SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE &
                xIPObj->GetMiscStatus() && GetPrt() )
                xIPObj->OnDocumentPrinterChanged( GetPrt() );

            if( InsertOle( xIPObj ) && bActivate && bDoVerb )
            {
                ASSERT( !xIPObj->IsLink(), "Link nicht aus Datei?" );
                SfxInPlaceClientRef xCli = GetView().FindIPClient( xIPObj,
                                                    &GetView().GetEditWin());
                if ( !xCli.Is() )
                    xCli = new SwOleClient( &GetView(), &GetView().GetEditWin());
                ErrCode nErr = xIPObj->DoConnect( xCli );
                ErrorHandler::HandleError( nErr );
                if ( !ERRCODE_TOERROR(nErr) )
                {
                    SvEmbeddedObjectRef xObj = &xIPObj;
                    CalcAndSetScale( xObj );
                    //#50270# Error brauchen wir nicht handeln, das erledigt das
                    //DoVerb in der SfxViewShell
                    nErr = GetView().SfxViewShell::DoVerb( xCli, SVVERB_SHOW );
                    if ( !ERRCODE_TOERROR( nErr ) )
                        xIPObj->SetDocumentName( GetView().GetDocShell()->GetTitle() );
                }
            }
        }
    }
    else
    {
        if( HasSelection() )
            DelRight();
        InsertOle( *pRef );
    }
}

/*------------------------------------------------------------------------
 Beschreibung:   Object in die Core einfuegen.
                 Vom ClipBoard oder Insert
------------------------------------------------------------------------*/

BOOL SwWrtShell::InsertOle( SvInPlaceObjectRef aRef )
{
    if ( aRef.Is() )
    {
        ResetCursorStack();
        StartAllAction();
        StartUndo(UNDO_INSERT);

        //Some differences between StarMath and any other objects:
        //1. Selections should be deleted. For StarMath the Text should be
        //   passed to the Object
        //2. If the cursor is at the end of an non empty paragraph a paragraph
        //   break should be insertet. StarMath objects are character bound and
        //   no break should be inserted.
        //3. If an selektion is passed to a StarMath object, this object should
        //   not be activated. FALSE should be returned then.
        BOOL bStarMath,
             bActivate = TRUE;

        SvGlobalName aCLSID;
        ULONG lDummy;
        String aDummy;
        // determine source CLSID
        aRef->SvPseudoObject::FillClass( &aCLSID, &lDummy, &aDummy, &aDummy, &aDummy);
        bStarMath = SmModuleDummy::HasID( *aRef->GetSvFactory() );
        if ( !bStarMath && aRef->IsLink() && SmModuleDummy::HasID( aCLSID ) )
        {
            //StarMath was the server which did the last recent work on
            //this object.
            bStarMath = TRUE;
        }

        if ( IsSelection() )
        {
            if ( bStarMath )
            {
                String aMathData;
                GetSelectedText( aMathData, GETSELTXT_PARABRK_TO_ONLYCR );
                if ( aMathData.Len() )
                {
                    SvData aData( FORMAT_STRING );
                    aData.SetData( aMathData );
                    if ( aRef->SetData( &aData ) )
                    {
                        bActivate = FALSE;
                        //StarMath size depends on the Printer, which is
                        //passed here direct for avoiding time consuming
                        //connections between StarWriter and StarMath
                        aRef->OnDocumentPrinterChanged( GetPrt() );
                    }
                    else
                        bActivate = TRUE;
                }
            }
            DelRight();
        }

        if ( !bStarMath )
            SwFEShell::SplitNode( FALSE, FALSE );

        EnterSelFrmMode();

        SwFlyFrmAttrMgr aFrmMgr( TRUE, this, FRMMGR_TYPE_OLE );
        aFrmMgr.SetSizeType(ATT_FIX_SIZE);

        SwRect aBound;
        CalcBoundRect( aBound, aFrmMgr.GetAnchor() );

        //The Size should be suggested by the OLE server
        MapMode aRefMap( aRef->GetMapUnit() );
        Size aSz( aRef->GetVisArea().GetSize() );
        if ( !aSz.Width() || !aSz.Height() )
        {
            aSz.Width() = aSz.Height() = 5000;
            aSz = OutputDevice::LogicToLogic
                                    ( aSz, MapMode( MAP_100TH_MM ), aRefMap );
        }
        MapMode aMyMap( MAP_TWIP );
        aSz = OutputDevice::LogicToLogic( aSz, aRefMap, aMyMap );
        //Object size can be limited
        if ( aSz.Width() > aBound.Width() )
        {
            //Immer proportional begrenzen.
            aSz.Height() = aSz.Height() * aBound.Width() / aSz.Width();
            aSz.Width() = aBound.Width();
        }
        aFrmMgr.SetSize( aSz );
        SwFEShell::Insert( &aRef, &aFrmMgr.GetAttrSet() );

        EndAllAction();
        GetView().AutoCaption(OLE_CAP, &aCLSID);
        EndUndo(UNDO_INSERT);

        return bActivate;
    }
    return FALSE;
}

/*------------------------------------------------------------------------
    Beschreibung: Das aktuelle selektierte OLE-Objekt wird mit dem
                  Verb in den Server geladen.
------------------------------------------------------------------------*/



void SwWrtShell::LaunchOLEObj( long nVerb )
{
    if ( GetCntType() == CNT_OLE &&
         !GetView().GetDocShell()->GetProtocol().IsInPlaceActive() )
    {
        SvInPlaceObjectRef xRef = GetOLEObj();
        ASSERT( xRef.Is(), "OLE not found" );
        SfxInPlaceClientRef xCli;
        if ( !xRef->IsLink() )
        {
            //  Link fuer Daten-Highlighting im Chart zuruecksetzen
            if( SFX_APP()->HasFeature( SFX_FEATURE_SCHART ) )
            {
                SvGlobalName aObjClsId( *xRef->GetSvFactory() );
                SchMemChart* pMemChart;
                if( SchModuleDummy::HasID( aObjClsId ) &&
                    0 != (pMemChart = SchDLL::GetChartData( xRef ) ))
                {
                    pMemChart->SetSelectionHdl( LINK( this, SwWrtShell,
                                                ChartSelectionHdl ) );
                    //#60043# Damit die DataBrowseBox nicht erscheint wird das
                    //Chart auf Readonly gesetzt wenn es eine Verbindung
                    //zu einer Tabelle hat.
                    if ( GetChartName( xRef ).Len() )
                        pMemChart->SetReadOnly( TRUE );
                }
            }

            xCli = GetView().FindIPClient( xRef, &GetView().GetEditWin() );
            if ( !xCli.Is() )
                xCli = new SwOleClient( &GetView(), &GetView().GetEditWin() );

            ((SwOleClient*)&xCli)->SetInDoVerb( TRUE );

            xRef->DoConnect( xCli );
            SvEmbeddedObjectRef xObj = &xRef;
            CalcAndSetScale( xObj );
            //#50270# Error brauchen wir nicht handeln, das erledigt das
            //DoVerb in der SfxViewShell
            GetView().SfxViewShell::DoVerb( xCli, nVerb );

            ((SwOleClient*)&xCli)->SetInDoVerb( FALSE );
            CalcAndSetScale( xObj );
        }
        else
        {
            ErrCode nErr;
            SfxErrorContext aEc( ERRCTX_SO_DOVERB, &GetView().GetEditWin(),
                                 RID_SO_ERRCTX );
            nErr = xRef->DoVerb( nVerb );
            if ( nErr )
                ErrorHandler::HandleError( nErr );
        }

    }
}

BOOL SwWrtShell::FinishOLEObj()                     // Server wird beendet
{
    SfxInPlaceClient* pIPClient = GetView().GetIPClient();
    BOOL bRet = pIPClient && pIPClient->IsInPlaceActive();
    if( bRet )
    {
        //  Link fuer Daten-Highlighting im Chart zuruecksetzen
        if( SFX_APP()->HasFeature(SFX_FEATURE_SCHART) )
        {
            SvInPlaceObject* pObj = pIPClient->GetIPObj();
            SvGlobalName aObjClsId( *pObj->GetSvFactory() );
            SchMemChart* pMemChart;
            if( SchModuleDummy::HasID( aObjClsId ) &&
                0 != (pMemChart = SchDLL::GetChartData( pObj ) ))
            {
                pMemChart->SetSelectionHdl( Link() );

//ggfs. auch die Selektion restaurieren
                LockView( TRUE );   //Scrollen im EndAction verhindern
                ClearMark();
                LockView( FALSE );
            }
        }

        //InPlace beenden.
        pIPClient->GetProtocol().Reset2Open();
        SFX_APP()->SetViewFrame( GetView().GetViewFrame() );
    }
    return bRet;
}


void SwWrtShell::CalcAndSetScale( SvEmbeddedObjectRef xObj,
                             const SwRect *pFlyPrtRect,
                             const SwRect *pFlyFrmRect )
{
    //Einstellen der Skalierung am Client. Diese ergibt sich aus der Differenz
    //zwischen der VisArea des Objektes und der ObjArea.
    ASSERT( xObj.Is(), "ObjectRef not  valid" );

    SfxInPlaceClientRef xCli = GetView().FindIPClient( xObj, &GetView().GetEditWin() );
    if ( !xCli.Is() || !xCli->GetEnv() )
    {
        //Das kann ja wohl nur ein nicht aktives Objekt sein. Diese bekommen
        //auf Wunsch die neue Groesse als VisArea gesetzt (StarChart)
        if( SVOBJ_MISCSTATUS_SERVERRESIZE & xObj->GetMiscStatus() )
        {
            SwRect aRect( pFlyPrtRect ? *pFlyPrtRect
                        : GetAnyCurRect( RECT_FLY_PRT_EMBEDDED, 0, &xObj ));
            if( !aRect.IsEmpty() )
                xObj->SetVisArea( OutputDevice::LogicToLogic(
                            aRect.SVRect(), MAP_TWIP, xObj->GetMapUnit() ));
            return;
        }
        if ( SVOBJ_MISCSTATUS_ALWAYSACTIVATE & xObj->GetMiscStatus() ||
             SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE & xObj->GetMiscStatus() )
        {
            xCli = new SwOleClient( &GetView(), &GetView().GetEditWin() );
        }
        else
            return;
    }

    Size aVisArea( xObj->GetVisArea().GetSize() );
    BOOL bSetScale100 = TRUE;
    SvContainerEnvironment *pEnv = xCli->GetEnv();

    // solange keine vernuenftige Size vom Object kommt, kann nichts
    // skaliert werden
    if( aVisArea.Width() && aVisArea.Height() )
    {
        const MapMode aTmp( MAP_TWIP );
        aVisArea = OutputDevice::LogicToLogic( aVisArea, xObj->GetMapUnit(), aTmp);
        Size aObjArea;
        if ( pFlyPrtRect )
            aObjArea = pFlyPrtRect->SSize();
        else
            aObjArea = GetAnyCurRect( RECT_FLY_PRT_EMBEDDED, 0, &xObj ).SSize();

        // differ the aObjArea and aVisArea by 1 Pixel then set new VisArea
        long nX, nY;
        SwSelPaintRects::Get1PixelInLogic( *this, &nX, &nY );
        if( !( aVisArea.Width() - nX <= aObjArea.Width() &&
               aVisArea.Width() + nX >= aObjArea.Width() &&
               aVisArea.Height()- nY <= aObjArea.Height()&&
               aVisArea.Height()+ nY >= aObjArea.Height() ))
        {
            if( SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE & xObj->GetMiscStatus() )
            {
                //This type of objects should never be resized.
                //If this request comes from the Writer core (inaktive Object
                //ist resized), the Object should be resized too.
                //If this request comes from the Object itself, the Frame
                //in the Writer core should be resized.
                if ( pFlyPrtRect )      //Request from core?
                {
                    xObj->SetVisArea( OutputDevice::LogicToLogic(
                        pFlyPrtRect->SVRect(), MAP_TWIP, xObj->GetMapUnit() ));
                }
                else
                {
                    SwRect aTmp( Point( LONG_MIN, LONG_MIN ), aVisArea );
                    RequestObjectResize( aTmp, xObj );
                }
                //Der Rest erledigt sich, weil wir eh wiederkommen sollten, evtl.
                //sogar rekursiv.
                return;
            }
            else
            {
                const Fraction aScaleWidth ( aObjArea.Width(),  aVisArea.Width() );
                const Fraction aScaleHeight( aObjArea.Height(), aVisArea.Height());
                pEnv->SetSizeScale( aScaleWidth, aScaleHeight );
                bSetScale100 = FALSE;
            }
        }
    }

    if( bSetScale100 )
    {
        const Fraction aScale( 1, 1 );
        pEnv->SetSizeScale( aScale, aScale );
    }

    //Jetzt ist auch der guenstige Zeitpunkt die ObjArea einzustellen.
    //Die Scalierung muss beruecksichtigt werden.
    SwRect aArea;
    if ( pFlyPrtRect )
    {
        aArea = *pFlyPrtRect;
        aArea += pFlyFrmRect->Pos();
    }
    else
    {
        aArea = GetAnyCurRect( RECT_FLY_PRT_EMBEDDED, 0, &xObj );
        aArea.Pos() += GetAnyCurRect( RECT_FLY_EMBEDDED, 0, &xObj ).Pos();
    }
    aArea.Width ( Fraction( aArea.Width()  ) / pEnv->GetScaleWidth() );
    aArea.Height( Fraction( aArea.Height() ) / pEnv->GetScaleHeight());
    pEnv->SetObjArea( aArea.SVRect() );

    if ( SVOBJ_MISCSTATUS_ALWAYSACTIVATE & xObj->GetMiscStatus() )
    {
        xObj->DoConnect( xCli );
        xObj->DoVerb();
    }
}



void SwWrtShell::ConnectObj( SvInPlaceObjectRef xIPObj, const SwRect &rPrt,
                            const SwRect &rFrm )
{
    if ( !xIPObj->IsLink() )
    {
        SfxInPlaceClientRef xCli = GetView().FindIPClient( xIPObj,
                                                           &GetView().GetEditWin());
        if ( !xCli.Is() )
            xCli = new SwOleClient( &GetView(), &GetView().GetEditWin() );
        xIPObj->DoConnect( xCli );
        SvEmbeddedObjectRef xObj = &xIPObj;
        CalcAndSetScale( xObj, &rPrt, &rFrm );
    }
}

IMPL_LINK( SwWrtShell, ChartSelectionHdl, ChartSelectionInfo *, pInfo )
{
    long nRet = 0;

#ifdef USED
// JP 03.11.98: mit der Selektion kann es nicht gehen, da dann der Cursor
//              immer sichtbar gemacht wird. Das fuehrt dann aber zu
//              unbeabsichtigten scrollen. Ausserdem sind 2 Selektionen
//              vorhanden - TabellenSelektion und die OLE-Rahmenselektion.

    if( pInfo )
    {
        if( CHART_SEL_QUERYSUPPORT & pInfo->nSelection )
            nRet = CHART_SEL_NONE | CHART_SEL_ALL | CHART_SEL_ROW |
                    CHART_SEL_COL | CHART_SEL_POINT;
        else
        {
            // dann suche mal die Tabelle zu diesem StarChart-Object
            SfxInPlaceClient* pIPClient = GetView().GetIPClient();
            SvInPlaceObject* pObj = pIPClient &&
                                    pIPClient->IsInPlaceActive()
                                    ? pIPClient->GetIPObj() : 0;
            String sTable;
            if( pObj )
                sTable = GetChartName( pObj );

            if( sTable.Len() )
            {
                LockView( TRUE );   //Scrollen im EndAction verhindern
                StartAction();

                ClearMark();
                if( GotoTable( sTable ) )
                {
                    // !!!!!!!!!!!!!!!
                    //      im nSelection sind Flags gesetzt, koennen also
                    //      auch gemischt auftauchen, darum das nICol, nIRow
                    // !!!!!!!!!!!!!!!!
                    USHORT nIRow = pInfo->nRow, nICol = pInfo->nCol,
                            nRow = 0, nCol = 0;

                    SchMemChart* pMemChart = SchDLL::GetChartData( pObj );
                    if( pMemChart && 2 == pMemChart->SomeData2().Len() )
                    {
                        if( '1' == pMemChart->SomeData2().GetChar( 0 ) )
                            ++nIRow;
                        if( '1' == pMemChart->SomeData2().GetChar( 1 ))
                            ++nICol;
                    }

// ist das default  if( (CHART_SEL_NONE | CHART_SEL_ALL) & pInfo->nSelection )
                    if( CHART_SEL_ROW & pInfo->nSelection )
                        nRow = nIRow, nCol = 0;
                    if( CHART_SEL_COL & pInfo->nSelection )
                        nCol = nICol, nRow = 0;
                    if( CHART_SEL_POINT & pInfo->nSelection )
                        nCol = nICol, nRow = nIRow;

                    if( GotoTblBox( SwTable::GetBoxName( nRow, nCol ) ) )
                    {
                        nRet = pInfo->nSelection;
                        if( ( CHART_SEL_ROW & pInfo->nSelection && !SelTblRow() ) ||
                            ( CHART_SEL_COL & pInfo->nSelection && !SelTblCol() ) ||
                            ( CHART_SEL_ALL & pInfo->nSelection &&
                                ( SetMark(),
                                    !MoveTable( fnTableCurr, fnTableEnd ))) )
                                nRet = 0;
                        else if( CHART_SEL_POINT & pInfo->nSelection )
                        {
                            // Selektion der einen Box
                            SetMark();
                            if( GoPrevCell() )
                                GoNextCell( FALSE );
                            else if( GoNextCell( FALSE ) )
                                GoPrevCell();
                            else
                                ClearMark();
                        }
                    }
                }

                EndAction();
                LockView( FALSE );
            }
        }
    }
#endif
    return nRet;
}

/*------------------------------------------------------------------------
 Beschreibung:  Einfuegen harter Seitenumbruch;
                Selektionen werden ueberschrieben
------------------------------------------------------------------------*/



void SwWrtShell::InsertPageBreak(const String *pPageDesc, USHORT nPgNum )
{
    ACT_KONTEXT(this);
    ResetCursorStack();
    if(!_CanInsert())
        return;

    StartUndo(UIUNDO_INSERT_PAGE_BREAK);

    if ( !IsCrsrInTbl() )
    {
        if(HasSelection())
            DelRight();
        SwFEShell::SplitNode();
    }

    const SwPageDesc *pDesc = pPageDesc
                            ? FindPageDescByName( *pPageDesc, TRUE ) : 0;
    if( pDesc )
    {
        SwFmtPageDesc aDesc( pDesc );
        aDesc.SetNumOffset( nPgNum );
        SetAttr( aDesc );
    }
    else
        SetAttr( SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE) );
    EndUndo(UIUNDO_INSERT_PAGE_BREAK);
}
/*------------------------------------------------------------------------
 Beschreibung:  Einfuegen harter Zeilenumbruch;
                Selektionen werden ueberschrieben
------------------------------------------------------------------------*/


void SwWrtShell::InsertLineBreak()
{
    ResetCursorStack();

    if(!_CanInsert())
        return;

    if(HasSelection())
        DelRight();

    SwWrtShell::Insert(String((char)0x0A));
}
/*------------------------------------------------------------------------
 Beschreibung:  Einfuegen harter Spaltenumbruch;
                Selektionen werden ueberschrieben
------------------------------------------------------------------------*/


void SwWrtShell::InsertColumnBreak()
{
    ACT_KONTEXT(this);
    ResetCursorStack();
    if(!_CanInsert())
        return;
    StartUndo(UIUNDO_INSERT_COLUMN_BREAK);

    if ( !IsCrsrInTbl() )
    {
        if(HasSelection())
            DelRight();
        SwFEShell::SplitNode();
    }
    SetAttr(SvxFmtBreakItem(SVX_BREAK_COLUMN_BEFORE));

    EndUndo(UIUNDO_INSERT_COLUMN_BREAK);
}

/*------------------------------------------------------------------------
 Beschreibung:  Einfuegen Fussnote
 Parameter:     rStr -- optionales Fussnotenzeichen
------------------------------------------------------------------------*/


void SwWrtShell::InsertFootnote(const String &rStr, BOOL bEndNote, BOOL bEdit )
{
    ResetCursorStack();
    if(!_CanInsert())
        return;

    if(HasSelection())
        DelRight();

    SwFmtFtn aFootNote( bEndNote );
    if(rStr.Len())
        aFootNote.SetNumStr( rStr );

    SetAttr(aFootNote);

    if ( bEdit )
    {
        // zur Bearbeiung des Fussnotentextes
        Left();
        GotoFtnTxt();
    }
}
/*------------------------------------------------------------------------
 Beschreibung:  SplitNode; hier auch, da
                    - selektierter Inhalt geloescht wird;
                    - der Cursorstack gfs. zurueckgesetzt wird.
------------------------------------------------------------------------*/


void SwWrtShell::SplitNode( BOOL bAutoFmt, BOOL bCheckTableStart )
{
    ResetCursorStack();
    if(!_CanInsert())
        return;

    ACT_KONTEXT(this);

    rView.GetEditWin().FlushInBuffer( this );
    BOOL bHasSel = HasSelection();
    if( bHasSel )
    {
        StartUndo( UNDO_INSERT );
        DelRight();
    }

    SwFEShell::SplitNode( bAutoFmt, bCheckTableStart );
    if( bHasSel )
        EndUndo( UNDO_INSERT );
}

/*------------------------------------------------------------------------
 Beschreibung:  Numerierung anschalten
 Parameter:     Optionale Angabe eines Namens fuer die benannte Liste;
                dieser bezeichnet eine Position, wenn er in eine
                Zahl konvertierbar ist und kleiner ist als nMaxRules.
-------------------------------------------------------------------------*/


// zum Testen der CharFormate an der Numerierung
// extern void SetNumChrFmt( SwWrtShell*, SwNumRules& );

void SwWrtShell::NumOn()
{
    SwNumRule aNumRule( GetUniqueNumRuleName() );

    const SwNumRule* pCurRule = GetCurNumRule();
    if( !pCurRule )
    {
        // Zeichenvorlage an die Numerierung haengen
        SwCharFmt* pChrFmt = GetCharFmtFromPool( RES_POOLCHR_NUM_LEVEL );
        SwDocShell* pDocSh = GetView().GetDocShell();
        BOOL bHtml = 0 != PTR_CAST(SwWebDocShell, pDocSh);
        for( BYTE nLvl = 0; nLvl < MAXLEVEL; ++nLvl )
        {
            SwNumFmt aFmt( aNumRule.Get( nLvl ) );
            aFmt.SetCharFmt( pChrFmt );
            if(bHtml && nLvl)
            {
                // 1/2" fuer HTML
                aFmt.SetLSpace(720);
                aFmt.SetAbsLSpace(nLvl * 720);
            }
            aNumRule.Set( nLvl, aFmt );
        }
        SetCurNumRule( aNumRule );
    }
}

/*------------------------------------------------------------------------
 Beschreibung:  Default-Bulletliste erfragen
------------------------------------------------------------------------*/

void SwWrtShell::BulletOn()
{
    SwNumRule aRule( GetUniqueNumRuleName() );

    SwCharFmt* pChrFmt = GetCharFmtFromPool( RES_POOLCHR_BUL_LEVEL );
    const Font* pFnt = &SwNumRule::GetDefBulletFont();

    SwDocShell* pDocSh = GetView().GetDocShell();
    BOOL bHtml = 0 != PTR_CAST(SwWebDocShell, pDocSh);
    for( BYTE n = 0; n < MAXLEVEL; ++n )
    {
        SwNumFmt aFmt( aRule.Get( n ) );
        aFmt.SetBulletFont( pFnt );
        aFmt.SetBulletChar( cBulletChar );
        aFmt.eType = SVX_NUM_CHAR_SPECIAL;
        aFmt.SetCharFmt( pChrFmt );
        if(bHtml && n)
        {
            // 1/2" fuer HTML
            aFmt.SetLSpace(720);
            aFmt.SetAbsLSpace(n * 720);
        }
        aRule.Set( n, aFmt );
    }
    SetCurNumRule( aRule );
}

/*--------------------------------------------------

--------------------------------------------------*/
int SwWrtShell::GetSelectionType() const
{
    // ContentType kann nicht ermittelt werden innerhalb einer
    // Start-/Endactionklammerung.
    // Da es keinen ungueltigen Wert gibt, wird TEXT geliefert.
    // Der Wert ist egal, da in EndAction ohnehin aktualisiert wird.

    if ( BasicActionPend() )
        return IsSelFrmMode() ? SEL_FRM : SEL_TXT;

//  if ( IsTableMode() )
//      return SEL_TBL | SEL_TBL_CELLS;

    SwView &rView = ((SwView&)GetView());
    USHORT nCnt;

    // Rahmen einfuegen ist kein DrawMode
    if ( !rView.GetEditWin().IsFrmAction() &&
            (IsObjSelected() || (rView.IsDrawMode() && !IsFrmSelected()) ))
    {
        if (GetDrawView()->IsTextEdit())
            nCnt = SEL_DRW_TXT;
        else
        {
            if (GetView().IsFormMode()) // Nur Forms selektiert
                nCnt = SEL_DRW_FORM;
            else
                nCnt = SEL_DRW;         // Irgendein Draw-Objekt

            if (rView.IsBezierEditMode())
                nCnt |= SEL_BEZ;
        }

        return nCnt;
    }

    nCnt = GetCntType();

    if ( IsFrmSelected() )
    {
        if (rView.IsDrawMode())
            rView.LeaveDrawCreate();    // Aufraeumen (Bug #45639)
        if ( !(nCnt & (CNT_GRF | CNT_OLE)) )
            return SEL_FRM;
    }

    if ( IsCrsrInTbl() )
        nCnt |= SEL_TBL;

    if ( IsTableMode() )
        nCnt |= (SEL_TBL | SEL_TBL_CELLS);

    if ( GetCurNumRule() )
        nCnt |= SEL_NUM;

    return nCnt;
}

/*------------------------------------------------------------------------
 Beschreibung:  Finden der TextCollection mit dem Name rCollname
 Return:                Pointer auf die Collection oder 0, wenn keine
                                TextCollection mit diesem Namen existiert oder
                                diese eine Defaultvorlage ist.
------------------------------------------------------------------------*/


SwTxtFmtColl *SwWrtShell::GetParaStyle(const String &rCollName, GetStyle eCreate )
{
    SwTxtFmtColl* pColl = FindTxtFmtCollByName( rCollName );
    if( !pColl && GETSTYLE_NOCREATE != eCreate )
    {
        USHORT nId = GetPoolId( rCollName, GET_POOLID_TXTCOLL );
        if( USHRT_MAX != nId || GETSTYLE_CREATEANY == eCreate )
            pColl = GetTxtCollFromPool( nId );
    }
    return pColl;
}
/*------------------------------------------------------------------------
 Beschreibung:  Finden der Zeichenvorlage mit dem Name rCollname
 Return:                Pointer auf die Collection oder 0, wenn keine
                                Zeichenvorlage mit diesem Namen existiert oder
                                diese eine Defaultvorlage oder automatische Vorlage ist.
------------------------------------------------------------------------*/



SwCharFmt *SwWrtShell::GetCharStyle(const String &rFmtName, GetStyle eCreate )
{
    SwCharFmt* pFmt = FindCharFmtByName( rFmtName );
    if( !pFmt && GETSTYLE_NOCREATE != eCreate )
    {
        USHORT nId = GetPoolId( rFmtName, GET_POOLID_CHRFMT );
        if( USHRT_MAX != nId || GETSTYLE_CREATEANY == eCreate )
            pFmt = (SwCharFmt*)GetFmtFromPool( nId );
    }
    return pFmt;
}

/*------------------------------------------------------------------------
 Beschreibung:  Finden des Tabellenformates mit dem Name rFmtname
 Return:                Pointer auf das Format oder 0, wenn kein
                                Rahmenformat mit diesem Namen existiert oder
                                dieses eine Defaultformat oder automatisches Format ist.
------------------------------------------------------------------------*/



SwFrmFmt *SwWrtShell::GetTblStyle(const String &rFmtName)
{
    SwFrmFmt *pFmt = 0;
    for( USHORT i = GetTblFrmFmtCount(); i; )
        if( !( pFmt = &GetTblFrmFmt( --i ) )->IsDefault() &&
            pFmt->GetName() == rFmtName && IsUsed( *pFmt ) )
            return pFmt;
    return 0;
}


/*------------------------------------------------------------------------
 Beschreibung:  Anwenden der Vorlagen
------------------------------------------------------------------------*/



void SwWrtShell::SetPageStyle(const String &rCollName)
{
    if( !SwCrsrShell::HasSelection() && !IsSelFrmMode() && !IsObjSelected() )
    {
        SwPageDesc* pDesc = FindPageDescByName( rCollName, TRUE );
        if( pDesc )
            ChgCurPageDesc( *pDesc );
    }
}

/*------------------------------------------------------------------------
 Beschreibung:  Zugriff Vorlagen
------------------------------------------------------------------------*/



String SwWrtShell::GetCurPageStyle( const BOOL bCalcFrm ) const
{
    return GetPageDesc(GetCurPageDesc( bCalcFrm )).GetName();
}

/*------------------------------------------------------------------------
 Beschreibung:  Aktuelle Vorlage anhand der geltenden Attribute aendern
------------------------------------------------------------------------*/


void SwWrtShell::QuickUpdateStyle()
{
    SwTxtFmtColl *pColl = GetCurTxtFmtColl();

    // Standard kann nicht geaendert werden
    if(pColl && !pColl->IsDefault())
    {
        FillByEx(pColl);
            // Vorlage auch anwenden, um harte Attributierung
            // zu entfernen
        SetTxtFmtColl(pColl);
    }
}


void SwWrtShell::AutoUpdatePara(SwTxtFmtColl* pColl, const SfxItemSet& rStyleSet)
{
    SfxItemSet aCoreSet( GetAttrPool(),
            RES_CHRATR_BEGIN,           RES_CHRATR_END - 1,
            RES_PARATR_BEGIN,           RES_PARATR_END - 1,
            RES_FRMATR_BEGIN,           RES_FRMATR_END - 1,
            SID_ATTR_TABSTOP_POS,       SID_ATTR_TABSTOP_POS,
            SID_ATTR_TABSTOP_DEFAULTS,  SID_ATTR_TABSTOP_DEFAULTS,
            SID_ATTR_TABSTOP_OFFSET,    SID_ATTR_TABSTOP_OFFSET,
            SID_ATTR_BORDER_INNER,      SID_ATTR_BORDER_INNER,
            SID_ATTR_PARA_MODEL,        SID_ATTR_PARA_KEEP,
            SID_ATTR_PARA_PAGENUM,      SID_ATTR_PARA_PAGENUM,
            0   );
    GetAttr( aCoreSet );
    BOOL bReset = FALSE;
    SfxItemIter aParaIter( aCoreSet );
    const SfxPoolItem* pParaItem = aParaIter.FirstItem();
    while( pParaItem )
    {
        if(!IsInvalidItem(pParaItem))
        {
            USHORT nWhich = pParaItem->Which();
            if(SFX_ITEM_SET == aCoreSet.GetItemState(nWhich) &&
               SFX_ITEM_SET == rStyleSet.GetItemState(nWhich))
            {
                aCoreSet.ClearItem(nWhich);
                bReset = TRUE;
            }
        }
        pParaItem = aParaIter.NextItem();
    }
    StartAction();
    if(bReset)
    {
        ResetAttr();
        SetAttr(aCoreSet);
    }
    pColl->SetAttr( rStyleSet );
    EndAction();
}

/*-----------------12.03.97 12.24-------------------

--------------------------------------------------*/

void SwWrtShell::AutoUpdateFrame( SwFrmFmt* pFmt, const SfxItemSet& rStyleSet )
{
    StartAction();

    ResetFlyFrmAttr( 0, &rStyleSet );
    pFmt->SetAttr( rStyleSet );

    EndAction();
}


void SwWrtShell::AutoCorrect( SvxAutoCorrect& rACorr, sal_Unicode cChar )
{
    ResetCursorStack();
    if(_CanInsert())
    {
        BOOL bStarted = FALSE;
        if(HasSelection())
        {
                // nur hier klammern, da das normale Insert schon an der
                // Editshell geklammert ist
            StartAllAction();
            StartUndo(UNDO_INSERT);
            bStarted = TRUE;
            DelRight();
        }
        SwEditShell::AutoCorrect( rACorr, IsInsMode(), cChar );

        if(bStarted)
        {
            EndAllAction();
            EndUndo(UNDO_INSERT);
        }
    }
}


/*
 * eine Art kontrollierter copy ctor
 */

SwWrtShell::SwWrtShell( SwWrtShell& rSh, Window *pWin, SwView &rShell )
    : SwFEShell( rSh, pWin ),
     COMMON_INI_LIST
{
    BITFLD_INI_LIST
    SET_CURR_SHELL( this );
    SetSfxViewShell( (SfxViewShell *)&rShell );
    SetFlyMacroLnk( LINK(this, SwWrtShell, ExecFlyMac) );
}


SwWrtShell::SwWrtShell( SwDoc& rDoc, Window *pWin, SwView &rShell,
                        SwRootFrm *pRoot, const SwViewOption *pViewOpt )
    : SwFEShell( rDoc, pWin, pRoot, pViewOpt),
      COMMON_INI_LIST
{
    BITFLD_INI_LIST
    SET_CURR_SHELL( this );
    SetSfxViewShell( (SfxViewShell *)&rShell );
    SetFlyMacroLnk( LINK(this, SwWrtShell, ExecFlyMac) );
}

/*
 * ctor
 */



SwWrtShell::~SwWrtShell()
{
    SET_CURR_SHELL( this );
    while(IsModePushed())
        PopMode();
    while(PopCrsr(FALSE))
        ;
}




void SwWrtShell::StartBasicAction()
{
    IncBasicAction();
    StartAllAction();
}



void SwWrtShell::SetBasicActionCount(USHORT nSet)
{
    DBG_ASSERT(!GetBasicActionCnt(), "Es sind schon Actions offen!")
    while( nSet )
    {
        IncBasicAction();
        StartAllAction();
        nSet--;
    }
}



void SwWrtShell::EndBasicAction()
{
    if(GetBasicActionCnt())
    {
        DecBasicAction();
        EndAllAction();
    }
}



USHORT SwWrtShell::EndAllBasicActions()
{
    USHORT nRet = GetBasicActionCnt();
    while( GetBasicActionCnt() )
    {
        DecBasicAction();
        EndAllAction();
    }
    return nRet;
}



FASTBOOL SwWrtShell::Pop( BOOL bOldCrsr )
{
    FASTBOOL bRet = SwCrsrShell::Pop( bOldCrsr );
    if( bRet && IsSelection() )
    {
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        fnKillSel = &SwWrtShell::ResetSelect;
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



BOOL SwWrtShell::CanInsert()
{
    return (!(IsSelFrmMode() | IsObjSelected() | (GetView().GetDrawFuncPtr() != NULL)));
}



    // die Core erzeugt eine Selektion, das SttSelect muss gerufen werden
void SwWrtShell::NewCoreSelection()
{
    SttSelect();
}

// --------------


/*************************************************************************

   $Log: not supported by cvs2svn $
   Revision 1.2  2000/10/06 13:41:54  jp
   should changes: don't use IniManager

   Revision 1.1.1.1  2000/09/18 17:14:53  hr
   initial import

   Revision 1.342  2000/09/18 16:06:27  willem.vandorp
   OpenOffice header added.

   Revision 1.341  2000/08/17 11:36:19  jp
   remove the SW graphicmanager

   Revision 1.340  2000/08/04 10:54:03  jp
   Soft-/HardHyphens & HardBlanks changed from attribute to unicode character; remove unused methods

   Revision 1.339  2000/07/04 15:19:25  tl
   XHyphenator1 => XHyphenator

   Revision 1.338  2000/06/27 17:42:38  jp
   Bug #76332#: internal SplitNode don't insert a new node before a table

   Revision 1.337  2000/06/26 13:28:29  os
   InsertDBColumnCntnt removed

   Revision 1.336  2000/05/19 12:58:39  jp
   use WordSelection class for check chars

   Revision 1.335  2000/05/19 11:00:26  jp
   Changes for Unicode

   Revision 1.334  2000/04/20 12:51:14  os
   GetName() returns String&

   Revision 1.333  2000/04/19 11:18:25  os
   UNICODE

   Revision 1.332  2000/03/30 13:26:18  os
   UNO III

   Revision 1.331  2000/03/23 07:51:11  os
   UNO III

*************************************************************************/



