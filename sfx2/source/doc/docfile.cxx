/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docfile.cxx,v $
 *
 *  $Revision: 1.198 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 12:34:12 $
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
#include "precompiled_sfx2.hxx"
#include <sfx2/docfile.hxx>
#include "sfx2/signaturestate.hxx"

#include <uno/mapping.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/document/XDocumentRevisionListPersistence.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_USEBACKUPEXCEPTION_HPP_
#include <com/sun/star/embed/UseBackupException.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XOPTIMIZEDSTORAGE_HPP_
#include <com/sun/star/embed/XOptimizedStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEIOEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_UNSUPPORTEDDATASINKEXCEPTION_HPP_
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDFAILEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandFailedException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIERFACTORY_HPP_
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPROGRESSHANDLER_HPP_
#include <com/sun/star/ucb/XProgressHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XARCHIVER_HPP_
#include <com/sun/star/util/XArchiver.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XTRUNCATE_HPP_
#include <com/sun/star/io/XTruncate.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSTREAMLISTENER_HPP_
#include <com/sun/star/io/XStreamListener.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASHEXCEPTION_HPP_
#include <com/sun/star/ucb/NameClashException.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SECURITY_DOCUMENTSIGNATURESINFORMATION_HPP_
#include <com/sun/star/security/DocumentSignatureInformation.hpp>
#endif
#ifndef _COM_SUN_STAR_SECURITY_XDOCUMENTDIGITALSIGNATURES_HPP_
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#endif

#ifndef _ZCODEC_HXX
#include <tools/zcodec.hxx>
#endif
#ifndef _CACHESTR_HXX //autogen
#include <tools/cachestr.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef  _UNOTOOLS_STREAMHELPER_HXX_
#include <unotools/streamhelper.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#include <svtools/lckbitem.hxx>
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#define _SVSTDARR_ULONGS
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

#include <unotools/streamwrap.hxx>

#include <rtl/logfile.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;

#include <comphelper/storagehelper.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <tools/urlobj.hxx>
#include <tools/inetmime.hxx>
#include <unotools/ucblockbytes.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/asynclink.hxx>
#include <svtools/inettype.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/progresshandlerwrap.hxx>
#include <ucbhelper/content.hxx>
#include <sot/stg.hxx>
#include <svtools/saveopt.hxx>

#include "opostponedtruncationstream.hxx"
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

//#include "xmlversion.hxx"

#define MAX_REDIRECT 5

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

SfxMediumHandler_Impl::~SfxMediumHandler_Impl()
{
}

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

class SfxPoolCancelManager_Impl  :   public SfxCancelManager ,
                                     public SfxCancellable   ,
                                     public SfxListener      ,
                                     public SvRefBase
{
    SfxCancelManagerWeak wParent;

                 ~SfxPoolCancelManager_Impl();
public:
                 SfxPoolCancelManager_Impl( SfxCancelManager* pParent, const String& rName );

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    using SfxCancelManager::Cancel;
    virtual void Cancel();
};

SV_DECL_IMPL_REF( SfxPoolCancelManager_Impl )


//----------------------------------------------------------------
SfxPoolCancelManager_Impl::SfxPoolCancelManager_Impl( SfxCancelManager* pParent, const String& rName )
    : SfxCancelManager( pParent ),
      SfxCancellable( pParent ? pParent : this, rName ),
      wParent( pParent )
{
    if( pParent )
    {
        StartListening( *this );
        SetManager( 0 );
    }
}

//----------------------------------------------------------------
SfxPoolCancelManager_Impl::~SfxPoolCancelManager_Impl()
{
    for( sal_uInt16 nPos = GetCancellableCount(); nPos--; )
    {
        // nicht an Parent uebernehmen!
        SfxCancellable* pCbl = GetCancellable( nPos );
        if ( pCbl )
            pCbl->SetManager( 0 );
    }
}


//----------------------------------------------------------------
void SfxPoolCancelManager_Impl::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& /*rHint*/ )
{
    if( !GetCancellableCount() ) SetManager( 0 );
    else if( !GetManager() )
    {
        if( !wParent.Is() ) wParent = SFX_APP()->GetCancelManager();
        SetManager( wParent );
    }
}

//----------------------------------------------------------------
void SfxPoolCancelManager_Impl::Cancel()
{
    SfxPoolCancelManager_ImplRef xThis = this;
    for( sal_uInt16 nPos = GetCancellableCount(); nPos--; )
    {
        SfxCancellable* pCbl = GetCancellable( nPos );
        // Wenn wir nicht im Button stehen
        if( pCbl && pCbl != this )
            pCbl->Cancel();
        if( GetCancellableCount() < nPos )
            nPos = GetCancellableCount();
    }
}

//----------------------------------------------------------------
class SfxMedium_Impl : public SvCompatWeakBase
{
public:
    ::ucbhelper::Content aContent;
    sal_Bool bUpdatePickList : 1;
    sal_Bool bIsTemp        : 1;
    sal_Bool bForceSynchron : 1;
    sal_Bool bDontCreateCancellable : 1;
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
    uno::Reference < embed::XStorage > xStorage;

    SfxPoolCancelManager_ImplRef xCancelManager;
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

    ::utl::TempFile*           pTempDir;
    ::utl::TempFile*           pTempFile;

    uno::Reference < embed::XStorage > m_xReadStorage;
    Reference < XInputStream > xInputStream;
    Reference < XStream > xStream;

    sal_uInt32                  nLastStorageError;
    ::rtl::OUString             aCharset;

    ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > xInteraction;

    sal_Bool        m_bRemoveBackup;
    ::rtl::OUString m_aBackupURL;

    // the following member is changed and makes sence only during saving
    // TODO/LATER: in future the signature state should be controlled by the medium not by the document
    //             in this case the member will hold this information
    sal_uInt16      m_nSignatureState;

    SfxPoolCancelManager_Impl* GetCancelManager();

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
    SetError( ERRCODE_IO_GENERAL );
}

SfxPoolCancelManager_Impl* SfxMedium_Impl::GetCancelManager()
{
    if( !xCancelManager.Is() )
    {
        if( !bDontCreateCancellable )
            xCancelManager = new SfxPoolCancelManager_Impl(
                wLoadTargetFrame ? wLoadTargetFrame->GetCancelManager() :
                SFX_APP()->GetCancelManager(),
                pAntiImpl->GetURLObject().GetURLNoPass() );
        else
            xCancelManager = new SfxPoolCancelManager_Impl(
                0, pAntiImpl->GetURLObject().GetURLNoPass() );
    }
    return xCancelManager;
}

