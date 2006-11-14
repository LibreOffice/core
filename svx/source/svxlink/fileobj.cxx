/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fileobj.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:51:42 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifdef OS2
#include <vcl/sysdep.hxx>
#endif


#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _FILTER_HXX
#include <svtools/filter.hxx>
#endif
#ifndef _SOERR_HXX
//#include <so3/soerr.hxx>
#endif
#ifndef _LNKBASE_HXX
#include <sfx2/lnkbase.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_PROGRESS_HXX
#include <sfx2/progress.hxx>
#endif
#ifndef _SFX_INTERNO_HXX
//#include <sfx2/interno.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif
#include <sot/exchange.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <sfx2/docfac.hxx>

#include "fileobj.hxx"
#include "linkmgr.hxx"
#include "dialmgr.hxx"
#include "dialogs.hrc"
#include "xoutbmp.hxx"
#include "opengrf.hxx"
#include "impgrf.hxx"

using namespace ::com::sun::star::uno;


#define FILETYPE_TEXT       1
#define FILETYPE_GRF        2
#define FILETYPE_OBJECT     3

struct Impl_DownLoadData
{
    Graphic aGrf;
    Timer aTimer;

    Impl_DownLoadData( const Link& rLink )
    {
        aTimer.SetTimeout( 100 );
        aTimer.SetTimeoutHdl( rLink  );
        aGrf.SetDefaultType();
    }
    ~Impl_DownLoadData()
    {
        aTimer.Stop();
    }
};

// --------------------------------------------------------------------------


SvFileObject::SvFileObject()
:   pDownLoadData( 0 )
,   nType( FILETYPE_TEXT )
{
    bLoadAgain = TRUE;
    bSynchron = bLoadError = bWaitForData = bDataReady = bNativFormat =
    bClearMedium = bStateChangeCalled = bInCallDownLoad = FALSE;
}


SvFileObject::~SvFileObject()
{
    if ( xMed.Is() )
    {
        xMed->SetDataAvailableLink( Link() );
        xMed->SetDoneLink( Link() );
        xMed.Clear();
    }
    delete pDownLoadData;
}


