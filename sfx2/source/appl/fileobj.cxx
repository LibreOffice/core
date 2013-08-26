/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <sot/formats.hxx>
#include <vcl/graphicfilter.hxx>
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
#include <vcl/dibtools.hxx>

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
        xMed->SetDoneLink( Link() );
        xMed.Clear();
    }
    delete pDownLoadData;
}


sal_Bool SvFileObject::GetData( ::com::sun::star::uno::Any & rData,
                                const OUString & rMimeType,
                                sal_Bool bGetSynchron )
{
    sal_uIntPtr nFmt = SotExchange::GetFormatStringId( rMimeType );
    switch( nType )
    {
    case FILETYPE_TEXT:
        if( FORMAT_FILE == nFmt )
        {
            // The media in the application must be opened to lookup the
            // relative file links!! This is done through the link manager
            // of the Storage.
            rData <<= OUString( sFileNm );
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

                // If the native format is reqested, has to be reset at the
                // end of the flag. Is solely in the sw/ndgrf.cxx used when
                // the link is removed form GraphicNode.
                sal_Bool bOldNativFormat = bNativFormat;

                // If about to print, waiting for the data to be available
                if( bGetSynchron )
                {
                    // call a LoadFile every second time to test the loading
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
                    ( !bWaitForData && ( xMed.Is() ||  // was loaded as URL
                        ( bSynchron && LoadFile_Impl() && xMed.Is() ) )) )
                {
                    // If it was loaded from the Internet, do not retry
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
                {
                    const Bitmap aBitmap(aGrf.GetBitmap());

                    if(!aBitmap.IsEmpty())
                    {
                        WriteDIB(aBitmap, aMemStm, false, true);
                    }

                    break;
                }

                default:
                    if( aGrf.GetGDIMetaFile().GetActionSize() )
                    {
                        GDIMetaFile aMeta( aGrf.GetGDIMetaFile() );
                        aMeta.Write( aMemStm );
                    }
                }
                rData <<= css::uno::Sequence< sal_Int8 >( (sal_Int8*) aMemStm.GetData(),
                                        aMemStm.Seek( STREAM_SEEK_TO_END ) );

                bNativFormat = bOldNativFormat;

                // Everything ready?
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
        rData <<= OUString( sFileNm );
        break;
    }
    return sal_True/*0 != aTypeList.Count()*/;
}

sal_Bool SvFileObject::Connect( sfx2::SvBaseLink* pLink )
{
    if( !pLink || !pLink->GetLinkManager() )
        return sal_False;

    // Test if not another link of the same connection already exists
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

    // and now register by this or other found Pseudo-Object
    AddDataAdvise( pLink, SotExchange::GetFormatMimeType( pLink->GetContentType()), 0 );
    return sal_True;
}

sal_Bool SvFileObject::LoadFile_Impl()
{
    // We are still at Loading!!
    if( bWaitForData || !bLoadAgain || xMed.Is() || pDownLoadData )
        return sal_False;

    // at the moment on the current DocShell
    xMed = new SfxMedium( sFileNm, STREAM_STD_READ );
    SvLinkSource::StreamToLoadFrom aStreamToLoadFrom =
        getStreamToLoadFrom();
    xMed->setStreamToLoadFrom(
        aStreamToLoadFrom.m_xInputStreamToLoadFrom,
        aStreamToLoadFrom.m_bIsReadOnly);

    if( !bSynchron )
    {
        bLoadAgain = bDataReady = bInNewData = sal_False;
        bWaitForData = sal_True;

        SfxMediumRef xTmpMed = xMed;
        bInCallDownLoad = sal_True;
        xMed->DownLoad( STATIC_LINK( this, SvFileObject, LoadGrfReady_Impl ) );
        bInCallDownLoad = sal_False;

        bClearMedium = !xMed.Is();
        if( bClearMedium )
            xMed = xTmpMed;  // If already finished in DownLoad
        return bDataReady;
    }

    bWaitForData = sal_True;
    bDataReady = bInNewData = sal_False;
    xMed->DownLoad();
    bLoadAgain = !xMed->IsRemote();
    bWaitForData = sal_False;

    // Graphic is finished, also send DataChanged of the Status change:
    SendStateChg_Impl( xMed->GetInStream() && xMed->GetInStream()->GetError()
                        ? sfx2::LinkManager::STATE_LOAD_ERROR : sfx2::LinkManager::STATE_LOAD_OK );
    return sal_True;
}


sal_Bool SvFileObject::GetGraphic_Impl( Graphic& rGrf, SvStream* pStream )
{
    GraphicFilter& rGF = GraphicFilter::GetGraphicFilter();

    const sal_uInt16 nFilter = !sFilter.isEmpty() && rGF.GetImportFormatCount()
                            ? rGF.GetImportFormatNumber( sFilter )
                            : GRFILTER_FORMAT_DONTKNOW;

    String aEmptyStr;
    int nRes;

    // To avoid that a native link is created
    if( ( !pStream || !pDownLoadData ) && !rGrf.IsLink() &&
        !rGrf.GetContext() && !bNativFormat )
        rGrf.SetLink( GfxLink() );

    if( !pStream )
        nRes = xMed.Is() ? GRFILTER_OPENERROR
                         : rGF.ImportGraphic( rGrf, INetURLObject(sFileNm),
                            nFilter );
    else if( !pDownLoadData )
    {
        pStream->Seek( STREAM_SEEK_TO_BEGIN );
        nRes = rGF.ImportGraphic( rGrf, aEmptyStr, *pStream, nFilter );
    }
    else
    {
        nRes = rGF.ImportGraphic( pDownLoadData->aGrf, aEmptyStr,
                                    *pStream, nFilter );

        if( pDownLoadData )
        {
            rGrf = pDownLoadData->aGrf;
            if( GRAPHIC_NONE == rGrf.GetType() )
                rGrf.SetDefaultType();


            if( !pDownLoadData->aGrf.GetContext() )
            {
                delete pDownLoadData, pDownLoadData = 0;
                bDataReady = sal_True;
                bWaitForData = sal_False;
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
            DBG_WARNING3( "Graphic error [%d] - [%s] URL[%s]",
                            nRes,
                            xMed->GetPhysicalName().getStr(),
                            sFileNm.getStr() );
        }
        else
        {
            DBG_WARNING2( "Graphic error [%d] - [%s]",
                            nRes, sFileNm.getStr() );
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
                OUString("com.sun.star.document.TypeDetection") ),
                css::uno::UNO_QUERY );
        if ( xTypeDetection.is() )
        {
            ::comphelper::MediaDescriptor aDescr;
            aDescr[ ::comphelper::MediaDescriptor::PROP_URL() ] <<= OUString( _rURL );
            css::uno::Sequence< css::beans::PropertyValue > aDescrList =
                aDescr.getAsConstPropertyValueList();
            OUString sType = xTypeDetection->queryTypeByDescriptor( aDescrList, sal_True );
            if ( !sType.isEmpty() )
            {
                css::uno::Reference< css::container::XNameAccess > xTypeCont( xTypeDetection,
                                                                              css::uno::UNO_QUERY );
                if ( xTypeCont.is() )
                {
                    ::comphelper::SequenceAsHashMap lTypeProps( xTypeCont->getByName( sType ) );
                    sFilter = lTypeProps.getUnpackedValueOrDefault(
                        OUString("PreferredFilter"), OUString() );
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
    OUString sFile, sRange, sTmpFilter;
    if( pLink && pLink->GetLinkManager() )
    {
        pLink->GetLinkManager()->GetDisplayNames( pLink, 0, &sFile, &sRange, &sTmpFilter );

        switch( pLink->GetObjType() )
        {
            case OBJECT_CLIENT_GRF:
            {
                nType = FILETYPE_GRF;       // If not set already

                SvxOpenGraphicDialog aDlg(SfxResId(RID_SVXSTR_EDITGRFLINK).toString());
                aDlg.EnableLink(sal_False);
                aDlg.SetPath( sFile, sal_True );
                aDlg.SetCurrentFilter( sTmpFilter );

                if( !aDlg.Execute() )
                {
                    sFile = aDlg.GetPath();
                    sFile += OUString(::sfx2::cTokenSeparator);
                    sFile += OUString(::sfx2::cTokenSeparator);
                    sFile += aDlg.GetCurrentFilter();

                    if ( aEndEditLink.IsSet() )
                        aEndEditLink.Call( &sFile );
                }
                else
                    sFile = "";
            }
            break;

            case OBJECT_CLIENT_OLE:
            {
                nType = FILETYPE_OBJECT; // if not set already
                pOldParent = Application::GetDefDialogParent();
                Application::SetDefDialogParent( pParent );

                ::sfx2::FileDialogHelper & rFileDlg =
                    pLink->GetInsertFileDialog( String() );
                rFileDlg.StartExecuteModal(
                        LINK( this, SvFileObject, DialogClosedHdl ) );
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

                ::sfx2::FileDialogHelper & rFileDlg =
                    pLink->GetInsertFileDialog(sFactory);
                rFileDlg.StartExecuteModal(
                        LINK( this, SvFileObject, DialogClosedHdl ) );
            }
            break;

            default:
                sFile = "";
        }
    }
}

IMPL_STATIC_LINK( SvFileObject, LoadGrfReady_Impl, void*, EMPTYARG )
{
    // When we come form here there it can not be an error no more.
    pThis->bLoadError = sal_False;
    pThis->bWaitForData = sal_False;
    pThis->bInCallDownLoad = sal_False;

    if( !pThis->bInNewData && !pThis->bDataReady )
    {
        // Graphic is finished, also send DataChanged from Status change
        pThis->bDataReady = sal_True;
        pThis->SendStateChg_Impl( sfx2::LinkManager::STATE_LOAD_OK );

        // and then send the data again
        pThis->NotifyDataChanged();
    }

    if( pThis->bDataReady )
    {
        pThis->bLoadAgain = sal_True;
        if( pThis->xMed.Is() )
        {
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
            sFile += ::sfx2::cTokenSeparator;
            sFile += ::sfx2::cTokenSeparator;
            sFile += impl_getFilter( sURL );
        }
    }
    else
    {
        SAL_WARN( "sfx.appl", "SvFileObject::DialogClosedHdl(): wrong file type" );
    }

    if ( aEndEditLink.IsSet() )
        aEndEditLink.Call( &sFile );
    return 0;
}

/*  [Description]

    The method determines whether the data-object can be read from a DDE.

    The following can be returned:
        ERRCODE_NONE                    if it has been completely read
        ERRCODE_SO_PENDING              if it has not been completely read
        ERRCODE_SO_FALSE                otherwise
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
    // unsubscribe from the cache if in the middle of loading
    if( !bDataReady )
    {
        // Do not set-up again
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
        aAny <<= OUString::number( nState );
        DataChanged( SotExchange::GetFormatName(
                        sfx2::LinkManager::RegisterStatusInfoId()), aAny );
        bStateChangeCalled = sal_True;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
