/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <config_features.h>

#ifdef UNX
#include <sys/stat.h>
#endif

#include <sfx2/docfile.hxx>
#include <sfx2/signaturestate.hxx>

#include <uno/mapping.hxx>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/document/XDocumentRevisionListPersistence.hpp>
#include <com/sun/star/document/LockedDocumentRequest.hpp>
#include <com/sun/star/document/LockedOnSavingRequest.hpp>
#include <com/sun/star/document/OwnLockOnDocumentRequest.hpp>
#include <com/sun/star/document/LockFileIgnoreRequest.hpp>
#include <com/sun/star/document/LockFileCorruptRequest.hpp>
#include <com/sun/star/document/ChangedByOthersRequest.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/UseBackupException.hpp>
#include <com/sun/star/embed/XOptimizedStorage.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/InteractiveLockingLockedException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkWriteException.hpp>
#include <com/sun/star/ucb/Lock.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XStreamListener.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/security/DocumentSignatureInformation.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/security/XCertificate.hpp>
#include <tools/urlobj.hxx>
#include <tools/fileutil.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/tempfile.hxx>
#include <comphelper/fileurl.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/simplefileaccessinteraction.hxx>
#include <framework/interaction.hxx>
#include <unotools/streamhelper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <utility>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/lckbitem.hxx>
#include <svtools/sfxecode.hxx>
#include <svl/itemset.hxx>
#include <svl/intitem.hxx>
#include <svtools/svparser.hxx>
#include <cppuhelper/weakref.hxx>
#include <sal/log.hxx>

#include <unotools/streamwrap.hxx>

#include <osl/file.hxx>

#include <comphelper/storagehelper.hxx>
#include <unotools/mediadescriptor.hxx>
#include <comphelper/docpasswordhelper.hxx>
#include <tools/datetime.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/asynclink.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/progresshandlerwrap.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/interactionrequest.hxx>
#include <sot/stg.hxx>
#include <sot/storage.hxx>
#include <unotools/saveopt.hxx>
#include <svl/documentlockfile.hxx>
#include <com/sun/star/document/DocumentRevisionListPersistence.hpp>

#include <helper.hxx>
#include <sfx2/request.hxx>
#include <sfx2/app.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/sfxsids.hrc>
#include <openflag.hxx>
#include <officecfg/Office/Common.hxx>
#include <comphelper/propertysequence.hxx>
#include <vcl/weld.hxx>
#include <tools/diagnose_ex.h>

#include <com/sun/star/io/WrongFormatException.hpp>

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::security;

namespace {

#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT

bool IsSystemFileLockingUsed()
{
#if HAVE_FEATURE_MACOSX_SANDBOX
    return true;
#else
    return officecfg::Office::Common::Misc::UseDocumentSystemFileLocking::get();
#endif
}


bool IsOOoLockFileUsed()
{
#if HAVE_FEATURE_MACOSX_SANDBOX
    return false;
#else
    return officecfg::Office::Common::Misc::UseDocumentOOoLockFile::get();
#endif
}

bool IsLockingUsed()
{
    return officecfg::Office::Common::Misc::UseLocking::get();
}

#endif

#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
bool IsWebDAVLockingUsed()
{
    return officecfg::Office::Common::Misc::UseWebDAVFileLocking::get();
}
#endif

/// Gets default attributes of a file:// URL.
sal_uInt64 GetDefaultFileAttributes(const OUString& rURL)
{
    sal_uInt64 nRet = 0;

    if (!comphelper::isFileUrl(rURL))
        return nRet;

    // Make sure the file exists (and create it if not).
    osl::File aFile(rURL);
    osl::File::RC nRes = aFile.open(osl_File_OpenFlag_Create);
    if (nRes != osl::File::E_None && nRes != osl::File::E_EXIST)
        return nRet;

    aFile.close();

    osl::DirectoryItem aItem;
    if (osl::DirectoryItem::get(rURL, aItem) != osl::DirectoryItem::E_None)
        return nRet;

    osl::FileStatus aStatus(osl_FileStatus_Mask_Attributes);
    if (aItem.getFileStatus(aStatus) != osl::DirectoryItem::E_None)
        return nRet;

    nRet = aStatus.getAttributes();
    return nRet;
}

/// Determines if rURL is safe to move or not.
bool IsFileMovable(const INetURLObject& rURL)
{
#ifdef MACOSX
    (void)rURL;
    // Hide extension macOS-specific file property would be lost.
    return false;
#else

    if (rURL.GetProtocol() != INetProtocol::File)
        // Not a file:// URL.
        return false;

#ifdef UNX
    OUString sPath = rURL.getFSysPath(FSysStyle::Unix);
    if (sPath.isEmpty())
        return false;

    struct stat buf;
    if (lstat(sPath.toUtf8().getStr(), &buf) != 0)
        return false;

    // Hardlink or symlink: osl::File::move() doesn't play with these nicely.
    if (buf.st_nlink > 1 || S_ISLNK(buf.st_mode))
        return false;
#elif defined _WIN32
    if (tools::IsMappedWebDAVPath(rURL))
        return false;
#endif

    return true;
#endif
}

} // anonymous namespace

class SfxMedium_Impl
{
public:
    StreamMode m_nStorOpenMode;
    ErrCode    m_eError;

    ::ucbhelper::Content aContent;
    bool bUpdatePickList:1;
    bool bIsTemp:1;
    bool bDownloadDone:1;
    bool bIsStorage:1;
    bool bUseInteractionHandler:1;
    bool bAllowDefaultIntHdl:1;
    bool bDisposeStorage:1;
    bool bStorageBasedOnInStream:1;
    bool m_bSalvageMode:1;
    bool m_bVersionsAlreadyLoaded:1;
    bool m_bLocked:1;
    bool m_bDisableUnlockWebDAV:1;
    bool m_bGotDateTime:1;
    bool m_bRemoveBackup:1;
    bool m_bOriginallyReadOnly:1;
    bool m_bOriginallyLoadedReadOnly:1;
    bool m_bTriedStorage:1;
    bool m_bRemote:1;
    bool m_bInputStreamIsReadOnly:1;
    bool m_bInCheckIn:1;
    bool m_bDisableFileSync = false;

    OUString m_aName;
    OUString m_aLogicName;
    OUString m_aLongName;

    mutable std::unique_ptr<SfxItemSet> m_pSet;
    mutable std::unique_ptr<INetURLObject> m_pURLObj;

    std::shared_ptr<const SfxFilter> m_pFilter;
    std::shared_ptr<const SfxFilter> m_pCustomFilter;

    std::unique_ptr<SvStream> m_pInStream;
    std::unique_ptr<SvStream> m_pOutStream;

    OUString    aOrigURL;
    DateTime         aExpireTime;
    SfxFrameWeakRef  wLoadTargetFrame;
    SvKeyValueIteratorRef xAttributes;

    svtools::AsynchronLink  aDoneLink;

    uno::Sequence < util::RevisionTag > aVersions;

    std::unique_ptr<::utl::TempFile> pTempFile;

    uno::Reference<embed::XStorage> xStorage;
    uno::Reference<embed::XStorage> m_xZipStorage;
    uno::Reference<io::XInputStream> m_xInputStreamToLoadFrom;
    uno::Reference<io::XInputStream> xInputStream;
    uno::Reference<io::XStream> xStream;
    uno::Reference<io::XStream> m_xLockingStream;
    uno::Reference<task::XInteractionHandler> xInteraction;

    ErrCode  nLastStorageError;

    OUString m_aBackupURL;

    // the following member is changed and makes sense only during saving
    // TODO/LATER: in future the signature state should be controlled by the medium not by the document
    //             in this case the member will hold this information
    SignatureState             m_nSignatureState;

    bool m_bHasEmbeddedObjects = false;

    util::DateTime m_aDateTime;

    explicit SfxMedium_Impl();
    ~SfxMedium_Impl();
    SfxMedium_Impl(const SfxMedium_Impl&) = delete;
    SfxMedium_Impl& operator=(const SfxMedium_Impl&) = delete;

    OUString getFilterMimeType()
        { return !m_pFilter ? OUString() : m_pFilter->GetMimeType(); }
};


SfxMedium_Impl::SfxMedium_Impl() :
    m_nStorOpenMode(SFX_STREAM_READWRITE),
    m_eError(ERRCODE_NONE),
    bUpdatePickList(true),
    bIsTemp( false ),
    bDownloadDone( true ),
    bIsStorage( false ),
    bUseInteractionHandler( true ),
    bAllowDefaultIntHdl( false ),
    bDisposeStorage( false ),
    bStorageBasedOnInStream( false ),
    m_bSalvageMode( false ),
    m_bVersionsAlreadyLoaded( false ),
    m_bLocked( false ),
    m_bDisableUnlockWebDAV( false ),
    m_bGotDateTime( false ),
    m_bRemoveBackup( false ),
    m_bOriginallyReadOnly(false),
    m_bOriginallyLoadedReadOnly(false),
    m_bTriedStorage(false),
    m_bRemote(false),
    m_bInputStreamIsReadOnly(false),
    m_bInCheckIn(false),
    aExpireTime( DateTime( DateTime::SYSTEM ) + static_cast<sal_Int32>(10) ),
    nLastStorageError( ERRCODE_NONE ),
    m_nSignatureState( SignatureState::NOSIGNATURES )
{
    aDoneLink.CreateMutex();
}


SfxMedium_Impl::~SfxMedium_Impl()
{
    aDoneLink.ClearPendingCall();

    pTempFile.reset();
    m_pSet.reset();
    m_pURLObj.reset();
}

void SfxMedium::ResetError()
{
    pImpl->m_eError = ERRCODE_NONE;
    if( pImpl->m_pInStream )
        pImpl->m_pInStream->ResetError();
    if( pImpl->m_pOutStream )
        pImpl->m_pOutStream->ResetError();
}

ErrCode const & SfxMedium::GetLastStorageCreationState()
{
    return pImpl->nLastStorageError;
}

void SfxMedium::SetError(ErrCode nError)
{
    pImpl->m_eError = nError;
}

ErrCode SfxMedium::GetErrorCode() const
{
    ErrCode lError = pImpl->m_eError;
    if(!lError && pImpl->m_pInStream)
        lError = pImpl->m_pInStream->GetErrorCode();
    if(!lError && pImpl->m_pOutStream)
        lError = pImpl->m_pOutStream->GetErrorCode();
    return lError;
}

void SfxMedium::CheckFileDate( const util::DateTime& aInitDate )
{
    GetInitFileDate( true );
    if ( pImpl->m_aDateTime.Seconds == aInitDate.Seconds
      && pImpl->m_aDateTime.Minutes == aInitDate.Minutes
      && pImpl->m_aDateTime.Hours == aInitDate.Hours
      && pImpl->m_aDateTime.Day == aInitDate.Day
      && pImpl->m_aDateTime.Month == aInitDate.Month
      && pImpl->m_aDateTime.Year == aInitDate.Year )
        return;

    uno::Reference< task::XInteractionHandler > xHandler = GetInteractionHandler();

    if ( !xHandler.is() )
        return;

    try
    {
        ::rtl::Reference< ::ucbhelper::InteractionRequest > xInteractionRequestImpl = new ::ucbhelper::InteractionRequest( uno::makeAny(
            document::ChangedByOthersRequest() ) );
        uno::Sequence< uno::Reference< task::XInteractionContinuation > > aContinuations( 3 );
        aContinuations[0] = new ::ucbhelper::InteractionAbort( xInteractionRequestImpl.get() );
        aContinuations[1] = new ::ucbhelper::InteractionApprove( xInteractionRequestImpl.get() );
        xInteractionRequestImpl->setContinuations( aContinuations );

        xHandler->handle( xInteractionRequestImpl.get() );

        ::rtl::Reference< ::ucbhelper::InteractionContinuation > xSelected = xInteractionRequestImpl->getSelection();
        if ( uno::Reference< task::XInteractionAbort >( xSelected.get(), uno::UNO_QUERY ).is() )
        {
            SetError(ERRCODE_ABORT);
        }
    }
    catch ( const uno::Exception& )
    {}
}

bool SfxMedium::DocNeedsFileDateCheck() const
{
    return ( !IsReadOnly() && ( GetURLObject().GetProtocol() == INetProtocol::File ||
                                GetURLObject().isAnyKnownWebDAVScheme() ) );
}

util::DateTime const & SfxMedium::GetInitFileDate( bool bIgnoreOldValue )
{
    if ( ( bIgnoreOldValue || !pImpl->m_bGotDateTime ) && !pImpl->m_aLogicName.isEmpty() )
    {
        try
        {
            // add a default css::ucb::XCommandEnvironment
            // in order to have the WebDAV UCP provider manage http/https authentication correctly
            ::ucbhelper::Content aContent( GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE ),
                                           utl::UCBContentHelper::getDefaultCommandEnvironment(),
                                           comphelper::getProcessComponentContext() );

            aContent.getPropertyValue("DateModified") >>= pImpl->m_aDateTime;
            pImpl->m_bGotDateTime = true;
        }
        catch ( const css::uno::Exception& )
        {
        }
    }

    return pImpl->m_aDateTime;
}


Reference < XContent > SfxMedium::GetContent() const
{
    if ( !pImpl->aContent.get().is() )
    {
        Reference < css::ucb::XContent > xContent;

        // tdf#95144 add a default css::ucb::XCommandEnvironment
        // in order to have the WebDAV UCP provider manage https protocol certificates correctly
        css:: uno::Reference< task::XInteractionHandler > xIH(
                css::task::InteractionHandler::createWithParent( comphelper::getProcessComponentContext(), nullptr ) );

        css::uno::Reference< css::ucb::XProgressHandler > xProgress;
        ::ucbhelper::CommandEnvironment* pCommandEnv = new ::ucbhelper::CommandEnvironment( new comphelper::SimpleFileAccessInteraction( xIH ), xProgress );

        Reference < css::ucb::XCommandEnvironment > xEnv( static_cast< css::ucb::XCommandEnvironment* >(pCommandEnv), css::uno::UNO_QUERY );

        const SfxUnoAnyItem* pItem = SfxItemSet::GetItem<SfxUnoAnyItem>(pImpl->m_pSet.get(), SID_CONTENT, false);
        if ( pItem )
            pItem->GetValue() >>= xContent;

        if ( xContent.is() )
        {
            try
            {
                pImpl->aContent = ::ucbhelper::Content( xContent, xEnv, comphelper::getProcessComponentContext() );
            }
            catch ( const Exception& )
            {
            }
        }
        else
        {
            // TODO: SAL_WARN( "sfx.doc", "SfxMedium::GetContent()\nCreate Content? This code exists as fallback only. Please clarify, why its used.");
            OUString aURL;
            if ( !pImpl->m_aName.isEmpty() )
                osl::FileBase::getFileURLFromSystemPath( pImpl->m_aName, aURL );
            else if ( !pImpl->m_aLogicName.isEmpty() )
                aURL = GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE );
            if (!aURL.isEmpty() )
                (void)::ucbhelper::Content::create( aURL, xEnv, comphelper::getProcessComponentContext(), pImpl->aContent );
        }
    }

    return pImpl->aContent.get();
}

OUString SfxMedium::GetBaseURL( bool bForSaving )
{
    OUString aBaseURL;
    const SfxStringItem* pBaseURLItem = GetItemSet()->GetItem<SfxStringItem>(SID_DOC_BASEURL);
    if ( pBaseURLItem )
        aBaseURL = pBaseURLItem->GetValue();
    else if (!utl::ConfigManager::IsFuzzing() && GetContent().is())
    {
        try
        {
            Any aAny = pImpl->aContent.getPropertyValue("BaseURI");
            aAny >>= aBaseURL;
        }
        catch ( const css::uno::Exception& )
        {
        }

        if ( aBaseURL.isEmpty() )
            aBaseURL = GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE );
    }

    if ( bForSaving )
    {
        SvtSaveOptions aOpt;
        bool bIsRemote = IsRemote();
        if( (bIsRemote && !aOpt.IsSaveRelINet()) || (!pImpl->m_bRemote && !aOpt.IsSaveRelFSys()) )
            return OUString();
    }

    return aBaseURL;
}

bool SfxMedium::IsSkipImages()
{
    const SfxStringItem* pSkipImagesItem = GetItemSet()->GetItem<SfxStringItem>(SID_FILE_FILTEROPTIONS);
    return pSkipImagesItem && pSkipImagesItem->GetValue() == "SkipImages";
}

SvStream* SfxMedium::GetInStream()
{
    if ( pImpl->m_pInStream )
        return pImpl->m_pInStream.get();

    if ( pImpl->pTempFile )
    {
        pImpl->m_pInStream.reset( new SvFileStream(pImpl->m_aName, pImpl->m_nStorOpenMode) );

        pImpl->m_eError = pImpl->m_pInStream->GetError();

        if (!pImpl->m_eError && (pImpl->m_nStorOpenMode & StreamMode::WRITE)
                    && ! pImpl->m_pInStream->IsWritable() )
        {
            pImpl->m_eError = ERRCODE_IO_ACCESSDENIED;
            pImpl->m_pInStream.reset();
        }
        else
            return pImpl->m_pInStream.get();
    }

    GetMedium_Impl();

    if ( GetError() )
        return nullptr;

    return pImpl->m_pInStream.get();
}


void SfxMedium::CloseInStream()
{
    CloseInStream_Impl();
}

void SfxMedium::CloseInStream_Impl()
{
    // if there is a storage based on the InStream, we have to
    // close the storage, too, because otherwise the storage
    // would use an invalid ( deleted ) stream.
    if ( pImpl->m_pInStream && pImpl->xStorage.is() )
    {
        if ( pImpl->bStorageBasedOnInStream )
            CloseStorage();
    }

    if ( pImpl->m_pInStream && !GetContent().is() )
    {
        CreateTempFile();
        return;
    }

    pImpl->m_pInStream.reset();
    if ( pImpl->m_pSet )
        pImpl->m_pSet->ClearItem( SID_INPUTSTREAM );

    CloseZipStorage_Impl();
    pImpl->xInputStream.clear();

    if ( !pImpl->m_pOutStream )
    {
        // output part of the stream is not used so the whole stream can be closed
        // TODO/LATER: is it correct?
        pImpl->xStream.clear();
        if ( pImpl->m_pSet )
            pImpl->m_pSet->ClearItem( SID_STREAM );
    }
}


