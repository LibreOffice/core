/*************************************************************************
 *
 *  $RCSfile: objxtor.cxx,v $
 *
 *  $Revision: 1.38 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:01:51 $
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

#include "arrdecl.hxx"

#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_RESARY_HXX
#include <vcl/resary.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _RTTI_HXX //autogen
#include <tools/rtti.hxx>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _SFX_HELP_HXX
#include <sfxhelp.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#pragma hdrstop

#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif

#include "objsh.hxx"

#ifndef _BASIC_SBUNO_HXX
#include <basic/sbuno.hxx>
#endif

#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PRINTWARNINGOPTIONS_HXX
#include <svtools/printwarningoptions.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

#include <svtools/urihelper.hxx>
#include <svtools/pathoptions.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <svtools/asynclink.hxx>
#include <sot/clsids.hxx>

#include "docfac.hxx"
#include "docfile.hxx"
#include "event.hxx"
#include "cfgmgr.hxx"
#include "dispatch.hxx"
#include "viewsh.hxx"
#include "interno.hxx"
#include "sfxresid.hxx"
#include "objshimp.hxx"
#include "appbas.hxx"
#include "sfxtypes.hxx"
#include "evntconf.hxx"
#include "request.hxx"
#include "doc.hrc"
#include "sfxlocal.hrc"
#include "docinf.hxx"
#include "objuno.hxx"
#include "appdata.hxx"
#include "appuno.hxx"
#include "sfxsids.hrc"
#include "basmgr.hxx"
#include "dlgcont.hxx"
#include "scriptcont.hxx"
#include "imgmgr.hxx"
#include "tbxconf.hxx"
#include "accmgr.hxx"
#include "helpid.hrc"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;

#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx>
#endif

//====================================================================

DBG_NAME(SfxObjectShell);

#define DocumentInfo
#include "sfxslots.hxx"

extern svtools::AsynchronLink* pPendingCloser;
static SfxObjectShell* pWorkingDoc = NULL;

//=========================================================================

TYPEINIT1(SfxObjectShell, SfxShell);

//--------------------------------------------------------------------

SfxObjectShell_Impl::~SfxObjectShell_Impl()
{
    if ( pPendingCloser == pCloser )
        pPendingCloser = 0;
    delete pCloser;
}

// initializes a document from a file-description

SfxObjectShell::SfxObjectShell
(
    SfxObjectCreateMode eMode   /*  Zweck, zu dem die SfxObjectShell
                                    erzeugt wird:

                                    SFX_CREATE_MODE_EMBEDDED (default)
                                        als SO-Server aus einem anderen
                                        Dokument heraus

                                    SFX_CREATE_MODE_STANDARD,
                                        als normales, selbst"aendig ge"offnetes
                                        Dokument

                                    SFX_CREATE_MODE_PREVIEW
                                        um ein Preview durchzuf"uhren,
                                        ggf. werden weniger Daten ben"otigt

                                    SFX_CREATE_MODE_ORGANIZER
                                        um im Organizer dargestellt zu
                                        werden, hier werden keine Inhalte
                                        ben"otigt */
)

/*  [Beschreibung]

    Konstruktor der Klasse SfxObjectShell.
*/

:   pImp( new SfxObjectShell_Impl ),
    _pFactory( 0 ),
    eCreateMode(eMode),
    pStyleSheetPool(0),
    pMedium(0)
{
    DBG_CTOR(SfxObjectShell, 0);

    bHasName = sal_False;
    SfxShell::SetName( GetTitle() );
    nViewNo = 0;

    // Aggregation InPlaceObject+Automation
//(mba)    AddInterface( SvDispatch::ClassFactory() );

    SfxObjectShell *pThis = this;
    SfxObjectShellArr_Impl &rArr = SFX_APP()->GetObjectShells_Impl();
    rArr.C40_INSERT( SfxObjectShell, pThis, rArr.Count() );
    pImp->bInList = sal_True;
    pImp->nLoadedFlags = SFX_LOADED_ALL;
    SetObjectShell( TRUE );
}

//--------------------------------------------------------------------

// virtual dtor of typical base-class SfxObjectShell