//------------------------------------------------------------------
SfxMedium_Impl::SfxMedium_Impl( SfxMedium* pAntiImplP )
 :  SvCompatWeakBase( pAntiImplP ),
    bUpdatePickList(sal_True),
    bIsTemp( sal_False ),
    bForceSynchron( sal_False ),
    bDontCreateCancellable( sal_False ),
    bDownloadDone( sal_True ),
    bDontCallDoneLinkOnSharingError( sal_False ),
    bIsStorage( sal_False ),
    bUseInteractionHandler( sal_True ),
    bAllowDefaultIntHdl( sal_False ),
    bIsCharsetInitialized( sal_False ),
    bStorageBasedOnInStream( sal_False ),
    m_bSalvageMode( sal_False ),
    m_bVersionsAlreadyLoaded( sal_False ),
    pAntiImpl( pAntiImplP ),
    nFileVersion( 0 ),
    pOrigFilter( 0 ),
    aExpireTime( Date() + 10, Time() ),
    pTempDir( NULL ),
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

    if ( pTempDir )
        delete pTempDir;
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
/*
const SvGlobalName&  SfxMedium::GetClassFilter()
{
    GetMedium_Impl();
    if( GetError() )
        return aFilterClass;
    if( !bSetFilter && GetStorage() )
        SetClassFilter( GetStorage()->GetClassName() );
    return aFilterClass;
}*/

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
long SfxMedium::GetFileVersion() const
{
    if ( !pImp->nFileVersion && pFilter )
        return pFilter->GetVersion();
    else
        return pImp->nFileVersion;
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
            // TODO: DBG_ERROR("SfxMedium::GetContent()\nCreate Content? This code exists as fallback only. Please clarify, why its used.");
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
        if( bIsRemote && !aOpt.IsSaveRelINet() || !bRemote && !aOpt.IsSaveRelFSys() )
            return ::rtl::OUString();
    }

    return aBaseURL;
}

//------------------------------------------------------------------
SvStream* SfxMedium::GetInStream()
{
    if ( pInStream )
        return pInStream;

    if ( pImp->pTempFile || pImp->pTempDir )
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
        CreateTempFile();
        return;
    }

    DELETEZ( pInStream );
    if ( pSet )
        pSet->ClearItem( SID_INPUTSTREAM );

    CloseReadStorage_Impl();
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
        if ( !pImp->pTempFile )
            CreateTempFile();

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
            //const SvStream *pStorage = aStorage->GetSvStream();
            //if ( pStorage == pOutStream )
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
        if ( !pImp->pTempFile )
            CreateTempFile();
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

    return GetError() == SVSTREAM_OK;
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
            DBG_ERROR("Physical name not convertable!");
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
sal_Bool SfxMedium::TryStorage()
{
    GetStorage();

    if ( pImp->xStorage.is() )
        return sal_True;

    // this code will be removed when binary filter components are available!
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xSMgr( ::comphelper::getProcessServiceFactory() );
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XArchiver >
            xPacker( xSMgr->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.util.Archiver" ) ), ::com::sun::star::uno::UNO_QUERY );

    if( !xPacker.is() )
        return sal_False;

    // extract extra data
    ::rtl::OUString aPath = GetURLObject().PathToFileName();
    ::rtl::OUString aExtraData = xPacker->getExtraData( aPath );
    const ::rtl::OUString aSig1( DEFINE_CONST_UNICODE( "private:" ) );
    String aTmp( '?' );
    aTmp += String::CreateFromAscii("simpress");//pFilter->GetFilterContainer()->GetName();
    const ::rtl::OUString aSig2( aTmp );
    sal_Int32 nIndex1 = aExtraData.indexOf( aSig1 );
    sal_Int32 nIndex2 = aExtraData.indexOf( aSig2 );

    if( nIndex1 != 0 || nIndex2 == -1 )
        return sal_False;

    nIndex1 += aSig1.getLength();
    ::rtl::OUString aTempDoku = aExtraData.copy( nIndex1, nIndex2 - nIndex1 );

    // create a temp dir to unpack to
    pImp->pTempDir = new ::utl::TempFile( NULL, sal_True );
    pImp->pTempDir->EnableKillingFile( sal_True );

    // unpack all files to temp dir
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs;
    com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > xInteractionHandler = GetInteractionHandler();
    if (xInteractionHandler.is())
    {
        aArgs.realloc(1);
        aArgs.getArray()[0].Name = DEFINE_CONST_UNICODE( "InteractionHandler" );
        aArgs.getArray()[0].Value <<= xInteractionHandler ;
    }
    ::com::sun::star::uno::Sequence< ::rtl::OUString > files(0);

    if( !xPacker->unpack( pImp->pTempDir->GetURL(), aPath, files, aArgs ) )
        return sal_False;

    String aNewName = pImp->pTempDir->GetURL();
    aNewName += '/';
    aNewName += String( aTempDoku );
    CloseInStream_Impl();
    String aTemp;
    ::utl::LocalFileHelper::ConvertURLToPhysicalName( aNewName, aTemp );
    SetPhysicalName_Impl( aTemp );
    GetStorage();

    return pImp->xStorage.is();
}

//------------------------------------------------------------------
sal_Bool SfxMedium::BasedOnOriginalFile_Impl()
{
    return ( !pImp->pTempFile && !( aLogicName.Len() && pImp->m_bSalvageMode )
      && GetURLObject().GetMainURL( INetURLObject::NO_DECODE ).getLength()
      && ::utl::LocalFileHelper::IsLocalFile( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) )
      && ::utl::UCBContentHelper::IsDocument( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) ) );
}

