/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <sfx2/docfile.hxx>
#include "sfx2/signaturestate.hxx"

#include <uno/mapping.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/document/XDocumentRevisionListPersistence.hpp>
#include <com/sun/star/document/LockedDocumentRequest.hpp>
#include <com/sun/star/document/OwnLockOnDocumentRequest.hpp>
#include <com/sun/star/document/LockedOnSavingRequest.hpp>
#include <com/sun/star/document/LockFileIgnoreRequest.hpp>
#include <com/sun/star/document/ChangedByOthersRequest.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/UseBackupException.hpp>
#include <com/sun/star/embed/XOptimizedStorage.hpp>
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/util/XArchiver.hpp>
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
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/logging/XSimpleLogRing.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/security/DocumentSignatureInformation.hpp>
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <tools/zcodec.hxx>
#include <tools/cachestr.hxx>
#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>
#include <framework/interaction.hxx>
#include <unotools/streamhelper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/msgbox.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/lckbitem.hxx>
#include <svtools/sfxecode.hxx>
#include <svl/itemset.hxx>
#include <svl/intitem.hxx>
#include <svtools/svparser.hxx> // SvKeyValue
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implbase1.hxx>

#define _SVSTDARR_ULONGS
#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>

#include <unotools/streamwrap.hxx>

#include <rtl/logfile.hxx>
#include <osl/file.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;

#include <comphelper/storagehelper.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/docpasswordhelper.hxx>
#include <tools/urlobj.hxx>
#include <tools/inetmime.hxx>
#include <unotools/ucblockbytes.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/asynclink.hxx>
#include <svl/inettype.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/progresshandlerwrap.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/interactionrequest.hxx>
#include <sot/stg.hxx>
#include <unotools/saveopt.hxx>
#include <svl/documentlockfile.hxx>

#include "helper.hxx"
#include <sfx2/request.hxx>      // SFX_ITEMSET_SET
#include <sfx2/app.hxx>          // GetFilterMatcher
#include <sfx2/frame.hxx>        // LoadTargetFrame
#include "fltfnc.hxx"       // SfxFilterMatcher
#include <sfx2/docfilt.hxx>      // SfxFilter
#include <sfx2/objsh.hxx>        // CheckOpenMode
#include <sfx2/docfac.hxx>       // GetFilterContainer
#include "doc.hrc"
#include "openflag.hxx"     // SFX_STREAM_READONLY etc.
#include "sfxresid.hxx"
#include <sfx2/appuno.hxx>
#include "sfxacldetect.hxx"

#define MAX_REDIRECT 5


//==========================================================
namespace {

static const sal_Int8 LOCK_UI_NOLOCK = 0;
static const sal_Int8 LOCK_UI_SUCCEEDED = 1;
static const sal_Int8 LOCK_UI_TRY = 2;

//----------------------------------------------------------------
sal_Bool IsSystemFileLockingUsed()
{
    // check whether system file locking has been used, the default value is false
    sal_Bool bUseSystemLock = sal_False;
    try
    {

        uno::Reference< uno::XInterface > xCommonConfig = ::comphelper::ConfigurationHelper::openConfig(
                            ::comphelper::getProcessServiceFactory(),
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.Common" ) ),
                            ::comphelper::ConfigurationHelper::E_STANDARD );
        if ( !xCommonConfig.is() )
            throw uno::RuntimeException();

        ::comphelper::ConfigurationHelper::readRelativeKey(
                xCommonConfig,
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Misc/" ) ),
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UseDocumentSystemFileLocking" ) ) ) >>= bUseSystemLock;
    }
    catch( const uno::Exception& )
    {
    }

    return bUseSystemLock;
}

//----------------------------------------------------------------
sal_Bool IsOOoLockFileUsed()
{
    // check whether system file locking has been used, the default value is false
    sal_Bool bOOoLockFileUsed = sal_False;
    try
    {

        uno::Reference< uno::XInterface > xCommonConfig = ::comphelper::ConfigurationHelper::openConfig(
                            ::comphelper::getProcessServiceFactory(),
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.Common" ) ),
                            ::comphelper::ConfigurationHelper::E_STANDARD );
        if ( !xCommonConfig.is() )
            throw uno::RuntimeException();

        ::comphelper::ConfigurationHelper::readRelativeKey(
                xCommonConfig,
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Misc/" ) ),
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UseDocumentOOoLockFile" ) ) ) >>= bOOoLockFileUsed;
    }
    catch( const uno::Exception& )
    {
    }

    return bOOoLockFileUsed;
}

} // anonymous namespace
//==========================================================


//----------------------------------------------------------------
class SfxMediumHandler_Impl : public ::cppu::WeakImplHelper1< com::sun::star::task::XInteractionHandler >
{
    com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > m_xInter;

public:
    virtual void SAL_CALL handle( const com::sun::star::uno::Reference< com::sun::star::task::XInteractionRequest >& xRequest )
            throw( com::sun::star::uno::RuntimeException );

    SfxMediumHandler_Impl( com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > xInteraction )
        : m_xInter( xInteraction )
        {}

    ~SfxMediumHandler_Impl();
};

//----------------------------------------------------------------
SfxMediumHandler_Impl::~SfxMediumHandler_Impl()
{
}

//----------------------------------------------------------------
void SAL_CALL SfxMediumHandler_Impl::handle( const com::sun::star::uno::Reference< com::sun::star::task::XInteractionRequest >& xRequest )
        throw( com::sun::star::uno::RuntimeException )
{
    if( !m_xInter.is() )
        return;

    com::sun::star::uno::Any aRequest = xRequest->getRequest();
    com::sun::star::ucb::InteractiveIOException aIoException;
    com::sun::star::ucb::UnsupportedDataSinkException aSinkException;
    if ( (aRequest >>= aIoException) && ( aIoException.Code == IOErrorCode_ACCESS_DENIED || aIoException.Code == IOErrorCode_LOCKING_VIOLATION ) )
        return;
    else
    if ( aRequest >>= aSinkException )
        return;
    else
        m_xInter->handle( xRequest );
}

//----------------------------------------------------------------
class SfxMedium_Impl : public SvCompatWeakBase
{
public:
    ::ucbhelper::Content aContent;
    sal_Bool bUpdatePickList : 1;
    sal_Bool bIsTemp        : 1;
    sal_Bool bForceSynchron : 1;
    sal_Bool bDownloadDone          : 1;
    sal_Bool bDontCallDoneLinkOnSharingError : 1;
    sal_Bool bIsStorage: 1;
    sal_Bool bUseInteractionHandler: 1;
    sal_Bool bAllowDefaultIntHdl: 1;
    sal_Bool bIsCharsetInitialized: 1;
    sal_Bool bDisposeStorage: 1;
    sal_Bool bStorageBasedOnInStream: 1;
    sal_Bool m_bSalvageMode: 1;
    sal_Bool m_bVersionsAlreadyLoaded: 1;
    sal_Bool m_bLocked: 1;
    sal_Bool m_bGotDateTime: 1;

    uno::Reference < embed::XStorage > xStorage;

    SfxMedium*       pAntiImpl;

    long             nFileVersion;

    const SfxFilter* pOrigFilter;
    String           aOrigURL;
    String           aPreRedirectionURL;
    String           aReferer;
    DateTime         aExpireTime;
    SfxFrameWeak     wLoadTargetFrame;
    SvKeyValueIteratorRef xAttributes;

    svtools::AsynchronLink  aDoneLink;
    svtools::AsynchronLink  aAvailableLink;

    uno::Sequence < util::RevisionTag > aVersions;

    ::utl::TempFile*           pTempFile;

    uno::Reference < embed::XStorage > m_xZipStorage;
    Reference < XInputStream > xInputStream;
    Reference < XStream > xStream;

    uno::Reference< io::XStream > m_xLockingStream;

    sal_uInt32                  nLastStorageError;
    ::rtl::OUString             aCharset;

    ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > xInteraction;

    sal_Bool        m_bRemoveBackup;
    ::rtl::OUString m_aBackupURL;

    // the following member is changed and makes sence only during saving
    // TODO/LATER: in future the signature state should be controlled by the medium not by the document
    //             in this case the member will hold this information
    sal_uInt16      m_nSignatureState;

    util::DateTime m_aDateTime;

    uno::Reference< logging::XSimpleLogRing > m_xLogRing;

    SfxMedium_Impl( SfxMedium* pAntiImplP );
    ~SfxMedium_Impl();
};

void SfxMedium::DataAvailable_Impl()
{
    pImp->aAvailableLink.ClearPendingCall();
    pImp->aAvailableLink.Call( NULL );
}

void SfxMedium::Cancel_Impl()
{
    SetError( ERRCODE_IO_GENERAL, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
}

//------------------------------------------------------------------
SfxMedium_Impl::SfxMedium_Impl( SfxMedium* pAntiImplP )
 :  SvCompatWeakBase( pAntiImplP ),
    bUpdatePickList(sal_True),
    bIsTemp( sal_False ),
    bForceSynchron( sal_False ),
    bDownloadDone( sal_True ),
    bDontCallDoneLinkOnSharingError( sal_False ),
    bIsStorage( sal_False ),
    bUseInteractionHandler( sal_True ),
    bAllowDefaultIntHdl( sal_False ),
    bIsCharsetInitialized( sal_False ),
    bStorageBasedOnInStream( sal_False ),
    m_bSalvageMode( sal_False ),
    m_bVersionsAlreadyLoaded( sal_False ),
    m_bLocked( sal_False ),
    m_bGotDateTime( sal_False ),
    pAntiImpl( pAntiImplP ),
    nFileVersion( 0 ),
    pOrigFilter( 0 ),
    aExpireTime( Date() + 10, Time() ),
    pTempFile( NULL ),
    nLastStorageError( 0 ),
    m_bRemoveBackup( sal_False ),
    m_nSignatureState( SIGNATURESTATE_NOSIGNATURES )
{
    aDoneLink.CreateMutex();
}

//------------------------------------------------------------------
SfxMedium_Impl::~SfxMedium_Impl()
{

    aDoneLink.ClearPendingCall();
    aAvailableLink.ClearPendingCall();

    if ( pTempFile )
        delete pTempFile;
}

//================================================================

#define IMPL_CTOR(rootVal,URLVal)           \
     eError( SVSTREAM_OK ),                 \
                                            \
     bDirect( sal_False ),                  \
     bRoot( rootVal ),                      \
     bSetFilter( sal_False ),               \
     bTriedStorage( sal_False ),            \
                                            \
     nStorOpenMode( SFX_STREAM_READWRITE ), \
     pURLObj( URLVal ),                     \
     pInStream(0),                          \
     pOutStream( 0 )

//------------------------------------------------------------------
void SfxMedium::ResetError()
{
    eError = SVSTREAM_OK;
    if( pInStream )
        pInStream->ResetError();
    if( pOutStream )
        pOutStream->ResetError();
}

//------------------------------------------------------------------
sal_uInt32 SfxMedium::GetLastStorageCreationState()
{
    return pImp->nLastStorageError;
}

//------------------------------------------------------------------
void SfxMedium::AddLog( const ::rtl::OUString& aMessage )
{
    if ( !pImp->m_xLogRing.is() )
    {
        try
        {
            ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
            if ( aContext.is() )
                pImp->m_xLogRing.set( aContext.getSingleton( "com.sun.star.logging.DocumentIOLogRing" ), UNO_QUERY_THROW );
        }
        catch( uno::Exception& )
        {}
    }

    if ( pImp->m_xLogRing.is() )
        pImp->m_xLogRing->logString( aMessage );
}

//------------------------------------------------------------------
void SfxMedium::SetError( sal_uInt32 nError, const ::rtl::OUString& aLogMessage )
{
    eError = nError;
    if ( eError != ERRCODE_NONE && aLogMessage.getLength() )
        AddLog( aLogMessage );
}

//------------------------------------------------------------------
sal_uInt32 SfxMedium::GetErrorCode() const
{
    sal_uInt32 lError=eError;
    if(!lError && pInStream)
        lError=pInStream->GetErrorCode();
    if(!lError && pOutStream)
        lError=pOutStream->GetErrorCode();
    return lError;
}

//------------------------------------------------------------------
void SfxMedium::CheckFileDate( const util::DateTime& aInitDate )
{
    GetInitFileDate( sal_True );
    if ( pImp->m_aDateTime.Seconds != aInitDate.Seconds
      || pImp->m_aDateTime.Minutes != aInitDate.Minutes
      || pImp->m_aDateTime.Hours != aInitDate.Hours
      || pImp->m_aDateTime.Day != aInitDate.Day
      || pImp->m_aDateTime.Month != aInitDate.Month
      || pImp->m_aDateTime.Year != aInitDate.Year )
    {
        uno::Reference< task::XInteractionHandler > xHandler = GetInteractionHandler();

        if ( xHandler.is() )
        {
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
                    SetError( ERRCODE_ABORT, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
                }
            }
            catch ( uno::Exception& )
            {}
        }
    }
}

//------------------------------------------------------------------
sal_Bool SfxMedium::DocNeedsFileDateCheck()
{
    return ( !IsReadOnly() && ::utl::LocalFileHelper::IsLocalFile( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) ) );
}

//------------------------------------------------------------------
util::DateTime SfxMedium::GetInitFileDate( sal_Bool bIgnoreOldValue )
{
    if ( ( bIgnoreOldValue || !pImp->m_bGotDateTime ) && aLogicName.Len() )
    {
        try
        {
            uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > xDummyEnv;
            ::ucbhelper::Content aContent( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ), xDummyEnv );

            aContent.getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "DateModified" )) ) >>= pImp->m_aDateTime;
            pImp->m_bGotDateTime = sal_True;
        }
        catch ( ::com::sun::star::uno::Exception& )
        {
        }
    }

    return pImp->m_aDateTime;
}

//------------------------------------------------------------------
Reference < XContent > SfxMedium::GetContent() const
{
    if ( !pImp->aContent.get().is() )
    {
        Reference < ::com::sun::star::ucb::XContent > xContent;
        Reference < ::com::sun::star::ucb::XCommandEnvironment > xEnv;

        SFX_ITEMSET_ARG( pSet, pItem, SfxUnoAnyItem, SID_CONTENT, sal_False);
        if ( pItem )
            pItem->GetValue() >>= xContent;

        if ( xContent.is() )
        {
            try
            {
                pImp->aContent = ::ucbhelper::Content( xContent, xEnv );
            }
            catch ( Exception& )
            {
            }
        }
        else
        {
            // TODO: OSL_FAIL("SfxMedium::GetContent()\nCreate Content? This code exists as fallback only. Please clarify, why its used.");
            String aURL;
            if ( aName.Len() )
                ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aURL );
            else if ( aLogicName.Len() )
                aURL = GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
            if ( aURL.Len() )
                ::ucbhelper::Content::create( aURL, xEnv, pImp->aContent );
        }
    }

    return pImp->aContent.get();
}

//------------------------------------------------------------------
::rtl::OUString SfxMedium::GetBaseURL( bool bForSaving )
{
    ::rtl::OUString aBaseURL;
    const SfxStringItem* pBaseURLItem = static_cast<const SfxStringItem*>( GetItemSet()->GetItem(SID_DOC_BASEURL) );
    if ( pBaseURLItem )
        aBaseURL = pBaseURLItem->GetValue();
    else if ( GetContent().is() )
    {
        try
        {
            Any aAny = pImp->aContent.getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BaseURI" )) );
            aAny >>= aBaseURL;
        }
        catch ( ::com::sun::star::uno::Exception& )
        {
        }

        if ( !aBaseURL.getLength() )
            aBaseURL = GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
    }

    if ( bForSaving )
    {
        SvtSaveOptions aOpt;
        sal_Bool bIsRemote = IsRemote();
        if( (bIsRemote && !aOpt.IsSaveRelINet()) || (!bRemote && !aOpt.IsSaveRelFSys()) )
            return ::rtl::OUString();
    }

    return aBaseURL;
}

