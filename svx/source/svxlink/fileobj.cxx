/*************************************************************************
 *
 *  $RCSfile: fileobj.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:26 $
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

#ifdef OS2
#include <vcl/sysdep.hxx>
#endif


#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _FILTER_HXX //autogen
#include <svtools/filter.hxx>
#endif
#ifndef _SOERR_HXX //autogen
#include <so3/soerr.hxx>
#endif
#ifndef _LNKBASE_HXX //autogen
#include <so3/lnkbase.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_PROGRESS_HXX //autogen
#include <sfx2/progress.hxx>
#endif
#ifndef _SFX_INTERNO_HXX //autogen
#include <sfx2/interno.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif

#include "fileobj.hxx"
#include "linkmgr.hxx"
#include "fobjcach.hxx"
#include "dialmgr.hxx"
#include "dialogs.hrc"
#include "xoutbmp.hxx"          // XOutBitmap
#include "impgrf.hxx"
#include "dialdll.hxx"

// es gibt wohl ein paar Compiler-Fehler beim optimieren
#pragma optimize( "", off )

#define FILETYPE_TEXT       1
#define FILETYPE_GRF        2

class SvxFileObjProgress_Impl : public SfxProgress
{
public:
    SvxFileObjProgress_Impl( const String& rStr )
        : SfxProgress( 0, rStr, 100, TRUE, FALSE )
    {}

    DECL_STATIC_LINK( SvxFileObjProgress_Impl, UpdatePercentHdl, GraphicFilter* );
};


class ImplGrfCastBaseLink : public SvBaseLink
{
public:
    ImplGrfCastBaseLink() : SvBaseLink( 0, 0 ) {}
    void SetObject( SvPseudoObject* pObj ) { SetObj( pObj ); }
};

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
    : nType( FILETYPE_TEXT ), pDownLoadData( 0 )
{
    bLoadAgain = bMedUseCache = TRUE;
    bSynchron = bLoadError = bWaitForData = bDataReady = bNativFormat =
    bClearMedium = bProgress = FALSE;
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


const SvDataTypeList & SvFileObject::GetTypeList() const
{
    return aTypeList;
}


BOOL SvFileObject::GetData( SvData * pSvData )
{
    aTypeList.Clear();
    switch( nType )
    {
    case FILETYPE_TEXT:
        if( FORMAT_FILE == pSvData->GetFormat() )
        {
            // das Medium muss in der Applikation geoffnet werden, um die
            // relativen Datei Links aufzuloesen!!!! Wird ueber den
            // LinkManager und damit von dessen Storage erledigt.
            pSvData->SetData( sFileNm );

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
            aTypeList.Append( *pSvData );
        }
        break;

    case FILETYPE_GRF:
        if( !bLoadError )
        {
            SfxMediumRef xTmpMed;
            ULONG nFmt = pSvData->GetFormat(),
                    nGrfFmtId = Graphic::RegisterClipboardFormatName();

            if( FORMAT_GDIMETAFILE == nFmt || FORMAT_BITMAP == nFmt ||
                nGrfFmtId == nFmt )
            {
                Graphic aGrf;

                //JP 15.07.98: Bug 52959
                //      falls das Nativformat doch erwuenscht ist, muss am
                //      Ende das Flag zurueckgesetzt werden.
                BOOL bOldNativFormat = bNativFormat;
                bNativFormat = 0 != (ASPECT_ICON & pSvData->GetAspect());

                // falls gedruckt werden soll, warten wir bis die
                // Daten vorhanden sind
                if( ASPECT_DOCPRINT & pSvData->GetAspect() )
                {

                    // testhalber mal ein LoadFile rufen um das nach-
                    // laden ueberahaupt anzustossen
                    if( !xMed.Is() )
                    {
                        LoadFile_Impl();
                        if( xMed.Is() )
                            // dann mit der hoechsten Prioritaet
                            xMed->SetTransferPriority( SFX_TFPRIO_VISIBLE_LOWRES_GRAPHIC );
                    }

                    xTmpMed = xMed;
                    while( bWaitForData )
                        Application::Reschedule();

                    xMed = xTmpMed;
                }

                if( pDownLoadData ||
                    ( !bWaitForData && ( xMed.Is() ||       // wurde als URL geladen
                        ( bSynchron && LoadFile_Impl() && xMed.Is() ) )) )
                {
                    // falls

                    // falls es uebers Internet gesogen wurde, nicht
                    // wieder versuchen
                    bLoadAgain = !xMed->IsRemote();
                    bLoadError = !GetGraphic_Impl( aGrf, xMed->GetInStream() );
                    if( bLoadError )
                        GetCache()->Remove( *this );
                }
                else if( !LoadFile_Impl() ||
                        !GetGraphic_Impl( aGrf, xMed.Is() ? xMed->GetInStream() : 0 ))
                {
                    if( !xMed.Is() )
                        break;
                    aGrf.SetDefaultType();
                }

                if( nGrfFmtId != nFmt )
                    nFmt = (bLoadError || GRAPHIC_BITMAP == aGrf.GetType())
                                ? FORMAT_BITMAP
                                : FORMAT_GDIMETAFILE;

                GDIMetaFile aMeta;
                Bitmap aBmp;
                SvData aData( nFmt );
                if( nGrfFmtId == nFmt )
                    aData.SetData( &aGrf, TRANSFER_REFERENCE );
                else if( FORMAT_BITMAP == nFmt )
                {
                    aBmp = aGrf.GetBitmap();
                    aData.SetData( &aBmp, TRANSFER_REFERENCE );
                }
                else
                {
                    aMeta = aGrf.GetGDIMetaFile();
                    aData.SetData( &aMeta, TRANSFER_REFERENCE );
                }

                *pSvData = aData;
                aTypeList.Append( *pSvData );

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
    }
    return 0 != aTypeList.Count();
}




BOOL SvFileObject::Connect( SvBaseLink& rLink )
{
    if( !rLink.GetLinkManager() )
        return FALSE;

    // teste doch mal, ob nicht ein anderer Link mit der gleichen
    // Verbindung schon existiert
    rLink.GetLinkManager()->GetDisplayNames( rLink, 0, &sFileNm, 0, &sFilter );

    SvPseudoObject* pObj = this;
    if( OBJECT_CLIENT_GRF == rLink.GetObjType() )
    {
        if( !rLink.IsUseCache() )
            bMedUseCache = FALSE;

        // Reload-Erkennung ???
        SvPersist* pPersist = rLink.GetLinkManager()->GetCacheContainer();
        SvInPlaceObjectRef aRef( pPersist );
        if( aRef.Is() )
        {
            SfxObjectShell* pShell = ((SfxInPlaceObject*)&aRef)->GetObjectShell();
            if( pShell->IsAbortingImport() )
                return FALSE;

            if( pShell->IsReloading() )
                bMedUseCache = FALSE;

            if( pShell->GetMedium() )
                sReferer = pShell->GetMedium()->GetName();
        }

        FileObjCacheEntry_Impl* pEntry = GetCache()->Insert( *this, sFileNm );
        pObj = pEntry->GetObject();
    }

    if( pObj == this )
    {
        switch( rLink.GetObjectType() )
        {
        case OBJECT_CLIENT_GRF:
            nType = FILETYPE_GRF;
            bSynchron = rLink.IsSynchron();
            break;

        case OBJECT_CLIENT_FILE:
            nType = FILETYPE_TEXT;
            break;

        default:
            return FALSE;
        }

        SetUpdateTimeout( 0 );
    }
    else
        bProgress = FALSE;

    // und jetzt bei diesem oder gefundenem Pseudo-Object anmelden
    pObj->AddDataAdvise( &rLink, rLink.GetContentType(),
                        (bProgress ? ADVISEMODE_ONLYONCE : 0 ));
    // um ueber Status-Aenderungen informiert zu werden
    pObj->AddDataAdvise( &rLink, SvxLinkManager::RegisterStatusInfoId(),
                            ADVISEMODE_ONLYONCE );
    pObj->AddConnectAdvise( &rLink, ADVISE_CLOSED );

    if( pObj != this )
        // am Link das richtige Object setzen
        ((ImplGrfCastBaseLink&)rLink).SetObject( pObj );

    return TRUE;
}


BOOL SvFileObject::LoadFile_Impl()
{
    // wir sind noch im Laden!!
    if( bWaitForData || !bLoadAgain || xMed.Is() || pDownLoadData )
        return FALSE;

    // z.Z. nur auf die aktuelle DocShell
    xMed = new SfxMedium( sFileNm, STREAM_STD_READ, TRUE );
    // Keinen Eintrag im Roter Button Menu
    xMed->SetDontCreateCancellable();
    xMed->SetUsesCache( bMedUseCache );
    if( sReferer.Len() )
        xMed->SetReferer( sReferer );
    // erstmal mit der niedrigsten Prioritaet
    xMed->SetTransferPriority( SFX_TFPRIO_INVISIBLE_HIGHRES_GRAPHIC );

    if( !bSynchron )
    {
        bLoadAgain = bDataReady = bInNewData = FALSE;
        bWaitForData = TRUE;

        SfxMediumRef xTmpMed = xMed;
        xMed->SetDataAvailableLink( STATIC_LINK( this, SvFileObject, LoadGrfNewData_Impl ) );
        xMed->DownLoad( STATIC_LINK( this, SvFileObject, LoadGrfReady_Impl ) );

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
    Link aPercentLnk;
    GraphicFilter* pGF = GetGrfFilter();
    SvxFileObjProgress_Impl* pProgress = 0;
    if( bProgress && !SfxGetpApp()->GetProgress() )
    {
        pProgress = new SvxFileObjProgress_Impl(
            String( ResId( RID_SVXSTR_GRFLINKPROGRESS, DIALOG_MGR() ) ) );

        aPercentLnk = pGF->GetUpdatePercentHdl();
        pGF->SetUpdatePercentHdl(
            STATIC_LINK( pProgress, SvxFileObjProgress_Impl, UpdatePercentHdl ));
    }

    const int nFilter = sFilter.Len() && pGF->GetImportFormatCount()
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
                         : pGF->ImportGraphic( rGrf, sFileNm, nFilter );
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

    if( pProgress )
    {
        pGF->SetUpdatePercentHdl( aPercentLnk );
        delete pProgress;

        // Statusaederung schicken:
        SendStateChg_Impl( GRFILTER_OK == nRes ? STATE_LOAD_OK : STATE_LOAD_ERROR );
    }

    return GRFILTER_OK == nRes;
}


SvLinkName* SvFileObject::Edit( Window* pParent, const SvBaseLink& rLink )
{
    if( !rLink.GetLinkManager() )
        return 0;

    String sFile, sRange, sTmpFilter;
    rLink.GetLinkManager()->GetDisplayNames( rLink, 0, &sFile, &sRange,
                                                        &sTmpFilter );

    SvLinkName* pNewNm = 0;
    if( OBJECT_CLIENT_GRF == rLink.GetObjectType() )
    {
        nType = FILETYPE_GRF;       // falls noch nicht gesetzt

        SvxImportGraphicDialog* pDlg = new SvxImportGraphicDialog( pParent,
                String( ResId( RID_SVXSTR_EDITGRFLINK, DIALOG_MGR() ) ), FALSE );

        pDlg->SetPath( sFile, FALSE );
        pDlg->SetCurFilter( sTmpFilter );

        if( RET_OK == pDlg->Execute() )
        {
            sFile = pDlg->GetPath();
            sFile += cTokenSeperator;
            sFile += cTokenSeperator;
            sFile += pDlg->GetCurFilter();
            pNewNm = new SvLinkName( sFile );
        }
        delete pDlg;
    }
    else if( OBJECT_CLIENT_FILE == rLink.GetObjectType() )
    {
        nType = FILETYPE_TEXT;      // falls noch nicht gesetzt
        Window* pOld = Application::GetDefModalDialogParent();
        Application::SetDefModalDialogParent( pParent );

        const SfxObjectFactory* pFactory;
        SvPersist* pPersist = rLink.GetLinkManager()->GetCacheContainer();
        SvInPlaceObjectRef aRef( pPersist );
        if( aRef.Is() )
        {
            SfxObjectShell* pShell = ((SfxInPlaceObject*)&aRef)->GetObjectShell();
            pFactory = &pShell->GetFactory();
        }
        else
            pFactory = &SFX_APP()->GetDefaultFactory();
        SfxMediumRef xMed = SFX_APP()->InsertDocumentDialog( 0, *pFactory );

        if( xMed.Is() )
        {
            sFile = xMed->GetName();
            sFile += cTokenSeperator;
// Bereich!         sFile += xMed->GetFilter()->GetName();
            sFile += cTokenSeperator;
            sFile += xMed->GetFilter()->GetFilterName();
            pNewNm = new SvLinkName( sFile );
        }
        Application::SetDefModalDialogParent( pOld );
    }
    return pNewNm;
}


IMPL_STATIC_LINK( SvxFileObjProgress_Impl, UpdatePercentHdl, GraphicFilter *, pFilter )
{
    pThis->SetState( pFilter->GetPercent() );
    return 0;
}


IMPL_STATIC_LINK( SvFileObject, LoadGrfReady_Impl, void*, EMPTYARG )
{
    // wenn wir von hier kommen, kann es kein Fehler mehr sein
    pThis->bLoadError = FALSE;
    pThis->bWaitForData = FALSE;
    if( !pThis->bInNewData && !pThis->bDataReady )
    {
            // Grafik ist fertig, also DataChanged von der Status-
            // aederung schicken:
        pThis->bDataReady = TRUE;
        pThis->SendStateChg_Impl( STATE_LOAD_OK );

            // und dann nochmal die Daten senden (muss das noch sein??)
        SvData aSvData;
        pThis->DataChanged( aSvData );
    }

    if( pThis->bDataReady )
    {
        pThis->bLoadAgain = TRUE;
        pThis->xMed->SetDataAvailableLink( Link() );
        pThis->xMed->SetDoneLink( Link() );

        Application::PostUserEvent(
                    STATIC_LINK( pThis, SvFileObject, DelMedium_Impl ),
                    new SfxMediumRef( pThis->xMed ));
        if( pThis->pDownLoadData )
            delete pThis->pDownLoadData, pThis->pDownLoadData = 0;
        pThis->xMed.Clear();
    }

    return 0;
}

IMPL_STATIC_LINK( SvFileObject, DelMedium_Impl, SfxMediumRef*, pDelMed )
{
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

    SvData aSvData;
    pThis->DataChanged( aSvData );

    SvStream* pStrm = pThis->xMed->GetInStream();
    if( pStrm && pStrm->GetError() )
    {
        if( ERRCODE_IO_PENDING == pStrm->GetError() )
            pStrm->ResetError();

        // im DataChanged ein DataReady?
        else if( pThis->bWaitForData && pThis->pDownLoadData )
        {
            pThis->bLoadError = TRUE;
            GetCache()->Remove( *pThis );
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


IMPL_STATIC_LINK( SvFileObject, OldCacheGrf_Impl, void*, EMPTYARG )
{
    ULONG nCnt = pThis->GetSelectorCount();
    if( nCnt )
    {
        SvAdvSelectorList& rSelLst = pThis->GetSelectorList();
        SvAdvDataSelector * pD;

        while( nCnt-- )
            if( 0 != ( pD = PTR_CAST( SvAdvDataSelector,
                                            rSelLst.GetObject( nCnt ) )) )
            {
                SvBaseLinkRef xLnkRef( pD->GetSink() );
                if( xLnkRef.Is() && OBJECT_CLIENT_GRF == xLnkRef->GetObjType() )
                {
                    SvPersist* pPersist = xLnkRef->GetLinkManager()->GetCacheContainer();
                    SvInPlaceObjectRef xRef( pPersist );
                    if ( xRef.Is() )
                        SfxObjectShell* pShell = ((SfxInPlaceObject*)&xRef)->GetObjectShell();
                }
            }
    }
    return 0;
}



ULONG SvFileObject::GetUpToDateStatus()
/*  [Beschreibung]

    Die Methode stellt fest, ob aus einem Grafik-Object die Grafik gelesen
    werden kann.
    Zurueckgegeben wird:
        ERRCODE_NONE            wenn sie komplett gelesen wurde
        ERRCODE_SO_PENDING      wenn sie noch nicht komplett gelesen wurde
        ERRCODE_SO_FALSE        sonst
*/
{
    ULONG nRet = ERRCODE_SO_FALSE;
    if( FILETYPE_GRF == nType && !bLoadError )
    {
        if( pDownLoadData || bWaitForData )
            nRet = ERRCODE_SO_PENDING;
        else if( !bWaitForData )
        {
            if( bDataReady || ( bSynchron && LoadFile_Impl() && xMed.Is() ) )
                nRet = ERRCODE_NONE;
            else
            {
                INetURLObject aUrl( sFileNm );

                if( aUrl.HasError() ||
                    INET_PROT_NOT_VALID == aUrl.GetProtocol() ||
                    INET_PROT_FILE == aUrl.GetProtocol() )
                    nRet = ERRCODE_NONE;
            }
        }

    }
    return nRet;
}


