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
#include <unotools/mediadescriptor.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/opengrf.hxx>
#include <sfx2/sfxresid.hxx>
#include "fileobj.hxx"
#include "app.hrc"
#include <vcl/dibtools.hxx>

#define FILETYPE_TEXT       1
#define FILETYPE_GRF        2
#define FILETYPE_OBJECT     3

SvFileObject::SvFileObject()
    : nPostUserEventId(nullptr)
    , mxDelMed()
    , nType(FILETYPE_TEXT)
    , bLoadAgain(true)
    , bSynchron(false)
    , bLoadError(false)
    , bWaitForData(false)
    , bInNewData(false)
    , bDataReady(false)
    , bNativFormat(false)
    , bClearMedium(false)
    , bStateChangeCalled(false)
    , bInCallDownload(false)
{
}

SvFileObject::~SvFileObject()
{
    if (xMed.is())
    {
        xMed->SetDoneLink( Link<void*,void>() );
        xMed.clear();
    }
    if (nPostUserEventId)
        Application::RemoveUserEvent(nPostUserEventId);
}

bool SvFileObject::GetData( css::uno::Any & rData,
                                const OUString & rMimeType,
                                bool bGetSynchron )
{
    SotClipboardFormatId nFmt = SotExchange::RegisterFormatMimeType( rMimeType );
    switch( nType )
    {
    case FILETYPE_TEXT:
        if( SotClipboardFormatId::SIMPLE_FILE == nFmt )
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
            tools::SvRef<SfxMedium> xTmpMed;

            if( SotClipboardFormatId::GDIMETAFILE == nFmt || SotClipboardFormatId::BITMAP == nFmt ||
                SotClipboardFormatId::SVXB == nFmt )
            {
                Graphic aGrf;

                // If the native format is requested, has to be reset at the
                // end of the flag. Is solely in the sw/ndgrf.cxx used when
                // the link is removed form GraphicNode.
                bool bOldNativFormat = bNativFormat;

                // If about to print, waiting for the data to be available
                if( bGetSynchron )
                {
                    // call a LoadFile every second time to test the loading
                    if( !xMed.is() )
                        LoadFile_Impl();

                    if( !bInCallDownload )
                    {
                        xTmpMed = xMed;
                        while( bWaitForData )
                            Application::Reschedule();

                        xMed = xTmpMed;
                        bClearMedium = true;
                    }
                }

                if( !bWaitForData && ( xMed.is() ||  // was loaded as URL
                      ( bSynchron && LoadFile_Impl() && xMed.is() ) ) )
                {
                    // If it was loaded from the Internet, do not retry
                    if( !bGetSynchron )
                        bLoadAgain = !xMed->IsRemote();
                    bLoadError = !GetGraphic_Impl( aGrf, xMed->GetInStream() );
                }
                else if( !LoadFile_Impl() ||
                        !GetGraphic_Impl( aGrf, xMed.is() ? xMed->GetInStream() : nullptr ))
                {
                    if( !xMed.is() )
                        break;
                    aGrf.SetDefaultType();
                }

                if( SotClipboardFormatId::SVXB != nFmt )
                    nFmt = (bLoadError || GraphicType::Bitmap == aGrf.GetType())
                                ? SotClipboardFormatId::BITMAP
                                : SotClipboardFormatId::GDIMETAFILE;

                SvMemoryStream aMemStm( 0, 65535 );
                switch ( nFmt )
                {
                case SotClipboardFormatId::SVXB:
                    if( GraphicType::NONE != aGrf.GetType() )
                    {
                        aMemStm.SetVersion( SOFFICE_FILEFORMAT_50 );
                        WriteGraphic( aMemStm, aGrf );
                    }
                    break;

                case SotClipboardFormatId::BITMAP:
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
                rData <<= css::uno::Sequence< sal_Int8 >( static_cast<sal_Int8 const *>(aMemStm.GetData()),
                                        aMemStm.Seek( STREAM_SEEK_TO_END ) );

                bNativFormat = bOldNativFormat;

                // Everything ready?
                if( xMed.is() && !bSynchron && bClearMedium )
                {
                    xMed.clear();
                    bClearMedium = false;
                }
            }
        }
        break;
    case FILETYPE_OBJECT:
        // TODO/LATER: possibility to insert a new object
        rData <<= OUString( sFileNm );
        break;
    }
    return true/*0 != aTypeList.Count()*/;
}