//------------------------------------------------------------------
SvStream* SfxMedium::GetInStream()
{
    if ( pInStream )
        return pInStream;

    if ( pImp->pTempFile )
    {
        pInStream = new SvFileStream( aName, nStorOpenMode );

        eError = pInStream->GetError();

        if( !eError && (nStorOpenMode & STREAM_WRITE)
                    && ! pInStream->IsWritable() )
        {
            eError = ERRCODE_IO_ACCESSDENIED;
            delete pInStream;
            pInStream = NULL;
        }
        else
            return pInStream;
    }

    GetMedium_Impl();

    if ( GetError() )
        return NULL;

    return pInStream;
}

//------------------------------------------------------------------
void SfxMedium::CloseInStream()
{
    CloseInStream_Impl();
}

void SfxMedium::CloseInStream_Impl()
{
    // if there is a storage based on the InStream, we have to
    // close the storage, too, because otherwise the storage
    // would use an invalid ( deleted ) stream.
    if ( pInStream && pImp->xStorage.is() )
    {
        if ( pImp->bStorageBasedOnInStream )
            CloseStorage();
    }

    if ( pInStream && !GetContent().is() )
    {
        CreateTempFile( sal_True );
        return;
    }

    DELETEZ( pInStream );
    if ( pSet )
        pSet->ClearItem( SID_INPUTSTREAM );

    CloseZipStorage_Impl();
    pImp->xInputStream = uno::Reference< io::XInputStream >();

    if ( !pOutStream )
    {
        // output part of the stream is not used so the whole stream can be closed
        // TODO/LATER: is it correct?
        pImp->xStream = uno::Reference< io::XStream >();
        if ( pSet )
            pSet->ClearItem( SID_STREAM );
    }
}

//------------------------------------------------------------------
SvStream* SfxMedium::GetOutStream()
{
    if ( !pOutStream )
    {
        // Create a temp. file if there is none because we always
        // need one.
        CreateTempFile( sal_False );

        if ( pImp->pTempFile )
        {
            pOutStream = new SvFileStream( aName, STREAM_STD_READWRITE );
            CloseStorage();
        }
    }

    return pOutStream;
}

//------------------------------------------------------------------
sal_Bool SfxMedium::CloseOutStream()
{
    CloseOutStream_Impl();
    return sal_True;
}

sal_Bool SfxMedium::CloseOutStream_Impl()
{
    if ( pOutStream )
    {
        // if there is a storage based on the OutStream, we have to
        // close the storage, too, because otherwise the storage
        // would use an invalid ( deleted ) stream.
        //TODO/MBA: how to deal with this?!
        //maybe we need a new flag when the storage was created from the outstream
        if ( pImp->xStorage.is() )
        {
                CloseStorage();
        }

        delete pOutStream;
        pOutStream = NULL;
    }

    if ( !pInStream )
    {
        // input part of the stream is not used so the whole stream can be closed
        // TODO/LATER: is it correct?
        pImp->xStream = uno::Reference< io::XStream >();
        if ( pSet )
            pSet->ClearItem( SID_STREAM );
    }

    return sal_True;
}

//------------------------------------------------------------------
const String& SfxMedium::GetPhysicalName() const
{
    if ( !aName.Len() && aLogicName.Len() )
        (( SfxMedium*)this)->CreateFileStream();

    // return the name then
    return aName;
}

//------------------------------------------------------------------
void SfxMedium::CreateFileStream()
{
    ForceSynchronStream_Impl( TRUE );
    GetInStream();
    if( pInStream )
    {
        CreateTempFile( sal_False );
        pImp->bIsTemp = sal_True;
        CloseInStream_Impl();
    }
}

//------------------------------------------------------------------
sal_Bool SfxMedium::Commit()
{
    if( pImp->xStorage.is() )
        StorageCommit_Impl();
    else if( pOutStream  )
        pOutStream->Flush();
    else if( pInStream  )
        pInStream->Flush();

    if ( GetError() == SVSTREAM_OK )
    {
        // does something only in case there is a temporary file ( means aName points to different location than aLogicName )
        Transfer_Impl();
    }

    sal_Bool bResult = ( GetError() == SVSTREAM_OK );

    if ( bResult && DocNeedsFileDateCheck() )
        GetInitFileDate( sal_True );

    // remove truncation mode from the flags
    nStorOpenMode &= (~STREAM_TRUNC);
    return bResult;
}

//------------------------------------------------------------------
sal_Bool SfxMedium::IsStorage()
{
    if ( pImp->xStorage.is() )
        return TRUE;

    if ( bTriedStorage )
        return pImp->bIsStorage;

    if ( pImp->pTempFile )
    {
        String aURL;
        if ( !::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aURL ) )
        {
            OSL_FAIL("Physical name not convertable!");
        }
        pImp->bIsStorage = SotStorage::IsStorageFile( aURL ) && !SotStorage::IsOLEStorage( aURL);
        if ( !pImp->bIsStorage )
            bTriedStorage = TRUE;
    }
    else if ( GetInStream() )
    {
        pImp->bIsStorage = SotStorage::IsStorageFile( pInStream ) && !SotStorage::IsOLEStorage( pInStream );
        if ( !pInStream->GetError() && !pImp->bIsStorage )
            bTriedStorage = TRUE;
    }

    return pImp->bIsStorage;
}

//------------------------------------------------------------------
Link SfxMedium::GetDataAvailableLink() const
{
    return pImp->aAvailableLink.GetLink();
}

//------------------------------------------------------------------
Link SfxMedium::GetDoneLink() const
{
    return pImp->aDoneLink.GetLink();
}

//------------------------------------------------------------------
sal_Bool SfxMedium::IsPreview_Impl()
{
    sal_Bool bPreview = sal_False;
    SFX_ITEMSET_ARG( GetItemSet(), pPreview, SfxBoolItem, SID_PREVIEW, sal_False);
    if ( pPreview )
        bPreview = pPreview->GetValue();
    else
    {
        SFX_ITEMSET_ARG( GetItemSet(), pFlags, SfxStringItem, SID_OPTIONS, sal_False);
        if ( pFlags )
        {
            String aFileFlags = pFlags->GetValue();
            aFileFlags.ToUpperAscii();
            if ( STRING_NOTFOUND != aFileFlags.Search( 'B' ) )
                bPreview = sal_True;
        }
    }

    return bPreview;
}

//------------------------------------------------------------------
void SfxMedium::StorageBackup_Impl()
{
    ::ucbhelper::Content aOriginalContent;
    Reference< ::com::sun::star::ucb::XCommandEnvironment > xDummyEnv;

    sal_Bool bBasedOnOriginalFile = ( !pImp->pTempFile && !( aLogicName.Len() && pImp->m_bSalvageMode )
        && GetURLObject().GetMainURL( INetURLObject::NO_DECODE ).getLength()
        && ::utl::LocalFileHelper::IsLocalFile( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) )
        && ::utl::UCBContentHelper::IsDocument( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) ) );

    if ( bBasedOnOriginalFile && !pImp->m_aBackupURL.getLength()
      && ::ucbhelper::Content::create( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ), xDummyEnv, aOriginalContent ) )
    {
        DoInternalBackup_Impl( aOriginalContent );
        if( !pImp->m_aBackupURL.getLength() )
            SetError( ERRCODE_SFX_CANTCREATEBACKUP, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
    }
}

//------------------------------------------------------------------
::rtl::OUString SfxMedium::GetBackup_Impl()
{
    if ( !pImp->m_aBackupURL.getLength() )
        StorageBackup_Impl();

    return pImp->m_aBackupURL;
}

//------------------------------------------------------------------
uno::Reference < embed::XStorage > SfxMedium::GetOutputStorage()
{
    if ( GetError() )
        return uno::Reference< embed::XStorage >();

    // if the medium was constructed with a Storage: use this one, not a temp. storage
    // if a temporary storage already exists: use it
    if ( pImp->xStorage.is() && ( !aLogicName.Len() || pImp->pTempFile ) )
        return pImp->xStorage;

    // if necessary close stream that was used for reading
    if ( pInStream && !pInStream->IsWritable() )
        CloseInStream();

    DBG_ASSERT( !pOutStream, "OutStream in a readonly Medium?!" );

    // TODO/LATER: The current solution is to store the document temporary and then copy it to the target location;
    // in future it should be stored directly and then copied to the temporary location, since in this case no
    // file attributes have to be preserved and system copying mechanics could be used instead of streaming.
    CreateTempFileNoCopy();

    return GetStorage();
}

//------------------------------------------------------------------
void SfxMedium::SetPasswordToStorage_Impl()
{
    // in case media-descriptor contains password it should be used on opening
    if ( pImp->xStorage.is() && pSet )
    {
        ::rtl::OUString aPasswd;
        if ( GetPasswd_Impl( pSet, aPasswd ) )
        {
            try
            {
                ::comphelper::OStorageHelper::SetCommonStoragePassword( pImp->xStorage, aPasswd );
            }
            catch( uno::Exception& )
            {
                OSL_ENSURE( sal_False, "It must be possible to set a common password for the storage" );
                // TODO/LATER: set the error code in case of problem
                // SetError( ERRCODE_IO_GENERAL, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
            }
        }
    }
}

//------------------------------------------------------------------
sal_Int8 SfxMedium::ShowLockedDocumentDialog( const uno::Sequence< ::rtl::OUString >& aData, sal_Bool bIsLoading, sal_Bool bOwnLock )
{
    sal_Int8 nResult = LOCK_UI_NOLOCK;

    // show the interaction regarding the document opening
    uno::Reference< task::XInteractionHandler > xHandler = GetInteractionHandler();

    if ( ::svt::DocumentLockFile::IsInteractionAllowed() && xHandler.is() && ( bIsLoading || bOwnLock ) )
    {
        ::rtl::OUString aDocumentURL = GetURLObject().GetLastName();
        ::rtl::OUString aInfo;
        ::rtl::Reference< ::ucbhelper::InteractionRequest > xInteractionRequestImpl;

        if ( bOwnLock )
        {
            if ( aData.getLength() > LOCKFILE_EDITTIME_ID )
                aInfo = aData[LOCKFILE_EDITTIME_ID];

            xInteractionRequestImpl = new ::ucbhelper::InteractionRequest( uno::makeAny(
                document::OwnLockOnDocumentRequest( ::rtl::OUString(), uno::Reference< uno::XInterface >(), aDocumentURL, aInfo, !bIsLoading ) ) );
        }
        else
        {
            if ( aData.getLength() > LOCKFILE_EDITTIME_ID )
            {
                if ( aData[LOCKFILE_OOOUSERNAME_ID].getLength() )
                    aInfo = aData[LOCKFILE_OOOUSERNAME_ID];
                else
                    aInfo = aData[LOCKFILE_SYSUSERNAME_ID];

                if ( aInfo.getLength() && aData[LOCKFILE_EDITTIME_ID].getLength() )
                {
                    aInfo += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " ( " ) );
                    aInfo += aData[LOCKFILE_EDITTIME_ID];
                    aInfo += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " )" ) );
                }
            }

            if ( bIsLoading )
            {
                xInteractionRequestImpl = new ::ucbhelper::InteractionRequest( uno::makeAny(
                    document::LockedDocumentRequest( ::rtl::OUString(), uno::Reference< uno::XInterface >(), aDocumentURL, aInfo ) ) );
            }
            else
            {
                xInteractionRequestImpl = new ::ucbhelper::InteractionRequest( uno::makeAny(
                    document::LockedOnSavingRequest( ::rtl::OUString(), uno::Reference< uno::XInterface >(), aDocumentURL, aInfo ) ) );

            }
        }

        uno::Sequence< uno::Reference< task::XInteractionContinuation > > aContinuations( 3 );
        aContinuations[0] = new ::ucbhelper::InteractionAbort( xInteractionRequestImpl.get() );
        aContinuations[1] = new ::ucbhelper::InteractionApprove( xInteractionRequestImpl.get() );
        aContinuations[2] = new ::ucbhelper::InteractionDisapprove( xInteractionRequestImpl.get() );
        xInteractionRequestImpl->setContinuations( aContinuations );

        xHandler->handle( xInteractionRequestImpl.get() );

        ::rtl::Reference< ::ucbhelper::InteractionContinuation > xSelected = xInteractionRequestImpl->getSelection();
        if ( uno::Reference< task::XInteractionAbort >( xSelected.get(), uno::UNO_QUERY ).is() )
        {
            SetError( ERRCODE_ABORT, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
        }
        else if ( uno::Reference< task::XInteractionDisapprove >( xSelected.get(), uno::UNO_QUERY ).is() )
        {
            // own lock on loading, user has selected to ignore the lock
            // own lock on saving, user has selected to ignore the lock
            // alien lock on loading, user has selected to edit a copy of document
            // TODO/LATER: alien lock on saving, user has selected to do SaveAs to different location
            if ( bIsLoading && !bOwnLock )
            {
                // means that a copy of the document should be opened
                GetItemSet()->Put( SfxBoolItem( SID_TEMPLATE, sal_True ) );
            }
            else if ( bOwnLock )
                nResult = LOCK_UI_SUCCEEDED;
        }
        else // if ( XSelected == aContinuations[1] )
        {
            // own lock on loading, user has selected to open readonly
            // own lock on saving, user has selected to open readonly
            // alien lock on loading, user has selected to retry saving
            // TODO/LATER: alien lock on saving, user has selected to retry saving

            if ( bIsLoading )
                GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, sal_True ) );
            else
                nResult = LOCK_UI_TRY;
        }
    }
    else
    {
        if ( bIsLoading )
        {
            // if no interaction handler is provided the default answer is open readonly
            // that usually happens in case the document is loaded per API
            // so the document must be opened readonly for backward compatibility
            GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, sal_True ) );
        }
        else
            SetError( ERRCODE_IO_ACCESSDENIED, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );

    }

    return nResult;
}

namespace
{
    bool isSuitableProtocolForLocking(const String & rLogicName)
    {
        INetURLObject aUrl( rLogicName );
        INetProtocol eProt = aUrl.GetProtocol();
        return eProt == INET_PROT_FILE || eProt == INET_PROT_SFTP;
    }
}