SfxObjectShell::~SfxObjectShell()
{
    DBG_DTOR(SfxObjectShell, 0);

    if ( IsEnableSetModified() )
        EnableSetModified( sal_False );

    // Niemals GetInPlaceObject() aufrufen, der Zugriff auf den
    // Ableitungszweig SfxInternObject ist wegen eines Compiler Bugs nicht
    // erlaubt
    SfxObjectShell::Close();
    pImp->xModel = NULL;

    String aPhysName;
    if ( pMedium )
        aPhysName = pMedium->GetPhysicalName();

    DELETEX(pImp->pEventConfig);
    DELETEX(pImp->pImageManager);
    DELETEX(pImp->pTbxConfig);
    DELETEX(pImp->pAccMgr);
    DELETEX(pImp->pCfgMgr);
    DELETEX(pImp->pReloadTimer );

    SfxApplication *pSfxApp = SFX_APP();
    if ( USHRT_MAX != pImp->nVisualDocumentNumber )
        pSfxApp->ReleaseIndex(pImp->nVisualDocumentNumber);

    // Basic-Manager zerst"oren
    if ( pImp->pBasicMgr )
        DELETEX(pImp->pBasicMgr);
    if( pImp->pBasicLibContainer )
        pImp->pBasicLibContainer->release();
    if( pImp->pDialogLibContainer )
        pImp->pDialogLibContainer->release();

    if ( pSfxApp->GetDdeService() )
        pSfxApp->RemoveDdeTopic( this );

    delete pImp->pDocInfo;
    if ( pImp->xModel.is() )
        pImp->xModel = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > ();

    if ( pMedium && pMedium->IsTemporary() )
        HandsOff();

    DELETEX( pMedium );

    if ( pImp->aTempName.Len() )
    {
        if ( aPhysName == pImp->aTempName && !IsHandsOff() )
            HandsOff();
        String aTmp;
        ::utl::LocalFileHelper::ConvertPhysicalNameToURL( pImp->aTempName, aTmp );
        ::utl::UCBContentHelper::Kill( aTmp );
    }

    delete pImp;
}

//--------------------------------------------------------------------

void SfxObjectShell::Stamp_SetPrintCancelState(sal_Bool bState)
{
    pImp->bIsPrintJobCancelable = bState;
}

//--------------------------------------------------------------------

sal_Bool SfxObjectShell::Stamp_GetPrintCancelState() const
{
    return pImp->bIsPrintJobCancelable;
}

//--------------------------------------------------------------------

SfxObjectFactory& SfxObjectShell::GetFactory() const
{
    return *_pFactory;
}
//--------------------------------------------------------------------

void SfxObjectShell::ViewAssigned()

/*  [Beschreibung]

    Diese Methode wird gerufen, wenn eine ::com::sun::star::sdbcx::View zugewiesen wird.
*/

{
    // Spaetestens jetzt die Factory initialisieren (wegen HelpFileName)
    GetFactory().DoInitFactory();
}

//--------------------------------------------------------------------
// closes the Object and all its views

sal_Bool SfxObjectShell::Close()
{
    {DBG_CHKTHIS(SfxObjectShell, 0);}
    if ( !pImp->bClosing )
    {
        // falls noch ein Progress l"auft, nicht schlie\sen
        if ( !pImp->bDisposing && GetProgress() )
            return sal_False;

        pImp->bClosing = sal_True;
        Reference< util::XCloseable > xCloseable( GetBaseModel(), UNO_QUERY );

        if ( xCloseable.is() )
        {
            try
            {
                xCloseable->close( sal_True );
            }
            catch( Exception& )
            {
                pImp->bClosing = sal_False;
            }
        }

        if ( pImp->bClosing )
        {
            // aus Document-Liste austragen
            SfxApplication *pSfxApp = SFX_APP();
            SfxObjectShellArr_Impl &rDocs = pSfxApp->GetObjectShells_Impl();
            const SfxObjectShell *pThis = this;
            sal_uInt16 nPos = rDocs.GetPos(pThis);
            if ( nPos < rDocs.Count() )
                rDocs.Remove( nPos );
            pImp->bInList = sal_False;

            // Broadcasten (w"ahrend dessen festhalten)
            SfxObjectShellRef aRef(this);
/*
            // Ist leider zu sp"at, da kaum noch Macros laufen, wenn keine View
            // mehr da ist!
            if ( _pFactory && _pFactory->GetFlags() & SFXOBJECTSHELL_HASOPENDOC )
                // Event nur bei echten Dokumenten
                pSfxApp->NotifyEvent( SfxEventHint(SFX_EVENT_CLOSEDOC, this) );
*/
            // Broadcast moved to SfxBaseModel!
            //Broadcast( SfxSimpleHint(SFX_HINT_DEINITIALIZING) );
            //pImp->bClosing = sal_False;
        }
    }

    return sal_True;
}

