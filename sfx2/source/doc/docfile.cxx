/*************************************************************************
 *
 *  $RCSfile: docfile.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: dv $ $Date: 2001-02-15 13:55:27 $
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

#include "docfile.hxx"

#include <uno/mapping.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/XContent.hpp>

#ifndef _COM_SUN_STAR_UTIL_XARCHIVER_HPP_
#include <com/sun/star/util/XArchiver.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATACONTROL_HPP_
#include <com/sun/star/io/XActiveDataControl.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSTREAMLISTENER_HPP_
#include <com/sun/star/io/XStreamListener.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
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
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _EXTATTR_HXX
#include <svtools/extattr.hxx>
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

#define _SVSTDARR_ULONGS
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

#pragma hdrstop

#include <so3/transbnd.hxx> // SvKeyValueIterator
#include <tools/urlobj.hxx>
#include <unotools/ucblockbytes.hxx>
#include <svtools/pathoptions.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <unotools/localfilehelper.hxx>

#include "ucbhelp.hxx"
#include "helper.hxx"
#include "request.hxx"      // SFX_ITEMSET_SET
#include "app.hxx"          // GetFilterMatcher
#include "frame.hxx"        // LoadTargetFrame
#include "fltfnc.hxx"       // SfxFilterMatcher
#include "sfxtypes.hxx"     // AsynchronLink
#include "docfilt.hxx"      // SfxFilter
#include "objsh.hxx"        // CheckOpenMode
#include "docfac.hxx"       // GetFilterContainer
#include "doc.hrc"          // MSG_WARNING_BACKUP, MSG_OPEN_READONLY
#include "openflag.hxx"     // SFX_STREAM_READONLY etc.
#include "sfxresid.hxx"
#if SUPD<613//MUSTINI
#include "inimgr.hxx"       // Backup Path
#endif

#define MAX_REDIRECT 5

class SfxLockBytesHandler_Impl : public ::utl::UcbLockBytesHandler
{
    ULONG           m_nAcquireCount;
    SfxMedium*      m_pMedium;
    ::vos::OMutex   m_aMutex;
public:
                    SfxLockBytesHandler_Impl( SfxMedium* pMedium )
                        : m_pMedium( pMedium )
                        , m_nAcquireCount( 0 )
                    {}

    virtual void    Handle( ::utl::UcbLockBytesHandler::LoadHandlerItem nWhich, ::utl::UcbLockBytesRef xLockBytes );
    ::vos::OMutex&  GetMutex()
                    { return m_aMutex; }
    void            ReleaseMedium()
                    { m_pMedium = NULL; }
};

SV_DECL_IMPL_REF( SfxLockBytesHandler_Impl );

void SfxLockBytesHandler_Impl::Handle( ::utl::UcbLockBytesHandler::LoadHandlerItem nWhich, ::utl::UcbLockBytesRef xLockBytes )
{
    ::vos::OGuard aGuard( m_aMutex );
    if ( IsActive() && xLockBytes.Is()&& m_pMedium )
    {
        switch( nWhich )
        {
            case BEFOREWAIT :
                if ( xLockBytes->IsSynchronMode() )
                {
                    DBG_ASSERT( !m_nAcquireCount, "BeforeWait inside beforewait!" );
                    ::vos::IMutex& rMutex = Application::GetSolarMutex();
                    if ( rMutex.tryToAcquire() )
                        m_nAcquireCount = Application::ReleaseSolarMutex() - 1;
                }

                break;
            case AFTERWAIT :
                if ( xLockBytes->IsSynchronMode() )
                {
                    if ( m_nAcquireCount )
                    {
                        Application::AcquireSolarMutex( m_nAcquireCount );
                        m_nAcquireCount = 0;
                    }
                }

                break;

            case DATA_AVAILABLE :
                m_pMedium->DataAvailable_Impl();
                break;
            case DONE :
                m_pMedium->Done_Impl( xLockBytes->GetError() );
                break;
            case CANCEL :
                m_pMedium->Cancel_Impl();
                break;
            default:
                break;
        }
    }
}

class UcbLockBytesCancellable_Impl : public SfxCancellable
{
    ::utl::UcbLockBytesRef         xLockBytes;

public:
                            UcbLockBytesCancellable_Impl( const ::utl::UcbLockBytesRef& rLockBytes, SfxCancelManager* pManager, const String& rTitle )
                                : SfxCancellable( pManager, rTitle )
                                , xLockBytes( rLockBytes )
                            {}

    virtual void            Cancel();
};

void UcbLockBytesCancellable_Impl::Cancel()
{
    xLockBytes->Cancel();
}

class FileSource_Impl   :   public ::com::sun::star::lang::XTypeProvider    ,
                            public ::com::sun::star::io::XActiveDataSource  ,
                            public ::com::sun::star::io::XActiveDataControl ,
                            public ::com::sun::star::lang::XInitialization  ,
                            public ::cppu::OWeakObject
{
private:
    SvStream*           pStream;
    SfxMedium*          pMedium;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener >   m_xListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >     m_xSink;

public:

    SFX_DECL_XINTERFACE_XTYPEPROVIDER

                    FileSource_Impl();
                    FileSource_Impl( SfxMedium* );
    virtual         ~FileSource_Impl();

    void            ResetMedium()
                    { pMedium = NULL; pStream = NULL;}
    DECL_LINK(      DataAvailableHdl, void* );

                    // ::com::sun::star::io::XActiveDataControl
    virtual void SAL_CALL   addListener(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL   removeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL   start(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL   terminate(void) throw( ::com::sun::star::uno::RuntimeException );

                    // ::com::sun::star::io::XActiveDataSource
    virtual void SAL_CALL   setOutputStream(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > & aStream) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > SAL_CALL    getOutputStream(void) throw( ::com::sun::star::uno::RuntimeException );

                    // ::com::sun::star::lang::XInitialization
    virtual void SAL_CALL   initialize(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments) throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException );

};

class FileSink_Impl :   public ::com::sun::star::lang::XTypeProvider    ,
                        public ::com::sun::star::io::XOutputStream      ,
                        public ::cppu::OWeakObject
{
private:
    SvStream*       pStream;
    SfxMedium*      pMedium;

public:
    SFX_DECL_XINTERFACE_XTYPEPROVIDER

                    FileSink_Impl( SfxMedium* );
    virtual         ~FileSink_Impl();

    void            ResetMedium()
                    { pMedium = NULL; pStream = NULL;}

                    // ::com::sun::star::io::XOutputStream
    virtual void  SAL_CALL  writeBytes(const ::com::sun::star::uno::Sequence< sal_Int8 >& aData) throw( ::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::uno::RuntimeException );
    virtual void  SAL_CALL  flush(void) throw( ::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::uno::RuntimeException );
    virtual void  SAL_CALL  closeOutput(void) throw( ::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::uno::RuntimeException );
};

SFX_IMPL_XINTERFACE_1( FileSink_Impl, OWeakObject, ::com::sun::star::io::XOutputStream )
SFX_IMPL_XTYPEPROVIDER_1( FileSink_Impl, ::com::sun::star::io::XOutputStream )

FileSink_Impl::FileSink_Impl( SfxMedium* pMed )
    : pMedium( pMed )
    , pStream( NULL )
{
}

FileSink_Impl::~FileSink_Impl()
{
    if ( pMedium )
    {
        pMedium->ResetDataSink();
        pMedium->ReleaseRef();
    }
}

void SAL_CALL FileSink_Impl::writeBytes(const ::com::sun::star::uno::Sequence< sal_Int8 >& Buffer) throw( ::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::uno::RuntimeException )
{
    if ( !pStream && pMedium )
        pStream = pMedium->GetOutStream();

    if ( pStream )
        pStream->Write( Buffer.getConstArray(), (sal_uInt32) Buffer.getLength() );
}

void SAL_CALL  FileSink_Impl::flush(void) throw( ::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::uno::RuntimeException )
{
    if ( pMedium )
        pMedium->GetOutStream()->Flush();
}

void SAL_CALL  FileSink_Impl::closeOutput(void) throw( ::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::uno::RuntimeException )
{
    if ( pMedium )
        pMedium->Close();
}

SFX_IMPL_XINTERFACE_3( FileSource_Impl, OWeakObject, ::com::sun::star::io::XActiveDataSource, ::com::sun::star::io::XActiveDataControl, ::com::sun::star::lang::XInitialization )
SFX_IMPL_XTYPEPROVIDER_3( FileSource_Impl, ::com::sun::star::io::XActiveDataSource, ::com::sun::star::io::XActiveDataControl, ::com::sun::star::lang::XInitialization )

FileSource_Impl::FileSource_Impl()
    : pMedium( NULL )
    , pStream( NULL )
{
}

void SAL_CALL  FileSource_Impl::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments) throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException )
{
    const ::com::sun::star::uno::Any *pArr = Arguments.getConstArray();
    ::rtl::OUString aName ;
    pArr[0] >>= aName ;
    if ( aName.len() )
    {
        pMedium = new SfxMedium( aName, STREAM_STD_READ, sal_True );
        pMedium->SetTransferPriority( SFX_TFPRIO_SYNCHRON );
        pMedium->SetDataAvailableLink( LINK( this, FileSource_Impl, DataAvailableHdl ) );
        pMedium->SetDoneLink( LINK( this, FileSource_Impl, DataAvailableHdl ) );
        pMedium->AddRef();
    }
}

FileSource_Impl::FileSource_Impl( SfxMedium* pMed )
    : pMedium( pMed )
    , pStream( NULL )
{
}

FileSource_Impl::~FileSource_Impl()
{
    if ( pMedium )
    {
        pMedium->ResetDataSource();
        pMedium->ReleaseRef();
    }
}

void SAL_CALL  FileSource_Impl::addListener(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
{
    if( m_xListener.is() )
        DBG_ERROR( "addSourceControllerListener called when already having a listener!" );
    m_xListener = aListener;
}

void SAL_CALL  FileSource_Impl::removeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
{
    m_xListener = ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener > ();
}

void SAL_CALL  FileSource_Impl::setOutputStream( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > & Listener )throw( ::com::sun::star::uno::RuntimeException )
{
    m_xSink = Listener;
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >  SAL_CALL  FileSource_Impl::getOutputStream(void) throw( ::com::sun::star::uno::RuntimeException )
{
    return m_xSink;
}

IMPL_LINK( FileSource_Impl, DataAvailableHdl, void*, pVoid )
{
    if ( !pStream )
        pStream = pMedium->GetInStream();

    if ( pStream && m_xSink.is() )
    {
        sal_Int8 buf[ 65536 ];
        sal_uInt32 nBytes = 1;
        while( nBytes && pStream->GetError() != ERRCODE_IO_PENDING )
        {
            nBytes = pStream->Read( buf, (sal_uInt32) sizeof( buf ) );
            if ( nBytes )
                m_xSink->writeBytes( ::com::sun::star::uno::Sequence<sal_Int8>( buf, nBytes ) );
        }

        if ( pStream->GetError() == ERRCODE_IO_PENDING )
            pStream->ResetError();
        else
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSource >  xRef( this );
            m_xSink->closeOutput();
            m_xSink = ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > ();
            if( m_xListener.is() )
                m_xListener->closed();
            m_xListener = ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener > ();
            pMedium->GetItemSet()->ClearItem( SID_LOADENVIRONMENT );
        }
    }

    return 0;
}

void SAL_CALL  FileSource_Impl::start() throw( ::com::sun::star::uno::RuntimeException )
{
    pStream = pMedium->GetInStream();
    if ( pStream && m_xSink.is() )
        DataAvailableHdl( 0 );
}

void SAL_CALL  FileSource_Impl::terminate() throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSource >  xRef( this );
    if ( m_xSink.is() )
        m_xSink->closeOutput();

    m_xSink = ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > ();
    if( m_xListener.is() )
        m_xListener->closed();
    m_xListener = ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener > ();
    pMedium->CancelTransfers();
    pMedium->GetItemSet()->ClearItem( SID_LOADENVIRONMENT );
    pMedium->Close();
}

String ConvertDateTime_Impl(const SfxStamp &rTime);

//----------------------------------------------------------------
SfxPoolCancelManager::SfxPoolCancelManager( SfxCancelManager* pParent, const String& rName )
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
SfxPoolCancelManager::~SfxPoolCancelManager()
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
void SfxPoolCancelManager::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if( !GetCancellableCount() ) SetManager( 0 );
    else if( !GetManager() )
    {
        if( !wParent.Is() ) wParent = SFX_APP()->GetCancelManager();
        SetManager( wParent );
    }
}

//----------------------------------------------------------------
void SfxPoolCancelManager::Cancel()
{
    SfxPoolCancelManagerRef xThis = this;
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
    Reference < XContent > xContent;
    sal_Bool bUpdatePickList : 1;
    sal_Bool bIsTemp        : 1;
    sal_Bool bUsesCache     : 1;
    sal_Bool bForceSynchron : 1;
    sal_Bool bDontCreateCancellable : 1;
    sal_Bool bDownloadDone          : 1;
    sal_Bool bDontCallDoneLinkOnSharingError : 1;
    sal_Bool bStreamReady: 1;
    sal_Bool bIsStorage: 1;

    sal_uInt16       nPrio;

    SfxPoolCancelManagerRef xCancelManager;
    UcbLockBytesCancellable_Impl* pCancellable;
    SfxMedium*       pAntiImpl;
    SvEaMgr*         pEaMgr;

    long             nFileVersion;

    const SfxFilter* pOrigFilter;
    String           aOrigURL;
    String           aPreRedirectionURL;
    String           aReferer;
    DateTime         aExpireTime;
    SfxFrameWeak     wLoadTargetFrame;
    LoadEnvironment_Impl* pLoadEnv;
    SvKeyValueIteratorRef xAttributes;

    AsynchronLink       aDoneLink;
    AsynchronLink       aAvailableLink;
    SfxLockBytesHandler_ImplRef  aHandler;

    SfxVersionTableDtor*    pVersions;
    FileSource_Impl*    pSource;
    FileSink_Impl*      pSink;
    ::utl::TempFile*           pTempDir;
    ::utl::TempFile*           pTempFile;

    SfxPoolCancelManager* GetCancelManager();

    SfxMedium_Impl( SfxMedium* pAntiImplP );
    ~SfxMedium_Impl();
};

void SfxMedium::Done_Impl( ErrCode nError )
{
    DELETEZ( pImp->pCancellable );
    pImp->bDownloadDone = sal_True;
    SetError( nError );
    if ( ( !nError || !pImp->bDontCallDoneLinkOnSharingError ) && ( pImp->bStreamReady || !pInStream ) )
    {
        pImp->aDoneLink.ClearPendingCall();
        pImp->aDoneLink.Call( (void*) nError );
    }
}

void SfxMedium::DataAvailable_Impl()
{
    pImp->aAvailableLink.ClearPendingCall();
    pImp->aAvailableLink.Call( NULL );
}

void SfxMedium::Cancel_Impl()
{
    SetError( ERRCODE_IO_GENERAL );
}

SfxPoolCancelManager* SfxMedium_Impl::GetCancelManager()
{
    if( !xCancelManager.Is() )
    {
        if( !bDontCreateCancellable )
            xCancelManager = new SfxPoolCancelManager(
                wLoadTargetFrame ? wLoadTargetFrame->GetCancelManager() :
                SFX_APP()->GetCancelManager(),
                pAntiImpl->GetURLObject().GetURLNoPass() );
        else
            xCancelManager = new SfxPoolCancelManager(
                0, pAntiImpl->GetURLObject().GetURLNoPass() );
    }
    return xCancelManager;
}

//------------------------------------------------------------------
SfxMedium_Impl::SfxMedium_Impl( SfxMedium* pAntiImplP )
 :
    SvCompatWeakBase( pAntiImplP ),
    bUpdatePickList(sal_True), bIsTemp( sal_False ), pOrigFilter( 0 ),
    bUsesCache(sal_True), pCancellable( 0 ),
    nPrio( 99 ), aExpireTime( Date() + 10, Time() ),
    bForceSynchron( sal_False ), bStreamReady( sal_False ), bIsStorage( sal_False ),
    pLoadEnv( 0 ), pAntiImpl( pAntiImplP ),
    bDontCreateCancellable( sal_False ), pSource( NULL ), pSink( NULL ), pTempDir( NULL ),
    bDownloadDone( sal_True ), bDontCallDoneLinkOnSharingError( sal_False ),nFileVersion( 0 ), pEaMgr( NULL ), pTempFile( NULL )
{
    aHandler = new SfxLockBytesHandler_Impl( pAntiImpl );
    aDoneLink.CreateMutex();
}

//------------------------------------------------------------------
SfxMedium_Impl::~SfxMedium_Impl()
{
    delete pCancellable;

    if ( aHandler.Is() )
        aHandler->Activate( sal_False );

    aDoneLink.ClearPendingCall();
    aAvailableLink.ClearPendingCall();

    delete pEaMgr;
    delete pVersions;

    if ( pSource )
    {
        pSource->ResetMedium();
        pSource->release();
    }

    if ( pSink )
    {
        pSink->ResetMedium();
        pSink->release();
    }

    if ( pTempFile )
        delete pTempFile;

    if ( pTempDir )
        delete pTempDir;
}

//================================================================

#define IMPL_CTOR()                         \
     eError( SVSTREAM_OK ),                 \
                                            \
     bDirect( sal_False ),                  \
     bTriedStorage( sal_False ),            \
     bSetFilter( sal_False ),               \
                                            \
     nStorOpenMode( SFX_STREAM_READWRITE ), \
     pInStream(0),                          \
     pOutStream( 0 )

//------------------------------------------------------------------
const SvGlobalName&  SfxMedium::GetClassFilter()
{
    GetMedium_Impl();
    if( GetError() )
        return aFilterClass;
    if( !bSetFilter && GetStorage() )
        SetClassFilter( GetStorage()->GetClassName() );
    return aFilterClass;
}

//------------------------------------------------------------------
void SfxMedium::ResetError()
{
    eError = SVSTREAM_OK;
    if( aStorage.Is() )
        aStorage->ResetError();
    if( pInStream )
        pInStream->ResetError();
    if( pOutStream )
        pOutStream->ResetError();
}

//------------------------------------------------------------------
sal_uInt32 SfxMedium::GetErrorCode() const
{
    sal_uInt32 lError=eError;
    if(!lError && pInStream)
        lError=pInStream->GetErrorCode();
    if(!lError && pOutStream)
        lError=pOutStream->GetErrorCode();
    if(!lError && aStorage.Is())
        lError=aStorage->GetErrorCode();
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
    if ( !pImp->xContent.is() && GetName().Len() )
    {
        String aURL     = GetURLObject().GetMainURL();
        pImp->xContent  = UCB_Helper::CreateContent( aURL );
    }

    if ( pImp->xContent.is() )
        return pImp->xContent;
    else
        return NULL;
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

    if ( !pInStream && eError == ERRCODE_IO_PENDING )
        eError = SVSTREAM_OK;

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
    if ( pInStream && aStorage.Is() )
    {
        const SvStream *pStorage = aStorage->GetSvStream();
        if ( pStorage == pInStream )
        {
            CloseStorage();
        }
    }

    DELETEZ( pInStream );
    DELETEZ( pImp->pCancellable );
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
//            pOutStream = ::utl::UcbStreamHelper::CreateStream( pImp->pTempFile->GetURL(), nStorOpenMode );
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
        if ( aStorage.Is() )
        {
            const SvStream *pStorage = aStorage->GetSvStream();
            if ( pStorage == pOutStream )
                CloseStorage();
        }

        delete pOutStream;
        pOutStream = NULL;
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

        SvFileStream aTmpStream( aName, STREAM_STD_WRITE );
        char        *pBuf = new char [8192];
        sal_uInt32   nErr = ERRCODE_NONE;

        pInStream->Seek( 0L );
        while( !pInStream->IsEof() && nErr == ERRCODE_NONE )
        {
            sal_uInt32 nRead = pInStream->Read( pBuf, 8192 );
            nErr = pInStream->GetError();
            aTmpStream.Write( pBuf, nRead );
        }

        delete pBuf;
        pImp->bIsTemp = sal_True;
        CloseInStream_Impl();
    }
}

//------------------------------------------------------------------
sal_Bool SfxMedium::Commit()
{
    if( aStorage.Is() )
    {
        // StorageStream immer direkt
        if( !aStorage->Commit() )
            eError = aStorage->GetError();
    }
    else if( pOutStream  )
        pOutStream->Flush();
    else if( pInStream  )
        pInStream->Flush();

    if ( ( GetError() == SVSTREAM_OK ) && pImp->pTempFile )
        Transfer_Impl();

    return GetError() == SVSTREAM_OK;
}

//------------------------------------------------------------------
sal_Bool SfxMedium::IsStorage()
{
    if ( aStorage.Is() )
        return TRUE;

    if ( bTriedStorage )
        return pImp->bIsStorage;

    if ( pImp->pTempFile )
    {
        pImp->bIsStorage = SotStorage::IsStorageFile( pImp->pTempFile->GetFileName() );
        if ( !pImp->bIsStorage )
            bTriedStorage = TRUE;
    }
    else
    {
        pImp->bIsStorage = SotStorage::IsStorageFile( GetInStream() );
        if ( pInStream && !pInStream->GetError() && !pImp->bIsStorage )
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

    if ( aStorage.Is() )
        return sal_True;

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
    aTmp += pFilter->GetFilterContainer()->GetName();
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

    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
    com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > xInteractionHandler(
                xFactory->createInstance( DEFINE_CONST_UNICODE("com.sun.star.uui.InteractionHandler") ), UNO_QUERY );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs(1);
    aArgs.getArray()[0].Name = DEFINE_CONST_UNICODE( "InteractionHandler" );
    aArgs.getArray()[0].Value <<= xInteractionHandler ;

    ::com::sun::star::uno::Sequence< ::rtl::OUString > files(0);

    if( !xPacker->unpack( pImp->pTempDir->GetURL(), aPath, files, aArgs ) )
        return sal_False;

    String aNewName = pImp->pTempDir->GetURL();
    aNewName += '/';
    aNewName += String( aTempDoku );
    CloseInStream_Impl();
    SetPhysicalName( aNewName );
    GetStorage();

    if ( aStorage.Is() )
    {
        const SfxFilter *pRealFilter = SFX_APP()->GetFilterMatcher().GetFilter4ClipBoardId( aStorage->GetFormat() );
        if ( pRealFilter )
        {
            pImp->nFileVersion = pRealFilter->GetVersion();
            aStorage->SetVersion( pImp->nFileVersion );
        }

        DBG_ASSERT( pRealFilter, "Unknown storage format!" );
    }

    return aStorage.Is();
}

ErrCode SfxMedium::Unpack_Impl( const String& rDest )
{
    ErrCode nRet = ERRCODE_NONE;
    if ( pImp->pTempDir )
    {
/*
        DirEntry aDestEntry( rDest );
        Dir aDir( pImp->pTempDir->GetName(), FSYS_KIND_FILE );
        sal_uInt16 nCount = aDir.Count();
        for ( sal_uInt16 n=0; n<nCount; n++ )
        {
            DirEntry aDest( aDestEntry.GetPath() );
            DirEntry aTmp = aDir[n];
            aDest += aTmp.GetName();
            if ( aDir[n] == DirEntry( GetPhysicalName() ) )
                continue;

            nRet = aTmp.CopyTo( aDest, FSYS_ACTION_COPYFILE );
            if ( nRet != ERRCODE_NONE )
                break;
        }
 */
    }

    return nRet;
}