BOOL SvFileObject::GetData( ::com::sun::star::uno::Any & rData,
                                const String & rMimeType,
                                BOOL bGetSynchron )
{
    ULONG nFmt = SotExchange::GetFormatStringId( rMimeType );
    switch( nType )
    {
    case FILETYPE_TEXT:
        if( FORMAT_FILE == nFmt )
        {
            // das Medium muss in der Applikation geoffnet werden, um die
            // relativen Datei Links aufzuloesen!!!! Wird ueber den
            // LinkManager und damit von dessen Storage erledigt.
            rData <<= rtl::OUString( sFileNm );

/*
===========================================================================
JP 28.02.96: noch eine Baustelle:
                Idee: hier das Medium und die DocShell anlegen, Doc laden
                      und ueber OLE-SS (GetObj(...)) den Bereich als
                      PseudoObject erfragen. Dieses mit den Daten oder
                      dessen Daten verschicken.

===========================================================================

            SfxMedium aMed( aFileNm.GetFull(), STREAM_READ, TRUE );
            aMed.DownLoad();        // nur mal das Medium anfassen (DownLoaden)

            if( aMed.IsStorage() )
                pSvData->SetData( SvStorageRef( aMed.GetStorage() ),
                                    TRANSFER_COPY );
            else
            {
                SvStream* pStream = aMed.GetInStream();
                if( !pStream )
                    return FALSE;

                UINT32 nLen = pStream->Seek( STREAM_SEEK_TO_END );
                pStream->Seek( STREAM_SEEK_TO_BEGIN );

                void* pData = SvMemAlloc( nLen );
                pStream->Read( pData, nLen );
                pSvData->SetData( pData, nLen, TRANSFER_MOVE );
            }
*/
        }
        break;

    case FILETYPE_GRF:
        if( !bLoadError )
        {
            SfxMediumRef xTmpMed;

            if( FORMAT_GDIMETAFILE == nFmt || FORMAT_BITMAP == nFmt ||
                SOT_FORMATSTR_ID_SVXB == nFmt )
            {
                Graphic aGrf;

                //JP 15.07.98: Bug 52959
                //      falls das Nativformat doch erwuenscht ist, muss am
                //      Ende das Flag zurueckgesetzt werden.
// wird einzig und allein im sw/ndgrf.cxx benutzt, wenn der Link vom
// GraphicNode entfernt wird.
                BOOL bOldNativFormat = bNativFormat;
//!!??              bNativFormat = 0 != (ASPECT_ICON & pSvData->GetAspect());

                // falls gedruckt werden soll, warten wir bis die
                // Daten vorhanden sind
                if( bGetSynchron )
                {
                    // testhalber mal ein LoadFile rufen um das nach-
                    // laden ueberahaupt anzustossen
                    if( !xMed.Is() )
                        LoadFile_Impl();

                    if( !bInCallDownLoad )
                    {
                        xTmpMed = xMed;
                        while( bWaitForData )
                            Application::Reschedule();

                        xMed = xTmpMed;
                        bClearMedium = TRUE;
                    }
                }

                if( pDownLoadData ||
                    ( !bWaitForData && ( xMed.Is() ||       // wurde als URL geladen
                        ( bSynchron && LoadFile_Impl() && xMed.Is() ) )) )
                {
                    // falls

                    // falls es uebers Internet gesogen wurde, nicht
                    // wieder versuchen
                    if( !bGetSynchron )
                        bLoadAgain = !xMed->IsRemote();
                    bLoadError = !GetGraphic_Impl( aGrf, xMed->GetInStream() );
                }
                else if( !LoadFile_Impl() ||
                        !GetGraphic_Impl( aGrf, xMed.Is() ? xMed->GetInStream() : 0 ))
                {
                    if( !xMed.Is() )
                        break;
                    aGrf.SetDefaultType();
                }

                if( SOT_FORMATSTR_ID_SVXB != nFmt )
                    nFmt = (bLoadError || GRAPHIC_BITMAP == aGrf.GetType())
                                ? FORMAT_BITMAP
                                : FORMAT_GDIMETAFILE;

                SvMemoryStream aMemStm( 0, 65535 );
                switch ( nFmt )
                {
                case SOT_FORMATSTR_ID_SVXB:
                    if( GRAPHIC_NONE != aGrf.GetType() )
                    {
                        aMemStm.SetVersion( SOFFICE_FILEFORMAT_50 );
                        aMemStm << aGrf;
                    }
                    break;

                case  FORMAT_BITMAP:
                    if( !aGrf.GetBitmap().IsEmpty())
                        aMemStm << aGrf.GetBitmap();
                    break;

                default:
                    if( aGrf.GetGDIMetaFile().GetActionCount() )
                    {
                        GDIMetaFile aMeta( aGrf.GetGDIMetaFile() );
                        aMeta.Write( aMemStm );
                    }
                }
                rData <<= Sequence< sal_Int8 >( (sal_Int8*) aMemStm.GetData(),
                                        aMemStm.Seek( STREAM_SEEK_TO_END ) );

                bNativFormat = bOldNativFormat;

                // alles fertig?
                if( xMed.Is() && !bSynchron && bClearMedium )
                {
                    xMed.Clear();
                    bClearMedium = FALSE;
                }
            }
        }
        break;
    case FILETYPE_OBJECT:
        // TODO/LATER: possibility to insert a new object
        rData <<= rtl::OUString( sFileNm );
        break;
    }
    return sal_True/*0 != aTypeList.Count()*/;
}




BOOL SvFileObject::Connect( sfx2::SvBaseLink* pLink )
{
    if( !pLink || !pLink->GetLinkManager() )
        return FALSE;

    // teste doch mal, ob nicht ein anderer Link mit der gleichen
    // Verbindung schon existiert
    pLink->GetLinkManager()->GetDisplayNames( pLink, 0, &sFileNm, 0, &sFilter );

    if( OBJECT_CLIENT_GRF == pLink->GetObjType() )
    {
        SfxObjectShellRef pShell = pLink->GetLinkManager()->GetPersist();
        if( pShell.Is() )
        {
            if( pShell->IsAbortingImport() )
                return FALSE;

            if( pShell->GetMedium() )
                sReferer = pShell->GetMedium()->GetName();
        }
    }

    switch( pLink->GetObjType() )
    {
    case OBJECT_CLIENT_GRF:
        nType = FILETYPE_GRF;
        bSynchron = pLink->IsSynchron();
        break;

    case OBJECT_CLIENT_FILE:
        nType = FILETYPE_TEXT;
        break;

    case OBJECT_CLIENT_OLE:
        nType = FILETYPE_OBJECT;
        // TODO/LATER: introduce own type to be used for exchanging
        break;

    default:
        return FALSE;
    }

    SetUpdateTimeout( 0 );

    // und jetzt bei diesem oder gefundenem Pseudo-Object anmelden
    AddDataAdvise( pLink, SotExchange::GetFormatMimeType( pLink->GetContentType()), 0 );
    return TRUE;
}