void SvFileObject::CancelTransfers()
{
    if( xMed.Is() )
        xMed->CancelTransfers();

    // und aus dem Cache austragen, wenn man mitten im Laden ist
    if( !bDataReady )
    {
        SvxGrfLinkData_Impl* pData = *(SvxGrfLinkData_Impl**)GetAppData( SHL_LNKCCH );
        if( pData->pCache )
            pData->pCache->Remove( *this );

        // nicht noch mal aufsetzen
        bLoadAgain = bMedUseCache = FALSE;
        bDataReady = bLoadError = bWaitForData = TRUE;
        SendStateChg_Impl( STATE_LOAD_ABORT );
    }
}


void SvFileObject::SetTransferPriority( USHORT nPrio )
{
    if( xMed.Is() )
        xMed->SetTransferPriority( nPrio );
}


void SvFileObject::SendStateChg_Impl( USHORT nState )
{
    if( GetSelectorCount() )
    {
        ULONG nId = SvxLinkManager::RegisterStatusInfoId();
        SvData aData( nId );
        String sData( String::CreateFromInt32( nState ));
        aData.SetData( sData );
        DataChanged( aData );
    }
}

/**/


FileObjCacheEntry_Impl::FileObjCacheEntry_Impl(
    SvFileObject& rObj, const String& rName )
    : pPrev( 0 ), pNext( 0 ),
    xObj( &rObj ),
    sFileName( rName )
{
}


