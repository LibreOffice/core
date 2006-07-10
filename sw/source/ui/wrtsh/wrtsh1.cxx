/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wrtsh1.cxx,v $
 *
 *  $Revision: 1.53 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:04:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#pragma hdrstop

#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XVISUALOBJECT_HPP_
#include <com/sun/star/embed/XVisualObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDMISC_HPP_
#include <com/sun/star/embed/EmbedMisc.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_NOVISUALAREASIZEEXCEPTION_HPP_
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#endif

#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif

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
#include <sot/storage.hxx>
#ifndef _INSDLG_HXX //autogen
#include <svtools/insdlg.hxx>
#endif
#ifndef _SFX_FRMDESCRHXX
#include <sfx2/frmdescr.hxx>
#endif
#ifndef _EHDL_HXX //autogen
#include <svtools/ehdl.hxx>
#endif
#ifndef _SOERR_HXX //autogen
#include <svtools/soerr.hxx>
#endif
#ifndef _CACHESTR_HXX //autogen
#include <tools/cachestr.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX //autogen
#include <svx/sizeitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SCH_DLL_HXX
#include <sch/schdll.hxx>
#endif
#ifndef _SCH_MEMCHRT_HXX
#include <sch/memchrt.hxx>
#endif
#ifndef _MySVXACORR_HXX
#include <svx/svxacorr.hxx>
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
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#include <comphelper/storagehelper.hxx>
#include <svx/svxdlg.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>

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
#ifndef _SWDTFLVR_HXX
#include <swdtflvr.hxx>
#endif
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWERROR_H
#include <swerror.h>
#endif
#ifndef _WRTSH_HRC
#include <wrtsh.hrc>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif

#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif

#include <ndtxt.hxx>
#include <svx/acorrcfg.hxx>

// -> #111827#
#include <SwRewriter.hxx>
#include <comcore.hrc>
#include <undobj.hxx>
// <- #111827#

#include <toolkit/helper/vclunohelper.hxx>
#include <sfx2/viewfrm.hxx>

#include <svx/acorrcfg.hxx>

using namespace com::sun::star;

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