SvStream* SfxMedium::GetOutStream()
{
    if ( !pImpl->m_pOutStream )
    {
        // Create a temp. file if there is none because we always
        // need one.
        CreateTempFile( false );

        if ( pImpl->pTempFile )
        {
            // On windows we try to re-use XOutStream from xStream if that exists;
            // because opening new SvFileStream in this situation may fail with ERROR_SHARING_VIOLATION
            // TODO: this is a horrible hack that should probably be removed,
            // somebody needs to investigate this more thoroughly...
            if (getenv("SFX_MEDIUM_REUSE_STREAM") && pImpl->xStream.is())
            {
                assert(pImpl->xStream->getOutputStream().is()); // need that...
                pImpl->m_pOutStream = utl::UcbStreamHelper::CreateStream(
                        pImpl->xStream, false);
            }
            else
            {
            // On Unix don't try to re-use XOutStream from xStream if that exists;
            // it causes fdo#59022 (fails opening files via SMB on Linux)
                pImpl->m_pOutStream.reset( new SvFileStream(
                            pImpl->m_aName, StreamMode::STD_READWRITE) );
            }
            CloseStorage();
        }
    }

    return pImpl->m_pOutStream.get();
}


void SfxMedium::CloseOutStream()
{
    CloseOutStream_Impl();
}

void SfxMedium::CloseOutStream_Impl()
{
    if ( pImpl->m_pOutStream )
    {
        // if there is a storage based on the OutStream, we have to
        // close the storage, too, because otherwise the storage
        // would use an invalid ( deleted ) stream.
        //TODO/MBA: how to deal with this?!
        //maybe we need a new flag when the storage was created from the outstream
        if ( pImpl->xStorage.is() )
        {
                CloseStorage();
        }

        pImpl->m_pOutStream.reset();
    }

    if ( !pImpl->m_pInStream )
    {
        // input part of the stream is not used so the whole stream can be closed
        // TODO/LATER: is it correct?
        pImpl->xStream.clear();
        if ( pImpl->m_pSet )
            pImpl->m_pSet->ClearItem( SID_STREAM );
    }
}


const OUString& SfxMedium::GetPhysicalName() const
{
    if ( pImpl->m_aName.isEmpty() && !pImpl->m_aLogicName.isEmpty() )
        const_cast<SfxMedium*>(this)->CreateFileStream();

    // return the name then
    return pImpl->m_aName;
}


void SfxMedium::CreateFileStream()
{
    // force synchron
    if( pImpl->m_pInStream )
    {
        SvLockBytes* pBytes = pImpl->m_pInStream->GetLockBytes();
        if( pBytes )
            pBytes->SetSynchronMode();
    }

    GetInStream();
    if( pImpl->m_pInStream )
    {
        CreateTempFile( false );
        pImpl->bIsTemp = true;
        CloseInStream_Impl();
    }
}


bool SfxMedium::Commit()
{
    if( pImpl->xStorage.is() )
        StorageCommit_Impl();
    else if( pImpl->m_pOutStream  )
        pImpl->m_pOutStream->Flush();
    else if( pImpl->m_pInStream  )
        pImpl->m_pInStream->Flush();

    if ( GetError() == ERRCODE_NONE )
    {
        // does something only in case there is a temporary file ( means aName points to different location than aLogicName )
        Transfer_Impl();
    }

    bool bResult = ( GetError() == ERRCODE_NONE );

    if ( bResult && DocNeedsFileDateCheck() )
        GetInitFileDate( true );

    // remove truncation mode from the flags
    pImpl->m_nStorOpenMode &= ~StreamMode::TRUNC;
    return bResult;
}


bool SfxMedium::IsStorage()
{
    if ( pImpl->xStorage.is() )
        return true;

    if ( pImpl->m_bTriedStorage )
        return pImpl->bIsStorage;

    if ( pImpl->pTempFile )
    {
        OUString aURL;
        if ( osl::FileBase::getFileURLFromSystemPath( pImpl->m_aName, aURL )
             == osl::FileBase::E_None )
        {
            SAL_WARN( "sfx.doc", "Physical name not convertible!");
        }
        pImpl->bIsStorage = SotStorage::IsStorageFile( aURL ) && !SotStorage::IsOLEStorage( aURL);
        if ( !pImpl->bIsStorage )
            pImpl->m_bTriedStorage = true;
    }
    else if ( GetInStream() )
    {
        pImpl->bIsStorage = SotStorage::IsStorageFile( pImpl->m_pInStream.get() ) && !SotStorage::IsOLEStorage( pImpl->m_pInStream.get() );
        if ( !pImpl->m_pInStream->GetError() && !pImpl->bIsStorage )
            pImpl->m_bTriedStorage = true;
    }

    return pImpl->bIsStorage;
}


bool SfxMedium::IsPreview_Impl()
{
    bool bPreview = false;
    const SfxBoolItem* pPreview = SfxItemSet::GetItem<SfxBoolItem>(GetItemSet(), SID_PREVIEW, false);
    if ( pPreview )
        bPreview = pPreview->GetValue();
    else
    {
        const SfxStringItem* pFlags = SfxItemSet::GetItem<SfxStringItem>(GetItemSet(), SID_OPTIONS, false);
        if ( pFlags )
        {
            OUString aFileFlags = pFlags->GetValue();
            aFileFlags = aFileFlags.toAsciiUpperCase();
            if ( -1 != aFileFlags.indexOf( 'B' ) )
                bPreview = true;
        }
    }

    return bPreview;
}


void SfxMedium::StorageBackup_Impl()
{
    ::ucbhelper::Content aOriginalContent;
    Reference< css::ucb::XCommandEnvironment > xDummyEnv;

    bool bBasedOnOriginalFile = ( !pImpl->pTempFile && !( !pImpl->m_aLogicName.isEmpty() && pImpl->m_bSalvageMode )
        && !GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE ).isEmpty()
        && GetURLObject().GetProtocol() == INetProtocol::File
        && ::utl::UCBContentHelper::IsDocument( GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE ) ) );

    if ( bBasedOnOriginalFile && pImpl->m_aBackupURL.isEmpty()
      && ::ucbhelper::Content::create( GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE ), xDummyEnv, comphelper::getProcessComponentContext(), aOriginalContent ) )
    {
        DoInternalBackup_Impl( aOriginalContent );
        if( pImpl->m_aBackupURL.isEmpty() )
            SetError(ERRCODE_SFX_CANTCREATEBACKUP);
    }
}


OUString const & SfxMedium::GetBackup_Impl()
{
    if ( pImpl->m_aBackupURL.isEmpty() )
        StorageBackup_Impl();

    return pImpl->m_aBackupURL;
}


uno::Reference < embed::XStorage > SfxMedium::GetOutputStorage()
{
    if ( GetError() )
        return uno::Reference< embed::XStorage >();

    // if the medium was constructed with a Storage: use this one, not a temp. storage
    // if a temporary storage already exists: use it
    if ( pImpl->xStorage.is() && ( pImpl->m_aLogicName.isEmpty() || pImpl->pTempFile ) )
        return pImpl->xStorage;

    // if necessary close stream that was used for reading
    if ( pImpl->m_pInStream && !pImpl->m_pInStream->IsWritable() )
        CloseInStream();

    DBG_ASSERT( !pImpl->m_pOutStream, "OutStream in a readonly Medium?!" );

    // TODO/LATER: The current solution is to store the document temporary and then copy it to the target location;
    // in future it should be stored directly and then copied to the temporary location, since in this case no
    // file attributes have to be preserved and system copying mechanics could be used instead of streaming.
    CreateTempFileNoCopy();

    return GetStorage();
}


void SfxMedium::SetEncryptionDataToStorage_Impl()
{
    // in case media-descriptor contains password it should be used on opening
    if ( !pImpl->xStorage.is() || !pImpl->m_pSet )
        return;

    uno::Sequence< beans::NamedValue > aEncryptionData;
    if ( !GetEncryptionData_Impl( pImpl->m_pSet.get(), aEncryptionData ) )
        return;

    // replace the password with encryption data
    pImpl->m_pSet->ClearItem( SID_PASSWORD );
    pImpl->m_pSet->Put( SfxUnoAnyItem( SID_ENCRYPTIONDATA, uno::makeAny( aEncryptionData ) ) );

    try
    {
        ::comphelper::OStorageHelper::SetCommonStorageEncryptionData( pImpl->xStorage, aEncryptionData );
    }
    catch( const uno::Exception& )
    {
        SAL_WARN( "sfx.doc", "It must be possible to set a common password for the storage" );
        // TODO/LATER: set the error code in case of problem
        // SetError(ERRCODE_IO_GENERAL);
    }
}

#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT

// FIXME: Hmm actually lock files should be used for sftp: documents
// even if !HAVE_FEATURE_MULTIUSER_ENVIRONMENT. Only the use of lock
// files for *local* documents is unnecessary in that case. But
// actually, the checks for sftp: here are just wishful thinking; I
// don't this there is any support for actually editing documents
// behind sftp: URLs anyway.

// Sure, there could perhaps be a 3rd-party extension that brings UCB
// the potential to handle files behind sftp:. But there could also be
// an extension that handles some arbitrary foobar: scheme *and* it
// could be that lock files would be the correct thing to use for
// foobar: documents, too. But the hardcoded test below won't know
// that. Clearly the knowledge whether lock files should be used or
// not for some URL scheme belongs in UCB, not here.

namespace
{
OUString tryMSOwnerFile(const INetURLObject& aLockfileURL)
{
    try
    {
        static osl::Mutex aMutex;
        osl::MutexGuard aGuard(aMutex);
        css::uno::Reference<css::ucb::XCommandEnvironment> xEnv;
        ucbhelper::Content aSourceContent(
            aLockfileURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), xEnv,
            comphelper::getProcessComponentContext());

        // Excel creates Owner Files with FILE_FLAG_DELETE_ON_CLOSE, so we need to open it with
        // FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE share mode
        css::uno::Reference<css::io::XInputStream> xStream = aSourceContent.openStreamNoLock();
        if (!xStream)
            return OUString();

        const sal_Int32 nBufLen = 256;
        css::uno::Sequence<sal_Int8> aBuf(nBufLen);
        const sal_Int32 nRead = xStream->readBytes(aBuf, nBufLen);
        xStream->closeInput();
        if (nRead >= 162)
        {
            // Reverse engineering of MS Office Owner Files format (MS Office 2016 tested).
            // It starts with a single byte with name length, after which characters of username go
            // in current Windows 8-bit codepage.
            // For Word lockfiles, the name is followed by zero bytes up to position 54.
            // For PowerPoint lockfiles, the name is followed by a single zero byte, and then 0x20
            // bytes up to position 55.
            // For Excel lockfiles, the name is followed by 0x20 bytes up to position 55.
            // At those positions in each type of lockfile, a name length 2-byte word goes, followed
            // by UTF-16-LE-encoded copy of username. Spaces or some garbage follow up to the end of
            // the lockfile (total 162 bytes for Word, 165 bytes for Excel/PowerPoint).
            // Apparently MS Office does not allow username to be longer than 52 characters (trying
            // to enter more in its options dialog results in error messages stating this limit).
            const int nACPLen = aBuf[0];
            if (nACPLen > 0 && nACPLen <= 52) // skip wrong format
            {
                const sal_Int8* pBuf = aBuf.getConstArray() + 54;
                int nUTF16Len = *pBuf; // try Word position
                // If UTF-16 length is 0x20, then ACP length is also less than maximal, which means
                // that in Word lockfile case, at least two preceeding bytes would be zero. Both
                // Excel and PowerPoint lockfiles would have at least one of those bytes non-zero.
                if (nUTF16Len == 0x20 && (*(pBuf - 1) != 0 || *(pBuf - 2) != 0))
                    nUTF16Len = *++pBuf; // use Excel/PowerPoint position

                if (nUTF16Len > 0 && nUTF16Len <= 52) // skip wrong format
                    return OUString(reinterpret_cast<const sal_Unicode*>(pBuf + 2), nUTF16Len);
            }
        }
    }
    catch (...) {} // we don't ever need to care about any exceptions here

    return OUString();
}

OUString tryMSOwnerFiles(const OUString& sDocURL)
{
    INetURLObject aURL(sDocURL);
    if (aURL.HasError())
        return OUString();
    const OUString sFileName = aURL.GetLastName(INetURLObject::DecodeMechanism::WithCharset);
    if (sFileName.isEmpty())
        return OUString();
    const OUString sFileExt = aURL.GetFileExtension();
    const sal_Int32 nFileNameLen
        = sFileName.getLength() - sFileExt.getLength() - (sFileExt.isEmpty() ? 0 : 1);
    // Word, Excel, PowerPoint all prepend the filename with "~$".
    aURL.SetName("~$" + sFileName, INetURLObject::EncodeMechanism::All);
    OUString sUserData = tryMSOwnerFile(aURL);
    // Additionally, Word strips first chars of the filename: 1 for length 7, 2 for length >=8.
    if (sUserData.isEmpty() && nFileNameLen > 6)
    {
        aURL.SetName("~$" + sFileName.copy((nFileNameLen == 7) ? 1 : 2),
                     INetURLObject::EncodeMechanism::All);
        sUserData = tryMSOwnerFile(aURL);
    }

    if (!sUserData.isEmpty())
        sUserData += " (MS Office)"; // Mention the used office suite

    return sUserData;
}

OUString tryForeignLockfiles(const OUString& sDocURL)
{
    OUString sUserData = tryMSOwnerFiles(sDocURL);
    // here we can test for empty result, and add other known applications' lockfile testing
    return sUserData.trim();
}
}

SfxMedium::ShowLockResult SfxMedium::ShowLockedDocumentDialog(const LockFileEntry& aData,
                                                              bool bIsLoading, bool bOwnLock,
                                                              bool bHandleSysLocked)
{
    ShowLockResult nResult = ShowLockResult::NoLock;

    // tdf#92817: Simple check for empty lock file that needs to be deleted, when system locking is enabled
    if( aData[LockFileComponent::OOOUSERNAME].isEmpty() && aData[LockFileComponent::SYSUSERNAME].isEmpty() && !bHandleSysLocked )
        bOwnLock=true;

    // show the interaction regarding the document opening
    uno::Reference< task::XInteractionHandler > xHandler = GetInteractionHandler();

    if ( xHandler.is() && ( bIsLoading || !bHandleSysLocked || bOwnLock ) )
    {
        OUString aDocumentURL
            = GetURLObject().GetLastName(INetURLObject::DecodeMechanism::WithCharset);
        OUString aInfo;
        ::rtl::Reference< ::ucbhelper::InteractionRequest > xInteractionRequestImpl;

        sal_Int32 nContinuations = 3;

        if ( bOwnLock )
        {
            aInfo = aData[LockFileComponent::EDITTIME];

            xInteractionRequestImpl = new ::ucbhelper::InteractionRequest( uno::makeAny(
                document::OwnLockOnDocumentRequest( OUString(), uno::Reference< uno::XInterface >(), aDocumentURL, aInfo, !bIsLoading ) ) );
        }
        else
        {
            // Use a fourth continuation in case there's no filesystem lock:
            // "Ignore lock file and open/replace the document"
            if (!bHandleSysLocked)
                nContinuations = 4;

            if ( !aData[LockFileComponent::OOOUSERNAME].isEmpty() )
                aInfo = aData[LockFileComponent::OOOUSERNAME];
            else
                aInfo = aData[LockFileComponent::SYSUSERNAME];

            if (aInfo.isEmpty() && !GetURLObject().isAnyKnownWebDAVScheme())
                // Try to get name of user who has locked the file using other applications
                aInfo = tryForeignLockfiles(
                    GetURLObject().GetMainURL(INetURLObject::DecodeMechanism::NONE));

            if ( !aInfo.isEmpty() && !aData[LockFileComponent::EDITTIME].isEmpty() )
                aInfo += " ( " + aData[LockFileComponent::EDITTIME] + " )";

            if (!bIsLoading) // so, !bHandleSysLocked
            {
                xInteractionRequestImpl = new ::ucbhelper::InteractionRequest(uno::makeAny(
                    document::LockedOnSavingRequest(OUString(), uno::Reference< uno::XInterface >(), aDocumentURL, aInfo)));
                // Currently, only the last "Retry" continuation (meaning ignore the lock and try overwriting) can be returned.
            }
            else /*logically therefore bIsLoading is set */
            {
                xInteractionRequestImpl = new ::ucbhelper::InteractionRequest( uno::makeAny(
                    document::LockedDocumentRequest( OUString(), uno::Reference< uno::XInterface >(), aDocumentURL, aInfo ) ) );
            }
        }

        uno::Sequence< uno::Reference< task::XInteractionContinuation > > aContinuations(nContinuations);
        aContinuations[0] = new ::ucbhelper::InteractionAbort( xInteractionRequestImpl.get() );
        aContinuations[1] = new ::ucbhelper::InteractionApprove( xInteractionRequestImpl.get() );
        aContinuations[2] = new ::ucbhelper::InteractionDisapprove( xInteractionRequestImpl.get() );
        if (nContinuations > 3)
        {
            // We use InteractionRetry to reflect that user wants to
            // ignore the (stale?) alien lock file and open/overwrite the document
            aContinuations[3] = new ::ucbhelper::InteractionRetry(xInteractionRequestImpl.get());
        }
        xInteractionRequestImpl->setContinuations( aContinuations );

        xHandler->handle( xInteractionRequestImpl.get() );

        ::rtl::Reference< ::ucbhelper::InteractionContinuation > xSelected = xInteractionRequestImpl->getSelection();
        if ( uno::Reference< task::XInteractionAbort >( xSelected.get(), uno::UNO_QUERY ).is() )
        {
            SetError(ERRCODE_ABORT);
        }
        else if ( uno::Reference< task::XInteractionDisapprove >( xSelected.get(), uno::UNO_QUERY ).is() )
        {
            // own lock on loading, user has selected to ignore the lock
            // own lock on saving, user has selected to ignore the lock
            // alien lock on loading, user has selected to edit a copy of document
            // TODO/LATER: alien lock on saving, user has selected to do SaveAs to different location
            if ( !bOwnLock ) // bIsLoading implied from outermost condition
            {
                // means that a copy of the document should be opened
                GetItemSet()->Put( SfxBoolItem( SID_TEMPLATE, true ) );
            }
            else
                nResult = ShowLockResult::Succeeded;
        }
        else if (uno::Reference< task::XInteractionRetry >(xSelected.get(), uno::UNO_QUERY).is())
        {
            // User decided to ignore the alien (stale?) lock file without filesystem lock
            nResult = ShowLockResult::Succeeded;
        }
        else // if ( XSelected == aContinuations[1] )
        {
            // own lock on loading, user has selected to open readonly
            // own lock on saving, user has selected to open readonly
            // alien lock on loading, user has selected to retry saving
            // TODO/LATER: alien lock on saving, user has selected to retry saving

            if ( bIsLoading )
                GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, true ) );
            else
                nResult = ShowLockResult::Try;
        }
    }
    else
    {
        if ( bIsLoading )
        {
            // if no interaction handler is provided the default answer is open readonly
            // that usually happens in case the document is loaded per API
            // so the document must be opened readonly for backward compatibility
            GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, true ) );
        }
        else
            SetError(ERRCODE_IO_ACCESSDENIED);

    }

    return nResult;
}