FileObjCacheEntry_Impl::~FileObjCacheEntry_Impl()
{
// xObj->pImpCachePointer = 0;
}


/**/


FileObjCache_Impl::FileObjCache_Impl( USHORT nMaxCnt )
    : pFirst( 0 ), pLast( 0 ), nSize( 0 ), nMaxSize( nMaxCnt )
{
    StartListening( *SFX_APP() );
}


FileObjCache_Impl::~FileObjCache_Impl()
{
    while( pFirst )
    {
        FileObjCacheEntry_Impl* p = pFirst->pNext;
        delete pFirst;
        pFirst = p;
    }
}


    // FileObject einfuegen. Wenn doppelt, dann returne das schon
    // vorhandene, ansonsten lege ein neues an.

FileObjCacheEntry_Impl* FileObjCache_Impl::Insert(
        SvFileObject& rObj, const String& rName )
{
    FileObjCacheEntry_Impl* p = pFirst;
    while( p && p->sFileName != rName )
        p = p->pNext;

    if( !p )        // nicht gefunden, also neu anlegen
    {
        p = new FileObjCacheEntry_Impl( rObj, rName );
        if( !nSize++ )
        {
            pFirst = pLast = p;
            return pFirst;
        }
        else if( nSize > nMaxSize )     // letzten entfernen
        {
            // evtuell nach vorne suchen, ob es einen gibt, der nicht
            // mehr verbunden ist ?
            pLast = pLast->pPrev;
            delete pLast->pNext;
            pLast->pNext = 0;
            --nSize;
        }
    }
    else if( p != pFirst )      // gefunden, also nach vorne verschieben
    {
        if( p->pNext )
            p->pNext->pPrev = p->pPrev;
        else if( p == pLast )           // das muss der Letzte sein!
            pLast = p->pPrev ? p->pPrev : p;

        if( p->pPrev )
        {
            p->pPrev->pNext = p->pNext;
            p->pPrev = 0;
        }
    }
    else
        return pFirst;          // der gefundene

    p->pNext = pFirst;
    pFirst->pPrev = p;
    pFirst = p;

    return pFirst;
}

    // enferne dieses Object