//------------------------------------------------------------------
sal_Bool SfxMedium::LockOrigFileOnDemand( sal_Bool bLoading, sal_Bool bNoUI )
{
    // returns true if the document can be opened for editing ( even if it should be a copy )
    // otherwise the document should be opened readonly
    // if user cancel the loading the ERROR_ABORT is set
    sal_Bool bResult = sal_False;

    if ( !GetURLObject().HasError() ) try
    {
        if ( pImp->m_bLocked && bLoading && ::utl::LocalFileHelper::IsLocalFile( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) ) )
        {
            // if the document is already locked the system locking might be temporarely off after storing
            // check whether the system file locking should be taken again
            GetLockingStream_Impl();
        }

        bResult = pImp->m_bLocked;

        if ( !bResult )
        {
            // no read-write access is necessary on loading if the document is explicitly opened as copy
            SFX_ITEMSET_ARG( GetItemSet(), pTemplateItem, SfxBoolItem, SID_TEMPLATE, sal_False);
            bResult = ( bLoading && pTemplateItem && pTemplateItem->GetValue() );
        }

        if ( !bResult && !IsReadOnly() )
        {
            sal_Bool bContentReadonly = sal_False;
            if ( bLoading && ::utl::LocalFileHelper::IsLocalFile( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) ) )
            {
                // let the original document be opened to check the possibility to open it for editing
                // and to let the writable stream stay open to hold the lock on the document
                GetLockingStream_Impl();
            }

            // "IsReadOnly" property does not allow to detect whether the file is readonly always
            // so we try always to open the file for editing
            // the file is readonly only in case the read-write stream can not be opened
            if ( bLoading && !pImp->m_xLockingStream.is() )
            {
                try
                {
                    // MediaDescriptor does this check also, the duplication should be avoided in future
                    Reference< ::com::sun::star::ucb::XCommandEnvironment > xDummyEnv;
                    ::ucbhelper::Content aContent( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ), xDummyEnv );
                    aContent.getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsReadOnly" ) ) ) >>= bContentReadonly;
                }
                catch( uno::Exception )
                {}

#if EXTRA_ACL_CHECK
                // This block was introduced as a fix to i#102464, but removing
                // this does not make the problem re-appear.  But leaving this
                // part would interfere with documents saved in samba share.  This
                // affects Windows only.
                if ( !bContentReadonly )
                {
                    // the file is not readonly, check the ACL

                    String aPhysPath;
                    if ( ::utl::LocalFileHelper::ConvertURLToPhysicalName( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ), aPhysPath ) )
                        bContentReadonly = IsReadonlyAccordingACL( aPhysPath.GetBuffer() );
                }
#endif
            }

            // do further checks only if the file not readonly in fs
            if ( !bContentReadonly )
            {
                // the special file locking should be used only for suitable URLs
                if ( isSuitableProtocolForLocking( aLogicName ) )
                {

                    // in case of storing the document should request the output before locking
                    if ( bLoading )
                    {
                        // let the stream be opened to check the system file locking
                        GetMedium_Impl();
                    }

                    sal_Int8 bUIStatus = LOCK_UI_NOLOCK;

                    // check whether system file locking has been used, the default value is false
                    sal_Bool bUseSystemLock = ::utl::LocalFileHelper::IsLocalFile( aLogicName ) && IsSystemFileLockingUsed();

                    // TODO/LATER: This implementation does not allow to detect the system lock on saving here, actually this is no big problem
                    // if system lock is used the writeable stream should be available
                    sal_Bool bHandleSysLocked = ( bLoading && bUseSystemLock && !pImp->xStream.is() && !pOutStream );

                    do
                    {
                        try
                        {
                            ::svt::DocumentLockFile aLockFile( aLogicName );
                            if ( !bHandleSysLocked )
                            {
                                try
                                {
                                    bResult = aLockFile.CreateOwnLockFile();
                                }
                                catch ( ucb::InteractiveIOException& e )
                                {
                                    // exception means that the lock file can not be successfuly accessed
                                    // in this case it should be ignored if system file locking is anyway active
                                    if ( bUseSystemLock || !IsOOoLockFileUsed() )
                                    {
                                        bResult = sal_True;
                                        // take the ownership over the lock file
                                        aLockFile.OverwriteOwnLockFile();
                                    }
                                    else if ( e.Code == IOErrorCode_INVALID_PARAMETER )
                                    {
                                        // system file locking is not active, ask user whether he wants to open the document without any locking
                                        uno::Reference< task::XInteractionHandler > xHandler = GetInteractionHandler();

                                        if ( xHandler.is() )
                                        {
                                            ::rtl::Reference< ::ucbhelper::InteractionRequest > xIgnoreRequestImpl
                                                = new ::ucbhelper::InteractionRequest( uno::makeAny( document::LockFileIgnoreRequest() ) );

                                            uno::Sequence< uno::Reference< task::XInteractionContinuation > > aContinuations( 2 );
                                            aContinuations[0] = new ::ucbhelper::InteractionAbort( xIgnoreRequestImpl.get() );
                                            aContinuations[1] = new ::ucbhelper::InteractionApprove( xIgnoreRequestImpl.get() );
                                            xIgnoreRequestImpl->setContinuations( aContinuations );

                                            xHandler->handle( xIgnoreRequestImpl.get() );

                                            ::rtl::Reference< ::ucbhelper::InteractionContinuation > xSelected = xIgnoreRequestImpl->getSelection();
                                            bResult = (  uno::Reference< task::XInteractionApprove >( xSelected.get(), uno::UNO_QUERY ).is() );
                                        }
                                    }
                                }
                                catch ( uno::Exception& )
                                {
                                    // exception means that the lock file can not be successfuly accessed
                                    // in this case it should be ignored if system file locking is anyway active
                                    if ( bUseSystemLock || !IsOOoLockFileUsed() )
                                    {
                                        bResult = sal_True;
                                        // take the ownership over the lock file
                                        aLockFile.OverwriteOwnLockFile();
                                    }
                                }

                                // in case OOo locking is turned off the lock file is still written if possible
                                // but it is ignored while deciding whether the document should be opened for editing or not
                                if ( !bResult && !IsOOoLockFileUsed() )
                                {
                                    bResult = sal_True;
                                    // take the ownership over the lock file
                                    aLockFile.OverwriteOwnLockFile();
                                }
                            }


                            if ( !bResult )
                            {
                                uno::Sequence< ::rtl::OUString > aData;
                                try
                                {
                                    // impossibility to get data is no real problem
                                    aData = aLockFile.GetLockData();
                                }
                                catch( uno::Exception ) {}

                                sal_Bool bOwnLock = sal_False;

                                if ( !bHandleSysLocked )
                                {
                                    uno::Sequence< ::rtl::OUString > aOwnData = aLockFile.GenerateOwnEntry();
                                    bOwnLock = ( aData.getLength() > LOCKFILE_USERURL_ID
                                              && aOwnData.getLength() > LOCKFILE_USERURL_ID
                                              && aOwnData[LOCKFILE_SYSUSERNAME_ID].equals( aData[LOCKFILE_SYSUSERNAME_ID] ) );

                                    if ( bOwnLock
                                      && aOwnData[LOCKFILE_LOCALHOST_ID].equals( aData[LOCKFILE_LOCALHOST_ID] )
                                      && aOwnData[LOCKFILE_USERURL_ID].equals( aData[LOCKFILE_USERURL_ID] ) )
                                    {
                                        // this is own lock from the same installation, it could remain because of crash
                                        bResult = sal_True;
                                    }
                                }

                                if ( !bResult && !bNoUI )
                                {
                                    bUIStatus = ShowLockedDocumentDialog( aData, bLoading, bOwnLock );
                                    if ( bUIStatus == LOCK_UI_SUCCEEDED )
                                    {
                                        // take the ownership over the lock file
                                        bResult = aLockFile.OverwriteOwnLockFile();
                                    }
                                }

                                bHandleSysLocked = sal_False;
                            }
                        }
                        catch( uno::Exception& )
                        {
                        }
                    } while( !bResult && bUIStatus == LOCK_UI_TRY );

                    pImp->m_bLocked = bResult;
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

            SFX_ITEMSET_ARG( pSet, pReadOnlyItem, SfxBoolItem, SID_DOC_READONLY, FALSE );
            if ( !bLoading || (pReadOnlyItem && !pReadOnlyItem->GetValue()) )
                SetError( ERRCODE_IO_ACCESSDENIED, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
            else
                GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, sal_True ) );
        }

        // when the file is locked, get the current file date
        if ( bResult && DocNeedsFileDateCheck() )
            GetInitFileDate( sal_True );
    }
    catch( uno::Exception& )
    {
        OSL_ENSURE( sal_False, "Unexpected problem by locking, high probability, that the content could not be created" );
    }
    return bResult;
}

//------------------------------------------------------------------
uno::Reference < embed::XStorage > SfxMedium::GetStorage( sal_Bool bCreateTempIfNo )
{
    if ( pImp->xStorage.is() || bTriedStorage )
        return pImp->xStorage;

    uno::Sequence< uno::Any > aArgs( 2 );

    // the medium should be retrieved before temporary file creation
    // to let the MediaDescriptor be filled with the streams
    GetMedium_Impl();

    if ( bCreateTempIfNo )
        CreateTempFile( sal_False );

    GetMedium_Impl();

    if ( GetError() )
        return pImp->xStorage;

    SFX_ITEMSET_ARG( GetItemSet(), pRepairItem, SfxBoolItem, SID_REPAIRPACKAGE, sal_False);
    if ( pRepairItem && pRepairItem->GetValue() )
    {
        // the storage should be created for repairing mode
        CreateTempFile( sal_False );
        GetMedium_Impl();

        Reference< ::com::sun::star::ucb::XProgressHandler > xProgressHandler;
        Reference< ::com::sun::star::task::XStatusIndicator > xStatusIndicator;

        SFX_ITEMSET_ARG( GetItemSet(), pxProgressItem, SfxUnoAnyItem, SID_PROGRESS_STATUSBAR_CONTROL, sal_False );
        if( pxProgressItem && ( pxProgressItem->GetValue() >>= xStatusIndicator ) )
            xProgressHandler = Reference< ::com::sun::star::ucb::XProgressHandler >(
                                    new utl::ProgressHandlerWrap( xStatusIndicator ) );

        uno::Sequence< beans::PropertyValue > aAddProps( 2 );
        aAddProps[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RepairPackage"));
        aAddProps[0].Value <<= (sal_Bool)sal_True;
        aAddProps[1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StatusIndicator"));
        aAddProps[1].Value <<= xProgressHandler;

        // the first arguments will be filled later
        aArgs.realloc( 3 );
        aArgs[2] <<= aAddProps;
    }

    if ( pImp->xStream.is() )
    {
        // since the storage is based on temporary stream we open it always read-write
        aArgs[0] <<= pImp->xStream;
        aArgs[1] <<= embed::ElementModes::READWRITE;
        pImp->bStorageBasedOnInStream = sal_True;
    }
    else if ( pImp->xInputStream.is() )
    {
        // since the storage is based on temporary stream we open it always read-write
        aArgs[0] <<= pImp->xInputStream;
        aArgs[1] <<= embed::ElementModes::READ;
        pImp->bStorageBasedOnInStream = sal_True;
    }
    else
    {
        CloseStreams_Impl();
        aArgs[0] <<= ::rtl::OUString( aName );
        aArgs[1] <<= embed::ElementModes::READ;
        pImp->bStorageBasedOnInStream = sal_False;
    }

    try
    {
        pImp->xStorage = uno::Reference< embed::XStorage >(
                            ::comphelper::OStorageHelper::GetStorageFactory()->createInstanceWithArguments( aArgs ),
                            uno::UNO_QUERY );
    }
    catch( uno::Exception& )
    {
        // impossibility to create the storage is no error
    }

    if( ( pImp->nLastStorageError = GetError() ) != SVSTREAM_OK )
    {
        pImp->xStorage = 0;
        if ( pInStream )
            pInStream->Seek(0);
        return uno::Reference< embed::XStorage >();
    }

    bTriedStorage = sal_True;

    // TODO/LATER: Get versionlist on demand
    if ( pImp->xStorage.is() )
    {
        SetPasswordToStorage_Impl();
        GetVersionList();
    }

    SFX_ITEMSET_ARG( pSet, pVersion, SfxInt16Item, SID_VERSION, sal_False);

    BOOL bResetStorage = FALSE;
    if ( pVersion && pVersion->GetValue() )
    {
        // Alle verf"ugbaren Versionen einlesen
        if ( pImp->aVersions.getLength() )
        {
            // Die zum Kommentar passende Version suchen
            // Die Versionen sind von 1 an durchnumeriert, mit negativen
            // Versionsnummern werden die Versionen von der aktuellen aus
            // r"uckw"arts gez"ahlt
            short nVersion = pVersion ? pVersion->GetValue() : 0;
            if ( nVersion<0 )
                nVersion = ( (short) pImp->aVersions.getLength() ) + nVersion;
            else if ( nVersion )
                nVersion--;

            util::RevisionTag& rTag = pImp->aVersions[nVersion];
            {
                // SubStorage f"ur alle Versionen "offnen
                uno::Reference < embed::XStorage > xSub = pImp->xStorage->openStorageElement( DEFINE_CONST_UNICODE( "Versions" ),
                        embed::ElementModes::READ );

                DBG_ASSERT( xSub.is(), "Versionsliste, aber keine Versionen!" );

                // Dort ist die Version als gepackter Stream gespeichert
                uno::Reference < io::XStream > xStr = xSub->openStreamElement( rTag.Identifier, embed::ElementModes::READ );
                SvStream* pStream = utl::UcbStreamHelper::CreateStream( xStr );
                if ( pStream && pStream->GetError() == SVSTREAM_OK )
                {
                    // Stream ins TempDir auspacken
                    ::utl::TempFile aTempFile;
                    String          aTmpName = aTempFile.GetURL();
                    SvFileStream    aTmpStream( aTmpName, SFX_STREAM_READWRITE );

                    *pStream >> aTmpStream;
                    aTmpStream.Close();

                    // Datei als Storage "offnen
                    nStorOpenMode = SFX_STREAM_READONLY;
                    pImp->xStorage = comphelper::OStorageHelper::GetStorageFromURL( aTmpName, embed::ElementModes::READ );
                    pImp->bStorageBasedOnInStream = sal_False;
                    String aTemp;
                    ::utl::LocalFileHelper::ConvertURLToPhysicalName( aTmpName, aTemp );
                    SetPhysicalName_Impl( aTemp );

                    pImp->bIsTemp = sal_True;
                    GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, sal_True ) );
                    // TODO/MBA
                    pImp->aVersions.realloc(0);
                }
                else
                    bResetStorage = TRUE;
            }
        }
        else
            bResetStorage = TRUE;
    }

    if ( bResetStorage )
    {
        pImp->xStorage = 0;
        if ( pInStream )
            pInStream->Seek( 0L );
    }

    pImp->bIsStorage = pImp->xStorage.is();
    return pImp->xStorage;
}

//------------------------------------------------------------------
uno::Reference< embed::XStorage > SfxMedium::GetZipStorageToSign_Impl( sal_Bool bReadOnly )
{
    if ( !GetError() && !pImp->m_xZipStorage.is() )
    {
        GetMedium_Impl();

        try
        {
            // we can not sign document if there is no stream
            // should it be possible at all?
            if ( !bReadOnly && pImp->xStream.is() )
            {
                pImp->m_xZipStorage = ::comphelper::OStorageHelper::GetStorageOfFormatFromStream( ZIP_STORAGE_FORMAT_STRING, pImp->xStream, embed::ElementModes::READWRITE );
            }
            else if ( pImp->xInputStream.is() )
            {
                pImp->m_xZipStorage = ::comphelper::OStorageHelper::GetStorageOfFormatFromInputStream( ZIP_STORAGE_FORMAT_STRING, pImp->xInputStream );
            }
        }
        catch( uno::Exception& )
        {
            OSL_ENSURE( sal_False, "No possibility to get readonly version of storage from medium!\n" );
        }

        if ( GetError() ) // do not remove warnings
            ResetError();
    }

    return pImp->m_xZipStorage;
}

//------------------------------------------------------------------
void SfxMedium::CloseZipStorage_Impl()
{
    if ( pImp->m_xZipStorage.is() )
    {
        try {
            pImp->m_xZipStorage->dispose();
        } catch( uno::Exception& )
        {}

        pImp->m_xZipStorage = uno::Reference< embed::XStorage >();
    }
}