//------------------------------------------------------------------
#if SUPD<615
SvStorage* SfxMedium::GetOutputStorage()
{
    return GetOutputStorage( FALSE );
}
#endif

SvStorage* SfxMedium::GetOutputStorage( BOOL bUCBStorage )
{
    // if the medium was constructed with a SvStorage: use this one, not a temp. storage
    if ( aStorage.Is() && !aLogicName.Len() )
        return aStorage;

    if ( !pImp->pTempFile )
        CreateTempFile();
    return GetStorage_Impl( bUCBStorage );
}

SvStorage* SfxMedium::GetStorage()
{
    return GetStorage_Impl( FALSE );
}

SvStorage* SfxMedium::GetStorage_Impl( BOOL bUCBStorage )
{
    if ( aStorage.Is() || bTriedStorage )
        return aStorage;

    BOOL bResetSorage = FALSE;
    SvStream *pStream;

    String aStorageName;
    if ( pImp->pTempFile )
    {
        aStorageName = pImp->pTempFile->GetURL();
        pStream = GetOutStream();
    }
    else
    {
        aStorageName = GetURLObject().GetMainURL();
        pStream = GetInStream();
        if ( pStream )
        {
            pStream->GetLockBytes()->SetSynchronMode( sal_True );
            if ( !pImp->aDoneLink.IsSet() )
                DownLoad();
        }
    }

    if( !pStream || ( GetError() != SVSTREAM_OK ) )
        return aStorage;

    bTriedStorage = sal_True;

    if ( pImp->pTempFile && pOutStream )
    {
        DELETEZ( pOutStream );
        aStorage = new SvStorage( bUCBStorage, aStorageName, nStorOpenMode, bDirect ? 0 : STORAGE_TRANSACTED );
    }
    else
    {
        aStorage = new SvStorage( pStream, FALSE );
        aStorage->SetName( aStorageName );
    }

    if ( aStorage->GetError() == SVSTREAM_OK )
        GetVersionList();

    // ???? wird das noch gebraucht?
//  GetMedium_Impl();
//  if ( !aStorage.Is() )
//      CreateFileStream();

    SFX_ITEMSET_ARG( pSet, pVersion, SfxInt16Item, SID_VERSION, sal_False);

    if ( pVersion )
    {
        // Alle verf"ugbaren Versionen einlesen
        if ( pImp->pVersions )
        {
            // Die zum Kommentar passende Version suchen
            // Die Versionen sind von 1 an durchnumeriert, mit negativen
            // Versionsnummern werden die Versionen von der aktuellen aus
            // r"uckw"arts gez"ahlt
            short nVersion = pVersion ? pVersion->GetValue() : 0;
            if ( nVersion<0 )
                nVersion = ( (short) pImp->pVersions->Count() ) + nVersion;
            else if ( nVersion )
                nVersion--;

            SfxVersionInfo* pInfo = nVersion>=0 ? pImp->pVersions->GetObject( nVersion ) : NULL;
            if ( pInfo )
            {
                String aVersionStream = pInfo->aName;

                // SubStorage f"ur alle Versionen "offnen
                SvStorageRef aSub =
                    aStorage->OpenStorage( DEFINE_CONST_UNICODE( "Versions" ), SFX_STREAM_READONLY | STREAM_NOCREATE );

                DBG_ASSERT( aSub.Is() && !aSub->GetError(), "Versionsliste, aber keine Versionen!" );

                // try to open the version stream as sub storage, first ( the new 6.0 format )
                // if it's not a storage then it's hopefully a compressed stream ( the old 5.x format )

                nStorOpenMode = SFX_STREAM_READONLY | STREAM_NOCREATE;
                aStorage = aSub->OpenStorage( aVersionStream, nStorOpenMode );

                if( aStorage.Is() && ! aStorage->GetError() )
                    GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, sal_True ) );
                else
                {
                    // Dort ist die Version als gepackter Stream gespeichert
                    SvStorageStreamRef aStream =
                        aSub->OpenStream( aVersionStream, SFX_STREAM_READONLY );

                    if ( aStream.Is() && aStream->GetError() == SVSTREAM_OK )
                    {
                        // Stream ins TempDir auspacken
                        ::utl::TempFile        aTempFile;
                        String          aTmpName = aTempFile.GetURL();
                        SvFileStream    aTmpStream( aTmpName, SFX_STREAM_READWRITE );

                        ZCodec aCodec;
                        aCodec.BeginCompression();
                        aCodec.Decompress( *aStream, aTmpStream );
                        aCodec.EndCompression();
                        aTmpStream.Close();

                        // Datei als Storage "offnen
                        nStorOpenMode = SFX_STREAM_READONLY;
                        aStorage = new SvStorage( aTmpName, nStorOpenMode );
                        SetPhysicalName( aTmpName );
                        pImp->bIsTemp = sal_True;
                        GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, sal_True ) );
                        DELETEZ( pImp->pVersions );
                    }
                    else
                        bResetSorage = TRUE;
                }
            }
            else
                bResetSorage = TRUE;
        }
        else
            bResetSorage = TRUE;
    }

    if ( aStorage.Is() )
    {
        if( aStorage->GetError() != SVSTREAM_OK )
            bResetSorage = TRUE;
        else if ( GetFilter() )
            aStorage->SetVersion( GetFilter()->GetVersion() );
    }

    if ( bResetSorage )
    {
        aStorage.Clear();
        pStream->Seek( 0L );
    }

    pImp->bIsStorage = aStorage.Is();
    return aStorage;
}