SvxAutoCorrect* lcl_IsAutoCorr()
{
       SvxAutoCorrect* pACorr = SvxAutoCorrCfg::Get()->GetAutoCorrect();
    if( pACorr && !pACorr->IsAutoCorrFlag( CptlSttSntnc | CptlSttWrd |
                            ChgFractionSymbol | ChgOrdinalNumber |
                            ChgToEnEmDash | SetINetAttr | Autocorrect ))
        pACorr = 0;
    return pACorr;
}

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
        BOOL bDelim = GetAppCharClass().isLetterNumeric( rStr, 0 );
        xub_StrLen nPos = 0, nStt = 0;
        for( ; nPos < rStr.Len(); nPos++ )
           {
            BOOL bTmpDelim = GetAppCharClass().isLetterNumeric( rStr, nPos );
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
    // FME: This seems to be an optimisation: CallChgLink should not be called
    // for an insert event. This cannot hold any longer, since inserting
    // characters from a different script type has to call CallChgLink.
//    SwChgLinkFlag *pChgFlg = bCallIns ? new SwChgLinkFlag( *this ) : 0;

    if( bHasSel || ( !bIns && SelectHiddenRange() ) )
    {
            // nur hier klammern, da das normale Insert schon an der
            // Editshell geklammert ist
        StartAllAction();

        // #111827#
        SwRewriter aRewriter;

        aRewriter.AddRule(UNDO_ARG1, GetCrsrDescr());
        aRewriter.AddRule(UNDO_ARG2, String(SW_RES(STR_YIELDS)));
        {
            String aTmpStr;
            aTmpStr += String(SW_RES(STR_START_QUOTE));
            aTmpStr += rStr;
            aTmpStr += String(SW_RES(STR_END_QUOTE));

            aRewriter.AddRule(UNDO_ARG3, rStr);
        }

        StartUndo(UNDO_REPLACE, &aRewriter);
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
        EndUndo(UNDO_REPLACE);
    }
//    delete pChgFlg;
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

    SwRewriter aRewriter;
    aRewriter.AddRule(UNDO_ARG1, SW_RES(STR_GRAPHIC));

    StartUndo(UNDO_INSERT, &aRewriter);

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

        pFrmMgr->SetHeightSizeType(ATT_FIX_SIZE);

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


void SwWrtShell::InsertObject( const svt::EmbeddedObjectRef& xRef, SvGlobalName *pName,
                            BOOL bActivate, USHORT nSlotId, SfxRequest* pReq )
{
    ResetCursorStack();
    if( !_CanInsert() )
        return;

    if( !xRef.is() )
    {
        // temporary storage
        svt::EmbeddedObjectRef xObj;
        uno::Reference < embed::XStorage > xStor = comphelper::OStorageHelper::GetTemporaryStorage();
        BOOL bDoVerb = TRUE;
        if ( pName )
        {
            comphelper::EmbeddedObjectContainer aCnt( xStor );
            ::rtl::OUString aName;
            // TODO/LATER: get aspect
            xObj.Assign( aCnt.CreateEmbeddedObject( pName->GetByteSequence(), aName ), embed::Aspects::MSOLE_CONTENT );
        }
        else
        {
            SvObjectServerList aServerList;
            switch (nSlotId)
            {
                case SID_INSERT_OBJECT:
                {
                    aServerList.FillInsertObjects();
                    aServerList.Remove( SwDocShell::Factory().GetClassId() );
                    // Intentionally no break!
                }

                // TODO/LATER: recording! Convert properties to items
                case SID_INSERT_PLUGIN:
                    /*
                    if(pReq)
                    {
                        INetURLObject* pURL = aDlg.GetURL();
                        if(pURL)
                            pReq->AppendItem(SfxStringItem(FN_PARAM_2, pURL->GetMainURL(INetURLObject::NO_DECODE)));
                        pReq->AppendItem(SfxStringItem(FN_PARAM_3 , aDlg.GetCommands()));
                    } */
                case SID_INSERT_APPLET:
                    /*
                    if(pReq)
                    {
                        SvAppletObjectRef xApplet ( xIPObj );
                        if(xApplet.Is())
                            pReq->AppendItem(SfxStringItem(FN_PARAM_1 , xApplet->GetCodeBase()));
                        pReq->AppendItem(SfxStringItem(FN_PARAM_2 , aDlg.GetClass()));
                        pReq->AppendItem(SfxStringItem(FN_PARAM_3 , aDlg.GetCommands()));
                    }*/
                case SID_INSERT_FLOATINGFRAME:
                    /*
                    if(pReq && xFloatingFrame.Is())
                    {
                        const SfxFrameDescriptor* pDescriptor = xFloatingFrame->GetFrameDescriptor();
                        pReq->AppendItem(SfxStringItem(FN_PARAM_1, pDescriptor->GetName()));
                        pReq->AppendItem(
                                SfxStringItem( FN_PARAM_2,
                                    pDescriptor->GetURL().GetMainURL(INetURLObject::NO_DECODE)));
                        pReq->AppendItem(SvxSizeItem(FN_PARAM_3, pDescriptor->GetMargin()));
                        pReq->AppendItem(SfxByteItem(FN_PARAM_4, pDescriptor->GetScrollingMode()));
                        pReq->AppendItem(SfxBoolItem(FN_PARAM_5, pDescriptor->HasFrameBorder()));
                    }*/
                {
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    SfxAbstractInsertObjectDialog* pDlg =
                            pFact->CreateInsertObjectDialog( GetWin(), nSlotId, xStor, &aServerList );
                    if ( pDlg )
                    {
                        pDlg->Execute();
                        bDoVerb = pDlg->IsCreateNew();
                        // TODO/LATER: pass Aspect
                        xObj.Assign( pDlg->GetObject(), embed::Aspects::MSOLE_CONTENT );
                        DELETEZ( pDlg );
                    }

                    break;
                }

                default:
                    break;
            }
        }

        if ( xObj.is() )
        {
            if( InsertOleObject( xObj ) && bActivate && bDoVerb )
            {
                SfxInPlaceClient* pClient = GetView().FindIPClient( xObj.GetObject(), &GetView().GetEditWin() );
                if ( !pClient )
                {
                    pClient = new SwOleClient( &GetView(), &GetView().GetEditWin(), xObj );
                    SetCheckForOLEInCaption( TRUE );
                }

                CalcAndSetScale( xObj );
                //#50270# Error brauchen wir nicht handeln, das erledigt das
                //DoVerb in der SfxViewShell
                ErrCode nErr = pClient->DoVerb( SVVERB_SHOW );

                // TODO/LATER: set document name - should be done in Client
                //if ( !ERRCODE_TOERROR( nErr ) )
                //    xIPObj->SetDocumentName( GetView().GetDocShell()->GetTitle() );
            }
        }
    }
    else
    {
        if( HasSelection() )
            DelRight();
        InsertOleObject( xRef );
    }
}

/*------------------------------------------------------------------------
 Beschreibung:   Object in die Core einfuegen.
                 Vom ClipBoard oder Insert
------------------------------------------------------------------------*/

BOOL SwWrtShell::InsertOleObject( const svt::EmbeddedObjectRef&  xRef )
{
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

        // set parent to get correct VisArea(in case of object needing parent printer)
        uno::Reference < container::XChild > xChild( xRef.GetObject(), uno::UNO_QUERY );
        if ( xChild.is() )
            xChild->setParent( pDoc->GetDocShell()->GetModel() );

        SvGlobalName aCLSID( xRef->getClassID() );
        bStarMath = ( SotExchange::IsMath( aCLSID ) != 0 );
        if( IsSelection() )
        {
            if( bStarMath )
            {
                String aMathData;
                GetSelectedText( aMathData, GETSELTXT_PARABRK_TO_ONLYCR );

                if( aMathData.Len() && svt::EmbeddedObjectRef::TryRunningState( xRef.GetObject() ) )
                {
                    uno::Reference < beans::XPropertySet > xSet( xRef->getComponent(), uno::UNO_QUERY );
                    if ( xSet.is() )
                    {
                        try
                        {
                            xSet->setPropertyValue( ::rtl::OUString::createFromAscii("Formula"), uno::makeAny( ::rtl::OUString( aMathData ) ) );
                            bActivate = FALSE;
                        }
                        catch ( uno::Exception& )
                        {
                        }
                    }
                }
            }
            DelRight();
        }

        if ( !bStarMath )
            SwFEShell::SplitNode( FALSE, FALSE );

        EnterSelFrmMode();

        SwFlyFrmAttrMgr aFrmMgr( TRUE, this, FRMMGR_TYPE_OLE );
        aFrmMgr.SetHeightSizeType(ATT_FIX_SIZE);

        SwRect aBound;
        CalcBoundRect( aBound, aFrmMgr.GetAnchor() );

        //The Size should be suggested by the OLE server
        sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;
        MapMode aRefMap( VCLUnoHelper::UnoEmbed2VCLMapUnit( xRef->getMapUnit( nAspect ) ) );

        awt::Size aSize;
        try
        {
            aSize = xRef->getVisualAreaSize( nAspect );
        }
        catch( embed::NoVisualAreaSizeException& )
        {
            // the default size will be set later
        }

        Size aSz( aSize.Width, aSize.Height );
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
        SwFEShell::InsertObject( xRef, &aFrmMgr.GetAttrSet() );

        EndAllAction();
        GetView().AutoCaption(OLE_CAP, &aCLSID);

        SwRewriter aRewriter;

        if ( bStarMath )
            aRewriter.AddRule(UNDO_ARG1, SW_RES(STR_FORMULA));
        else if ( SotExchange::IsChart( aCLSID ) )
            aRewriter.AddRule(UNDO_ARG1, SW_RES(STR_CHART));
        else
            aRewriter.AddRule(UNDO_ARG1, SW_RES(STR_OLE));


        EndUndo(UNDO_INSERT, &aRewriter);

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
         !GetView().GetViewFrame()->GetFrame()->IsInPlace() )
    {
        svt::EmbeddedObjectRef& xRef = GetOLEObject();
        ASSERT( xRef.is(), "OLE not found" );
        SfxInPlaceClient* pCli=0;

        //  Link fuer Daten-Highlighting im Chart zuruecksetzen
        SvtModuleOptions aMOpt;
        if( aMOpt.IsChart() )
        {
            SchMemChart* pMemChart=0;
            if( SotExchange::IsChart( SvGlobalName( xRef->getClassID() ) ) &&
                0 != (pMemChart = SchDLL::GetChartData( xRef.GetObject() ) ))
            {
                pMemChart->SetSelectionHdl( LINK( this, SwWrtShell,
                                            ChartSelectionHdl ) );
                //#60043# Damit die DataBrowseBox nicht erscheint wird das
                //Chart auf Readonly gesetzt wenn es eine Verbindung
                //zu einer Tabelle hat.
                if ( GetChartName( xRef.GetObject() ).Len() )
                {
                    pMemChart->SetReadOnly( TRUE );
                    pMemChart->SetNumberFormatter(GetDoc()->GetNumberFormatter( sal_True ));
                    SchDLL::Update(xRef.GetObject(), pMemChart);
                    xRef.UpdateReplacement();
                }
            }
        }

        pCli = GetView().FindIPClient( xRef.GetObject(), &GetView().GetEditWin() );
        if ( !pCli )
            pCli = new SwOleClient( &GetView(), &GetView().GetEditWin(), xRef );

        ((SwOleClient*)pCli)->SetInDoVerb( TRUE );

        CalcAndSetScale( xRef );
        pCli->DoVerb( nVerb );

        ((SwOleClient*)pCli)->SetInDoVerb( FALSE );
        CalcAndSetScale( xRef );
    }
}