//------------------------------------------------------------------
void SfxMedium::CloseStorage()
{
    if ( pImp->xStorage.is() )
    {
        uno::Reference < lang::XComponent > xComp( pImp->xStorage, uno::UNO_QUERY );
        // in the salvage mode the medium does not own the storage
        if ( pImp->bDisposeStorage && !pImp->m_bSalvageMode )
        {
            try {
                xComp->dispose();
            } catch( uno::Exception& )
            {
                OSL_ENSURE( sal_False, "Medium's storage is already disposed!\n" );
            }
        }

        pImp->xStorage = 0;
        pImp->bStorageBasedOnInStream = sal_False;
    }

    bTriedStorage = sal_False;
    pImp->bIsStorage = sal_False;
}

void SfxMedium::CanDisposeStorage_Impl( sal_Bool bDisposeStorage )
{
    pImp->bDisposeStorage = bDisposeStorage;
}

sal_Bool SfxMedium::WillDisposeStorageOnClose_Impl()
{
    return pImp->bDisposeStorage;
}

//------------------------------------------------------------------
void SfxMedium::SetOpenMode( StreamMode nStorOpen,
                             sal_Bool bDirectP,
                             sal_Bool bDontClose )
{
    if ( nStorOpenMode != nStorOpen )
    {
        nStorOpenMode = nStorOpen;

        if( !bDontClose )
        {
            if ( pImp->xStorage.is() )
                CloseStorage();

            CloseStreams_Impl();
        }
    }

    bDirect     = bDirectP;
    bSetFilter  = sal_False;
}

//------------------------------------------------------------------
sal_Bool SfxMedium::UseBackupToRestore_Impl( ::ucbhelper::Content& aOriginalContent,
                                            const Reference< ::com::sun::star::ucb::XCommandEnvironment >& xComEnv )
{
    try
    {
        ::ucbhelper::Content aTransactCont( pImp->m_aBackupURL, xComEnv );

        Reference< XInputStream > aOrigInput = aTransactCont.openStream();
        aOriginalContent.writeStream( aOrigInput, sal_True );
        return sal_True;
    }
    catch( Exception& )
    {
        // in case of failure here the backup file should not be removed
        // TODO/LATER: a message should be used to let user know about the backup
        pImp->m_bRemoveBackup = sal_False;
        // TODO/LATER: needs a specific error code
        eError = ERRCODE_IO_GENERAL;
    }

    return sal_False;
}

//------------------------------------------------------------------
sal_Bool SfxMedium::StorageCommit_Impl()
{
    sal_Bool bResult = sal_False;
    Reference< ::com::sun::star::ucb::XCommandEnvironment > xDummyEnv;
    ::ucbhelper::Content aOriginalContent;

    if ( pImp->xStorage.is() )
    {
        if ( !GetError() )
        {
            uno::Reference < embed::XTransactedObject > xTrans( pImp->xStorage, uno::UNO_QUERY );
            if ( xTrans.is() )
            {
                try
                {
                    xTrans->commit();
                    CloseZipStorage_Impl();
                    bResult = sal_True;
                }
                catch ( embed::UseBackupException& aBackupExc )
                {
                    // since the temporary file is created always now, the scenario is close to be impossible
                    if ( !pImp->pTempFile )
                    {
                        OSL_ENSURE( pImp->m_aBackupURL.getLength(), "No backup on storage commit!\n" );
                        if ( pImp->m_aBackupURL.getLength()
                            && ::ucbhelper::Content::create( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ),
                                                        xDummyEnv,
                                                        aOriginalContent ) )
                        {
                            // use backup to restore the file
                            // the storage has already disconnected from original location
                            CloseAndReleaseStreams_Impl();
                            if ( !UseBackupToRestore_Impl( aOriginalContent, xDummyEnv ) )
                            {
                                // connect the medium to the temporary file of the storage
                                pImp->aContent = ::ucbhelper::Content();
                                aName = aBackupExc.TemporaryFileURL;
                                OSL_ENSURE( aName.Len(), "The exception _must_ contain the temporary URL!\n" );
                            }
                        }

                        if ( !GetError() )
                            SetError( ERRCODE_IO_GENERAL, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
                    }
                }
                catch ( uno::Exception& )
                {
                    //TODO/LATER: improve error handling
                    SetError( ERRCODE_IO_GENERAL, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
                }
            }
        }
    }

    return bResult;
}

//------------------------------------------------------------------
sal_Bool SfxMedium::TransactedTransferForFS_Impl( const INetURLObject& aSource,
                                                 const INetURLObject& aDest,
                                                 const Reference< ::com::sun::star::ucb::XCommandEnvironment >& xComEnv )
{
    sal_Bool bResult = sal_False;
    Reference< ::com::sun::star::ucb::XCommandEnvironment > xDummyEnv;
    Reference< XOutputStream > aDestStream;
    ::ucbhelper::Content aOriginalContent;

    try
    {
        aOriginalContent = ::ucbhelper::Content( aDest.GetMainURL( INetURLObject::NO_DECODE ), xComEnv );
    }
    catch ( ::com::sun::star::ucb::CommandAbortedException& )
    {
        eError = ERRCODE_ABORT;
    }
    catch ( ::com::sun::star::ucb::CommandFailedException& )
    {
        eError = ERRCODE_ABORT;
    }
    catch (const ::com::sun::star::ucb::ContentCreationException& ex)
    {
        eError = ERRCODE_IO_GENERAL;
        if (
            (ex.eError == ::com::sun::star::ucb::ContentCreationError_NO_CONTENT_PROVIDER    ) ||
            (ex.eError == ::com::sun::star::ucb::ContentCreationError_CONTENT_CREATION_FAILED)
           )
        {
            eError = ERRCODE_IO_NOTEXISTSPATH;
        }
    }
    catch (const ::com::sun::star::uno::Exception&)
    {
       eError = ERRCODE_IO_GENERAL;
    }

    if( !eError || (eError & ERRCODE_WARNING_MASK) )
    {
        if ( pImp->xStorage.is() )
            CloseStorage();

        CloseStreams_Impl();

        ::ucbhelper::Content aTempCont;
        if( ::ucbhelper::Content::create( aSource.GetMainURL( INetURLObject::NO_DECODE ), xDummyEnv, aTempCont ) )
        {
            sal_Bool bTransactStarted = sal_False;
            SFX_ITEMSET_ARG( GetItemSet(), pOverWrite, SfxBoolItem, SID_OVERWRITE, sal_False );
               SFX_ITEMSET_ARG( GetItemSet(), pRename, SfxBoolItem, SID_RENAME, sal_False );
            sal_Bool bRename = pRename ? pRename->GetValue() : FALSE;
            sal_Bool bOverWrite = pOverWrite ? pOverWrite->GetValue() : !bRename;

            try
            {
                if( bOverWrite && ::utl::UCBContentHelper::IsDocument( aDest.GetMainURL( INetURLObject::NO_DECODE ) ) )
                {
                    if( ! pImp->m_aBackupURL.getLength() )
                        DoInternalBackup_Impl( aOriginalContent );

                    if( pImp->m_aBackupURL.getLength() )
                    {
                        Reference< XInputStream > aTempInput = aTempCont.openStream();
                        bTransactStarted = sal_True;
                        aOriginalContent.setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Size")),
                                                            uno::makeAny( (sal_Int64)0 ) );
                        aOriginalContent.writeStream( aTempInput, bOverWrite );
                        bResult = sal_True;
                    }
                    else
                    {
                        eError = ERRCODE_SFX_CANTCREATEBACKUP;
                    }
                }
                else
                {
                    Reference< XInputStream > aTempInput = aTempCont.openStream();
                    aOriginalContent.writeStream( aTempInput, bOverWrite );
                    bResult = sal_True;
                }
            }
            catch ( ::com::sun::star::ucb::CommandAbortedException& )
            {
                eError = ERRCODE_ABORT;
            }
            catch ( ::com::sun::star::ucb::CommandFailedException& )
            {
                eError = ERRCODE_ABORT;
            }
            catch ( ::com::sun::star::ucb::InteractiveIOException& r )
            {
                if ( r.Code == IOErrorCode_ACCESS_DENIED )
                    eError = ERRCODE_IO_ACCESSDENIED;
                else if ( r.Code == IOErrorCode_NOT_EXISTING )
                    eError = ERRCODE_IO_NOTEXISTS;
                else if ( r.Code == IOErrorCode_CANT_READ )
                    eError = ERRCODE_IO_CANTREAD;
                else
                    eError = ERRCODE_IO_GENERAL;
            }
            catch ( ::com::sun::star::uno::Exception& )
            {
                eError = ERRCODE_IO_GENERAL;
            }

               if ( bResult )
               {
                if ( pImp->pTempFile )
                {
                    pImp->pTempFile->EnableKillingFile( sal_True );
                       delete pImp->pTempFile;
                       pImp->pTempFile = NULL;
                }
               }
            else if ( bTransactStarted )
            {
                UseBackupToRestore_Impl( aOriginalContent, xDummyEnv );
            }
        }
        else
            eError = ERRCODE_IO_CANTREAD;
    }

    return bResult;
}

//------------------------------------------------------------------
sal_Bool SfxMedium::TryDirectTransfer( const ::rtl::OUString& aURL, SfxItemSet& aTargetSet )
{
    if ( GetError() )
        return sal_False;

    // if the document had no password it should be stored without password
    // if the document had password it should be stored with the same password
    // otherwise the stream copying can not be done
    SFX_ITEMSET_ARG( &aTargetSet, pNewPassItem, SfxStringItem, SID_PASSWORD, sal_False );
    SFX_ITEMSET_ARG( GetItemSet(), pOldPassItem, SfxStringItem, SID_PASSWORD, sal_False );
    if ( ( !pNewPassItem && !pOldPassItem )
      || ( pNewPassItem && pOldPassItem && pNewPassItem->GetValue().Equals( pOldPassItem->GetValue() ) ) )
    {
        // the filter must be the same
        SFX_ITEMSET_ARG( &aTargetSet, pNewFilterItem, SfxStringItem, SID_FILTER_NAME, sal_False );
        SFX_ITEMSET_ARG( GetItemSet(), pOldFilterItem, SfxStringItem, SID_FILTER_NAME, sal_False );
        if ( pNewFilterItem && pOldFilterItem && pNewFilterItem->GetValue().Equals( pOldFilterItem->GetValue() ) )
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

                    uno::Reference < ::com::sun::star::ucb::XCommandEnvironment > xEnv;
                    ::ucbhelper::Content aTargetContent( aURL, xEnv );

                    InsertCommandArgument aInsertArg;
                    aInsertArg.Data = xInStream;
                       SFX_ITEMSET_ARG( &aTargetSet, pRename, SfxBoolItem, SID_RENAME, sal_False );
                       SFX_ITEMSET_ARG( &aTargetSet, pOverWrite, SfxBoolItem, SID_OVERWRITE, sal_False );
                       if ( (pOverWrite && !pOverWrite->GetValue()) // argument says: never overwrite
                         || (pRename && pRename->GetValue()) ) // argument says: rename file
                        aInsertArg.ReplaceExisting = sal_False;
                       else
                        aInsertArg.ReplaceExisting = sal_True; // default is overwrite existing files

                    Any aCmdArg;
                    aCmdArg <<= aInsertArg;
                    aTargetContent.executeCommand( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ),
                                                    aCmdArg );

                    if ( xSeek.is() )
                        xSeek->seek( nPos );

                    return sal_True;
                }
                catch( uno::Exception& )
                {}
            }
        }
    }

    return sal_False;
}

//------------------------------------------------------------------
void SfxMedium::Transfer_Impl()
{
    // The transfer is required only in two cases: either if there is a temporary file or if there is a salvage item
    String aNameURL;
    if ( pImp->pTempFile )
        aNameURL = pImp->pTempFile->GetURL();
    else if ( aLogicName.Len() && pImp->m_bSalvageMode )
    {
        // makes sence only in case logic name is set
        if ( !::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aNameURL ) )
            OSL_ENSURE( sal_False, "The medium name is not convertable!\n" );
    }

    if ( aNameURL.Len() && ( !eError || (eError & ERRCODE_WARNING_MASK) ) )
    {
        RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxMedium::Transfer_Impl, copying to target" );

        Reference < ::com::sun::star::ucb::XCommandEnvironment > xEnv;
        Reference< XOutputStream > rOutStream;

        // in case an output stream is provided from outside and the URL is correct
        // commit to the stream
        if( aLogicName.CompareToAscii( "private:stream", 14 ) == COMPARE_EQUAL )
        {
            // TODO/LATER: support storing to SID_STREAM
               SFX_ITEMSET_ARG( pSet, pOutStreamItem, SfxUnoAnyItem, SID_OUTPUTSTREAM, sal_False);
             if( pOutStreamItem && ( pOutStreamItem->GetValue() >>= rOutStream ) )
            {
                if ( pImp->xStorage.is() )
                    CloseStorage();

                CloseStreams_Impl();

                INetURLObject aSource( aNameURL );
                ::ucbhelper::Content aTempCont;
                if( ::ucbhelper::Content::create( aSource.GetMainURL( INetURLObject::NO_DECODE ), xEnv, aTempCont ) )
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
                        if ( pImp->pTempFile )
                        {
                            pImp->pTempFile->EnableKillingFile( sal_True );
                            delete pImp->pTempFile;
                            pImp->pTempFile = NULL;
                        }
                    }
                    catch( Exception& )
                    {}
                }
               }
            else
            {
                OSL_FAIL( "Illegal Output stream parameter!\n" );
                SetError( ERRCODE_IO_GENERAL, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
            }

            // free the reference
            if ( pSet )
                pSet->ClearItem( SID_OUTPUTSTREAM );

            return;
        }

        GetContent();
        if ( !pImp->aContent.get().is() )
        {
            eError = ERRCODE_IO_NOTEXISTS;
            return;
        }

        SFX_ITEMSET_ARG( GetItemSet(), pSegmentSize, SfxInt32Item, SID_SEGMENTSIZE, sal_False);
        if ( pSegmentSize )
        {
            // this file must be stored into a disk spanned package
            try
            {
                uno::Reference < embed::XStorage > xStor = comphelper::OStorageHelper::GetStorageFromURL( GetName(),
                        embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );

                // set segment size property; package will automatically be divided in pieces fitting
                // into this size
                ::com::sun::star::uno::Any aAny;
                aAny <<= pSegmentSize->GetValue();

                uno::Reference < beans::XPropertySet > xSet( pImp->xStorage, uno::UNO_QUERY );
                xSet->setPropertyValue( String::CreateFromAscii("SegmentSize"), aAny );

                // copy the temporary storage into the disk spanned package
                GetStorage()->copyToStorage( xStor );
                uno::Reference < embed::XTransactedObject > xTrans( pImp->xStorage, uno::UNO_QUERY );
                if ( xTrans.is() )
                    xTrans->commit();

            }
            catch ( uno::Exception& )
            {
                //TODO/MBA: error handling
            }
            return;
        }

        INetURLObject aDest( GetURLObject() );

        // source is the temp file written so far
        INetURLObject aSource( aNameURL );

        // a special case, an interaction handler should be used for
        // authentication in case it is available
        Reference< ::com::sun::star::ucb::XCommandEnvironment > xComEnv;
           Reference< ::com::sun::star::task::XInteractionHandler > xInteractionHandler = GetInteractionHandler();
        if (xInteractionHandler.is())
            xComEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler,
                                                      Reference< ::com::sun::star::ucb::XProgressHandler >() );

        if ( ::utl::LocalFileHelper::IsLocalFile( aDest.GetMainURL( INetURLObject::NO_DECODE ) ) || !aDest.removeSegment() )
        {
            TransactedTransferForFS_Impl( aSource, aDest, xComEnv );
        }
        else
        {
            // create content for the parent folder and call transfer on that content with the source content
            // and the destination file name as parameters
            ::ucbhelper::Content aSourceContent;
            ::ucbhelper::Content aTransferContent;

            String aFileName = GetLongName();
            if ( !aFileName.Len() )
                aFileName = GetURLObject().getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );

            try
            {
                aTransferContent = ::ucbhelper::Content( aDest.GetMainURL( INetURLObject::NO_DECODE ), xComEnv );
            }
            catch (const ::com::sun::star::ucb::ContentCreationException& ex)
            {
                eError = ERRCODE_IO_GENERAL;
                if (
                    (ex.eError == ::com::sun::star::ucb::ContentCreationError_NO_CONTENT_PROVIDER    ) ||
                    (ex.eError == ::com::sun::star::ucb::ContentCreationError_CONTENT_CREATION_FAILED)
                   )
                {
                    eError = ERRCODE_IO_NOTEXISTSPATH;
                }
            }
            catch (const ::com::sun::star::uno::Exception&)
            {
                eError = ERRCODE_IO_GENERAL;
            }

            if ( !eError || (eError & ERRCODE_WARNING_MASK) )
            {
                // free resources, otherwise the transfer may fail
                if ( pImp->xStorage.is() )
                    CloseStorage();

                CloseStreams_Impl();

                ::ucbhelper::Content::create( aSource.GetMainURL( INetURLObject::NO_DECODE ), xEnv, aSourceContent );

                // check for external parameters that may customize the handling of NameClash situations
                SFX_ITEMSET_ARG( GetItemSet(), pRename, SfxBoolItem, SID_RENAME, sal_False );
                SFX_ITEMSET_ARG( GetItemSet(), pOverWrite, SfxBoolItem, SID_OVERWRITE, sal_False );
                sal_Int32 nNameClash;
                if ( pOverWrite && !pOverWrite->GetValue() )
                    // argument says: never overwrite
                    nNameClash = NameClash::ERROR;
                else if ( pRename && pRename->GetValue() )
                    // argument says: rename file
                    nNameClash = NameClash::RENAME;
                else
                    // default is overwrite existing files
                    nNameClash = NameClash::OVERWRITE;

                try
                {
                    if (!aTransferContent.transferContent( aSourceContent, ::ucbhelper::InsertOperation_COPY, aFileName, nNameClash ))
                        eError = ERRCODE_IO_GENERAL;
                }
                catch ( ::com::sun::star::ucb::CommandAbortedException& )
                {
                    eError = ERRCODE_ABORT;
                }
                catch ( ::com::sun::star::ucb::CommandFailedException& )
                {
                    eError = ERRCODE_ABORT;
                }
                catch ( ::com::sun::star::ucb::InteractiveIOException& r )
                {
                    if ( r.Code == IOErrorCode_ACCESS_DENIED )
                        eError = ERRCODE_IO_ACCESSDENIED;
                    else if ( r.Code == IOErrorCode_NOT_EXISTING )
                        eError = ERRCODE_IO_NOTEXISTS;
                    else if ( r.Code == IOErrorCode_CANT_READ )
                        eError = ERRCODE_IO_CANTREAD;
                    else
                        eError = ERRCODE_IO_GENERAL;
                }
                catch ( ::com::sun::star::uno::Exception& )
                {
                    eError = ERRCODE_IO_GENERAL;
                }

                // do not switch from temporary file in case of nonfile protocol
            }
        }

        if ( ( !eError || (eError & ERRCODE_WARNING_MASK) ) && !pImp->pTempFile )
        {
            // without a TempFile the physical and logical name should be the same after successful transfer
              ::utl::LocalFileHelper::ConvertURLToPhysicalName( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ),
                                                              aName );
            pImp->m_bSalvageMode = sal_False;
        }
    }
}