BOOL SvFileObject::LoadFile_Impl()
{
    // wir sind noch im Laden!!
    if( bWaitForData || !bLoadAgain || xMed.Is() || pDownLoadData )
        return FALSE;

    // z.Z. nur auf die aktuelle DocShell
    xMed = new SfxMedium( sFileNm, STREAM_STD_READ, TRUE );
    SvLinkSource::StreamToLoadFrom aStreamToLoadFrom =
        getStreamToLoadFrom();
    xMed->setStreamToLoadFrom(
        aStreamToLoadFrom.m_xInputStreamToLoadFrom,
        aStreamToLoadFrom.m_bIsReadOnly);
    // setStreamToLoadFrom(0,0);
    // Keinen Eintrag im Roter Button Menu
    xMed->SetDontCreateCancellable();
    if( sReferer.Len() )
        xMed->SetReferer( sReferer );

    if( !bSynchron )
    {
        bLoadAgain = bDataReady = bInNewData = FALSE;
        bWaitForData = TRUE;

        SfxMediumRef xTmpMed = xMed;
        xMed->SetDataAvailableLink( STATIC_LINK( this, SvFileObject, LoadGrfNewData_Impl ) );
        bInCallDownLoad = TRUE;
        xMed->DownLoad( STATIC_LINK( this, SvFileObject, LoadGrfReady_Impl ) );
        bInCallDownLoad = FALSE;

        bClearMedium = !xMed.Is();
        if( bClearMedium )
            xMed = xTmpMed;     // falls gleich im DownLoad schon schluss ist
        return bDataReady;
    }

    bWaitForData = TRUE;
    bDataReady = bInNewData = FALSE;
    xMed->DownLoad();
    bLoadAgain = !xMed->IsRemote();
    bWaitForData = FALSE;

    // Grafik ist fertig, also DataChanged von der Statusaederung schicken:
    SendStateChg_Impl( xMed->GetInStream() && xMed->GetInStream()->GetError()
                        ? STATE_LOAD_ERROR : STATE_LOAD_OK );

    return TRUE;
}


BOOL SvFileObject::GetGraphic_Impl( Graphic& rGrf, SvStream* pStream )
{
    GraphicFilter* pGF = GetGrfFilter();

    const USHORT nFilter = sFilter.Len() && pGF->GetImportFormatCount()
                            ? pGF->GetImportFormatNumber( sFilter )
                            : GRFILTER_FORMAT_DONTKNOW;

    String aEmptyStr;
    int nRes;

    // vermeiden, dass ein native Link angelegt wird
    if( ( !pStream || !pDownLoadData ) && !rGrf.IsLink() &&
        !rGrf.GetContext() && !bNativFormat )
        rGrf.SetLink( GfxLink() );

    if( !pStream )
        nRes = xMed.Is() ? GRFILTER_OPENERROR
                         : pGF->ImportGraphic( rGrf, INetURLObject(sFileNm),
                            nFilter );
    else if( !pDownLoadData )
    {
        pStream->Seek( STREAM_SEEK_TO_BEGIN );
        nRes = pGF->ImportGraphic( rGrf, aEmptyStr, *pStream, nFilter );
    }
    else
    {
        nRes = pGF->ImportGraphic( pDownLoadData->aGrf, aEmptyStr,
                                    *pStream, nFilter );

        if( pDownLoadData )
        {
            rGrf = pDownLoadData->aGrf;
            if( GRAPHIC_NONE == rGrf.GetType() )
                rGrf.SetDefaultType();


            if( !pDownLoadData->aGrf.GetContext() )
            {
                xMed->SetDataAvailableLink( Link() );
//              xMed->SetDoneLink( Link() );
                delete pDownLoadData, pDownLoadData = 0;
                bDataReady = TRUE;
                bWaitForData = FALSE;
            }
            else if( FALSE )
            {
                // Timer aufsetzen, um zurueck zukehren
                pDownLoadData->aTimer.Start();
            }
        }
    }

    if( pStream && ERRCODE_IO_PENDING == pStream->GetError() )
        pStream->ResetError();

#ifndef PRODUCT
    if( nRes )
    {
        if( xMed.Is() && !pStream )
        {
            DBG_WARNING3( "GrafikFehler [%d] - [%s] URL[%s]",
                            nRes,
                            xMed->GetPhysicalName().GetBuffer(),
                            sFileNm.GetBuffer() );
        }
        else
        {
            DBG_WARNING2( "GrafikFehler [%d] - [%s]",
                            nRes, sFileNm.GetBuffer() );
        }
    }
#endif

    return GRFILTER_OK == nRes;
}