bool SfxMedium::ShowLockFileProblemDialog(MessageDlg nWhichDlg)
{
    // system file locking is not active, ask user whether he wants to open the document without any locking
    uno::Reference< task::XInteractionHandler > xHandler = GetInteractionHandler();

    if (xHandler.is())
    {
        ::rtl::Reference< ::ucbhelper::InteractionRequest > xIgnoreRequestImpl;

        switch (nWhichDlg)
        {
            case MessageDlg::LockFileIgnore:
                xIgnoreRequestImpl = new ::ucbhelper::InteractionRequest(uno::makeAny( document::LockFileIgnoreRequest() ));
                break;
            case MessageDlg::LockFileCorrupt:
                xIgnoreRequestImpl = new ::ucbhelper::InteractionRequest(uno::makeAny( document::LockFileCorruptRequest() ));
                break;
        }

        uno::Sequence< uno::Reference< task::XInteractionContinuation > > aContinuations(2);
        aContinuations[0] = new ::ucbhelper::InteractionAbort(xIgnoreRequestImpl.get());
        aContinuations[1] = new ::ucbhelper::InteractionApprove(xIgnoreRequestImpl.get());
        xIgnoreRequestImpl->setContinuations(aContinuations);

        xHandler->handle(xIgnoreRequestImpl.get());

        ::rtl::Reference< ::ucbhelper::InteractionContinuation > xSelected = xIgnoreRequestImpl->getSelection();
        bool bReadOnly = uno::Reference< task::XInteractionApprove >(xSelected.get(), uno::UNO_QUERY).is();

        if (bReadOnly)
        {
            GetItemSet()->Put(SfxBoolItem(SID_DOC_READONLY, true));
        }
        else
        {
            SetError(ERRCODE_ABORT);
        }

        return bReadOnly;
    }

    return false;
}

namespace
{
    bool isSuitableProtocolForLocking(const OUString & rLogicName)
    {
        INetURLObject aUrl( rLogicName );
        INetProtocol eProt = aUrl.GetProtocol();
#if !HAVE_FEATURE_MACOSX_SANDBOX
        if (eProt == INetProtocol::File) {
            return true;
        }
#endif
        return eProt == INetProtocol::Smb || eProt == INetProtocol::Sftp;
    }
}

#endif // HAVE_FEATURE_MULTIUSER_ENVIRONMENT

// sets SID_DOC_READONLY if the document cannot be opened for editing
// if user cancel the loading the ERROR_ABORT is set
SfxMedium::LockFileResult SfxMedium::LockOrigFileOnDemand( bool bLoading, bool bNoUI, bool bTryIgnoreLockFile )
{
#if !HAVE_FEATURE_MULTIUSER_ENVIRONMENT
    (void) bLoading;
    (void) bNoUI;
    (void) bTryIgnoreLockFile;
    return LockFileResult::Succeeded;
#else
    LockFileResult eResult = LockFileResult::Failed;

    // check if path scheme is http:// or https://
    // may be this is better if used always, in Android and iOS as well?
    // if this code should be always there, remember to move the relevant code in UnlockFile method as well !

    if ( GetURLObject().isAnyKnownWebDAVScheme() )
    {
        // do nothing if WebDAV locking is disabled
        if (!IsWebDAVLockingUsed())
            return LockFileResult::Succeeded;

        try
        {
            bool bResult = pImpl->m_bLocked;
            bool bIsTemplate = false;
            // so, this is webdav stuff...
            if ( !bResult )
            {
                // no read-write access is necessary on loading if the document is explicitly opened as copy
                const SfxBoolItem* pTemplateItem = SfxItemSet::GetItem<SfxBoolItem>(GetItemSet(), SID_TEMPLATE, false);
                bIsTemplate = ( bLoading && pTemplateItem && pTemplateItem->GetValue() );
            }

            if ( !bIsTemplate && !bResult && !IsReadOnly() )
            {
                ShowLockResult bUIStatus = ShowLockResult::NoLock;
                do
                {
                    if( !bResult )
                    {
                        uno::Reference< task::XInteractionHandler > xCHandler = GetInteractionHandler( true );
                        Reference< css::ucb::XCommandEnvironment > xComEnv = new ::ucbhelper::CommandEnvironment(
                            xCHandler, Reference< css::ucb::XProgressHandler >() );

                        ucbhelper::Content aContentToLock(
                            GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE ),
                            xComEnv, comphelper::getProcessComponentContext() );

                        try
                        {
                            aContentToLock.lock();
                            bResult = true;
                        }
                        catch ( ucb::InteractiveLockingLockedException& )
                        {
                            // received when the resource is already locked
                            // get the lock owner, using a special ucb.webdav property
                            // the owner property retrieved here is  what the other principal send the server
                            // when activating the lock.
                            // See http://tools.ietf.org/html/rfc4918#section-14.17 for details
                            LockFileEntry aLockData;
                            aLockData[LockFileComponent::OOOUSERNAME] = "Unknown user";
                            // This solution works right when the LO user name and the WebDAV user
                            // name are the same.
                            // A better thing to do would be to obtain the 'real' WebDAV user name,
                            // but that's not possible from a WebDAV UCP provider client.
                            LockFileEntry aOwnData = svt::LockFileCommon::GenerateOwnEntry();
                            // use the current LO user name as the system name
                            aLockData[LockFileComponent::SYSUSERNAME] = aOwnData[LockFileComponent::SYSUSERNAME];

                            uno::Sequence< css::ucb::Lock >  aLocks;
                            // getting the property, send a PROPFIND to the server over the net
                            if( aContentToLock.getPropertyValue( "DAV:lockdiscovery" )  >>= aLocks )
                            {
                                // got at least a lock, show the owner of the first lock returned
                                css::ucb::Lock aLock = aLocks[0];
                                OUString aOwner;
                                if(aLock.Owner >>= aOwner)
                                {
                                    // we need to display the WebDAV user name owning the lock, not the local one
                                    aLockData[LockFileComponent::OOOUSERNAME] = aOwner;
                                }
                            }

                            if ( !bResult && !bNoUI )
                            {
                                bUIStatus
                                    = ShowLockedDocumentDialog(aLockData, bLoading, false, true);
                            }
                        }
                        catch( ucb::InteractiveNetworkWriteException& )
                        {
                            // This catch it's not really needed, here just for the sake of documentation on the behaviour.
                            // This is the most likely reason:
                            // - the remote site is a WebDAV with special configuration: read/only for read operations
                            //   and read/write for write operations, the user is not allowed to lock/write and
                            //   she cancelled the credentials request.
                            //   this is not actually an error, but the exception is sent directly from ucb, avoiding the automatic
                            //   management that takes part in cancelCommandExecution()
                            // Unfortunately there is no InteractiveNetwork*Exception available to signal this more correctly
                            // since it mostly happens on read/only part of webdav, this can be the most correct
                            // exception available
                        }
                        catch( uno::Exception& )
                        {}
                    }
                } while( !bResult && bUIStatus == ShowLockResult::Try );
            }

            pImpl->m_bLocked = bResult;

            if ( !bResult && GetError() == ERRCODE_NONE )
            {
                // the error should be set in case it is storing process
                // or the document has been opened for editing explicitly
                const SfxBoolItem* pReadOnlyItem = SfxItemSet::GetItem<SfxBoolItem>(pImpl->m_pSet.get(), SID_DOC_READONLY, false);

                if ( !bLoading || (pReadOnlyItem && !pReadOnlyItem->GetValue()) )
                    SetError(ERRCODE_IO_ACCESSDENIED);
                else
                    GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, true ) );
            }

            // when the file is locked, get the current file date
            if ( bResult && DocNeedsFileDateCheck() )
                GetInitFileDate( true );

            if ( bResult )
                eResult = LockFileResult::Succeeded;
        }
        catch ( const uno::Exception& )
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "sfx.doc", "Locking exception: WebDAV while trying to lock the file " << exceptionToString(ex) );
        }
        return eResult;
    }

    if (!IsLockingUsed())
        return LockFileResult::Succeeded;
    if (GetURLObject().HasError())
        return eResult;

    try
    {
        if ( pImpl->m_bLocked && bLoading
             && GetURLObject().GetProtocol() == INetProtocol::File )
        {
            // if the document is already locked the system locking might be temporarily off after storing
            // check whether the system file locking should be taken again
            GetLockingStream_Impl();
        }

        bool bResult = pImpl->m_bLocked;

        if ( !bResult )
        {
            // no read-write access is necessary on loading if the document is explicitly opened as copy
            const SfxBoolItem* pTemplateItem = SfxItemSet::GetItem<SfxBoolItem>(GetItemSet(), SID_TEMPLATE, false);
            bResult = ( bLoading && pTemplateItem && pTemplateItem->GetValue() );
        }

        if ( !bResult && !IsReadOnly() )
        {
            bool bContentReadonly = false;
            if ( bLoading && GetURLObject().GetProtocol() == INetProtocol::File )
            {
                // let the original document be opened to check the possibility to open it for editing
                // and to let the writable stream stay open to hold the lock on the document
                GetLockingStream_Impl();
            }

            // "IsReadOnly" property does not allow to detect whether the file is readonly always
            // so we try always to open the file for editing
            // the file is readonly only in case the read-write stream can not be opened
            if ( bLoading && !pImpl->m_xLockingStream.is() )
            {
                try
                {
                    // MediaDescriptor does this check also, the duplication should be avoided in future
                    Reference< css::ucb::XCommandEnvironment > xDummyEnv;
                    ::ucbhelper::Content aContent( GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE ), xDummyEnv, comphelper::getProcessComponentContext() );
                    aContent.getPropertyValue("IsReadOnly") >>= bContentReadonly;
                }
                catch( const uno::Exception& ) {}
            }

            // do further checks only if the file not readonly in fs
            if ( !bContentReadonly )
            {
                // the special file locking should be used only for suitable URLs
                if ( isSuitableProtocolForLocking( pImpl->m_aLogicName ) )
                {

                    // in case of storing the document should request the output before locking
                    if ( bLoading )
                    {
                        // let the stream be opened to check the system file locking
                        GetMedium_Impl();
                        if (GetError() != ERRCODE_NONE) {
                            return eResult;
                        }
                    }

                    ShowLockResult bUIStatus = ShowLockResult::NoLock;

                    // check whether system file locking has been used, the default value is false
                    bool bUseSystemLock = comphelper::isFileUrl( pImpl->m_aLogicName ) && IsSystemFileLockingUsed();

                    // TODO/LATER: This implementation does not allow to detect the system lock on saving here, actually this is no big problem
                    // if system lock is used the writeable stream should be available
                    bool bHandleSysLocked = ( bLoading && bUseSystemLock && !pImpl->xStream.is() && !pImpl->m_pOutStream );

                    // The file is attempted to get locked for the duration of lockfile creation on save
                    std::unique_ptr<osl::File> pFileLock;
                    if (!bLoading && bUseSystemLock && pImpl->pTempFile)
                    {
                        INetURLObject aDest(GetURLObject());
                        OUString aDestURL(aDest.GetMainURL(INetURLObject::DecodeMechanism::NONE));

                        if (comphelper::isFileUrl(aDestURL) || !aDest.removeSegment())
                        {
                            pFileLock = std::make_unique<osl::File>(aDestURL);
                            auto rc = pFileLock->open(osl_File_OpenFlag_Write);
                            if (rc == osl::FileBase::E_ACCES)
                                bHandleSysLocked = true;
                        }
                    }

                    do
                    {
                        try
                        {
                            ::svt::DocumentLockFile aLockFile( pImpl->m_aLogicName );
                            bool  bIoErr = false;

                            if (!bHandleSysLocked)
                            {
                                try
                                {
                                    bResult = aLockFile.CreateOwnLockFile();
                                }
                                catch (const uno::Exception&)
                                {
                                    if (tools::IsMappedWebDAVPath(GetURLObject()))
                                    {
                                        // This is a path that redirects to a WebDAV resource;
                                        // so failure creating lockfile is not an error here.
                                        bResult = true;
                                    }
                                    else if (bLoading && !bNoUI)
                                    {
                                        bIoErr = true;
                                        ShowLockFileProblemDialog(MessageDlg::LockFileIgnore);
                                        bResult = true;   // always delete the defect lock-file
                                    }
                                }

                                // in case OOo locking is turned off the lock file is still written if possible
                                // but it is ignored while deciding whether the document should be opened for editing or not
                                if (!bResult && !IsOOoLockFileUsed() && !bIoErr)
                                {
                                    bResult = true;
                                    // take the ownership over the lock file
                                    aLockFile.OverwriteOwnLockFile();
                                }
                            }

                            if ( !bResult )
                            {
                                LockFileEntry aData;
                                try
                                {
                                    aData = aLockFile.GetLockData();
                                }
                                catch (const io::WrongFormatException&)
                                {
                                    // we get empty or corrupt data
                                    // info to the user
                                    if (!bIoErr && bLoading && !bNoUI )
                                        bResult = ShowLockFileProblemDialog(MessageDlg::LockFileCorrupt);

                                    // not show the Lock Document Dialog
                                    bIoErr = true;
                                }
                                catch( const uno::Exception& )
                                {
                                    // show the Lock Document Dialog, when locked from other app
                                    bIoErr = !bHandleSysLocked;
                                }

                                bool bOwnLock = false;

                                if (!bHandleSysLocked)
                                {
                                    LockFileEntry aOwnData = svt::LockFileCommon::GenerateOwnEntry();
                                    bOwnLock = aOwnData[LockFileComponent::SYSUSERNAME] == aData[LockFileComponent::SYSUSERNAME];

                                    if (bOwnLock
                                        && aOwnData[LockFileComponent::LOCALHOST] == aData[LockFileComponent::LOCALHOST]
                                        && aOwnData[LockFileComponent::USERURL] == aData[LockFileComponent::USERURL])
                                    {
                                        // this is own lock from the same installation, it could remain because of crash
                                        bResult = true;
                                    }
                                }

                                if ( !bResult && !bIoErr)
                                {
                                    if (!bNoUI)
                                        bUIStatus = ShowLockedDocumentDialog(
                                            aData, bLoading, bOwnLock, bHandleSysLocked);
                                    else if (bLoading && bTryIgnoreLockFile && !bHandleSysLocked)
                                        bUIStatus = ShowLockResult::Succeeded;

                                    if ( bUIStatus == ShowLockResult::Succeeded )
                                    {
                                        // take the ownership over the lock file
                                        bResult = aLockFile.OverwriteOwnLockFile();
                                    }
                                    else if (bLoading && !bHandleSysLocked)
                                        eResult = LockFileResult::FailedLockFile;
                                }
                            }
                        }
                        catch( const uno::Exception& )
                        {
                        }
                    } while( !bResult && bUIStatus == ShowLockResult::Try );

                    pImpl->m_bLocked = bResult;
                }
                else
                {
                    // this is no file URL, check whether the file is readonly
                    bResult = !bContentReadonly;
                }
            }
        }

        if ( !bResult && GetError() == ERRCODE_NONE )
        {
            // the error should be set in case it is storing process
            // or the document has been opened for editing explicitly
            const SfxBoolItem* pReadOnlyItem = SfxItemSet::GetItem<SfxBoolItem>(pImpl->m_pSet.get(), SID_DOC_READONLY, false);

            if ( !bLoading || (pReadOnlyItem && !pReadOnlyItem->GetValue()) )
                SetError(ERRCODE_IO_ACCESSDENIED);
            else
                GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, true ) );
        }

        // when the file is locked, get the current file date
        if ( bResult && DocNeedsFileDateCheck() )
            GetInitFileDate( true );

        if ( bResult )
            eResult = LockFileResult::Succeeded;
    }
    catch( const uno::Exception& )
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN( "sfx.doc", "Locking exception: high probability, that the content has not been created " << exceptionToString(ex) );
    }

    return eResult;
#endif
}