//--------------------------------------------------------------------

// returns a pointer the first SfxDocument of specified type

SfxObjectShell* SfxObjectShell::GetFirst
(
    const TypeId* pType ,
    sal_Bool            bOnlyVisible
)
{
    SfxObjectShellArr_Impl &rDocs = SFX_APP()->GetObjectShells_Impl();

    // seach for a SfxDocument of the specified type
    for ( sal_uInt16 nPos = 0; nPos < rDocs.Count(); ++nPos )
    {
        SfxObjectShell* pSh = rDocs.GetObject( nPos );
        if ( bOnlyVisible && pSh->IsPreview() && pSh->IsReadOnly() )
            continue;

        if ( ( !pType || pSh->IsA(*pType) ) &&
             ( !bOnlyVisible || SfxViewFrame::GetFirst( pSh, 0, sal_True )))
            return pSh;
    }

    return 0;
}
//--------------------------------------------------------------------

// returns a pointer to the next SfxDocument of specified type behind *pDoc

SfxObjectShell* SfxObjectShell::GetNext
(
    const SfxObjectShell&   rPrev,
    const TypeId*           pType,
    sal_Bool                    bOnlyVisible
)
{
    SfxObjectShellArr_Impl &rDocs = SFX_APP()->GetObjectShells_Impl();

    // refind the specified predecessor
    sal_uInt16 nPos;
    for ( nPos = 0; nPos < rDocs.Count(); ++nPos )
        if ( rDocs.GetObject(nPos) == &rPrev )
            break;

    // search for the next SfxDocument of the specified type
    for ( ++nPos; nPos < rDocs.Count(); ++nPos )
    {
        SfxObjectShell* pSh = rDocs.GetObject( nPos );
        if ( bOnlyVisible && pSh->IsPreview() && pSh->IsReadOnly() )
            continue;

        if ( ( !pType || pSh->IsA(*pType) ) &&
             ( !bOnlyVisible || SfxViewFrame::GetFirst( pSh, 0, sal_True )))
            return pSh;
    }
    return 0;
}

//--------------------------------------------------------------------

