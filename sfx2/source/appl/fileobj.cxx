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

#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <sot/formats.hxx>
#include <sal/log.hxx>
#include <sfx2/lnkbase.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sot/exchange.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <sfx2/docfac.hxx>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <unotools/mediadescriptor.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/opengrf.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/objsh.hxx>
#include "fileobj.hxx"
#include <sfx2/strings.hrc>
#include <vcl/svapp.hxx>

enum class SvFileObjectType
{
    Text = 1, Graphic = 2, Object = 3
};

SvFileObject::SvFileObject()
    : nPostUserEventId(nullptr)
    , mxDelMed()
    , nType(SvFileObjectType::Text)
    , bLoadAgain(true)
    , bSynchron(false)
    , bLoadError(false)
    , bWaitForData(false)
    , bDataReady(false)
    , bClearMedium(false)
    , bStateChangeCalled(false)
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
                                bool /*bGetSynchron*/ )
{
    SotClipboardFormatId nFmt = SotExchange::RegisterFormatMimeType( rMimeType );
    switch( nType )
    {
    case SvFileObjectType::Text:
        if( SotClipboardFormatId::SIMPLE_FILE == nFmt )
        {
            // The media in the application must be opened to lookup the
            // relative file links!! This is done through the link manager
            // of the Storage.
            rData <<= sFileNm;
        }
        break;

    case SvFileObjectType::Graphic:
        if (SotClipboardFormatId::GDIMETAFILE == nFmt
         || SotClipboardFormatId::BITMAP == nFmt
         || SotClipboardFormatId::SVXB == nFmt)
        {
            rData <<= sFileNm;
        }
        break;
    case SvFileObjectType::Object:
        // TODO/LATER: possibility to insert a new object
        rData <<= sFileNm;
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

    if( sfx2::SvBaseLinkObjectType::ClientGraphic == pLink->GetObjType() )
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
    case sfx2::SvBaseLinkObjectType::ClientGraphic:
        nType = SvFileObjectType::Graphic;
        bSynchron = pLink->IsSynchron();
        break;

    case sfx2::SvBaseLinkObjectType::ClientFile:
        nType = SvFileObjectType::Text;
        break;

    case sfx2::SvBaseLinkObjectType::ClientOle:
        nType = SvFileObjectType::Object;
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
        bLoadAgain = bDataReady = false;
        bWaitForData = true;

        tools::SvRef<SfxMedium> xTmpMed = xMed;
        xMed->Download( LINK( this, SvFileObject, LoadGrfReady_Impl ) );

        bClearMedium = !xMed.is();
        if( bClearMedium )
            xMed = xTmpMed;  // If already finished in Download
        return bDataReady;
    }

    bWaitForData = true;
    bDataReady = false;
    xMed->Download();
    bLoadAgain = !xMed->IsRemote();
    bWaitForData = false;

    // Graphic is finished, also send DataChanged of the Status change:
    SendStateChg_Impl( xMed->GetInStream() && xMed->GetInStream()->GetError()
                        ? sfx2::LinkManager::STATE_LOAD_ERROR : sfx2::LinkManager::STATE_LOAD_OK );
    return true;
}


/** detect the filter of the given file

    @param _rURL
        specifies the URL of the file which filter is to detected.<br/>
        If the URL doesn't denote a valid (existent and accessible) file, the
        request is silently dropped.
*/
static OUString impl_getFilter( const OUString& _rURL )
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
            aDescr[ utl::MediaDescriptor::PROP_URL() ] <<= _rURL;
            css::uno::Sequence< css::beans::PropertyValue > aDescrList =
                aDescr.getAsConstPropertyValueList();
            OUString sType = xTypeDetection->queryTypeByDescriptor( aDescrList, true );
            if ( !sType.isEmpty() )
            {
                // Honor a selected/detected filter.
                for (const auto& rDescr : std::as_const(aDescrList))
                {
                    if (rDescr.Name == "FilterName")
                    {
                        if (rDescr.Value >>= sFilter)
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

void SvFileObject::Edit(weld::Window* pParent, sfx2::SvBaseLink* pLink, const Link<const OUString&, void>& rEndEditHdl)
{
    aEndEditLink = rEndEditHdl;
    OUString sFile, sRange, sTmpFilter;
    if( !pLink || !pLink->GetLinkManager() )
        return;

    sfx2::LinkManager::GetDisplayNames( pLink, nullptr, &sFile, &sRange, &sTmpFilter );

    switch( pLink->GetObjType() )
    {
        case sfx2::SvBaseLinkObjectType::ClientGraphic:
        {
            nType = SvFileObjectType::Graphic;       // If not set already

            SvxOpenGraphicDialog aDlg(SfxResId(RID_SVXSTR_EDITGRFLINK), pParent);
            aDlg.EnableLink(false);
            aDlg.SetPath( sFile, true );
            aDlg.SetCurrentFilter( sTmpFilter );

            if( !aDlg.Execute() )
            {
                sFile = aDlg.GetPath()
                    + OUStringChar(sfx2::cTokenSeparator)
                    + OUStringChar(sfx2::cTokenSeparator)
                    + aDlg.GetDetectedFilter();

                aEndEditLink.Call( sFile );
            }
            else
                sFile.clear();
        }
        break;

        case sfx2::SvBaseLinkObjectType::ClientOle:
        {
            nType = SvFileObjectType::Object; // if not set already

            ::sfx2::FileDialogHelper & rFileDlg =
                pLink->GetInsertFileDialog( OUString() );
            rFileDlg.StartExecuteModal(
                    LINK( this, SvFileObject, DialogClosedHdl ) );
        }
        break;

        case sfx2::SvBaseLinkObjectType::ClientFile:
        {
            nType = SvFileObjectType::Text; // if not set already

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

IMPL_LINK_NOARG( SvFileObject, LoadGrfReady_Impl, void*, void )
{
    // When we come from here there it can not be an error no more.
    bLoadError = false;
    bWaitForData = false;

    if( !bDataReady )
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

    if ( SvFileObjectType::Text == nType || SvFileObjectType::Object == nType )
    {
        if ( _pFileDlg && _pFileDlg->GetError() == ERRCODE_NONE )
        {
            OUString sURL( _pFileDlg->GetPath() );
            sFile = sURL + OUStringChar(sfx2::cTokenSeparator)
                + OUStringChar(sfx2::cTokenSeparator)
                + impl_getFilter( sURL );
        }
    }
    else
    {
        SAL_WARN( "sfx.appl", "SvFileObject::DialogClosedHdl(): wrong file type" );
    }

    aEndEditLink.Call( sFile );
}

/*
    The method determines whether the data-object can be read from a DDE.
*/
bool SvFileObject::IsPending() const
{
    return SvFileObjectType::Graphic == nType && !bLoadError && bWaitForData;
}

bool SvFileObject::IsDataComplete() const
{
    bool bRet = false;
    if( SvFileObjectType::Graphic != nType )
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
                        sfx2::LinkManager::RegisterStatusInfoId()), css::uno::makeAny(OUString::number( nState )) );
        bStateChangeCalled = true;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
