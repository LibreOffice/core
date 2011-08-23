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

#include "docfile.hxx"

#include <uno/mapping.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/XContent.hpp>

#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/util/XArchiver.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XStreamListener.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <tools/zcodec.hxx>
#include <tools/cachestr.hxx>
#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/streamhelper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/msgbox.hxx>
#include <bf_so3/svstor.hxx>
#include <bf_svtools/stritem.hxx>
#include <bf_svtools/eitem.hxx>
#include <bf_svtools/lckbitem.hxx>
#include <bf_svtools/sfxecode.hxx>
#include <bf_svtools/itemset.hxx>
#include <bf_svtools/intitem.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implbase1.hxx>

#define _SVSTDARR_ULONGS
#define _SVSTDARR_STRINGSDTOR
#include <bf_svtools/svstdarr.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <comphelper/processfactory.hxx>
#include <bf_so3/transbnd.hxx> // SvKeyValueIterator
#include <tools/urlobj.hxx>
#include <tools/inetmime.hxx>
#include <unotools/ucblockbytes.hxx>
#include <bf_svtools/pathoptions.hxx>
#include <bf_svtools/asynclink.hxx>
#include <bf_svtools/inettype.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/progresshandlerwrap.hxx>
#include <ucbhelper/content.hxx>
#include <sot/stg.hxx>

#include "helper.hxx"
#include "request.hxx"      // SFX_ITEMSET_SET
#include "app.hxx"          // GetFilterMatcher
#include "appuno.hxx"        // LoadTargetFrame
#include "fltfnc.hxx"       // SfxFilterMatcher
#include "docfilt.hxx"      // SfxFilter
#include "objsh.hxx"        // CheckOpenMode
#include "docfac.hxx"       // GetFilterContainer
#include "openflag.hxx"     // SFX_STREAM_READONLY etc.
#include "sfxsids.hrc"

#include "xmlversion.hxx"

#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002
namespace binfilter {

#define MAX_REDIRECT 5

/*N*/ class SfxLockBytesHandler_Impl : public ::utl::UcbLockBytesHandler
/*N*/ {
/*N*/     ULONG           m_nAcquireCount;
/*N*/     SfxMedium*      m_pMedium;
/*N*/     ::osl::Mutex    m_aMutex;
/*N*/ public:
/*N*/                     SfxLockBytesHandler_Impl( SfxMedium* pMedium )
/*N*/                         : m_pMedium( pMedium )
/*N*/                         , m_nAcquireCount( 0 )
/*N*/                     {}
/*N*/
/*N*/     virtual void    Handle( ::utl::UcbLockBytesHandler::LoadHandlerItem nWhich, ::utl::UcbLockBytesRef xLockBytes );
/*N*/     ::osl::Mutex&   GetMutex()
/*N*/                     { return m_aMutex; }
/*N*/     void            ReleaseMedium()
/*N*/                     { m_pMedium = NULL; }
/*N*/ };

/*?*/ SV_DECL_IMPL_REF( SfxLockBytesHandler_Impl );

/*N*/ void SfxLockBytesHandler_Impl::Handle( ::utl::UcbLockBytesHandler::LoadHandlerItem nWhich, ::utl::UcbLockBytesRef xLockBytes )
/*N*/ {
/*N*/     ::osl::MutexGuard aGuard( m_aMutex );
/*N*/     if ( IsActive() && xLockBytes.Is()&& m_pMedium )
/*N*/     {
/*N*/         switch( nWhich )
/*N*/         {
/*N*/                 break;
/*N*/             case DATA_AVAILABLE :
/*?*/                 m_pMedium->DataAvailable_Impl();
/*N*/                 break;
/*N*/             case DONE :
/*N*/                 m_pMedium->Done_Impl( xLockBytes->GetError() );
/*N*/                 break;
/*N*/             case CANCEL :
/*?*/                 m_pMedium->Cancel_Impl();
/*N*/                 break;
/*N*/             default:
/*N*/                 break;
/*N*/         }
/*N*/     }
/*N*/ }

/*N*/ class UcbLockBytesCancellable_Impl : public SfxCancellable
/*N*/ {
/*N*/     ::utl::UcbLockBytesRef         xLockBytes;
/*N*/
/*N*/ public:
/*N*/                             UcbLockBytesCancellable_Impl( const ::utl::UcbLockBytesRef& rLockBytes, SfxCancelManager* pManager, const String& rTitle )
/*N*/                                 : SfxCancellable( pManager, rTitle )
/*N*/                                 , xLockBytes( rLockBytes )
/*N*/                             {}

/*N*/     virtual void            Cancel();
/*N*/ };

/*N*/ void UcbLockBytesCancellable_Impl::Cancel()
/*N*/ {
/*N*/     xLockBytes->Cancel();
/*N*/ }

/*N*/ class SfxMediumHandler_Impl : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionHandler >
/*N*/ {
/*N*/     ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > m_xInter;
/*N*/
/*N*/ public:
/*N*/     virtual void SAL_CALL handle( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& xRequest )
/*N*/             throw( ::com::sun::star::uno::RuntimeException );
/*N*/
/*N*/     SfxMediumHandler_Impl( ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > xInteraction )
/*N*/         : m_xInter( xInteraction )
/*N*/         {}
/*N*/
/*N*/     ~SfxMediumHandler_Impl();
/*N*/ };

/*N*/ SfxMediumHandler_Impl::~SfxMediumHandler_Impl()
/*N*/ {
/*N*/ }

/*N*/ void SAL_CALL SfxMediumHandler_Impl::handle( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& xRequest )
/*N*/         throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ }

/*?*/ String ConvertDateTime_Impl(const SfxStamp &rTime, const LocaleDataWrapper& rWrapper);

//----------------------------------------------------------------
/*N*/ SfxPoolCancelManager::SfxPoolCancelManager( SfxCancelManager* pParent, const String& rName )
/*N*/     : SfxCancelManager( pParent ),
/*N*/       SfxCancellable( pParent ? pParent : this, rName ),
/*N*/       wParent( pParent )
/*N*/ {
/*N*/     if( pParent )
/*N*/     {
/*N*/         StartListening( *this );
/*N*/         SetManager( 0 );
/*N*/     }
/*N*/ }

//----------------------------------------------------------------
/*N*/ SfxPoolCancelManager::~SfxPoolCancelManager()
/*N*/ {
/*N*/     for( sal_uInt16 nPos = GetCancellableCount(); nPos--; )
/*N*/     {
/*N*/         // nicht an Parent uebernehmen!
/*?*/         SfxCancellable* pCbl = GetCancellable( nPos );
/*?*/         if ( pCbl )
/*?*/             pCbl->SetManager( 0 );
/*N*/     }
/*N*/ }


//----------------------------------------------------------------
/*N*/ void SfxPoolCancelManager::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
/*N*/ {
/*N*/     if( !GetCancellableCount() ) SetManager( 0 );
/*N*/     else if( !GetManager() )
/*N*/     {
/*N*/         if( !wParent.Is() ) wParent = SFX_APP()->GetCancelManager();
/*N*/         SetManager( wParent );
/*N*/     }
/*N*/ }

//----------------------------------------------------------------
/*N*/ void SfxPoolCancelManager::Cancel()
/*N*/ {
/*N*/     SfxPoolCancelManagerRef xThis = this;
/*N*/     for( sal_uInt16 nPos = GetCancellableCount(); nPos--; )
/*N*/     {
/*?*/         SfxCancellable* pCbl = GetCancellable( nPos );
/*?*/         // Wenn wir nicht im Button stehen
/*?*/         if( pCbl && pCbl != this )
/*?*/             pCbl->Cancel();
/*?*/         if( GetCancellableCount() < nPos )
/*?*/             nPos = GetCancellableCount();
/*N*/     }
/*N*/ }

//----------------------------------------------------------------
/*?*/ class SfxMedium_Impl : public SvCompatWeakBase
/*?*/ {
/*?*/ public:
/*?*/     ::ucbhelper::Content aContent;
/*?*/ 	String aBaseURL;
/*?*/     sal_Bool bUpdatePickList : 1;
/*?*/     sal_Bool bIsTemp        : 1;
/*?*/     sal_Bool bUsesCache     : 1;
/*?*/     sal_Bool bForceSynchron : 1;
/*?*/     sal_Bool bDontCreateCancellable : 1;
/*?*/     sal_Bool bDownloadDone          : 1;
/*?*/     sal_Bool bDontCallDoneLinkOnSharingError : 1;
/*?*/     sal_Bool bStreamReady: 1;
/*?*/     sal_Bool bIsStorage: 1;
/*?*/     sal_Bool bUseInteractionHandler: 1;
/*?*/     sal_Bool bAllowDefaultIntHdl: 1;
/*?*/     sal_Bool bIsDiskSpannedJAR: 1;
/*?*/     sal_Bool bIsCharsetInitialized: 1;
/*?*/
/*?*/     sal_uInt16       nPrio;
/*?*/
/*?*/     SfxPoolCancelManagerRef xCancelManager;
/*?*/     UcbLockBytesCancellable_Impl* pCancellable;
/*?*/     SfxMedium*       pAntiImpl;
/*?*/
/*?*/     long             nFileVersion;
/*?*/
/*?*/     const SfxFilter* pOrigFilter;
/*?*/     String           aOrigURL;
/*?*/     String           aPreRedirectionURL;
/*?*/     String           aReferer;
/*?*/     DateTime         aExpireTime;
/*?*/     SvKeyValueIteratorRef xAttributes;
/*?*/     SvRefBaseRef    xLoadRef;
/*?*/
          AsynchronLink  aDoneLink;
        AsynchronLink  aAvailableLink;
/*?*/     SfxLockBytesHandler_ImplRef  aHandler;
/*?*/
/*?*/     SfxVersionTableDtor*    pVersions;
/*?*/     ::utl::TempFile*           pTempDir;
/*?*/     ::utl::TempFile*           pTempFile;
/*?*/
/*?*/     Reference < XInputStream > xInputStream;
/*?*/     ::utl::UcbLockBytesRef     xLockBytes;
/*?*/
/*?*/ 	sal_uInt32					nLastStorageError;
/*?*/ 	::rtl::OUString				aCharset;
/*?*/
/*?*/     ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > xInteraction;
/*?*/
/*?*/ 	sal_Bool 		m_bRemoveBackup;
/*?*/ 	::rtl::OUString m_aBackupURL;
/*?*/
/*?*/     SfxPoolCancelManager* GetCancelManager();
/*?*/
/*?*/     SfxMedium_Impl( SfxMedium* pAntiImplP );
/*?*/     ~SfxMedium_Impl();
/*?*/ };

