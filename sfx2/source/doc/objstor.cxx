/*************************************************************************
 *
 *  $RCSfile: objstor.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: mba $ $Date: 2001-03-09 10:21:54 $
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

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _EXTATTR_HXX //autogen
#include <svtools/extattr.hxx>
#endif
#ifndef _ZCODEC_HXX //autogen
#include <tools/zcodec.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_DATATRANSFEREVENT_HPP_
#include <com/sun/star/io/DataTransferEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XDATATRANSFEREVENTLISTENER_HPP_
#include <com/sun/star/io/XDataTransferEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XDATAEXPORTER_HPP_
#include <com/sun/star/io/XDataExporter.hpp>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XIMPORTER_HPP_
#include <com/sun/star/document/XImporter.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif
#pragma hdrstop

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif
#ifndef _CACHESTR_HXX
#include <tools/cachestr.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_ADDXMLTOSTORAGEOPTIONS_HXX
#include <svtools/addxmltostorageoptions.hxx>
#endif

#include <svtools/saveopt.hxx>
#include <svtools/useroptions.hxx>
#include <svtools/pathoptions.hxx>
#include <tools/urlobj.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/tempfile.hxx>

#include "objsh.hxx"
#include "childwin.hxx"
#if SUPD<613//MUSTINI
#include "inimgr.hxx"
#endif
#include "sfxdir.hxx"
#include "request.hxx"
#include "sfxresid.hxx"
#include "iodlg.hxx"
#include "docfile.hxx"
#include "fltfnc.hxx"
#include "docfilt.hxx"
#include "docinf.hxx"
#include "docfac.hxx"
#include "cfgmgr.hxx"
#include "objshimp.hxx"
#include "sfxtypes.hxx"
#include "appdata.hxx"
#include "doc.hrc"
#include "sfxsids.hrc"
#include "interno.hxx"
#include "module.hxx"
#include "dispatch.hxx"
#include "openflag.hxx"
#include "helper.hxx"


#define S2BS(s) ByteString( s, RTL_TEXTENCODING_MS_1252 )

class DataTransferEventListener_Impl    :   public ::com::sun::star::io::XDataTransferEventListener ,
                                            public ::com::sun::star::lang::XTypeProvider            ,
                                            public ::cppu::OWeakObject
{
private:
    SfxObjectShellRef   xDoc;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >       xTransfer;

public:
    SFX_DECL_XINTERFACE_XTYPEPROVIDER

                        DataTransferEventListener_Impl( SfxObjectShell *pDoc, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & xRef )
                            : xDoc( pDoc )
                            , xTransfer( xRef )
                        {}

                        ~DataTransferEventListener_Impl();

    void                Finish();
    virtual void   SAL_CALL         finished(const ::com::sun::star::io::DataTransferEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException );
    virtual void   SAL_CALL         cancelled(const ::com::sun::star::io::DataTransferEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException );
    virtual void   SAL_CALL         disposing(const ::com::sun::star::lang::EventObject &) throw( ::com::sun::star::uno::RuntimeException );
};

SFX_IMPL_XINTERFACE_2( DataTransferEventListener_Impl, OWeakObject, ::com::sun::star::io::XDataTransferEventListener, ::com::sun::star::lang::XEventListener )
SFX_IMPL_XTYPEPROVIDER_2( DataTransferEventListener_Impl, ::com::sun::star::io::XDataTransferEventListener, ::com::sun::star::lang::XEventListener )

void DataTransferEventListener_Impl::Finish()
{
    if ( xDoc.Is() )
    {
        if ( !xDoc->Get_Impl()->nLoadedFlags )
            xDoc->FinishedLoading();
        xDoc.Clear();
    }

    xTransfer = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > ();
}

DataTransferEventListener_Impl::~DataTransferEventListener_Impl()
{
    Finish();
}

void SAL_CALL  DataTransferEventListener_Impl::disposing(const ::com::sun::star::lang::EventObject &) throw( ::com::sun::star::uno::RuntimeException )
{
    Finish();
}

void SAL_CALL  DataTransferEventListener_Impl::finished(const ::com::sun::star::io::DataTransferEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException )
{
    Finish();
}

void SAL_CALL  DataTransferEventListener_Impl::cancelled(const ::com::sun::star::io::DataTransferEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException )
{
    Finish();
}

extern sal_uInt32 CheckPasswd_Impl( Window*, SfxItemPool&, SfxMedium* );

//=========================================================================

sal_Bool ShallSetBaseURL_Impl( SfxMedium &rMed )
{
    SvtSaveOptions aOpt;
    sal_Bool bIsRemote = rMed.IsRemote();
    return  aOpt.IsSaveRelINet() && bIsRemote || aOpt.IsSaveRelFSys() && !bIsRemote;
}

//=========================================================================

sal_Bool SfxObjectShell::Save()
{
    return SaveInfoAndConfig_Impl( GetMedium()->GetStorage() );
}

//--------------------------------------------------------------------------

sal_Bool SfxObjectShell::SaveAs( SvStorage* pNewStg )
{
    return SaveInfoAndConfig_Impl( pNewStg );
}

//-------------------------------------------------------------------------

sal_Bool GetPasswd_Impl( const SfxItemSet* pSet, String& rPasswd )
{
    const SfxPoolItem* pItem = NULL;
    if ( pSet && SFX_ITEM_SET == pSet->GetItemState( SID_PASSWORD, sal_True, &pItem ) )
    {
        DBG_ASSERT( pItem->IsA( TYPE(SfxStringItem) ), "wrong item type" );
        rPasswd = ( (const SfxStringItem*)pItem )->GetValue();
        return sal_True;
    }
    return sal_False;
}

//-------------------------------------------------------------------------
sal_Bool SfxObjectShell::DoInitNew_Impl( const String& rName )

/*  [Beschreibung]
*/

{
    if ( rName.Len() )
    {
        ModifyBlocker_Impl aBlock( this );
        pMedium = new SfxMedium( rName, SFX_STREAM_READONLY_MAKECOPY, sal_False );
        if ( InitNew( pMedium->GetStorage() ) )
        {
            bIsTmp = !( pMedium->GetStorage() );
            if ( SFX_CREATE_MODE_EMBEDDED == eCreateMode )
                SetTitle( String( SfxResId( STR_NONAME ) ));
            return sal_True;
        }
        return sal_False;
    }
    else
        return DoInitNew(0);
}


sal_Bool SfxObjectShell::DoInitNew( SvStorage * pStor )

/*  [Beschreibung]

    Diese von SvPersist geerbte virtuelle Methode wird gerufen, um
    die SfxObjectShell-Instanz aus einem Storage (pStor != 0) bzw.
    (pStor == 0) ganz neu zu initialisieren.

    Wie alle Do...-Methoden liegt hier eine Steuerung vor, die eigentliche
    Implementierung erfolgt, indem die ebenfalls virtuellen Methode
    InitNew(SvStorate*) von der SfxObjectShell-Subclass implementiert wird.

    F"ur pStor == 0 wird ein die SfxObjectShell-Instanz mit einem leeren
    SfxMedium verbunden, sonst mit einem SfxMedium, welches auf den
    als Parameter "ubergeben SvStorage verweist.

    Erst nach InitNew() oder Load() ist das Objekt korrekt initialisiert.

    [R"uckgabewert]
    sal_True            Das Objekt wurde initialisiert.
    sal_False           Das Objekt konnte nicht initialisiert werden
*/

{
    ModifyBlocker_Impl aBlock( this );
    if ( pStor )
        pMedium = new SfxMedium( pStor );
    else
    {
        bIsTmp = sal_True;
        pMedium = new SfxMedium;
    }

    if ( InitNew( pStor ) )
    {
        if ( SFX_CREATE_MODE_EMBEDDED == eCreateMode )
            SetTitle( String( SfxResId( STR_NONAME ) ));
        return sal_True;
    }
    return sal_False;
}

//-------------------------------------------------------------------------

void SfxObjectShell::DoHandsOff()

/*  [Beschreibung]

    Diese von SvPersist geerbte virtuelle Methode wird gerufen, um
    das Objekt aufzufordern, den ihm zugeteilten SvStorage freizugeben,
    insbesondere Substorages und Streams zu schlie"sen.

    Als Do...-Methode liegt hier nur die Steuerung. Der Implementierer
    von Subclasses kann die ebenfalls virtuelle Methode HandsOff()
    implementieren, um seine Substorages und Streams zu schlie"sen.

    Nach dem Aufruf dieser Methode, ist dem Objekt kein SfxMedium mehr
    zugeordnet, bis SaveCompleted() durchlaufen ist.
*/