SfxObjectShell* SfxObjectShell::Current()
{
    SfxViewFrame *pFrame = SFX_APP()->GetViewFrame();
    return pFrame ? pFrame->GetObjectShell() : 0;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsInPrepareClose() const

/*  [Beschreibung]

    Diese Methode liefert sal_True, falls gerade ein PrepareClose laeuft.
*/

{
    return pImp->bInPrepareClose;
}

//------------------------------------------------------------------------

struct BoolEnv_Impl
{
    SfxObjectShell_Impl* pImp;
    BoolEnv_Impl( SfxObjectShell_Impl* pImpP) : pImp( pImpP )
    { pImpP->bInPrepareClose = sal_True; }
    ~BoolEnv_Impl() { pImp->bInPrepareClose = sal_False; }
};


sal_uInt16 SfxObjectShell::PrepareClose
(
    sal_Bool    bUI,        // sal_True: Dialoge etc. erlaubt, sal_False: silent-mode
    sal_Bool    bForBrowsing
)
{
    if( pImp->bInPrepareClose || pImp->bPreparedForClose )
        return sal_True;
    BoolEnv_Impl aBoolEnv( pImp );

    // DocModalDialog?
    if ( IsInModalMode() )
        return sal_False;

    SfxViewFrame* pFirst = SfxViewFrame::GetFirst( this );
    if( pFirst && !pFirst->GetFrame()->PrepareClose_Impl( bUI, bForBrowsing ) )
        return sal_False;

    // prepare views for closing
    for ( SfxViewFrame* pFrm = SfxViewFrame::GetFirst(
        this, TYPE(SfxViewFrame));
          pFrm; pFrm = SfxViewFrame::GetNext( *pFrm, this ) )
    {
        DBG_ASSERT(pFrm->GetViewShell(),"KeineShell");
        if ( pFrm->GetViewShell() )
        {
            sal_uInt16 nRet = pFrm->GetViewShell()->PrepareClose( bUI, bForBrowsing );
            if ( nRet != sal_True )
                return nRet;
        }
    }

    if( GetInPlaceObject() && GetInPlaceObject()->GetClient())
    {
        pImp->bPreparedForClose = sal_True;
        return sal_True;
    }

    // ggf. nachfragen, ob gespeichert werden soll
        // nur fuer in sichtbaren Fenstern dargestellte Dokumente fragen
    SfxViewFrame *pFrame = SfxObjectShell::Current() == this
        ? SfxViewFrame::Current() : SfxViewFrame::GetFirst( this );
    while ( pFrame && (pFrame->GetFrameType() & SFXFRAME_SERVER ) )
        pFrame = SfxViewFrame::GetNext( *pFrame, this );

    SfxApplication *pSfxApp = SFX_APP();
    pSfxApp->NotifyEvent( SfxEventHint(SFX_EVENT_PREPARECLOSEDOC, this) );
    sal_Bool bClose = sal_False;
    if ( bUI && IsModified() )
    {
        if ( pFrame )
        {
            // minimierte restoren
            SfxFrame* pTop = pFrame->GetTopFrame();
            pSfxApp->SetViewFrame( pTop->GetCurrentViewFrame() );
            pFrame->GetFrame()->Appear();

            // fragen, ob gespeichert werden soll
            String aText( SfxResId( STR_QUERY_SAVE_DOCUMENT ) );
            aText.SearchAndReplace( DEFINE_CONST_UNICODE( "$(DOC)" ),
                                    GetTitle( SFX_TITLE_PICKLIST ) );
            /*HACK for plugin::destroy()*/
            // Don't show SAVE dialog in plugin mode! We save our document in every way.
            short nRet = RET_YES;
            if( SfxApplication::IsPlugin() == sal_False || bUI == 2 )
            {
                //initiate help agent to inform about "print modifies the document"
                SfxStamp aStamp = GetDocInfo().GetPrinted();
                SvtPrintWarningOptions aPrintOptions;
                if(aPrintOptions.IsModifyDocumentOnPrintingAllowed() && HasName() && aStamp.IsValid())
                {
                    SfxHelp::OpenHelpAgent(pFirst->GetFrame(), HID_CLOSE_WARNING);
                }
                QueryBox aQBox( &pFrame->GetWindow(), WB_YES_NO_CANCEL | WB_DEF_YES, aText );
                aQBox.SetButtonText( BUTTONID_NO, SfxResId( STR_NOSAVEANDCLOSE ) );
                aQBox.SetButtonText( BUTTONID_YES, SfxResId( STR_SAVEDOC ) );
    //(mba)/task            if ( bForBrowsing )
    //                aQBox.AddButton( String( SfxResId( RID_STR_NEW_TASK ) ), RET_NEWTASK, BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON );
                nRet = aQBox.Execute();
            }
            /*HACK for plugin::destroy()*/

            if ( RET_YES == nRet )
            {
                sal_Bool bVersion = GetDocInfo().IsSaveVersionOnClose();

                // per Dispatcher speichern
                const SfxPoolItem *pPoolItem;
                if ( bVersion )
                {
                    SfxStringItem aItem( SID_DOCINFO_COMMENTS, String( SfxResId( STR_AUTOMATICVERSION ) ) );
                    SfxBoolItem aWarnItem( SID_FAIL_ON_WARNING, bUI );
                    const SfxPoolItem* ppArgs[] = { &aItem, &aWarnItem, 0 };
                    pPoolItem = pFrame->GetBindings().ExecuteSynchron( SID_SAVEDOC, ppArgs );
                }
                else
                {
                    SfxBoolItem aWarnItem( SID_FAIL_ON_WARNING, bUI );
                    const SfxPoolItem* ppArgs[] = { &aWarnItem, 0 };
                    pPoolItem = pFrame->GetBindings().ExecuteSynchron( SID_SAVEDOC, ppArgs );
                }

                if ( !pPoolItem || pPoolItem->ISA(SfxVoidItem) || ( pPoolItem->ISA(SfxBoolItem) && !( (const SfxBoolItem*) pPoolItem )->GetValue() ) )
                    return sal_False;
                else
                    bClose = sal_True;
            }
            else if ( RET_CANCEL == nRet )
                // abgebrochen
                return sal_False;
            else if ( RET_NEWTASK == nRet )
            {
                return RET_NEWTASK;
            }
            else
            {
                // Bei Nein nicht noch Informationlost
                bClose = sal_True;
            }
        }
    }

    // ggf. hinweisen, da\s unter Fremdformat gespeichert
    if( pMedium )
    {
        SFX_ITEMSET_ARG( pMedium->GetItemSet(), pIgnoreInformationLost,
                         SfxBoolItem, SID_DOC_IGNOREINFORMATIONLOST, sal_False);
        if( pIgnoreInformationLost && pIgnoreInformationLost->GetValue() )
            bUI = sal_False;
    }
    if ( bUI && !bClose && IsInformationLost() )
    {
        // minimierte restoren
        SfxFrame* pTop = pFrame->GetTopFrame();
        pSfxApp->SetViewFrame( pTop->GetCurrentViewFrame() );
        pFrame->GetFrame()->Appear();
        QueryBox aBox( &pFrame->GetWindow(), SfxResId(MSG_CONFIRM_FILTER));
        if ( RET_NO == aBox.Execute() )
            return sal_False;
    }

    pImp->bPreparedForClose = sal_True;
    return sal_True;
}

//--------------------------------------------------------------------

BasicManager* SfxObjectShell::GetBasicManager() const
{
    return HasBasic() ? pImp->pBasicMgr : SFX_APP()->GetBasicManager();
}

sal_Bool SfxObjectShell::HasBasic() const
{
    if ( !pImp->bBasicInitialized )
    {
        String aName( GetMedium()->GetName() );
        ((SfxObjectShell*)this)->InitBasicManager_Impl( GetStorage(), aName.Len() ? &aName : NULL );
    }
    return pImp->pBasicMgr != NULL;
}

//--------------------------------------------------------------------

Reference< XLibraryContainer > SfxObjectShell::GetDialogContainer()
{
    if( !pImp->pDialogLibContainer )
        GetBasicManager();
    Reference< XLibraryContainer > xRet
        = static_cast< XLibraryContainer* >( pImp->pDialogLibContainer );
    return xRet;
}

//--------------------------------------------------------------------

Reference< XLibraryContainer > SfxObjectShell::GetBasicContainer()
{
    if( !pImp->pBasicLibContainer )
        GetBasicManager();
    Reference< XLibraryContainer > xRet
        = static_cast< XLibraryContainer* >( pImp->pBasicLibContainer );
    return xRet;
}

//--------------------------------------------------------------------

StarBASIC* SfxObjectShell::GetBasic() const
{
    return GetBasicManager()->GetLib(0);
}

//--------------------------------------------------------------------

void SfxObjectShell::InitBasicManager_Impl
(
    SvStorage*  pStor       /* Storage, aus dem das Dokument geladen wird
                               (aus <SvPersist::Load()>) bzw. 0, falls es
                               sich um ein neues Dokument handelt
                               (aus <SvPersist::InitNew()>). */
    , const String* pName
)
/*  [Beschreibung]

    Erzeugt einen Dokument-BasicManager und l"adt diesen ggf. (pStor != 0)
    aus dem Storage.


    [Anmerkung]

    Diese Methode mu"s aus den "Uberladungen von <SvPersist::Load()> (mit
    dem pStor aus dem Parameter von Load()) sowie aus der "Uberladung
    von <SvPersist::InitNew()> (mit pStor = 0) gerufen werden.
*/

{
    StarBASIC *pAppBasic = SFX_APP()->GetBasic();
    DBG_ASSERT( !pImp->bBasicInitialized && !pImp->pBasicMgr, "Lokaler BasicManager bereits vorhanden");

    pImp->bBasicInitialized = TRUE;
    BasicManager* pBasicManager;
    if ( pStor )
    {
        String aOldURL = INetURLObject::GetBaseURL();
        String aNewURL;
        if( HasName() )
            aNewURL = GetMedium()->GetName();
        else
        {
            aNewURL = GetDocInfo().GetTemplateFileName();
            // Bei Templates keine ::com::sun::star::util::URL...
            aNewURL = URIHelper::SmartRelToAbs( aNewURL );
        }
        INetURLObject::SetBaseURL( aNewURL );

        // load BASIC-manager
        SfxErrorContext aErrContext( ERRCTX_SFX_LOADBASIC, GetTitle() );
#if SUPD<613//MUSTINI
        SfxIniManager *pIniMgr = SFX_APP()->GetIniManager();
        String aAppBasicDir( pIniMgr->Get(SFX_KEY_BASIC_PATH) );
#else
        String aAppBasicDir = SvtPathOptions().GetBasicPath();
#endif
        pImp->pBasicMgr = pBasicManager = new BasicManager( *pStor, pAppBasic, &aAppBasicDir );
        if ( pImp->pBasicMgr->HasErrors() )
        {
            // handle errors
            BasicError *pErr = pImp->pBasicMgr->GetFirstError();
            while ( pErr )
            {
                // show message to user
                if ( ERRCODE_BUTTON_CANCEL ==
                     ErrorHandler::HandleError( pErr->GetErrorId() ) )
                {
                    // user wants to break loading of BASIC-manager
                    delete pImp->pBasicMgr;
                    pStor = 0;
                    break;
                }
                pErr = pImp->pBasicMgr->GetNextError();
            }
        }

        INetURLObject::SetBaseURL( aOldURL );
    }

    // not loaded?
    if ( !pStor )
    {
        // create new BASIC-manager
        StarBASIC *pBas = new StarBASIC(pAppBasic);
        pBas->SetFlag( SBX_EXTSEARCH );
        pImp->pBasicMgr = pBasicManager = new BasicManager( pBas );
    }

    // Basic container
    SfxScriptLibraryContainer* pBasicCont = new SfxScriptLibraryContainer
        ( DEFINE_CONST_UNICODE( "StarBasic" ), pBasicManager, pStor );
    pBasicCont->acquire();  // Hold via UNO
    Reference< XLibraryContainer > xBasicCont = static_cast< XLibraryContainer* >( pBasicCont );
    pImp->pBasicLibContainer = pBasicCont;

    // Dialog container
    SfxDialogLibraryContainer* pDialogCont = new SfxDialogLibraryContainer( pStor );
    pDialogCont->acquire(); // Hold via UNO
    Reference< XLibraryContainer > xDialogCont = static_cast< XLibraryContainer* >( pDialogCont );
    pImp->pDialogLibContainer = pDialogCont;

    LibraryContainerInfo* pInfo = new LibraryContainerInfo
        ( xBasicCont, xDialogCont, static_cast< OldBasicPassword* >( pBasicCont ) );
    pBasicManager->SetLibraryContainerInfo( pInfo );
    pBasicCont->setBasicManager( pBasicManager );

    // damit auch Dialoge etc. 'qualifiziert' angesprochen werden k"onnen
    StarBASIC *pBas = pImp->pBasicMgr->GetLib(0);
    // Initialize Uno
    //pBas->setRoot( GetModel() );
    sal_Bool bWasModified = pBas->IsModified();
    pBas->SetParent( pAppBasic );

    // Properties im Doc-BASIC
    // ThisComponent
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xInterface ( GetModel(), ::com::sun::star::uno::UNO_QUERY );
    ::com::sun::star::uno::Any aComponent;
    aComponent <<= xInterface;
    SbxObjectRef xUnoObj = GetSbUnoObject( DEFINE_CONST_UNICODE("ThisComponent"), aComponent );
    xUnoObj->SetFlag( SBX_DONTSTORE );
    pBas->Insert( xUnoObj );

    // Standard lib name
    rtl::OUString aStdLibName( RTL_CONSTASCII_USTRINGPARAM( "Standard" ) );

    // Basic container
    if ( xBasicCont.is() && !xBasicCont->hasByName( aStdLibName ) )
        xBasicCont->createLibrary( aStdLibName );   // create Standard library
    Any aBasicCont;
    aBasicCont <<= xBasicCont;
    xUnoObj = GetSbUnoObject( DEFINE_CONST_UNICODE("BasicLibraries"), aBasicCont );
    pBas->Insert( xUnoObj );

    // Dialog container
    if ( xDialogCont.is() && !xDialogCont->hasByName( aStdLibName ) )
        xDialogCont->createLibrary( aStdLibName );  // create Standard library
    Any aDialogCont;
    aDialogCont <<= xDialogCont;
    xUnoObj = GetSbUnoObject( DEFINE_CONST_UNICODE("DialogLibraries"), aDialogCont );
    pBas->Insert( xUnoObj );


    // Modify-Flag wird bei MakeVariable gesetzt
    pBas->SetModified( bWasModified );
}

//--------------------------------------------------------------------
#if 0 //(mba)
SotObjectRef SfxObjectShell::CreateAggObj( const SotFactory* pFact )
{
    // SvDispatch?
    SotFactory* pDispFact = SvDispatch::ClassFactory();
    if( pFact == pDispFact )
        return( (SfxShellObject*)GetSbxObject() );

    // sonst unbekannte Aggregation
    DBG_ERROR("unkekannte Factory");
    SotObjectRef aSvObjectRef;
    return aSvObjectRef;
}
#endif

//--------------------------------------------------------------------

sal_uInt16 SfxObjectShell::Count()
{
    return SFX_APP()->GetObjectShells_Impl().Count();
}

//--------------------------------------------------------------------

sal_Bool SfxObjectShell::DoClose()
{
    return Close();
}

//--------------------------------------------------------------------

void SfxObjectShell::SetLastMark_Impl( const String &rMark )
{
    pImp->aMark = rMark;
}

//--------------------------------------------------------------------

const String& SfxObjectShell::GetLastMark_Impl() const
{
    return pImp->aMark;
}

//--------------------------------------------------------------------

SfxObjectShell* SfxObjectShell::GetObjectShell()
{
    return this;
}

//--------------------------------------------------------------------

SEQUENCE< OUSTRING > SfxObjectShell::GetEventNames()
{
    return GetEventNames_Impl();
}

SEQUENCE< OUSTRING > SfxObjectShell::GetEventNames_Impl()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    ResStringArray aEventNames( SfxResId( EVENT_NAMES_ARY ) );
    USHORT nCount = aEventNames.Count();

    SEQUENCE < OUSTRING > aSequence( nCount );

    OUSTRING* pNames = aSequence.getArray();

    for ( USHORT i=0; i<nCount; i++ )
    {
        pNames[i] = aEventNames.GetString( i );
    }

    return aSequence;
}