/*N*/ void SfxMedium::Done_Impl( ErrCode nError )
/*N*/ {
/*N*/     DELETEZ( pImp->pCancellable );
/*N*/     pImp->bDownloadDone = sal_True;
/*N*/     SetError( nError );
/*N*/     if ( pImp->xLockBytes.Is() )
/*N*/         pImp->xInputStream = pImp->xLockBytes->getInputStream();
/*N*/
/*N*/     if ( ( !nError || !pImp->bDontCallDoneLinkOnSharingError ) && ( pImp->bStreamReady || !pInStream ) )
/*N*/     {
/*N*/         pImp->aDoneLink.ClearPendingCall();
/*N*/         pImp->aDoneLink.Call( (void*) nError );
/*N*/     }
/*N*/ }

/*N*/ void SfxMedium::DataAvailable_Impl()
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ }

/*N*/ void SfxMedium::Cancel_Impl()
/*N*/ {
/*N*/     SetError( ERRCODE_IO_GENERAL );
/*N*/ }

/*N*/ SfxPoolCancelManager* SfxMedium_Impl::GetCancelManager()
/*N*/ {
/*N*/     if( !xCancelManager.Is() )
/*N*/     {
/*N*/         if( !bDontCreateCancellable )
/*N*/             xCancelManager = new SfxPoolCancelManager(
/*N*/                 SFX_APP()->GetCancelManager(),
/*N*/                 pAntiImpl->GetURLObject().GetURLNoPass() );
/*N*/         else
/*?*/             xCancelManager = new SfxPoolCancelManager(
/*?*/                 0, pAntiImpl->GetURLObject().GetURLNoPass() );
/*N*/     }
/*N*/     return xCancelManager;
/*N*/ }

//------------------------------------------------------------------
/*N*/ SfxMedium_Impl::SfxMedium_Impl( SfxMedium* pAntiImplP )
/*N*/  :
/*N*/     SvCompatWeakBase( pAntiImplP ),
/*N*/     bUpdatePickList(sal_True), bIsTemp( sal_False ), pOrigFilter( 0 ),
/*N*/     bUsesCache(sal_True), pCancellable( 0 ),
/*N*/     nPrio( 99 ), aExpireTime( Date() + 10, Time() ),
/*N*/     bForceSynchron( sal_False ), bStreamReady( sal_False ), bIsStorage( sal_False ),
/*N*/     pAntiImpl( pAntiImplP ),
/*N*/     bDontCreateCancellable( sal_False ), pTempDir( NULL ), bIsDiskSpannedJAR( sal_False ),
/*N*/     bDownloadDone( sal_True ), bDontCallDoneLinkOnSharingError( sal_False ),nFileVersion( 0 ),
/*N*/   pTempFile( NULL ),
/*N*/ 	nLastStorageError( 0 ),
/*N*/ 	bIsCharsetInitialized( sal_False ),
/*N*/ 	bUseInteractionHandler( sal_True ),
/*N*/ 	bAllowDefaultIntHdl( sal_False ),
/*N*/ 	m_bRemoveBackup( sal_False )
/*N*/ {
/*N*/     aHandler = new SfxLockBytesHandler_Impl( pAntiImpl );
/*N*/     aDoneLink.CreateMutex();
/*N*/ }

//------------------------------------------------------------------
/*N*/ SfxMedium_Impl::~SfxMedium_Impl()
/*N*/ {
/*N*/     delete pCancellable;
/*N*/
/*N*/     if ( aHandler.Is() )
/*N*/         aHandler->Activate( sal_False );
/*N*/
/*N*/     aDoneLink.ClearPendingCall();
/*N*/     aAvailableLink.ClearPendingCall();
/*N*/
/*N*/     delete pVersions;
/*N*/
/*N*/     if ( pTempFile )
/*N*/         delete pTempFile;
/*N*/
/*N*/     if ( pTempDir )
/*?*/         delete pTempDir;
/*N*/ }

//================================================================

/*?*/ #define IMPL_CTOR()                         \
/*?*/      eError( SVSTREAM_OK ),                 \
/*?*/                                             \
/*?*/      bDirect( sal_False ),                  \
/*?*/      bTriedStorage( sal_False ),            \
/*?*/      bSetFilter( sal_False ),               \
/*?*/                                             \
/*?*/      nStorOpenMode( SFX_STREAM_READWRITE ), \
/*?*/      pInStream(0),                          \
/*?*/      pOutStream( 0 )

//------------------------------------------------------------------
/*N*/ void SfxMedium::ResetError()
/*N*/ {
/*N*/     eError = SVSTREAM_OK;
/*N*/     if( aStorage.Is() )
/*N*/         aStorage->ResetError();
/*N*/     if( pInStream )
/*N*/         pInStream->ResetError();
/*N*/     if( pOutStream )
/*?*/         pOutStream->ResetError();
/*N*/ }

//------------------------------------------------------------------
/*N*/ sal_uInt32 SfxMedium::GetLastStorageCreationState()
/*N*/ {
/*N*/ 	return pImp->nLastStorageError;
/*N*/ }

//------------------------------------------------------------------
/*N*/ sal_uInt32 SfxMedium::GetErrorCode() const
/*N*/ {
/*N*/     sal_uInt32 lError=eError;
/*N*/     if(!lError && pInStream)
/*N*/         lError=pInStream->GetErrorCode();
/*N*/     if(!lError && pOutStream)
/*?*/         lError=pOutStream->GetErrorCode();
/*N*/     if(!lError && aStorage.Is())
/*N*/         lError=aStorage->GetErrorCode();
/*N*/     return lError;
/*N*/ }

//------------------------------------------------------------------
/*N*/ long SfxMedium::GetFileVersion() const
/*N*/ {
/*N*/     if ( !pImp->nFileVersion && pFilter )
/*N*/         return pFilter->GetVersion();
/*N*/     else
/*?*/         return pImp->nFileVersion;
/*N*/ }

//------------------------------------------------------------------
/*N*/ Reference < XContent > SfxMedium::GetContent() const
/*N*/ {
/*N*/     if ( !pImp->aContent.get().is() )
/*N*/     {
/*N*/ 		Reference < ::com::sun::star::ucb::XContent > xContent;
/*N*/ 	    Reference < ::com::sun::star::ucb::XCommandEnvironment > xEnv;
/*N*/         SFX_ITEMSET_ARG( pSet, pItem, SfxUnoAnyItem, SID_CONTENT, sal_False);
/*N*/         if ( pItem )
/*?*/             pItem->GetValue() >>= xContent;
/*N*/
/*N*/ 		if ( xContent.is() )
/*N*/ 		{
/*N*/ 			try
/*N*/ 			{
/*?*/ 				pImp->aContent = ::ucbhelper::Content( xContent, xEnv );
/*N*/ 			}
/*N*/ 			catch ( Exception& )
/*N*/ 			{
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			String aURL;
/*N*/ 	        if ( aName.Len() )
/*N*/ 	        	::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aURL );
/*N*/ 	        else if ( aLogicName.Len() )
/*N*/ 	            aURL = GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
/*N*/ 			if ( aURL.Len() )
/*N*/ 	            ::ucbhelper::Content::create( aURL, xEnv, pImp->aContent );
/*N*/ 		}
/*N*/     }
/*N*/
/*N*/     return pImp->aContent.get();
/*N*/ }

/*N*/ const String& SfxMedium::GetBaseURL()
/*N*/ {
/*N*/     if ( !pImp->aBaseURL.Len() && GetContent().is() )
/*N*/     {
/*N*/         try
/*N*/         {
/*N*/             Any aAny = pImp->aContent.getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BaseURI" )) );
/*N*/             ::rtl::OUString aStr;
/*N*/             if ( ( aAny >>= aStr ) && aStr.getLength() )
/*N*/ 				pImp->aBaseURL = aStr;
/*N*/         }
/*N*/         catch ( ::com::sun::star::uno::Exception& )
/*N*/         {
/*N*/         }
/*N*/     }
/*N*/
/*N*/ 	if ( !pImp->aBaseURL.Len() )
/*N*/ 		pImp->aBaseURL = GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
/*N*/ 	return pImp->aBaseURL;
/*N*/ }

//------------------------------------------------------------------
/*N*/ SvStream* SfxMedium::GetInStream()
/*N*/ {
/*N*/     if ( pInStream )
/*N*/         return pInStream;
/*N*/
/*N*/     if ( pImp->pTempFile || pImp->pTempDir )
/*N*/     {
/*?*/         pInStream = new SvFileStream( aName, nStorOpenMode );
/*?*/
/*?*/         eError = pInStream->GetError();
/*?*/
/*?*/         if( !eError && (nStorOpenMode & STREAM_WRITE)
/*?*/                     && ! pInStream->IsWritable() )
/*?*/         {
/*?*/             eError = ERRCODE_IO_ACCESSDENIED;
/*?*/             delete pInStream;
/*?*/             pInStream = NULL;
/*?*/         }
/*?*/         else
/*?*/             return pInStream;
/*N*/     }
/*N*/
/*N*/     GetMedium_Impl();
/*N*/
/*N*/     if ( !pInStream && eError == ERRCODE_IO_PENDING )
/*?*/         eError = SVSTREAM_OK;
/*N*/
/*N*/     return pInStream;
/*N*/ }