//------------------------------------------------------------------
void SfxMedium::DoInternalBackup_Impl( const ::ucbhelper::Content& aOriginalContent,
                                       const String& aPrefix,
                                       const String& aExtension,
                                       const String& aDestDir )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxMedium::DoInternalBackup_Impl( with destdir )" );

    if ( pImp->m_aBackupURL.getLength() )
        return; // the backup was done already

    ::utl::TempFile aTransactTemp( aPrefix, &aExtension, &aDestDir );
    aTransactTemp.EnableKillingFile( sal_False );

    INetURLObject aBackObj( aTransactTemp.GetURL() );
    ::rtl::OUString aBackupName = aBackObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );

    Reference < ::com::sun::star::ucb::XCommandEnvironment > xDummyEnv;
    ::ucbhelper::Content aBackupCont;
    if( ::ucbhelper::Content::create( aDestDir, xDummyEnv, aBackupCont ) )
    {
        try
        {
            if( aBackupCont.transferContent( aOriginalContent,
                                            ::ucbhelper::InsertOperation_COPY,
                                            aBackupName,
                                            NameClash::OVERWRITE ) )
            {
                pImp->m_aBackupURL = aBackObj.GetMainURL( INetURLObject::NO_DECODE );
                pImp->m_bRemoveBackup = sal_True;
            }
        }
        catch( Exception& )
        {}
    }

    if ( !pImp->m_aBackupURL.getLength() )
        aTransactTemp.EnableKillingFile( sal_True );
}

//------------------------------------------------------------------
void SfxMedium::DoInternalBackup_Impl( const ::ucbhelper::Content& aOriginalContent )
{
    if ( pImp->m_aBackupURL.getLength() )
        return; // the backup was done already

    ::rtl::OUString aFileName =  GetURLObject().getName( INetURLObject::LAST_SEGMENT,
                                                        true,
                                                        INetURLObject::NO_DECODE );

    sal_Int32 nPrefixLen = aFileName.lastIndexOf( '.' );
    String aPrefix = ( nPrefixLen == -1 ) ? aFileName : aFileName.copy( 0, nPrefixLen );
    String aExtension = ( nPrefixLen == -1 ) ? String() : String(aFileName.copy( nPrefixLen ));
       String aBakDir = SvtPathOptions().GetBackupPath();

    DoInternalBackup_Impl( aOriginalContent, aPrefix, aExtension, aBakDir );

    if ( !pImp->m_aBackupURL.getLength() )
    {
        // the copiing to the backup catalog failed ( for example because
        // of using an encrypted partition as target catalog )
        // since the user did not specify to make backup explicitly
        // office should try to make backup in another place,
        // target catalog does not look bad for this case ( and looks
        // to be the only way for encrypted partitions )

        INetURLObject aDest = GetURLObject();
        if ( aDest.removeSegment() )
            DoInternalBackup_Impl( aOriginalContent, aPrefix, aExtension, aDest.GetMainURL( INetURLObject::NO_DECODE ) );
    }
}


//------------------------------------------------------------------
void SfxMedium::DoBackup_Impl()
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxMedium::DoBackup_Impl" );

       // source file name is the logical name of this medium
    INetURLObject aSource( GetURLObject() );

    // there is nothing to backup in case source file does not exist
    if ( !::utl::UCBContentHelper::IsDocument( aSource.GetMainURL( INetURLObject::NO_DECODE ) ) )
        return;

    sal_Bool        bSuccess = sal_False;

    // get path for backups
    String aBakDir = SvtPathOptions().GetBackupPath();
    if( aBakDir.Len() )
    {
        // create content for the parent folder ( = backup folder )
        ::ucbhelper::Content  aContent;
        Reference < ::com::sun::star::ucb::XCommandEnvironment > xEnv;
        if( ::ucbhelper::Content::create( aBakDir, xEnv, aContent ) )
        {
            // save as ".bak" file
            INetURLObject aDest( aBakDir );
            aDest.insertName( aSource.getName() );
            aDest.setExtension( DEFINE_CONST_UNICODE( "bak" ) );
            String aFileName = aDest.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );

            // create a content for the source file
            ::ucbhelper::Content aSourceContent;
            if ( ::ucbhelper::Content::create( aSource.GetMainURL( INetURLObject::NO_DECODE ), xEnv, aSourceContent ) )
            {
                try
                {
                    // do the transfer ( copy source file to backup dir )
                    bSuccess = aContent.transferContent( aSourceContent,
                                                        ::ucbhelper::InsertOperation_COPY,
                                                        aFileName,
                                                        NameClash::OVERWRITE );
                    if( bSuccess )
                    {
                        pImp->m_aBackupURL = aDest.GetMainURL( INetURLObject::NO_DECODE );
                        pImp->m_bRemoveBackup = sal_False;
                    }
                }
                catch ( ::com::sun::star::uno::Exception& )
                {
                }
            }
        }
    }

    if ( !bSuccess )
    {
        eError = ERRCODE_SFX_CANTCREATEBACKUP;
    }
}

//------------------------------------------------------------------
void SfxMedium::ClearBackup_Impl()
{
    if( pImp->m_bRemoveBackup )
    {
        // currently a document is always stored in a new medium,
        // thus if a backup can not be removed the backup URL should not be cleaned
        if ( pImp->m_aBackupURL.getLength() )
        {
            if ( ::utl::UCBContentHelper::Kill( pImp->m_aBackupURL ) )
            {
                pImp->m_bRemoveBackup = sal_False;
                pImp->m_aBackupURL = ::rtl::OUString();
            }
            else
            {

                OSL_FAIL("Couldn't remove backup file!");
            }
        }
    }
    else
        pImp->m_aBackupURL = ::rtl::OUString();
}

//----------------------------------------------------------------
void SfxMedium::GetLockingStream_Impl()
{
    if ( ::utl::LocalFileHelper::IsLocalFile( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) )
      && !pImp->m_xLockingStream.is() )
    {
        SFX_ITEMSET_ARG( pSet, pWriteStreamItem, SfxUnoAnyItem, SID_STREAM, sal_False);
        if ( pWriteStreamItem )
            pWriteStreamItem->GetValue() >>= pImp->m_xLockingStream;

        if ( !pImp->m_xLockingStream.is() )
        {
            // open the original document
            uno::Sequence< beans::PropertyValue > xProps;
            TransformItems( SID_OPENDOC, *GetItemSet(), xProps );
            comphelper::MediaDescriptor aMedium( xProps );

            aMedium.addInputStreamOwnLock();

            uno::Reference< io::XInputStream > xInputStream;
            aMedium[comphelper::MediaDescriptor::PROP_STREAM()] >>= pImp->m_xLockingStream;
            aMedium[comphelper::MediaDescriptor::PROP_INPUTSTREAM()] >>= xInputStream;

            if ( !pImp->pTempFile && !aName.Len() )
            {
                // the medium is still based on the original file, it makes sence to initialize the streams
                if ( pImp->m_xLockingStream.is() )
                    pImp->xStream = pImp->m_xLockingStream;

                if ( xInputStream.is() )
                    pImp->xInputStream = xInputStream;

                if ( !pImp->xInputStream.is() && pImp->xStream.is() )
                    pImp->xInputStream = pImp->xStream->getInputStream();
            }
        }
    }
}

//----------------------------------------------------------------
void SfxMedium::GetMedium_Impl()
{
    if ( !pInStream )
    {
        pImp->bDownloadDone = sal_False;
        Reference< ::com::sun::star::task::XInteractionHandler > xInteractionHandler = GetInteractionHandler();

        //TODO/MBA: need support for SID_STREAM
        SFX_ITEMSET_ARG( pSet, pWriteStreamItem, SfxUnoAnyItem, SID_STREAM, sal_False);
        SFX_ITEMSET_ARG( pSet, pInStreamItem, SfxUnoAnyItem, SID_INPUTSTREAM, sal_False);
        if ( pWriteStreamItem )
        {
            pWriteStreamItem->GetValue() >>= pImp->xStream;

            if ( pInStreamItem )
                pInStreamItem->GetValue() >>= pImp->xInputStream;

            if ( !pImp->xInputStream.is() && pImp->xStream.is() )
                pImp->xInputStream = pImp->xStream->getInputStream();
        }
        else if ( pInStreamItem )
        {
            pInStreamItem->GetValue() >>= pImp->xInputStream;
        }
        else
        {
            uno::Sequence < beans::PropertyValue > xProps;
            String aFileName;
            if ( aName.Len() )
            {
                if ( !::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aFileName ) )
                {
                    OSL_FAIL("Physical name not convertable!");
                }
            }
            else
                aFileName = GetName();

            // in case the temporary file exists the streams should be initialized from it,
            // but the original MediaDescriptor should not be changed
            sal_Bool bFromTempFile = ( pImp->pTempFile != NULL );

            if ( !bFromTempFile )
            {
                GetItemSet()->Put( SfxStringItem( SID_FILE_NAME, aFileName ) );
                if( !(nStorOpenMode & STREAM_WRITE ) )
                    GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, TRUE ) );
                if (xInteractionHandler.is())
                    GetItemSet()->Put( SfxUnoAnyItem( SID_INTERACTIONHANDLER, makeAny(xInteractionHandler) ) );
            }

            if ( m_xInputStreamToLoadFrom.is() )
            {
                pImp->xInputStream = m_xInputStreamToLoadFrom;
                pImp->xInputStream->skipBytes(0);
                if(m_bIsReadOnly)
                    GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, sal_True ) );
            }
            else
            {
                TransformItems( SID_OPENDOC, *GetItemSet(), xProps );
                comphelper::MediaDescriptor aMedium( xProps );

                if ( pImp->m_xLockingStream.is() && !bFromTempFile )
                {
                    // the medium is not based on the temporary file, so the original stream can be used
                    pImp->xStream = pImp->m_xLockingStream;
                }
                else
                {
                    if ( bFromTempFile )
                    {
                        aMedium[comphelper::MediaDescriptor::PROP_URL()] <<= ::rtl::OUString( aFileName );
                        aMedium.erase( comphelper::MediaDescriptor::PROP_READONLY() );
                        aMedium.addInputStream();
                    }
                    else if ( ::utl::LocalFileHelper::IsLocalFile( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) ) )
                    {
                        // use the special locking approach only for file URLs
                        aMedium.addInputStreamOwnLock();
                    }
                    else
                        aMedium.addInputStream();

                    // the ReadOnly property set in aMedium is ignored
                    // the check is done in LockOrigFileOnDemand() for file and non-file URLs

                    //TODO/MBA: what happens if property is not there?!
                    aMedium[comphelper::MediaDescriptor::PROP_STREAM()] >>= pImp->xStream;
                    aMedium[comphelper::MediaDescriptor::PROP_INPUTSTREAM()] >>= pImp->xInputStream;
                }

                GetContent();
                if ( !pImp->xInputStream.is() && pImp->xStream.is() )
                    pImp->xInputStream = pImp->xStream->getInputStream();
            }

            if ( !bFromTempFile )
            {
                //TODO/MBA: need support for SID_STREAM
                if ( pImp->xStream.is() )
                    GetItemSet()->Put( SfxUsrAnyItem( SID_STREAM, makeAny( pImp->xStream ) ) );

                GetItemSet()->Put( SfxUsrAnyItem( SID_INPUTSTREAM, makeAny( pImp->xInputStream ) ) );
            }
        }

        //TODO/MBA: ErrorHandling - how to transport error from MediaDescriptor
        if ( !GetError() && !pImp->xStream.is() && !pImp->xInputStream.is() )
            SetError( ERRCODE_IO_ACCESSDENIED, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );

        if ( !GetError() )
        {
            if ( pImp->xStream.is() )
                pInStream = utl::UcbStreamHelper::CreateStream( pImp->xStream );
            else if ( pImp->xInputStream.is() )
                pInStream = utl::UcbStreamHelper::CreateStream( pImp->xInputStream );
        }

        pImp->bDownloadDone = sal_True;
        pImp->aDoneLink.ClearPendingCall();
        sal_uIntPtr nError = GetError();
        pImp->aDoneLink.Call( (void*)nError );
    }
}