//--------------------------------------------------------------------
/* ASDBG
void SfxObjectShell::SetModel( SfxModel* pModel )
{
    if ( pImp->xModel.is() )
        DBG_WARNING( "Model already set!" );
    pImp->xModel = pModel;
}

//--------------------------------------------------------------------

XModel* SfxObjectShell::GetModel()
{
    return pImp->xModel;
}
*/
//--------------------------------------------------------------------

void SfxObjectShell::SetModel( SfxBaseModel* pModel )
{
    if ( pImp->xModel.is() )
        DBG_WARNING( "Model already set!" );

    pImp->xModel = pModel;
}

//--------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SfxObjectShell::GetModel()
{
    return pImp->xModel;
}

void SfxObjectShell::SetBaseModel( SfxBaseModel* pModel )
{
    if ( pImp->xModel.is() && pModel )
        DBG_WARNING( "Model already set!" );

    pImp->xModel = pModel;
}

//--------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SfxObjectShell::GetBaseModel()
{
    return pImp->xModel;
}
/* -----------------------------10.09.2001 15:56------------------------------

 ---------------------------------------------------------------------------*/
void SfxObjectShell::SetAutoStyleFilterIndex(sal_uInt16 nSet)
{
    pImp->nStyleFilter = nSet;
}

void SfxObjectShell::SetWorkingDocument( SfxObjectShell* pDoc )
{
    pWorkingDoc = pDoc;
    StarBASIC* pBas = SFX_APP()->GetBasic_Impl();
    if ( pDoc && pBas )
    {
        SFX_APP()->Get_Impl()->pThisDocument = pDoc;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xInterface ( pDoc->GetModel() , ::com::sun::star::uno::UNO_QUERY );
        ::com::sun::star::uno::Any aComponent;
        aComponent <<= xInterface;
        SbxVariable *pCompVar = pBas->Find( DEFINE_CONST_UNICODE("ThisComponent"), SbxCLASS_PROPERTY );
        if ( pCompVar )
        {
            pCompVar->PutObject( GetSbUnoObject( DEFINE_CONST_UNICODE("ThisComponent"), aComponent ) );
        }
        else
        {
            SbxObjectRef xUnoObj = GetSbUnoObject( DEFINE_CONST_UNICODE("ThisComponent"), aComponent );
            xUnoObj->SetFlag( SBX_DONTSTORE );
            pBas->Insert( xUnoObj );
        }
    }
}