//------------------------------------------------------------------
void SfxMedium::CloseStorage()
{
    aStorage.Clear();
    bTriedStorage = sal_False;
    pImp->bIsStorage = sal_False;
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
void SfxMedium::Transfer_Impl()
{
    if( pImp->pTempFile && ( !eError || eError & ERRCODE_WARNING_MASK ) )
    {
        Reference < XContent > xContent = GetContent();
        if ( ! xContent.is() )
        {
            eError = ERRCODE_IO_NOTEXISTS;
            return;
        }

        sal_Bool bSuccess = sal_False;

        // check wether the the temp file has the same protocol
        // scheme as the destination, wether the destination supports
        // the transfer command and wether or not the command
        // executed successfully

        BOOL            bTryTransfer = FALSE;
        String          aName;
        INetURLObject   aDest = GetURLObject();
        INetURLObject   aSource( pImp->pTempFile->GetURL() );

        aName = GetLongName();
        if ( !aName.Len() )
            aName = aDest.getName( INetURLObject::LAST_SEGMENT, true,
                                   INetURLObject::DECODE_WITH_CHARSET );

        if ( aDest.GetProtocol() == aSource.GetProtocol() )
        {
            bTryTransfer = UCB_Helper::HasCommand( xContent,
                                                   WID_TRANSFER );
        }

        // free resources, otherwise the transfer may fail
        Close();
        if ( bTryTransfer && aDest.removeSegment() )
        {
            // content says: direct transfer is possible
            TransferInfo    aInfo;
            String          aParentURL = aDest.GetMainURL();
            Any             aAny;

            aInfo.MoveData = sal_True;
            aInfo.SourceURL = aSource.GetMainURL();
            aInfo.NewTitle = aName;

            SFX_ITEMSET_ARG( GetItemSet(), pRename, SfxBoolItem, SID_RENAME, sal_False );
            SFX_ITEMSET_ARG( GetItemSet(), pOverWrite, SfxBoolItem, SID_OVERWRITE, sal_False );

            if ( pOverWrite && !pOverWrite->GetValue() )
            {
                aInfo.NameClash = NameClash::ERROR;
            }
            else if ( pRename && pRename->GetValue() )
            {
                aInfo.NameClash = NameClash::RENAME;
            }
            else
            {
                aInfo.NameClash = NameClash::OVERWRITE;
            }

            aAny <<= aInfo;
            UCB_Helper::ExecuteCommand( aParentURL, WID_TRANSFER, aAny, &bSuccess );
            if ( bSuccess )
            {
                // when the transfer command executed successful, there will be no tempfile
                // anymore ( because the file has been moved! ), so we have to get rid of it
                pImp->pTempFile->EnableKillingFile( sal_False );
                delete pImp->pTempFile;
                pImp->pTempFile = NULL;
                return;
            }
        }

        if ( !bSuccess )
        {
            // direct transfer impossible or failed: transfer data as stream
            SvStream *pStream = new SvFileStream( pImp->pTempFile->GetFileName(), STREAM_STD_READ );
            SvLockBytesRef xLockBytes = new SvLockBytes( pStream );
            Reference < ::com::sun::star::io::XInputStream > xStream = new ::utl::OInputStreamHelper( xLockBytes, 8192 );

            Any aAny;
            InsertCommandArgument aArg;
            aArg.Data = xStream;
            aArg.ReplaceExisting = sal_True;
            aAny <<= aArg;

            UCB_Helper::ExecuteCommand( xContent, WID_INSERT, aAny, &bSuccess );
            delete pStream;
        }

        if ( !bSuccess )
            eError = ERRCODE_IO_GENERAL;
    }
}

//------------------------------------------------------------------
void SfxMedium::DoBackup_Impl()
{
    INetURLObject   aDest;
    INetURLObject   aSource = GetURLObject();
    String          aParentURL;
    String          aName;
    String          aBakDir = SvtPathOptions().GetBackupPath();
    BOOL            bTryTransfer = FALSE;
    sal_Bool        bSuccess = sal_False;
    Reference < XContent > xContent;

    // Backup Path gesetzt ? Dann diesen benutzen.
    if( aBakDir.Len() )
    {
        aDest.SetURL( aBakDir );
        aDest.insertName( aSource.getName() );
    }
    else
    {
        aDest = GetURLObject();
    }

    aDest.setExtension( DEFINE_CONST_UNICODE( "bak" ) );
    aName = aDest.getName( INetURLObject::LAST_SEGMENT, true,
                           INetURLObject::DECODE_WITH_CHARSET );

    // check wether the the temp file has the same protocol
    // scheme as the destination, wether the destination supports
    // the transfer command and wether or not the command
    // executed successfully

    if ( aDest.GetProtocol() == aSource.GetProtocol() )
    {
        INetURLObject aDestDir = aDest;
        if ( aDestDir.removeSegment() )
        {
            aParentURL = aDestDir.GetMainURL();
            xContent = UCB_Helper::CreateContent( aParentURL );
            if ( xContent.is() )
            {
                bTryTransfer = UCB_Helper::HasCommand( xContent,
                                               WID_TRANSFER );
            }
        }
    }

    if ( bTryTransfer )
    {
        TransferInfo    aInfo;
        Any             aAny;

        aInfo.MoveData = sal_True;
        aInfo.SourceURL = aSource.GetMainURL();
        aInfo.NewTitle = aName;
        aInfo.NameClash = NameClash::OVERWRITE;

        aAny <<= aInfo;
        Close();
        UCB_Helper::ExecuteCommand( xContent, WID_TRANSFER,
                                    aAny, &bSuccess );
    }

    if ( !bSuccess )
    {
        xContent = UCB_Helper::CreateContent( aDest.GetMainURL() );
        if ( xContent.is() )
        {
            SvStream *pStream = GetInStream();
            SvLockBytesRef xLockBytes = new SvLockBytes( pStream );
            Reference < ::com::sun::star::io::XInputStream > xStream
                    = new ::utl::OInputStreamHelper( xLockBytes, 8192 );

            Any aAny;
            InsertCommandArgument aArg;

            aArg.Data = xStream;
            aArg.ReplaceExisting = sal_True;
            aAny <<= aArg;

            UCB_Helper::ExecuteCommand( xContent, WID_INSERT, aAny, &bSuccess );
        }
    }

    if ( ! bSuccess )
        WarningBox( NULL, SfxResId( MSG_WARNING_BACKUP ) ).Execute();
}

//----------------------------------------------------------------
void SfxMedium::GetMedium_Impl()
{
    if ( !pInStream )
    {
        pImp->bDownloadDone = sal_False;
        pImp->bStreamReady = sal_False;

        ::utl::UcbLockBytesRef xLockBytes;
        ::utl::UcbLockBytesHandler* pHandler = pImp->aHandler;
        INetProtocol eProt = GetURLObject().GetProtocol();
        if ( eProt != INET_PROT_HTTP && eProt != INET_PROT_FTP )
            pHandler = NULL;
        BOOL bSynchron = pImp->bForceSynchron || ! pImp->aDoneLink.IsSet();
        SFX_ITEMSET_ARG( pSet, pStreamItem, SfxUsrAnyItem, SID_INPUTSTREAM, sal_False);
        if ( pStreamItem )
        {
            Reference < ::com::sun::star::io::XInputStream > xStream;
            if ( ( pStreamItem->GetValue() >>= xStream ) && xStream.is() )
                xLockBytes = utl::UcbLockBytes::CreateInputLockBytes( xStream );
            Done_Impl( xLockBytes.Is() ? xLockBytes->GetError() : ERRCODE_IO_NOTSUPPORTED );
        }
        else
        {
            SFX_ITEMSET_ARG( GetItemSet(), pItem, SfxBoolItem, SID_DOC_READONLY, sal_False);
            BOOL bAllowReadOnlyMode = pItem ? pItem->GetValue() : TRUE;
            BOOL bIsWritable = ( nStorOpenMode & STREAM_WRITE );

            // no callbacks for opening read/write because we might try readonly later
            pImp->bDontCallDoneLinkOnSharingError = ( bIsWritable && bAllowReadOnlyMode );
            xLockBytes = ::utl::UcbLockBytes:: CreateLockBytes( GetContent(), nStorOpenMode, bIsWritable ? NULL : pHandler );
            if ( !xLockBytes.Is() )
            {
                pImp->bDontCallDoneLinkOnSharingError = sal_False;
                Done_Impl( ERRCODE_IO_NOTEXISTS );
            }
            else if ( xLockBytes->GetError() == ERRCODE_IO_NOTEXISTS && bIsWritable && bAllowReadOnlyMode )
            {
                GetItemSet()->Put( SfxBoolItem(SID_DOC_READONLY, sal_True));
                SetOpenMode(SFX_STREAM_READONLY, sal_False);
                ResetError();
                pImp->bDownloadDone = sal_False;
                pImp->bDontCallDoneLinkOnSharingError = sal_False;
                xLockBytes = ::utl::UcbLockBytes::CreateLockBytes( GetContent(), nStorOpenMode, pHandler );
                if ( !pHandler && !pImp->bDownloadDone )
                    Done_Impl( xLockBytes->GetError() );
            }
            else if ( !pHandler && !pImp->bDownloadDone )
                // opening readwrite is always done synchronously
                Done_Impl( xLockBytes->GetError() );
        }

        if ( xLockBytes.Is() )
        {
            if ( bSynchron )
                xLockBytes->SetSynchronMode( sal_True );
            if ( !pImp->bDownloadDone )
                pImp->pCancellable = new UcbLockBytesCancellable_Impl( xLockBytes, pImp->GetCancelManager(), aLogicName );
            pInStream = new SvStream( xLockBytes );
            pInStream->SetBufferSize( 4096 );
            pImp->bStreamReady = sal_True;
        }
    }

    // Download completion happened while pInStream was constructed
    if ( pImp->bDownloadDone )
        Done_Impl( GetError() );

#ifdef OLD_CODE_FOR_POSTING

    SFX_ITEMSET_ARG( pSet, pContentType, SfxStringItem, SID_CONTENTTYPE, sal_False);
    if( pContentType ) pImp->xBinding->SetSendMimeType(
        pContentType->GetValue() );
    pImp->xBinding->SetReferer( pImp->aReferer );
    pImp->xBinding->SetPriority( pImp->nPrio );
    const SfxPoolItem *pItem = 0;
    if( pSet )
    {
        pSet->GetItemState( SID_POSTSTRING, sal_False, &pItem );
        if( !pItem )
            pSet->GetItemState( SID_POSTLOCKBYTES, sal_False, &pItem );
    }

    if( pItem )
    {
        if( pItem->ISA( SfxStringItem ))
        {
            SvCacheStream* pCacheStr = new SvCacheStream( 8192 );
            pCacheStr->WriteByteString( ((SfxStringItem*)pItem)->GetValue(), RTL_TEXTENCODING_UTF8 );
            SvLockBytesRef xRef (new SvLockBytes(pCacheStr, sal_True));
            pImp->xBinding->GetBindContext().SetPostLockBytes ( xRef);
        }
        else if( pItem->ISA( SfxRefItem ))
        {
            SvLockBytes* pBytes = new SvLockBytes;
            int nDiff = (char*)pBytes - (char*)(SvRefBase*)pBytes;
            SvLockBytes* pLB = (SvLockBytes*)(
                    (char*)(SvRefBase*)&((SfxRefItem*)pItem )->GetValue() + nDiff );
            delete pBytes;
            SvLockBytesRef rTmpLB ( pLB );
            pImp->xBinding->GetBindContext().SetPostLockBytes ( rTmpLB );
        }
        else if( pItem->ISA( SfxLockBytesItem ))
        {
            // f"ur sp"ater ...
            SvLockBytes* pBytes = ((SfxLockBytesItem*)pItem)->GetValue();
            SvLockBytesRef rTmpLB ( pBytes );
            pImp->xBinding->GetBindContext().SetPostLockBytes ( rTmpLB );
        }
    }
#endif
}

//------------------------------------------------------------------
SfxPoolCancelManager* SfxMedium::GetCancelManager_Impl() const
{
    return pImp->GetCancelManager();
}

//------------------------------------------------------------------
void SfxMedium::SetCancelManager_Impl( SfxPoolCancelManager* pMgr )
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
    GetMedium_Impl();
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
    pImp->pVersions = NULL;

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
            // try to convert the URL into a physical name
            ::utl::LocalFileHelper::ConvertURLToPhysicalName( aLogicName, aName );
        }
    }

    SetIsRemote_Impl();

    // Recover-Files sind immer temp
    SFX_ITEMSET_ARG( pSet, pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False);
    if ( pSalvageItem )
        pImp->bIsTemp = sal_True;
}