//----------------------------------------------------------------
sal_Bool SfxMedium::IsRemote()
{
    return bRemote;
}

//------------------------------------------------------------------

void SfxMedium::SetUpdatePickList(sal_Bool bVal)
{
    if(!pImp)
        pImp = new SfxMedium_Impl( this );
    pImp->bUpdatePickList = bVal;
}
//------------------------------------------------------------------

sal_Bool SfxMedium::IsUpdatePickList() const
{
    return pImp? pImp->bUpdatePickList: sal_True;
}
//----------------------------------------------------------------

void SfxMedium::SetDoneLink( const Link& rLink )
{
    pImp->aDoneLink = rLink;
}

//----------------------------------------------------------------

void SfxMedium::SetDataAvailableLink( const Link& rLink )
{
    pImp->aAvailableLink = rLink;
}

//----------------------------------------------------------------
void SfxMedium::StartDownload()
{
    GetInStream();
}

void SfxMedium::DownLoad( const Link& aLink )
{
    SetDoneLink( aLink );
    GetInStream();
    if ( pInStream && !aLink.IsSet() )
    {
        while( !pImp->bDownloadDone )
            Application::Yield();
    }
}

//------------------------------------------------------------------
void SfxMedium::Init_Impl()
/*  [Beschreibung]
    Setzt in den Logischen Namen eine gueltige ::com::sun::star::util::URL (Falls zuvor ein Filename
    drin war) und setzt den physikalschen Namen auf den Filenamen, falls
    vorhanden.
 */

{
    Reference< XOutputStream > rOutStream;

    // TODO/LATER: handle lifetime of storages
    pImp->bDisposeStorage = FALSE;

    SFX_ITEMSET_ARG( pSet, pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False);
    if ( pSalvageItem && !pSalvageItem->GetValue().Len() )
    {
        pSalvageItem = NULL;
        pSet->ClearItem( SID_DOC_SALVAGE );
    }

    if( aLogicName.Len() )
    {
        INetURLObject aUrl( aLogicName );
        INetProtocol eProt = aUrl.GetProtocol();
        if ( eProt == INET_PROT_NOT_VALID )
        {
            OSL_FAIL( "Unknown protocol!" );
        }
        else
        {
            if ( aUrl.HasMark() )
            {
                aLogicName = aUrl.GetURLNoMark( INetURLObject::NO_DECODE );
                GetItemSet()->Put( SfxStringItem( SID_JUMPMARK, aUrl.GetMark() ) );
            }

            // try to convert the URL into a physical name - but never change a physical name
            // physical name may be set if the logical name is changed after construction
            if ( !aName.Len() )
                ::utl::LocalFileHelper::ConvertURLToPhysicalName( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ), aName );
            else {
                DBG_ASSERT( pSalvageItem, "Suspicious change of logical name!" );
            }
        }
    }

    if ( pSalvageItem && pSalvageItem->GetValue().Len() )
    {
        aLogicName = pSalvageItem->GetValue();
        DELETEZ( pURLObj );
        pImp->m_bSalvageMode = sal_True;
    }

    // in case output stream is by mistake here
    // clear the reference
    SFX_ITEMSET_ARG( pSet, pOutStreamItem, SfxUnoAnyItem, SID_OUTPUTSTREAM, sal_False);
    if( pOutStreamItem
     && ( !( pOutStreamItem->GetValue() >>= rOutStream )
          || !aLogicName.CompareToAscii( "private:stream", 14 ) == COMPARE_EQUAL ) )
    {
        pSet->ClearItem( SID_OUTPUTSTREAM );
        OSL_FAIL( "Unexpected Output stream parameter!\n" );
    }

    if ( aLogicName.Len() )
    {
        // if the logic name is set it should be set in MediaDescriptor as well
        SFX_ITEMSET_ARG( pSet, pFileNameItem, SfxStringItem, SID_FILE_NAME, FALSE );
        if ( !pFileNameItem )
        {
            // let the ItemSet be created if necessary
            GetItemSet()->Put( SfxStringItem( SID_FILE_NAME, INetURLObject( aLogicName ).GetMainURL( INetURLObject::NO_DECODE ) ) );
        }
    }

    SetIsRemote_Impl();
}

//------------------------------------------------------------------
SfxMedium::SfxMedium()
:   IMPL_CTOR( sal_False, 0 ),  // bRoot, pURLObj

    pFilter(0),
    pSet(0),
    pImp(new SfxMedium_Impl( this ))
{
    Init_Impl();
}
//------------------------------------------------------------------

SfxMedium::SfxMedium( const SfxMedium& rMedium, sal_Bool bTemporary )
:   SvRefBase(),
    IMPL_CTOR( sal_True,    // bRoot, pURLObj
        rMedium.pURLObj ? new INetURLObject(*rMedium.pURLObj) : 0 ),
    pImp(new SfxMedium_Impl( this ))
{
    bDirect       = rMedium.IsDirect();
    nStorOpenMode = rMedium.GetOpenMode();
    if ( !bTemporary )
        aName = rMedium.aName;

    pImp->bIsTemp = bTemporary;
    DBG_ASSERT( ! rMedium.pImp->bIsTemp, "Temporaeres Medium darf nicht kopiert werden" );
    aLogicName = rMedium.aLogicName;
    pSet =  rMedium.GetItemSet() ? new SfxItemSet(*rMedium.GetItemSet()) : 0;
    pFilter = rMedium.pFilter;
    Init_Impl();
    if( bTemporary )
        CreateTempFile( sal_True );
}

//------------------------------------------------------------------

void SfxMedium::UseInteractionHandler( BOOL bUse )
{
    pImp->bAllowDefaultIntHdl = bUse;
}

//------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >
SfxMedium::GetInteractionHandler()
{
    // if interaction isnt allowed explicitly ... return empty reference!
    if ( !pImp->bUseInteractionHandler )
        return ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >();

    // search a possible existing handler inside cached item set
    if ( pSet )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > xHandler;
        SFX_ITEMSET_ARG( pSet, pHandler, SfxUnoAnyItem, SID_INTERACTIONHANDLER, sal_False);
        if ( pHandler && (pHandler->GetValue() >>= xHandler) && xHandler.is() )
            return xHandler;
    }

    // if default interaction isnt allowed explicitly ... return empty reference!
    if ( !pImp->bAllowDefaultIntHdl )
        return ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >();

    // otherwhise return cached default handler ... if it exist.
    if ( pImp->xInteraction.is() )
        return pImp->xInteraction;

    // create default handler and cache it!
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
    if ( xFactory.is() )
    {
        pImp->xInteraction = ::com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler >( xFactory->createInstance( DEFINE_CONST_UNICODE("com.sun.star.task.InteractionHandler") ), ::com::sun::star::uno::UNO_QUERY );
        return pImp->xInteraction;
    }

    return ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >();
}

//----------------------------------------------------------------

void SfxMedium::SetFilter( const SfxFilter* pFilterP, sal_Bool /*bResetOrig*/ )
{
    pFilter = pFilterP;
    pImp->nFileVersion = 0;
}

//----------------------------------------------------------------

const SfxFilter* SfxMedium::GetOrigFilter( sal_Bool bNotCurrent ) const
{
    return ( pImp->pOrigFilter || bNotCurrent ) ? pImp->pOrigFilter : pFilter;
}

//----------------------------------------------------------------

void SfxMedium::SetOrigFilter_Impl( const SfxFilter* pOrigFilter )
{
    pImp->pOrigFilter = pOrigFilter;
}

//------------------------------------------------------------------

sal_uInt32 SfxMedium::CreatePasswordToModifyHash( const ::rtl::OUString& aPasswd, sal_Bool bWriter )
{
    sal_uInt32 nHash = 0;

    if ( aPasswd.getLength() )
    {
        if ( bWriter )
        {
            nHash = ::comphelper::DocPasswordHelper::GetWordHashAsUINT32( aPasswd );
        }
        else
        {
            rtl_TextEncoding nEncoding = RTL_TEXTENCODING_UTF8;

            // if the MS-filter should be used
            // use the inconsistent algorithm to find the encoding specified by MS
            nEncoding = osl_getThreadTextEncoding();
            switch( nEncoding )
            {
                case RTL_TEXTENCODING_ISO_8859_15:
                case RTL_TEXTENCODING_MS_874:
                case RTL_TEXTENCODING_MS_1250:
                case RTL_TEXTENCODING_MS_1251:
                case RTL_TEXTENCODING_MS_1252:
                case RTL_TEXTENCODING_MS_1253:
                case RTL_TEXTENCODING_MS_1254:
                case RTL_TEXTENCODING_MS_1255:
                case RTL_TEXTENCODING_MS_1256:
                case RTL_TEXTENCODING_MS_1257:
                case RTL_TEXTENCODING_MS_1258:
                case RTL_TEXTENCODING_SHIFT_JIS:
                case RTL_TEXTENCODING_GB_2312:
                case RTL_TEXTENCODING_BIG5:
                    // in case the system uses an encoding from the list above, it should be used
                    break;

                default:
                    // in case other encoding is used, use one of the encodings from the list
                    nEncoding = RTL_TEXTENCODING_MS_1250;
                    break;
            }

            nHash = ::comphelper::DocPasswordHelper::GetXLHashAsUINT16( aPasswd, nEncoding );
        }
    }

    return nHash;
}

//------------------------------------------------------------------

void SfxMedium::Close()
{
    if ( pImp->xStorage.is() )
    {
        CloseStorage();
    }

    CloseStreams_Impl();

    UnlockFile( sal_False );
}

void SfxMedium::CloseAndRelease()
{
    if ( pImp->xStorage.is() )
    {
        CloseStorage();
    }

    CloseAndReleaseStreams_Impl();

    UnlockFile( sal_True );
}

void SfxMedium::UnlockFile( sal_Bool bReleaseLockStream )
{
    if ( pImp->m_xLockingStream.is() )
    {
        if ( bReleaseLockStream )
        {
            try
            {
                uno::Reference< io::XInputStream > xInStream = pImp->m_xLockingStream->getInputStream();
                uno::Reference< io::XOutputStream > xOutStream = pImp->m_xLockingStream->getOutputStream();
                if ( xInStream.is() )
                    xInStream->closeInput();
                if ( xOutStream.is() )
                    xOutStream->closeOutput();
            }
            catch( uno::Exception& )
            {}
        }

        pImp->m_xLockingStream = uno::Reference< io::XStream >();
    }

    if ( pImp->m_bLocked )
    {
        try
        {
            pImp->m_bLocked = sal_False;
            ::svt::DocumentLockFile aLockFile( aLogicName );
            // TODO/LATER: A warning could be shown in case the file is not the own one
            aLockFile.RemoveFile();
        }
        catch( uno::Exception& )
        {}
    }
}

void SfxMedium::CloseAndReleaseStreams_Impl()
{
    CloseZipStorage_Impl();

    uno::Reference< io::XInputStream > xInToClose = pImp->xInputStream;
    uno::Reference< io::XOutputStream > xOutToClose;
    if ( pImp->xStream.is() )
    {
        xOutToClose = pImp->xStream->getOutputStream();

        // if the locking stream is closed here the related member should be cleaned
        if ( pImp->xStream == pImp->m_xLockingStream )
            pImp->m_xLockingStream = uno::Reference< io::XStream >();
    }

    // The probably exsisting SvStream wrappers should be closed first
    CloseStreams_Impl();

    // in case of salvage mode the storage is based on the streams
    if ( !pImp->m_bSalvageMode )
    {
        try
        {
            if ( xInToClose.is() )
                xInToClose->closeInput();
            if ( xOutToClose.is() )
                xOutToClose->closeOutput();
        }
        catch ( uno::Exception& )
        {
        }
    }
}

//------------------------------------------------------------------
void SfxMedium::CloseStreams_Impl()
{
    CloseInStream_Impl();
    CloseOutStream_Impl();

    if ( pSet )
        pSet->ClearItem( SID_CONTENT );

    pImp->aContent = ::ucbhelper::Content();
}

//------------------------------------------------------------------

void SfxMedium::RefreshName_Impl()
{
}

void SfxMedium::SetIsRemote_Impl()
{
    INetURLObject aObj( GetName() );
    switch( aObj.GetProtocol() )
    {
        case INET_PROT_FTP:
        case INET_PROT_HTTP:
        case INET_PROT_HTTPS:
        case INET_PROT_POP3:
        case INET_PROT_NEWS:
        case INET_PROT_IMAP:
        case INET_PROT_VIM:
            bRemote = TRUE; break;
        default:
            bRemote = ( GetName().CompareToAscii( "private:msgid", 13 ) == COMPARE_EQUAL );
            break;
    }

    // Da Dateien, die Remote geschrieben werden zur Uebertragung auch
    // gelesen werden koennen muessen
    if( bRemote )
        nStorOpenMode |= STREAM_READ;
}



void SfxMedium::SetName( const String& aNameP, sal_Bool bSetOrigURL )
{
    if( !pImp->aOrigURL.Len() )
        pImp->aOrigURL = aLogicName;
    if( bSetOrigURL )
        pImp->aOrigURL = aNameP;
    aLogicName = aNameP;
    DELETEZ( pURLObj );
    pImp->aContent = ::ucbhelper::Content();
    Init_Impl();
}

//----------------------------------------------------------------
const String& SfxMedium::GetOrigURL() const
{
    return !pImp->aOrigURL.Len() ? (String &)aLogicName : pImp->aOrigURL;
}

//----------------------------------------------------------------

void SfxMedium::SetPhysicalName_Impl( const String& rNameP )
{
    if ( rNameP != aName )
    {
        if( pImp->pTempFile )
        {
            delete pImp->pTempFile;
            pImp->pTempFile = NULL;
        }

        if ( aName.Len() || rNameP.Len() )
            pImp->aContent = ::ucbhelper::Content();

        aName = rNameP;
        bTriedStorage = sal_False;
        pImp->bIsStorage = sal_False;
    }
}

//------------------------------------------------------------------
void SfxMedium::SetTemporary( sal_Bool bTemp )
{
    pImp->bIsTemp = bTemp;
}

//------------------------------------------------------------------
sal_Bool SfxMedium::IsTemporary() const
{
    return pImp->bIsTemp;
}

//------------------------------------------------------------------

sal_Bool SfxMedium::Exists( sal_Bool /*bForceSession*/ )
{
    OSL_FAIL( "Not implemented!" );
    return sal_True;
}

//------------------------------------------------------------------

void SfxMedium::ReOpen()
{
    BOOL bUseInteractionHandler = pImp->bUseInteractionHandler;
    pImp->bUseInteractionHandler = FALSE;
    GetMedium_Impl();
    pImp->bUseInteractionHandler = bUseInteractionHandler;
}

//------------------------------------------------------------------

void SfxMedium::CompleteReOpen()
{
    // do not use temporary file for reopen and in case of success throw the temporary file away
    BOOL bUseInteractionHandler = pImp->bUseInteractionHandler;
    pImp->bUseInteractionHandler = FALSE;

    ::utl::TempFile* pTmpFile = NULL;
    if ( pImp->pTempFile )
    {
        pTmpFile = pImp->pTempFile;
        pImp->pTempFile = NULL;
        aName = String();
    }

    GetMedium_Impl();

    if ( GetError() )
    {
        if ( pImp->pTempFile )
        {
            pImp->pTempFile->EnableKillingFile( sal_True );
            delete pImp->pTempFile;
        }
        pImp->pTempFile = pTmpFile;
        if ( pImp->pTempFile )
            aName = pImp->pTempFile->GetFileName();
    }
    else
    {
        pTmpFile->EnableKillingFile( sal_True );
        delete pTmpFile;

    }

    pImp->bUseInteractionHandler = bUseInteractionHandler;
}