SfxObjectShell* SfxObjectShell::GetWorkingDocument()
{
    return pWorkingDoc;
}


String SfxObjectShell::GetServiceNameFromFactory( const String& rFact )
{
    //! Remove everything behind name!
    String aFact( rFact );
    String aPrefix = String::CreateFromAscii( "private:factory/" );
    if ( aPrefix.Len() == aFact.Match( aPrefix ) )
        aFact.Erase( 0, aPrefix.Len() );
    USHORT nPos = aFact.Search( '?' );
    String aParam;
    if ( nPos != STRING_NOTFOUND )
    {
        aParam = aFact.Copy( nPos, aFact.Len() );
        aFact.Erase( nPos, aFact.Len() );
        aParam.Erase(0,1);
    }

    aFact.EraseAllChars('4').ToLowerAscii();
    ::rtl::OUString aServiceName;
    if ( aFact.EqualsAscii("swriter") )
    {
        aServiceName = ::rtl::OUString::createFromAscii("com.sun.star.text.TextDocument");
    }
    else if ( aFact.EqualsAscii("swriter/web") )
    {
        aServiceName = ::rtl::OUString::createFromAscii("com.sun.star.text.WebDocument");
    }
    else if ( aFact.EqualsAscii("swriter/globaldocument") )
    {
        aServiceName = ::rtl::OUString::createFromAscii("com.sun.star.text.GlobalDocument");
    }
    else if ( aFact.EqualsAscii("scalc") )
    {
        aServiceName = ::rtl::OUString::createFromAscii("com.sun.star.sheet.SpreadsheetDocument");
    }
    else if ( aFact.EqualsAscii("sdraw") )
    {
        aServiceName = ::rtl::OUString::createFromAscii("com.sun.star.drawing.DrawingDocument");
    }
    else if ( aFact.EqualsAscii("simpress") )
    {
        aServiceName = ::rtl::OUString::createFromAscii("com.sun.star.presentation.PresentationDocument");
    }
    else if ( aFact.EqualsAscii("schart") )
    {
        aServiceName = ::rtl::OUString::createFromAscii("com.sun.star.chart.ChartDocument");
    }
    else if ( aFact.EqualsAscii("smath") )
    {
        aServiceName = ::rtl::OUString::createFromAscii("com.sun.star.formula.FormulaProperties");
    }

    return aServiceName;
}

SfxObjectShell* SfxObjectShell::CreateObjectByFactoryName( const String& rFact, SfxObjectCreateMode eMode )
{
    return CreateObject( GetServiceNameFromFactory( rFact ), eMode );
}


SfxObjectShell* SfxObjectShell::CreateObject( const String& rServiceName, SfxObjectCreateMode eMode )
{
    if ( rServiceName.Len() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::frame::XModel > xDoc(
            ::comphelper::getProcessServiceFactory()->createInstance( rServiceName ), UNO_QUERY );
        if ( xDoc.is() )
        {
            ::com::sun::star::uno::Reference < ::com::sun::star::lang::XUnoTunnel > xObj( xDoc, UNO_QUERY );
            ::com::sun::star::uno::Sequence < sal_Int8 > aSeq( (sal_Int8*) SvGlobalName( SFX_GLOBAL_CLASSID ).GetBytes(), 16 );
            sal_Int64 nHandle = xObj->getSomething( aSeq );
            if ( nHandle )
                return (SfxObjectShell*) (sal_Int32*) nHandle;
        }
    }

    return 0;
}