uno::Reference < embed::XStorage > SfxMedium::GetStorage( bool bCreateTempIfNo )
{
    if ( pImpl->xStorage.is() || pImpl->m_bTriedStorage )
        return pImpl->xStorage;

    uno::Sequence< uno::Any > aArgs( 2 );

    // the medium should be retrieved before temporary file creation
    // to let the MediaDescriptor be filled with the streams
    GetMedium_Impl();

    if ( bCreateTempIfNo )
        CreateTempFile( false );

    GetMedium_Impl();

    if ( GetError() )
        return pImpl->xStorage;

    const SfxBoolItem* pRepairItem = SfxItemSet::GetItem<SfxBoolItem>(GetItemSet(), SID_REPAIRPACKAGE, false);
    if ( pRepairItem && pRepairItem->GetValue() )
    {
        // the storage should be created for repairing mode
        CreateTempFile( false );
        GetMedium_Impl();

        Reference< css::ucb::XProgressHandler > xProgressHandler;
        Reference< css::task::XStatusIndicator > xStatusIndicator;

        const SfxUnoAnyItem* pxProgressItem = SfxItemSet::GetItem<SfxUnoAnyItem>(GetItemSet(), SID_PROGRESS_STATUSBAR_CONTROL, false);
        if( pxProgressItem && ( pxProgressItem->GetValue() >>= xStatusIndicator ) )
            xProgressHandler.set( new utl::ProgressHandlerWrap( xStatusIndicator ) );

        uno::Sequence< beans::PropertyValue > aAddProps( 2 );
        aAddProps[0].Name = "RepairPackage";
        aAddProps[0].Value <<= true;
        aAddProps[1].Name = "StatusIndicator";
        aAddProps[1].Value <<= xProgressHandler;

        // the first arguments will be filled later
        aArgs.realloc( 3 );
        aArgs[2] <<= aAddProps;
    }

    if ( pImpl->xStream.is() )
    {
        // since the storage is based on temporary stream we open it always read-write
        aArgs[0] <<= pImpl->xStream;
        aArgs[1] <<= embed::ElementModes::READWRITE;
        pImpl->bStorageBasedOnInStream = true;
        if (pImpl->m_bDisableFileSync)
        {
            // Forward NoFileSync to the storage factory.
            aArgs.realloc(3);
            uno::Sequence<beans::PropertyValue> aProperties(
                comphelper::InitPropertySequence({ { "NoFileSync", uno::makeAny(true) } }));
            aArgs[2] <<= aProperties;
        }
    }
    else if ( pImpl->xInputStream.is() )
    {
        // since the storage is based on temporary stream we open it always read-write
        aArgs[0] <<= pImpl->xInputStream;
        aArgs[1] <<= embed::ElementModes::READ;
        pImpl->bStorageBasedOnInStream = true;
    }
    else
    {
        CloseStreams_Impl();
        aArgs[0] <<= pImpl->m_aName;
        aArgs[1] <<= embed::ElementModes::READ;
        pImpl->bStorageBasedOnInStream = false;
    }

    try
    {
        pImpl->xStorage.set( ::comphelper::OStorageHelper::GetStorageFactory()->createInstanceWithArguments( aArgs ),
                            uno::UNO_QUERY );
    }
    catch( const uno::Exception& )
    {
        // impossibility to create the storage is no error
    }

    if( ( pImpl->nLastStorageError = GetError() ) != ERRCODE_NONE )
    {
        pImpl->xStorage = nullptr;
        if ( pImpl->m_pInStream )
            pImpl->m_pInStream->Seek(0);
        return uno::Reference< embed::XStorage >();
    }

    pImpl->m_bTriedStorage = true;

    // TODO/LATER: Get versionlist on demand
    if ( pImpl->xStorage.is() )
    {
        SetEncryptionDataToStorage_Impl();
        GetVersionList();
    }

    const SfxInt16Item* pVersion = SfxItemSet::GetItem<SfxInt16Item>(pImpl->m_pSet.get(), SID_VERSION, false);

    bool bResetStorage = false;
    if ( pVersion && pVersion->GetValue() )
    {
        // Read all available versions
        if ( pImpl->aVersions.getLength() )
        {
            // Search for the version fits the comment
            // The versions are numbered starting with 1, versions with
            // negative versions numbers are counted backwards from the
            // current version
            short nVersion = pVersion->GetValue();
            if ( nVersion<0 )
                nVersion = static_cast<short>(pImpl->aVersions.getLength()) + nVersion;
            else // nVersion > 0; pVersion->GetValue() != 0 was the condition to this block
                nVersion--;

            util::RevisionTag& rTag = pImpl->aVersions[nVersion];
            {
                // Open SubStorage for all versions
                uno::Reference < embed::XStorage > xSub = pImpl->xStorage->openStorageElement( "Versions",
                        embed::ElementModes::READ );

                DBG_ASSERT( xSub.is(), "Version list, but no Versions!" );

                // There the version is stored as packed Stream
                uno::Reference < io::XStream > xStr = xSub->openStreamElement( rTag.Identifier, embed::ElementModes::READ );
                std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream( xStr ));
                if ( pStream && pStream->GetError() == ERRCODE_NONE )
                {
                    // Unpack Stream  in TempDir
                    ::utl::TempFile aTempFile;
                    const OUString& aTmpName = aTempFile.GetURL();
                    SvFileStream    aTmpStream( aTmpName, SFX_STREAM_READWRITE );

                    pStream->ReadStream( aTmpStream );
                    pStream.reset();
                    aTmpStream.Close();

                    // Open data as Storage
                    pImpl->m_nStorOpenMode = SFX_STREAM_READONLY;
                    pImpl->xStorage = comphelper::OStorageHelper::GetStorageFromURL( aTmpName, embed::ElementModes::READ );
                    pImpl->bStorageBasedOnInStream = false;
                    OUString aTemp;
                    osl::FileBase::getSystemPathFromFileURL( aTmpName, aTemp );
                    SetPhysicalName_Impl( aTemp );

                    pImpl->bIsTemp = true;
                    GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, true ) );
                    // TODO/MBA
                    pImpl->aVersions.realloc(0);
                }
                else
                    bResetStorage = true;
            }
        }
        else
            bResetStorage = true;
    }

    if ( bResetStorage )
    {
        pImpl->xStorage.clear();
        if ( pImpl->m_pInStream )
            pImpl->m_pInStream->Seek( 0 );
    }

    pImpl->bIsStorage = pImpl->xStorage.is();
    return pImpl->xStorage;
}


uno::Reference< embed::XStorage > const & SfxMedium::GetZipStorageToSign_Impl( bool bReadOnly )
{
    if ( !GetError() && !pImpl->m_xZipStorage.is() )
    {
        GetMedium_Impl();

        try
        {
            // we can not sign document if there is no stream
            // should it be possible at all?
            if ( !bReadOnly && pImpl->xStream.is() )
            {
                pImpl->m_xZipStorage = ::comphelper::OStorageHelper::GetStorageOfFormatFromStream( ZIP_STORAGE_FORMAT_STRING, pImpl->xStream );
            }
            else if ( pImpl->xInputStream.is() )
            {
                pImpl->m_xZipStorage = ::comphelper::OStorageHelper::GetStorageOfFormatFromInputStream( ZIP_STORAGE_FORMAT_STRING, pImpl->xInputStream );
            }
        }
        catch( const uno::Exception& )
        {
            SAL_WARN( "sfx.doc", "No possibility to get readonly version of storage from medium!" );
        }

        if ( GetError() ) // do not remove warnings
            ResetError();
    }

    return pImpl->m_xZipStorage;
}


void SfxMedium::CloseZipStorage_Impl()
{
    if ( pImpl->m_xZipStorage.is() )
    {
        try {
            pImpl->m_xZipStorage->dispose();
        } catch( const uno::Exception& )
        {}

        pImpl->m_xZipStorage.clear();
    }
}

void SfxMedium::CloseStorage()
{
    if ( pImpl->xStorage.is() )
    {
        uno::Reference < lang::XComponent > xComp( pImpl->xStorage, uno::UNO_QUERY );
        // in the salvage mode the medium does not own the storage
        if ( pImpl->bDisposeStorage && !pImpl->m_bSalvageMode )
        {
            try {
                xComp->dispose();
            } catch( const uno::Exception& )
            {
                SAL_WARN( "sfx.doc", "Medium's storage is already disposed!" );
            }
        }

        pImpl->xStorage.clear();
        pImpl->bStorageBasedOnInStream = false;
    }

    pImpl->m_bTriedStorage = false;
    pImpl->bIsStorage = false;
}

void SfxMedium::CanDisposeStorage_Impl( bool bDisposeStorage )
{
    pImpl->bDisposeStorage = bDisposeStorage;
}

bool SfxMedium::WillDisposeStorageOnClose_Impl()
{
    return pImpl->bDisposeStorage;
}

StreamMode SfxMedium::GetOpenMode() const
{
    return pImpl->m_nStorOpenMode;
}

void SfxMedium::SetOpenMode( StreamMode nStorOpen,
                             bool bDontClose )
{
    if ( pImpl->m_nStorOpenMode != nStorOpen )
    {
        pImpl->m_nStorOpenMode = nStorOpen;

        if( !bDontClose )
        {
            if ( pImpl->xStorage.is() )
                CloseStorage();

            CloseStreams_Impl();
        }
    }
}


bool SfxMedium::UseBackupToRestore_Impl( ::ucbhelper::Content& aOriginalContent,
                                            const Reference< css::ucb::XCommandEnvironment >& xComEnv )
{
    try
    {
        ::ucbhelper::Content aTransactCont( pImpl->m_aBackupURL, xComEnv, comphelper::getProcessComponentContext() );

        Reference< XInputStream > aOrigInput = aTransactCont.openStream();
        aOriginalContent.writeStream( aOrigInput, true );
        return true;
    }
    catch( const Exception& )
    {
        // in case of failure here the backup file should not be removed
        // TODO/LATER: a message should be used to let user know about the backup
        pImpl->m_bRemoveBackup = false;
        // TODO/LATER: needs a specific error code
        pImpl->m_eError = ERRCODE_IO_GENERAL;
    }

    return false;
}


bool SfxMedium::StorageCommit_Impl()
{
    bool bResult = false;
    Reference< css::ucb::XCommandEnvironment > xDummyEnv;
    ::ucbhelper::Content aOriginalContent;

    if ( pImpl->xStorage.is() )
    {
        if ( !GetError() )
        {
            uno::Reference < embed::XTransactedObject > xTrans( pImpl->xStorage, uno::UNO_QUERY );
            if ( xTrans.is() )
            {
                try
                {
                    xTrans->commit();
                    CloseZipStorage_Impl();
                    bResult = true;
                }
                catch ( const embed::UseBackupException& aBackupExc )
                {
                    // since the temporary file is created always now, the scenario is close to be impossible
                    if ( !pImpl->pTempFile )
                    {
                        OSL_ENSURE( !pImpl->m_aBackupURL.isEmpty(), "No backup on storage commit!" );
                        if ( !pImpl->m_aBackupURL.isEmpty()
                            && ::ucbhelper::Content::create( GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE ),
                                                        xDummyEnv, comphelper::getProcessComponentContext(),
                                                        aOriginalContent ) )
                        {
                            // use backup to restore the file
                            // the storage has already disconnected from original location
                            CloseAndReleaseStreams_Impl();
                            if ( !UseBackupToRestore_Impl( aOriginalContent, xDummyEnv ) )
                            {
                                // connect the medium to the temporary file of the storage
                                pImpl->aContent = ::ucbhelper::Content();
                                pImpl->m_aName = aBackupExc.TemporaryFileURL;
                                OSL_ENSURE( !pImpl->m_aName.isEmpty(), "The exception _must_ contain the temporary URL!" );
                            }
                        }
                    }

                    if (!GetError())
                        SetError(ERRCODE_IO_GENERAL);
                }
                catch ( const uno::Exception& )
                {
                    //TODO/LATER: improve error handling
                    SetError(ERRCODE_IO_GENERAL);
                }
            }
        }
    }

    return bResult;
}


void SfxMedium::TransactedTransferForFS_Impl( const INetURLObject& aSource,
                                                 const INetURLObject& aDest,
                                                 const Reference< css::ucb::XCommandEnvironment >& xComEnv )
{
    bool bResult = false;
    Reference< css::ucb::XCommandEnvironment > xDummyEnv;
    ::ucbhelper::Content aOriginalContent;

    try
    {
        aOriginalContent = ::ucbhelper::Content( aDest.GetMainURL( INetURLObject::DecodeMechanism::NONE ), xComEnv, comphelper::getProcessComponentContext() );
    }
    catch ( const css::ucb::CommandAbortedException& )
    {
        pImpl->m_eError = ERRCODE_ABORT;
    }
    catch ( const css::ucb::CommandFailedException& )
    {
        pImpl->m_eError = ERRCODE_ABORT;
    }
    catch (const css::ucb::ContentCreationException& ex)
    {
        pImpl->m_eError = ERRCODE_IO_GENERAL;
        if (
            (ex.eError == css::ucb::ContentCreationError_NO_CONTENT_PROVIDER    ) ||
            (ex.eError == css::ucb::ContentCreationError_CONTENT_CREATION_FAILED)
           )
        {
            pImpl->m_eError = ERRCODE_IO_NOTEXISTSPATH;
        }
    }
    catch (const css::uno::Exception&)
    {
       pImpl->m_eError = ERRCODE_IO_GENERAL;
    }

    if( pImpl->m_eError && !pImpl->m_eError.IsWarning() )
        return;

    if ( pImpl->xStorage.is() )
        CloseStorage();

    CloseStreams_Impl();

    ::ucbhelper::Content aTempCont;
    if( ::ucbhelper::Content::create( aSource.GetMainURL( INetURLObject::DecodeMechanism::NONE ), xDummyEnv, comphelper::getProcessComponentContext(), aTempCont ) )
    {
        bool bTransactStarted = false;
        const SfxBoolItem* pOverWrite = SfxItemSet::GetItem<SfxBoolItem>(GetItemSet(), SID_OVERWRITE, false);
        bool bOverWrite = !pOverWrite || pOverWrite->GetValue();

        try
        {
            OUString aSourceMainURL = aSource.GetMainURL(INetURLObject::DecodeMechanism::NONE);
            OUString aDestMainURL = aDest.GetMainURL(INetURLObject::DecodeMechanism::NONE);

            sal_uInt64 nAttributes = GetDefaultFileAttributes(aDestMainURL);
            if (IsFileMovable(aDest)
                && osl::File::replace(aSourceMainURL, aDestMainURL) == osl::FileBase::E_None)
            {
                if (nAttributes)
                    // Adjust attributes, source might be created with
                    // the osl_File_OpenFlag_Private flag.
                    osl::File::setAttributes(aDestMainURL, nAttributes);
                bResult = true;
            }
            else
            {
                if (bOverWrite && ::utl::UCBContentHelper::IsDocument(aDestMainURL))
                {
                    if( pImpl->m_aBackupURL.isEmpty() )
                        DoInternalBackup_Impl( aOriginalContent );

                    if( !pImpl->m_aBackupURL.isEmpty() )
                    {
                        Reference< XInputStream > aTempInput = aTempCont.openStream();
                        bTransactStarted = true;
                        aOriginalContent.setPropertyValue( "Size", uno::makeAny( sal_Int64(0) ) );
                        aOriginalContent.writeStream( aTempInput, bOverWrite );
                        bResult = true;
                    }
                    else
                    {
                        pImpl->m_eError = ERRCODE_SFX_CANTCREATEBACKUP;
                    }
                }
                else
                {
                    Reference< XInputStream > aTempInput = aTempCont.openStream();
                    aOriginalContent.writeStream( aTempInput, bOverWrite );
                    bResult = true;
                }
            }
        }
        catch ( const css::ucb::CommandAbortedException& )
        {
            pImpl->m_eError = ERRCODE_ABORT;
        }
        catch ( const css::ucb::CommandFailedException& )
        {
            pImpl->m_eError = ERRCODE_ABORT;
        }
        catch ( const css::ucb::InteractiveIOException& r )
        {
            if ( r.Code == IOErrorCode_ACCESS_DENIED )
                pImpl->m_eError = ERRCODE_IO_ACCESSDENIED;
            else if ( r.Code == IOErrorCode_NOT_EXISTING )
                pImpl->m_eError = ERRCODE_IO_NOTEXISTS;
            else if ( r.Code == IOErrorCode_CANT_READ )
                pImpl->m_eError = ERRCODE_IO_CANTREAD;
            else
                pImpl->m_eError = ERRCODE_IO_GENERAL;
        }
        catch ( const css::uno::Exception& )
        {
            pImpl->m_eError = ERRCODE_IO_GENERAL;
        }

        if ( bResult )
        {
            if ( pImpl->pTempFile )
            {
                pImpl->pTempFile->EnableKillingFile();
                pImpl->pTempFile.reset();
            }
        }
        else if ( bTransactStarted )
        {
            UseBackupToRestore_Impl( aOriginalContent, xDummyEnv );
        }
    }
    else
        pImpl->m_eError = ERRCODE_IO_CANTREAD;
}


bool SfxMedium::TryDirectTransfer( const OUString& aURL, SfxItemSet const & aTargetSet )
{
    if ( GetError() )
        return false;

    // if the document had no password it should be stored without password
    // if the document had password it should be stored with the same password
    // otherwise the stream copying can not be done
    const SfxStringItem* pNewPassItem = aTargetSet.GetItem<SfxStringItem>(SID_PASSWORD, false);
    const SfxStringItem* pOldPassItem = SfxItemSet::GetItem<SfxStringItem>(GetItemSet(), SID_PASSWORD, false);
    if ( ( !pNewPassItem && !pOldPassItem )
      || ( pNewPassItem && pOldPassItem && pNewPassItem->GetValue() == pOldPassItem->GetValue() ) )
    {
        // the filter must be the same
        const SfxStringItem* pNewFilterItem = aTargetSet.GetItem<SfxStringItem>(SID_FILTER_NAME, false);
        const SfxStringItem* pOldFilterItem = SfxItemSet::GetItem<SfxStringItem>(GetItemSet(), SID_FILTER_NAME, false);
        if ( pNewFilterItem && pOldFilterItem && pNewFilterItem->GetValue() == pOldFilterItem->GetValue() )
        {
            // get the input stream and copy it
            // in case of success return true
            uno::Reference< io::XInputStream > xInStream = GetInputStream();

            ResetError();
            if ( xInStream.is() )
            {
                try
                {
                    uno::Reference< io::XSeekable > xSeek( xInStream, uno::UNO_QUERY );
                    sal_Int64 nPos = 0;
                    if ( xSeek.is() )
                    {
                        nPos = xSeek->getPosition();
                        xSeek->seek( 0 );
                    }

                    uno::Reference < css::ucb::XCommandEnvironment > xEnv;
                    ::ucbhelper::Content aTargetContent( aURL, xEnv, comphelper::getProcessComponentContext() );

                    InsertCommandArgument aInsertArg;
                    aInsertArg.Data = xInStream;
                    const SfxBoolItem* pOverWrite = aTargetSet.GetItem<SfxBoolItem>(SID_OVERWRITE, false);
                    if ( pOverWrite && !pOverWrite->GetValue() ) // argument says: never overwrite
                        aInsertArg.ReplaceExisting = false;
                    else
                        aInsertArg.ReplaceExisting = true; // default is overwrite existing files

                    Any aCmdArg;
                    aCmdArg <<= aInsertArg;
                    aTargetContent.executeCommand( "insert",
                                                    aCmdArg );

                    if ( xSeek.is() )
                        xSeek->seek( nPos );

                    return true;
                }
                catch( const uno::Exception& )
                {}
            }
        }
    }

    return false;
}