//------------------------------------------------------------------
/*N*/ void SfxMedium::CloseInStream()
/*N*/ {
/*N*/     CloseInStream_Impl();
/*N*/ }

/*N*/ void SfxMedium::CloseInStream_Impl()
/*N*/ {
/*N*/     // if there is a storage based on the InStream, we have to
/*N*/     // close the storage, too, because otherwise the storage
/*N*/     // would use an invalid ( deleted ) stream.
/*N*/     if ( pInStream && aStorage.Is() )
/*N*/     {
/*?*/         const SvStream *pStorage = aStorage->GetSvStream();
/*?*/         if ( pStorage == pInStream )
/*?*/         {
/*?*/             CloseStorage();
/*?*/         }
/*?*/     }
/*N*/
/*N*/     DELETEZ( pInStream );
/*N*/     pImp->xInputStream = Reference < XInputStream >();
/*N*/     pImp->xLockBytes.Clear();
/*N*/     if ( pSet )
/*N*/         pSet->ClearItem( SID_INPUTSTREAM );
/*N*/
/*N*/     DELETEZ( pImp->pCancellable );
/*N*/ }

//------------------------------------------------------------------
/*N*/ SvStream* SfxMedium::GetOutStream()
/*N*/ {
/*N*/     if ( !pOutStream )
/*N*/     {
/*N*/         // Create a temp. file if there is none because we always
/*N*/         // need one.
/*N*/         if ( !pImp->pTempFile )
/*?*/             CreateTempFile();
/*N*/
/*N*/         if ( pImp->pTempFile )
/*N*/         {
/*N*/             pOutStream = new SvFileStream( aName, STREAM_STD_READWRITE );
/*N*/             CloseStorage();
/*N*/         }
/*N*/     }
/*N*/
/*N*/     return pOutStream;
/*N*/ }

//------------------------------------------------------------------
/*N*/ sal_Bool SfxMedium::CloseOutStream()
/*N*/ {
/*N*/     CloseOutStream_Impl();
/*N*/     return sal_True;
/*N*/ }

/*N*/ sal_Bool SfxMedium::CloseOutStream_Impl()
/*N*/ {
/*N*/     if ( pOutStream )
/*N*/     {
/*N*/         // if there is a storage based on the OutStream, we have to
/*N*/         // close the storage, too, because otherwise the storage
/*N*/         // would use an invalid ( deleted ) stream.
/*N*/         if ( aStorage.Is() )
/*N*/         {
/*?*/             const SvStream *pStorage = aStorage->GetSvStream();
/*?*/             if ( pStorage == pOutStream )
/*?*/                 CloseStorage();
/*N*/         }
/*N*/
/*N*/         delete pOutStream;
/*N*/         pOutStream = NULL;
/*N*/     }
/*N*/
/*N*/     return sal_True;
/*N*/ }

//------------------------------------------------------------------
/*N*/ const String& SfxMedium::GetPhysicalName() const
/*N*/ {
/*N*/     if ( !aName.Len() && aLogicName.Len() )
/*N*/         (( SfxMedium*)this)->CreateFileStream();
/*N*/
/*N*/     // return the name then
/*N*/     return aName;
/*N*/ }

//------------------------------------------------------------------
/*N*/ void SfxMedium::CreateFileStream()
/*N*/ {
/*N*/     ForceSynchronStream_Impl( TRUE );
/*N*/     GetInStream();
/*N*/     if( pInStream )
/*N*/     {
/*N*/         if ( !pImp->pTempFile )
/*N*/             CreateTempFile();
/*N*/         pImp->bIsTemp = sal_True;
/*N*/         CloseInStream_Impl();
/*N*/     }
/*N*/ }

//------------------------------------------------------------------
/*N*/ sal_Bool SfxMedium::Commit()
/*N*/ {
/*N*/     if( aStorage.Is() )
/*N*/     {
/*N*/         // StorageStream immer direkt
/*N*/         if( !aStorage->Commit() )
/*?*/             eError = aStorage->GetError();
/*N*/     }
/*N*/     else if( pOutStream  )
/*?*/         pOutStream->Flush();
/*N*/     else if( pInStream  )
/*?*/         pInStream->Flush();
/*N*/
/*N*/     if ( ( GetError() == SVSTREAM_OK ) && pImp->pTempFile )
/*N*/         Transfer_Impl();
/*N*/
/*N*/ 	ClearBackup_Impl();
/*N*/
/*N*/     return GetError() == SVSTREAM_OK;
/*N*/ }

//------------------------------------------------------------------
/*N*/ sal_Bool SfxMedium::IsStorage()
/*N*/ {
/*N*/     if ( aStorage.Is() )
/*?*/         return TRUE;
/*N*/
/*N*/     if ( bTriedStorage )
/*?*/         return pImp->bIsStorage;
/*N*/
/*N*/     if ( pImp->pTempFile )
/*N*/     {
/*?*/ 		String aURL;
#ifdef DBG_UTIL
/*?*/ 		if ( !::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aURL ) )
/*?*/ 			DBG_ERROR("Physical name not convertable!");
#endif
/*?*/         pImp->bIsStorage = SotStorage::IsStorageFile( aURL );
/*?*/         if ( !pImp->bIsStorage )
/*?*/             bTriedStorage = TRUE;
/*N*/     }
/*N*/     else if ( GetInStream() )
/*N*/     {
/*?*/         pImp->bIsStorage = SotStorage::IsStorageFile( pInStream );
/*?*/         if ( !pInStream->GetError() && !pImp->bIsStorage )
/*?*/             bTriedStorage = TRUE;
/*N*/     }
/*N*/
/*N*/     return pImp->bIsStorage;
/*N*/ }

//------------------------------------------------------------------
/*N*/ sal_Bool SfxMedium::TryStorage()
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001
/*N*/ }


//------------------------------------------------------------------
/*N*/ SvStorage* SfxMedium::GetOutputStorage( BOOL bUCBStorage )
/*N*/ {
/*N*/     // if the medium was constructed with a SvStorage: use this one, not a temp. storage
/*N*/     if ( aStorage.Is() && !aLogicName.Len() )
/*N*/         return aStorage;
/*N*/
/*N*/     if ( !pImp->pTempFile )
/*N*/         CreateTempFile();
/*N*/     return GetStorage_Impl( bUCBStorage );
/*N*/ }

/*N*/ SvStorage* SfxMedium::GetStorage()
/*N*/ {
/*N*/     return GetStorage_Impl( pFilter && SOFFICE_FILEFORMAT_60 <= pFilter->GetVersion() );
/*N*/ }