//------------------------------------------------------------------

SfxMedium::SfxMedium()
:   IMPL_CTOR(),
    bRoot( sal_False ),
    pURLObj(0),

    pSet(0),
    pImp(new SfxMedium_Impl( this )),
    pFilter(0)
{
    Init_Impl();
}
//------------------------------------------------------------------

SfxMedium::SfxMedium( const SfxMedium& rMedium, sal_Bool bTemporary )
:   IMPL_CTOR(),
    bRoot(sal_True),
    pURLObj( rMedium.pURLObj ? new INetURLObject(*rMedium.pURLObj) : 0 ),
    pImp(new SfxMedium_Impl( this ))
{
    bDirect       = rMedium.IsDirect();
    nStorOpenMode = rMedium.GetOpenMode();
    if ( bTemporary )
        CreateTempFile();
    else
        aName = rMedium.GetName();

    pImp->bIsTemp = bTemporary;
    DBG_ASSERT( ! rMedium.pImp->bIsTemp,
                "Temporaeres Medium darf nicht kopiert werden" );
    aLogicName = rMedium.aLogicName;
    pSet =  rMedium.GetItemSet() ? new SfxItemSet(*rMedium.GetItemSet()) : 0;
    pFilter = rMedium.pFilter;
    Init_Impl();
    if( bTemporary )
    {
        if ( !SfxContentHelper::CopyTo( rMedium.GetName(), GetPhysicalName() ) )
            SetError( ERRCODE_IO_GENERAL );
    }

    if ( rMedium.pImp->pEaMgr )
        GetEaMgr();
}
//------------------------------------------------------------------