void SfxMedium::Transfer_Impl()
{
    // The transfer is required only in two cases: either if there is a temporary file or if there is a salvage item
    OUString aNameURL;
    if ( pImpl->pTempFile )
        aNameURL = pImpl->pTempFile->GetURL();
    else if ( !pImpl->m_aLogicName.isEmpty() && pImpl->m_bSalvageMode )
    {
        // makes sense only in case logic name is set
        if ( osl::FileBase::getFileURLFromSystemPath( pImpl->m_aName, aNameURL )
             != osl::FileBase::E_None )
            SAL_WARN( "sfx.doc", "The medium name is not convertible!" );
    }

    if ( aNameURL.isEmpty() || ( pImpl->m_eError && !pImpl->m_eError.IsWarning() ) )
        return;

    SAL_INFO( "sfx.doc", "SfxMedium::Transfer_Impl, copying to target" );

    Reference < css::ucb::XCommandEnvironment > xEnv;
    Reference< XOutputStream > rOutStream;

    // in case an output stream is provided from outside and the URL is correct
    // commit to the stream
    if (pImpl->m_aLogicName.startsWith("private:stream"))
    {
        // TODO/LATER: support storing to SID_STREAM
        const SfxUnoAnyItem* pOutStreamItem = SfxItemSet::GetItem<SfxUnoAnyItem>(pImpl->m_pSet.get(), SID_OUTPUTSTREAM, false);
        if( pOutStreamItem && ( pOutStreamItem->GetValue() >>= rOutStream ) )
        {
            if ( pImpl->xStorage.is() )
                CloseStorage();

            CloseStreams_Impl();

            INetURLObject aSource( aNameURL );
            ::ucbhelper::Content aTempCont;
            if( ::ucbhelper::Content::create( aSource.GetMainURL( INetURLObject::DecodeMechanism::NONE ), xEnv, comphelper::getProcessComponentContext(), aTempCont ) )
            {
                try
                {
                    sal_Int32 nRead;
                    sal_Int32 nBufferSize = 32767;
                    Sequence < sal_Int8 > aSequence ( nBufferSize );
                    Reference< XInputStream > aTempInput = aTempCont.openStream();

                    do
                    {
                        nRead = aTempInput->readBytes ( aSequence, nBufferSize );
                        if ( nRead < nBufferSize )
                        {
                            Sequence < sal_Int8 > aTempBuf ( aSequence.getConstArray(), nRead );
                            rOutStream->writeBytes ( aTempBuf );
                        }
                        else
                            rOutStream->writeBytes ( aSequence );
                    }
                    while ( nRead == nBufferSize );

                    // remove temporary file
                    if ( pImpl->pTempFile )
                    {
                        pImpl->pTempFile->EnableKillingFile();
                        pImpl->pTempFile.reset();
                    }
                }
                catch( const Exception& )
                {}
            }
        }
        else
        {
            SAL_WARN( "sfx.doc", "Illegal Output stream parameter!" );
            SetError(ERRCODE_IO_GENERAL);
        }

        // free the reference
        if ( pImpl->m_pSet )
            pImpl->m_pSet->ClearItem( SID_OUTPUTSTREAM );

        return;
    }

    GetContent();
    if ( !pImpl->aContent.get().is() )
    {
        pImpl->m_eError = ERRCODE_IO_NOTEXISTS;
        return;
    }

    INetURLObject aDest( GetURLObject() );

    // source is the temp file written so far
    INetURLObject aSource( aNameURL );

    // a special case, an interaction handler should be used for
    // authentication in case it is available
    Reference< css::ucb::XCommandEnvironment > xComEnv;
    Reference< css::task::XInteractionHandler > xInteractionHandler = GetInteractionHandler();
    if (xInteractionHandler.is())
        xComEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler,
                                                  Reference< css::ucb::XProgressHandler >() );

    OUString aDestURL( aDest.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

    if ( comphelper::isFileUrl( aDestURL ) || !aDest.removeSegment() )
    {
        TransactedTransferForFS_Impl( aSource, aDest, xComEnv );

        if (!pImpl->m_bDisableFileSync)
        {
            // Hideous - no clean way to do this, so we re-open the file just to fsync it
            osl::File aFile( aDestURL );
            if ( aFile.open( osl_File_OpenFlag_Write ) == osl::FileBase::E_None )
            {
                aFile.sync();
                SAL_INFO( "sfx.doc", "fsync'd saved file '" << aDestURL << "'" );
                aFile.close();
            }
        }
    }
    else
    {
        // create content for the parent folder and call transfer on that content with the source content
        // and the destination file name as parameters
        ::ucbhelper::Content aSourceContent;
        ::ucbhelper::Content aTransferContent;

        ::ucbhelper::Content aDestContent;
        (void)::ucbhelper::Content::create( aDestURL, xComEnv, comphelper::getProcessComponentContext(), aDestContent );
        // For checkin, we need the object URL, not the parent folder:
        if ( !IsInCheckIn( ) )
        {
            // Get the parent URL from the XChild if possible: why would the URL necessarily have
            // a hierarchical path? It's not always the case for CMIS.
            Reference< css::container::XChild> xChild( aDestContent.get(), uno::UNO_QUERY );
            OUString sParentUrl;
            if ( xChild.is( ) )
            {
                Reference< css::ucb::XContent > xParent( xChild->getParent( ), uno::UNO_QUERY );
                if ( xParent.is( ) )
                {
                    sParentUrl = xParent->getIdentifier( )->getContentIdentifier();
                }
            }

            if ( sParentUrl.isEmpty() )
                aDestURL = aDest.GetMainURL( INetURLObject::DecodeMechanism::NONE );
                    // adjust to above aDest.removeSegment()
            else
                aDestURL = sParentUrl;
        }

        // LongName wasn't defined anywhere, only used here... get the Title instead
        // as it's less probably empty
        OUString aFileName;
        Any aAny = aDestContent.getPropertyValue("Title");
        aAny >>= aFileName;
        aAny = aDestContent.getPropertyValue( "ObjectId" );
        OUString sObjectId;
        aAny >>= sObjectId;
        if ( aFileName.isEmpty() )
            aFileName = GetURLObject().getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset );

        try
        {
            aTransferContent = ::ucbhelper::Content( aDestURL, xComEnv, comphelper::getProcessComponentContext() );
        }
        catch (const css::ucb::ContentCreationException& ex)
        {
            pImpl->m_eError = ERRCODE_IO_GENERAL;
            if (
                (ex.eError == css::ucb::ContentCreationError_NO_CONTENT_PROVIDER    ) ||
                (ex.eError == css::ucb::ContentCreationError_CONTENT_CREATION_FAILED)
               )
            {
                pImpl->m_eError = ERRCODE_IO_NOTEXISTSPATH;
            }
        }
        catch (const css::uno::Exception&)
        {
            pImpl->m_eError = ERRCODE_IO_GENERAL;
        }

        if ( !pImpl->m_eError || pImpl->m_eError.IsWarning() )
        {
            // free resources, otherwise the transfer may fail
            if ( pImpl->xStorage.is() )
                CloseStorage();

            CloseStreams_Impl();

            (void)::ucbhelper::Content::create( aSource.GetMainURL( INetURLObject::DecodeMechanism::NONE ), xEnv, comphelper::getProcessComponentContext(), aSourceContent );

            // check for external parameters that may customize the handling of NameClash situations
            const SfxBoolItem* pOverWrite = SfxItemSet::GetItem<SfxBoolItem>(GetItemSet(), SID_OVERWRITE, false);
            sal_Int32 nNameClash;
            if ( pOverWrite && !pOverWrite->GetValue() )
                // argument says: never overwrite
                nNameClash = NameClash::ERROR;
            else
                // default is overwrite existing files
                nNameClash = NameClash::OVERWRITE;

            try
            {
                OUString aMimeType = pImpl->getFilterMimeType();
                ::ucbhelper::InsertOperation eOperation = ::ucbhelper::InsertOperation::Copy;
                bool bMajor = false;
                OUString sComment;
                if ( IsInCheckIn( ) )
                {
                    eOperation = ::ucbhelper::InsertOperation::Checkin;
                    const SfxBoolItem* pMajor = SfxItemSet::GetItem<SfxBoolItem>(GetItemSet(), SID_DOCINFO_MAJOR, false);
                    bMajor = pMajor && pMajor->GetValue( );
                    const SfxStringItem* pComments = SfxItemSet::GetItem<SfxStringItem>(GetItemSet(), SID_DOCINFO_COMMENTS, false);
                    if ( pComments )
                        sComment = pComments->GetValue( );
                }
                OUString sResultURL;
                aTransferContent.transferContent(
                    aSourceContent, eOperation,
                    aFileName, nNameClash, aMimeType, bMajor, sComment,
                    &sResultURL, sObjectId );

                if ( !sResultURL.isEmpty( ) )  // Likely to happen only for checkin
                    SwitchDocumentToFile( sResultURL );
                try
                {
                    if ( GetURLObject().isAnyKnownWebDAVScheme() &&
                         eOperation == ::ucbhelper::InsertOperation::Copy )
                    {
                        // tdf#95272 try to re-issue a lock command when a new file is created.
                        // This may be needed because some WebDAV servers fail to implement the
                        // 'LOCK on unallocated reference', see issue comment:
                        // <https://bugs.documentfoundation.org/show_bug.cgi?id=95792#c8>
                        // and specification at:
                        // <http://tools.ietf.org/html/rfc4918#section-7.3>
                        // If the WebDAV resource is already locked by this LO instance, nothing will
                        // happen, e.g. the LOCK method will not be sent to the server.
                        ::ucbhelper::Content aLockContent = ::ucbhelper::Content( GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE ), xComEnv, comphelper::getProcessComponentContext() );
                        aLockContent.lock();
                    }
                }
                catch ( css::uno::Exception & e )
                {
                    SAL_WARN( "sfx.doc", "LOCK not working while re-issuing it. Exception message: " << e );
                }
            }
            catch ( const css::ucb::CommandAbortedException& )
            {
                pImpl->m_eError = ERRCODE_ABORT;
            }
            catch ( const css::ucb::CommandFailedException& )
            {
                pImpl->m_eError = ERRCODE_ABORT;
            }
            catch ( const css::ucb::InteractiveIOException& r )
            {
                if ( r.Code == IOErrorCode_ACCESS_DENIED )
                    pImpl->m_eError = ERRCODE_IO_ACCESSDENIED;
                else if ( r.Code == IOErrorCode_NOT_EXISTING )
                    pImpl->m_eError = ERRCODE_IO_NOTEXISTS;
                else if ( r.Code == IOErrorCode_CANT_READ )
                    pImpl->m_eError = ERRCODE_IO_CANTREAD;
                else
                    pImpl->m_eError = ERRCODE_IO_GENERAL;
            }
            catch ( const css::uno::Exception& )
            {
                pImpl->m_eError = ERRCODE_IO_GENERAL;
            }

            // do not switch from temporary file in case of nonfile protocol
        }
    }

    if ( ( !pImpl->m_eError || pImpl->m_eError.IsWarning() ) && !pImpl->pTempFile )
    {
        // without a TempFile the physical and logical name should be the same after successful transfer
        if (osl::FileBase::getSystemPathFromFileURL(
              GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE ), pImpl->m_aName )
            != osl::FileBase::E_None)
        {
            pImpl->m_aName.clear();
        }
        pImpl->m_bSalvageMode = false;
    }
}


void SfxMedium::DoInternalBackup_Impl( const ::ucbhelper::Content& aOriginalContent,
                                       const OUString& aPrefix,
                                       const OUString& aExtension,
                                       const OUString& aDestDir )
{
    if ( !pImpl->m_aBackupURL.isEmpty() )
        return; // the backup was done already

    ::utl::TempFile aTransactTemp( aPrefix, true, &aExtension, &aDestDir );

    INetURLObject aBackObj( aTransactTemp.GetURL() );
    OUString aBackupName = aBackObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset );

    Reference < css::ucb::XCommandEnvironment > xDummyEnv;
    ::ucbhelper::Content aBackupCont;
    if( ::ucbhelper::Content::create( aDestDir, xDummyEnv, comphelper::getProcessComponentContext(), aBackupCont ) )
    {
        try
        {
            OUString sMimeType = pImpl->getFilterMimeType();
            aBackupCont.transferContent( aOriginalContent,
                                            ::ucbhelper::InsertOperation::Copy,
                                            aBackupName,
                                            NameClash::OVERWRITE,
                                            sMimeType );
            pImpl->m_aBackupURL = aBackObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
            pImpl->m_bRemoveBackup = true;
        }
        catch( const Exception& )
        {}
    }

    if ( pImpl->m_aBackupURL.isEmpty() )
        aTransactTemp.EnableKillingFile();
}


void SfxMedium::DoInternalBackup_Impl( const ::ucbhelper::Content& aOriginalContent )
{
    if ( !pImpl->m_aBackupURL.isEmpty() )
        return; // the backup was done already

    OUString aFileName =  GetURLObject().getName( INetURLObject::LAST_SEGMENT,
                                                        true,
                                                        INetURLObject::DecodeMechanism::NONE );

    sal_Int32 nPrefixLen = aFileName.lastIndexOf( '.' );
    OUString aPrefix = ( nPrefixLen == -1 ) ? aFileName : aFileName.copy( 0, nPrefixLen );
    OUString aExtension = ( nPrefixLen == -1 ) ? OUString() : aFileName.copy( nPrefixLen );
    OUString aBakDir = SvtPathOptions().GetBackupPath();

    // create content for the parent folder ( = backup folder )
    ::ucbhelper::Content  aContent;
    Reference < css::ucb::XCommandEnvironment > xEnv;
    if( ::utl::UCBContentHelper::ensureFolder(comphelper::getProcessComponentContext(), xEnv, aBakDir, aContent) )
        DoInternalBackup_Impl( aOriginalContent, aPrefix, aExtension, aBakDir );

    if ( !pImpl->m_aBackupURL.isEmpty() )
        return;

    // the copiing to the backup catalog failed ( for example because
    // of using an encrypted partition as target catalog )
    // since the user did not specify to make backup explicitly
    // office should try to make backup in another place,
    // target catalog does not look bad for this case ( and looks
    // to be the only way for encrypted partitions )

    INetURLObject aDest = GetURLObject();
    if ( aDest.removeSegment() )
        DoInternalBackup_Impl( aOriginalContent, aPrefix, aExtension, aDest.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
}


void SfxMedium::DoBackup_Impl()
{
    // source file name is the logical name of this medium
    INetURLObject aSource( GetURLObject() );

    // there is nothing to backup in case source file does not exist
    if ( !::utl::UCBContentHelper::IsDocument( aSource.GetMainURL( INetURLObject::DecodeMechanism::NONE ) ) )
        return;

    bool        bSuccess = false;

    // get path for backups
    OUString aBakDir = SvtPathOptions().GetBackupPath();
    if( !aBakDir.isEmpty() )
    {
        // create content for the parent folder ( = backup folder )
        ::ucbhelper::Content  aContent;
        Reference < css::ucb::XCommandEnvironment > xEnv;
        if( ::utl::UCBContentHelper::ensureFolder(comphelper::getProcessComponentContext(), xEnv, aBakDir, aContent) )
        {
            // save as ".bak" file
            INetURLObject aDest( aBakDir );
            aDest.insertName( aSource.getName() );
            aDest.setExtension( "bak" );
            OUString aFileName = aDest.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset );

            // create a content for the source file
            ::ucbhelper::Content aSourceContent;
            if ( ::ucbhelper::Content::create( aSource.GetMainURL( INetURLObject::DecodeMechanism::NONE ), xEnv, comphelper::getProcessComponentContext(), aSourceContent ) )
            {
                try
                {
                    // do the transfer ( copy source file to backup dir )
                    OUString sMimeType = pImpl->getFilterMimeType();
                    aContent.transferContent( aSourceContent,
                                                        ::ucbhelper::InsertOperation::Copy,
                                                        aFileName,
                                                        NameClash::OVERWRITE,
                                                        sMimeType );
                    pImpl->m_aBackupURL = aDest.GetMainURL( INetURLObject::DecodeMechanism::NONE );
                    pImpl->m_bRemoveBackup = false;
                    bSuccess = true;
                }
                catch ( const css::uno::Exception& )
                {
                }
            }
        }
    }

    if ( !bSuccess )
    {
        pImpl->m_eError = ERRCODE_SFX_CANTCREATEBACKUP;
    }
}


void SfxMedium::ClearBackup_Impl()
{
    if( pImpl->m_bRemoveBackup )
    {
        // currently a document is always stored in a new medium,
        // thus if a backup can not be removed the backup URL should not be cleaned
        if ( !pImpl->m_aBackupURL.isEmpty() )
        {
            if ( ::utl::UCBContentHelper::Kill( pImpl->m_aBackupURL ) )
            {
                pImpl->m_bRemoveBackup = false;
                pImpl->m_aBackupURL.clear();
            }
            else
            {

                SAL_WARN( "sfx.doc", "Couldn't remove backup file!");
            }
        }
    }
    else
        pImpl->m_aBackupURL.clear();
}


void SfxMedium::GetLockingStream_Impl()
{
    if ( GetURLObject().GetProtocol() != INetProtocol::File
         || pImpl->m_xLockingStream.is() )
        return;

    const SfxUnoAnyItem* pWriteStreamItem = SfxItemSet::GetItem<SfxUnoAnyItem>(pImpl->m_pSet.get(), SID_STREAM, false);
    if ( pWriteStreamItem )
        pWriteStreamItem->GetValue() >>= pImpl->m_xLockingStream;

    if ( pImpl->m_xLockingStream.is() )
        return;

    // open the original document
    uno::Sequence< beans::PropertyValue > xProps;
    TransformItems( SID_OPENDOC, *GetItemSet(), xProps );
    utl::MediaDescriptor aMedium( xProps );

    aMedium.addInputStreamOwnLock();

    uno::Reference< io::XInputStream > xInputStream;
    aMedium[utl::MediaDescriptor::PROP_STREAM()] >>= pImpl->m_xLockingStream;
    aMedium[utl::MediaDescriptor::PROP_INPUTSTREAM()] >>= xInputStream;

    if ( !pImpl->pTempFile && pImpl->m_aName.isEmpty() )
    {
        // the medium is still based on the original file, it makes sense to initialize the streams
        if ( pImpl->m_xLockingStream.is() )
            pImpl->xStream = pImpl->m_xLockingStream;

        if ( xInputStream.is() )
            pImpl->xInputStream = xInputStream;

        if ( !pImpl->xInputStream.is() && pImpl->xStream.is() )
            pImpl->xInputStream = pImpl->xStream->getInputStream();
    }
}