/*N*/ SvStorage* SfxMedium::GetStorage_Impl( BOOL bUCBStorage )
/*N*/ {
/*N*/     if ( aStorage.Is() || bTriedStorage )
/*N*/         return aStorage;
/*N*/
/*N*/     String aStorageName;
/*N*/     if ( pImp->pTempFile || pImp->pTempDir )
/*N*/     {
/*N*/         // open storage with the URL of the tempfile
/*N*/ 		if ( !::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aStorageName ) )
/*N*/ 		{
/*?*/ 			DBG_ERROR("Physical name not convertable!");
/*N*/ 		}
/*N*/         CloseOutStream();
/*N*/         aStorage = new SvStorage( bUCBStorage, aStorageName, nStorOpenMode, bDirect ? 0 : STORAGE_TRANSACTED );
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         if ( aName.Len() )
/*N*/         {
/*N*/             if ( !::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aStorageName ) )
/*N*/ 			{
/*?*/                 DBG_ERROR("Physical name not convertable!");
/*N*/ 			}
/*N*/         }
/*N*/         else
/*N*/             aStorageName = GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
/*N*/
/*N*/         GetInStream();
/*N*/         if ( pInStream )
/*N*/         {
/*?*/             pInStream->GetLockBytes()->SetSynchronMode( sal_True );
/*?*/             if( UCBStorage::IsDiskSpannedFile( pInStream ) )
/*?*/             {
/*?*/                 // packed remote files can't be opened outside the storage, so they must be reopened
/*?*/                 // inside the storage even if it is expensive
/*?*/                 pImp->bIsDiskSpannedJAR = TRUE;
/*?*/                 CloseInStream();
/*?*/                 aStorage = new SvStorage( TRUE, aStorageName, nStorOpenMode, bDirect ? 0 : STORAGE_TRANSACTED );
/*?*/                 SfxFilterFlags nMust = SFX_FILTER_IMPORT, nDont = SFX_FILTER_NOTINSTALLED | SFX_FILTER_STARONEFILTER;
/*?*/                 SetFilter( SFX_APP()->GetFilterMatcher().GetFilter4ClipBoardId( aStorage->GetFormat(), nMust, nDont ) );
/*N*/             }
/*N*/             else
/*N*/             {
/*N*/                 // download the stream ( or at least start the download )
/*N*/                 if ( !pImp->aDoneLink.IsSet() )
/*N*/                     DownLoad();
/*N*/
/*N*/        			SFX_ITEMSET_ARG( GetItemSet(), pItem, SfxBoolItem, SID_REPAIRPACKAGE, sal_False);
/*N*/        			if ( pItem && pItem->GetValue() )
/*N*/ 				{
/*N*/ 					// CreateTempFile();
/*?*/ 					Reference< ::com::sun::star::ucb::XProgressHandler > xProgressHandler;
/*?*/ 					Reference< ::com::sun::star::task::XStatusIndicator > xStatusIndicator;
/*?*/ 					SFX_ITEMSET_ARG( GetItemSet(), pxProgressItem, SfxUnoAnyItem, SID_PROGRESS_STATUSBAR_CONTROL, sal_False );
/*?*/ 					if( pxProgressItem && ( pxProgressItem->GetValue() >>= xStatusIndicator ) )
/*?*/ 						xProgressHandler = Reference< ::com::sun::star::ucb::XProgressHandler >(
/*?*/ 												new ::utl::ProgressHandlerWrap( xStatusIndicator ) );
/*?*/
/*?*/        				INetURLObject aObj( aName );
/*?*/        				if ( aObj.GetProtocol() == INET_PROT_NOT_VALID )
/*?*/        				{
/*?*/            				String aURL;
/*?*/            				::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aURL );
/*?*/            				aObj.SetURL( aURL );
/*?*/        				}
/*?*/
/*?*/ 					UCBStorage* pUCBStorage = new UCBStorage( aObj.GetMainURL( INetURLObject::NO_DECODE ),
/*?*/ 															  nStorOpenMode,
/*?*/ 															  bDirect ? 0 : STORAGE_TRANSACTED,
/*?*/ 															  sal_True,
/*?*/ 															  sal_True,
/*?*/ 															  xProgressHandler );
/*?*/
/*?*/ 					aStorage = new SvStorage( pUCBStorage );
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					if( SotStorage::IsStorageFile( pInStream ) )
/*N*/ 					{
/*N*/ 				/*		if ( IsReadOnly() && ::utl::LocalFileHelper::IsLocalFile( aLogicName ) )
                         {
                             CreateTempFile();
                             aStorage = new SvStorage( bUCBStorage, aName, nStorOpenMode, bDirect ? 0 : STORAGE_TRANSACTED );
                         }
                         else */
/*N*/ 						{
/*N*/ 							if ( bUCBStorage && !UCBStorage::IsStorageFile( pInStream ) )
/*N*/ 								return NULL;
/*N*/
/*N*/ 							// create a storage on the stream
/*N*/                 			aStorage = new SvStorage( pInStream, sal_False );
/*N*/                 			if ( !aStorage->GetName().Len() )
/*N*/                    				aStorage->SetName( aStorageName );
/*N*/ 						}
/*N*/ 					}
/*N*/ 					else
/*N*/ 						return NULL;
/*N*/ 				}
/*N*/             }
/*N*/         }
/*N*/         else
/*N*/             return NULL;
/*N*/     }
/*N*/
/*N*/     if( ( pImp->nLastStorageError = GetError() ) != SVSTREAM_OK )
/*N*/     {
/*N*/         aStorage.Clear();
/*N*/         if ( pInStream )
/*N*/             pInStream->Seek(0);
/*N*/         return NULL;
/*N*/     }
/*N*/
/*N*/     bTriedStorage = sal_True;
/*N*/
/*N*/     if ( aStorage->GetError() == SVSTREAM_OK )
/*N*/         GetVersionList();
/*N*/
/*N*/     // ???? wird das noch gebraucht?
/*N*/ //  GetMedium_Impl();
/*N*/ //  if ( !aStorage.Is() )
/*N*/ //      CreateFileStream();
/*N*/
/*N*/     SFX_ITEMSET_ARG( pSet, pVersion, SfxInt16Item, SID_VERSION, sal_False);
/*N*/
/*N*/     BOOL bResetStorage = FALSE;
/*N*/     if ( pVersion && pVersion->GetValue() )
/*N*/     {
/*N*/         // Alle verf"ugbaren Versionen einlesen
/*N*/         if ( pImp->pVersions )
/*N*/         {
/*?*/             // Die zum Kommentar passende Version suchen
/*?*/             // Die Versionen sind von 1 an durchnumeriert, mit negativen
/*?*/             // Versionsnummern werden die Versionen von der aktuellen aus
/*?*/             // r"uckw"arts gez"ahlt
/*?*/             short nVersion = pVersion ? pVersion->GetValue() : 0;
/*?*/             if ( nVersion<0 )
/*?*/                 nVersion = ( (short) pImp->pVersions->Count() ) + nVersion;
/*?*/             else if ( nVersion )
/*?*/                 nVersion--;
/*?*/
/*?*/             SfxVersionInfo* pInfo = nVersion>=0 ? pImp->pVersions->GetObject( nVersion ) : NULL;
/*?*/             if ( pInfo )
/*?*/             {
/*?*/                 String aVersionStream = pInfo->aName;
/*?*/
/*?*/                 // SubStorage f"ur alle Versionen "offnen
/*?*/                 SvStorageRef aSub =
/*?*/                     aStorage->OpenStorage( DEFINE_CONST_UNICODE( "Versions" ), SFX_STREAM_READONLY | STREAM_NOCREATE );
/*?*/
/*?*/                 DBG_ASSERT( aSub.Is() && !aSub->GetError(), "Versionsliste, aber keine Versionen!" );
/*?*/
/*?*/                 // Dort ist die Version als gepackter Stream gespeichert
/*?*/                 SvStorageStreamRef aStream =
/*?*/                     aSub->OpenStream( aVersionStream, SFX_STREAM_READONLY );
/*?*/
/*?*/                 if ( aStream.Is() && aStream->GetError() == SVSTREAM_OK )
/*?*/                 {
/*?*/                     // Stream ins TempDir auspacken
/*?*/                     ::utl::TempFile aTempFile;
/*?*/                     String          aTmpName = aTempFile.GetURL();
/*?*/                     SvFileStream    aTmpStream( aTmpName, SFX_STREAM_READWRITE );
/*?*/
/*?*/                     // The new file format uses UCB storages instead of OLE storages.
/*?*/                     // These storages aren't compressed.
/*?*/                     if ( !aSub->IsOLEStorage() )
/*?*/                     {
/*?*/                         *aStream >> aTmpStream;
/*?*/                     }
/*?*/                     else
/*?*/                     {
/*?*/                         ZCodec aCodec;
/*?*/                         aCodec.BeginCompression();
/*?*/                         aCodec.Decompress( *aStream, aTmpStream );
/*?*/                         aCodec.EndCompression();
/*?*/                     }
/*?*/                     aTmpStream.Close();
/*?*/
/*?*/                     // Datei als Storage "offnen
/*?*/                     nStorOpenMode = SFX_STREAM_READONLY;
/*?*/                     aStorage = new SvStorage( aTmpName, nStorOpenMode );
/*?*/
/*?*/                     String aTemp;
/*?*/                     ::utl::LocalFileHelper::ConvertURLToPhysicalName( aTmpName, aTemp );
/*?*/                     SetPhysicalName_Impl( aTemp );
/*?*/
/*?*/                     pImp->bIsTemp = sal_True;
/*?*/                     GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, sal_True ) );
/*?*/                     DELETEZ( pImp->pVersions );
/*?*/                 }
/*?*/                 else
/*?*/                     bResetStorage = TRUE;
/*?*/             }
/*?*/             else
/*?*/                 bResetStorage = TRUE;
/*?*/         }
/*?*/         else
/*?*/             bResetStorage = TRUE;
/*N*/     }
/*N*/
/*N*/     if ( aStorage.Is() )
/*N*/     {
/*N*/         if( ( pImp->nLastStorageError = aStorage->GetError() ) != SVSTREAM_OK )
/*N*/             bResetStorage = TRUE;
/*N*/         else if ( GetFilter() )
/*N*/             aStorage->SetVersion( GetFilter()->GetVersion() );
/*N*/     }
/*N*/
/*N*/     if ( bResetStorage )
/*N*/     {
/*?*/         aStorage.Clear();
/*?*/         if ( pInStream )
/*?*/             pInStream->Seek( 0L );
/*N*/     }
/*N*/
/*N*/     pImp->bIsStorage = aStorage.Is();
/*N*/     return aStorage;
/*N*/ }

//------------------------------------------------------------------
/*N*/ void SfxMedium::CloseStorage()
/*N*/ {
/*N*/     aStorage.Clear();
/*N*/     bTriedStorage = sal_False;
/*N*/     pImp->bIsStorage = sal_False;
/*N*/ }

//------------------------------------------------------------------
/*?*/ void SfxMedium::SetOpenMode( StreamMode nStorOpen,
/*?*/                              sal_Bool bDirectP,
/*?*/                              sal_Bool bDontClose )
/*?*/ {
/*?*/     if ( nStorOpenMode != nStorOpen )
/*?*/     {
/*?*/         nStorOpenMode = nStorOpen;
/*?*/
/*?*/         if( !bDontClose )
/*?*/             Close();
/*?*/     }
/*?*/
/*?*/     bDirect     = bDirectP;
/*?*/     bSetFilter  = sal_False;
/*?*/ }