//------------------------------------------------------------------
void SfxMedium::StorageBackup_Impl()
{
    ::ucbhelper::Content aOriginalContent;
    Reference< ::com::sun::star::ucb::XCommandEnvironment > xDummyEnv;
    if ( BasedOnOriginalFile_Impl() && !pImp->m_aBackupURL.getLength()
      && ::ucbhelper::Content::create( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ), xDummyEnv, aOriginalContent ) )
    {
        DoInternalBackup_Impl( aOriginalContent );
        if( !pImp->m_aBackupURL.getLength() )
            SetError( ERRCODE_SFX_CANTCREATEBACKUP );
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
::rtl::OUString SfxMedium::GetOutputStorageURL_Impl()
{
    String aStorageName;

    if ( aName.Len() )
    {
        if ( !::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aStorageName ) )
        {
            DBG_ERROR("Physical name not convertable!");
        }
    }
    else
    {
        aStorageName = GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
    }

    return aStorageName;
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

    // medium based on OutputStream: must work with TempFile
    if( aLogicName.CompareToAscii( "private:stream", 14 ) == COMPARE_EQUAL
      || !::utl::LocalFileHelper::IsLocalFile( aLogicName ) )
        CreateTempFileNoCopy();
    // if Medium already contains a stream - TODO/LATER: store stream/outputstream in ImplData, not in Medium
    else if ( GetItemSet()->GetItemState( SID_STREAM ) < SFX_ITEM_SET )
    {
        // check whether the backup should be created
        StorageBackup_Impl();

        if ( GetError() )
            return uno::Reference< embed::XStorage >();

        // the target file must be truncated before a storage based on it is created
        try {
               uno::Reference< lang::XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
                  uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > xSimpleFileAccess(
                    xFactory->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.ucb.SimpleFileAccess") ),
                    uno::UNO_QUERY_THROW );

            SFX_ITEMSET_ARG( GetItemSet(), pOverWrite, SfxBoolItem, SID_OVERWRITE, sal_False );
               SFX_ITEMSET_ARG( GetItemSet(), pRename, SfxBoolItem, SID_RENAME, sal_False );
            sal_Bool bRename = pRename ? pRename->GetValue() : FALSE;
            sal_Bool bOverWrite = pOverWrite ? pOverWrite->GetValue() : !bRename;

            ::rtl::OUString aOutputURL = GetOutputStorageURL_Impl();

            // TODO/LATER: nonatomar operation
            sal_Bool bExists = xSimpleFileAccess->exists( aOutputURL );
            if ( !bOverWrite && bExists )
                throw ::com::sun::star::ucb::NameClashException();

            uno::Reference< io::XStream > xStream;

            if ( BasedOnOriginalFile_Impl() )
            {
                // the storage will be based on original file, the wrapper should be used
                xStream = new OPostponedTruncationFileStream( aOutputURL, xFactory, xSimpleFileAccess, sal_True );
            }
            else
            {
                // the storage will be based on the temporary file, the stream can be truncated directly
                xStream = xSimpleFileAccess->openFileReadWrite( aOutputURL );
                uno::Reference< io::XOutputStream > xOutStream = xStream->getOutputStream();
                uno::Reference< io::XTruncate > xTruncate( xOutStream, uno::UNO_QUERY );
                if ( !xTruncate.is() )
                    throw uno::RuntimeException();

                xTruncate->truncate();
                xOutStream->flush();
            }

            pImp->xStream = xStream;
               GetItemSet()->Put( SfxUsrAnyItem( SID_STREAM, makeAny( xStream ) ) );
        }
        catch( uno::Exception& )
        {
            DBG_ERROR( "Can't truncate target stream!\n" );
               SetError( ERRCODE_IO_GENERAL );
        }
    }

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
                // SetError( ERRCODE_IO_GENERAL );
            }
        }
    }
}