//------------------------------------------------------------------
SfxMedium::SfxMedium
(
    const String &rName, StreamMode nOpenMode,  sal_Bool bDirectP,
    const SfxFilter *pFlt, SfxItemSet *pInSet
)
:   IMPL_CTOR( sal_False, 0 ),  // bRoot, pURLObj
    pFilter(pFlt),
    pSet( pInSet ),
    pImp(new SfxMedium_Impl( this ))
{
    aLogicName = rName;
    nStorOpenMode = nOpenMode;
    bDirect = bDirectP;
    Init_Impl();
}


SfxMedium::SfxMedium( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs )
    : IMPL_CTOR( sal_False, 0 ),  // bRoot, pURLObj
    pFilter(0),
    pSet(0),
    pImp(new SfxMedium_Impl( this ))
{
    SfxAllItemSet *pParams = new SfxAllItemSet( SFX_APP()->GetPool() );
    pSet = pParams;
    TransformParameters( SID_OPENDOC, aArgs, *pParams );

    String aFilterName;
    SFX_ITEMSET_ARG( pSet, pFilterNameItem, SfxStringItem, SID_FILTER_NAME, sal_False );
    if( pFilterNameItem )
        aFilterName = pFilterNameItem->GetValue();
    pFilter = SFX_APP()->GetFilterMatcher().GetFilter4FilterName( aFilterName );

    SFX_ITEMSET_ARG( pSet, pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False );
    if( pSalvageItem )
    {
        // QUESTION: there is some treatment of Salvage in Init_Impl; align!
        if ( pSalvageItem->GetValue().Len() )
        {
            // if an URL is provided in SalvageItem that means that the FileName refers to a temporary file
            // that must be copied here

            SFX_ITEMSET_ARG( pSet, pFileNameItem, SfxStringItem, SID_FILE_NAME, FALSE );
            if (!pFileNameItem) throw uno::RuntimeException();
            ::rtl::OUString aNewTempFileURL = SfxMedium::CreateTempCopyWithExt( pFileNameItem->GetValue() );
            if ( aNewTempFileURL.getLength() )
            {
                pSet->Put( SfxStringItem( SID_FILE_NAME, aNewTempFileURL ) );
                pSet->ClearItem( SID_INPUTSTREAM );
                pSet->ClearItem( SID_STREAM );
                pSet->ClearItem( SID_CONTENT );
            }
            else
            {
                OSL_ENSURE( sal_False, "Can not create a new temporary file for crash recovery!\n" );
            }
        }
    }

    BOOL bReadOnly = FALSE;
    SFX_ITEMSET_ARG( pSet, pReadOnlyItem, SfxBoolItem, SID_DOC_READONLY, FALSE );
    if ( pReadOnlyItem && pReadOnlyItem->GetValue() )
        bReadOnly = TRUE;

    SFX_ITEMSET_ARG( pSet, pFileNameItem, SfxStringItem, SID_FILE_NAME, FALSE );
    if (!pFileNameItem) throw uno::RuntimeException();
    aLogicName = pFileNameItem->GetValue();
    nStorOpenMode = bReadOnly ? SFX_STREAM_READONLY : SFX_STREAM_READWRITE;
    bDirect = FALSE;
    Init_Impl();
}


//------------------------------------------------------------------

SfxMedium::SfxMedium( const uno::Reference < embed::XStorage >& rStor, const String& rBaseURL, const SfxItemSet* p, sal_Bool bRootP )
:   IMPL_CTOR( bRootP, 0 ), // bRoot, pURLObj
    pSet(0),
    pImp( new SfxMedium_Impl( this ))
{
    String aType = SfxFilter::GetTypeFromStorage( rStor );
    pFilter = SFX_APP()->GetFilterMatcher().GetFilter4EA( aType );
    DBG_ASSERT( pFilter, "No Filter for storage found!" );

    Init_Impl();
    pImp->xStorage = rStor;
    pImp->bDisposeStorage = FALSE;

    // always take BaseURL first, could be overwritten by ItemSet
    GetItemSet()->Put( SfxStringItem( SID_DOC_BASEURL, rBaseURL ) );
    if ( p )
        GetItemSet()->Put( *p );
}

//------------------------------------------------------------------

SfxMedium::~SfxMedium()
{
    // if there is a requirement to clean the backup this is the last possibility to do it
    ClearBackup_Impl();

    Close();

    delete pSet;

    if( pImp->bIsTemp && aName.Len() )
    {
        String aTemp;
        if ( !::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aTemp ))
        {
            OSL_FAIL("Physical name not convertable!");
        }

        if ( !::utl::UCBContentHelper::Kill( aTemp ) )
        {
            OSL_FAIL("Couldn't remove temporary file!");
        }
    }

    pFilter = 0;

    delete pURLObj;
    delete pImp;
}

//------------------------------------------------------------------
void SfxMedium::SetItemSet(SfxItemSet *pNewSet)
{
    delete pSet;
    pSet = pNewSet;
}

//----------------------------------------------------------------
const INetURLObject& SfxMedium::GetURLObject() const
{
    if( !pURLObj )
    {
        SfxMedium* pThis = const_cast < SfxMedium* > (this);
        pThis->pURLObj = new INetURLObject( aLogicName );
        if ( pThis->pURLObj->HasMark() )
            (*pThis->pURLObj) = INetURLObject( aLogicName ).GetURLNoMark();
    }

    return *pURLObj;
}

//----------------------------------------------------------------

const String& SfxMedium::GetPreRedirectedURL() const
{
    return pImp->aPreRedirectionURL;
}

//----------------------------------------------------------------

void SfxMedium::SetReferer( const String& rRefer )
{
    pImp->aReferer = rRefer;
}
//----------------------------------------------------------------

const String& SfxMedium::GetReferer( ) const
{
    return pImp->aReferer;
}

//----------------------------------------------------------------

void SfxMedium::SetExpired_Impl( const DateTime& rDateTime )
{
    pImp->aExpireTime = rDateTime;
}
//----------------------------------------------------------------

sal_Bool SfxMedium::IsExpired() const
{
    return pImp->aExpireTime.IsValid() && pImp->aExpireTime < DateTime();
}
//----------------------------------------------------------------

void SfxMedium::ForceSynchronStream_Impl( sal_Bool bForce )
{
    if( pInStream )
    {
        SvLockBytes* pBytes = pInStream->GetLockBytes();
        if( pBytes )
            pBytes->SetSynchronMode( bForce );
    }
    pImp->bForceSynchron = bForce;
}

//----------------------------------------------------------------
SfxFrame* SfxMedium::GetLoadTargetFrame() const
{
    return pImp->wLoadTargetFrame;
}
//----------------------------------------------------------------

void SfxMedium::SetLoadTargetFrame(SfxFrame* pFrame )
{
    pImp->wLoadTargetFrame = pFrame;
}
//----------------------------------------------------------------

void SfxMedium::SetStorage_Impl( const uno::Reference < embed::XStorage >& rStor )
{
    pImp->xStorage = rStor;
}
//----------------------------------------------------------------

SfxItemSet* SfxMedium::GetItemSet() const
{
    // this method *must* return an ItemSet, returning NULL can cause crashes
    if( !pSet )
        ((SfxMedium*)this)->pSet = new SfxAllItemSet( SFX_APP()->GetPool() );
    return pSet;
}
//----------------------------------------------------------------

SvKeyValueIterator* SfxMedium::GetHeaderAttributes_Impl()
{
    if( !pImp->xAttributes.Is() )
    {
        pImp->xAttributes = SvKeyValueIteratorRef( new SvKeyValueIterator );

        if ( GetContent().is() )
        {
            pImp->bIsCharsetInitialized = sal_True;

            try
            {
                Any aAny = pImp->aContent.getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MediaType")) );
                ::rtl::OUString aContentType;
                aAny >>= aContentType;

                pImp->xAttributes->Append( SvKeyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("content-type")), aContentType ) );
            }
            catch ( ::com::sun::star::uno::Exception& )
            {
            }
        }
    }

    return pImp->xAttributes;
}
//----------------------------------------------------------------

SvCompatWeakHdl* SfxMedium::GetHdl()
{
    return pImp->GetHdl();
}