//------------------------------------------------------------------
/*N*/ void SfxMedium::Transfer_Impl()
/*N*/ {
/*N*/     if( pImp->pTempFile && ( !eError || eError & ERRCODE_WARNING_MASK ) )
/*N*/     {
/*N*/         Reference < ::com::sun::star::ucb::XCommandEnvironment > xEnv;
/*N*/ 		Reference< XOutputStream > rOutStream;
/*N*/
/*N*/ 		// in case an output stream is provided from outside and the URL is correct
/*N*/ 		// commit to the stream
/*N*/         if( aLogicName.CompareToAscii( "private:stream", 14 ) == COMPARE_EQUAL )
/*N*/ 		{
/*N*/ 	   		SFX_ITEMSET_ARG( pSet, pOutStreamItem, SfxUnoAnyItem, SID_OUTPUTSTREAM, sal_False);
/*N*/ 	 		if( pOutStreamItem && ( pOutStreamItem->GetValue() >>= rOutStream ) )
/*N*/ 			{
/*N*/ 				// write directly to the stream
/*N*/ 				Close();
/*N*/
/*N*/     		    INetURLObject aSource( pImp->pTempFile->GetURL() );
/*N*/ 				::ucbhelper::Content aTempCont;
/*N*/ 				if( ::ucbhelper::Content::create( aSource.GetMainURL( INetURLObject::NO_DECODE ), xEnv, aTempCont ) )
/*N*/ 				{
/*N*/ 					try
/*N*/ 					{
/*N*/ 						sal_Int32 nRead;
/*N*/ 						sal_Int32 nBufferSize = 32767;
/*N*/ 						Sequence < sal_Int8 > aSequence ( nBufferSize );
/*N*/ 						Reference< XInputStream > aTempInput = aTempCont.openStream();
/*N*/
/*N*/ 						do
/*N*/ 						{
/*N*/ 							nRead = aTempInput->readBytes ( aSequence, nBufferSize );
/*N*/ 							if ( nRead < nBufferSize )
/*N*/ 							{
/*N*/ 								Sequence < sal_Int8 > aTempBuf ( aSequence.getConstArray(), nRead );
/*N*/ 								rOutStream->writeBytes ( aTempBuf );
/*N*/ 							}
/*N*/ 							else
/*N*/ 								rOutStream->writeBytes ( aSequence );
/*N*/ 						}
/*N*/ 						while ( nRead == nBufferSize );
/*N*/
/*N*/ 						// remove temporary file
/*N*/             			// pImp->pTempFile->EnableKillingFile( sal_True );
/*N*/             			// delete pImp->pTempFile;
/*N*/             			// pImp->pTempFile = NULL;
/*N*/ 					}
/*N*/ 					catch( Exception& )
/*N*/ 					{}
/*N*/ 				}
/*N*/        		}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				DBG_ERROR( "Illegal Output stream parameter!\n" );
/*N*/         		SetError( ERRCODE_IO_GENERAL );
/*N*/ 			}
/*N*/
/*N*/ 			// free the reference
/*N*/ 			pSet->ClearItem( SID_OUTPUTSTREAM );
/*N*/
/*N*/ 			return;
/*N*/ 		}
/*N*/ DBG_BF_ASSERT(0, "STRIP"); return;//STRIP001
/*?*/     }
/*N*/ }

//------------------------------------------------------------------
/*N*/ void SfxMedium::ClearBackup_Impl()
/*N*/ {
/*N*/ 	if( pImp->m_bRemoveBackup )
/*N*/ 	{
#ifdef DBG_UTIL
/*?*/ 		if ( pImp->m_aBackupURL.getLength() )
/*?*/ 			if ( !::utl::UCBContentHelper::Kill( pImp->m_aBackupURL ) )
/*?*/ 				DBG_ERROR("Couldn't remove backup file!");
#endif
/*?*/ 		pImp->m_bRemoveBackup = sal_False;
/*N*/ 	}
/*N*/
/*N*/ 	pImp->m_aBackupURL = ::rtl::OUString();
/*N*/ }

//----------------------------------------------------------------
/*N*/ void SfxMedium::GetMedium_Impl()
/*N*/ {
/*N*/     if ( !pInStream )
/*N*/     {
/*N*/         pImp->bDownloadDone = sal_False;
/*N*/         pImp->bStreamReady = sal_False;
/*N*/         Reference< ::com::sun::star::task::XInteractionHandler > xInteractionHandler = GetInteractionHandler();
/*N*/
/*N*/         ::utl::UcbLockBytesHandler* pHandler = pImp->aHandler;
/*N*/         INetProtocol eProt = GetURLObject().GetProtocol();
/*N*/         if ( eProt != INET_PROT_HTTP && eProt != INET_PROT_FTP || aName.Len() )
/*N*/             pHandler = NULL;
/*N*/         BOOL bSynchron = pImp->bForceSynchron || ! pImp->aDoneLink.IsSet();
/*N*/         SFX_ITEMSET_ARG( pSet, pStreamItem, SfxUnoAnyItem, SID_INPUTSTREAM, sal_False);
/*N*/         if ( pStreamItem )
/*N*/         {
/*N*/             if ( GetContent().is() && !IsReadOnly() )
/*N*/             {
/*?*/                 try
/*?*/                 {
/*?*/                     Any aAny = pImp->aContent.getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsReadOnly" )) );
/*?*/                     BOOL bReadonly;
/*?*/                     if ( ( aAny >>= bReadonly ) && bReadonly )
/*?*/                     {
/*?*/                         GetItemSet()->Put( SfxBoolItem(SID_DOC_READONLY, sal_True));
/*?*/                         SetOpenMode(SFX_STREAM_READONLY, sal_False);
/*?*/                     }
/*?*/                 }
/*?*/                 catch ( ::com::sun::star::uno::Exception& )
/*?*/                 {
/*?*/                 }
/*N*/             }
/*N*/
/*N*/             Reference < ::com::sun::star::io::XInputStream > xStream;
/*N*/             if ( ( pStreamItem->GetValue() >>= xStream ) && xStream.is() )
/*N*/                 pImp->xLockBytes = ::utl::UcbLockBytes::CreateInputLockBytes( xStream );
/*N*/             Done_Impl( pImp->xLockBytes.Is() ? pImp->xLockBytes->GetError() : ERRCODE_IO_NOTSUPPORTED );
/*N*/         }
/*N*/         else
/*N*/         {
/*N*/             SFX_ITEMSET_ARG( GetItemSet(), pItem, SfxBoolItem, SID_DOC_READONLY, sal_False);
/*N*/             BOOL bAllowReadOnlyMode = pItem ? pItem->GetValue() : TRUE;
/*N*/             BOOL bIsWritable = ( nStorOpenMode & STREAM_WRITE );
/*N*/
/*N*/             SFX_ITEMSET_ARG( GetItemSet(), pPostDataItem, SfxUnoAnyItem, SID_POSTDATA, sal_False);
/*N*/             SFX_ITEMSET_ARG( GetItemSet(), pContentTypeItem, SfxStringItem, SID_CONTENT_TYPE, sal_False);
/*N*/             SFX_ITEMSET_ARG( GetItemSet(), pRefererItem, SfxStringItem, SID_REFERER, sal_False);
/*N*/
/*N*/ 			::rtl::OUString aReferer;
/*N*/ 			if ( pRefererItem )
/*N*/ 				aReferer = pRefererItem->GetValue();
/*N*/
/*N*/             if ( pPostDataItem )
/*N*/ 			{
/*?*/                 DBG_ASSERT( bAllowReadOnlyMode, "Strange open mode!" );
/*?*/ 				bIsWritable = FALSE;
/*?*/                 GetItemSet()->Put( SfxBoolItem(SID_DOC_READONLY, sal_True));
/*?*/                 SetOpenMode(SFX_STREAM_READONLY, sal_False);
/*?*/
/*?*/ 				::rtl::OUString aMimeType;
/*?*/ 				if ( pContentTypeItem )
/*?*/ 					aMimeType = pContentTypeItem->GetValue();
/*?*/ 				else
/*?*/ 					aMimeType = ::rtl::OUString::createFromAscii( "application/x-www-form-urlencoded" );
/*?*/
/*?*/ 				Reference < XInputStream > xPostData;
/*?*/ 				if ( pPostDataItem )
/*?*/ 				{
/*?*/                     Any aAny = pPostDataItem->GetValue();
/*?*/                     aAny >>= xPostData;
/*?*/ 				}
/*?*/
/*?*/                 pImp->xLockBytes = ::utl::UcbLockBytes::CreateLockBytes(
/*?*/                         GetContent(), aReferer, aMimeType, xPostData, xInteractionHandler, pHandler );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/             	// no callbacks for opening read/write because we might try readonly later
/*N*/                 pImp->bDontCallDoneLinkOnSharingError = ( bIsWritable && bAllowReadOnlyMode );
/*N*/ 				if ( pImp->bDontCallDoneLinkOnSharingError )
/*N*/ 				{
/*N*/ 					::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > aTempHandler =
/*N*/ 							new SfxMediumHandler_Impl( xInteractionHandler );
/*N*/                 	pImp->xLockBytes = ::utl::UcbLockBytes::CreateLockBytes(
/*N*/                         GetContent(), Sequence < PropertyValue >(), nStorOpenMode, aTempHandler );
/*N*/ 				}
/*N*/ 				else
/*N*/                 	pImp->xLockBytes = ::utl::UcbLockBytes::CreateLockBytes(
/*N*/ 						GetContent(), Sequence < PropertyValue >(), nStorOpenMode, xInteractionHandler, bIsWritable ? NULL : pHandler );
/*N*/ 			}
/*N*/
/*N*/             if ( !pImp->xLockBytes.Is() )
/*N*/             {
/*N*/                 pImp->bDontCallDoneLinkOnSharingError = sal_False;
/*N*/                 Done_Impl( ERRCODE_IO_NOTEXISTS );
/*N*/             }
/*N*/             else if ( pImp->xLockBytes->GetError() == ERRCODE_IO_ACCESSDENIED && bIsWritable && bAllowReadOnlyMode ||
/*N*/                     pImp->xLockBytes->GetError() == ERRCODE_IO_NOTSUPPORTED && bIsWritable )
/*N*/             {
/*?*/                 if ( pImp->xLockBytes->GetError() == ERRCODE_IO_ACCESSDENIED )
/*?*/                 {
/*?*/                     GetItemSet()->Put( SfxBoolItem(SID_DOC_READONLY, sal_True));
/*?*/                     SetOpenMode(SFX_STREAM_READONLY, sal_False);
/*?*/                 }
/*?*/
/*?*/                 ResetError();
/*?*/                 pImp->bDownloadDone = sal_False;
/*?*/                 pImp->bDontCallDoneLinkOnSharingError = sal_False;
/*?*/                 pImp->xLockBytes = ::utl::UcbLockBytes::CreateLockBytes(
/*?*/                         GetContent(), Sequence < PropertyValue >(), SFX_STREAM_READONLY, xInteractionHandler, pHandler );
/*?*/
/*?*/ 				if ( !pHandler && !pImp->bDownloadDone )
/*?*/                     Done_Impl( pImp->xLockBytes->GetError() );
/*?*/             }
/*?*/             else if ( !pHandler && !pImp->bDownloadDone )
/*?*/                 // opening readwrite is always done synchronously
/*?*/                 Done_Impl( pImp->xLockBytes->GetError() );
/*N*/         }
/*N*/
/*N*/         if ( pImp->xLockBytes.Is() && !GetError() )
/*N*/         {
/*N*/             if ( bSynchron )
/*N*/                 pImp->xLockBytes->SetSynchronMode( sal_True );
/*N*/             if ( !pImp->bDownloadDone )
/*?*/                 pImp->pCancellable = new UcbLockBytesCancellable_Impl( pImp->xLockBytes, pImp->GetCancelManager(),
/*?*/ 										GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
/*N*/             pInStream = new SvStream( pImp->xLockBytes );
/*N*/             pInStream->SetBufferSize( 4096 );
/*N*/             pImp->bStreamReady = sal_True;
/*N*/         }
/*N*/     }

/*N*/     // Download completion happened while pInStream was constructed
/*N*/     if ( pImp->bDownloadDone )
/*N*/         Done_Impl( GetError() );
/*N*/
/*N*/ }

//------------------------------------------------------------------
/*N*/ SfxPoolCancelManager* SfxMedium::GetCancelManager_Impl() const
/*N*/ {
/*N*/     return pImp->GetCancelManager();
/*N*/ }

//------------------------------------------------------------------
/*N*/ void SfxMedium::SetCancelManager_Impl( SfxPoolCancelManager* pMgr )
/*N*/ {
/*N*/     pImp->xCancelManager = pMgr;
/*N*/ }

//----------------------------------------------------------------
/*N*/ void SfxMedium::CancelTransfers()
/*N*/ {
/*N*/     if( pImp->xCancelManager.Is() )
/*N*/         pImp->xCancelManager->Cancel();
/*N*/ }

/*N*/ sal_Bool SfxMedium::IsRemote()
/*N*/ {
/*N*/     return bRemote;
/*N*/ }

//------------------------------------------------------------------

/*N*/ void SfxMedium::SetUpdatePickList(sal_Bool bVal)
/*N*/ {
/*N*/     if(!pImp)
/*?*/         pImp = new SfxMedium_Impl( this );
/*N*/     pImp->bUpdatePickList = bVal;
/*N*/ }
//------------------------------------------------------------------

/*N*/ void SfxMedium::SetDoneLink( const Link& rLink )
/*N*/ {
/*N*/     pImp->aDoneLink = rLink;
/*N*/ }

//----------------------------------------------------------------

/*N*/ void SfxMedium::SetDataAvailableLink( const Link& rLink )
/*N*/ {
/*N*/     pImp->aAvailableLink = rLink;
/*N*/ }

//----------------------------------------------------------------
/*N*/ void SfxMedium::StartDownload()
/*N*/ {
/*N*/     GetInStream();
/*N*/ }

/*N*/ void SfxMedium::DownLoad( const Link& aLink )
/*N*/ {
/*N*/     SetDoneLink( aLink );
/*N*/     GetInStream();
/*N*/     if ( pInStream && !aLink.IsSet() )
/*N*/     {
/*N*/         while( !pImp->bDownloadDone )
/*?*/             Application::Yield();
/*N*/     }
/*N*/ }

//------------------------------------------------------------------
/*N*/ void SfxMedium::Init_Impl()
/*  [Beschreibung]
    Setzt in den Logischen Namen eine gueltige ::com::sun::star::util::URL (Falls zuvor ein Filename
    drin war) und setzt den physikalschen Namen auf den Filenamen, falls
    vorhanden.
 */

/*N*/ {
/*N*/ 	Reference< XOutputStream > rOutStream;
/*N*/     pImp->pVersions = NULL;
/*N*/
/*N*/     SFX_ITEMSET_ARG( pSet, pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False);
/*N*/     if( aLogicName.Len() )
/*N*/     {
/*N*/         INetURLObject aUrl( aLogicName );
/*N*/         INetProtocol eProt = aUrl.GetProtocol();
/*N*/         if ( eProt == INET_PROT_NOT_VALID )
/*N*/         {
/*?*/             DBG_ERROR ( "Unknown protocol!" );
/*N*/         }
/*N*/         else
/*N*/         {
/*N*/             // try to convert the URL into a physical name - but never change a physical name
/*N*/             // physical name may be set if the logical name is changed after construction
/*N*/             if ( !aName.Len() )
/*N*/                 ::utl::LocalFileHelper::ConvertURLToPhysicalName( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ), aName );
/*N*/             else
/*?*/                 DBG_ASSERT( pSalvageItem, "Suspicious change of logical name!" );
/*N*/         }
/*N*/     }
/*N*/
/*N*/     if ( pSalvageItem && pSalvageItem->GetValue().Len() )
/*N*/         aLogicName = pSalvageItem->GetValue();
/*N*/
/*N*/ 	// in case output stream is by mistake here
/*N*/ 	// clear the reference
/*N*/     SFX_ITEMSET_ARG( pSet, pOutStreamItem, SfxUnoAnyItem, SID_OUTPUTSTREAM, sal_False);
/*N*/ 	if( pOutStreamItem
/*N*/ 	 && ( !( pOutStreamItem->GetValue() >>= rOutStream )
/*N*/           || !aLogicName.CompareToAscii( "private:stream", 14 ) == COMPARE_EQUAL ) )
/*N*/ 	{
/*?*/ 		pSet->ClearItem( SID_OUTPUTSTREAM );
/*?*/ 		DBG_ERROR( "Unexpected Output stream parameter!\n" );
/*N*/ 	}
/*N*/
/*N*/     SetIsRemote_Impl();
/*N*/ }