void SwWrtShell::CalcAndSetScale( svt::EmbeddedObjectRef& xObj,
                             const SwRect *pFlyPrtRect,
                             const SwRect *pFlyFrmRect )
{
    //Einstellen der Skalierung am Client. Diese ergibt sich aus der Differenz
    //zwischen der VisArea des Objektes und der ObjArea.
    ASSERT( xObj.is(), "ObjectRef not  valid" );

    sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;
    sal_Int64 nMisc = xObj->getStatus( nAspect );

    //Das kann ja wohl nur ein nicht aktives Objekt sein. Diese bekommen
    //auf Wunsch die neue Groesse als VisArea gesetzt (StarChart)
    // TODO/LATER: how to get aspect?
    if( embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE & nMisc )
    {
        // TODO/MBA: testing
        SwRect aRect( pFlyPrtRect ? *pFlyPrtRect
                    : GetAnyCurRect( RECT_FLY_PRT_EMBEDDED, 0, xObj.GetObject() ));
        if( !aRect.IsEmpty() )
        {
            // TODO/LEAN: getMapUnit can switch object to running state
            // xObj.TryRunningState();

            MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );

            // TODO/LATER: needs complete VisArea?!
            Size aSize( OutputDevice::LogicToLogic( aRect.SVRect(), MAP_TWIP, aUnit ).GetSize() );
            awt::Size aSz;
            aSz.Width = aSize.Width();
            aSz.Height = aSize.Height();
            xObj->setVisualAreaSize( nAspect, aSz );
            // --> OD 2005-05-02 #i48419# - action 'UpdateReplacement' doesn't
            // have to change the modified state of the document.
            // This is only a workaround for the defect, that this action
            // modifies a document after load, because unnecessarily the
            // replacement graphic is updated, in spite of the fact that
            // nothing has been changed.
            // If the replacement graphic changes by this action, the document
            // will be already modified via other mechanisms.
            {
                bool bResetEnableSetModified(false);
                if ( GetDoc()->GetDocShell()->IsEnableSetModified() )
                {
                    GetDoc()->GetDocShell()->EnableSetModified( FALSE );
                    bResetEnableSetModified = true;
                }

                xObj.UpdateReplacement();

                if ( bResetEnableSetModified )
                {
                    GetDoc()->GetDocShell()->EnableSetModified( TRUE );
                }
            }
            // <--
        }
    }

    SfxInPlaceClient* pCli = GetView().FindIPClient( xObj.GetObject(), &GetView().GetEditWin() );
    if ( !pCli )
    {
        if ( (embed::EmbedMisc::EMBED_ACTIVATEIMMEDIATELY & nMisc)
            // TODO/LATER: ResizeOnPrinterChange
             //|| SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE & xObj->GetMiscStatus()
             )
        {
            pCli = new SwOleClient( &GetView(), &GetView().GetEditWin(), xObj );
        }
        else
            return;
    }

    // TODO/LEAN: getMapUnit can switch object to running state
    // xObj.TryRunningState();

    awt::Size aSize;
    try
    {
        aSize = xObj->getVisualAreaSize( nAspect );
    }
    catch( embed::NoVisualAreaSizeException& )
    {
        DBG_ERROR( "Can't get visual area size!\n" );
        // the scaling will not be done
    }

    Size aVisArea( aSize.Width, aSize.Height );

    BOOL bSetScale100 = TRUE;
    sal_Bool bUseObjectSize = sal_False;

    // solange keine vernuenftige Size vom Object kommt, kann nichts
    // skaliert werden
    if( aVisArea.Width() && aVisArea.Height() )
    {
        const MapMode aTmp( MAP_TWIP );
        MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
        aVisArea = OutputDevice::LogicToLogic( aVisArea, aUnit, aTmp);
        Size aObjArea;
        if ( pFlyPrtRect )
            aObjArea = pFlyPrtRect->SSize();
        else
            aObjArea = GetAnyCurRect( RECT_FLY_PRT_EMBEDDED, 0, xObj.GetObject() ).SSize();

        // differ the aObjArea and aVisArea by 1 Pixel then set new VisArea
        long nX, nY;
        SwSelPaintRects::Get1PixelInLogic( *this, &nX, &nY );
        if( !( aVisArea.Width() - nX <= aObjArea.Width() &&
               aVisArea.Width() + nX >= aObjArea.Width() &&
               aVisArea.Height()- nY <= aObjArea.Height()&&
               aVisArea.Height()+ nY >= aObjArea.Height() ))
        {
            // TODO/LATER: MISCSTATUS_RESIZEONPRINTERCHANGE
            /*
            if( SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE & nMisc )
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
            else*/

            if ( nMisc & embed::EmbedMisc::EMBED_NEVERRESIZE )
            {
                // the object must not be scaled, the size stored in object must be used for restoring
                bUseObjectSize = sal_True;
            }
            else
            {
                const Fraction aScaleWidth ( aObjArea.Width(),  aVisArea.Width() );
                const Fraction aScaleHeight( aObjArea.Height(), aVisArea.Height());
                pCli->SetSizeScale( aScaleWidth, aScaleHeight );
                bSetScale100 = FALSE;
            }
        }
    }

    if( bSetScale100 )
    {
        const Fraction aScale( 1, 1 );
        pCli->SetSizeScale( aScale, aScale );
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
        aArea = GetAnyCurRect( RECT_FLY_PRT_EMBEDDED, 0, xObj.GetObject() );
        aArea.Pos() += GetAnyCurRect( RECT_FLY_EMBEDDED, 0, xObj.GetObject() ).Pos();
    }

    if ( bUseObjectSize )
    {
        aArea.Width ( aVisArea.Width() );
        aArea.Height( aVisArea.Height() );
        RequestObjectResize( aArea, xObj.GetObject() );
    }
    else
    {
        aArea.Width ( Fraction( aArea.Width()  ) / pCli->GetScaleWidth() );
        aArea.Height( Fraction( aArea.Height() ) / pCli->GetScaleHeight());
    }

    pCli->SetObjArea( aArea.SVRect() );
}