void SfxMedium::GetMedium_Impl()
{
    if ( pImpl->m_pInStream
        && (!pImpl->bIsTemp || pImpl->xInputStream.is() || pImpl->m_xInputStreamToLoadFrom.is() || pImpl->xStream.is() || pImpl->m_xLockingStream.is() ) )
        return;

    pImpl->bDownloadDone = false;
    Reference< css::task::XInteractionHandler > xInteractionHandler = GetInteractionHandler();

    //TODO/MBA: need support for SID_STREAM
    const SfxUnoAnyItem* pWriteStreamItem = SfxItemSet::GetItem<SfxUnoAnyItem>(pImpl->m_pSet.get(), SID_STREAM, false);
    const SfxUnoAnyItem* pInStreamItem = SfxItemSet::GetItem<SfxUnoAnyItem>(pImpl->m_pSet.get(), SID_INPUTSTREAM, false);
    if ( pWriteStreamItem )
    {
        pWriteStreamItem->GetValue() >>= pImpl->xStream;

        if ( pInStreamItem )
            pInStreamItem->GetValue() >>= pImpl->xInputStream;

        if ( !pImpl->xInputStream.is() && pImpl->xStream.is() )
            pImpl->xInputStream = pImpl->xStream->getInputStream();
    }
    else if ( pInStreamItem )
    {
        pInStreamItem->GetValue() >>= pImpl->xInputStream;
    }
    else
    {
        uno::Sequence < beans::PropertyValue > xProps;
        OUString aFileName;
        if (!pImpl->m_aName.isEmpty())
        {
            if ( osl::FileBase::getFileURLFromSystemPath( pImpl->m_aName, aFileName )
                 != osl::FileBase::E_None )
            {
                SAL_WARN( "sfx.doc", "Physical name not convertible!");
            }
        }
        else
            aFileName = GetName();

        // in case the temporary file exists the streams should be initialized from it,
        // but the original MediaDescriptor should not be changed
        bool bFromTempFile = ( pImpl->pTempFile != nullptr );

        if ( !bFromTempFile )
        {
            GetItemSet()->Put( SfxStringItem( SID_FILE_NAME, aFileName ) );
            if( !(pImpl->m_nStorOpenMode & StreamMode::WRITE) )
                GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, true ) );
            if (xInteractionHandler.is())
                GetItemSet()->Put( SfxUnoAnyItem( SID_INTERACTIONHANDLER, makeAny(xInteractionHandler) ) );
        }

        if ( pImpl->m_xInputStreamToLoadFrom.is() )
        {
            pImpl->xInputStream = pImpl->m_xInputStreamToLoadFrom;
            if (pImpl->m_bInputStreamIsReadOnly)
                GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, true ) );
        }
        else
        {
            TransformItems( SID_OPENDOC, *GetItemSet(), xProps );
            utl::MediaDescriptor aMedium( xProps );

            if ( pImpl->m_xLockingStream.is() && !bFromTempFile )
            {
                // the medium is not based on the temporary file, so the original stream can be used
                pImpl->xStream = pImpl->m_xLockingStream;
            }
            else
            {
                if ( bFromTempFile )
                {
                    aMedium[utl::MediaDescriptor::PROP_URL()] <<= aFileName;
                    aMedium.erase( utl::MediaDescriptor::PROP_READONLY() );
                    aMedium.addInputStream();
                }
                else if ( GetURLObject().GetProtocol() == INetProtocol::File )
                {
                    // use the special locking approach only for file URLs
                    aMedium.addInputStreamOwnLock();
                }
                else
                {
                    // add a check for protocol, if it's http or https or provide webdav then add
                    // the interaction handler to be used by the authentication dialog
                    if ( GetURLObject().isAnyKnownWebDAVScheme() )
                    {
                        aMedium[utl::MediaDescriptor::PROP_AUTHENTICATIONHANDLER()] <<= GetInteractionHandler( true );
                    }
                    aMedium.addInputStream();
                }
                // the ReadOnly property set in aMedium is ignored
                // the check is done in LockOrigFileOnDemand() for file and non-file URLs

                //TODO/MBA: what happens if property is not there?!
                aMedium[utl::MediaDescriptor::PROP_STREAM()] >>= pImpl->xStream;
                aMedium[utl::MediaDescriptor::PROP_INPUTSTREAM()] >>= pImpl->xInputStream;
            }

            GetContent();
            if ( !pImpl->xInputStream.is() && pImpl->xStream.is() )
                pImpl->xInputStream = pImpl->xStream->getInputStream();
        }

        if ( !bFromTempFile )
        {
            //TODO/MBA: need support for SID_STREAM
            if ( pImpl->xStream.is() )
                GetItemSet()->Put( SfxUnoAnyItem( SID_STREAM, makeAny( pImpl->xStream ) ) );

            GetItemSet()->Put( SfxUnoAnyItem( SID_INPUTSTREAM, makeAny( pImpl->xInputStream ) ) );
        }
    }

    //TODO/MBA: ErrorHandling - how to transport error from MediaDescriptor
    if ( !GetError() && !pImpl->xStream.is() && !pImpl->xInputStream.is() )
        SetError(ERRCODE_IO_ACCESSDENIED);

    if ( !GetError() && !pImpl->m_pInStream )
    {
        if ( pImpl->xStream.is() )
            pImpl->m_pInStream = utl::UcbStreamHelper::CreateStream( pImpl->xStream );
        else if ( pImpl->xInputStream.is() )
            pImpl->m_pInStream = utl::UcbStreamHelper::CreateStream( pImpl->xInputStream );
    }

    pImpl->bDownloadDone = true;
    pImpl->aDoneLink.ClearPendingCall();
    ErrCode nError = GetError();
    pImpl->aDoneLink.Call( reinterpret_cast<void*>(sal_uInt32(nError)) );
}

bool SfxMedium::IsRemote() const
{
    return pImpl->m_bRemote;
}

void SfxMedium::SetUpdatePickList(bool bVal)
{
    pImpl->bUpdatePickList = bVal;
}

bool SfxMedium::IsUpdatePickList() const
{
    return pImpl->bUpdatePickList;
}

void SfxMedium::SetLongName(const OUString &rName)
{
    pImpl->m_aLongName = rName;
}

const OUString& SfxMedium::GetLongName() const
{
    return pImpl->m_aLongName;
}

void SfxMedium::SetDoneLink( const Link<void*,void>& rLink )
{
    pImpl->aDoneLink = rLink;
}

void SfxMedium::Download( const Link<void*,void>& aLink )
{
    SetDoneLink( aLink );
    GetInStream();
    if ( pImpl->m_pInStream && !aLink.IsSet() )
    {
        while( !pImpl->bDownloadDone )
            Application::Yield();
    }
}


void SfxMedium::Init_Impl()
/*  [Description]
    Includes a valid:: sun:: com:: star:: util:: URL (If a file name was
    previously in there) in the logical name and if available sets the
    physical name as the file name.
 */

{
    Reference< XOutputStream > rOutStream;

    // TODO/LATER: handle lifetime of storages
    pImpl->bDisposeStorage = false;

    const SfxStringItem* pSalvageItem = SfxItemSet::GetItem<SfxStringItem>(pImpl->m_pSet.get(), SID_DOC_SALVAGE, false);
    if ( pSalvageItem && pSalvageItem->GetValue().isEmpty() )
    {
        pSalvageItem = nullptr;
        pImpl->m_pSet->ClearItem( SID_DOC_SALVAGE );
    }

    if (!pImpl->m_aLogicName.isEmpty())
    {
        INetURLObject aUrl( pImpl->m_aLogicName );
        INetProtocol eProt = aUrl.GetProtocol();
        if ( eProt == INetProtocol::NotValid )
        {
            SAL_WARN( "sfx.doc", "URL <" << pImpl->m_aLogicName << "> with unknown protocol" );
        }
        else
        {
            if ( aUrl.HasMark() )
            {
                pImpl->m_aLogicName = aUrl.GetURLNoMark( INetURLObject::DecodeMechanism::NONE );
                GetItemSet()->Put( SfxStringItem( SID_JUMPMARK, aUrl.GetMark() ) );
            }

            // try to convert the URL into a physical name - but never change a physical name
            // physical name may be set if the logical name is changed after construction
            if ( pImpl->m_aName.isEmpty() )
                osl::FileBase::getSystemPathFromFileURL( GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE ), pImpl->m_aName );
            else
            {
                DBG_ASSERT( pSalvageItem, "Suspicious change of logical name!" );
            }
        }
    }

    if ( pSalvageItem && !pSalvageItem->GetValue().isEmpty() )
    {
        pImpl->m_aLogicName = pSalvageItem->GetValue();
        pImpl->m_pURLObj.reset();
        pImpl->m_bSalvageMode = true;
    }

    // in case output stream is by mistake here
    // clear the reference
    const SfxUnoAnyItem* pOutStreamItem = SfxItemSet::GetItem<SfxUnoAnyItem>(pImpl->m_pSet.get(), SID_OUTPUTSTREAM, false);
    if( pOutStreamItem
     && ( !( pOutStreamItem->GetValue() >>= rOutStream )
          || !pImpl->m_aLogicName.startsWith("private:stream")) )
    {
        pImpl->m_pSet->ClearItem( SID_OUTPUTSTREAM );
        SAL_WARN( "sfx.doc", "Unexpected Output stream parameter!" );
    }

    if (!pImpl->m_aLogicName.isEmpty())
    {
        // if the logic name is set it should be set in MediaDescriptor as well
        const SfxStringItem* pFileNameItem = SfxItemSet::GetItem<SfxStringItem>(pImpl->m_pSet.get(), SID_FILE_NAME, false);
        if ( !pFileNameItem )
        {
            // let the ItemSet be created if necessary
            GetItemSet()->Put(
                SfxStringItem(
                    SID_FILE_NAME, INetURLObject( pImpl->m_aLogicName ).GetMainURL( INetURLObject::DecodeMechanism::NONE ) ) );
        }
    }

    SetIsRemote_Impl();

    osl::DirectoryItem item;
    if (osl::DirectoryItem::get(GetName(), item) == osl::FileBase::E_None) {
        osl::FileStatus stat(osl_FileStatus_Mask_Attributes);
        if (item.getFileStatus(stat) == osl::FileBase::E_None
            && stat.isValid(osl_FileStatus_Mask_Attributes))
        {
            if ((stat.getAttributes() & osl_File_Attribute_ReadOnly) != 0) {
                pImpl->m_bOriginallyReadOnly = true;
            }
        }
    }
}


SfxMedium::SfxMedium() : pImpl(new SfxMedium_Impl)
{
    Init_Impl();
}


void SfxMedium::UseInteractionHandler( bool bUse )
{
    pImpl->bAllowDefaultIntHdl = bUse;
}


css::uno::Reference< css::task::XInteractionHandler >
SfxMedium::GetInteractionHandler( bool bGetAlways )
{
    // if interaction isn't allowed explicitly ... return empty reference!
    if ( !bGetAlways && !pImpl->bUseInteractionHandler )
        return css::uno::Reference< css::task::XInteractionHandler >();

    // search a possible existing handler inside cached item set
    if ( pImpl->m_pSet )
    {
        css::uno::Reference< css::task::XInteractionHandler > xHandler;
        const SfxUnoAnyItem* pHandler = SfxItemSet::GetItem<SfxUnoAnyItem>(pImpl->m_pSet.get(), SID_INTERACTIONHANDLER, false);
        if ( pHandler && (pHandler->GetValue() >>= xHandler) && xHandler.is() )
            return xHandler;
    }

    // if default interaction isn't allowed explicitly ... return empty reference!
    if ( !bGetAlways && !pImpl->bAllowDefaultIntHdl )
        return css::uno::Reference< css::task::XInteractionHandler >();

    // otherwise return cached default handler ... if it exist.
    if ( pImpl->xInteraction.is() )
        return pImpl->xInteraction;

    // create default handler and cache it!
    Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    pImpl->xInteraction.set(
        task::InteractionHandler::createWithParent(xContext, nullptr), UNO_QUERY_THROW );
    return pImpl->xInteraction;
}


void SfxMedium::SetFilter( const std::shared_ptr<const SfxFilter>& pFilter )
{
    pImpl->m_pFilter = pFilter;
}

const std::shared_ptr<const SfxFilter>& SfxMedium::GetFilter() const
{
    return pImpl->m_pFilter;
}

sal_uInt32 SfxMedium::CreatePasswordToModifyHash( const OUString& aPasswd, bool bWriter )
{
    sal_uInt32 nHash = 0;

    if ( !aPasswd.isEmpty() )
    {
        if ( bWriter )
        {
            nHash = ::comphelper::DocPasswordHelper::GetWordHashAsUINT32( aPasswd );
        }
        else
        {
            rtl_TextEncoding nEncoding = osl_getThreadTextEncoding();
            nHash = ::comphelper::DocPasswordHelper::GetXLHashAsUINT16( aPasswd, nEncoding );
        }
    }

    return nHash;
}


void SfxMedium::Close()
{
    if ( pImpl->xStorage.is() )
    {
        CloseStorage();
    }

    CloseStreams_Impl();

    UnlockFile( false );
}

void SfxMedium::CloseAndRelease()
{
    if ( pImpl->xStorage.is() )
    {
        CloseStorage();
    }

    CloseAndReleaseStreams_Impl();

    UnlockFile( true );
}

void SfxMedium::DisableUnlockWebDAV( bool bDisableUnlockWebDAV )
{
    pImpl->m_bDisableUnlockWebDAV = bDisableUnlockWebDAV;
}

void SfxMedium::DisableFileSync(bool bDisableFileSync)
{
    pImpl->m_bDisableFileSync = bDisableFileSync;
}

void SfxMedium::UnlockFile( bool bReleaseLockStream )
{
#if !HAVE_FEATURE_MULTIUSER_ENVIRONMENT
    (void) bReleaseLockStream;
#else
    // check if webdav
    if ( GetURLObject().isAnyKnownWebDAVScheme() )
    {
        // do nothing if WebDAV locking if disabled
        // (shouldn't happen because we already skipped locking,
        // see LockOrigFileOnDemand, but just in case ...)
        if (!IsWebDAVLockingUsed())
            return;

        if ( pImpl->m_bLocked )
        {
            // an interaction handler should be used for authentication, if needed
            try {
                uno::Reference< css::task::XInteractionHandler > xHandler = GetInteractionHandler( true );
                uno::Reference< css::ucb::XCommandEnvironment > xComEnv = new ::ucbhelper::CommandEnvironment( xHandler,
                                                               Reference< css::ucb::XProgressHandler >() );
                ucbhelper::Content aContentToUnlock( GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE ), xComEnv, comphelper::getProcessComponentContext());
                pImpl->m_bLocked = false;
                //check if WebDAV unlock was explicitly disabled
                if ( !pImpl->m_bDisableUnlockWebDAV )
                    aContentToUnlock.unlock();
            }
            catch ( uno::Exception& )
            {
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN( "sfx.doc", "Locking exception: WebDAV while trying to lock the file " << exceptionToString(ex) );
            }
        }
        return;
    }

    if ( pImpl->m_xLockingStream.is() )
    {
        if ( bReleaseLockStream )
        {
            try
            {
                uno::Reference< io::XInputStream > xInStream = pImpl->m_xLockingStream->getInputStream();
                uno::Reference< io::XOutputStream > xOutStream = pImpl->m_xLockingStream->getOutputStream();
                if ( xInStream.is() )
                    xInStream->closeInput();
                if ( xOutStream.is() )
                    xOutStream->closeOutput();
            }
            catch( const uno::Exception& )
            {}
        }

        pImpl->m_xLockingStream.clear();
    }

    if ( !pImpl->m_bLocked )
        return;

    ::svt::DocumentLockFile aLockFile( pImpl->m_aLogicName );

    try
    {
        pImpl->m_bLocked = false;
        // TODO/LATER: A warning could be shown in case the file is not the own one
        aLockFile.RemoveFile();
    }
    catch( const io::WrongFormatException& )
    {
        try
        {
            // erase the empty or corrupt file
            aLockFile.RemoveFileDirectly();
        }
        catch( const uno::Exception& )
        {}
    }
    catch( const uno::Exception& )
    {}
#endif
}

void SfxMedium::CloseAndReleaseStreams_Impl()
{
    CloseZipStorage_Impl();

    uno::Reference< io::XInputStream > xInToClose = pImpl->xInputStream;
    uno::Reference< io::XOutputStream > xOutToClose;
    if ( pImpl->xStream.is() )
    {
        xOutToClose = pImpl->xStream->getOutputStream();

        // if the locking stream is closed here the related member should be cleaned
        if ( pImpl->xStream == pImpl->m_xLockingStream )
            pImpl->m_xLockingStream.clear();
    }

    // The probably existing SvStream wrappers should be closed first
    CloseStreams_Impl();

    // in case of salvage mode the storage is based on the streams
    if ( pImpl->m_bSalvageMode )
        return;

    try
    {
        if ( xInToClose.is() )
            xInToClose->closeInput();
        if ( xOutToClose.is() )
            xOutToClose->closeOutput();
    }
    catch ( const uno::Exception& )
    {
    }
}


void SfxMedium::CloseStreams_Impl()
{
    CloseInStream_Impl();
    CloseOutStream_Impl();

    if ( pImpl->m_pSet )
        pImpl->m_pSet->ClearItem( SID_CONTENT );

    pImpl->aContent = ::ucbhelper::Content();
}


void SfxMedium::SetIsRemote_Impl()
{
    INetURLObject aObj( GetName() );
    switch( aObj.GetProtocol() )
    {
        case INetProtocol::Ftp:
        case INetProtocol::Http:
        case INetProtocol::Https:
            pImpl->m_bRemote = true;
        break;
        default:
            pImpl->m_bRemote = GetName().startsWith("private:msgid");
            break;
    }

    // As files that are written to the remote transmission must also be able
    // to be read.
    if (pImpl->m_bRemote)
        pImpl->m_nStorOpenMode |= StreamMode::READ;
}


void SfxMedium::SetName( const OUString& aNameP, bool bSetOrigURL )
{
    if (pImpl->aOrigURL.isEmpty())
        pImpl->aOrigURL = pImpl->m_aLogicName;
    if( bSetOrigURL )
        pImpl->aOrigURL = aNameP;
    pImpl->m_aLogicName = aNameP;
    pImpl->m_pURLObj.reset();
    pImpl->aContent = ::ucbhelper::Content();
    Init_Impl();
}


const OUString& SfxMedium::GetOrigURL() const
{
    return pImpl->aOrigURL.isEmpty() ? pImpl->m_aLogicName : pImpl->aOrigURL;
}


void SfxMedium::SetPhysicalName_Impl( const OUString& rNameP )
{
    if ( rNameP != pImpl->m_aName )
    {
        pImpl->pTempFile.reset();

        if ( !pImpl->m_aName.isEmpty() || !rNameP.isEmpty() )
            pImpl->aContent = ::ucbhelper::Content();

        pImpl->m_aName = rNameP;
        pImpl->m_bTriedStorage = false;
        pImpl->bIsStorage = false;
    }
}


void SfxMedium::ReOpen()
{
    bool bUseInteractionHandler = pImpl->bUseInteractionHandler;
    pImpl->bUseInteractionHandler = false;
    GetMedium_Impl();
    pImpl->bUseInteractionHandler = bUseInteractionHandler;
}


void SfxMedium::CompleteReOpen()
{
    // do not use temporary file for reopen and in case of success throw the temporary file away
    bool bUseInteractionHandler = pImpl->bUseInteractionHandler;
    pImpl->bUseInteractionHandler = false;

    std::unique_ptr<::utl::TempFile> pTmpFile;
    if ( pImpl->pTempFile )
    {
        pTmpFile = std::move(pImpl->pTempFile);
        pImpl->m_aName.clear();
    }

    GetMedium_Impl();

    if ( GetError() )
    {
        if ( pImpl->pTempFile )
        {
            pImpl->pTempFile->EnableKillingFile();
            pImpl->pTempFile.reset();
        }
        pImpl->pTempFile = std::move( pTmpFile );
        if ( pImpl->pTempFile )
            pImpl->m_aName = pImpl->pTempFile->GetFileName();
    }
    else if (pTmpFile)
    {
        pTmpFile->EnableKillingFile();
        pTmpFile.reset();
    }

    pImpl->bUseInteractionHandler = bUseInteractionHandler;
}

SfxMedium::SfxMedium(const OUString &rName, StreamMode nOpenMode, std::shared_ptr<const SfxFilter> pFilter, std::unique_ptr<SfxItemSet> pInSet) :
    pImpl(new SfxMedium_Impl)
{
    pImpl->m_pSet = std::move( pInSet );
    pImpl->m_pFilter = std::move(pFilter);
    pImpl->m_aLogicName = rName;
    pImpl->m_nStorOpenMode = nOpenMode;
    Init_Impl();
}