//------------------------------------------------------------------
/*N*/ SfxMedium::SfxMedium()
/*N*/ :   IMPL_CTOR(),
/*N*/     bRoot( sal_False ),
/*N*/     pURLObj(0),
/*N*/
/*N*/     pSet(0),
/*N*/     pImp(new SfxMedium_Impl( this )),
/*N*/     pFilter(0)
/*N*/ {
/*N*/     Init_Impl();
/*N*/ }
//------------------------------------------------------------------

/*N*/ void SfxMedium::UseInteractionHandler( BOOL bUse )
/*N*/ {
/*N*/     pImp->bAllowDefaultIntHdl = bUse;
/*N*/ }

//------------------------------------------------------------------

/*N*/ ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >
/*N*/ SfxMedium::GetInteractionHandler()
/*N*/ {
/*N*/     // if interaction isnt allowed explicitly ... return empty reference!
/*N*/     if ( !pImp->bUseInteractionHandler )
/*?*/         return ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >();
/*N*/
/*N*/     // search a possible existing handler inside cached item set
/*N*/     if ( pSet )
/*N*/     {
/*N*/         ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > xHandler;
/*N*/         SFX_ITEMSET_ARG( pSet, pHandler, SfxUnoAnyItem, SID_INTERACTIONHANDLER, sal_False);
/*N*/         if ( pHandler && (pHandler->GetValue() >>= xHandler) && xHandler.is() )
/*N*/             return xHandler;
/*N*/     }
/*N*/
/*N*/     // if default interaction isnt allowed explicitly ... return empty reference!
/*N*/     if ( !pImp->bAllowDefaultIntHdl )
/*N*/         return ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >();
/*N*/
/*N*/     // otherwhise return cached default handler ... if it exist.
/*?*/     if ( pImp->xInteraction.is() )
/*?*/         return pImp->xInteraction;
/*?*/
/*?*/     // create default handler and cache it!
/*?*/     ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory = ::legacy_binfilters::getLegacyProcessServiceFactory();
/*?*/     if ( xFactory.is() )
/*?*/     {
/*?*/         pImp->xInteraction = ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >( xFactory->createInstance( DEFINE_CONST_UNICODE("com.sun.star.task.InteractionHandler") ), ::com::sun::star::uno::UNO_QUERY );
/*?*/         return pImp->xInteraction;
/*?*/     }
/*?*/
/*?*/ 	return ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >();
/*N*/ }

//------------------------------------------------------------------

/*N*/ void SfxMedium::SetFilter( const SfxObjectFactory& rFact, const String & rFilter )
/*N*/ {
/*N*/     SetFilter(  rFact.GetFilterContainer()->GetFilter(rFilter) );
/*N*/ }
//----------------------------------------------------------------

/*N*/ void SfxMedium::SetFilter( const SfxFilter* pFilterP, sal_Bool bResetOrig )
/*N*/ {
/*N*/     pFilter = pFilterP;
/*N*/     pImp->nFileVersion = 0;
/*N*/ }
//----------------------------------------------------------------

/*N*/ void SfxMedium::Close()
/*N*/ {
/*N*/     if ( aStorage.Is() )
/*N*/     {
/*N*/         // don't close the streams if they belong to the
/*N*/         // storage
/*N*/
/*N*/         const SvStream *pStream = aStorage->GetSvStream();
/*N*/         if ( pStream && pStream == pInStream )
/*N*/         {
/*N*/             pInStream = NULL;
/*N*/             pImp->xInputStream = Reference < XInputStream >();
/*N*/             pImp->xLockBytes.Clear();
/*N*/             if ( pSet )
/*N*/                 pSet->ClearItem( SID_INPUTSTREAM );
/*N*/             aStorage->SetDeleteStream( TRUE );
/*N*/         }
/*N*/         else if ( pStream && pStream == pOutStream )
/*N*/         {
/*?*/             pOutStream = NULL;
/*?*/             aStorage->SetDeleteStream( TRUE );
/*N*/         }
/*N*/
/*N*/         CloseStorage();
/*N*/     }
/*N*/
/*N*/     if ( pInStream )
/*N*/         CloseInStream_Impl();
/*N*/
/*N*/     if ( pOutStream )
/*?*/         CloseOutStream_Impl();
/*N*/
/*N*/     if ( pSet )
/*N*/         pSet->ClearItem( SID_CONTENT );
/*N*/
/*N*/     pImp->aContent = ::ucbhelper::Content();
/*N*/ }