//------------------------------------------------------------------
uno::Reference < embed::XStorage > SfxMedium::GetStorage()
{
    if ( pImp->xStorage.is() || bTriedStorage )
        return pImp->xStorage;

    uno::Sequence< uno::Any > aArgs( 2 );

    String aStorageName;
    if ( pImp->pTempFile || pImp->pTempDir )
    {
        // open storage from the temporary file
        if ( !::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aStorageName ) )
        {
            DBG_ERROR("Physical name not convertable!");
        }

        CloseOutStream();
        // create the set of the streams based on the temporary file
        GetMedium_Impl();

        OSL_ENSURE( pImp->xStream.is(), "It must be possible to create read write stream access!" );
        if ( pImp->xStream.is() )
        {
            aArgs[0] <<= pImp->xStream;
               pImp->bStorageBasedOnInStream = sal_True;
        }
        else
        {
            CloseStreams_Impl();
            aArgs[0] <<= ::rtl::OUString( aName );
               pImp->bStorageBasedOnInStream = sal_False;
        }

        aArgs[1] <<= ( nStorOpenMode&STREAM_WRITE ? embed::ElementModes::READWRITE : embed::ElementModes::READ );

        try
        {
            pImp->xStorage = uno::Reference< embed::XStorage >(
                                ::comphelper::OStorageHelper::GetStorageFactory()->createInstanceWithArguments( aArgs ),
                                uno::UNO_QUERY );
        }
        catch( uno::Exception& )
        {
            //TODO/LATER: error handling; Error and LastStorageError
        }
    }
    else
    {
        // open the storage from original location
        {
            GetMedium_Impl();
            if ( GetError() )
                return pImp->xStorage;

            try
            {
                if ( IsReadOnly() && ::utl::LocalFileHelper::IsLocalFile( aLogicName ) )
                {
                    //TODO/LATER: performance problem if not controlled by special Mode in SfxMedium
                    //(should be done only for permanently open storages)
                    // create a copy, the following method will close all existing streams
                    CreateTempFile();

                    // create the set of the streams based on the temporary file
                    GetMedium_Impl();

                    OSL_ENSURE( pImp->xStream.is(), "It must be possible to create read write stream access!" );
                    if ( pImp->xStream.is() )
                    {
                        aArgs[0] <<= pImp->xStream;
                        pImp->bStorageBasedOnInStream = sal_True;
                    }
                    else
                    {
                        CloseStreams_Impl();
                        aArgs[0] <<= ::rtl::OUString( aName );
                        pImp->bStorageBasedOnInStream = sal_False;
                    }

                    aArgs[1] <<= embed::ElementModes::READWRITE;

                }
                else
                {
                    // there is no explicit request to open the document readonly

                    // create a storage on the stream
                    if ( pImp->xStream.is() )
                    {
                        aArgs[0] <<= pImp->xStream;
                        aArgs[1] <<= ( ( nStorOpenMode & STREAM_WRITE ) ?
                                        embed::ElementModes::READWRITE : embed::ElementModes::READ );

                        pImp->bStorageBasedOnInStream = sal_True;
                    }
                    else
                    {
                        // no readwrite stream, but it can be a case of http protocol
                        sal_Bool bReadOnly = sal_False;

                        if ( aLogicName.CompareToAscii( "private:stream", 14 ) != COMPARE_EQUAL
                          && GetContent().is() )
                        {
                            // unfortunately the content can not always have the interaction handler
                            // so in some cases it has to be set for some time
                            Reference < ::com::sun::star::ucb::XCommandEnvironment > xEnv;
                            Reference < ::com::sun::star::ucb::XCommandEnvironment > xOldEnv;
                               Reference < ::com::sun::star::task::XInteractionHandler > xInteractionHandler = ((SfxMedium*)this)->GetInteractionHandler();
                            if ( xInteractionHandler.is() )
                                xEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler,
                                                      Reference< ::com::sun::star::ucb::XProgressHandler >() );

                            if ( xEnv.is() )
                            {
                                xOldEnv = pImp->aContent.getCommandEnvironment();
                                pImp->aContent.setCommandEnvironment( xEnv );
                            }

                            try
                            {
                                Any aAny = pImp->aContent.getPropertyValue(
                                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsReadOnly" )) );

                                if ( ( aAny >>= bReadOnly ) && bReadOnly )
                                {
                                    GetItemSet()->Put( SfxBoolItem(SID_DOC_READONLY, sal_True));
                                    SetOpenMode( SFX_STREAM_READONLY, sal_False, sal_True );
                                }
                            }
                            catch( uno::Exception& )
                            {}

                            if ( xEnv.is() )
                                pImp->aContent.setCommandEnvironment( xOldEnv );
                        }

                        // if the document is opened as readonly the copy should be done according to selected approach
                        // if the document is opened for editing the copy should be done to use it as a temporary location for changes before the final transfer
                        // the following method will close all existing streams
                           CreateTempFile();

                        // create the set of the streams based on the temporary file
                        GetMedium_Impl();

                        OSL_ENSURE( pImp->xStream.is(), "It must be possible to create read write stream access!" );
                        if ( pImp->xStream.is() )
                        {
                            aArgs[0] <<= pImp->xStream;
                            pImp->bStorageBasedOnInStream = sal_True;
                        }
                        else
                        {
                            CloseStreams_Impl();
                            aArgs[0] <<= ::rtl::OUString( aName );
                            pImp->bStorageBasedOnInStream = sal_False;
                        }

                        if ( bReadOnly )
                            aArgs[1] <<= embed::ElementModes::READ;
                        else
                            aArgs[1] <<= embed::ElementModes::READWRITE;
                    }
                }

                   SFX_ITEMSET_ARG( GetItemSet(), pRepairItem, SfxBoolItem, SID_REPAIRPACKAGE, sal_False);
                   if ( pRepairItem && pRepairItem->GetValue() )
                {
                    // the storage should be created for repairing mode
                    CreateTempFile();
                    Reference< ::com::sun::star::ucb::XProgressHandler > xProgressHandler;
                    Reference< ::com::sun::star::task::XStatusIndicator > xStatusIndicator;

                    SFX_ITEMSET_ARG( GetItemSet(), pxProgressItem, SfxUnoAnyItem, SID_PROGRESS_STATUSBAR_CONTROL, sal_False );
                    if( pxProgressItem && ( pxProgressItem->GetValue() >>= xStatusIndicator ) )
                        xProgressHandler = Reference< ::com::sun::star::ucb::XProgressHandler >(
                                                new utl::ProgressHandlerWrap( xStatusIndicator ) );

                    uno::Sequence< beans::PropertyValue > aAddProps( 2 );
                    aAddProps[0].Name = ::rtl::OUString::createFromAscii( "RepairPackage" );
                    aAddProps[0].Value <<= (sal_Bool)sal_True;
                    aAddProps[1].Name = ::rtl::OUString::createFromAscii( "StatusIndicator" );
                    aAddProps[1].Value <<= xProgressHandler;

                    aArgs.realloc( 3 );
                    aArgs[0] <<= ::rtl::OUString( aName );
                    aArgs[1] <<= embed::ElementModes::READWRITE;
                    aArgs[2] <<= aAddProps;

                    pImp->bStorageBasedOnInStream = sal_False;
                }

                pImp->xStorage = uno::Reference< embed::XStorage >(
                                    ::comphelper::OStorageHelper::GetStorageFactory()->createInstanceWithArguments( aArgs ),
                                    uno::UNO_QUERY );

                if ( !pImp->xStorage.is() )
                    throw uno::RuntimeException();

                if ( pRepairItem && pRepairItem->GetValue() )
                {
                    // in repairing mode the mediatype required by filter should be used
                    ::rtl::OUString aMediaType;
                    ::rtl::OUString aMediaTypePropName( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) );
                       uno::Reference < beans::XPropertySet > xPropSet( pImp->xStorage, uno::UNO_QUERY_THROW );
                    xPropSet->getPropertyValue( aMediaTypePropName ) >>= aMediaType;
                    if ( !aMediaType.getLength() && pFilter )
                        xPropSet->setPropertyValue( aMediaTypePropName,
                                                    uno::makeAny( ::rtl::OUString( pFilter->GetMimeType() ) ) );
                }
            }
            catch ( uno::Exception& )
            {
                //TODO/MBA: error handling; Error and LastStorageError
                pImp->bStorageBasedOnInStream = sal_False;
            }
        }
    }

    if( ( pImp->nLastStorageError = GetError() ) != SVSTREAM_OK )
    {
        pImp->xStorage = 0;
        if ( pInStream )
            pInStream->Seek(0);
        return NULL;
    }

    bTriedStorage = sal_True;

    //TODO/MBA: error handling; Error and LastStorageError
    //if ( aStorage->GetError() == SVSTREAM_OK )
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

    //TODO/MBA: error handling; Error and LastStorageError
    if ( pImp->xStorage.is() )
    {   /*
        if( ( pImp->nLastStorageError = aStorage->GetError() ) != SVSTREAM_OK )
            bResetStorage = TRUE;
        else if ( GetFilter() )
            aStorage->SetVersion( GetFilter()->GetVersion() );*/
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
uno::Reference< embed::XStorage > SfxMedium::GetLastCommitReadStorage_Impl()
{
    if ( !GetError() && !pImp->m_xReadStorage.is() )
    {
        GetMedium_Impl();

        try
        {
            if ( pImp->xInputStream.is() )
            {
                uno::Sequence< uno::Any > aArgs( 2 );
                aArgs[0] <<= pImp->xInputStream;
                aArgs[1] <<= embed::ElementModes::READ;
                pImp->m_xReadStorage = uno::Reference< embed::XStorage >(
                                    ::comphelper::OStorageHelper::GetStorageFactory()->createInstanceWithArguments( aArgs ),
                                    uno::UNO_QUERY );
            }
            else if ( GetStorage().is() )
            {
                uno::Reference< embed::XStorage > xTempStor = ::comphelper::OStorageHelper::GetTemporaryStorage();
                GetStorage()->copyLastCommitTo( xTempStor );
                pImp->m_xReadStorage = xTempStor;
            }
        }
        catch( uno::Exception& )
        {
            OSL_ENSURE( sal_False, "No possibility to get readonly version of storage from medium!\n" );
        }

        if ( GetError() ) // do not remove warnings
            ResetError();
    }

    return pImp->m_xReadStorage;
}

//------------------------------------------------------------------
void SfxMedium::CloseReadStorage_Impl()
{
    if ( pImp->m_xReadStorage.is() )
    {
        try {
            pImp->m_xReadStorage->dispose();
        } catch( uno::Exception& )
        {}

        pImp->m_xReadStorage = uno::Reference< embed::XStorage >();
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
            Close();
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
        StorageBackup_Impl();

        if ( !GetError() )
        {
            uno::Reference < embed::XTransactedObject > xTrans( pImp->xStorage, uno::UNO_QUERY );
            if ( xTrans.is() )
            {
                try
                {
                    xTrans->commit();
                    CloseReadStorage_Impl();
                    bResult = sal_True;
                }
                catch ( embed::UseBackupException& aBackupExc )
                {
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
                            SetError( ERRCODE_IO_GENERAL );
                    }
                }
                catch ( uno::Exception& )
                {
                    //TODO/LATER: improve error handling
                    SetError( ERRCODE_IO_GENERAL );
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
    Reference< XSimpleFileAccess > aSimpleAccess;
    ::ucbhelper::Content aOriginalContent;

//  actualy it should work even for contents different from file content
//  DBG_ASSERT( ::utl::LocalFileHelper::IsLocalFile( aDest.GetMainURL( INetURLObject::NO_DECODE ) ),
//              "SfxMedium::TransactedTransferForFS() should be used only for local contents!" );
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
        Close();
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
                        aOriginalContent.setPropertyValue( ::rtl::OUString::createFromAscii( "Size" ),
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
                       if ( pOverWrite && !pOverWrite->GetValue() // argument says: never overwrite
                         || pRename && pRename->GetValue() ) // argument says: rename file
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
                // write directly to the stream
                Close();

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
                DBG_ERROR( "Illegal Output stream parameter!\n" );
                SetError( ERRCODE_IO_GENERAL );
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
                //if ( !GetError() )
                  //  SetError( xStor->GetError() );
            }
            return;
        }

        if ( pFilter && SOFFICE_FILEFORMAT_60 <= pFilter->GetVersion() )
        {
            //TODO/LATER: how?!
            /*
            SFX_ITEMSET_ARG( GetItemSet(), pItem, SfxBoolItem, SID_UNPACK, sal_False);
            if ( pItem && pItem->GetValue() )
            {
                // this file must be stored without packing into a JAR file
                // check for an existing unpacked storage
                SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( GetName(), STREAM_STD_READ );
                if ( !pStream->GetError() )
                {
                    String aURL = UCBStorage::GetLinkedFile( *pStream );
                    if ( aURL.Len() )
                        // remove a possibly existing old folder
                        ::utl::UCBContentHelper::Kill( aURL );

                    DELETEZ( pStream );
                }

                // create a new folder based storage
                SvStorageRef xStor = new SvStorage( TRUE, GetName(), STREAM_STD_READWRITE, STORAGE_CREATE_UNPACKED );

                // copy package into unpacked storage
                if ( xStor->GetError() == ERRCODE_NONE && GetStorage()->copyToStorage( xStor ) )
                {
                    // commit changes, writing will happen now
                    xStor->Commit();

                    // take new unpacked storage as own storage
                    Close();
                    DELETEZ( pImp->pTempFile );
                    ::utl::LocalFileHelper::ConvertURLToPhysicalName( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ), aName );
                    SetStorage_Impl( xStor );
                }
                else if ( !GetError() )
                    SetError( xStor->GetError() );
                return;
            }*/
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
                Close();
                // don't create content before Close(), because if the storage was opened in direct mode, it will be flushed
                // in Close() and this leads to a transfer command executed in the package, which currently is implemented as
                // remove+move in the file FCP. The "remove" is notified to the ::ucbhelper::Content, that clears its URL and its
                // content reference in this notification and thus will never get back any URL, so my transfer will fail!
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
              // || !::utl::UCBContentHelper::IsDocument( pImp->m_aBackupURL ) );
            {
                pImp->m_bRemoveBackup = sal_False;
                pImp->m_aBackupURL = ::rtl::OUString();
            }
            else
            {

                DBG_ERROR("Couldn't remove backup file!");
            }
        }
    }
    else
        pImp->m_aBackupURL = ::rtl::OUString();
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
                    DBG_ERROR("Physical name not convertable!");
                }
            }
            else
                aFileName = GetName();

            // in case the temporary file exists the streams should be initialized from it,
            // but the original MediaDescriptor should not be changed
            sal_Bool bFromTempFile = ( pImp->pTempFile || pImp->pTempDir );

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

                // m_xInputStreamToLoadFrom = 0;
            }
            else
            {
                TransformItems( SID_OPENDOC, *GetItemSet(), xProps );
                comphelper::MediaDescriptor aMedium( xProps );

                if ( bFromTempFile )
                {
                    aMedium[comphelper::MediaDescriptor::PROP_URL()] <<= ::rtl::OUString( aFileName );
                    aMedium.erase( comphelper::MediaDescriptor::PROP_READONLY() );
                }

                aMedium.addInputStream();
                sal_Bool bReadOnly = aMedium.isStreamReadOnly();
                if ( bReadOnly && !bFromTempFile )
                    GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, sal_True ) );

                //TODO/MBA: what happens if property is not there?!
                GetContent();
                aMedium[comphelper::MediaDescriptor::PROP_STREAM()] >>= pImp->xStream;
                aMedium[comphelper::MediaDescriptor::PROP_INPUTSTREAM()] >>= pImp->xInputStream;
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
            SetError( ERRCODE_IO_ACCESSDENIED );

        if ( !GetError() )
        {
            if ( pImp->xStream.is() )
                pInStream = utl::UcbStreamHelper::CreateStream( pImp->xStream );
            else if ( pImp->xInputStream.is() )
                pInStream = utl::UcbStreamHelper::CreateStream( pImp->xInputStream );
        }

        pImp->bDownloadDone = sal_True;
        pImp->aDoneLink.ClearPendingCall();
        pImp->aDoneLink.Call( (void*) GetError() );
    }
}