SfxMedium::SfxMedium(const OUString &rName, const OUString &rReferer, StreamMode nOpenMode, std::shared_ptr<const SfxFilter> pFilter, std::unique_ptr<SfxItemSet> pInSet) :
    pImpl(new SfxMedium_Impl)
{
    pImpl->m_pSet = std::move(pInSet);
    SfxItemSet * s = GetItemSet();
    if (s->GetItem(SID_REFERER) == nullptr) {
        s->Put(SfxStringItem(SID_REFERER, rReferer));
    }
    pImpl->m_pFilter = std::move(pFilter);
    pImpl->m_aLogicName = rName;
    pImpl->m_nStorOpenMode = nOpenMode;
    Init_Impl();
}

SfxMedium::SfxMedium( const uno::Sequence<beans::PropertyValue>& aArgs ) :
    pImpl(new SfxMedium_Impl)
{
    SfxAllItemSet *pParams = new SfxAllItemSet( SfxGetpApp()->GetPool() );
    pImpl->m_pSet.reset( pParams );
    TransformParameters( SID_OPENDOC, aArgs, *pParams );

    OUString aFilterProvider, aFilterName;
    {
        const SfxPoolItem* pItem = nullptr;
        if (pImpl->m_pSet->HasItem(SID_FILTER_PROVIDER, &pItem))
            aFilterProvider = static_cast<const SfxStringItem*>(pItem)->GetValue();

        if (pImpl->m_pSet->HasItem(SID_FILTER_NAME, &pItem))
            aFilterName = static_cast<const SfxStringItem*>(pItem)->GetValue();
    }

    if (aFilterProvider.isEmpty())
    {
        // This is a conventional filter type.
        pImpl->m_pFilter = SfxGetpApp()->GetFilterMatcher().GetFilter4FilterName( aFilterName );
    }
    else
    {
        // This filter is from an external provider such as orcus.
        pImpl->m_pCustomFilter.reset(new SfxFilter(aFilterProvider, aFilterName));
        pImpl->m_pFilter = pImpl->m_pCustomFilter;
    }

    const SfxStringItem* pSalvageItem = SfxItemSet::GetItem<SfxStringItem>(pImpl->m_pSet.get(), SID_DOC_SALVAGE, false);
    if( pSalvageItem )
    {
        // QUESTION: there is some treatment of Salvage in Init_Impl; align!
        if ( !pSalvageItem->GetValue().isEmpty() )
        {
            // if an URL is provided in SalvageItem that means that the FileName refers to a temporary file
            // that must be copied here

            const SfxStringItem* pFileNameItem = SfxItemSet::GetItem<SfxStringItem>(pImpl->m_pSet.get(), SID_FILE_NAME, false);
            if (!pFileNameItem) throw uno::RuntimeException();
            OUString aNewTempFileURL = SfxMedium::CreateTempCopyWithExt( pFileNameItem->GetValue() );
            if ( !aNewTempFileURL.isEmpty() )
            {
                pImpl->m_pSet->Put( SfxStringItem( SID_FILE_NAME, aNewTempFileURL ) );
                pImpl->m_pSet->ClearItem( SID_INPUTSTREAM );
                pImpl->m_pSet->ClearItem( SID_STREAM );
                pImpl->m_pSet->ClearItem( SID_CONTENT );
            }
            else
            {
                SAL_WARN( "sfx.doc", "Can not create a new temporary file for crash recovery!" );
            }
        }
    }

    const SfxBoolItem* pReadOnlyItem = SfxItemSet::GetItem<SfxBoolItem>(pImpl->m_pSet.get(), SID_DOC_READONLY, false);
    if ( pReadOnlyItem && pReadOnlyItem->GetValue() )
        pImpl->m_bOriginallyLoadedReadOnly = true;

    const SfxStringItem* pFileNameItem = SfxItemSet::GetItem<SfxStringItem>(pImpl->m_pSet.get(), SID_FILE_NAME, false);
    if (!pFileNameItem) throw uno::RuntimeException();
    pImpl->m_aLogicName = pFileNameItem->GetValue();
    pImpl->m_nStorOpenMode = pImpl->m_bOriginallyLoadedReadOnly
        ? SFX_STREAM_READONLY : SFX_STREAM_READWRITE;
    Init_Impl();
}


SfxMedium::SfxMedium( const uno::Reference < embed::XStorage >& rStor, const OUString& rBaseURL, const SfxItemSet* p ) :
    pImpl(new SfxMedium_Impl)
{
    OUString aType = SfxFilter::GetTypeFromStorage(rStor);
    pImpl->m_pFilter = SfxGetpApp()->GetFilterMatcher().GetFilter4EA( aType );
    DBG_ASSERT( pImpl->m_pFilter, "No Filter for storage found!" );

    Init_Impl();
    pImpl->xStorage = rStor;
    pImpl->bDisposeStorage = false;

    // always take BaseURL first, could be overwritten by ItemSet
    GetItemSet()->Put( SfxStringItem( SID_DOC_BASEURL, rBaseURL ) );
    if ( p )
        GetItemSet()->Put( *p );
}


SfxMedium::SfxMedium( const uno::Reference < embed::XStorage >& rStor, const OUString& rBaseURL, const OUString &rTypeName, const SfxItemSet* p ) :
    pImpl(new SfxMedium_Impl)
{
    pImpl->m_pFilter = SfxGetpApp()->GetFilterMatcher().GetFilter4EA( rTypeName );
    DBG_ASSERT( pImpl->m_pFilter, "No Filter for storage found!" );

    Init_Impl();
    pImpl->xStorage = rStor;
    pImpl->bDisposeStorage = false;

    // always take BaseURL first, could be overwritten by ItemSet
    GetItemSet()->Put( SfxStringItem( SID_DOC_BASEURL, rBaseURL ) );
    if ( p )
        GetItemSet()->Put( *p );
}


SfxMedium::~SfxMedium()
{
    // if there is a requirement to clean the backup this is the last possibility to do it
    ClearBackup_Impl();

    Close();

    if( !pImpl->bIsTemp || pImpl->m_aName.isEmpty() )
        return;

    OUString aTemp;
    if ( osl::FileBase::getFileURLFromSystemPath( pImpl->m_aName, aTemp )
         != osl::FileBase::E_None )
    {
        SAL_WARN( "sfx.doc", "Physical name not convertible!");
    }

    if ( !::utl::UCBContentHelper::Kill( aTemp ) )
    {
        SAL_WARN( "sfx.doc", "Couldn't remove temporary file!");
    }
}

const OUString& SfxMedium::GetName() const
{
    return pImpl->m_aLogicName;
}

const INetURLObject& SfxMedium::GetURLObject() const
{
    if (!pImpl->m_pURLObj)
    {
        pImpl->m_pURLObj.reset( new INetURLObject( pImpl->m_aLogicName ) );
        pImpl->m_pURLObj->SetMark("");
    }

    return *pImpl->m_pURLObj;
}

void SfxMedium::SetExpired_Impl( const DateTime& rDateTime )
{
    pImpl->aExpireTime = rDateTime;
}


bool SfxMedium::IsExpired() const
{
    return pImpl->aExpireTime.IsValidAndGregorian() && pImpl->aExpireTime < DateTime( DateTime::SYSTEM );
}


SfxFrame* SfxMedium::GetLoadTargetFrame() const
{
    return pImpl->wLoadTargetFrame;
}

void SfxMedium::setStreamToLoadFrom(const css::uno::Reference<css::io::XInputStream>& xInputStream, bool bIsReadOnly )
{
    pImpl->m_xInputStreamToLoadFrom = xInputStream;
    pImpl->m_bInputStreamIsReadOnly = bIsReadOnly;
}

void SfxMedium::SetLoadTargetFrame(SfxFrame* pFrame )
{
    pImpl->wLoadTargetFrame = pFrame;
}


void SfxMedium::SetStorage_Impl( const uno::Reference < embed::XStorage >& rStor )
{
    pImpl->xStorage = rStor;
}


SfxItemSet* SfxMedium::GetItemSet() const
{
    // this method *must* return an ItemSet, returning NULL can cause crashes
    if (!pImpl->m_pSet)
        pImpl->m_pSet.reset( new SfxAllItemSet( SfxGetpApp()->GetPool() ) );
    return pImpl->m_pSet.get();
}


SvKeyValueIterator* SfxMedium::GetHeaderAttributes_Impl()
{
    if( !pImpl->xAttributes.is() )
    {
        pImpl->xAttributes = SvKeyValueIteratorRef( new SvKeyValueIterator );

        if ( GetContent().is() )
        {
            try
            {
                Any aAny = pImpl->aContent.getPropertyValue("MediaType");
                OUString aContentType;
                aAny >>= aContentType;

                pImpl->xAttributes->Append( SvKeyValue( "content-type", aContentType ) );
            }
            catch ( const css::uno::Exception& )
            {
            }
        }
    }

    return pImpl->xAttributes.get();
}

css::uno::Reference< css::io::XInputStream > const &  SfxMedium::GetInputStream()
{
    if ( !pImpl->xInputStream.is() )
        GetMedium_Impl();
    return pImpl->xInputStream;
}

const uno::Sequence < util::RevisionTag >& SfxMedium::GetVersionList( bool _bNoReload )
{
    // if the medium has no name, then this medium should represent a new document and can have no version info
    if ( ( !_bNoReload || !pImpl->m_bVersionsAlreadyLoaded ) && !pImpl->aVersions.getLength() &&
         ( !pImpl->m_aName.isEmpty() || !pImpl->m_aLogicName.isEmpty() ) && GetStorage().is() )
    {
        uno::Reference < document::XDocumentRevisionListPersistence > xReader =
                document::DocumentRevisionListPersistence::create( comphelper::getProcessComponentContext() );
        try
        {
            pImpl->aVersions = xReader->load( GetStorage() );
        }
        catch ( const uno::Exception& )
        {
        }
    }

    if ( !pImpl->m_bVersionsAlreadyLoaded )
        pImpl->m_bVersionsAlreadyLoaded = true;

    return pImpl->aVersions;
}

uno::Sequence < util::RevisionTag > SfxMedium::GetVersionList( const uno::Reference < embed::XStorage >& xStorage )
{
    uno::Reference < document::XDocumentRevisionListPersistence > xReader =
        document::DocumentRevisionListPersistence::create( comphelper::getProcessComponentContext() );
    try
    {
        return xReader->load( xStorage );
    }
    catch ( const uno::Exception& )
    {
    }

    return uno::Sequence < util::RevisionTag >();
}

void SfxMedium::AddVersion_Impl( util::RevisionTag& rRevision )
{
    if ( !GetStorage().is() )
        return;

    // To determine a unique name for the stream
    std::vector<sal_uInt32> aLongs;
    sal_Int32 nLength = pImpl->aVersions.getLength();
    for ( sal_Int32 m=0; m<nLength; m++ )
    {
        sal_uInt32 nVer = static_cast<sal_uInt32>( pImpl->aVersions[m].Identifier.copy(7).toInt32());
        size_t n;
        for ( n=0; n<aLongs.size(); ++n )
            if ( nVer<aLongs[n] )
                break;

        aLongs.insert( aLongs.begin()+n, nVer );
    }

    std::vector<sal_uInt32>::size_type nKey;
    for ( nKey=0; nKey<aLongs.size(); ++nKey )
        if ( aLongs[nKey] > nKey+1 )
            break;

    OUString aRevName = "Version" + OUString::number( nKey + 1 );
    pImpl->aVersions.realloc( nLength+1 );
    rRevision.Identifier = aRevName;
    pImpl->aVersions[nLength] = rRevision;
}

void SfxMedium::RemoveVersion_Impl( const OUString& rName )
{
    if ( !pImpl->aVersions.getLength() )
        return;

    sal_Int32 nLength = pImpl->aVersions.getLength();
    for ( sal_Int32 n=0; n<nLength; n++ )
    {
        if ( pImpl->aVersions[n].Identifier == rName )
        {
            for ( sal_Int32 m=n; m<nLength-1; m++ )
                pImpl->aVersions[m] = pImpl->aVersions[m+1];
            pImpl->aVersions.realloc(nLength-1);
            return;
        }
    }
}

bool SfxMedium::TransferVersionList_Impl( SfxMedium const & rMedium )
{
    if ( rMedium.pImpl->aVersions.getLength() )
    {
        pImpl->aVersions = rMedium.pImpl->aVersions;
        return true;
    }

    return false;
}

void SfxMedium::SaveVersionList_Impl()
{
    if ( !GetStorage().is() )
        return;

    if ( !pImpl->aVersions.getLength() )
        return;

    uno::Reference < document::XDocumentRevisionListPersistence > xWriter =
             document::DocumentRevisionListPersistence::create( comphelper::getProcessComponentContext() );
    try
    {
        xWriter->store( GetStorage(), pImpl->aVersions );
    }
    catch ( const uno::Exception& )
    {
    }
}

bool SfxMedium::IsReadOnly() const
{
    // a) ReadOnly filter can't produce read/write contents!
    bool bReadOnly = pImpl->m_pFilter && (pImpl->m_pFilter->GetFilterFlags() & SfxFilterFlags::OPENREADONLY);

    // b) if filter allow read/write contents .. check open mode of the storage
    if (!bReadOnly)
        bReadOnly = !( GetOpenMode() & StreamMode::WRITE );

    // c) the API can force the readonly state!
    if (!bReadOnly)
    {
        const SfxBoolItem* pItem = SfxItemSet::GetItem<SfxBoolItem>(GetItemSet(), SID_DOC_READONLY, false);
        if (pItem)
            bReadOnly = pItem->GetValue();
    }

    return bReadOnly;
}

bool SfxMedium::IsOriginallyReadOnly() const
{
    return pImpl->m_bOriginallyReadOnly;
}

bool SfxMedium::IsOriginallyLoadedReadOnly() const
{
    return pImpl->m_bOriginallyLoadedReadOnly;
}

bool SfxMedium::SetWritableForUserOnly( const OUString& aURL )
{
    // UCB does not allow to allow write access only for the user,
    // use osl API
    bool bResult = false;

    ::osl::DirectoryItem aDirItem;
    if ( ::osl::DirectoryItem::get( aURL, aDirItem ) == ::osl::FileBase::E_None )
    {
        ::osl::FileStatus aFileStatus( osl_FileStatus_Mask_Attributes );
        if ( aDirItem.getFileStatus( aFileStatus ) == osl::FileBase::E_None
          && aFileStatus.isValid( osl_FileStatus_Mask_Attributes ) )
        {
            sal_uInt64 nAttributes = aFileStatus.getAttributes();

            nAttributes &= ~(osl_File_Attribute_OwnWrite |
                             osl_File_Attribute_GrpWrite |
                             osl_File_Attribute_OthWrite |
                             osl_File_Attribute_ReadOnly);
            nAttributes |=  (osl_File_Attribute_OwnWrite |
                             osl_File_Attribute_OwnRead);

            bResult = ( osl::File::setAttributes( aURL, nAttributes ) == ::osl::FileBase::E_None );
        }
    }

    return bResult;
}

namespace
{
/// Get the parent directory of a temporary file for output purposes.
OUString GetLogicBase(std::unique_ptr<SfxMedium_Impl> const & pImpl)
{
    OUString aLogicBase;

// In a sandboxed environment we don't want to attempt to create temporary files in the same
// directory where the user has selected an output file to be stored. The sandboxed process has
// permission only to create the specifically named output file in that directory.
#if !HAVE_FEATURE_MACOSX_SANDBOX

    if (comphelper::isFileUrl(pImpl->m_aLogicName) && !pImpl->m_pInStream)
    {
        // Try to create the temp file in the same directory when storing.
        sal_Int32 nOffset = pImpl->m_aLogicName.lastIndexOf("/");
        if (nOffset != -1)
            aLogicBase = pImpl->m_aLogicName.copy(0, nOffset);
        if (aLogicBase == "file://")
            // Doesn't make sense.
            aLogicBase.clear();
    }

    if (pImpl->m_bHasEmbeddedObjects)
        // Embedded objects would mean a special base, ignore that.
        aLogicBase.clear();

#endif // !HAVE_FEATURE_MACOSX_SANDBOX

    return aLogicBase;
}
}

void SfxMedium::CreateTempFile( bool bReplace )
{
    if ( pImpl->pTempFile )
    {
        if ( !bReplace )
            return;

        pImpl->pTempFile.reset();
        pImpl->m_aName.clear();
    }

    OUString aLogicBase = GetLogicBase(pImpl);
    pImpl->pTempFile.reset( new ::utl::TempFile(aLogicBase.isEmpty() ? nullptr : &aLogicBase) );
    pImpl->pTempFile->EnableKillingFile();
    pImpl->m_aName = pImpl->pTempFile->GetFileName();
    OUString aTmpURL = pImpl->pTempFile->GetURL();
    if ( pImpl->m_aName.isEmpty() || aTmpURL.isEmpty() )
    {
        SetError(ERRCODE_IO_CANTWRITE);
        return;
    }

    if ( !(pImpl->m_nStorOpenMode & StreamMode::TRUNC) )
    {
        bool bTransferSuccess = false;

        if ( GetContent().is()
          && GetURLObject().GetProtocol() == INetProtocol::File
          && ::utl::UCBContentHelper::IsDocument( GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE ) ) )
        {
            // if there is already such a document, we should copy it
            // if it is a file system use OS copy process
            try
            {
                uno::Reference< css::ucb::XCommandEnvironment > xComEnv;
                INetURLObject aTmpURLObj( aTmpURL );
                OUString aFileName = aTmpURLObj.getName( INetURLObject::LAST_SEGMENT,
                                                                true,
                                                                INetURLObject::DecodeMechanism::WithCharset );
                if ( !aFileName.isEmpty() && aTmpURLObj.removeSegment() )
                {
                    ::ucbhelper::Content aTargetContent( aTmpURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), xComEnv, comphelper::getProcessComponentContext() );
                    OUString sMimeType = pImpl->getFilterMimeType();
                    aTargetContent.transferContent( pImpl->aContent, ::ucbhelper::InsertOperation::Copy, aFileName, NameClash::OVERWRITE, sMimeType );
                    SetWritableForUserOnly( aTmpURL );
                    bTransferSuccess = true;
                }
            }
            catch( const uno::Exception& )
            {}

            if ( bTransferSuccess )
            {
                CloseOutStream();
                CloseInStream();
            }
        }

        if ( !bTransferSuccess && pImpl->m_pInStream )
        {
            // the case when there is no URL-access available or this is a remote protocol
            // but there is an input stream
            GetOutStream();
            if ( pImpl->m_pOutStream )
            {
                std::unique_ptr<char[]> pBuf(new char [8192]);
                ErrCode      nErr = ERRCODE_NONE;

                pImpl->m_pInStream->Seek(0);
                pImpl->m_pOutStream->Seek(0);

                while( !pImpl->m_pInStream->eof() && nErr == ERRCODE_NONE )
                {
                    sal_uInt32 nRead = pImpl->m_pInStream->ReadBytes(pBuf.get(), 8192);
                    nErr = pImpl->m_pInStream->GetError();
                    pImpl->m_pOutStream->WriteBytes( pBuf.get(), nRead );
                }

                bTransferSuccess = true;
                CloseInStream();
            }
            CloseOutStream_Impl();
        }
        else
        {
            // Quite strange design, but currently it is expected that in this case no transfer happens
            // TODO/LATER: get rid of this inconsistent part of the call design
            bTransferSuccess = true;
            CloseInStream();
        }

        if ( !bTransferSuccess )
        {
            SetError(ERRCODE_IO_CANTWRITE);
            return;
        }
    }

    CloseStorage();
}