bool SvFileObject::Connect( sfx2::SvBaseLink* pLink )
{
    if( !pLink || !pLink->GetLinkManager() )
        return false;

    // Test if not another link of the same connection already exists
    sfx2::LinkManager::GetDisplayNames( pLink, nullptr, &sFileNm, nullptr, &sFilter );

    if( OBJECT_CLIENT_GRF == pLink->GetObjType() )
    {
        SfxObjectShellRef pShell = pLink->GetLinkManager()->GetPersist();
        if( pShell.is() )
        {
            if( pShell->IsAbortingImport() )
                return false;

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
        return false;
    }

    SetUpdateTimeout( 0 );

    // and now register by this or other found Pseudo-Object
    AddDataAdvise( pLink, SotExchange::GetFormatMimeType( pLink->GetContentType()), 0 );
    return true;
}

bool SvFileObject::LoadFile_Impl()
{
    // We are still at Loading!!
    if( bWaitForData || !bLoadAgain || xMed.is() )
        return false;

    // at the moment on the current DocShell
    xMed = new SfxMedium( sFileNm, sReferer, StreamMode::STD_READ );
    SvLinkSource::StreamToLoadFrom aStreamToLoadFrom =
        getStreamToLoadFrom();
    xMed->setStreamToLoadFrom(
        aStreamToLoadFrom.m_xInputStreamToLoadFrom,
        aStreamToLoadFrom.m_bIsReadOnly);

    if( !bSynchron )
    {
        bLoadAgain = bDataReady = bInNewData = false;
        bWaitForData = true;

        tools::SvRef<SfxMedium> xTmpMed = xMed;
        bInCallDownload = true;
        xMed->Download( LINK( this, SvFileObject, LoadGrfReady_Impl ) );
        bInCallDownload = false;

        bClearMedium = !xMed.is();
        if( bClearMedium )
            xMed = xTmpMed;  // If already finished in Download
        return bDataReady;
    }

    bWaitForData = true;
    bDataReady = bInNewData = false;
    xMed->Download();
    bLoadAgain = !xMed->IsRemote();
    bWaitForData = false;

    // Graphic is finished, also send DataChanged of the Status change:
    SendStateChg_Impl( xMed->GetInStream() && xMed->GetInStream()->GetError()
                        ? sfx2::LinkManager::STATE_LOAD_ERROR : sfx2::LinkManager::STATE_LOAD_OK );
    return true;
}


bool SvFileObject::GetGraphic_Impl( Graphic& rGrf, SvStream* pStream )
{
    GraphicFilter& rGF = GraphicFilter::GetGraphicFilter();

    const sal_uInt16 nFilter = !sFilter.isEmpty() && rGF.GetImportFormatCount()
                            ? rGF.GetImportFormatNumber( sFilter )
                            : GRFILTER_FORMAT_DONTKNOW;

    int nRes;

    // To avoid that a native link is created
    if( !rGrf.IsLink() &&
        !rGrf.GetContext() && !bNativFormat )
        rGrf.SetLink( GfxLink() );

    if( !pStream )
        nRes = xMed.is() ? GRFILTER_OPENERROR
                         : rGF.ImportGraphic( rGrf, INetURLObject(sFileNm),
                            nFilter );
    else
    {
        pStream->Seek( STREAM_SEEK_TO_BEGIN );

        // #i123042# for e.g. SVG the path is needed, see same TaskID in svx for more info
        nRes = rGF.ImportGraphic( rGrf, sFileNm, *pStream, nFilter );
    }

    if( pStream && ERRCODE_IO_PENDING == pStream->GetError() )
        pStream->ResetError();

    if( nRes )
    {
        if( xMed.is() && !pStream )
            SAL_WARN( "sfx.appl", "Graphic error [" << nRes << "] - [" << xMed->GetPhysicalName() << "] URL[" << sFileNm << "]" );
        else
            SAL_WARN( "sfx.appl", "Graphic error [" << nRes << "] - [" << sFileNm << "]" );
    }

    return GRFILTER_OK == nRes;
}

/** detect the filter of the given file

    @param _rURL
        specifies the URL of the file which filter is to detected.<br/>
        If the URL doesn't denote a valid (existent and accessible) file, the
        request is silently dropped.
*/
OUString impl_getFilter( const OUString& _rURL )
{
    OUString sFilter;
    if ( _rURL.isEmpty() )
        return sFilter;

    try
    {
        css::uno::Reference< css::document::XTypeDetection > xTypeDetection(
            ::comphelper::getProcessServiceFactory()->createInstance( "com.sun.star.document.TypeDetection" ),
            css::uno::UNO_QUERY );
        if ( xTypeDetection.is() )
        {
            utl::MediaDescriptor aDescr;
            aDescr[ utl::MediaDescriptor::PROP_URL() ] <<= OUString( _rURL );
            css::uno::Sequence< css::beans::PropertyValue > aDescrList =
                aDescr.getAsConstPropertyValueList();
            OUString sType = xTypeDetection->queryTypeByDescriptor( aDescrList, true );
            if ( !sType.isEmpty() )
            {
                // Honor a selected/detected filter.
                for (sal_Int32 i=0; i < aDescrList.getLength(); ++i)
                {
                    if (aDescrList[i].Name == "FilterName")
                    {
                        if (aDescrList[i].Value >>= sFilter)
                            break;
                    }
                }
                if (sFilter.isEmpty())
                {
                    css::uno::Reference< css::container::XNameAccess > xTypeCont( xTypeDetection,
                            css::uno::UNO_QUERY );
                    if ( xTypeCont.is() )
                    {
                        /* XXX: for fdo#69948 scenario the sequence returned by
                         * getByName() contains an empty PreferredFilter
                         * property value (since? expected?) */
                        ::comphelper::SequenceAsHashMap lTypeProps( xTypeCont->getByName( sType ) );
                        sFilter = lTypeProps.getUnpackedValueOrDefault(
                                "PreferredFilter", OUString() );
                    }
                }
            }
        }
    }
    catch( const css::uno::Exception& )
    {
    }

    return sFilter;
}

void SvFileObject::Edit( vcl::Window* /*pParent*/, sfx2::SvBaseLink* pLink, const Link<const OUString&, void>& rEndEditHdl )
{
    aEndEditLink = rEndEditHdl;
    OUString sFile, sRange, sTmpFilter;
    if( pLink && pLink->GetLinkManager() )
    {
        sfx2::LinkManager::GetDisplayNames( pLink, nullptr, &sFile, &sRange, &sTmpFilter );

        switch( pLink->GetObjType() )
        {
            case OBJECT_CLIENT_GRF:
            {
                nType = FILETYPE_GRF;       // If not set already

                SvxOpenGraphicDialog aDlg(SfxResId(RID_SVXSTR_EDITGRFLINK).toString());
                aDlg.EnableLink(false);
                aDlg.SetPath( sFile, true );
                aDlg.SetCurrentFilter( sTmpFilter );

                if( !aDlg.Execute() )
                {
                    sFile = aDlg.GetPath()
                        + OUStringLiteral1(sfx2::cTokenSeparator)
                        + OUStringLiteral1(sfx2::cTokenSeparator)
                        + aDlg.GetCurrentFilter();

                    aEndEditLink.Call( sFile );
                }
                else
                    sFile.clear();
            }
            break;

            case OBJECT_CLIENT_OLE:
            {
                nType = FILETYPE_OBJECT; // if not set already

                ::sfx2::FileDialogHelper & rFileDlg =
                    pLink->GetInsertFileDialog( OUString() );
                rFileDlg.StartExecuteModal(
                        LINK( this, SvFileObject, DialogClosedHdl ) );
            }
            break;

            case OBJECT_CLIENT_FILE:
            {
                nType = FILETYPE_TEXT; // if not set already

                OUString sFactory;
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
                sFile.clear();
        }
    }
}

IMPL_LINK_NOARG( SvFileObject, LoadGrfReady_Impl, void*, void )
{
    // When we come form here there it can not be an error no more.
    bLoadError = false;
    bWaitForData = false;
    bInCallDownload = false;

    if( !bInNewData && !bDataReady )
    {
        // Graphic is finished, also send DataChanged from Status change
        bDataReady = true;
        SendStateChg_Impl( sfx2::LinkManager::STATE_LOAD_OK );

        // and then send the data again
        NotifyDataChanged();
    }

    if( bDataReady )
    {
        bLoadAgain = true;
        if( xMed.is() )
        {
            xMed->SetDoneLink( Link<void*,void>() );
            mxDelMed = xMed;
            nPostUserEventId = Application::PostUserEvent(
                        LINK( this, SvFileObject, DelMedium_Impl ));
            xMed.clear();
        }
    }
}

IMPL_LINK_NOARG( SvFileObject, DelMedium_Impl, void*, void )
{
    nPostUserEventId = nullptr;
    mxDelMed.clear();
}

IMPL_LINK( SvFileObject, DialogClosedHdl, sfx2::FileDialogHelper*, _pFileDlg, void )
{
    OUString sFile;

    if ( FILETYPE_TEXT == nType || FILETYPE_OBJECT == nType )
    {
        if ( _pFileDlg && _pFileDlg->GetError() == ERRCODE_NONE )
        {
            OUString sURL( _pFileDlg->GetPath() );
            sFile = sURL + OUStringLiteral1(sfx2::cTokenSeparator)
                + OUStringLiteral1(sfx2::cTokenSeparator)
                + impl_getFilter( sURL );
        }
    }
    else
    {
        SAL_WARN( "sfx.appl", "SvFileObject::DialogClosedHdl(): wrong file type" );
    }

    aEndEditLink.Call( sFile );
}

/*  [Description]

    The method determines whether the data-object can be read from a DDE.

    The following can be returned:
        ERRCODE_NONE                    if it has been completely read
        ERRCODE_SO_PENDING              if it has not been completely read
        ERRCODE_SO_FALSE                otherwise
*/
bool SvFileObject::IsPending() const
{
    return FILETYPE_GRF == nType && !bLoadError && bWaitForData;
}

bool SvFileObject::IsDataComplete() const
{
    bool bRet = false;
    if( FILETYPE_GRF != nType )
        bRet = true;
    else if( !bLoadError && !bWaitForData )
    {
        SvFileObject* pThis = const_cast<SvFileObject*>(this);
        if( bDataReady ||
            ( bSynchron && pThis->LoadFile_Impl() && xMed.is() ) )
            bRet = true;
        else
        {
            INetURLObject aUrl( sFileNm );
            if( aUrl.HasError() ||
                INetProtocol::NotValid == aUrl.GetProtocol() )
                bRet = true;
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
        bLoadAgain = false;
        bDataReady = bLoadError = bWaitForData = true;
        SendStateChg_Impl( sfx2::LinkManager::STATE_LOAD_ABORT );
    }
}


void SvFileObject::SendStateChg_Impl( sfx2::LinkManager::LinkState nState )
{
    if( !bStateChangeCalled && HasDataLinks() )
    {
        DataChanged( SotExchange::GetFormatName(
                        sfx2::LinkManager::RegisterStatusInfoId()), css::uno::Any(OUString::number( nState )) );
        bStateChangeCalled = true;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