//------------------------------------------------------------------
SfxPoolCancelManager_Impl* SfxMedium::GetCancelManager_Impl() const
{
    return pImp->GetCancelManager();
}

//------------------------------------------------------------------
void SfxMedium::SetCancelManager_Impl( SfxPoolCancelManager_Impl* pMgr )
{
    pImp->xCancelManager = pMgr;
}

//----------------------------------------------------------------
void SfxMedium::CancelTransfers()
{
    if( pImp->xCancelManager.Is() )
        pImp->xCancelManager->Cancel();
}

//----------------------------------------------------------------
/*
String SfxMedium::GetStatusString( const SvProgressArg* pArg )
{
    String aString;
    StringList_Impl aSL( SfxResId( RID_DLSTATUS2 ), (USHORT)pArg->eStatus );
    USHORT nTotal = 0;

    if ( pArg->eStatus == SVBINDSTATUS_ENDDOWNLOADDATA && nTotal <= 1 )
        return aString;

    if( aSL )
    {
        INetURLObject aObj( pArg->rStatus );
        aString = aSL.GetString();
        aString.SearchAndReplaceAscii( "$(HOST)", aObj.GetHost() );
        String aTarget = aObj.GetFull();
        if( aTarget.Len() <= 1 && pArg->eStatus != SVBINDSTATUS_CONNECTING )
            aTarget = aObj.GetHost();
        if( pArg->nMax )
        {
            aTarget += DEFINE_CONST_UNICODE( " (" );
            AddNumber_Impl( aTarget, pArg->nMax );
            aTarget += ')';
        }

        aString.SearchAndReplaceAscii( "$(TARGET)",aTarget );
        String aNumber;
        AddNumber_Impl( aNumber, pArg->nProgress );
        if( pArg->nRate )
        {
            aNumber+= DEFINE_CONST_UNICODE( " (" );
            AddNumber_Impl( aNumber, (ULONG)pArg->nRate );
            aNumber+= DEFINE_CONST_UNICODE( "/s)" );
        }
        if( pArg->nMax && pArg->nProgress && pArg->nMax != pArg->nProgress )
        {
            aNumber += DEFINE_CONST_UNICODE( " [" );
            float aPerc = pArg->nProgress / (float)pArg->nMax;
            aNumber += String::CreateFromInt32( (USHORT)(aPerc * 100) );
            aNumber += DEFINE_CONST_UNICODE( "%]" );
        }
        aString.SearchAndReplaceAscii( "$(BYTE)", aNumber );
    }
    return aString;
}
*/

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
            DBG_ERROR ( "Unknown protocol!" );
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
            else
                DBG_ASSERT( pSalvageItem, "Suspicious change of logical name!" );
        }
    }

    if ( pSalvageItem && pSalvageItem->GetValue().Len() )
    {
        aLogicName = pSalvageItem->GetValue();
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
        DBG_ERROR( "Unexpected Output stream parameter!\n" );
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
        CreateTempFile();
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

void SfxMedium::Close()
{
    if ( pImp->xStorage.is() )
    {
        // don't close the streams if they belong to the
        // storage
        //TODO/MBA: how to?! Do we need the flag?!
        /*
        const SvStream *pStream = aStorage->GetSvStream();
        if ( pStream && pStream == pInStream )
        {
            CloseReadStorage_Impl();
            pInStream = NULL;
            pImp->xInputStream = Reference < XInputStream >();
            pImp->xLockBytes.Clear();
            if ( pSet )
                pSet->ClearItem( SID_INPUTSTREAM );
            aStorage->SetDeleteStream( TRUE );
        }
        else if ( pStream && pStream == pOutStream )
        {
            pOutStream = NULL;
            aStorage->SetDeleteStream( TRUE );
        } */

        CloseStorage();
    }

    CloseStreams_Impl();
}

void SfxMedium::CloseAndRelease()
{
    if ( pImp->xStorage.is() )
    {
        // don't close the streams if they belong to the
        // storage
        //TODO/MBA: how to?! Do we need the flag?!
        /*
        const SvStream *pStream = aStorage->GetSvStream();
        if ( pStream && pStream == pInStream )
        {
            CloseReadStorage_Impl();
            pInStream = NULL;
            pImp->xInputStream = Reference < XInputStream >();
            pImp->xLockBytes.Clear();
            if ( pSet )
                pSet->ClearItem( SID_INPUTSTREAM );
            aStorage->SetDeleteStream( TRUE );
        }
        else if ( pStream && pStream == pOutStream )
        {
            pOutStream = NULL;
            aStorage->SetDeleteStream( TRUE );
        } */

        CloseStorage();
    }

    CloseAndReleaseStreams_Impl();
}

void SfxMedium::CloseAndReleaseStreams_Impl()
{
    CloseReadStorage_Impl();

    uno::Reference< io::XInputStream > xInToClose = pImp->xInputStream;
    uno::Reference< io::XOutputStream > xOutToClose;
    if ( pImp->xStream.is() )
        xOutToClose = pImp->xStream->getOutputStream();

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
#if 0   //(dv)
    if ( pImp->aContent.get().is() )
    {
        String aNameP = pImp->xAnchor->GetViewURL();
        pImp->aOrigURL = aNameP;
        aLogicName = aNameP;
        DELETEZ( pURLObj );
        if (aLogicName.Len())
            aLogicName = GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
        SetIsRemote_Impl();
    }
#endif  //(dv)
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
//        case INET_PROT_OUT:
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

//----------------------------------------------------------------
void SfxMedium::MoveStorageTo_Impl( SfxMedium* pMedium )
{
    if ( pMedium && pMedium != this && pImp->xStorage.is() )
    {
        if( pMedium->pImp->pTempFile )
        {
            pMedium->pImp->pTempFile->EnableKillingFile( sal_True );
            delete pMedium->pImp->pTempFile;
            pMedium->pImp->pTempFile = NULL;
        }

        pMedium->Close();
        pMedium->aName = aName;
        pMedium->pImp->xStorage = pImp->xStorage;

        CanDisposeStorage_Impl( sal_False );
    }
}

//----------------------------------------------------------------
void SfxMedium::MoveTempTo_Impl( SfxMedium* pMedium )
{
    if ( pMedium && pMedium != this && pImp->pTempFile )
    {
        if( pMedium->pImp->pTempFile )
            delete pMedium->pImp->pTempFile;
        pMedium->pImp->pTempFile = pImp->pTempFile;

        pImp->pTempFile->EnableKillingFile( sal_True );
        pImp->pTempFile = NULL;

        pMedium->aName = pMedium->pImp->pTempFile->GetFileName();

        pMedium->CloseInStream();
        pMedium->CloseStorage();
        pMedium->pImp->aContent = ::ucbhelper::Content();
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
    DBG_ERROR( "Not implemented!" );
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

    sal_Bool bSalvage = sal_False;
    SFX_ITEMSET_ARG( pSet, pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False );
    if( pSalvageItem )
    {
        // QUESTION: there is some treatment of Salvage in Init_Impl; align!
        bSalvage = sal_True;
        if ( pSalvageItem->GetValue().Len() )
        {
            // if an URL is provided in SalvageItem that means that the FileName refers to a temporary file
            // that must be copied here

            SFX_ITEMSET_ARG( pSet, pFileNameItem, SfxStringItem, SID_FILE_NAME, FALSE );
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
    /* Attention
        Don't enable CancelTransfers() till you know that the writer/web has changed his asynchronous load
        behaviour. Otherwhise may StyleSheets inside a html file will be loaded at the right time.
        => further the help will be empty then ... #100490#
     */
    //CancelTransfers();

    // if there is a requirement to clean the backup this is the last possibility to do it
    ClearBackup_Impl();

    Close();

    delete pSet;

    if( pImp->bIsTemp && aName.Len() )
    {
        String aTemp;
        if ( !::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aTemp ))
        {
            DBG_ERROR("Physical name not convertable!");
        }

        if ( !::utl::UCBContentHelper::Kill( aTemp ) )
        {
            DBG_ERROR("Couldn't remove temporary file!");
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
//------------------------------------------------------------------

void SfxMedium::SetClassFilter( const SvGlobalName & rFilterClass )
{
    bSetFilter = sal_True;
    aFilterClass = rFilterClass;
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

sal_uInt32 SfxMedium::GetMIMEAndRedirect( String& /*rName*/ )
{
/* dv !!!! not needed any longer ?
    INetProtocol eProt = GetURLObject().GetProtocol();
    if( eProt == INET_PROT_FTP && SvBinding::ShouldUseFtpProxy( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) ) )
    {
        Any aAny( UCB_Helper::GetProperty( GetContent(), WID_FLAG_IS_FOLDER ) );
        sal_Bool bIsFolder = FALSE;
        if ( ( aAny >>= bIsFolder ) && bIsFolder )
            return ERRCODE_NONE;
    }

    GetMedium_Impl();
    if( !eError && pImp->xBinding.Is() )
    {
        eError = pImp->xBinding->GetMimeType( rName );

        // Wir koennen keine Parameter wie CharSets usw.
        rName = rName.GetToken( 0, ';' );
        if( !eError )
        {
            if( !pImp->aPreRedirectionURL.Len() )
                pImp->aPreRedirectionURL = aLogicName;
            SetName( pImp->xBinding->GetRedirectedURL() );
        }
        pImp->aExpireTime = pImp->xBinding->GetExpireDateTime();
    }
    return eError;
*/
    return 0;
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
    if( !pSet ) ((SfxMedium*)this)->pSet =
                    new SfxAllItemSet( SFX_APP()->GetPool() );
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
                Any aAny = pImp->aContent.getPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ) );
                ::rtl::OUString aContentType;
                aAny >>= aContentType;

                pImp->xAttributes->Append( SvKeyValue( ::rtl::OUString::createFromAscii( "content-type" ), aContentType ) );
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

//----------------------------------------------------------------

void SfxMedium::SetDontCreateCancellable( )
{
    pImp->bDontCreateCancellable = sal_True;
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
                ::rtl::OUString::createFromAscii("com.sun.star.document.DocumentRevisionListPersistence") ), uno::UNO_QUERY );
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
            ::rtl::OUString::createFromAscii("com.sun.star.document.DocumentRevisionListPersistence") ), uno::UNO_QUERY );
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
                ::rtl::OUString::createFromAscii("com.sun.star.document.DocumentRevisionListPersistence") ), uno::UNO_QUERY );
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
void SfxMedium::TryToSwitchToRepairedTemp()
{
    // the medium should be opened in repair mode
    SFX_ITEMSET_ARG( GetItemSet(), pRepairItem, SfxBoolItem, SID_REPAIRPACKAGE, FALSE );
    if ( pRepairItem && pRepairItem->GetValue() )
    {
        DBG_ASSERT( pImp->xStorage.is(), "Possible performance problem" );
        if ( GetStorage().is() )
        {
            ::utl::TempFile* pTmpFile = new ::utl::TempFile();
            pTmpFile->EnableKillingFile( sal_True );
            ::rtl::OUString aNewName = pTmpFile->GetFileName();

            if( aNewName.getLength() )
            {
                try
                {
                    uno::Reference < embed::XStorage > xNewStorage = comphelper::OStorageHelper::GetStorageFromURL( aNewName,
                            embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
                //SvStorageRef aNewStorage = new SvStorage( sal_True, aNewName, STREAM_WRITE | STREAM_TRUNC, STORAGE_TRANSACTED );

                    pImp->xStorage->copyToStorage( xNewStorage );
                    //if ( aNewStorage->GetError() == SVSTREAM_OK )
                    {
                        CloseInStream();
                        CloseStorage();
                        if ( pImp->pTempFile )
                            DELETEZ( pImp->pTempFile );

                        pImp->pTempFile = pTmpFile;
                        aName = aNewName;
                    }
                }
                catch ( uno::Exception& )
                {
                    //TODO/MBA: error handling
                    //SetError( aNewStorage->GetError() );
                }
            }
            else
                SetError( ERRCODE_IO_CANTWRITE );

            if (pImp->pTempFile != pTmpFile)
                delete pTmpFile;
        }
        else
            SetError( ERRCODE_IO_CANTREAD );
    }
}

//----------------------------------------------------------------
void SfxMedium::CreateTempFile()
{
    if ( pImp->pTempFile )
    {
        DELETEZ( pImp->pTempFile );
        aName = String();
    }

    StreamMode nOpenMode = nStorOpenMode;
    BOOL bCopy = ( nStorOpenMode == nOpenMode && ! ( nOpenMode & STREAM_TRUNC ) );
    if ( bCopy && !pInStream )
    {
        if ( GetContent().is() )
        {
            try
            {
                // make sure that the desired file exists before trying to open
                SvMemoryStream aStream(0,0);
                ::utl::OInputStreamWrapper* pInput = new ::utl::OInputStreamWrapper( aStream );
                Reference< XInputStream > xInput( pInput );

                InsertCommandArgument aInsertArg;
                aInsertArg.Data = xInput;

                aInsertArg.ReplaceExisting = sal_False;
                Any aCmdArg;
                aCmdArg <<= aInsertArg;
                pImp->aContent.executeCommand( ::rtl::OUString::createFromAscii( "insert" ), aCmdArg );
            }
            catch ( Exception& )
            {
                // it is NOT an error when the stream already exists!
                GetInStream();
            }
        }
    }

    nStorOpenMode = nOpenMode;
    ResetError();

    pImp->pTempFile = new ::utl::TempFile();
    pImp->pTempFile->EnableKillingFile( sal_True );
    aName = pImp->pTempFile->GetFileName();
    if ( !aName.Len() )
    {
        SetError( ERRCODE_IO_CANTWRITE );
        return;
    }

    if ( bCopy && pInStream )
    {
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

            delete[] pBuf;
            CloseInStream();
        }
        CloseOutStream_Impl();
    }
    else
        CloseInStream();

    CloseStorage();
}

//----------------------------------------------------------------
void SfxMedium::CreateTempFileNoCopy()
{
    if ( pImp->pTempFile )
        delete pImp->pTempFile;

    pImp->pTempFile = new ::utl::TempFile();
    pImp->pTempFile->EnableKillingFile( sal_True );
    aName = pImp->pTempFile->GetFileName();
    if ( !aName.Len() )
    {
        SetError( ERRCODE_IO_CANTWRITE );
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
                Any aAny = pImp->aContent.getPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ) );
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

sal_Bool SfxMedium::SignContents_Impl( sal_Bool bScriptingContent )
{
    DBG_ASSERT( GetStorage().is(), "SfxMedium::SignContents_Impl - Storage doesn't exist!" );

    sal_Bool bChanges = FALSE;

    ::com::sun::star::uno::Reference< ::com::sun::star::security::XDocumentDigitalSignatures > xD(
        comphelper::getProcessServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.security.DocumentDigitalSignatures" ) ) ), ::com::sun::star::uno::UNO_QUERY );

    // TODO/LATER: error handling
    if ( xD.is() && GetStorage().is() )
    {
        sal_Int32 nEncrMode = IsReadOnly() ? embed::ElementModes::READ
                                           : embed::ElementModes::READWRITE;

        try
        {
            uno::Reference< embed::XStorage > xMetaInf = GetStorage()->openStorageElement(
                                                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "META-INF" ) ),
                                                nEncrMode );
            if ( !xMetaInf.is() )
                throw uno::RuntimeException();

               if ( bScriptingContent )
            {
                if ( !IsReadOnly() )
                {
                    uno::Reference< io::XStream > xStream = xMetaInf->openStreamElement(
                                                                    xD->getScriptingContentSignatureDefaultStreamName(),
                                                                    nEncrMode );
                    if ( !xStream.is() )
                        throw uno::RuntimeException();

                    try
                    {
                        // to leave the stream unencrypted as before
                        uno::Reference< beans::XPropertySet > xStrmProps( xStream, uno::UNO_QUERY_THROW );
                        xStrmProps->setPropertyValue(
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UseCommonStoragePasswordEncryption" ) ),
                            uno::makeAny( (sal_Bool)sal_False ) );
                    }
                    catch ( uno::Exception& )
                    {}

                    if ( xD->signScriptingContent( GetLastCommitReadStorage_Impl(), xStream ) )
                    {
                        uno::Reference< embed::XTransactedObject > xTrans( xMetaInf, uno::UNO_QUERY );
                        xTrans->commit();
                        Commit();
                        bChanges = TRUE;
                    }
                }
                else
                    xD->showScriptingContentSignatures( GetLastCommitReadStorage_Impl(), uno::Reference< io::XInputStream >() );
            }
            else
            {
                if ( !IsReadOnly() )
                {
                    uno::Reference< io::XStream > xStream = xMetaInf->openStreamElement(
                                                                    xD->getDocumentContentSignatureDefaultStreamName(),
                                                                    nEncrMode );
                    if ( !xStream.is() )
                        throw uno::RuntimeException();

                    try
                    {
                        // to leave the stream unencrypted as before
                        uno::Reference< beans::XPropertySet > xStrmProps( xStream, uno::UNO_QUERY_THROW );
                        xStrmProps->setPropertyValue(
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UseCommonStoragePasswordEncryption" ) ),
                            uno::makeAny( (sal_Bool)sal_False ) );
                    }
                    catch ( uno::Exception& )
                    {}

                    if ( xD->signDocumentContent( GetLastCommitReadStorage_Impl(), xStream ) )
                    {
                        uno::Reference< embed::XTransactedObject > xTrans( xMetaInf, uno::UNO_QUERY );
                        xTrans->commit();
                        Commit();
                        bChanges = TRUE;
                    }

                }
                else
                    xD->showDocumentContentSignatures( GetLastCommitReadStorage_Impl(), uno::Reference< io::XInputStream >() );
            }
        }
        catch( uno::Exception& )
        {
            OSL_ENSURE( sal_False, "Couldn't use signing functionality!\n" );
        }
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

//----------------------------------------------------------------
sal_Bool SfxMedium::EqualURLs( const ::rtl::OUString& aFirstURL, const ::rtl::OUString& aSecondURL )
{
    sal_Bool bResult = sal_False;

    if ( aFirstURL.getLength() && aSecondURL.getLength() )
    {
        INetURLObject aFirst( aFirstURL );
        INetURLObject aSecond( aSecondURL );

        if ( aFirst.GetProtocol() != INET_PROT_NOT_VALID && aSecond.GetProtocol() != INET_PROT_NOT_VALID )
        {
            try
            {
                ::ucbhelper::ContentBroker* pBroker = ::ucbhelper::ContentBroker::get();
                if ( !pBroker )
                    throw uno::RuntimeException();

                uno::Reference< ::com::sun::star::ucb::XContentIdentifierFactory > xIdFac
                    = pBroker->getContentIdentifierFactoryInterface();
                if ( !xIdFac.is() )
                    throw uno::RuntimeException();

                uno::Reference< ::com::sun::star::ucb::XContentIdentifier > xIdFirst
                    = xIdFac->createContentIdentifier( aFirst.GetMainURL( INetURLObject::NO_DECODE ) );
                uno::Reference< ::com::sun::star::ucb::XContentIdentifier > xIdSecond
                    = xIdFac->createContentIdentifier( aSecond.GetMainURL( INetURLObject::NO_DECODE ) );

                if ( xIdFirst.is() && xIdSecond.is() )
                {
                    uno::Reference< ::com::sun::star::ucb::XContentProvider > xProvider =
                                                            pBroker->getContentProviderInterface();
                    if ( !xProvider.is() )
                        throw uno::RuntimeException();
                    bResult = !xProvider->compareContentIds( xIdFirst, xIdSecond );
                }
            }
            catch( uno::Exception& )
            {
                OSL_ENSURE( sal_False, "Can't compare URL's, treat as different!\n" );
            }
        }
    }

    return bResult;
}

SV_DECL_PTRARR_DEL(SvKeyValueList_Impl, SvKeyValue*, 0, 4)
SV_IMPL_PTRARR(SvKeyValueList_Impl, SvKeyValue*);

/*
 * SvKeyValueIterator.
 */
SvKeyValueIterator::SvKeyValueIterator (void)
    : m_pList (new SvKeyValueList_Impl),
      m_nPos  (0)
{
}

/*
 * ~SvKeyValueIterator.
 */
SvKeyValueIterator::~SvKeyValueIterator (void)
{
    delete m_pList;
}

/*
 * GetFirst.
 */
BOOL SvKeyValueIterator::GetFirst (SvKeyValue &rKeyVal)
{
    m_nPos = m_pList->Count();
    return GetNext (rKeyVal);
}

/*
 * GetNext.
 */
BOOL SvKeyValueIterator::GetNext (SvKeyValue &rKeyVal)
{
    if (m_nPos > 0)
    {
        rKeyVal = *m_pList->GetObject(--m_nPos);
        return TRUE;
    }
    else
    {
        // Nothing to do.
        return FALSE;
    }
}

/*
 * Append.
 */
void SvKeyValueIterator::Append (const SvKeyValue &rKeyVal)
{
    SvKeyValue *pKeyVal = new SvKeyValue (rKeyVal);
    m_pList->C40_INSERT(SvKeyValue, pKeyVal, m_pList->Count());
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

        if ( aNewURL.getLength() )
        {
            uno::Reference< embed::XStorage > xStorage = GetStorage();
            uno::Reference< embed::XOptimizedStorage > xOptStorage( xStorage, uno::UNO_QUERY );

            if ( xOptStorage.is() )
            {
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
    sal_Bool bResult = sal_False;
    ::rtl::OUString aOrigURL = aLogicName;

    if ( aURL.getLength() && aOrigURL.getLength() )
    {
        uno::Reference< embed::XStorage > xStorage = GetStorage();
        uno::Reference< embed::XOptimizedStorage > xOptStorage( xStorage, uno::UNO_QUERY );

        if ( xOptStorage.is() )
        {
            CanDisposeStorage_Impl( sal_False );
            Close();
            SetPhysicalName_Impl( String() );
            SetName( aURL );
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