void FileObjCache_Impl::Remove( SvFileObject& rObj )
{
    FileObjCacheEntry_Impl* p = pFirst;
    while( p && &p->xObj != &rObj )
        p = p->pNext;

    if( p )
    {
        // ausketten
        if( p == pFirst )
            pFirst = p->pNext;
        if( p == pLast )
            pLast = p->pPrev;

        if( p->pNext )
            p->pNext->pPrev = p->pPrev;

        if( p->pPrev )
            p->pPrev->pNext = p->pNext;

        delete p;
        --nSize;
    }
}

    // enferne dieses Object

void FileObjCache_Impl::Remove( const String& rName )
{
    FileObjCacheEntry_Impl* p = pFirst;
    while( p && p->sFileName != rName )
        p = p->pNext;

    if( p )
    {
        // ausketten
        if( p == pFirst )
            pFirst = p->pNext;
        if( p == pLast )
            pLast = p->pPrev;

        if( p->pNext )
            p->pNext->pPrev = p->pPrev;

        if( p->pPrev )
            p->pPrev->pNext = p->pNext;

        delete p;
        --nSize;
    }
}


void FileObjCache_Impl::SFX_NOTIFY( SfxBroadcaster& , const TypeId& ,
                                    const SfxHint& rHint, const TypeId& )
{
    if( rHint.ISA( SfxSimpleHint ) && SFX_HINT_DEINITIALIZING ==
        ((SfxSimpleHint&)rHint).GetId() )
    {
        // die Applikation wird herunter gefahren, lasse alle Mediums los
        while( pFirst )
        {
            FileObjCacheEntry_Impl* p = pFirst->pNext;
            delete pFirst;
            pFirst = p;
        }

        EndListening( *SFX_APP() );
    }
}

    // suche nach einem Object mit dem Namen

FileObjCacheEntry_Impl* FileObjCache_Impl::Found( const String& rName )
{
    FileObjCacheEntry_Impl* p = pFirst;
    while( p && p->sFileName != rName )
        p = p->pNext;
    return p;
}


/**/

SvxGrfLinkCacheDll::SvxGrfLinkCacheDll()
{
    DBG_ASSERT( !(*GetAppData(SHL_LNKCCH)), "Ctor, but pointer not null" );

    (*(SvxGrfLinkData_Impl**)GetAppData(SHL_LNKCCH)) = new SvxGrfLinkData_Impl;
}


SvxGrfLinkCacheDll::~SvxGrfLinkCacheDll()
{
    DBG_ASSERT( (*GetAppData(SHL_LNKCCH)), "Dtor, pointer == null" );

    delete (*(SvxGrfLinkData_Impl**)GetAppData(SHL_LNKCCH));
    (*(SvxGrfLinkData_Impl**)GetAppData(SHL_LNKCCH)) = 0;
}