//------------------------------------------------------------------


/*N*/ void SfxMedium::SetIsRemote_Impl()
/*N*/ {
/*N*/     INetURLObject aObj( GetName() );
/*N*/     switch( aObj.GetProtocol() )
/*N*/     {
/*N*/         case INET_PROT_FTP:
/*N*/         case INET_PROT_HTTP:
/*N*/         case INET_PROT_HTTPS:
/*N*/         case INET_PROT_POP3:
/*N*/         case INET_PROT_NEWS:
/*N*/         case INET_PROT_IMAP:
/*N*/ //        case INET_PROT_OUT:
/*N*/         case INET_PROT_VIM:
/*N*/             bRemote = TRUE; break;
/*N*/         default:
/*N*/             bRemote = ( GetName().CompareToAscii( "private:msgid", 13 ) == COMPARE_EQUAL );
/*N*/             break;
/*N*/     }
/*N*/
/*N*/     // Da Dateien, die Remote geschrieben werden zur Uebertragung auch
/*N*/     // gelesen werden koennen muessen
/*N*/     if( bRemote )
/*N*/         nStorOpenMode |= STREAM_READ;
/*N*/ }




//----------------------------------------------------------------
/*N*/ const String& SfxMedium::GetOrigURL() const
/*N*/ {
/*N*/     return !pImp->aOrigURL.Len() ? (String &)aLogicName : pImp->aOrigURL;
/*N*/ }

//----------------------------------------------------------------

/*N*/ void SfxMedium::SetPhysicalName_Impl( const String& rNameP )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ }

//----------------------------------------------------------------

//------------------------------------------------------------------

//------------------------------------------------------------------
/*N*/ sal_Bool SfxMedium::IsTemporary() const
/*N*/ {
/*N*/     return pImp->bIsTemp;
/*N*/ }

//------------------------------------------------------------------

/*N*/ void SfxMedium::ReOpen()
/*N*/ {
/*N*/     BOOL bUseInteractionHandler = pImp->bUseInteractionHandler;
/*N*/     pImp->bUseInteractionHandler = FALSE;
/*N*/     DBG_ASSERT( pFilter, "Kein Filter, aber ReOpen!" );
/*N*/     if( pFilter )
/*N*/     {
/*N*/         if( pFilter->UsesStorage() )
/*N*/             GetStorage();
/*N*/         else
/*N*/             GetInStream();
/*N*/     }
/*N*/
/*N*/     pImp->bUseInteractionHandler = bUseInteractionHandler;
/*N*/ }
//------------------------------------------------------------------
/*N*/ SfxMedium::SfxMedium
/*N*/ (
/*N*/     const String &rName, StreamMode nOpenMode,  sal_Bool bDirectP,
/*N*/     const SfxFilter *pFlt, SfxItemSet *pInSet
/*N*/ )
/*N*/ :   IMPL_CTOR(),
/*N*/     bRoot( sal_False ),
/*N*/     pFilter(pFlt),
/*N*/     pURLObj(0),
/*N*/     pImp(new SfxMedium_Impl( this )),
/*N*/     pSet( pInSet )
/*N*/ {
/*N*/     aLogicName = rName;
/*N*/     nStorOpenMode = nOpenMode;
/*N*/     bDirect = bDirectP;
/*N*/     Init_Impl();
/*N*/ }
//------------------------------------------------------------------

/*N*/ SfxMedium::SfxMedium( SvStorage *pStorage, sal_Bool bRootP )
/*N*/ :   IMPL_CTOR(),
/*N*/     bRoot( bRootP ),
/*N*/     aStorage(pStorage),
/*N*/     pURLObj(0),
/*N*/     pImp( new SfxMedium_Impl( this )),
/*N*/     pSet(0)
/*N*/ {
/*N*/     SfxApplication* pApp = SFX_APP();
/*N*/     sal_uInt32 nFormat = pStorage->GetFormat();
/*N*/     if( !nFormat )
/*N*/     {
/*N*/ #ifdef DBG_UTIL
/*N*/         if( aLogicName.Len() )
/*N*/             DBG_ERROR( "Unbekanntes StorageFormat, versuche eigenes Format" );
/*N*/ #endif
/*N*/         pFilter = SfxObjectFactory::GetDefaultFactory().GetFilterContainer()->
/*N*/             GetFilter( 0 );
/*N*/     }
/*N*/     else
/*N*/         pFilter = pApp->GetFilterMatcher().GetFilter4ClipBoardId( nFormat, 0, 0 );
/*N*/
/*N*/     Init_Impl();
/*N*/
/*N*/     if( !pFilter && nFormat )
/*N*/     {
/*STRIP003*/ pApp->GetFilterMatcher().GetFilter4Content( *this, &pFilter );  // #91292# PowerPoint does not support an OleComp stream,
/*STRIP003*/ if ( !pFilter )                                                 // so GetFilter4ClipBoardId is not able to detect the format,
/*STRIP003*/ {                                                               // for such cases we try to get the filter by GetFilter4Content
/*STRIP003*/	DBG_ERROR( "No Filter for storage found!" );
/*STRIP003*/	pFilter = SfxObjectFactory::GetDefaultFactory().GetFilterContainer()->GetFilter( 0 );
/*STRIP003*/ }
/*N*/     }
/*N*/ }

//------------------------------------------------------------------

/*N*/ SfxMedium::~SfxMedium()
/*N*/ {
/*N*/     /* Attention
        Don't enable CancelTransfers() till you know that the writer/web has changed his asynchronous load
        behaviour. Otherwhise may StyleSheets inside a html file will be loaded at the right time.
        => further the help will be empty then ... #100490#
     */
/*N*/     //CancelTransfers();
/*N*/     ::osl::ClearableMutexGuard aGuard( pImp->aHandler->GetMutex() );
/*N*/     pImp->aHandler->ReleaseMedium();
/*N*/     aGuard.clear();
/*N*/
/*N*/     Close();
/*N*/
/*N*/     delete pSet;
/*N*/
/*N*/     #ifdef  DBG_UTIL
/*N*/     if( pImp->bIsTemp && aName.Len() )
/*N*/     {
/*N*/         String aTemp;
/*N*/         if ( !::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aTemp ))
/*N*/ 			DBG_ERROR("Physical name not convertable!");
/*N*/
/*N*/         if ( !::utl::UCBContentHelper::Kill( aTemp ) )
/*N*/ 			DBG_ERROR("Couldn't remove temporary file!");
/*N*/     }
/*N*/     #endif
/*N*/
/*N*/     pFilter = 0;
/*N*/
/*N*/     delete pURLObj;
/*N*/     delete pImp;
/*N*/ }
//------------------------------------------------------------------

/*N*/ const INetURLObject& SfxMedium::GetURLObject() const
/*N*/ {
/*N*/     if( !pURLObj )
/*N*/     {
/*N*/         SfxMedium* pThis = const_cast < SfxMedium* > (this);
/*N*/         pThis->pURLObj = new INetURLObject( aLogicName );
/*N*/ 		if ( pThis->pURLObj->HasMark() )
/*N*/ 			(*pThis->pURLObj) = INetURLObject( aLogicName ).GetURLNoMark();
/*N*/     }
/*N*/
/*N*/     return *pURLObj;
/*N*/ }

//----------------------------------------------------------------

//----------------------------------------------------------------

/*N*/ sal_uInt32 SfxMedium::GetMIMEAndRedirect( String &rName )
/*N*/ {
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
/*N*/     return 0;
/*N*/ }

//----------------------------------------------------------------

/*N*/ void SfxMedium::SetUsesCache( sal_Bool bUse )
/*N*/ {
/*N*/     pImp->bUsesCache = bUse;
/*N*/ }
//----------------------------------------------------------------

/*N*/ sal_Bool SfxMedium::UsesCache() const
/*N*/ {
/*N*/     return pImp->bUsesCache;
/*N*/ }
//----------------------------------------------------------------

//----------------------------------------------------------------

/*N*/ void SfxMedium::SetReferer( const String& rRefer )
/*N*/ {
/*N*/     pImp->aReferer = rRefer;
/*N*/ }
//----------------------------------------------------------------

//----------------------------------------------------------------

/*N*/ void SfxMedium::SetTransferPriority( sal_uInt16 nPrio )
/*N*/ {
/*N*/     pImp->nPrio = nPrio;
/*N*/ }
//----------------------------------------------------------------

//----------------------------------------------------------------

//----------------------------------------------------------------

//----------------------------------------------------------------