sal_Bool SfxMedium::IsDownloadDone_Impl()
{
    return pImp->bDownloadDone;
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >  SfxMedium::GetInputStream()
{
    if ( !pImp->xInputStream.is() )
        GetMedium_Impl();
    return pImp->xInputStream;
}

const uno::Sequence < util::RevisionTag >& SfxMedium::GetVersionList( bool _bNoReload )
{
    // if the medium has no name, then this medium should represent a new document and can have no version info
    if ( ( !_bNoReload || !pImp->m_bVersionsAlreadyLoaded ) && !pImp->aVersions.getLength() &&
         ( aName.Len() || aLogicName.Len() ) && GetStorage().is() )
    {
        uno::Reference < document::XDocumentRevisionListPersistence > xReader( comphelper::getProcessServiceFactory()->createInstance(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.DocumentRevisionListPersistence")) ), uno::UNO_QUERY );
        if ( xReader.is() )
        {
            try
            {
                pImp->aVersions = xReader->load( GetStorage() );
            }
            catch ( uno::Exception& )
            {
            }
        }
    }

    if ( !pImp->m_bVersionsAlreadyLoaded )
        pImp->m_bVersionsAlreadyLoaded = sal_True;

    return pImp->aVersions;
}

uno::Sequence < util::RevisionTag > SfxMedium::GetVersionList( const uno::Reference < embed::XStorage >& xStorage )
{
    uno::Reference < document::XDocumentRevisionListPersistence > xReader( comphelper::getProcessServiceFactory()->createInstance(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.DocumentRevisionListPersistence")) ), uno::UNO_QUERY );
    if ( xReader.is() )
    {
        try
        {
            return xReader->load( xStorage );
        }
        catch ( uno::Exception& )
        {
        }
    }

    return uno::Sequence < util::RevisionTag >();
}

sal_uInt16 SfxMedium::AddVersion_Impl( util::RevisionTag& rRevision )
{
    if ( GetStorage().is() )
    {
        // Einen eindeutigen Namen f"ur den Stream ermitteln
        SvULongs aLongs;
        sal_Int32 nLength = pImp->aVersions.getLength();
        for ( sal_Int32 m=0; m<nLength; m++ )
        {
            sal_uInt32 nVer = (sal_uInt32) String( pImp->aVersions[m].Identifier ).Copy(7).ToInt32();
            sal_uInt16 n;
            for ( n=0; n<aLongs.Count(); n++ )
                if ( nVer<aLongs[n] )
                    break;

            aLongs.Insert( nVer, n );
        }

        sal_uInt16 nKey;
        for ( nKey=0; nKey<aLongs.Count(); nKey++ )
            if ( aLongs[nKey] > ( ULONG ) nKey+1 )
                break;

        String aRevName = DEFINE_CONST_UNICODE( "Version" );
        aRevName += String::CreateFromInt32( nKey + 1 );
        pImp->aVersions.realloc( nLength+1 );
        rRevision.Identifier = aRevName;
        pImp->aVersions[nLength] = rRevision;
        return nKey;
    }

    return 0;
}

sal_Bool SfxMedium::RemoveVersion_Impl( const ::rtl::OUString& rName )
{
    if ( !pImp->aVersions.getLength() )
        return sal_False;

    sal_Int32 nLength = pImp->aVersions.getLength();
    for ( sal_Int32 n=0; n<nLength; n++ )
    {
        if ( pImp->aVersions[n].Identifier == rName )
        {
            for ( sal_Int32 m=n; m<nLength-1; m++ )
                pImp->aVersions[m] = pImp->aVersions[m+1];
            pImp->aVersions.realloc(nLength-1);
            return sal_True;
        }
    }

    return sal_False;
}

sal_Bool SfxMedium::TransferVersionList_Impl( SfxMedium& rMedium )
{
    if ( rMedium.pImp->aVersions.getLength() )
    {
        pImp->aVersions = rMedium.pImp->aVersions;
        return sal_True;
    }

    return sal_False;
}

sal_Bool SfxMedium::SaveVersionList_Impl( sal_Bool /*bUseXML*/ )
{
    if ( GetStorage().is() )
    {
        if ( !pImp->aVersions.getLength() )
            return sal_True;

        uno::Reference < document::XDocumentRevisionListPersistence > xWriter( comphelper::getProcessServiceFactory()->createInstance(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.DocumentRevisionListPersistence")) ), uno::UNO_QUERY );
        if ( xWriter.is() )
        {
            try
            {
                xWriter->store( GetStorage(), pImp->aVersions );
                return sal_True;
            }
            catch ( uno::Exception& )
            {
            }
        }
    }

    return sal_False;
}

//----------------------------------------------------------------
sal_Bool SfxMedium::IsReadOnly()
{
    sal_Bool bReadOnly = sal_False;

    // a) ReadOnly filter cant produce read/write contents!
    bReadOnly = (
                    (pFilter                                                                         ) &&
                    ((pFilter->GetFilterFlags() & SFX_FILTER_OPENREADONLY) == SFX_FILTER_OPENREADONLY)
                );

    // b) if filter allow read/write contents .. check open mode of the storage
    if (!bReadOnly)
        bReadOnly = !( GetOpenMode() & STREAM_WRITE );

    // c) the API can force the readonly state!
    if (!bReadOnly)
    {
        SFX_ITEMSET_ARG( GetItemSet(), pItem, SfxBoolItem, SID_DOC_READONLY, sal_False);
        if (pItem)
            bReadOnly = pItem->GetValue();
    }

    return bReadOnly;
}

//----------------------------------------------------------------
sal_Bool SfxMedium::SetWritableForUserOnly( const ::rtl::OUString& aURL )
{
    // UCB does not allow to allow write access only for the user,
    // use osl API
    sal_Bool bResult = sal_False;

    ::osl::DirectoryItem aDirItem;
    if ( ::osl::DirectoryItem::get( aURL, aDirItem ) == ::osl::FileBase::E_None )
    {
        ::osl::FileStatus aFileStatus( FileStatusMask_Attributes );
        if ( aDirItem.getFileStatus( aFileStatus ) == osl::FileBase::E_None
          && aFileStatus.isValid( FileStatusMask_Attributes ) )
        {
            sal_uInt64 nAttributes = aFileStatus.getAttributes();

            nAttributes &= ~(Attribute_OwnWrite |
                             Attribute_GrpWrite |
                             Attribute_OthWrite |
                             Attribute_ReadOnly);
            nAttributes |= Attribute_OwnWrite;

            bResult = ( osl::File::setAttributes( aURL, nAttributes ) == ::osl::FileBase::E_None );
        }
    }

    return bResult;
}

//----------------------------------------------------------------
void SfxMedium::CreateTempFile( sal_Bool bReplace )
{
    if ( pImp->pTempFile )
    {
        if ( !bReplace )
            return;

        DELETEZ( pImp->pTempFile );
        aName = String();
    }

    pImp->pTempFile = new ::utl::TempFile();
    pImp->pTempFile->EnableKillingFile( sal_True );
    aName = pImp->pTempFile->GetFileName();
    ::rtl::OUString aTmpURL = pImp->pTempFile->GetURL();
    if ( !aName.Len() || !aTmpURL.getLength() )
    {
        SetError( ERRCODE_IO_CANTWRITE, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
        return;
    }

    if ( !( nStorOpenMode & STREAM_TRUNC ) )
    {
        sal_Bool bTransferSuccess = sal_False;

        if ( GetContent().is()
          && ::utl::LocalFileHelper::IsLocalFile( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) )
          && ::utl::UCBContentHelper::IsDocument( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) ) )
        {
            // if there is already such a document, we should copy it
            // if it is a file system use OS copy process
            try
            {
                uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > xComEnv;
                INetURLObject aTmpURLObj( aTmpURL );
                ::rtl::OUString aFileName = aTmpURLObj.getName( INetURLObject::LAST_SEGMENT,
                                                                true,
                                                                INetURLObject::DECODE_WITH_CHARSET );
                if ( aFileName.getLength() && aTmpURLObj.removeSegment() )
                {
                    ::ucbhelper::Content aTargetContent( aTmpURLObj.GetMainURL( INetURLObject::NO_DECODE ), xComEnv );
                    if ( aTargetContent.transferContent( pImp->aContent, ::ucbhelper::InsertOperation_COPY, aFileName, NameClash::OVERWRITE ) )
                    {
                        SetWritableForUserOnly( aTmpURL );
                        bTransferSuccess = sal_True;
                    }
                }
            }
            catch( uno::Exception& )
            {}

            if ( bTransferSuccess )
            {
                CloseOutStream();
                CloseInStream();
            }
        }

        if ( !bTransferSuccess && pInStream )
        {
            // the case when there is no URL-access available or this is a remote protocoll
            // but there is an input stream
            GetOutStream();
            if ( pOutStream )
            {
                char        *pBuf = new char [8192];
                sal_uInt32   nErr = ERRCODE_NONE;

                pInStream->Seek(0);
                pOutStream->Seek(0);

                while( !pInStream->IsEof() && nErr == ERRCODE_NONE )
                {
                    sal_uInt32 nRead = pInStream->Read( pBuf, 8192 );
                    nErr = pInStream->GetError();
                    pOutStream->Write( pBuf, nRead );
                }

                bTransferSuccess = sal_True;
                delete[] pBuf;
                CloseInStream();
            }
            CloseOutStream_Impl();
        }
        else
        {
            // Quite strange design, but currently it is expected that in this case no transfer happens
            // TODO/LATER: get rid of this inconsistent part of the call design
            bTransferSuccess = sal_True;
            CloseInStream();
        }

        if ( !bTransferSuccess )
        {
            SetError( ERRCODE_IO_CANTWRITE, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
            return;
        }
    }

    CloseStorage();
}

//----------------------------------------------------------------
void SfxMedium::CreateTempFileNoCopy()
{
    // this call always replaces the existing temporary file
    if ( pImp->pTempFile )
        delete pImp->pTempFile;

    pImp->pTempFile = new ::utl::TempFile();
    pImp->pTempFile->EnableKillingFile( sal_True );
    aName = pImp->pTempFile->GetFileName();
    if ( !aName.Len() )
    {
        SetError( ERRCODE_IO_CANTWRITE, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
        return;
    }

    CloseOutStream_Impl();
    CloseStorage();
}

::rtl::OUString SfxMedium::GetCharset()
{
    if( !pImp->bIsCharsetInitialized )
    {
        // Set an error in case there is no content?
        if ( GetContent().is() )
        {
            pImp->bIsCharsetInitialized = sal_True;

            try
            {
                Any aAny = pImp->aContent.getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MediaType")) );
                ::rtl::OUString aField;
                aAny >>= aField;

                ::rtl::OString sContent = ::rtl::OUStringToOString( aField, RTL_TEXTENCODING_ASCII_US );
                ByteString sType, sSubType;
                INetContentTypeParameterList aParameters;

                if( INetContentTypes::parse( sContent, sType, sSubType, &aParameters ) )
                {
                    const INetContentTypeParameter * pCharset = aParameters.find("charset");
                    if (pCharset != 0)
                        pImp->aCharset = pCharset->m_sValue;
                }
            }
            catch ( ::com::sun::star::uno::Exception& )
            {
            }
        }
    }

    return pImp->aCharset;
}

void SfxMedium::SetCharset( ::rtl::OUString aChs )
{
    pImp->bIsCharsetInitialized = sal_True;
    pImp->aCharset = aChs;
}

sal_Bool SfxMedium::SignContents_Impl( sal_Bool bScriptingContent, const ::rtl::OUString& aODFVersion, sal_Bool bHasValidDocumentSignature )
{
    sal_Bool bChanges = FALSE;

    // the medium should be closed to be able to sign, the caller is responsible to close it
    if ( !IsOpen() && !GetError() )
    {
        // The component should know if there was a valid document signature, since
        // it should show a warning in this case
        uno::Sequence< uno::Any > aArgs( 2 );
        aArgs[0] <<= aODFVersion;
        aArgs[1] <<= bHasValidDocumentSignature;
        ::com::sun::star::uno::Reference< ::com::sun::star::security::XDocumentDigitalSignatures > xSigner(
            comphelper::getProcessServiceFactory()->createInstanceWithArguments(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.security.DocumentDigitalSignatures" ) ),
                aArgs ),
            ::com::sun::star::uno::UNO_QUERY );

        if ( xSigner.is() )
        {
            uno::Reference< embed::XStorage > xWriteableZipStor;
            if ( !IsReadOnly() )
            {
                // we can reuse the temporary file if there is one already
                CreateTempFile( sal_False );
                GetMedium_Impl();

                try
                {
                    if ( !pImp->xStream.is() )
                        throw uno::RuntimeException();

                    xWriteableZipStor = ::comphelper::OStorageHelper::GetStorageOfFormatFromStream( ZIP_STORAGE_FORMAT_STRING, pImp->xStream );
                    if ( !xWriteableZipStor.is() )
                        throw uno::RuntimeException();

                    uno::Reference< embed::XStorage > xMetaInf = xWriteableZipStor->openStorageElement(
                                                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "META-INF" ) ),
                                                    embed::ElementModes::READWRITE );
                    if ( !xMetaInf.is() )
                        throw uno::RuntimeException();

                    if ( bScriptingContent )
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
                            ::rtl::OUString aDocSigName = xSigner->getDocumentContentSignatureDefaultStreamName();
                            if ( aDocSigName.getLength() && xMetaInf->hasByName( aDocSigName ) )
                                xMetaInf->removeElement( aDocSigName );

                            uno::Reference< embed::XTransactedObject > xTransact( xMetaInf, uno::UNO_QUERY_THROW );
                            xTransact->commit();
                            xTransact.set( xWriteableZipStor, uno::UNO_QUERY_THROW );
                            xTransact->commit();

                            // the temporary file has been written, commit it to the original file
                            Commit();
                            bChanges = TRUE;
                        }
                    }
                    else
                    {
                         uno::Reference< io::XStream > xStream(
                            xMetaInf->openStreamElement( xSigner->getDocumentContentSignatureDefaultStreamName(),
                                                         embed::ElementModes::READWRITE ),
                            uno::UNO_SET_THROW );

                        if ( xSigner->signDocumentContent( GetZipStorageToSign_Impl(), xStream ) )
                        {
                            uno::Reference< embed::XTransactedObject > xTransact( xMetaInf, uno::UNO_QUERY_THROW );
                            xTransact->commit();
                            xTransact.set( xWriteableZipStor, uno::UNO_QUERY_THROW );
                            xTransact->commit();

                            // the temporary file has been written, commit it to the original file
                            Commit();
                            bChanges = TRUE;
                        }
                    }
                }
                catch ( uno::Exception& )
                {
                    OSL_ENSURE( sal_False, "Couldn't use signing functionality!\n" );
                }

                CloseAndRelease();
            }
            else
            {
                try
                {
                    if ( bScriptingContent )
                        xSigner->showScriptingContentSignatures( GetZipStorageToSign_Impl(), uno::Reference< io::XInputStream >() );
                    else
                        xSigner->showDocumentContentSignatures( GetZipStorageToSign_Impl(), uno::Reference< io::XInputStream >() );
                }
                catch( uno::Exception& )
                {
                    OSL_ENSURE( sal_False, "Couldn't use signing functionality!\n" );
                }
            }
        }

        ResetError();
    }

    return bChanges;
}

//----------------------------------------------------------------
sal_uInt16 SfxMedium::GetCachedSignatureState_Impl()
{
    return pImp->m_nSignatureState;
}

//----------------------------------------------------------------
void SfxMedium::SetCachedSignatureState_Impl( sal_uInt16 nState )
{
    pImp->m_nSignatureState = nState;
}

BOOL SfxMedium::HasStorage_Impl() const
{
    return pImp->xStorage.is();
}

BOOL SfxMedium::IsOpen() const
{
    return pInStream || pOutStream || pImp->xStorage.is();
}

::rtl::OUString SfxMedium::CreateTempCopyWithExt( const ::rtl::OUString& aURL )
{
    ::rtl::OUString aResult;

    if ( aURL.getLength() )
    {
        sal_Int32 nPrefixLen = aURL.lastIndexOf( '.' );
        String aExt = ( nPrefixLen == -1 ) ? String() : String( aURL.copy( nPrefixLen ) );

        ::rtl::OUString aNewTempFileURL = ::utl::TempFile( String(), &aExt ).GetURL();
        if ( aNewTempFileURL.getLength() )
        {
            INetURLObject aSource( aURL );
            INetURLObject aDest( aNewTempFileURL );
            ::rtl::OUString aFileName = aDest.getName( INetURLObject::LAST_SEGMENT,
                                                        true,
                                                        INetURLObject::DECODE_WITH_CHARSET );
            if ( aFileName.getLength() && aDest.removeSegment() )
            {
                try
                {
                    uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > xComEnv;
                    ::ucbhelper::Content aTargetContent( aDest.GetMainURL( INetURLObject::NO_DECODE ), xComEnv );
                    ::ucbhelper::Content aSourceContent( aSource.GetMainURL( INetURLObject::NO_DECODE ), xComEnv );
                    if ( aTargetContent.transferContent( aSourceContent,
                                                        ::ucbhelper::InsertOperation_COPY,
                                                        aFileName,
                                                        NameClash::OVERWRITE ) )
                    {
                        // Success
                        aResult = aNewTempFileURL;
                    }
                }
                catch( uno::Exception& )
                {}
            }
        }
    }

    return aResult;
}

sal_Bool SfxMedium::CallApproveHandler( const uno::Reference< task::XInteractionHandler >& xHandler, uno::Any aRequest, sal_Bool bAllowAbort )
{
    sal_Bool bResult = sal_False;

    if ( xHandler.is() )
    {
        try
        {
            uno::Sequence< uno::Reference< task::XInteractionContinuation > > aContinuations( bAllowAbort ? 2 : 1 );

            ::rtl::Reference< ::framework::ContinuationApprove > pApprove( new ::framework::ContinuationApprove() );
            aContinuations[ 0 ] = pApprove.get();

            if ( bAllowAbort )
            {
                ::rtl::Reference< ::framework::ContinuationAbort > pAbort( new ::framework::ContinuationAbort() );
                aContinuations[ 1 ] = pAbort.get();
            }

            uno::Reference< task::XInteractionRequest > xRequest( new ::framework::InteractionRequest( aRequest, aContinuations ) );
            xHandler->handle( xRequest );

            bResult = pApprove->isSelected();
        }
        catch( const Exception& )
        {
        }
    }

    return bResult;
}

::rtl::OUString SfxMedium::SwitchDocumentToTempFile()
{
    // the method returns empty string in case of failure
    ::rtl::OUString aResult;
    ::rtl::OUString aOrigURL = aLogicName;

    if ( aOrigURL.getLength() )
    {
        sal_Int32 nPrefixLen = aOrigURL.lastIndexOf( '.' );
        String aExt = ( nPrefixLen == -1 ) ? String() : String( aOrigURL.copy( nPrefixLen ) );
        ::rtl::OUString aNewURL = ::utl::TempFile( String(), &aExt ).GetURL();

        // TODO/LATER: In future the aLogicName should be set to shared folder URL
        //             and a temporary file should be created. Transport_Impl should be impossible then.
        if ( aNewURL.getLength() )
        {
            uno::Reference< embed::XStorage > xStorage = GetStorage();
            uno::Reference< embed::XOptimizedStorage > xOptStorage( xStorage, uno::UNO_QUERY );

            if ( xOptStorage.is() )
            {
                // TODO/LATER: reuse the pImp->pTempFile if it already exists
                CanDisposeStorage_Impl( sal_False );
                Close();
                SetPhysicalName_Impl( String() );
                SetName( aNewURL );

                // remove the readonly state
                sal_Bool bWasReadonly = sal_False;
                nStorOpenMode = SFX_STREAM_READWRITE;
                SFX_ITEMSET_ARG( pSet, pReadOnlyItem, SfxBoolItem, SID_DOC_READONLY, FALSE );
                if ( pReadOnlyItem && pReadOnlyItem->GetValue() )
                    bWasReadonly = sal_True;
                GetItemSet()->ClearItem( SID_DOC_READONLY );

                GetMedium_Impl();
                LockOrigFileOnDemand( sal_False, sal_False );
                CreateTempFile( sal_True );
                GetMedium_Impl();

                if ( pImp->xStream.is() )
                {
                    try
                    {
                        xOptStorage->writeAndAttachToStream( pImp->xStream );
                        pImp->xStorage = xStorage;
                        aResult = aNewURL;
                    }
                    catch( uno::Exception& )
                    {}
                }

                if ( !aResult.getLength() )
                {
                    Close();
                    SetPhysicalName_Impl( String() );
                    SetName( aOrigURL );
                    if ( bWasReadonly )
                    {
                        // set the readonly state back
                        nStorOpenMode = SFX_STREAM_READONLY;
                        GetItemSet()->Put( SfxBoolItem(SID_DOC_READONLY, sal_True));
                    }
                    GetMedium_Impl();
                    pImp->xStorage = xStorage;
                }
            }
        }
    }

    return aResult;
}

sal_Bool SfxMedium::SwitchDocumentToFile( ::rtl::OUString aURL )
{
    // the method is only for storage based documents
    sal_Bool bResult = sal_False;
    ::rtl::OUString aOrigURL = aLogicName;

    if ( aURL.getLength() && aOrigURL.getLength() )
    {
        uno::Reference< embed::XStorage > xStorage = GetStorage();
        uno::Reference< embed::XOptimizedStorage > xOptStorage( xStorage, uno::UNO_QUERY );

        if ( xOptStorage.is() )
        {
            // TODO/LATER: reuse the pImp->pTempFile if it already exists
            CanDisposeStorage_Impl( sal_False );
            Close();
            SetPhysicalName_Impl( String() );
            SetName( aURL );

            // open the temporary file based document
            GetMedium_Impl();
            LockOrigFileOnDemand( sal_False, sal_False );
            CreateTempFile( sal_True );
            GetMedium_Impl();

            if ( pImp->xStream.is() )
            {
                try
                {
                    uno::Reference< io::XTruncate > xTruncate( pImp->xStream, uno::UNO_QUERY_THROW );
                    if ( xTruncate.is() )
                        xTruncate->truncate();

                    xOptStorage->writeAndAttachToStream( pImp->xStream );
                    pImp->xStorage = xStorage;
                    bResult = sal_True;
                }
                catch( uno::Exception& )
                {}
            }

            if ( !bResult )
            {
                Close();
                SetPhysicalName_Impl( String() );
                SetName( aOrigURL );
                GetMedium_Impl();
                pImp->xStorage = xStorage;
            }
        }
    }

    return bResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
