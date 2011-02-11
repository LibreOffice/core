/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <sot/formats.hxx>
#include <svtools/filter.hxx>
#include <sfx2/lnkbase.hxx>
#include <sfx2/app.hxx>
#include <sfx2/progress.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sot/exchange.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <sfx2/docfac.hxx>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <comphelper/mediadescriptor.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/opengrf.hxx>
#include "sfx2/sfxresid.hxx"
#include "fileobj.hxx"
#include "app.hrc"

namespace css = ::com::sun::star;

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


SvFileObject::SvFileObject() :
    pDownLoadData( NULL ), pOldParent( NULL ), nType( FILETYPE_TEXT )
{
    bLoadAgain = sal_True;
    bSynchron = bLoadError = bWaitForData = bDataReady = bNativFormat =
    bClearMedium = bStateChangeCalled = bInCallDownLoad = sal_False;
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


sal_Bool SvFileObject::GetData( ::com::sun::star::uno::Any & rData,
                                const String & rMimeType,
                                sal_Bool bGetSynchron )
{
    sal_uIntPtr nFmt = SotExchange::GetFormatStringId( rMimeType );
    switch( nType )
    {
    case FILETYPE_TEXT:
        if( FORMAT_FILE == nFmt )
        {
            // das Medium muss in der Applikation geoffnet werden, um die
            // relativen Datei Links aufzuloesen!!!! Wird ueber den
            // LinkManager und damit von dessen Storage erledigt.
            rData <<= rtl::OUString( sFileNm );
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
                sal_Bool bOldNativFormat = bNativFormat;
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
                        bClearMedium = sal_True;
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
                rData <<= css::uno::Sequence< sal_Int8 >( (sal_Int8*) aMemStm.GetData(),
                                        aMemStm.Seek( STREAM_SEEK_TO_END ) );

                bNativFormat = bOldNativFormat;

                // alles fertig?
                if( xMed.Is() && !bSynchron && bClearMedium )
                {
                    xMed.Clear();
                    bClearMedium = sal_False;
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




sal_Bool SvFileObject::Connect( sfx2::SvBaseLink* pLink )
{
    if( !pLink || !pLink->GetLinkManager() )
        return sal_False;

    // teste doch mal, ob nicht ein anderer Link mit der gleichen
    // Verbindung schon existiert
    pLink->GetLinkManager()->GetDisplayNames( pLink, 0, &sFileNm, 0, &sFilter );

    if( OBJECT_CLIENT_GRF == pLink->GetObjType() )
    {
        SfxObjectShellRef pShell = pLink->GetLinkManager()->GetPersist();
        if( pShell.Is() )
        {
            if( pShell->IsAbortingImport() )
                return sal_False;

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
        return sal_False;
    }

    SetUpdateTimeout( 0 );

    // und jetzt bei diesem oder gefundenem Pseudo-Object anmelden
    AddDataAdvise( pLink, SotExchange::GetFormatMimeType( pLink->GetContentType()), 0 );
    return sal_True;
}


sal_Bool SvFileObject::LoadFile_Impl()
{
    // wir sind noch im Laden!!
    if( bWaitForData || !bLoadAgain || xMed.Is() || pDownLoadData )
        return sal_False;

    // z.Z. nur auf die aktuelle DocShell
    xMed = new SfxMedium( sFileNm, STREAM_STD_READ, sal_True );
    SvLinkSource::StreamToLoadFrom aStreamToLoadFrom =
        getStreamToLoadFrom();
    xMed->setStreamToLoadFrom(
        aStreamToLoadFrom.m_xInputStreamToLoadFrom,
        aStreamToLoadFrom.m_bIsReadOnly);
    // setStreamToLoadFrom(0,0);
    if( sReferer.Len() )
        xMed->SetReferer( sReferer );

    if( !bSynchron )
    {
        bLoadAgain = bDataReady = bInNewData = sal_False;
        bWaitForData = sal_True;

        SfxMediumRef xTmpMed = xMed;
        xMed->SetDataAvailableLink( STATIC_LINK( this, SvFileObject, LoadGrfNewData_Impl ) );
        bInCallDownLoad = sal_True;
        xMed->DownLoad( STATIC_LINK( this, SvFileObject, LoadGrfReady_Impl ) );
        bInCallDownLoad = sal_False;

        bClearMedium = !xMed.Is();
        if( bClearMedium )
            xMed = xTmpMed;     // falls gleich im DownLoad schon schluss ist
        return bDataReady;
    }

    bWaitForData = sal_True;
    bDataReady = bInNewData = sal_False;
    xMed->DownLoad();
    bLoadAgain = !xMed->IsRemote();
    bWaitForData = sal_False;

    // Grafik ist fertig, also DataChanged von der Statusaederung schicken:
    SendStateChg_Impl( xMed->GetInStream() && xMed->GetInStream()->GetError()
                        ? sfx2::LinkManager::STATE_LOAD_ERROR : sfx2::LinkManager::STATE_LOAD_OK );
    return sal_True;
}


sal_Bool SvFileObject::GetGraphic_Impl( Graphic& rGrf, SvStream* pStream )
{
    GraphicFilter* pGF = GraphicFilter::GetGraphicFilter();

    const sal_uInt16 nFilter = sFilter.Len() && pGF->GetImportFormatCount()
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
                bDataReady = sal_True;
                bWaitForData = sal_False;
            }
            else if( sal_False )
            {
                // Timer aufsetzen, um zurueck zukehren
                pDownLoadData->aTimer.Start();
            }
        }
    }

    if( pStream && ERRCODE_IO_PENDING == pStream->GetError() )
        pStream->ResetError();

#ifdef DBG_UTIL
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

/** detect the filter of the given file

    @param _rURL
        specifies the URL of the file which filter is to detected.<br/>
        If the URL doesn't denote a valid (existent and accessible) file, the
        request is silently dropped.
*/
String impl_getFilter( const String& _rURL )
{
    String sFilter;
    if ( _rURL.Len() == 0 )
        return sFilter;

    try
    {
        css::uno::Reference< ::com::sun::star::document::XTypeDetection > xTypeDetection(
            ::comphelper::getProcessServiceFactory()->createInstance(
                ::rtl::OUString::createFromAscii("com.sun.star.document.TypeDetection") ),
                css::uno::UNO_QUERY );
        if ( xTypeDetection.is() )
        {
            ::comphelper::MediaDescriptor aDescr;
            aDescr[ ::comphelper::MediaDescriptor::PROP_URL() ] <<= ::rtl::OUString( _rURL );
            css::uno::Sequence< css::beans::PropertyValue > aDescrList =
                aDescr.getAsConstPropertyValueList();
            ::rtl::OUString sType = xTypeDetection->queryTypeByDescriptor( aDescrList, sal_True );
            if ( sType.getLength() )
            {
                css::uno::Reference< css::container::XNameAccess > xTypeCont( xTypeDetection,
                                                                              css::uno::UNO_QUERY );
                if ( xTypeCont.is() )
                {
                    ::comphelper::SequenceAsHashMap lTypeProps( xTypeCont->getByName( sType ) );
                    sFilter = lTypeProps.getUnpackedValueOrDefault(
                        ::rtl::OUString::createFromAscii("PreferredFilter"), ::rtl::OUString() );
                }
            }
        }
    }
    catch( const css::uno::Exception& )
    {
    }

    return sFilter;
}

void SvFileObject::Edit( Window* pParent, sfx2::SvBaseLink* pLink, const Link& rEndEditHdl )
{
    aEndEditLink = rEndEditHdl;
    String sFile, sRange, sTmpFilter;
    if( pLink && pLink->GetLinkManager() )
    {
        pLink->GetLinkManager()->GetDisplayNames( pLink, 0, &sFile, &sRange, &sTmpFilter );

        switch( pLink->GetObjType() )
        {
            case OBJECT_CLIENT_GRF:
            {
                nType = FILETYPE_GRF;       // falls noch nicht gesetzt

                SvxOpenGraphicDialog aDlg(SfxResId(RID_SVXSTR_EDITGRFLINK));
                aDlg.EnableLink(sal_False);
                aDlg.SetPath( sFile, sal_True );
                aDlg.SetCurrentFilter( sTmpFilter );

                if( !aDlg.Execute() )
                {
                    sFile = aDlg.GetPath();
                    sFile += ::sfx2::cTokenSeperator;
                    sFile += ::sfx2::cTokenSeperator;
                    sFile += aDlg.GetCurrentFilter();

                    if ( aEndEditLink.IsSet() )
                        aEndEditLink.Call( &sFile );
                }
                else
                    sFile.Erase();
            }
            break;

            case OBJECT_CLIENT_OLE:
            {
                nType = FILETYPE_OBJECT; // if not set already
                pOldParent = Application::GetDefDialogParent();
                Application::SetDefDialogParent( pParent );

                ::sfx2::FileDialogHelper* pFileDlg =
                    pLink->GetFileDialog( (SFXWB_INSERT | WB_3DLOOK), String() );
                pFileDlg->StartExecuteModal( LINK( this, SvFileObject, DialogClosedHdl ) );
            }
            break;

            case OBJECT_CLIENT_FILE:
            {
                nType = FILETYPE_TEXT; // if not set already
                pOldParent = Application::GetDefDialogParent();
                Application::SetDefDialogParent( pParent );

                String sFactory;
                SfxObjectShell* pShell = pLink->GetLinkManager()->GetPersist();
                if ( pShell )
                    sFactory = pShell->GetFactory().GetFactoryName();

                ::sfx2::FileDialogHelper* pFileDlg =
                    pLink->GetFileDialog( (SFXWB_INSERT | WB_3DLOOK), sFactory );
                pFileDlg->StartExecuteModal( LINK( this, SvFileObject, DialogClosedHdl ) );
            }
            break;

            default:
                sFile.Erase();
        }
    }
}

IMPL_STATIC_LINK( SvFileObject, LoadGrfReady_Impl, void*, EMPTYARG )
{
    // wenn wir von hier kommen, kann es kein Fehler mehr sein
    pThis->bLoadError = sal_False;
    pThis->bWaitForData = sal_False;
    pThis->bInCallDownLoad = sal_False;

    if( !pThis->bInNewData && !pThis->bDataReady )
    {
            // Grafik ist fertig, also DataChanged von der Status-
            // aederung schicken:
        pThis->bDataReady = sal_True;
        pThis->SendStateChg_Impl( sfx2::LinkManager::STATE_LOAD_OK );

            // und dann nochmal die Daten senden
        pThis->NotifyDataChanged();
    }

    if( pThis->bDataReady )
    {
        pThis->bLoadAgain = sal_True;
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

    pThis->bInNewData = sal_True;
    pThis->bLoadError = sal_False;

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
            pThis->bLoadError = sal_True;
        }
    }

    if( pThis->bDataReady )
    {
        // Grafik ist fertig, also DataChanged von der Status-
        // aederung schicken:
        pThis->SendStateChg_Impl( pStrm->GetError() ? sfx2::LinkManager::STATE_LOAD_ERROR : sfx2::LinkManager::STATE_LOAD_OK );
    }

    pThis->bInNewData = sal_False;
    return 0;
}

IMPL_LINK( SvFileObject, DialogClosedHdl, sfx2::FileDialogHelper*, _pFileDlg )
{
    String sFile;
    Application::SetDefDialogParent( pOldParent );

    if ( FILETYPE_TEXT == nType || FILETYPE_OBJECT == nType )
    {
        if ( _pFileDlg && _pFileDlg->GetError() == ERRCODE_NONE )
        {
            String sURL( _pFileDlg->GetPath() );
            sFile = sURL;
            sFile += ::sfx2::cTokenSeperator;
            sFile += ::sfx2::cTokenSeperator;
            sFile += impl_getFilter( sURL );
        }
    }
    else
    {
        DBG_ERRORFILE( "SvFileObject::DialogClosedHdl(): wrong file type" );
    }

    if ( aEndEditLink.IsSet() )
        aEndEditLink.Call( &sFile );
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
sal_Bool SvFileObject::IsPending() const
{
    return FILETYPE_GRF == nType && !bLoadError &&
            ( pDownLoadData || bWaitForData );
}
sal_Bool SvFileObject::IsDataComplete() const
{
    sal_Bool bRet = sal_False;
    if( FILETYPE_GRF != nType )
        bRet = sal_True;
    else if( !bLoadError && ( !bWaitForData && !pDownLoadData ))
    {
        SvFileObject* pThis = (SvFileObject*)this;
        if( bDataReady ||
            ( bSynchron && pThis->LoadFile_Impl() && xMed.Is() ) )
            bRet = sal_True;
        else
        {
            INetURLObject aUrl( sFileNm );
            if( aUrl.HasError() ||
                INET_PROT_NOT_VALID == aUrl.GetProtocol() )
                bRet = sal_True;
        }
    }
    return bRet;
}



void SvFileObject::CancelTransfers()
{
    // und aus dem Cache austragen, wenn man mitten im Laden ist
    if( !bDataReady )
    {
        // nicht noch mal aufsetzen
        bLoadAgain = sal_False;
        bDataReady = bLoadError = bWaitForData = sal_True;
        SendStateChg_Impl( sfx2::LinkManager::STATE_LOAD_ABORT );
    }
}


void SvFileObject::SendStateChg_Impl( sfx2::LinkManager::LinkState nState )
{
    if( !bStateChangeCalled && HasDataLinks() )
    {
        css::uno::Any aAny;
        aAny <<= rtl::OUString::valueOf( (sal_Int32)nState );
        DataChanged( SotExchange::GetFormatName(
                        sfx2::LinkManager::RegisterStatusInfoId()), aAny );
        bStateChangeCalled = sal_True;
    }
}