String SvFileObject::Edit( Window* pParent, sfx2::SvBaseLink* pLink )
{
    String sFile, sRange, sTmpFilter;
    if( !pLink || !pLink->GetLinkManager() )
        return sFile;

    pLink->GetLinkManager()->GetDisplayNames( pLink, 0, &sFile, &sRange,
                                                        &sTmpFilter );

    switch( pLink->GetObjType() )
    {
    case OBJECT_CLIENT_GRF:
        {
            nType = FILETYPE_GRF;       // falls noch nicht gesetzt

            SvxOpenGraphicDialog aDlg(ResId(RID_SVXSTR_EDITGRFLINK, DIALOG_MGR()));
            aDlg.EnableLink(sal_False);
            aDlg.SetPath( sFile, sal_True );
            aDlg.SetCurrentFilter( sTmpFilter );

            if( !aDlg.Execute() )
            {
                sFile = aDlg.GetPath();
                sFile += ::sfx2::cTokenSeperator;
                sFile += ::sfx2::cTokenSeperator;
                sFile += aDlg.GetCurrentFilter();
            }
            else
                sFile.Erase();
        }
        break;

    case OBJECT_CLIENT_OLE:
        {
            nType = FILETYPE_OBJECT;        // falls noch nicht gesetzt
            Window* pOld = Application::GetDefDialogParent();
            Application::SetDefDialogParent( pParent );

            SfxMediumRef _xMed = SFX_APP()->InsertDocumentDialog( 0, String() );
            if( _xMed.Is() )
            {
                sFile = _xMed->GetName();
                sFile += ::sfx2::cTokenSeperator;
// Bereich!         sFile += _xMed->GetFilter()->GetName();
                sFile += ::sfx2::cTokenSeperator;
                sFile += _xMed->GetFilter()->GetFilterName();
            }
            else
                sFile.Erase();

            Application::SetDefDialogParent( pOld );
        }
        break;

    case OBJECT_CLIENT_FILE:
        {
            nType = FILETYPE_TEXT;      // falls noch nicht gesetzt
            Window* pOld = Application::GetDefDialogParent();
            Application::SetDefDialogParent( pParent );

            const SfxObjectFactory* pFactory=0;
            SfxObjectShell* pShell = pLink->GetLinkManager()->GetPersist();
            if ( pShell )
                pFactory = &pShell->GetFactory();

            SfxMediumRef _xMed = SFX_APP()->InsertDocumentDialog( 0, pFactory ? pFactory->GetFactoryName() : String() );
            if( _xMed.Is() )
            {
                sFile = _xMed->GetName();
                sFile += ::sfx2::cTokenSeperator;
// Bereich!         sFile += _xMed->GetFilter()->GetName();
                sFile += ::sfx2::cTokenSeperator;
                sFile += _xMed->GetFilter()->GetFilterName();
            }
            else
                sFile.Erase();

            Application::SetDefDialogParent( pOld );
        }
        break;

    default:
        sFile.Erase();
    }

    return sFile;
}

IMPL_STATIC_LINK( SvFileObject, LoadGrfReady_Impl, void*, EMPTYARG )
{
    // wenn wir von hier kommen, kann es kein Fehler mehr sein
    pThis->bLoadError = FALSE;
    pThis->bWaitForData = FALSE;
    pThis->bInCallDownLoad = FALSE;

    if( !pThis->bInNewData && !pThis->bDataReady )
    {
            // Grafik ist fertig, also DataChanged von der Status-
            // aederung schicken:
        pThis->bDataReady = TRUE;
        pThis->SendStateChg_Impl( STATE_LOAD_OK );

            // und dann nochmal die Daten senden
        pThis->NotifyDataChanged();
    }

    if( pThis->bDataReady )
    {
        pThis->bLoadAgain = TRUE;
        if( pThis->xMed.Is() )
        {
            pThis->xMed->SetDataAvailableLink( Link() );
            pThis->xMed->SetDoneLink( Link() );

            Application::PostUserEvent(
                        STATIC_LINK( pThis, SvFileObject, DelMedium_Impl ),
                        new SfxMediumRef( pThis->xMed ));
            pThis->xMed.Clear();
        }
        if( pThis->pDownLoadData )
            delete pThis->pDownLoadData, pThis->pDownLoadData = 0;
    }

    return 0;
}