void SfxMedium::SetFilter( const SfxObjectFactory& rFact, const String & rFilter )
{
    SetFilter(  rFact.GetFilterContainer()->GetFilter(rFilter) );
}
//----------------------------------------------------------------

void SfxMedium::SetFilter( const SfxFilter* pFilterP, sal_Bool bResetOrig )
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

void SfxMedium::SetOrigFilter_Impl( const SfxFilter* pFilter )
{
    pImp->pOrigFilter = pFilter;
}
//------------------------------------------------------------------

void SfxMedium::Close()
{
    if ( aStorage.Is() )
    {
        // don't close the streams if they belong to the
        // storage

        const SvStream *pStream = aStorage->GetSvStream();
        if ( pStream == pInStream )
        {
            pInStream = NULL;
            aStorage->SetDeleteStream( TRUE );
        }
        if ( pStream == pOutStream )
        {
            pOutStream = NULL;
            aStorage->SetDeleteStream( TRUE );
        }

        CloseStorage();
    }

    if ( pInStream )
        CloseInStream_Impl();

    if ( pOutStream )
        CloseOutStream_Impl();
}

//------------------------------------------------------------------

void SfxMedium::RefreshName_Impl()
{
#if 0   //(dv)
    if ( pImp->xContent.is() )
    {
        String aNameP = pImp->xAnchor->GetViewURL();
        pImp->aOrigURL = aNameP;
        aLogicName = aNameP;
        DELETEZ( pURLObj );
        if (aLogicName.Len())
            aLogicName = GetURLObject().GetMainURL();
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
    pImp->xContent = ::com::sun::star::uno::Reference < ::com::sun::star::ucb::XContent > ();
    Init_Impl();
}

//----------------------------------------------------------------
const String& SfxMedium::GetOrigURL() const
{
    return !pImp->aOrigURL.Len() ? (String &)aLogicName : pImp->aOrigURL;
}

//----------------------------------------------------------------

void SfxMedium::SetPhysicalName( const String& rNameP )
{
    if( pImp->pTempFile )
    {
        delete pImp->pTempFile;
        pImp->pTempFile = NULL;
    }

    aName = rNameP;
    bTriedStorage = sal_False;
    pImp->bIsStorage = sal_False;
    if ( aName.Len() )
        pImp->xContent = Reference < XContent >();
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

sal_Bool SfxMedium::Exists( sal_Bool bForceSession )
{
    DBG_ERROR( "Not implemented!" );
    return sal_True;
}

//------------------------------------------------------------------

void SfxMedium::ReOpen()
{
    DBG_ASSERT( pFilter, "Kein Filter, aber ReOpen!" );
    if( pFilter )
    {
        if( pFilter->UsesStorage() )
            GetStorage();
        else
            GetInStream();
    }
}
//------------------------------------------------------------------
SfxMedium::SfxMedium
(
    const String &rName, StreamMode nOpenMode,  sal_Bool bDirectP,
#if SUPD<609
    sal_Bool bDownLoadP,
#endif
    const SfxFilter *pFlt, SfxItemSet *pInSet
)
:   IMPL_CTOR(),
    bRoot( sal_False ),
    pFilter(pFlt),
    pURLObj(0),
    pImp(new SfxMedium_Impl( this )),
    pSet( pInSet )
{
    aName = aLogicName = rName;
    nStorOpenMode = nOpenMode;
    bDirect = bDirectP;
    Init_Impl();
}
//------------------------------------------------------------------

SfxMedium::SfxMedium( SvStorage *pStorage, sal_Bool bRootP )
:   IMPL_CTOR(),
    bRoot( bRootP ),
    aStorage(pStorage),
    pURLObj(0),
    pImp( new SfxMedium_Impl( this )),
    pSet(0)
{
    SfxApplication* pApp = SFX_APP();
    sal_uInt32 nFormat = pStorage->GetFormat();
    if( !nFormat )
    {
#ifdef DBG_UTIL
        if( aLogicName.Len() )
            DBG_ERROR( "Unbekanntes StorageFormat, versuche eigenes Format" );
#endif
        pFilter = SfxObjectFactory::GetDefaultFactory().GetFilterContainer()->
            GetFilter( 0 );
    }
    else
        pFilter = pApp->GetFilterMatcher().GetFilter4ClipBoardId( nFormat, 0, 0 );
    if( !pFilter && nFormat )
    {
        DBG_ERROR( "No Filter for storage found!" );
        pFilter = SfxObjectFactory::GetDefaultFactory().GetFilterContainer()->GetFilter( 0 );
    }

    Init_Impl();
}

//------------------------------------------------------------------

SfxMedium::~SfxMedium()
{
//  CancelTransfers();
    ::vos::OClearableGuard aGuard( pImp->aHandler->GetMutex() );
    pImp->aHandler->ReleaseMedium();
    aGuard.clear();

    Close();

    delete pSet;

    if( pImp->bIsTemp && GetPhysicalName().Len() )
    {
        String aTemp;
        ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aTemp );
        SfxContentHelper::Kill( aTemp );
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
    }

    return *pURLObj;
}

//----------------------------------------------------------------

const String& SfxMedium::GetPreRedirectedURL() const
{
    return pImp->aPreRedirectionURL;
}
//----------------------------------------------------------------

sal_uInt32 SfxMedium::GetMIMEAndRedirect( String &rName )
{
/* dv !!!! not needed any longer ?
    INetProtocol eProt = GetURLObject().GetProtocol();
    if( eProt == INET_PROT_FTP && SvBinding::ShouldUseFtpProxy( GetURLObject().GetMainURL() ) )
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

void SfxMedium::SetUsesCache( sal_Bool bUse )
{
    pImp->bUsesCache = bUse;
}
//----------------------------------------------------------------

sal_Bool SfxMedium::UsesCache() const
{
    return pImp->bUsesCache;
}
//----------------------------------------------------------------

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

void SfxMedium::SetTransferPriority( sal_uInt16 nPrio )
{
    pImp->nPrio = nPrio;
}
//----------------------------------------------------------------

sal_uInt16 SfxMedium::GetTransferPriority( ) const
{
    return pImp->nPrio;
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
/* Kann der URL ein MIME Type zugeordnent werden? */
sal_Bool SfxMedium::SupportsMIME_Impl() const
{
    INetProtocol eProt = GetURLObject().GetProtocol();
    if( eProt == INET_PROT_HTTPS || eProt == INET_PROT_HTTP  )
        return sal_True;

    if( eProt == INET_PROT_NOT_VALID )
        return sal_False;

    if( eProt == INET_PROT_FTP )
    {
        Any aAny( UCB_Helper::GetProperty( GetContent(), WID_FLAG_IS_FOLDER ) );
        sal_Bool bIsFolder = FALSE;
        if ( ( aAny >>= bIsFolder ) && bIsFolder )
            return SvBinding::ShouldUseFtpProxy( GetURLObject().GetMainURL() );
        else
            return sal_False;
    }

    return sal_False;
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

SvStream* SfxMedium::RemoveStream_Impl()
{
    SvStream* pRet = GetInStream();
    pInStream = 0;
    return pRet;
}
//----------------------------------------------------------------

void SfxMedium::SetStream_Impl( SvStream* pStrm )
{
    CloseInStream_Impl();
    pInStream = pStrm;
}
//----------------------------------------------------------------

void SfxMedium::SetStorage_Impl( SvStorage* pStor )
{
    aStorage = pStor;
}
//----------------------------------------------------------------

SfxItemSet* SfxMedium::GetItemSet() const
{
    if( !pSet ) ((SfxMedium*)this)->pSet =
                    new SfxAllItemSet( SFX_APP()->GetPool() );
    return pSet;
}
//----------------------------------------------------------------

void SfxMedium::SetLoadEnvironment_Impl( LoadEnvironment_Impl* pEnv )
{
    pImp->pLoadEnv = pEnv;
}
//----------------------------------------------------------------

LoadEnvironment_Impl* SfxMedium::GetLoadEnvironment_Impl() const
{
    return pImp->pLoadEnv;
}
//----------------------------------------------------------------

SvKeyValueIterator* SfxMedium::GetHeaderAttributes_Impl()
{
    if( !pImp->xAttributes.Is() )
/*!!!!      if ( pImp->xBinding.Is() )
            pImp->xAttributes = pImp->xBinding->GetHeaders();
        else
*/          pImp->xAttributes = SvKeyValueIteratorRef( new SvKeyValueIterator );

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

SvEaMgr* SfxMedium::GetEaMgr()
{
    if ( !pImp->pEaMgr && pFilter )
    {
        /* the stream in the storage is probably not a filestream ( the stream is
            closed anyway! ). Therefor we will always use GetPhysicalName to
            create the SvEaMgr. */
        //  SvStream *pStream = aStorage.Is() ? aStorage->GetTargetSvStream() : NULL;
        //  if ( pStream && pStream->IsA() == ID_FILESTREAM )
        //      pImp->pEaMgr = new SvEaMgr(*(SvFileStream *)pStream);
        //  else
        pImp->pEaMgr = new SvEaMgr( GetPhysicalName() );
    }

    return pImp->pEaMgr;
}

//----------------------------------------------------------------

void SfxMedium::SetDontCreateCancellable( )
{
    pImp->bDontCreateCancellable = sal_True;
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSource >  SfxMedium::GetDataSource()
{
    if ( !pImp->pSource )
        CreateDataSource();
    return pImp->pSource;
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >  SfxMedium::GetDataSink()
{
    if ( !pImp->pSink )
        CreateDataSink();
    return pImp->pSink;
}

void SfxMedium::CreateDataSource()
{
    SfxLoadEnvironment *pEnv = NULL;
    if ( pImp->pLoadEnv )
    {
        pEnv = new SfxLoadEnvironment( pImp->pLoadEnv );
        SfxRefItem aItem( SID_LOADENVIRONMENT, pEnv );
        GetItemSet()->Put( aItem );
    }

    pImp->pSource = new FileSource_Impl( this );
    pImp->pSource->acquire();
    if ( pEnv )
        pEnv->SetDataAvailableLink( LINK( pImp->pSource, FileSource_Impl, DataAvailableHdl ) );
}

void SfxMedium::CreateDataSink()
{
    pImp->pSink = new FileSink_Impl( this );
    pImp->pSink->acquire();
}

void SfxMedium::ResetDataSource()
{
    if ( pImp->pSource )
    {
        pImp->pSource->ResetMedium();
        pImp->pSource->release();
    }
}

void SfxMedium::ResetDataSink()
{
    if ( pImp->pSink )
    {
        pImp->pSink->ResetMedium();
        pImp->pSink->release();
    }
}

const SfxVersionTableDtor* SfxMedium::GetVersionList()
{
    if ( !pImp->pVersions && GetStorage() )
    {
        SvStorageStreamRef aStream =
            GetStorage()->OpenStream( DEFINE_CONST_UNICODE( "VersionList" ), SFX_STREAM_READONLY | STREAM_NOCREATE );
        if ( aStream.Is() && aStream->GetError() == SVSTREAM_OK )
        {
            pImp->pVersions = new SfxVersionTableDtor;
            pImp->pVersions->Read( *aStream );
        }
    }

    return pImp->pVersions;
}

sal_uInt16 SfxMedium::AddVersion_Impl( SfxVersionInfo& rInfo )
{
    if ( GetStorage() )
    {
        if ( !pImp->pVersions )
            pImp->pVersions = new SfxVersionTableDtor;

        // Einen eindeutigen Namen f"ur den Stream ermitteln
        SvULongs aLongs;
        SfxVersionInfo* pInfo = pImp->pVersions->First();
        while ( pInfo )
        {
            sal_uInt32 nVer = (sal_uInt32) pInfo->aName.Copy(7).ToInt32();
            sal_uInt16 n;
            for ( n=0; n<aLongs.Count(); n++ )
                if ( nVer<aLongs[n] )
                    break;

            aLongs.Insert( nVer, n );
            pInfo = pImp->pVersions->Next();
        }

        sal_uInt16 nKey;
        for ( nKey=0; nKey<aLongs.Count(); nKey++ )
            if ( aLongs[nKey] > ( ULONG ) nKey+1 )
                break;

        rInfo.aName = DEFINE_CONST_UNICODE( "Version" );
        rInfo.aName += String::CreateFromInt32( nKey + 1 );
        pInfo = new SfxVersionInfo( rInfo );
        pImp->pVersions->Insert( pInfo, LIST_APPEND );
        return nKey;
    }

    return 0;
}

sal_Bool SfxMedium::RemoveVersion_Impl( const SfxVersionInfo& rInfo )
{
    if ( !pImp->pVersions )
        return sal_False;

    SfxVersionInfo* pInfo = pImp->pVersions->First();
    while( pInfo )
    {
        if ( pInfo->aName == rInfo.aName )
        {
            pImp->pVersions->Remove( pInfo );
            delete pInfo;
            return sal_True;
        }

        pInfo = pImp->pVersions->Next();
    }

    return sal_False;
}

sal_Bool SfxMedium::TransferVersionList_Impl( SfxMedium& rMedium )
{
    if ( rMedium.pImp->pVersions )
    {
        delete pImp->pVersions;
        pImp->pVersions = new SfxVersionTableDtor( *rMedium.pImp->pVersions );
        return sal_True;
    }

    return sal_False;
}

sal_Bool SfxMedium::SaveVersionList_Impl()
{
    if ( GetStorage() )
    {
        if ( !pImp->pVersions )
            return sal_True;

        SvStorageStreamRef aStream =
            GetStorage()->OpenStream( DEFINE_CONST_UNICODE( "VersionList" ), SFX_STREAM_READWRITE );
        if ( aStream.Is() && aStream->GetError() == SVSTREAM_OK )
        {
            pImp->pVersions->Write( *aStream );
            return sal_True;
        }
    }

    return sal_False;
}

//----------------------------------------------------------------
sal_Bool SfxMedium::IsReadOnly()
{
    sal_Bool bReadOnly = !( GetOpenMode() & STREAM_WRITE );
/*(dv)  if ( bReadOnly && pURLObj && CntAnchor::IsViewURL( pURLObj->GetMainURL() ) )
        // Chaos-Storages sind niemals als readonly anzusehen!
        return sal_False;
*/
    if ( !bReadOnly )
    {
        // logisch readonly ge"offnet
        SFX_ITEMSET_ARG( GetItemSet(), pItem, SfxBoolItem, SID_DOC_READONLY, sal_False);
        if ( pItem )
            bReadOnly = pItem->GetValue();
    }

    return bReadOnly;
}

//----------------------------------------------------------------
void SfxMedium::CreateTempFile()
{
    if ( pImp->pTempFile )
        delete pImp->pTempFile;

    String          aParentName;
    INetURLObject   aParent = GetURLObject();

    if ( aParent.removeSegment() )
        aParentName = aParent.GetMainURL();

    pImp->pTempFile = new ::utl::TempFile( &aParentName );
    pImp->pTempFile->EnableKillingFile( sal_True );

    aName = pImp->pTempFile->GetFileName();

    CloseOutStream_Impl();
    CloseStorage();
}

//----------------------------------------------------------------
#define nActVersion 1

SvStream& SfxVersionTableDtor::Read( SvStream& rStrm )
{
    sal_uInt16 nCount = 0, nVersion = 0;

    rStrm >> nVersion;
    rStrm >> nCount;

    for( sal_uInt16 i=0; i<nCount; ++i )
    {
        SfxVersionInfo *pNew = new SfxVersionInfo;
        rStrm.ReadByteString( pNew->aComment, RTL_TEXTENCODING_UTF8 );
        rStrm.ReadByteString( pNew->aName, RTL_TEXTENCODING_UTF8 );
        pNew->aCreateStamp.Load( rStrm );
        Insert( pNew, LIST_APPEND );
    }

    return rStrm;
}

SvStream& SfxVersionTableDtor::Write( SvStream& rStream ) const
{
    rStream << (sal_uInt16) nActVersion;
    rStream << (sal_uInt16) Count();

    SfxVersionInfo* pInfo = ((SfxVersionTableDtor*)this)->First();
    while( pInfo && rStream.GetError() == SVSTREAM_OK )
    {
        rStream.WriteByteString( pInfo->aComment, RTL_TEXTENCODING_UTF8 );
        rStream.WriteByteString( pInfo->aName, RTL_TEXTENCODING_UTF8 );
        pInfo->aCreateStamp.Save( rStream );
        pInfo = ((SfxVersionTableDtor*)this)->Next();
    }

    return rStream;
}

void SfxVersionTableDtor::DelDtor()
{
    SfxVersionInfo* pTmp = First();
    while( pTmp )
    {
        delete pTmp;
        pTmp = Next();
    }
    Clear();
}

SfxVersionTableDtor& SfxVersionTableDtor::operator=( const SfxVersionTableDtor& rTbl )
{
    DelDtor();
    SfxVersionInfo* pTmp = ((SfxVersionTableDtor&)rTbl).First();
    while( pTmp )
    {
        SfxVersionInfo *pNew = new SfxVersionInfo( *pTmp );
        Insert( pNew, LIST_APPEND );
        pTmp = ((SfxVersionTableDtor&)rTbl).Next();
    }
    return *this;
}

//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
SfxVersionInfo::SfxVersionInfo()
{
}

SvStringsDtor* SfxVersionTableDtor::GetVersions() const
{
    SvStringsDtor *pList = new SvStringsDtor;
    SfxVersionInfo* pInfo = ((SfxVersionTableDtor*) this)->First();
    while ( pInfo )
    {
        String *pString = new String( pInfo->aComment );
        (*pString) += DEFINE_CONST_UNICODE( "; " );
        (*pString) += ConvertDateTime_Impl( pInfo->aCreateStamp );
        pList->Insert( pString, pList->Count() );
        pInfo = ((SfxVersionTableDtor*) this)->Next();
    }

    return pList;
}