{
    const SfxFilter *pFilter = pMedium->GetFilter();
    if( !pFilter || pFilter->IsOwnFormat() || ( pFilter->GetFilterFlags() & SFX_FILTER_PACKED ) )
        HandsOff();
    pMedium->Close();
//  DELETEZ( pMedium );
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::DoLoad(
    const String& rFileName, StreamMode nStreamMode, StorageMode nStorageMode)
{
    // Es wird nur die IPersistStorage-Schnittstelle angeboten
    ModifyBlocker_Impl aBlock( this );
    SvStorageRef xStor = new SvStorage( rFileName, nStreamMode | STREAM_WRITE, nStorageMode );
    if( !xStor.Is() )
        xStor = new SvStorage( rFileName, nStreamMode, nStorageMode );

    if ( SVSTREAM_OK == xStor->GetError() )
    {
        SfxMedium* pMedium = new SfxMedium( xStor );
        pMedium->SetName( rFileName );
        pMedium->Init_Impl();

        // Muss !!!
        SetFileName( rFileName );

        if( DoLoad( pMedium ) )
        {
            if ( SFX_CREATE_MODE_EMBEDDED == eCreateMode )
                SetTitle( rFileName );
            return sal_True;
        }
    }
    return sal_False;
}


//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::DoLoad( SvStorage * pStor )

/*  [Beschreibung]

    Diese von SvPersist geerbte virtuelle Methode steuert das Laden
    des Objektes aus einem Storage. Dabei wird der SvStorage zun"achst
    in einem SfxMedium verpackt und SfxObjectShell::DoLoad(SfxMedium*)
    mit diesem gerufen.

    [R"uckgabewert]
    sal_True                Das Objekt wurde initialisiert.
    sal_False               Das Objekt konnte nicht initialisiert werden
*/

{
    pMedium = new SfxMedium( pStor );
    if ( DoLoad(pMedium) )
    {
        if ( SFX_CREATE_MODE_EMBEDDED == eCreateMode )
        {
            ModifyBlocker_Impl aBlock( this );
            // bei Embedded Objekten setzt sonst keiner den Namen
//            DBG_ASSERT( pStor->GetName().Len(),
//                        "StorageName hat Laenge Null" );
            SetTitle( pStor->GetName() );
        }
        return sal_True;
    }

    return sal_False;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::DoLoad( SfxMedium *pMed )

/*  [Beschreibung]

    Diese Methode steuert das Laden des Objektes aus dem von dem
    "ubergebenen SfxMedium beschriebenen Medium. Hinterher ist das Objekt
    mit diesem SfxMedium verbunden.

    Handelt es sich bei dem SfxMedium um einen Storage im eigenen Format,
    wird die virtuelle Methode SvPersit::Load(SvStorage*) gerufen, welche
    die Implementierer von Subclasses "uberladen m"ussen, um das Objekt
    aus einem eigenen Storage zu laden (=> Swapping m"oeglich).

    Handelt es sich bei dem SfxMedium um einen Storage in einem fremden
    Format, oder um ein Flat-File, dann wird die virtuelle Methode
    <SfxObjectShell::ConvertFrom(SfxMedium*)> gerufen, welche die
    Implementierer von Subclasses "uberladen m"ussen, um das Objekt
    aus dem SfxMedium zu konvertieren. W"ahrend der Bearbeitung ist
    das Objekt dann mit einem tempor"aren SvStorage verbunden.

    Erst nach InitNew() oder Load() ist das Objekt korrekt
    initialisiert.

    [R"uckgabewert]
    sal_True                Das Objekt wurde geladen.
    sal_False           Das Objekt konnte nicht geladen werden
*/

{
    SfxApplication *pSfxApp = SFX_APP();
    ModifyBlocker_Impl aBlock( this );

    if ( SFX_CREATE_MODE_EMBEDDED != eCreateMode )
        GetpApp()->ShowStatusText( SfxResId(STR_DOC_LOADING) );

    pMedium = pMed;
    sal_Bool bOk = sal_False;
    const SfxFilter* pFilter = pMed->GetFilter();
    SfxItemSet* pSet = pMedium->GetItemSet();
    if( !pImp->nEventId )
    {
        SFX_ITEMSET_ARG(
            pSet, pTemplateItem, SfxBoolItem,
            SID_TEMPLATE, sal_False);
        SFX_ITEMSET_ARG(
            pSet, pBrowsingItem, SfxBoolItem, SID_BROWSING, sal_False );
        SetActivateEvent_Impl(
            ( pTemplateItem && pTemplateItem->GetValue() )
            ? SFX_EVENT_CREATEDOC : SFX_EVENT_OPENDOC,
            pBrowsingItem && pBrowsingItem->GetValue() );
    }


    SFX_ITEMSET_ARG( pSet, pBaseItem, SfxStringItem,
                     SID_BASEURL, sal_False);
    String aBaseURL;
    if( pBaseItem ) aBaseURL = pBaseItem->GetValue();
    else
    {
        SFX_ITEMSET_ARG( pMedium->GetItemSet(), pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False);
        if( GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
        {
            aBaseURL = INetURLObject::GetBaseURL();
            SetBaseURL( aBaseURL );
        }
        else if ( pSalvageItem )
        {
            String aName( pMed->GetPhysicalName() );
            ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aBaseURL );
        }
        else
            aBaseURL = pMed->GetName();
    }

    SfxApplication* pApp = SFX_APP();
    pImp->nLoadedFlags = 0;
    sal_Bool bHasStorage = !pFilter || ( pFilter->IsOwnFormat() && pFilter->UsesStorage() );
    if ( !bHasStorage && pFilter && ( pFilter->GetFilterFlags() & SFX_FILTER_PACKED ) )
    {
        bHasStorage = pMed->TryStorage();
        if ( bHasStorage )
        {
            String aName( pMed->GetPhysicalName() );
            ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aBaseURL );
        }
    }

    if ( bHasStorage )
    {
        SvStorageRef xStor( pMed->GetStorage() );
        // Undoobjekte aufraeumen, muss vor dem eigentlichen Laden erfolgen
        if(xStor.Is() && !xStor->GetError() )
        {
            SvEmbeddedObjectRef xThis = this;
            SvPersistRef xPer;
            if ( xThis.Is() )
                xPer = new SvEmbeddedObject;
            else
                xPer = new SvPersist;

            xPer->DoOwnerLoad(xStor);
            xPer->CleanUp();
            xPer->DoSave();
            xPer->DoSaveCompleted( 0 );
        }

        // Load
        const String aOldURL( INetURLObject::GetBaseURL() );
        if( aBaseURL.Len() ) INetURLObject::SetBaseURL( aBaseURL );
        pImp->nLoadedFlags = 0;
        bOk = xStor.Is() && LoadOwnFormat( *pMed );
        INetURLObject::SetBaseURL( aOldURL );

        if ( bOk )
        {
            GetDocInfo().Load(xStor);
            bHasName = sal_True;
        }
        else
            SetError( ERRCODE_ABORT );
    }
    else if ( InitNew(0) )
    {
        // Name vor ConvertFrom setzen, damit GetSbxObject() schon funktioniert
        bHasName = sal_True;
        SetName( SfxResId( STR_NONAME ) );

        // Importieren
        const String aOldURL( INetURLObject::GetBaseURL() );
        if( aBaseURL.Len() ) INetURLObject::SetBaseURL( aBaseURL );
        if( !pMedium->GetFilter()->UsesStorage() )
            pMedium->GetInStream();
        else
            pMedium->GetStorage();

        pImp->nLoadedFlags = 0;
        if ( pMedium->GetFilter() &&  ( pMedium->GetFilter()->GetFilterFlags() & SFX_FILTER_STARONEFILTER ) )
        {
            bOk = ImportFrom(*pMedium);
        }
        else
        {
            bOk = ConvertFrom(*pMedium);
        }

        INetURLObject::SetBaseURL( aOldURL );

        if( bOk && pMedium->GetOpenMode() & STREAM_WRITE )
        //Medium offen halten um andere Zugriffe zu verhindern
        {
            if(pMedium->GetFilter() && pMedium->GetFilter()->UsesStorage())
                pMedium->GetStorage();
            else
                pMedium->GetInStream();
            if(pMedium->GetError())
                bOk = sal_False;
        }
    }

    if ( bOk )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >  xModel ( GetModel(), ::com::sun::star::uno::UNO_QUERY );
        if ( xModel.is() )
        {
            ::rtl::OUString aURL = GetMedium()->GetOrigURL();
            SfxItemSet *pSet = GetMedium()->GetItemSet();
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs;
            TransformItems( SID_OPENDOC, *pSet, aArgs );
            xModel->attachResource( aURL, aArgs );
        }

        GetTitle( SFX_TITLE_DETECT );

        // Falls nicht asynchron geladen wird selbst FinishedLoading aufrufen
        if ( !( pImp->nLoadedFlags & SFX_LOADED_MAINDOCUMENT ) &&
            ( !pMedium->GetFilter() ||
             pMedium->GetFilter()->UsesStorage() ||
             pMedium->GetInStream() && pMedium->GetInStream()->GetLockBytes() &&
             pMedium->GetInStream()->GetLockBytes()->IsSynchronMode() ) )
            FinishedLoading( SFX_LOADED_MAINDOCUMENT );

        if( IsOwnStorageFormat_Impl(*pMed) && pMed->GetFilter() )
        {
//???? dv           DirEntry aDirEntry( pMed->GetPhysicalName() );
//???? dv           SetFileName( aDirEntry.GetFull() );
        }
        Broadcast( SfxSimpleHint(SFX_HINT_NAMECHANGED) );
    }

    if( bOk )
    {
        String aFacName = String::CreateFromAscii( GetFactory().GetShortName() );
        if( ! aFacName.EqualsAscii( "swriter" ) &&
            ! aFacName.EqualsAscii( "FrameSet" ) &&
            ! aFacName.EqualsAscii( "swriter/web" ) )
        {
#ifdef DBG_UTIL
            if( pImp->nLoadedFlags != SFX_LOADED_ALL )
            {
                ByteString aError( U2S( aFacName ) );
                aError += " hat uralte Mussaenderung nicht gemacht. TLX fragen";
                DBG_ERROR( aError.GetBuffer() );
            }
#endif
            FinishedLoading( SFX_LOADED_ALL );
        }
    }

    if ( SFX_CREATE_MODE_EMBEDDED != eCreateMode )
        GetpApp()->HideStatusText();
    return bOk;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsOwnStorageFormat_Impl(const SfxMedium &rMedium) const
{
    return !rMedium.GetFilter() || // Embedded
           ( rMedium.GetFilter()->IsOwnFormat() &&
             rMedium.GetFilter()->UsesStorage() );
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::DoSave()
// DoSave wird nur noch ueber OLE aufgerufen. Sichern eigener Dokument
// laeuft uber SaveAs, um das Anlegen von Backups zu ermoeglichen.
// Save in eigenes Format jetzt auch wieder Hierueber
{
    sal_Bool bOk = sal_False ;
    {
        ModifyBlocker_Impl aBlock( this );
        SfxForceLinkTimer_Impl aFLT( this );
        pImp->bIsSaving = sal_True;
        String aPasswd;
        if ( IsOwnStorageFormat_Impl( *GetMedium() ) &&
             GetPasswd_Impl( GetMedium()->GetItemSet(), aPasswd ) )
            GetMedium()->GetStorage()->SetKey( S2BS( aPasswd ) );   //!!! (pb) needs new implementation
        GetStorage()->SetVersion( GetMedium()->GetFilter()->GetVersion() );
        bOk = Save();
    }

    if ( bOk )
        SetModified( sal_False );
    return bOk;
}

void Lock_Impl( SfxObjectShell* pDoc, BOOL bLock )
{
    SfxViewFrame *pFrame= SfxViewFrame::GetFirst( pDoc );
    while ( pFrame )
    {
        pFrame->GetDispatcher()->Lock( bLock );
        pFrame->Enable( !bLock );
        pFrame = SfxViewFrame::GetNext( *pFrame, pDoc );
    }

}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::SaveTo_Impl
(
     SfxMedium &rMedium, // Medium, in das gespeichert werden soll
     const SfxItemSet* pSet
)

/*  [Beschreibung]

    Schreibt den aktuellen Inhalt in das Medium rMedium.
    Ist das Zielmedium kein Storage, so wird ueber ein temporaeres
    Medium gespeichert, sonst direkt, da das Medium transacted
    geschaltet ist, wenn wir es selbst geoeffnet haben und falls wir
    Server sind entweder der Container einen transacted Storage zur
    Verfuegung stellt oder selbst einen temporaeren Storage erzeugt hat.
*/

{
    sal_Bool bOk = sal_False;
    SfxForceLinkTimer_Impl aFLT( this );
    ModifyBlocker_Impl aBlock( this );
    const SfxFilter *pFilter = rMedium.GetFilter();
    if ( !pFilter )
    {
        pFilter = GetFactory().GetFilter(0);
        rMedium.SetFilter(pFilter);
    }
    sal_Bool bStorage = pFilter->UsesStorage();
    if( bStorage )
        rMedium.GetOutputStorage( SOFFICE_FILEFORMAT_60 <= pFilter->GetVersion() );
    else
        rMedium.GetOutStream();
    if( rMedium.GetErrorCode() )
        return sal_False;

#ifdef DBG_UTILx
    SvStorageRef xRef;
    if( bStorage )
    {
        xRef = rMedium.GetStorage();
        if( xRef.Is() )
            xRef->EnableRootCommit( sal_False );
    }
#endif

    sal_Bool bOldStat = pImp->bForbidReload;
    pImp->bForbidReload = sal_True;

    Lock_Impl( this, TRUE );
    if(IsOwnStorageFormat_Impl(rMedium))
    {
        SvStorageRef aMedRef = rMedium.GetStorage();
        if ( !aMedRef.Is() )
        {
            Lock_Impl( this, FALSE );
            return sal_False;
        }

        String aPasswd;
        if ( GetPasswd_Impl( rMedium.GetItemSet(), aPasswd ) )
            aMedRef->SetKey( S2BS( aPasswd ) ); //!!! (pb) needs new implementation

        // Speichern
        const SfxFilter* pFilter = rMedium.GetFilter();
        if(  ((SvStorage *)aMedRef) == ((SvStorage *)GetStorage() ) )
        {
            aMedRef->SetVersion( pFilter->GetVersion() );
            bOk = Save();
        }
        else
            bOk = SaveAsOwnFormat( rMedium );

        // Soll als Version gespeichert werden ?

        const SfxStringItem *pVersionItem = pSet ? (const SfxStringItem*)
            SfxRequest::GetItem( pSet, SID_VERSION, sal_False, TYPE(SfxStringItem) ) : NULL;

        const SfxStringItem *pAuthorItem = pSet ? (const SfxStringItem*)
            SfxRequest::GetItem( pSet, SID_DOCINFO_AUTHOR, sal_False, TYPE(SfxStringItem) ) : NULL;

        if ( pVersionItem )
        {
            // Versionskommentar und Author der Version
            SfxVersionInfo aInfo;
            aInfo.aComment = pVersionItem->GetValue();
            String aAuthor;
            if ( pAuthorItem )
                aAuthor = pAuthorItem->GetValue();
            else
                aAuthor = SvtUserOptions().GetFullName();

            aInfo.aCreateStamp.SetName( aAuthor );

            sal_Bool bUseXML = SOFFICE_FILEFORMAT_60 <= pFilter->GetVersion();

            // Den Storage f"ur die Versionen "offnen
            SvStorageRef xVersion = bUseXML ?
                    aMedRef->OpenUCBStorage( DEFINE_CONST_UNICODE( "Versions" ) ) :
                    aMedRef->OpenStorage( DEFINE_CONST_UNICODE( "Versions" ) );

            // Ggf. alle schon vorhandenen Versionen kopieren
            SvStorageRef xOldVersions = GetStorage()->OpenStorage( DEFINE_CONST_UNICODE( "Versions" ), SFX_STREAM_READONLY | STREAM_NOCREATE);
            if ( xOldVersions.Is() && xOldVersions->GetError() == SVSTREAM_OK )
            {
                const SfxVersionTableDtor *pList = rMedium.GetVersionList();
                sal_uInt32 n=0;
                SfxVersionInfo* pInfo = pList->GetObject(n++);
                while( pInfo )
                {
                    const String& rName = pInfo->aName;
                    if ( xOldVersions->IsContained( rName ) )
                        xOldVersions->CopyTo( rName, xVersion, rName );
                    pInfo = pList->GetObject(n++);
                }
            }

            // Version in die Liste aufnehmen; diese mu\s vorher schon vom
            // "alten" Medium "ubertragen worden sein
            rMedium.AddVersion_Impl( aInfo );
            rMedium.SaveVersionList_Impl( bUseXML );

            // Einen Stream aufmachen, auf den dann der Storage gesetzt wird,
            // in den gespeichert wird
//            SvMemoryStream aTmp;

            ::utl::TempFile aTmpFile;
            aTmpFile.EnableKillingFile( TRUE );
            SvStorageRef xTmp = new SvStorage( ( SOFFICE_FILEFORMAT_60 <= pFilter->GetVersion() ), aTmpFile.GetURL() );
            rMedium.SetStorage_Impl( xTmp );

            // Version speichern
            if ( aPasswd.Len() )
                xTmp->SetKey( S2BS( aPasswd ) ); //!!! (pb) needs new implementation
            if( ((SvStorage*) xTmp ) == ((SvStorage*) GetStorage()) )
            {
                xTmp->SetVersion( pFilter->GetVersion() );
                bOk = Save();
            }
            else
                bOk = SaveAsOwnFormat( rMedium );

            xTmp->Commit();

            // Medium wieder auf den alten Storage setzen
            rMedium.SetStorage_Impl( aMedRef );

            // storage freigeben, um ihn als stream zu öffnen
            xTmp.Clear();

            // Den Stream mit dem Storage komprimiert abspeichern
            SvStorageStreamRef xStrm = xVersion->OpenStream( aInfo.aName );

            if ( SOFFICE_FILEFORMAT_60 <= pFilter->GetVersion() )
            {
                *xStrm << *aTmpFile.GetStream( STREAM_READ );
            }
            else
            {
                ZCodec aCodec;
                aCodec.BeginCompression( ZCODEC_BEST_COMPRESSION );
                aCodec.Compress( *aTmpFile.GetStream( STREAM_READ ), *xStrm );
                aCodec.EndCompression();
            }

            // Versionen-Storage committen
            xVersion->Commit();
        }
        else if ( pImp->bIsSaving )
        {
            sal_Bool bUseXML = SOFFICE_FILEFORMAT_60 <= pFilter->GetVersion();

            rMedium.SaveVersionList_Impl( bUseXML );
            const SfxVersionTableDtor *pList = rMedium.GetVersionList();
            if ( pList && pList->Count() )
            {
                SvStorageRef xVersion = bUseXML ?
                    aMedRef->OpenUCBStorage( DEFINE_CONST_UNICODE( "Versions" ) ) :
                    aMedRef->OpenStorage( DEFINE_CONST_UNICODE( "Versions" ) );
                SvStorageRef xOldVersions = GetStorage()->OpenStorage( DEFINE_CONST_UNICODE( "Versions" ), SFX_STREAM_READONLY | STREAM_NOCREATE );
                if ( xOldVersions.Is() && xOldVersions->GetError() == SVSTREAM_OK )
                {
                    sal_uInt32 n=0;
                    SfxVersionInfo* pInfo = pList->GetObject(n++);
                    while( pInfo )
                    {
                        const String& rName = pInfo->aName;
                        if ( xOldVersions->IsContained( rName ) )
                            xOldVersions->CopyTo( rName, xVersion, rName );
                        pInfo = pList->GetObject(n++);
                    }
                }

                xVersion->Commit();
            }
        }
    }
    else
    {
        if ( rMedium.GetFilter() && ( rMedium.GetFilter()->GetFilterFlags() & SFX_FILTER_STARONEFILTER ) )
            bOk = ExportTo( rMedium, *pSet );
        else
            bOk = ConvertTo( rMedium );

        if( bOk )
            bOk = SaveChilds();
    }


#ifdef DBG_UTILx
    if( bStorage )
    {
        if( xRef.Is() )
            xRef->EnableRootCommit( sal_True );
    }
#endif

    if(bOk)
    {
        /*  When the new medium ( rMedium ) has the same name as the
            current one, we need to call DoHandsOff() so Commit() can
            overwrite the old version. This is a good time to check
            wether we want a backup copy, too.
            (dv) We have to call DoHandsOff wether or not the names
            are the same
        */

        sal_Bool bCopyTo = sal_False;
        SfxItemSet *pSet = rMedium.GetItemSet();

        if( pSet )
        {
            SFX_ITEMSET_ARG( pSet, pSaveToItem, SfxBoolItem, SID_SAVETO, sal_False );
            bCopyTo =   GetCreateMode() == SFX_CREATE_MODE_EMBEDDED ||
                        pSaveToItem && pSaveToItem->GetValue();
        }

        if ( ! bCopyTo )
            DoHandsOff();

        if ( pMedium &&
             ( rMedium.GetName() == pMedium->GetName() ) )
        {
            const sal_Bool bDoBackup = SvtSaveOptions().IsBackup();
            if ( bDoBackup )
                pMedium->DoBackup_Impl();
        }

        RegisterTransfer( rMedium );
        bOk=rMedium.Commit();
    }
    else
    {
        Lock_Impl( this, FALSE );
        return sal_False;
    }

    Lock_Impl( this, FALSE );
    pImp->bForbidReload = bOldStat;

    if(bOk && pFilter)
        if(pFilter->IsAlienFormat())
            pImp->bDidDangerousSave=sal_True;
        else
            pImp->bDidDangerousSave=sal_False;

    if ( bOk )
        SetEAs_Impl(rMedium);

    return bOk;
}

//------------------------------------------------------------------------

sal_Bool SfxObjectShell::ConnectTmpStorage_Impl( SvStorage* pStg)

/*   [Beschreibung]

     Arbeitet die Applikation auf einem temporaeren Storage,
     so darf der temporaere Storage nicht aus dem SaveCompleted
     genommen werden. Daher wird in diesem Fall schon hier an
     den neuen Storage connected. SaveCompleted tut dann nichts.

     */

{
    // wenn es kein temp. Storage ist, einen anlegen
    SvStorageRef aTmpMed = new SvStorage( (pStg->GetVersion() >= SOFFICE_FILEFORMAT_60), String() );

    // nach HandsOff muss der alte Storage wieder eingesetzt werden
    if ( !pStg->CopyTo(aTmpMed) )
    {
        SetError(aTmpMed->GetErrorCode());
        aTmpMed.Clear();
        return sal_False;
    }

    SetError(GetMedium()->GetErrorCode());
    SaveCompleted(aTmpMed); // neuer temp. Storage; gibt alten frei
    return sal_True;
}


//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::DoSaveAs( SvStorage * pNewStor )
{
    sal_Bool bOk;
    {
        SfxForceLinkTimer_Impl aFLT( this );
        ModifyBlocker_Impl aBlock( this );
        //Abwehr gegen feindlich gesinnte Applikationen.
        if ( !pNewStor->GetFormat() )
            SetupStorage( pNewStor );

        pImp->bIsSaving = sal_False;
        SfxMedium* pNewMed = new SfxMedium( pNewStor );
        const String aOldURL( INetURLObject::GetBaseURL() );

        bOk = SaveAsOwnFormat( *pNewMed );
        INetURLObject::SetBaseURL( aOldURL );
        delete pNewMed;
    }
    return bOk;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::DoSaveAs( SfxMedium &rMedium )
{
    // hier kommen nur Root-Storages rein, die via Temp-File gespeichert werden
    const String aOldURL( INetURLObject::GetBaseURL() );
    if( GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
        if ( ShallSetBaseURL_Impl( rMedium ) )
            INetURLObject::SetBaseURL( rMedium.GetName() );
        else
            INetURLObject::SetBaseURL( String() );

    rMedium.CreateTempFile();

    sal_Bool bRet = SaveTo_Impl(rMedium);
    INetURLObject::SetBaseURL( aOldURL );
    if( bRet )
        DoHandsOff();
    else
        SetError(rMedium.GetErrorCode());
    return bRet;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::DoSaveCompleted( SfxMedium * pNewMed )
{
    sal_Bool bOk = sal_True;
    sal_Bool bMedChanged = pNewMed && pNewMed!=pMedium;
/*  sal_Bool bCreatedTempStor = pNewMed && pMedium &&
        IsOwnStorageFormat_Impl(*pMedium) &&
        !IsOwnStorageFormat_Impl(*pNewMed) &&
        pMedium->GetName().Len();
*/
    DBG_ASSERT( !pNewMed || pNewMed->GetError() == ERRCODE_NONE, "DoSaveCompleted: Medium has error!" );
    if ( bMedChanged )
    {
        delete pMedium;
        pMedium = pNewMed;
        Broadcast( SfxSimpleHint(SFX_HINT_MODECHANGED) );
        //MI? DELETEZ(pImp->pDocInfo);
    }

    const SfxFilter *pFilter = pMedium ? pMedium->GetFilter() : 0;
    if ( pNewMed )
    {
        if( bMedChanged )
        {
            if( pNewMed->GetName().Len() )
                bHasName = sal_True;
            String aBase = GetBaseURL();
            if( Current() == this && aBase.Len() )
                INetURLObject::SetBaseURL( aBase );
            Broadcast( SfxSimpleHint(SFX_HINT_NAMECHANGED) );
        }
        if ( !pFilter || pFilter->IsOwnFormat())
            bOk = SaveCompleted( pMedium->GetStorage() );
        else if( pFilter->UsesStorage() )
            pMedium->GetStorage();
        else if( pMedium->GetOpenMode() & STREAM_WRITE )
            pMedium->GetInStream();
    }
    else
    {
        if( pMedium )
        {
            const SfxFilter* pFilter = pMedium->GetFilter();
            if( pFilter && !pFilter->IsOwnFormat() &&
                (pMedium->GetOpenMode() & STREAM_WRITE ))
                pMedium->ReOpen();
            else
                SaveCompleted( 0 );
        }
        // entweder Save oder ConvertTo
        else
            bOk = SaveCompleted( NULL );
    }

    if ( bOk && pNewMed )
    {
        if( bMedChanged )
        {
            // Titel neu setzen
            if ( pNewMed->GetName().Len() &&
                 SFX_CREATE_MODE_EMBEDDED != eCreateMode )
                InvalidateName();
            SetModified(sal_False); // nur bei gesetztem Medium zur"ucksetzen
        }
    }

    return bOk;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::DoSaveCompleted( SvStorage * pNewStor )
{
    return DoSaveCompleted(pNewStor? new SfxMedium( pNewStor ): 0);
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::ConvertFrom
(
    SfxMedium&  rMedium     /*  <SfxMedium>, welches die Quell-Datei beschreibt
                                (z.B. Dateiname, <SfxFilter>, Open-Modi etc.) */
)

/*  [Beschreibung]

    Diese Methode wird zum Laden von Dokumenten "uber alle Filter gerufen,
    die nicht SFX_FILTER_OWN sind oder f"ur die kein Clipboard-Format
    registriert wurde (also kein Storage-Format benutzen). Mit anderen Worten:
    mit dieser Methode wird importiert.

    Das hier zu "offende File sollte "uber 'rMedium' ge"offnet werden,
    um die richtigen Open-Modi zu gew"ahrleisten. Insbesondere wenn das
    Format beibehalten wird (nur m"oglich bei SFX_FILTER_SIMULATE oder
    SFX_FILTER_ONW) mu\s die Datei STREAM_SHARE_DENYWRITE ge"offnet werden.


    [R"uckgabewert]

    sal_Bool                sal_True
                        Das Dokument konnte geladen werden.

                        sal_False
                        Das Dokument konnte nicht geladen werden, ein
                        Fehlercode ist mit <SvMedium::GetError()const> zu
                        erhalten.


    [Beispiel]

    sal_Bool DocSh::ConvertFrom( SfxMedium &rMedium )
    {
        SvStreamRef xStream = rMedium.GetInStream();
        if( xStream.is() )
        {
            xStream->SetBufferSize(4096);
            *xStream >> ...;

            // NICHT 'rMedium.CloseInStream()' rufen! File gelockt halten!
            return SVSTREAM_OK == rMedium.GetError();
        }

        return sal_False;
    }


    [Querverweise]

    <SfxObjectShell::ConvertTo(SfxMedium&)>
    <SFX_FILTER_REGISTRATION>
*/
{
    return sal_False;
}

sal_Bool SfxObjectShell::ImportFrom( SfxMedium&  rMedium )
{
    String aName( GetMedium()->GetFilter()->GetFilterName() );
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xMan = ::comphelper::getProcessServiceFactory();
    ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > xFilters (
                xMan->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.FilterFactory" ) ), ::com::sun::star::uno::UNO_QUERY );

    ::com::sun::star::uno::Reference< ::com::sun::star::document::XFilter > xLoader( xFilters->createInstance( aName ), ::com::sun::star::uno::UNO_QUERY );
    if ( xLoader.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >  xComp( GetModel(), ::com::sun::star::uno::UNO_QUERY );
        ::com::sun::star::uno::Reference< ::com::sun::star::document::XImporter > xImporter( xLoader, ::com::sun::star::uno::UNO_QUERY );
        xImporter->setTargetDocument( xComp );
        xLoader->filter( GetModel()->getArgs() );

        pImp->xFilter = ::com::sun::star::uno::Reference< ::com::sun::star::document::XFilter > ( xLoader, ::com::sun::star::uno::UNO_QUERY );
        if ( pImp->xFilter.is() )
            pImp->aStarOneFilterName = aName;
        else
            pImp->aStarOneFilterName.Erase();
        return sal_True;
    }

    return sal_False;
}

sal_Bool SfxObjectShell::ExportTo( SfxMedium& rMedium, const SfxItemSet& rSet )
{
    String aName( rMedium.GetFilter()->GetFilterName() );
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xMan = ::comphelper::getProcessServiceFactory();
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XExporter > xExporter;
    if ( pImp->aStarOneFilterName == aName )
        xExporter = ::com::sun::star::uno::Reference< ::com::sun::star::document::XExporter > ( pImp->xFilter, ::com::sun::star::uno::UNO_QUERY );
    else
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > xFilters (
                xMan->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.FilterFactory" ) ), ::com::sun::star::uno::UNO_QUERY );

        xExporter = ::com::sun::star::uno::Reference< ::com::sun::star::document::XExporter > ( xFilters->createInstance( aName ), ::com::sun::star::uno::UNO_QUERY );
    }

    if ( xExporter.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >  xComp( GetModel(), ::com::sun::star::uno::UNO_QUERY );
        ::com::sun::star::uno::Reference< ::com::sun::star::document::XFilter > xFilter( xExporter, ::com::sun::star::uno::UNO_QUERY );
        xExporter->setSourceDocument( xComp );
        xFilter->filter( GetModel()->getArgs() );
        return sal_True;
    }

    return sal_False;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::ConvertTo
(
    SfxMedium&  rMedium     /*  <SfxMedium>, welches die Ziel-Datei beschreibt
                                (z.B. Dateiname, <SfxFilter>, Open-Modi etc.) */
)

/*  [Beschreibung]

    Diese Methode wird zum Speichern von Dokumenten "uber alle Filter gerufen,
    die nicht SFX_FILTER_OWN sind oder f"ur die kein Clipboard-Format
    registriert wurde (also kein Storage-Format benutzen). Mit anderen Worten:
    mit dieser Methode wird exportiert.

    Das hier zu "offende File sollte "uber 'rMedium' ge"offnet werden,
    um die richtigen Open-Modi zu gew"ahrleisten. Insbesondere wenn das
    Format beibehalten wird (nur m"oglich bei SFX_FILTER_SIMULATE oder
    SFX_FILTER_ONW) mu\s die Datei auch nach dem Speichern im Modus
    STREAM_SHARE_DENYWRITE ge"offnet bleiben.


    [R"uckgabewert]

    sal_Bool                sal_True
                        Das Dokument konnte gespeichert werden.

                        sal_False
                        Das Dokument konnte nicht gespeichert werden, ein
                        Fehlercode ist mit <SvMedium::GetError()const> zu
                        erhalten.


    [Beispiel]

    sal_Bool DocSh::ConvertTo( SfxMedium &rMedium )
    {
        SvStreamRef xStream = rMedium.GetOutStream();
        if ( xStream.is() )
        {
            xStream->SetBufferSize(4096);
            *xStream << ...;

            rMedium.CloseOutStream(); // "offnet automatisch wieder den InStream
            return SVSTREAM_OK == rMedium.GetError();
        }
        return sal_False ;
    }


    [Querverweise]

    <SfxObjectShell::ConvertFrom(SfxMedium&)>
    <SFX_FILTER_REGISTRATION>
*/

{
    return sal_False;
}

//-------------------------------------------------------------------------

void SfxObjectShell::SetEAs_Impl( SfxMedium &rMedium )
{
    //!! wenn OV eine entsprechende Funktionalitaet zur Verfuegung stellt,
    // besser auf der geoeffneten Datei arbeiten
    SvEaMgr *pMgr = rMedium.GetEaMgr();
    SvEaMgr *pOld = GetMedium()->GetEaMgr();
    if ( !pMgr )
        return;

    if ( pOld )
        pOld->Clone( *pMgr );

    String aBuffer;
    pMgr->SetComment( GetDocInfo().GetComment() );

    pMgr->SetFileType( rMedium.GetFilter()->GetTypeName().GetToken( 0, ';' ) );
    if ( SvEaMgr::GetAppCreator(aBuffer) )
        pMgr->SetCreator(aBuffer);

    if ( rMedium.GetLongName().Len() )
        pMgr->SetLongName(rMedium.GetLongName());
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::DoSave_Impl( const SfxItemSet* pArgs )

//Hier jetzt mal eine Einordnung der einzelnen Save Funktionen
//
//DoSave / DoSaveAs: Werden ausschliesslich ueber OLE gerufen
//DoSave_Impl      : Einfaches Speichern mit allem OLE Protokoll SchnickSchnack
//Save_Impl        : Bearbeitungsfunktion fuer SAVEDOC

{
    sal_Bool bSaved = sal_False;
    SfxMedium *pMedium = GetMedium();

// Save jetzt in jedem Fall ueber SaveAs in temporaeres Medium
// Ausser, wenn kein Backup gewuensch ist und wir ins eigene
// Storageformat schreiben und wir nicht in SaveAs sind.

    //  Backup will be created in SaveTo_Impl()
    //  const sal_Bool bDoBackup=SvtSaveOptions().IsBackup();
    const sal_Bool bIsOwn=IsOwnStorageFormat_Impl(*pMedium);

// Zur Zeit wirder immer ueber temporaere Datei, um Storages schrumpfen
// zu lassen.
//
    {
        const StreamMode nFlags = pMedium->GetOpenMode();
        const sal_Bool bDirect = pMedium->IsDirect();
        const String aLongName(pMedium->GetLongName());

        SfxItemSet  *pSet =
            pMedium->GetItemSet()?
                new SfxAllItemSet(*pMedium->GetItemSet()): 0;

        const SfxFilter* pFilter = GetMedium()->GetFilter();
        SfxMedium* pMediumTmp = new SfxMedium( pMedium->GetName(), nFlags, bDirect, pFilter, pSet );
        pMediumTmp->CreateTempFileNoCopy();
        pMediumTmp->SetLongName( aLongName );

        // Nat"urlich keine Version in einem neuen Medium!
        pMediumTmp->GetItemSet()->ClearItem( SID_VERSION );
        pMediumTmp->GetItemSet()->Put( SfxStringItem( SID_DOCTEMPLATE, pMedium->GetURLObject().GetBase()) );

        const String aOldURL( INetURLObject::GetBaseURL() );
        if( GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
            if ( ShallSetBaseURL_Impl(*pMedium) )
                INetURLObject::SetBaseURL( pMedium->GetName() );
            else
                INetURLObject::SetBaseURL( String() );

        pMediumTmp->TransferVersionList_Impl( *pMedium );

        if ( pFilter && ( pFilter->GetFilterFlags() & SFX_FILTER_PACKED ) )
            SetError( GetMedium()->Unpack_Impl( pMedium->GetPhysicalName() ) );

        if( !GetError() && SaveTo_Impl(*pMediumTmp, pArgs) )
        {
            INetURLObject::SetBaseURL( aOldURL );
            ByteString aKey;
            if ( IsOwnStorageFormat_Impl( *pMediumTmp ) )
                aKey = pMediumTmp->GetStorage()->GetKey();

            DoHandsOff();
            SfxItemSet *pSet = pMediumTmp->GetItemSet();
            if(pSet)
                pMedium->GetItemSet()->Put(*pSet);

            pMedium->TransferVersionList_Impl( *pMediumTmp );
            SetError(pMediumTmp->GetErrorCode());
            pMediumTmp->Close();
            bSaved=sal_True;
            delete pMediumTmp;

            sal_Bool bOpen = DoSaveCompleted(pMedium);
            if (  bOpen && aKey.Len() )
                pMedium->GetStorage()->SetKey( aKey );
            DBG_ASSERT(bOpen,"Fehlerbehandlung fuer DoSaveCompleted nicht implementiert");
        }
        else
        {
            INetURLObject::SetBaseURL( aOldURL );
            SetError( pMediumTmp->GetError() );
            String aTmp( pMediumTmp->GetPhysicalName() );
            delete pMediumTmp;
            SfxContentHelper::Kill( aTmp );
            DoSaveCompleted( (SvStorage*)0 );
        }
    }

    SetModified(!bSaved);
    return bSaved;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::Save_Impl( const SfxItemSet* pSet )
{
    DBG_CHKTHIS(SfxObjectShell, 0);
    SfxApplication *pSfxApp = SFX_APP();

    pImp->bIsSaving = sal_True;
    sal_Bool bSaved;
    SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False);
    if ( pSalvageItem )
    {
        SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pFilterItem, SfxStringItem, SID_FILTER_NAME, sal_False);
        const SfxFilter *pFilter = pFilterItem
                    ? GetFactory().GetFilterContainer()->GetFilter(pFilterItem->GetValue())
                    : 0;
        SfxMedium *pMed = new SfxMedium(
            pSalvageItem->GetValue(), STREAM_READWRITE | STREAM_SHARE_DENYWRITE, sal_False, pFilter );

        SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pPasswordItem, SfxStringItem, SID_PASSWORD, sal_False );
        if ( pPasswordItem )
            pMed->GetItemSet()->Put( *pPasswordItem );

        bSaved = DoSaveAs( *pMed );
        if ( bSaved )
            bSaved = DoSaveCompleted( pMed );
        else
            delete pMed;
    }
    else
        bSaved = DoSave_Impl( pSet );
    if ( bSaved && SvtSaveOptions().IsAutoSave() )
        pSfxApp->GetAutoSaveTimer_Impl()->Start();
    return bSaved;
}

//-------------------------------------------------------------------------

class SfxSaveAsContext_Impl
{
    String&     _rNewNameVar;
    String      _aNewName;

public:
                SfxSaveAsContext_Impl( String &rNewNameVar,
                                       const String &rNewName )
                :   _rNewNameVar( rNewNameVar ),
                    _aNewName( rNewName )
                { rNewNameVar = rNewName; }
                ~SfxSaveAsContext_Impl()
                { _rNewNameVar.Erase(); }
};

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::SaveAs_Impl(sal_Bool bUrl, SfxRequest *pRequest)
{
    // must we ask the user for the filename?
    INetURLObject aURL;
    sal_uInt16 nActFilt = 0;
    const SfxFilter* pFilt;
    for( pFilt = GetFactory().GetFilter( 0 );
         pFilt && ( !pFilt->CanExport() || pFilt->IsInternal() );
         pFilt = GetFactory().GetFilter( ++nActFilt ) );

    DBG_ASSERT( pFilt, "Kein Filter zum Speichern" );

    String aFilterName;
    if( pFilt )
        aFilterName = pFilt->GetName();

    SfxItemSet *pParams = new SfxAllItemSet( SFX_APP()->GetPool() );
    SFX_REQUEST_ARG( (*pRequest), pFileNameItem, SfxStringItem, SID_FILE_NAME, sal_False );
    if ( pRequest->GetArgs() )
        pParams->Put( *pRequest->GetArgs() );

    if ( !pFileNameItem )
    {
        if(! bUrl )
        {
            // get the filename by dialog
            ISfxModule *pMod = GetModule();
            if ( !pMod )
            {
                DBG_ERROR( "ObjectShell without Module!" );
                SetError(ERRCODE_IO_ABORT);
                return FALSE;
            }

            SfxFileDialog* pDlg = pMod->CreateDocFileDialog( WB_SAVEAS | WB_3DLOOK, GetFactory(), pParams );
            if ( HasName() )
            {
                String aLastName = QueryTitle(
                    SFX_TITLE_QUERY_SAVE_NAME_PROPOSAL );
                const SfxFilter* pMedFilter = GetMedium()->GetFilter();
                if( pImp->bSetStandardName && !IsTemplate() || !pMedFilter ||
                    !pMedFilter->CanExport() ||
                    pMedFilter->GetVersion() != SOFFICE_FILEFORMAT_CURRENT )
                {
                    if( aLastName.Len() )
                    {
                        String aPath( aLastName );
                        bool bWasAbsolute = FALSE;
                        INetURLObject aObj( SvtPathOptions().GetWorkPath() );
                        aObj.setFinalSlash();
                        aObj = INetURLObject( aObj.RelToAbs( aPath, bWasAbsolute ) );
                        aObj.SetExtension( pFilt->GetDefaultExtension().Copy(2) );
                        pDlg->SetPath( aObj.GetMainURL() );
                    }

                    pDlg->SetCurFilter( pFilt->GetName() );
                }
                else
                {
                    if( aLastName.Len() )
                    {
                        String aPath( pDlg->GetPath() );
                        aPath += aLastName;
                        pDlg->SetSmartPath( aPath );
                    }

                    pDlg->SetCurFilter( pMedFilter->GetName() );
                }
            }
            else
            {
                pDlg->SetPath( SvtPathOptions().GetWorkPath() );
            }

            if ( pDlg->Execute() == RET_CANCEL )
            {
                delete pDlg;
                SetError(ERRCODE_IO_ABORT);
                return sal_False;
            }

            aURL.SetURL( pDlg->GetPath() );

            // gibt es schon ein Doc mit dem Namen?
            const String aName(aURL.GetMainURL());
            SfxObjectShell* pDoc = 0;
            for ( SfxObjectShell* pTmp = SfxObjectShell::GetFirst();
                  pTmp && !pDoc;
                  pTmp = SfxObjectShell::GetNext(*pTmp) )
                //! fsys bug op==
                if ( pTmp->GetMedium() )
                    // ??? HasName() MM
                    if(pTmp != this && pTmp->GetMedium()->GetName() == aName )
                        pDoc = pTmp;
            if ( pDoc )
            {
                // dann Fehlermeldeung: "schon offen"
                SetError(ERRCODE_SFX_ALREADYOPEN);
                delete pDlg;
                return sal_False;
            }

            // Parameter aus Dialog holen
            const String aFilter(pDlg->GetCurFilter());
            if(aFilter.Len())
                aFilterName = aFilter;
            pParams->Put( *pDlg->GetItemSet() );
            delete pDlg;
        }
        else
        {
            SfxUrlDialog aDlg( 0 );
            if( aDlg.Execute() == RET_OK )
                aURL.SetURL( aDlg.GetUrl() );
            else
            {
                SetError(ERRCODE_IO_ABORT);
                return sal_False;
            }
        }

        // Request mit Dateiname und Filter vervollst"andigen
        pRequest->AppendItem(SfxStringItem( SID_FILE_NAME, aURL.GetMainURL()) );
        pRequest->AppendItem(SfxStringItem( SID_FILTER_NAME, aFilterName));
        const SfxPoolItem* pItem=0;
        pRequest->GetArgs()->GetItemState( SID_FILE_NAME, sal_False, &pItem );
        pFileNameItem = PTR_CAST( SfxStringItem, pItem );
    }

    // neuen Namen an der Object-Shell merken
    SfxSaveAsContext_Impl aSaveAsCtx( pImp->aNewName, aURL.GetMainURL() );

    // now we can get the filename from the SfxRequest
    DBG_ASSERT( pRequest->GetArgs() != 0, "fehlerhafte Parameter");
    SFX_REQUEST_ARG( (*pRequest), pSaveToItem, SfxBoolItem, SID_SAVETO, sal_False );
    FASTBOOL bSaveTo = pSaveToItem ? pSaveToItem->GetValue() : sal_False;
    if ( !pFileNameItem && bSaveTo )
    {
        // SaveTo auch ohne Filenamen m"oglich -> dann FileDialog "offnen
        ISfxModule *pMod = GetModule();
        if ( !pMod )
        {
            DBG_ERROR( "ObjectShell without Module!" );
            SetError(ERRCODE_IO_ABORT);
            return sal_False;;
        }

        SfxFileDialog* pDlg = pMod->CreateDocFileDialog( WB_SAVEAS | WB_3DLOOK, GetFactory(), pRequest->GetArgs() );
        if ( pDlg->Execute() == RET_CANCEL )
        {
            // Speichern abbrechen
            delete pDlg;
            SetError(ERRCODE_IO_ABORT);
            return sal_False;
        }
        else
        {
            // Einstellungen aus dem FileDialog "ubernehmen
            aURL.SetURL( pDlg->GetPath() );
            aFilterName = pDlg->GetCurFilter();
            delete pDlg;
        }
    }
    else if ( pFileNameItem )
    {
        aURL.SetURL(((const SfxStringItem *)pFileNameItem)->GetValue() );
        DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "Illegal URL!" );

        const SfxPoolItem* pFilterNameItem=0;
        const SfxItemState eState = pRequest->GetArgs()->GetItemState(SID_FILTER_NAME, sal_True, &pFilterNameItem);
        if ( SFX_ITEM_SET == eState )
        {
            DBG_ASSERT(pFilterNameItem->IsA( TYPE(SfxStringItem) ), "Fehler Parameter");
            aFilterName = ((const SfxStringItem *)pFilterNameItem)->GetValue();
        }
    }
    else
    {
        SetError( ERRCODE_IO_INVALIDPARAMETER );
        return sal_False;
    }

    const SfxFilter* pFilter = GetFactory().GetFilterContainer()->GetFilter( aFilterName );
    if ( !pFilter )
    {
        SetError( ERRCODE_IO_INVALIDPARAMETER );
        return sal_False;
    }

    pImp->bPasswd = pParams && SFX_ITEM_SET == pParams->GetItemState(SID_PASSWORD);

    // unter gleichem Namen speichern?
    SfxMedium *pActMed = GetMedium();
    const INetURLObject aActName(pActMed->GetName());

    if ( aURL == aActName )
    {
        if ( IsReadOnly() )
        {
            SetError(ERRCODE_SFX_DOCUMENTREADONLY);
            return sal_False;
        }
        // gleicher Filter? -> Save()
        const SfxFilter *pFilter = pActMed->GetFilter();
        if ( pFilter && pFilter->GetName() == aFilterName )
        {
            pImp->bIsSaving=sal_False;
            if ( pParams )
            {
                SfxItemSet* pSet = pMedium->GetItemSet();
                pSet->ClearItem( SID_PASSWORD );
                pSet->Put( *pParams );
            }
            return DoSave_Impl();
        }
    }

    if( aURL.HasError() )
    {
        SetError( ERRCODE_IO_INVALIDPARAMETER );
        return sal_False;
    }

    if ( PreDoSaveAs_Impl(aURL.GetMainURL(),aFilterName,pParams))
    {
        pImp->bWaitingForPicklist = sal_True;
        if (!pImp->bSetStandardName)
            pImp->bDidWarnFormat=sal_False;
        // Muss !!!
        if ( IsOwnStorageFormat_Impl(*GetMedium()))
        {
            SetFileName( GetMedium()->GetPhysicalName() );
        }

        return sal_True;
    }
    else
        return sal_False;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::PreDoSaveAs_Impl
(
    const String&   rFileName,
    const String&   aFilterName,
    SfxItemSet*     pParams
)
{
    if ( pParams )
        pParams->Put(
            SfxStringItem(
                SID_DOCTEMPLATE, INetURLObject( rFileName ).GetBase()) );
    SfxAllItemSet* pMergedParams = new SfxAllItemSet(
        *pMedium->GetItemSet() );
    pMergedParams->ClearItem( SID_PASSWORD );
    pMergedParams->ClearItem( SID_DOCINFO_TITLE );

    if( pParams )
        pMergedParams->Put( *pParams );
    delete pParams;
    pMergedParams->ClearItem( SID_DOC_SALVAGE );
    pParams = pMergedParams;

//  SfxItemSet  *pSet = pParams ? new SfxAllItemSet(*pParams) : 0;

    SfxMedium *pNewFile = new SfxMedium(
        rFileName, STREAM_READWRITE | STREAM_SHARE_DENYWRITE, sal_False, 0, pParams );

    SFX_ITEMSET_ARG(
        pParams, pSaveToItem, SfxBoolItem, SID_SAVETO, sal_False );

    sal_Bool bCopyTo =
        GetCreateMode() == SFX_CREATE_MODE_EMBEDDED ||
        pSaveToItem && pSaveToItem->GetValue();

    SfxDocumentInfo aSavedInfo;
    if ( bCopyTo )
        aSavedInfo = GetDocInfo();

    pNewFile->SetFilter( GetFactory(), aFilterName);
    pNewFile->CreateTempFile();

    sal_Bool bOk;

    SfxMedium *pMediumTmp;

    const String aOldURL( INetURLObject::GetBaseURL() );
    if( GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
        if ( ShallSetBaseURL_Impl(*pNewFile) )
            INetURLObject::SetBaseURL( pNewFile->GetName() );
        else
            INetURLObject::SetBaseURL( String() );

    pMediumTmp = pNewFile;

    // Nat"urlich keine Version in einem neuen Medium!
    pMediumTmp->GetItemSet()->ClearItem( SID_VERSION );

    if ( aFilterName.Len() )
        pMediumTmp->SetFilter( GetFactory(), aFilterName );
    else
        pMediumTmp->SetFilter( GetFactory().GetFilterContainer()->GetFilter(0) );

    pImp-> bIsSaving = sal_False;
    bOk = sal_False;

    if ( IsOwnStorageFormat_Impl(*pNewFile) )
    {
        long nFormat = pMediumTmp->GetFilter()->GetFormat();
        SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
        const SfxFilter *pFilt = rMatcher.GetFilter4ClipBoardId( nFormat );
        if ( pFilt )
        {
            if ( pFilt->GetFilterContainer() != pMediumTmp->GetFilter()->GetFilterContainer() )
                pMediumTmp->GetStorage()->SetClass( SvFactory::GetServerName( nFormat ), nFormat, pFilt->GetTypeName() );
        }
    }

    if ( GetMedium()->GetFilter() && ( GetMedium()->GetFilter()->GetFilterFlags() & SFX_FILTER_PACKED ) )
    {
        SfxMedium *pMed = bCopyTo ? pMedium : pNewFile;
        pMediumTmp->SetError( GetMedium()->Unpack_Impl( pMed->GetPhysicalName() ) );
    }

    sal_Bool bCouldNotConnect = sal_False;
    sal_Bool bNeedsStorage;
    SvStorageRef xNewTempRef;

    if ( !pMediumTmp->GetErrorCode() )
    {

        // Falls jetzt in ein Fremdformat gespeichert wird, darf nicht der
        // Objektstorage weiterverwendet werden, wenn das alte Format das
        // eigene war. Daher wird hier eine temporaerer erzeugt.
        // Damit DoHandsOff gerufen werden kann, merken wir uns den
        // Storage und rufen anschliessend von Hand SaveCompleted

        bNeedsStorage = !bCopyTo && IsOwnStorageFormat_Impl(*pMedium) &&
                        !IsOwnStorageFormat_Impl(*pNewFile);

        if ( bNeedsStorage && pMedium->GetName().Len() )
        {
            if(!ConnectTmpStorage_Impl( pMedium->GetStorage() ))
                bCouldNotConnect = sal_True;
        }

        if( bNeedsStorage || !pMedium->GetName().Len() )
            xNewTempRef = GetStorage();
    }

    if ( !pMediumTmp->GetErrorCode() && SaveTo_Impl( *pMediumTmp ) )
    {
        bOk = sal_True;
        INetURLObject::SetBaseURL( aOldURL );

        if ( bCouldNotConnect )
            bOk = sal_False;

        SetError( pMediumTmp->GetErrorCode() );

        if( bNeedsStorage )
            SaveCompleted( xNewTempRef );

        if ( !bCopyTo )
            bOk = DoSaveCompleted( pNewFile );

        //! Vorsich. Muss nicht immer klappen.
        DBG_ASSERT( bOk, "DoSaveCompleted nicht geklappt und keine Fehlerbehandlung");
        if( bOk )
        {
            if( !bCopyTo )
            {
                SetModified(sal_False);
                bOk=sal_True;
            }
        }
        else
        {
            SetError( pNewFile->GetErrorCode() );
            if ( !pMedium->GetName().Len() )
                SaveCompleted( xNewTempRef );
            else
            DoSaveCompleted( pMedium );
            DELETEZ( pNewFile );
        }

        String aPasswd;
        if ( IsOwnStorageFormat_Impl( *GetMedium() ) && GetPasswd_Impl( GetMedium()->GetItemSet(), aPasswd ) )
            GetMedium()->GetStorage()->SetKey( S2BS( aPasswd ) );   //!!! (pb) needs new implementation
    }
    else
    {
        INetURLObject::SetBaseURL( aOldURL );
        SetError(pMediumTmp->GetErrorCode());
        if ( bCopyTo )
            DoSaveCompleted( (SvStorage*)0 );
        else
        {
            if( !pMedium->GetName().Len() )
                SaveCompleted( xNewTempRef );
            else
                DoSaveCompleted( pMedium );
        }
    }

    if(!bOk)
        SetModified(sal_True);

    if ( bCopyTo )
    {
        SfxDocumentInfo &rDocInfo = GetDocInfo();
        rDocInfo = aSavedInfo;
    }

    return bOk;
}

//------------------------------------------------------------------------

sal_Bool SfxObjectShell::LoadFrom(  SvStorage *pStor )
{
    if (pStor->IsStream(SfxConfigManager::GetStreamName()))
        SetConfigManager (new SfxConfigManager( pStor, 0));
    GetDocInfo().Load(pStor);
    return sal_True;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::CanReload_Impl()

/*  [Beschreibung]

    Interne Methode zum Feststellen, ob eine erneutes Laden des
    Dokuments (auch als RevertToSaved oder LastVersion bekannt)
    m"oglich ist.
*/

{
    const SfxMedium *pMedium = GetMedium();
    const SfxFilter *pFilter = pMedium ? pMedium->GetFilter() : 0;
    return pMedium && HasName() &&
        ( !pFilter || ! pFilter->GetFilterName().EqualsAscii( SFX_FILTER_DOWNLOAD ) ) &&
        !IsInModalMode() && !Application::IsInModalMode() &&
        !pImp->bForbidReload;
    // Fuer AutoLoad muss Reload immer enabled sein, also NICHT:
    // ( IsModified() || GetMedium()->IsRemote()
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::LoadLayout() const
{
    return pImp->bLoadLayout;
}

//-------------------------------------------------------------------------

void SfxObjectShell::SetLoadLayout_Impl( sal_Bool bLoadLayout )
{
    pImp->bLoadLayout = bLoadLayout;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsInformationLost()
{
    const SfxFilter *pFilt = GetMedium()->GetFilter();
    return pFilt && pFilt->IsAlienFormat() && pImp->bDidDangerousSave && !(pFilt->GetFilterFlags() & SFX_FILTER_SILENTEXPORT);
}

sal_Bool SfxObjectShell::LoadOwnFormat( SfxMedium& rMedium )
{
    SvStorageRef xStor = rMedium.GetStorage();
    if ( xStor.Is() )
    {
        // Config
        if ( xStor->IsStream(SfxConfigManager::GetStreamName()) )
            SetConfigManager (new SfxConfigManager( xStor, SFX_CFGMANAGER()));
        if ( rMedium.GetFileVersion() )
            xStor->SetVersion( rMedium.GetFileVersion() );

        // Password
        SFX_ITEMSET_ARG( rMedium.GetItemSet(), pPasswdItem,
                         SfxStringItem, SID_PASSWORD, sal_False );
        SfxApplication *pApp = SFX_APP();
        if ( pPasswdItem || ERRCODE_IO_ABORT != CheckPasswd_Impl( 0, pApp->GetPool(), pMedium ) )
        {
            String aPasswd;
            if ( GetPasswd_Impl(pMedium->GetItemSet(), aPasswd) )
                xStor->SetKey( S2BS( aPasswd ) ); //!!! (pb) needs new implementation

            // load document
            return Load( xStor );
        }
        return sal_False;
    }
    else
        return sal_False;
}

sal_Bool SfxObjectShell::SaveAsOwnFormat( SfxMedium& rMedium )
{
    SvStorageRef xStor = rMedium.GetStorage();
    if( xStor.Is() )
    {
        xStor->SetVersion( rMedium.GetFilter()->GetVersion() );
        const SfxFilter* pFilter = rMedium.GetFilter();
        return SaveAs( xStor );
    }
    else return sal_False;
}


void SfxObjectShell::AddXMLAsZipToTheStorage( SvStorage& rRoot )
{
    static struct _ObjExpType {
        sal_Bool (SvtAddXMLToStorageOptions:: *fnIsAdd)() const;
        const sal_Char* pModuleNm;
        // GlobalNameId
        UINT32 n1;
        USHORT n2, n3;
        BYTE b8, b9, b10, b11, b12, b13, b14, b15;
    } aArr[] = {
        { &SvtAddXMLToStorageOptions::IsWriter_Add_XML_to_Storage,
            "Writer", SO3_SW_CLASSID_50 },
        { &SvtAddXMLToStorageOptions::IsCalc_Add_XML_to_Storage,
            "Calc", SO3_SC_CLASSID_50 },
        { &SvtAddXMLToStorageOptions::IsImpress_Add_XML_to_Storage,
            "Impress", SO3_SIMPRESS_CLASSID_50 },
        { &SvtAddXMLToStorageOptions::IsDraw_Add_XML_to_Storage,
            "Draw", SO3_SDRAW_CLASSID_50 },
        { 0 }
    };

    for( const _ObjExpType* pArr = aArr; pArr->fnIsAdd; ++pArr )
    {
        SvGlobalName aGlbNm( pArr->n1, pArr->n2, pArr->n3,
                            pArr->b8, pArr->b9, pArr->b10, pArr->b11,
                            pArr->b12, pArr->b13, pArr->b14, pArr->b15 );
        if( *GetSvFactory() == aGlbNm )
        {
            // 1. check if the option is set and unequal 0 or is not set
            SvtAddXMLToStorageOptions aOpt;
            if( (aOpt.*pArr->fnIsAdd)() )
            {
                // the flag is set
                String sStr;
                sStr.AssignAscii( "StarOffice XML (" );
                sStr.AppendAscii( pArr->pModuleNm );
                sStr += ')';
                // 2. exist the XML filter? "StarOffice XML (<Application>)"?
                const SfxFilter* pFilter = GetFactory().GetFilterContainer()->
                                                GetFilter4FilterName( sStr );
                if( pFilter )
                {
                    ::utl::TempFile aTempFile;
                    SfxMedium       aTmpMed( aTempFile.GetURL(), STREAM_READ | STREAM_WRITE, TRUE );

                    aTmpMed.SetFilter( pFilter );

                    if( ConvertTo( aTmpMed ) )
                    {
                        SvStorage* pXMLStor = aTmpMed.GetStorage();

                        if( pXMLStor )
                        {
                            const String    aContent( String::CreateFromAscii( "Content" ) );
                            const String    aContentXML( String::CreateFromAscii( "Content.xml" ) );
                            const String    aXMLFormatName( String::CreateFromAscii( "XMLFormat2" ) );
                            String          aContentName;

                            if( pXMLStor->IsContained( aContentXML ) )
                                aContentName = aContentXML;
                            else if( pXMLStor->IsContained( aContent ) )
                                aContentName = aContent;

                            if( aContentName.Len() )
                            {
                                SvStorageStreamRef  xOStm( rRoot.OpenStream( aXMLFormatName, STREAM_WRITE | STREAM_TRUNC ) );
                                SvStorageStreamRef  xIStm( pXMLStor->OpenStream( aContentName, STREAM_READ | STREAM_NOCREATE ) );

                                if( xOStm.Is() && xIStm.Is() )
                                {
                                    ZCodec aCodec;

                                    xIStm->Seek( 0 );
                                    aCodec.BeginCompression( ZCODEC_BEST_COMPRESSION );
                                    aCodec.Compress( *xIStm, *xOStm );
                                    aCodec.EndCompression();
                                    xOStm->Commit();
                                }
                            }
                        }
                    }
                }
            }
            // that's all
            break;
        }
    }
}