void SfxMedium::CreateTempFileNoCopy()
{
    // this call always replaces the existing temporary file
    pImpl->pTempFile.reset();

    OUString aLogicBase = GetLogicBase(pImpl);
    pImpl->pTempFile.reset( new ::utl::TempFile(aLogicBase.isEmpty() ? nullptr : &aLogicBase) );
    pImpl->pTempFile->EnableKillingFile();
    pImpl->m_aName = pImpl->pTempFile->GetFileName();
    if ( pImpl->m_aName.isEmpty() )
    {
        SetError(ERRCODE_IO_CANTWRITE);
        return;
    }

    CloseOutStream_Impl();
    CloseStorage();
}

bool SfxMedium::SignDocumentContentUsingCertificate(bool bHasValidDocumentSignature,
                                                    const Reference<XCertificate>& xCertificate)
{
    bool bChanges = false;

    if (IsOpen() || GetError())
    {
        SAL_WARN("sfx.doc", "The medium must be closed by the signer!");
        return bChanges;
    }

    // The component should know if there was a valid document signature, since
    // it should show a warning in this case
    OUString aODFVersion(comphelper::OStorageHelper::GetODFVersionFromStorage(GetStorage()));
    uno::Reference< security::XDocumentDigitalSignatures > xSigner(
        security::DocumentDigitalSignatures::createWithVersionAndValidSignature(
            comphelper::getProcessComponentContext(), aODFVersion, bHasValidDocumentSignature ) );

    uno::Reference< embed::XStorage > xWriteableZipStor;

    // we can reuse the temporary file if there is one already
    CreateTempFile( false );
    GetMedium_Impl();

    try
    {
        if ( !pImpl->xStream.is() )
            throw uno::RuntimeException();

        bool bODF = GetFilter()->IsOwnFormat();
        try
        {
            xWriteableZipStor = ::comphelper::OStorageHelper::GetStorageOfFormatFromStream( ZIP_STORAGE_FORMAT_STRING, pImpl->xStream );
        }
        catch (const io::IOException& rException)
        {
            if (bODF)
                SAL_WARN("sfx.doc", "ODF stream is not a zip storage: " << rException);
        }

        if ( !xWriteableZipStor.is() && bODF )
            throw uno::RuntimeException();

        uno::Reference< embed::XStorage > xMetaInf;
        uno::Reference<container::XNameAccess> xNameAccess(xWriteableZipStor, uno::UNO_QUERY);
        if (xNameAccess.is() && xNameAccess->hasByName("META-INF"))
        {
            xMetaInf = xWriteableZipStor->openStorageElement(
                                            "META-INF",
                                            embed::ElementModes::READWRITE );
            if ( !xMetaInf.is() )
                throw uno::RuntimeException();
        }

        {
            if (xMetaInf.is())
            {
                // ODF.
                uno::Reference< io::XStream > xStream;
                if (GetFilter() && GetFilter()->IsOwnFormat())
                    xStream.set(xMetaInf->openStreamElement(xSigner->getDocumentContentSignatureDefaultStreamName(), embed::ElementModes::READWRITE), uno::UNO_SET_THROW);

                bool bSuccess = xSigner->signDocumentWithCertificate(xCertificate, GetZipStorageToSign_Impl(), xStream);

                if (bSuccess)
                {
                    uno::Reference< embed::XTransactedObject > xTransact( xMetaInf, uno::UNO_QUERY_THROW );
                    xTransact->commit();
                    xTransact.set( xWriteableZipStor, uno::UNO_QUERY_THROW );
                    xTransact->commit();

                    // the temporary file has been written, commit it to the original file
                    Commit();
                    bChanges = true;
                }
            }
            else if (xWriteableZipStor.is())
            {
                // OOXML.
                uno::Reference<io::XStream> xStream;

                    // We need read-write to be able to add the signature relation.
                bool bSuccess =xSigner->signDocumentWithCertificate(
                        xCertificate, GetZipStorageToSign_Impl(/*bReadOnly=*/false), xStream);

                if (bSuccess)
                {
                    uno::Reference<embed::XTransactedObject> xTransact(xWriteableZipStor, uno::UNO_QUERY_THROW);
                    xTransact->commit();

                    // the temporary file has been written, commit it to the original file
                    Commit();
                    bChanges = true;
                }
            }
            else
            {
                // Something not ZIP based: e.g. PDF.
                std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(GetName(), StreamMode::READ | StreamMode::WRITE));
                uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStream));
                if (xSigner->signDocumentWithCertificate(xCertificate, uno::Reference<embed::XStorage>(), xStream))
                    bChanges = true;
            }
        }
    }
    catch ( const uno::Exception& )
    {
        SAL_WARN( "sfx.doc", "Couldn't use signing functionality!" );
    }

    CloseAndRelease();

    ResetError();

    return bChanges;
}

bool SfxMedium::SignContents_Impl(weld::Window* pDialogParent,
                                  bool bSignScriptingContent,
                                  bool bHasValidDocumentSignature,
                                  const OUString& aSignatureLineId,
                                  const Reference<XCertificate>& xCert,
                                  const Reference<XGraphic>& xValidGraphic,
                                  const Reference<XGraphic>& xInvalidGraphic,
                                  const OUString& aComment)
{
    bool bChanges = false;

    if (IsOpen() || GetError())
    {
        SAL_WARN("sfx.doc", "The medium must be closed by the signer!");
        return bChanges;
    }

    // The component should know if there was a valid document signature, since
    // it should show a warning in this case
    OUString aODFVersion(comphelper::OStorageHelper::GetODFVersionFromStorage(GetStorage()));
    uno::Reference< security::XDocumentDigitalSignatures > xSigner(
        security::DocumentDigitalSignatures::createWithVersionAndValidSignature(
            comphelper::getProcessComponentContext(), aODFVersion, bHasValidDocumentSignature ) );
    if (pDialogParent)
        xSigner->setParentWindow(pDialogParent->GetXWindow());

    uno::Reference< embed::XStorage > xWriteableZipStor;

    // we can reuse the temporary file if there is one already
    CreateTempFile( false );
    GetMedium_Impl();

    try
    {
        if ( !pImpl->xStream.is() )
            throw uno::RuntimeException();

        bool bODF = GetFilter()->IsOwnFormat();
        try
        {
            xWriteableZipStor = ::comphelper::OStorageHelper::GetStorageOfFormatFromStream( ZIP_STORAGE_FORMAT_STRING, pImpl->xStream );
        }
        catch (const io::IOException& rException)
        {
            if (bODF)
                SAL_WARN("sfx.doc", "ODF stream is not a zip storage: " << rException);
        }

        if ( !xWriteableZipStor.is() && bODF )
            throw uno::RuntimeException();

        uno::Reference< embed::XStorage > xMetaInf;
        uno::Reference<container::XNameAccess> xNameAccess(xWriteableZipStor, uno::UNO_QUERY);
        if (xNameAccess.is() && xNameAccess->hasByName("META-INF"))
        {
            xMetaInf = xWriteableZipStor->openStorageElement(
                                            "META-INF",
                                            embed::ElementModes::READWRITE );
            if ( !xMetaInf.is() )
                throw uno::RuntimeException();
        }

        if ( bSignScriptingContent )
        {
            // If the signature has already the document signature it will be removed
            // after the scripting signature is inserted.
            uno::Reference< io::XStream > xStream(
                xMetaInf->openStreamElement( xSigner->getScriptingContentSignatureDefaultStreamName(),
                                                embed::ElementModes::READWRITE ),
                uno::UNO_SET_THROW );

            if ( xSigner->signScriptingContent( GetZipStorageToSign_Impl(), xStream ) )
            {
                // remove the document signature if any
                OUString aDocSigName = xSigner->getDocumentContentSignatureDefaultStreamName();
                if ( !aDocSigName.isEmpty() && xMetaInf->hasByName( aDocSigName ) )
                    xMetaInf->removeElement( aDocSigName );

                uno::Reference< embed::XTransactedObject > xTransact( xMetaInf, uno::UNO_QUERY_THROW );
                xTransact->commit();
                xTransact.set( xWriteableZipStor, uno::UNO_QUERY_THROW );
                xTransact->commit();

                // the temporary file has been written, commit it to the original file
                Commit();
                bChanges = true;
            }
        }
        else
        {
            if (xMetaInf.is())
            {
                // ODF.
                uno::Reference< io::XStream > xStream;
                if (GetFilter() && GetFilter()->IsOwnFormat())
                    xStream.set(xMetaInf->openStreamElement(xSigner->getDocumentContentSignatureDefaultStreamName(), embed::ElementModes::READWRITE), uno::UNO_SET_THROW);

                bool bSuccess = false;
                if (xCert.is())
                    bSuccess = xSigner->signSignatureLine(
                        GetZipStorageToSign_Impl(), xStream, aSignatureLineId, xCert,
                        xValidGraphic, xInvalidGraphic, aComment);
                else
                    bSuccess = xSigner->signDocumentContent(GetZipStorageToSign_Impl(),
                                                            xStream);

                if (bSuccess)
                {
                    uno::Reference< embed::XTransactedObject > xTransact( xMetaInf, uno::UNO_QUERY_THROW );
                    xTransact->commit();
                    xTransact.set( xWriteableZipStor, uno::UNO_QUERY_THROW );
                    xTransact->commit();

                    // the temporary file has been written, commit it to the original file
                    Commit();
                    bChanges = true;
                }
            }
            else if (xWriteableZipStor.is())
            {
                // OOXML.
                uno::Reference<io::XStream> xStream;

                bool bSuccess = false;
                if (xCert.is())
                {
                    bSuccess = xSigner->signSignatureLine(
                        GetZipStorageToSign_Impl(/*bReadOnly=*/false), xStream, aSignatureLineId,
                        xCert, xValidGraphic, xInvalidGraphic, aComment);
                }
                else
                {
                    // We need read-write to be able to add the signature relation.
                    bSuccess =xSigner->signDocumentContent(
                        GetZipStorageToSign_Impl(/*bReadOnly=*/false), xStream);
                }

                if (bSuccess)
                {
                    uno::Reference<embed::XTransactedObject> xTransact(xWriteableZipStor, uno::UNO_QUERY_THROW);
                    xTransact->commit();

                    // the temporary file has been written, commit it to the original file
                    Commit();
                    bChanges = true;
                }
            }
            else
            {
                // Something not ZIP based: e.g. PDF.
                std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(GetName(), StreamMode::READ | StreamMode::WRITE));
                uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStream));
                if (xSigner->signDocumentContent(uno::Reference<embed::XStorage>(), xStream))
                    bChanges = true;
            }
        }
    }
    catch ( const uno::Exception& )
    {
        SAL_WARN( "sfx.doc", "Couldn't use signing functionality!" );
    }

    CloseAndRelease();

    ResetError();

    return bChanges;
}


SignatureState SfxMedium::GetCachedSignatureState_Impl()
{
    return pImpl->m_nSignatureState;
}


void SfxMedium::SetCachedSignatureState_Impl( SignatureState nState )
{
    pImpl->m_nSignatureState = nState;
}

void SfxMedium::SetHasEmbeddedObjects(bool bHasEmbeddedObjects)
{
    pImpl->m_bHasEmbeddedObjects = bHasEmbeddedObjects;
}

bool SfxMedium::HasStorage_Impl() const
{
    return pImpl->xStorage.is();
}

bool SfxMedium::IsOpen() const
{
    return pImpl->m_pInStream || pImpl->m_pOutStream || pImpl->xStorage.is();
}

OUString SfxMedium::CreateTempCopyWithExt( const OUString& aURL )
{
    OUString aResult;

    if ( !aURL.isEmpty() )
    {
        sal_Int32 nPrefixLen = aURL.lastIndexOf( '.' );
        OUString aExt = ( nPrefixLen == -1 ) ? OUString() :  aURL.copy( nPrefixLen );

        OUString aNewTempFileURL = ::utl::TempFile( OUString(), true, &aExt ).GetURL();
        if ( !aNewTempFileURL.isEmpty() )
        {
            INetURLObject aSource( aURL );
            INetURLObject aDest( aNewTempFileURL );
            OUString aFileName = aDest.getName( INetURLObject::LAST_SEGMENT,
                                                        true,
                                                        INetURLObject::DecodeMechanism::WithCharset );
            if ( !aFileName.isEmpty() && aDest.removeSegment() )
            {
                try
                {
                    uno::Reference< css::ucb::XCommandEnvironment > xComEnv;
                    ::ucbhelper::Content aTargetContent( aDest.GetMainURL( INetURLObject::DecodeMechanism::NONE ), xComEnv, comphelper::getProcessComponentContext() );
                    ::ucbhelper::Content aSourceContent( aSource.GetMainURL( INetURLObject::DecodeMechanism::NONE ), xComEnv, comphelper::getProcessComponentContext() );
                    aTargetContent.transferContent( aSourceContent,
                                                        ::ucbhelper::InsertOperation::Copy,
                                                        aFileName,
                                                        NameClash::OVERWRITE );
                    aResult = aNewTempFileURL;
                }
                catch( const uno::Exception& )
                {}
            }
        }
    }

    return aResult;
}

bool SfxMedium::CallApproveHandler(const uno::Reference< task::XInteractionHandler >& xHandler, const uno::Any& rRequest, bool bAllowAbort)
{
    bool bResult = false;

    if ( xHandler.is() )
    {
        try
        {
            uno::Sequence< uno::Reference< task::XInteractionContinuation > > aContinuations( bAllowAbort ? 2 : 1 );

            ::rtl::Reference< ::comphelper::OInteractionApprove > pApprove( new ::comphelper::OInteractionApprove );
            aContinuations[ 0 ] = pApprove.get();

            if ( bAllowAbort )
            {
                ::rtl::Reference< ::comphelper::OInteractionAbort > pAbort( new ::comphelper::OInteractionAbort );
                aContinuations[ 1 ] = pAbort.get();
            }

            xHandler->handle(::framework::InteractionRequest::CreateRequest(rRequest, aContinuations));
            bResult = pApprove->wasSelected();
        }
        catch( const Exception& )
        {
        }
    }

    return bResult;
}

OUString SfxMedium::SwitchDocumentToTempFile()
{
    // the method returns empty string in case of failure
    OUString aResult;
    OUString aOrigURL = pImpl->m_aLogicName;

    if ( !aOrigURL.isEmpty() )
    {
        sal_Int32 nPrefixLen = aOrigURL.lastIndexOf( '.' );
        OUString const aExt = (nPrefixLen == -1)
                                ? OUString()
                                : aOrigURL.copy(nPrefixLen);
        OUString aNewURL = ::utl::TempFile( OUString(), true, &aExt ).GetURL();

        // TODO/LATER: In future the aLogicName should be set to shared folder URL
        //             and a temporary file should be created. Transport_Impl should be impossible then.
        if ( !aNewURL.isEmpty() )
        {
            uno::Reference< embed::XStorage > xStorage = GetStorage();
            uno::Reference< embed::XOptimizedStorage > xOptStorage( xStorage, uno::UNO_QUERY );

            if ( xOptStorage.is() )
            {
                // TODO/LATER: reuse the pImpl->pTempFile if it already exists
                CanDisposeStorage_Impl( false );
                Close();
                SetPhysicalName_Impl( OUString() );
                SetName( aNewURL );

                // remove the readonly state
                bool bWasReadonly = false;
                pImpl->m_nStorOpenMode = SFX_STREAM_READWRITE;
                const SfxBoolItem* pReadOnlyItem = SfxItemSet::GetItem<SfxBoolItem>(pImpl->m_pSet.get(), SID_DOC_READONLY, false);
                if ( pReadOnlyItem && pReadOnlyItem->GetValue() )
                    bWasReadonly = true;
                GetItemSet()->ClearItem( SID_DOC_READONLY );

                GetMedium_Impl();
                LockOrigFileOnDemand( false, false );
                CreateTempFile();
                GetMedium_Impl();

                if ( pImpl->xStream.is() )
                {
                    try
                    {
                        xOptStorage->writeAndAttachToStream( pImpl->xStream );
                        pImpl->xStorage = xStorage;
                        aResult = aNewURL;
                    }
                    catch( const uno::Exception& )
                    {}
                }

                if ( aResult.isEmpty() )
                {
                    Close();
                    SetPhysicalName_Impl( OUString() );
                    SetName( aOrigURL );
                    if ( bWasReadonly )
                    {
                        // set the readonly state back
                        pImpl->m_nStorOpenMode = SFX_STREAM_READONLY;
                        GetItemSet()->Put( SfxBoolItem(SID_DOC_READONLY, true));
                    }
                    GetMedium_Impl();
                    pImpl->xStorage = xStorage;
                }
            }
        }
    }

    return aResult;
}

bool SfxMedium::SwitchDocumentToFile( const OUString& aURL )
{
    // the method is only for storage based documents
    bool bResult = false;
    OUString aOrigURL = pImpl->m_aLogicName;

    if ( !aURL.isEmpty() && !aOrigURL.isEmpty() )
    {
        uno::Reference< embed::XStorage > xStorage = GetStorage();
        uno::Reference< embed::XOptimizedStorage > xOptStorage( xStorage, uno::UNO_QUERY );

        // TODO/LATER: reuse the pImpl->pTempFile if it already exists
        CanDisposeStorage_Impl( false );
        Close();
        SetPhysicalName_Impl( OUString() );
        SetName( aURL );

        // open the temporary file based document
        GetMedium_Impl();
        LockOrigFileOnDemand( false, false );
        CreateTempFile();
        GetMedium_Impl();

        if ( pImpl->xStream.is() )
        {
            try
            {
                uno::Reference< io::XTruncate > xTruncate( pImpl->xStream, uno::UNO_QUERY_THROW );
                xTruncate->truncate();
                if ( xOptStorage.is() )
                    xOptStorage->writeAndAttachToStream( pImpl->xStream );
                pImpl->xStorage = xStorage;
                bResult = true;
            }
            catch( const uno::Exception& )
            {}
        }

        if ( !bResult )
        {
            Close();
            SetPhysicalName_Impl( OUString() );
            SetName( aOrigURL );
            GetMedium_Impl();
            pImpl->xStorage = xStorage;
        }
    }

    return bResult;
}

void SfxMedium::SetInCheckIn( bool bInCheckIn )
{
    pImpl->m_bInCheckIn = bInCheckIn;
}

bool SfxMedium::IsInCheckIn( )
{
    return pImpl->m_bInCheckIn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