void SwWrtShell::ConnectObj( svt::EmbeddedObjectRef& xObj, const SwRect &rPrt,
                            const SwRect &rFrm )
{
    SfxInPlaceClient* pCli = GetView().FindIPClient( xObj.GetObject(), &GetView().GetEditWin());
    if ( !pCli )
        pCli = new SwOleClient( &GetView(), &GetView().GetEditWin(), xObj );
    CalcAndSetScale( xObj, &rPrt, &rFrm );
}

IMPL_LINK( SwWrtShell, ChartSelectionHdl, ChartSelectionInfo *, pInfo )
{
    long nRet = 0;
    return nRet;
}

/*------------------------------------------------------------------------
 Beschreibung:  Einfuegen harter Seitenumbruch;
                Selektionen werden ueberschrieben
------------------------------------------------------------------------*/



void SwWrtShell::InsertPageBreak(const String *pPageDesc, USHORT nPgNum )
{
    ResetCursorStack();
    if( _CanInsert() )
    {
        ACT_KONTEXT(this);
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
}
/*------------------------------------------------------------------------
 Beschreibung:  Einfuegen harter Zeilenumbruch;
                Selektionen werden ueberschrieben
------------------------------------------------------------------------*/


void SwWrtShell::InsertLineBreak()
{
    ResetCursorStack();
    if( _CanInsert() )
    {
        if(HasSelection())
            DelRight();

        const sal_Unicode cIns = 0x0A;
        SvxAutoCorrect* pACorr = lcl_IsAutoCorr();
        if( pACorr )
            AutoCorrect( *pACorr, cIns );
        else
            SwWrtShell::Insert( String( cIns ) );
    }
}
/*------------------------------------------------------------------------
 Beschreibung:  Einfuegen harter Spaltenumbruch;
                Selektionen werden ueberschrieben
------------------------------------------------------------------------*/


void SwWrtShell::InsertColumnBreak()
{
    ACT_KONTEXT(this);
    ResetCursorStack();
    if( _CanInsert() )
    {
        StartUndo(UIUNDO_INSERT_COLUMN_BREAK);

        if ( !IsCrsrInTbl() )
        {
            if(HasSelection())
                DelRight();
            SwFEShell::SplitNode( FALSE, FALSE );
        }
        SetAttr(SvxFmtBreakItem(SVX_BREAK_COLUMN_BEFORE));

        EndUndo(UIUNDO_INSERT_COLUMN_BREAK);
    }
}

/*------------------------------------------------------------------------
 Beschreibung:  Einfuegen Fussnote
 Parameter:     rStr -- optionales Fussnotenzeichen
------------------------------------------------------------------------*/


void SwWrtShell::InsertFootnote(const String &rStr, BOOL bEndNote, BOOL bEdit )
{
    ResetCursorStack();
    if( _CanInsert() )
    {
        if(HasSelection())
            DelRight();

        SwFmtFtn aFootNote( bEndNote );
        if(rStr.Len())
            aFootNote.SetNumStr( rStr );

        SetAttr(aFootNote);

        if( bEdit )
        {
            // zur Bearbeiung des Fussnotentextes
            Left(CRSR_SKIP_CHARS, FALSE, 1, FALSE );
            GotoFtnTxt();
        }
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
    if( _CanInsert() )
    {
        ACT_KONTEXT(this);

        rView.GetEditWin().FlushInBuffer();
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
}

/*------------------------------------------------------------------------
 Beschreibung:  Numerierung anschalten
 Parameter:     Optionale Angabe eines Namens fuer die benannte Liste;
                dieser bezeichnet eine Position, wenn er in eine
                Zahl konvertierbar ist und kleiner ist als nMaxRules.
-------------------------------------------------------------------------*/


// zum Testen der CharFormate an der Numerierung
// extern void SetNumChrFmt( SwWrtShell*, SwNumRules& );

// -> #i40041#
// --> OD 2005-10-25 #b6340308#
// Preconditions (as far as OD has figured out):
// - <SwEditShell::HasNumber()> is FALSE, if <bNum> is TRUE
// - <SwEditShell::HasBullet()> is FALSE, if <bNum> is FALSE
// Behavior of method is determined by the current situation at the current
// cursor position in the document.
void SwWrtShell::NumOrBulletOn(BOOL bNum)
{
    // determine numbering rule found at current cursor position in the docment.
    const SwNumRule* pCurRule = GetCurNumRule();

    StartUndo(UNDO_NUMORNONUM);

    const SwNumRule * pNumRule = pCurRule;

    // --> OD 2005-10-25 #b6340308#
    // - activate outline rule respectively turning on outline rule for
    //   current text node. But, only for turning on a numbering (<bNum> == TRUE).
    // - overwrite found numbering rule at current cursor position, if
    //   no numbering rule can be retrieved from the paragraph style.
    bool bContinueFoundNumRule( false );
    bool bActivateOutlineRule( false );
    int nActivateOutlineLvl( MAXLEVEL );    // only relevant, if <bActivateOutlineRule> == TRUE
    SwTxtFmtColl * pColl = GetCurTxtFmtColl();
    if ( pColl )
    {
        // --> OD 2005-10-25 #b6340308# - retrieve numbering rule at paragraph
        // style, which is found at current cursor position in the document.
        SwNumRule* pCollRule = pDoc->FindNumRulePtr(pColl->GetNumRule().GetValue());
        // --> OD 2005-10-25 #125993# - The outline numbering rule isn't allowed
        // to be derived from a parent paragraph style to a derived one.
        // Thus check, if the found outline numbering rule is directly
        // set at the paragraph style <pColl>. If not, set <pCollRule> to NULL
        if ( pCollRule && pCollRule == GetDoc()->GetOutlineNumRule() )
        {
            const SwNumRule* pDirectCollRule =
                    pDoc->FindNumRulePtr(pColl->GetNumRule( FALSE ).GetValue());
            if ( !pDirectCollRule )
            {
                pCollRule = 0L;
            }
        }
        if ( pCollRule == NULL &&
             NO_NUMBERING != pColl->GetOutlineLevel() &&
             GetDoc()->IsOutlineLevelYieldsOutlineRule() )
        {
            pCollRule = GetDoc()->GetOutlineNumRule();
        }
        // <--
        // --> OD 2005-10-25 #b6340308#
        if ( !pCollRule )
        {
            pNumRule = pCollRule;
        }
        // --> OD 2006-06-12 #b6435904#
        // no activation or continuation of outline numbering in Writer/Web document
        else if ( bNum &&
                  !dynamic_cast<SwWebDocShell*>(GetDoc()->GetDocShell()) &&
                  pCollRule == GetDoc()->GetOutlineNumRule() )
        // <--
        {
            if ( pNumRule == pCollRule )
            {
                // check, if text node at current cursor positioned is counted.
                // If not, let it been counted. Then it has to be checked,
                // of the outline numbering has to be activated or continued.
                SwTxtNode* pTxtNode =
                            GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();
                if ( pTxtNode && !pTxtNode->IsCounted() )
                {
                    // check, if numbering of the outline level of the pararaph
                    // style is active. If not, activate this outline level.
                    nActivateOutlineLvl = pColl->GetOutlineLevel();
                    ASSERT( nActivateOutlineLvl >= 0 && nActivateOutlineLvl < MAXLEVEL,
                            "<SwWrtShell::NumOrBulletOn(..)> - paragraph style with outline rule, but no outline level" );
                    if ( nActivateOutlineLvl >= 0 && nActivateOutlineLvl < MAXLEVEL &&
                         pCollRule->Get( nActivateOutlineLvl ).GetNumberingType()
                            == SVX_NUM_NUMBER_NONE )
                    {
                        // activate outline numbering
                        bActivateOutlineRule = true;
                    }
                    else
                    {
                        // turning on outline numbering at current cursor position
                        bContinueFoundNumRule = true;
                    }
                }
                else
                {
                    // activate outline numbering, because from the precondition
                    // it's known, that <SwEdit::HasNumber()> == FALSE
                    bActivateOutlineRule = true;
                    nActivateOutlineLvl = pColl->GetOutlineLevel();
                }
            }
            else if ( !pNumRule )
            {
                // activate outline numbering, because from the precondition
                // it's known, that <SwEdit::HasNumber()> == FALSE
                bActivateOutlineRule = true;
                nActivateOutlineLvl = pColl->GetOutlineLevel();
            }
            else
            {
                // check, if numbering of the outline level of the pararaph
                // style is active. If not, activate this outline level.
                nActivateOutlineLvl = pColl->GetOutlineLevel();
                ASSERT( nActivateOutlineLvl >= 0 && nActivateOutlineLvl < MAXLEVEL,
                        "<SwWrtShell::NumOrBulletOn(..)> - paragraph style with outline rule, but no outline level" );
                if ( nActivateOutlineLvl >= 0 && nActivateOutlineLvl < MAXLEVEL &&
                     pCollRule->Get( nActivateOutlineLvl ).GetNumberingType()
                        == SVX_NUM_NUMBER_NONE )
                {
                    // activate outline numbering
                    bActivateOutlineRule = true;
                }
                else
                {
                    // turning on outline numbering at current cursor position
                    bContinueFoundNumRule = true;
                }
            }
            pNumRule = pCollRule;
        }
    }

    // --> OD 2005-10-25 #b6340308#
    // Only automatic numbering/bullet rules should be changed.
    // Note: The outline numbering rule is also an automatic one. It's only
    //       changed, if it has to be activated.
    if ( pNumRule )
    {
        if ( !pNumRule->IsAutoRule() )
        {
            pNumRule = 0L;
        }
        else if ( pNumRule == GetDoc()->GetOutlineNumRule() &&
                  !bActivateOutlineRule && !bContinueFoundNumRule )
        {
            pNumRule = 0L;
        }
    }
    // <--

    // --> OD 2005-10-25 #b6340308#
    // Search for a previous numbering/bullet rule to continue it.
    if (!pNumRule )
    {
        pNumRule = GetDoc()->SearchNumRule(*GetCrsr()->GetPoint(),
                                           FALSE, bNum, FALSE, 0);
        bContinueFoundNumRule = pNumRule != 0;
    }
    // <--

    if (pNumRule)
    {
        SwNumRule aNumRule(*pNumRule);

        // --> OD 2005-10-25 #b6340308#
        // do not change found numbering/bullet rule, if it should only be continued.
        if ( !bContinueFoundNumRule )
        {
            SwTxtNode * pTxtNode = GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();

            if (pTxtNode)
            {
                // --> OD 2005-10-26 #b6340308# - use above retrieve outline
                // level, if outline numbering has to be activated.
                int nLevel = bActivateOutlineRule
                             ? nActivateOutlineLvl
                             : pTxtNode->GetLevel();
                // <--

                if (nLevel < 0)
                    nLevel = 0;

                if (nLevel >= MAXLEVEL)
                    nLevel = MAXLEVEL - 1;

                SwNumFmt aFmt(aNumRule.Get(nLevel));

                if (bNum)
                    aFmt.SetNumberingType(SVX_NUM_ARABIC);
                else
                {
                    const Font* pFnt = &SwNumRule::GetDefBulletFont();
                    aFmt.SetBulletFont( pFnt );
                    aFmt.SetBulletChar( GetBulletChar(nLevel));
                    aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
                }
                aNumRule.Set(nLevel, aFmt);
            }
        }
        // <--

        SetCurNumRule(aNumRule);
    }
    else
    {
        SwNumRule aNumRule(GetUniqueNumRuleName());
        // Zeichenvorlage an die Numerierung haengen
        SwCharFmt* pChrFmt;
        SwDocShell* pDocSh = GetView().GetDocShell();
        const Font* pFnt = &SwNumRule::GetDefBulletFont();

        if (bNum)
        {
            pChrFmt = GetCharFmtFromPool( RES_POOLCHR_NUM_LEVEL );
        }
        else
        {
            pChrFmt = GetCharFmtFromPool( RES_POOLCHR_BUL_LEVEL );
        }

        SwTxtNode * pTxtNode =
            GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();
        USHORT nWidthOfTabs = pTxtNode->GetWidthOfLeadingTabs();
        GetDoc()->RemoveLeadingWhiteSpace( *GetCrsr()->GetPoint() );

        const bool bHtml = 0 != PTR_CAST(SwWebDocShell, pDocSh);
        const bool bRightToLeft = IsInRightToLeftText();
        for( BYTE nLvl = 0; nLvl < MAXLEVEL; ++nLvl )
        {
            SwNumFmt aFmt( aNumRule.Get( nLvl ) );
            aFmt.SetCharFmt( pChrFmt );

            if (! bNum)
            {
                aFmt.SetBulletFont( pFnt );
                aFmt.SetBulletChar( GetBulletChar(nLvl) );
                aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            }

            if(bHtml && nLvl)
            {
                // 1/2" fuer HTML
                aFmt.SetLSpace(720);
                aFmt.SetAbsLSpace(nLvl * 720);
            }
            else if ( nWidthOfTabs > 0 )
            {
                aFmt.SetAbsLSpace(nWidthOfTabs + nLvl * 720);
            }

            // --> FME 2005-01-21 #i38904#  Default alignment for
            // numbering/bullet should be rtl in rtl paragraph:
            if ( bRightToLeft )
            {
                aFmt.SetNumAdjust( SVX_ADJUST_RIGHT );
            }
            // <--

            aNumRule.Set( nLvl, aFmt );
        }

        SetCurNumRule( aNumRule );
    }

    EndUndo(UNDO_NUMORNONUM);
}
// <- #i40041#

void SwWrtShell::NumOn()
{
    NumOrBulletOn(TRUE);
}

void SwWrtShell::NumOrBulletOff()
{
    const SwNumRule * pCurNumRule = GetCurNumRule();

    if (pCurNumRule)
    {
        if (pCurNumRule->IsOutlineRule())
        {
            SwNumRule aNumRule(*pCurNumRule);

            SwTxtNode * pTxtNode =
                GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();

            if (pTxtNode)
            {
                unsigned int nLevel = pTxtNode->GetLevel();
                SwNumFmt aFmt(aNumRule.Get(nLevel));

                aFmt.SetNumberingType(SVX_NUM_NUMBER_NONE);
                aNumRule.Set(nLevel, aFmt);

                SetCurNumRule(aNumRule);
            }
        }
        else
        {
            DelNumRules();
        }

        // --> OD 2005-10-24 #126346# - Cursor can not be anymore in front of
        // a label, because numbering/bullet is switched off.
        SetInFrontOfLabel( FALSE );
        // <--
    }
}
// <- #i29560#

/*------------------------------------------------------------------------
 Beschreibung:  Default-Bulletliste erfragen
------------------------------------------------------------------------*/

void SwWrtShell::BulletOn()
{
    NumOrBulletOn(FALSE);
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
    int nCnt;

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
            else if( GetDrawView()->GetContext() == SDRCONTEXT_MEDIA )
                nCnt |= SEL_MEDIA;

            if (svx::checkForSelectedCustomShapes(
                    const_cast<SdrView *>(GetDrawView()),
                    true /* bOnlyExtruded */ ))
            {
                nCnt |= SwWrtShell::SEL_EXTRUDED_CUSTOMSHAPE;
            }
            sal_uInt32 nCheckStatus = 0;
            if (svx::checkForSelectedFontWork(
                    const_cast<SdrView *>(GetDrawView()), nCheckStatus ))
            {
                nCnt |= SwWrtShell::SEL_FONTWORK;
            }
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

    // --> FME 2005-01-12 #i39855#
    // Do not pop up numbering toolbar, if the text node has a numbering
    // of type SVX_NUM_NUMBER_NONE.
    const SwNumRule* pNumRule = GetCurNumRule();
    if ( pNumRule )
    {
        const SwTxtNode* pTxtNd =
            GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();

        if ( pTxtNd )
        {
            const SwNumFmt& rFmt = pNumRule->Get(pTxtNd->GetLevel());
            if ( SVX_NUM_NUMBER_NONE != rFmt.GetNumberingType() )
                nCnt |= SEL_NUM;
        }
    }
    // <--

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
        USHORT nId = SwStyleNameMapper::GetPoolIdFromUIName( rCollName, GET_POOLID_TXTCOLL );
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
        USHORT nId = SwStyleNameMapper::GetPoolIdFromUIName( rFmtName, GET_POOLID_CHRFMT );
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
    pDoc->ChgFmt(*pColl, rStyleSet );
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
    SwTransferable::ClearSelection( *this );
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
void SwWrtShell::ChgDBData(const SwDBData& aDBData)
{
    SwEditShell::ChgDBData(aDBData);
    //notify the db-beamer if available
    GetView().NotifyDBChanged();
}

String SwWrtShell::GetSelDescr() const
{
    String aResult;

    int nSelType = GetSelectionType();
    switch (nSelType)
    {
    case SEL_GRF:
        aResult = SW_RES(STR_GRAPHIC);

        break;
    case SEL_FRM:
        {
            const SwFrmFmt * pFrmFmt = GetCurFrmFmt();

            if (pFrmFmt)
                aResult = pFrmFmt->GetDescription();
        }
        break;
    case SEL_DRW:
        {
            aResult = SW_RES(STR_DRAWING_OBJECTS);
        }
        break;
    default:
        if (0 != pDoc)
            aResult = GetCrsrDescr();
    }

    return aResult;
}