IMPL_STATIC_LINK( SvFileObject, DelMedium_Impl, SfxMediumRef*, pDelMed )
{
    (void)pThis;
    delete pDelMed;
    return 0;
}

IMPL_STATIC_LINK( SvFileObject, LoadGrfNewData_Impl, void*, EMPTYARG )
{
    // wenn wir von hier kommen, kann es kein Fehler mehr sein
    if( pThis->bInNewData )
        return 0;

    pThis->bInNewData = TRUE;
    pThis->bLoadError = FALSE;

    if( !pThis->pDownLoadData )
    {
        pThis->pDownLoadData = new Impl_DownLoadData(
                        STATIC_LINK( pThis, SvFileObject, LoadGrfNewData_Impl ) );

        // Null-Link setzen, damit keine temporaeren Grafiken
        // rausgeswapt werden; der Filter prueft, ob schon
        // ein Link gesetzt ist => falls dies zutrifft, wird
        // _kein_ neuer Link gesetzt; der Link muss hier gesetzt werden,
        // (bevor das erste Mal gefiltert wird), um zu verhindern,
        // dass der Kontext zurueckgesetzt wird (aynchrones Laden)
        if( !pThis->bNativFormat )
        {
            static GfxLink aDummyLink;
            pThis->pDownLoadData->aGrf.SetLink( aDummyLink );
        }
    }

    pThis->NotifyDataChanged();

    SvStream* pStrm = pThis->xMed.Is() ? pThis->xMed->GetInStream() : 0;
    if( pStrm && pStrm->GetError() )
    {
        if( ERRCODE_IO_PENDING == pStrm->GetError() )
            pStrm->ResetError();

        // im DataChanged ein DataReady?
        else if( pThis->bWaitForData && pThis->pDownLoadData )
        {
            pThis->bLoadError = TRUE;
        }
    }

    if( pThis->bDataReady )
    {
        // Grafik ist fertig, also DataChanged von der Status-
        // aederung schicken:
        pThis->SendStateChg_Impl( pStrm->GetError() ? STATE_LOAD_ERROR
                                                    : STATE_LOAD_OK );
    }

    pThis->bInNewData = FALSE;
    return 0;
}


/*  [Beschreibung]

    Die Methode stellt fest, ob aus einem DDE-Object die Daten gelesen
    werden kann.
    Zurueckgegeben wird:
        ERRCODE_NONE            wenn sie komplett gelesen wurde
        ERRCODE_SO_PENDING      wenn sie noch nicht komplett gelesen wurde
        ERRCODE_SO_FALSE        sonst
*/
BOOL SvFileObject::IsPending() const
{
    return FILETYPE_GRF == nType && !bLoadError &&
            ( pDownLoadData || bWaitForData );
}
BOOL SvFileObject::IsDataComplete() const
{
    BOOL bRet = FALSE;
    if( FILETYPE_GRF != nType )
        bRet = TRUE;
    else if( !bLoadError && ( !bWaitForData && !pDownLoadData ))
    {
        SvFileObject* pThis = (SvFileObject*)this;
        if( bDataReady ||
            ( bSynchron && pThis->LoadFile_Impl() && xMed.Is() ) )
            bRet = TRUE;
        else
        {
            INetURLObject aUrl( sFileNm );
            if( aUrl.HasError() ||
                INET_PROT_NOT_VALID == aUrl.GetProtocol() )
                bRet = TRUE;
        }
    }
    return bRet;
}



void SvFileObject::CancelTransfers()
{
    if( xMed.Is() )
        xMed->CancelTransfers();

    // und aus dem Cache austragen, wenn man mitten im Laden ist
    if( !bDataReady )
    {
        // nicht noch mal aufsetzen
        bLoadAgain = FALSE;
        bDataReady = bLoadError = bWaitForData = TRUE;
        SendStateChg_Impl( STATE_LOAD_ABORT );
    }
}


void SvFileObject::SetTransferPriority( USHORT )
{
}


void SvFileObject::SendStateChg_Impl( LinkState nState )
{
    if( !bStateChangeCalled && HasDataLinks() )
    {
        Any aAny;
        aAny <<= rtl::OUString::valueOf( (sal_Int32)nState );
        DataChanged( SotExchange::GetFormatName(
                        SvxLinkManager::RegisterStatusInfoId()), aAny );
        bStateChangeCalled = TRUE;
    }
}