/*N*/ void SfxMedium::ForceSynchronStream_Impl( sal_Bool bForce )
/*N*/ {
/*N*/     if( pInStream )
/*N*/     {
/*N*/         SvLockBytes* pBytes = pInStream->GetLockBytes();
/*N*/         if( pBytes )
/*N*/             pBytes->SetSynchronMode( bForce );
/*N*/     }
/*N*/     pImp->bForceSynchron = bForce;
/*N*/ }
//----------------------------------------------------------------
/* Kann der URL ein MIME Type zugeordnent werden? */
/*N*/ sal_Bool SfxMedium::SupportsMIME_Impl() const
/*N*/ {
/*N*/     INetProtocol eProt = GetURLObject().GetProtocol();
/*N*/     if( eProt == INET_PROT_HTTPS || eProt == INET_PROT_HTTP  )
/*N*/         return sal_True;
/*N*/
/*N*/     if( eProt == INET_PROT_NOT_VALID )
/*N*/         return sal_False;
/*N*/
/*N*/     if( eProt == INET_PROT_FTP )
/*N*/     {
/*N*/         try
/*N*/         {
/*?*/             Any aAny = pImp->aContent.getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsFolder")) );
/*?*/             sal_Bool bIsFolder = FALSE;
/*?*/             if ( ( aAny >>= bIsFolder ) && bIsFolder )
/*?*/                 return SvBinding::ShouldUseFtpProxy( GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
/*?*/         }
/*?*/         catch ( ::com::sun::star::uno::Exception& )
/*?*/         {
/*?*/         }
/*N*/     }
/*N*/
/*N*/     return sal_False;
/*N*/ }

//----------------------------------------------------------------

/*N*/ SfxItemSet* SfxMedium::GetItemSet() const
/*N*/ {
/*N*/     if( !pSet ) ((SfxMedium*)this)->pSet =
/*N*/                     new SfxAllItemSet( SFX_APP()->GetPool() );
/*N*/     return pSet;
/*N*/ }

/*N*/ SvKeyValueIterator* SfxMedium::GetHeaderAttributes_Impl()
/*N*/ {
/*N*/     if( !pImp->xAttributes.Is() )
/*N*/ 	{
/*N*/ 		pImp->xAttributes = SvKeyValueIteratorRef( new SvKeyValueIterator );
/*N*/
/*N*/ 		if ( GetContent().is() )
/*N*/ 		{
/*?*/ 			pImp->bIsCharsetInitialized = sal_True;
/*?*/
/*?*/ 			try
/*?*/ 			{
/*?*/ 				Any aAny = pImp->aContent.getPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ) );
/*?*/ 				::rtl::OUString aContentType;
/*?*/ 				aAny >>= aContentType;
/*?*/
/*?*/ 				pImp->xAttributes->Append( SvKeyValue( ::rtl::OUString::createFromAscii( "content-type" ), aContentType ) );
/*?*/ 			}
/*?*/ 			catch ( ::com::sun::star::uno::Exception& )
/*?*/ 			{
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/
/*N*/     return pImp->xAttributes;
/*N*/ }
//----------------------------------------------------------------

/*N*/ SvCompatWeakHdl* SfxMedium::GetHdl()
/*N*/ {
/*N*/     return pImp->GetHdl();
/*N*/ }

/*N*/ sal_Bool SfxMedium::IsDownloadDone_Impl()
/*N*/ {
/*N*/     return pImp->bDownloadDone;
/*N*/ }

//----------------------------------------------------------------

/*N*/ void SfxMedium::SetDontCreateCancellable( )
/*N*/ {
/*N*/     pImp->bDontCreateCancellable = sal_True;
/*N*/ }

/*N*/ ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >  SfxMedium::GetInputStream()
/*N*/ {
/*N*/ 	if ( !pImp->xInputStream.is() )
/*N*/ 	{
/*N*/ 		// if pInStream is already opened then we have problem
/*N*/ 		// probably GetInStream should allways open pInStream based on xInputStream
/*N*/ 		GetMedium_Impl();
/*N*/ 	}
/*N*/
/*N*/     return pImp->xInputStream;
/*N*/ }

/*N*/ const SfxVersionTableDtor* SfxMedium::GetVersionList()
/*N*/ {
/*N*/     if ( !pImp->pVersions && GetStorage() )
/*N*/     {
/*N*/         if ( pImp->bIsDiskSpannedJAR )
/*N*/             return NULL;
/*N*/
/*N*/         SvStorageStreamRef aStream =
/*N*/             GetStorage()->OpenStream( DEFINE_CONST_UNICODE( "VersionList" ), SFX_STREAM_READONLY | STREAM_NOCREATE );
/*N*/         if ( aStream.Is() && aStream->GetError() == SVSTREAM_OK )
/*N*/         {
/*?*/             pImp->pVersions = new SfxVersionTableDtor;
/*?*/             pImp->pVersions->Read( *aStream );
/*N*/         }
/*N*/         else
/*N*/         {
/*N*/ 			SfxVersionTableDtor *pList = new SfxVersionTableDtor;
/*N*/ 			if ( SfxXMLVersList_Impl::ReadInfo( GetStorage(), pList ) )
/*?*/ 			    pImp->pVersions = pList;
/*N*/ 			else
/*N*/ 			    delete pList;
/*N*/         }
/*N*/     }
/*N*/
/*N*/     return pImp->pVersions;
/*N*/ }







//----------------------------------------------------------------
/*N*/ sal_Bool SfxMedium::IsReadOnly()
/*N*/ {
/*N*/     sal_Bool bReadOnly = !( GetOpenMode() & STREAM_WRITE );
/*(dv)  if ( bReadOnly && pURLObj && CntAnchor::IsViewURL( pURLObj->GetMainURL( INetURLObject::NO_DECODE ) ) )
        // Chaos-Storages sind niemals als readonly anzusehen!
        return sal_False;
*/
/*N*/     if ( !bReadOnly )
/*N*/     {
/*N*/         // logisch readonly ge"offnet
/*N*/         SFX_ITEMSET_ARG( GetItemSet(), pItem, SfxBoolItem, SID_DOC_READONLY, sal_False);
/*N*/         if ( pItem )
/*N*/             bReadOnly = pItem->GetValue();
/*N*/     }
/*N*/
/*N*/     return bReadOnly;
/*N*/ }

//----------------------------------------------------------------

//----------------------------------------------------------------
/*N*/ void SfxMedium::CreateTempFile()
/*N*/ {
/*N*/     if ( pImp->pTempFile )
/*N*/         DELETEZ( pImp->pTempFile );
/*N*/
/*N*/     StreamMode nOpenMode = nStorOpenMode;
/*N*/     GetInStream();
/*N*/     BOOL bCopy = ( nStorOpenMode == nOpenMode && ! ( nOpenMode & STREAM_TRUNC ) );
/*N*/     nStorOpenMode = nOpenMode;
/*N*/     ResetError();
/*N*/ 
/*N*/     pImp->pTempFile = new ::utl::TempFile();
/*N*/     pImp->pTempFile->EnableKillingFile( sal_True );
/*N*/     aName = pImp->pTempFile->GetFileName();
/*N*/     if ( !aName.Len() )
/*N*/     {
/*N*/         SetError( ERRCODE_IO_CANTWRITE );
/*N*/         return;
/*N*/     }
/*N*/ 
/*N*/     if ( bCopy )
/*N*/     {
/*N*/         GetOutStream();
/*N*/         if ( pInStream && pOutStream )
/*N*/         {
/*N*/             char        *pBuf = new char [8192];
/*N*/             sal_uInt32   nErr = ERRCODE_NONE;
/*N*/ 
/*N*/             pInStream->Seek(0);
/*N*/             pOutStream->Seek(0);
/*N*/ 
/*N*/             while( !pInStream->IsEof() && nErr == ERRCODE_NONE )
/*N*/             {
/*N*/                 sal_uInt32 nRead = pInStream->Read( pBuf, 8192 );
/*N*/                 nErr = pInStream->GetError();
/*N*/                 pOutStream->Write( pBuf, nRead );
/*N*/             }
/*N*/ 
/*N*/             delete[] pBuf;
/*N*/             CloseInStream();
/*N*/         }
/*N*/         CloseOutStream_Impl();
/*N*/     }
/*N*/     else
/*?*/         CloseInStream();
/*N*/ 
/*N*/     CloseStorage();
/*N*/ }

//----------------------------------------------------------------
/*N*/ void SfxMedium::CreateTempFileNoCopy()
/*N*/ {
/*N*/     if ( pImp->pTempFile )
/*N*/         delete pImp->pTempFile;
/*N*/ 
/*N*/     pImp->pTempFile = new ::utl::TempFile();
/*N*/     pImp->pTempFile->EnableKillingFile( sal_True );
/*N*/     aName = pImp->pTempFile->GetFileName();
/*N*/     if ( !aName.Len() )
/*N*/     {
/*?*/         SetError( ERRCODE_IO_CANTWRITE );
/*?*/         return;
/*N*/     }
/*N*/ 
/*N*/     CloseOutStream_Impl();
/*N*/     CloseStorage();
/*N*/ }

//----------------------------------------------------------------
#define nActVersion 1

/*?*/ SvStream& SfxVersionTableDtor::Read( SvStream& rStrm )
/*?*/ {
/*?*/     sal_uInt16 nCount = 0, nVersion = 0;
/*?*/ 
/*?*/     rStrm >> nVersion;
/*?*/     rStrm >> nCount;
/*?*/ 
/*?*/     for( sal_uInt16 i=0; i<nCount; ++i )
/*?*/     {
/*?*/         SfxVersionInfo *pNew = new SfxVersionInfo;
/*?*/         rStrm.ReadByteString( pNew->aComment, RTL_TEXTENCODING_UTF8 );
/*?*/         rStrm.ReadByteString( pNew->aName, RTL_TEXTENCODING_UTF8 );
/*?*/         pNew->aCreateStamp.Load( rStrm );
/*?*/         Insert( pNew, LIST_APPEND );
/*?*/     }
/*?*/ 
/*?*/     return rStrm;
/*?*/ }

/*N*/ void SfxVersionTableDtor::DelDtor()
/*N*/ {
/*N*/     SfxVersionInfo* pTmp = First();
/*N*/     while( pTmp )
/*N*/     {
/*?*/         delete pTmp;
/*?*/         pTmp = Next();
/*N*/     }
/*N*/     Clear();
/*N*/ }


//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
/*?*/ SfxVersionInfo::SfxVersionInfo()
/*?*/ {
/*?*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
