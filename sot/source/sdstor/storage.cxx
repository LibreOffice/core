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

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <rtl/digest.h>
#include <osl/file.hxx>
#include <sot/stg.hxx>
#include <sot/storinfo.hxx>
#include <sot/storage.hxx>
#include <sot/formats.hxx>
#include <sot/exchange.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/fsys.hxx>
#include <tools/cachestr.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;

/************** class SotStorageStream ***********************************/
class SotStorageStreamFactory : public SotFactory
{
public:
         TYPEINFO();
        SotStorageStreamFactory( const SvGlobalName & rName,
                              const String & rClassName,
                              CreateInstanceType pCreateFuncP )
            : SotFactory( rName, rClassName, pCreateFuncP )
        {}
};
TYPEINIT1(SotStorageStreamFactory,SotFactory);


SO2_IMPL_BASIC_CLASS1_DLL(SotStorageStream,SotStorageStreamFactory,SotObject,
                        SvGlobalName( 0xd7deb420, 0xf902, 0x11d0,
                            0xaa, 0xa1, 0x0, 0xa0, 0x24, 0x9d, 0x55, 0x90 ) )

/************************************************************************
|*    SotStorageStream::SotStorageStream()
|*
|*    Beschreibung
*************************************************************************/
SvLockBytesRef MakeLockBytes_Impl( const String & rName, StreamMode nMode )
{
    SvLockBytesRef xLB;
    if( rName.Len() )
    {
        SvStream * pFileStm = new SvFileStream( rName, nMode );
        xLB = new SvLockBytes( pFileStm, sal_True );
    }
    else
    {
        SvStream * pCacheStm = new SvCacheStream();
        xLB = new SvLockBytes( pCacheStm, sal_True );
    }
    return xLB;
}

SotStorageStream::SotStorageStream( const String & rName, StreamMode nMode,
                                  StorageMode
                                  #ifdef DBG_UTIL
                                  nStorageMode
                                  #endif
                                  )
    : SvStream( MakeLockBytes_Impl( rName, nMode ) )
    , pOwnStm( NULL )
{
    if( nMode & STREAM_WRITE )
        bIsWritable = sal_True;
    else
        bIsWritable = sal_False;

    DBG_ASSERT( !nStorageMode,"StorageModes ignored" );
}

SotStorageStream::SotStorageStream( BaseStorageStream * pStm )
{
    if( pStm )
    {
        if( STREAM_WRITE & pStm->GetMode() )
            bIsWritable = sal_True;
        else
            bIsWritable = sal_False;

        pOwnStm = pStm;
        SetError( pStm->GetError() );
        pStm->ResetError();
    }
    else
    {
        pOwnStm = NULL;
        bIsWritable = sal_True;
        SetError( SVSTREAM_INVALID_PARAMETER );
    }
}

SotStorageStream::SotStorageStream()
    : pOwnStm( NULL )
{
    // ??? wenn Init virtuell ist, entsprechen setzen
    bIsWritable = sal_True;
}

/************************************************************************
|*    SotStorageStream::~SotStorageStream()
|*
|*    Beschreibung
*************************************************************************/
SotStorageStream::~SotStorageStream()
{
    Flush(); //SetBufferSize(0);
    delete pOwnStm;
}

/*************************************************************************
|*    SotStorageStream::ResetError()
|*
|*    Beschreibung
*************************************************************************/
void SotStorageStream::ResetError()
{
    SvStream::ResetError();
    if( pOwnStm )
         pOwnStm->ResetError();
}

/*************************************************************************
|*    SotStorageStream::GetData()
|*
|*    Beschreibung
*************************************************************************/
sal_uLong SotStorageStream::GetData( void* pData, sal_uLong nSize )
{
    sal_uLong nRet = 0;

    if( pOwnStm )
    {
        nRet = pOwnStm->Read( pData, nSize );
        SetError( pOwnStm->GetError() );
    }
    else
        nRet = SvStream::GetData( (sal_Char *)pData, nSize );
    return nRet;
}

/*************************************************************************
|*    SotStorageStream::PutData()
|*
|*    Beschreibung
*************************************************************************/
sal_uLong SotStorageStream::PutData( const void* pData, sal_uLong nSize )
{
    sal_uLong nRet = 0;

    if( pOwnStm )
    {
        nRet = pOwnStm->Write( pData, nSize );
        SetError( pOwnStm->GetError() );
    }
    else
        nRet = SvStream::PutData( (sal_Char *)pData, nSize );
    return nRet;
}

/*************************************************************************
|*    SotStorageStream::SeekPos()
|*
|*    Beschreibung
*************************************************************************/
sal_uLong SotStorageStream::SeekPos( sal_uLong nPos )
{
    sal_uLong nRet = 0;

    if( pOwnStm )
    {
        nRet = pOwnStm->Seek( nPos );
        SetError( pOwnStm->GetError() );
    }
    else
        nRet = SvStream::SeekPos( nPos );
    return nRet;
}

/*************************************************************************
|*    SotStorageStream::Flush()
|*
|*    Beschreibung
*************************************************************************/
void SotStorageStream::FlushData()
{
    if( pOwnStm )
    {
        pOwnStm->Flush();
        SetError( pOwnStm->GetError() );
    }
    else
        SvStream::FlushData();
}

/*************************************************************************
|*    SotStorageStream::SetSize()
|*
|*    Beschreibung
*************************************************************************/
void SotStorageStream::SetSize( sal_uLong nNewSize )
{
    sal_uLong   nPos = Tell();
    if( pOwnStm )
    {
        pOwnStm->SetSize( nNewSize );
        SetError( pOwnStm->GetError() );
    }
    else
        SvStream::SetSize( nNewSize );

    if( nNewSize < nPos )
        // ans Ende setzen
        Seek( nNewSize );

    //return GetError() == SVSTREAM_OK;
}

/*************************************************************************
|*
|*    SotStorageStream::GetSize()
|*
|*    Beschreibung
|*
*************************************************************************/
sal_uInt32 SotStorageStream::GetSize() const
{
    sal_uLong nPos = Tell();
    ((SotStorageStream *)this)->Seek( STREAM_SEEK_TO_END );
    sal_uLong nSize = Tell();
    ((SotStorageStream *)this)->Seek( nPos );
    return nSize;
}

sal_Size SotStorageStream::remainingSize()
{
    if (pOwnStm)
        return pOwnStm->GetSize() - Tell();
    return SvStream::remainingSize();
}

/*************************************************************************
|*    SotStorageStream::CopyTo()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SotStorageStream::CopyTo( SotStorageStream * pDestStm )
{
    Flush(); // alle Daten schreiben
    pDestStm->ClearBuffer();
    if( !pOwnStm || !pDestStm->pOwnStm )
    { // Wenn Ole2 oder nicht nur eigene StorageStreams

        sal_uLong nPos = Tell();    // Position merken
        Seek( 0L );
        pDestStm->SetSize( 0 ); // Ziel-Stream leeren

        void * pMem = new sal_uInt8[ 8192 ];
        sal_uLong  nRead;
        while( 0 != (nRead = Read( pMem, 8192 )) )
        {
            if( nRead != pDestStm->Write( pMem, nRead ) )
            {
                SetError( SVSTREAM_GENERALERROR );
                break;
            }
        }
        delete [] static_cast<sal_uInt8*>(pMem);
        // Position setzen
        pDestStm->Seek( nPos );
        Seek( nPos );
    }
    else
    {
        /*
        // Kopieren
        nErr = pObjI->CopyTo( pDestStm->pObjI, uSize, NULL, &uWrite );
        if( SUCCEEDED( nErr ) )
        {
            // Ziel-Streamzeiger steht hinter den Daten
            // SvSeek abgleichen
            pDestStm->Seek( uWrite.LowPart );
        }
        else if( GetScode( nErr ) == E_NOTIMPL )
        { // Eines Tages werden alle MS... ?!#
        */
        pOwnStm->CopyTo( pDestStm->pOwnStm );
        SetError( pOwnStm->GetError() );
    }
    return GetError() == SVSTREAM_OK;
}

/*************************************************************************
|*    SotStorageStream::Commit()
|*    SotStorageStream::Revert()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SotStorageStream::Commit()
{
    if( pOwnStm )
    {
        pOwnStm->Flush();
        if( pOwnStm->GetError() == SVSTREAM_OK )
            pOwnStm->Commit();
        SetError( pOwnStm->GetError() );
    }
    return GetError() == SVSTREAM_OK;
}

sal_Bool SotStorageStream::Revert()
{
    if( !pOwnStm )
    {
        pOwnStm->Revert();
        SetError( pOwnStm->GetError() );
    }
    return GetError() == SVSTREAM_OK;
}

sal_Bool SotStorageStream::SetProperty( const String& rName, const ::com::sun::star::uno::Any& rValue )
{
    UCBStorageStream* pStg = PTR_CAST( UCBStorageStream, pOwnStm );
    if ( pStg )
    {
        return pStg->SetProperty( rName, rValue );
    }
    else
    {
        OSL_FAIL("Not implemented!");
        return sal_False;
    }
}

sal_Bool SotStorageStream::GetProperty( const String& rName, ::com::sun::star::uno::Any& rValue )
{
    UCBStorageStream* pStg = PTR_CAST( UCBStorageStream, pOwnStm );
    if ( pStg )
    {
        return pStg->GetProperty( rName, rValue );
    }
    else
    {
        OSL_FAIL("Not implemented!");
        return sal_False;
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SotStorageStream::GetXInputStream() const
{
    UCBStorageStream* pStg = PTR_CAST( UCBStorageStream, pOwnStm );
    if ( pStg )
    {
        return pStg->GetXInputStream();
    }
    else
    {
        OSL_FAIL("Not implemented!");
        return ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >();
    }
}



/************** class SotStorage ******************************************
*************************************************************************/
class SotStorageFactory : public SotFactory
{
public:
         TYPEINFO();
        SotStorageFactory( const SvGlobalName & rName,
                              const String & rClassName,
                              CreateInstanceType pCreateFuncP )
            : SotFactory( rName, rClassName, pCreateFuncP )
        {}
};
TYPEINIT1(SotStorageFactory,SotFactory);


SO2_IMPL_BASIC_CLASS1_DLL(SotStorage,SotStorageFactory,SotObject,
                        SvGlobalName( 0x980ce7e0, 0xf905, 0x11d0,
                            0xaa, 0xa1, 0x0, 0xa0, 0x24, 0x9d, 0x55, 0x90 ) )

/************************************************************************
|*
|*    SotStorage::SotStorage()
|*
|*    Beschreibung      Es muss ein I... Objekt an SvObject uebergeben
|*                      werden, da es sonst selbst ein IUnknown anlegt und
|*                      festlegt, dass alle weiteren I... Objekte mit
|*                      delete zerstoert werden (Owner() == sal_True).
|*                      Es werden aber nur IStorage Objekte benutzt und nicht
|*                      selbst implementiert, deshalb wird so getan, als ob
|*                      das IStorage Objekt von aussen kam und es wird mit
|*                      Release() freigegeben.
|*                      Die CreateStorage Methoden werden benoetigt, um
|*                      ein IStorage Objekt vor dem Aufruf von SvObject
|*                      zu erzeugen (Own, !Own automatik).
|*                      Hat CreateStorage ein Objekt erzeugt, dann wurde
|*                      der RefCounter schon um 1 erhoet.
|*                      Die Uebergabe erfolgt in pStorageCTor. Die Variable
|*                      ist NULL, wenn es nicht geklappt hat.
|*
*************************************************************************/
#define INIT_SotStorage()                   \
    : m_pOwnStg( NULL )                       \
    , m_pStorStm( NULL )                      \
    , m_nError( SVSTREAM_OK )               \
    , m_bIsRoot( sal_False )                    \
    , m_bDelStm( sal_False )                        \
    , m_nVersion( SOFFICE_FILEFORMAT_CURRENT )

SotStorage::SotStorage()
    INIT_SotStorage()
{
    // ??? What's this ???
}

#define ERASEMASK  ( STREAM_TRUNC | STREAM_WRITE | STREAM_SHARE_DENYALL )
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <ucbhelper/content.hxx>

SotStorage::SotStorage( const String & rName, StreamMode nMode, StorageMode nStorageMode )
    INIT_SotStorage()
{
    m_aName = rName; // Namen merken
    CreateStorage( sal_True, nMode, nStorageMode );
    if ( IsOLEStorage() )
        m_nVersion = SOFFICE_FILEFORMAT_50;
}

void SotStorage::CreateStorage( sal_Bool bForceUCBStorage, StreamMode nMode, StorageMode nStorageMode  )
{
    DBG_ASSERT( !m_pStorStm && !m_pOwnStg, "Use only in ctor!" );
    if( m_aName.Len() )
    {
        // named storage
        if( ( ( nMode & ERASEMASK ) == ERASEMASK ) )
            ::utl::UCBContentHelper::Kill( m_aName );

        INetURLObject aObj( m_aName );
        if ( aObj.GetProtocol() == INET_PROT_NOT_VALID )
        {
            rtl::OUString aURL;
            ::utl::LocalFileHelper::ConvertPhysicalNameToURL( m_aName, aURL );
            aObj.SetURL( aURL );
            m_aName = aObj.GetMainURL( INetURLObject::NO_DECODE );
        }

        // a new unpacked storage should be created
        if ( nStorageMode == STORAGE_CREATE_UNPACKED )
        {
            // don't open stream readwrite, content provider may not support this !
            String aURL = UCBStorage::CreateLinkFile( m_aName );
            if ( aURL.Len() )
            {
                ::ucbhelper::Content aContent( aURL, ::com::sun::star::uno::Reference < ::com::sun::star::ucb::XCommandEnvironment >() );
                m_pOwnStg = new UCBStorage( aContent, aURL, nMode, sal_False );
            }
            else
            {
                m_pOwnStg = new Storage( m_aName, nMode, (nStorageMode & STORAGE_TRANSACTED) ? sal_False : sal_True );
                SetError( ERRCODE_IO_NOTSUPPORTED );
            }
        }
        else
        {
            // check the stream
            m_pStorStm = ::utl::UcbStreamHelper::CreateStream( m_aName, nMode );
            if ( m_pStorStm && m_pStorStm->GetError() )
                DELETEZ( m_pStorStm );

            if ( m_pStorStm )
            {
                // try as UCBStorage, next try as OLEStorage
                sal_Bool bIsUCBStorage = UCBStorage::IsStorageFile( m_pStorStm );
                if ( !bIsUCBStorage && bForceUCBStorage )
                    // if UCBStorage has priority, it should not be used only if it is really an OLEStorage
                    bIsUCBStorage = !Storage::IsStorageFile( m_pStorStm );

                if ( bIsUCBStorage )
                {
                    if ( UCBStorage::GetLinkedFile( *m_pStorStm ).Len() )
                    {
                        // detect special unpacked storages
                        m_pOwnStg = new UCBStorage( *m_pStorStm, (nStorageMode & STORAGE_TRANSACTED) ? sal_False : sal_True );
                        m_bDelStm = sal_True;
                    }
                    else
                    {
                        // detect special disk spanned storages
                        if ( UCBStorage::IsDiskSpannedFile( m_pStorStm ) )
                            nMode |= STORAGE_DISKSPANNED_MODE;

                        // UCBStorage always works directly on the UCB content, so discard the stream first
                        DELETEZ( m_pStorStm );
                        m_pOwnStg = new UCBStorage( m_aName, nMode, (nStorageMode & STORAGE_TRANSACTED) ? sal_False : sal_True );
                    }
                }
                else
                {
                    // OLEStorage can be opened with a stream
                    m_pOwnStg = new Storage( *m_pStorStm, (nStorageMode & STORAGE_TRANSACTED) ? sal_False : sal_True );
                    m_bDelStm = sal_True;
                }
            }
            else if ( bForceUCBStorage )
            {
                m_pOwnStg = new UCBStorage( m_aName, nMode, (nStorageMode & STORAGE_TRANSACTED) ? sal_False : sal_True );
                SetError( ERRCODE_IO_NOTSUPPORTED );
            }
            else
            {
                m_pOwnStg = new Storage( m_aName, nMode, (nStorageMode & STORAGE_TRANSACTED) ? sal_False : sal_True );
                SetError( ERRCODE_IO_NOTSUPPORTED );
            }
        }
    }
    else
    {
        // temporary storage
        if ( bForceUCBStorage )
            m_pOwnStg = new UCBStorage( m_aName, nMode, (nStorageMode & STORAGE_TRANSACTED) ? sal_False : sal_True );
        else
            m_pOwnStg = new Storage( m_aName, nMode, (nStorageMode & STORAGE_TRANSACTED) ? sal_False : sal_True );
        m_aName = m_pOwnStg->GetName();
    }

    SetError( m_pOwnStg->GetError() );

    SignAsRoot( m_pOwnStg->IsRoot() );
}

SotStorage::SotStorage( sal_Bool bUCBStorage, const String & rName, StreamMode nMode, StorageMode nStorageMode )
    INIT_SotStorage()
{
    m_aName = rName;
    CreateStorage( bUCBStorage, nMode, nStorageMode );
    if ( IsOLEStorage() )
        m_nVersion = SOFFICE_FILEFORMAT_50;
}

SotStorage::SotStorage( BaseStorage * pStor )
    INIT_SotStorage()
{
    if ( pStor )
    {
        m_aName = pStor->GetName(); // Namen merken
        SignAsRoot( pStor->IsRoot() );
        SetError( pStor->GetError() );
    }

    m_pOwnStg = pStor;
    sal_uLong nErr = m_pOwnStg ? m_pOwnStg->GetError() : SVSTREAM_CANNOT_MAKE;
    SetError( nErr );
    if ( IsOLEStorage() )
        m_nVersion = SOFFICE_FILEFORMAT_50;
}

SotStorage::SotStorage( sal_Bool bUCBStorage, SvStream & rStm )
    INIT_SotStorage()
{
    SetError( rStm.GetError() );

    // try as UCBStorage, next try as OLEStorage
    if ( UCBStorage::IsStorageFile( &rStm ) || bUCBStorage )
        m_pOwnStg = new UCBStorage( rStm, sal_False );
    else
        m_pOwnStg = new Storage( rStm, sal_False );

    SetError( m_pOwnStg->GetError() );

    if ( IsOLEStorage() )
        m_nVersion = SOFFICE_FILEFORMAT_50;

    SignAsRoot( m_pOwnStg->IsRoot() );
}

SotStorage::SotStorage( SvStream & rStm )
    INIT_SotStorage()
{
    SetError( rStm.GetError() );

    // try as UCBStorage, next try as OLEStorage
    if ( UCBStorage::IsStorageFile( &rStm ) )
        m_pOwnStg = new UCBStorage( rStm, sal_False );
    else
        m_pOwnStg = new Storage( rStm, sal_False );

    SetError( m_pOwnStg->GetError() );

    if ( IsOLEStorage() )
        m_nVersion = SOFFICE_FILEFORMAT_50;

    SignAsRoot( m_pOwnStg->IsRoot() );
}

SotStorage::SotStorage( SvStream * pStm, sal_Bool bDelete )
    INIT_SotStorage()
{
    SetError( pStm->GetError() );

    // try as UCBStorage, next try as OLEStorage
    if ( UCBStorage::IsStorageFile( pStm ) )
        m_pOwnStg = new UCBStorage( *pStm, sal_False );
    else
        m_pOwnStg = new Storage( *pStm, sal_False );

    SetError( m_pOwnStg->GetError() );

    m_pStorStm = pStm;
    m_bDelStm = bDelete;
    if ( IsOLEStorage() )
        m_nVersion = SOFFICE_FILEFORMAT_50;

    SignAsRoot( m_pOwnStg->IsRoot() );
}

/*************************************************************************
|*    SotStorage::~SotStorage()
|*
|*    Beschreibung
*************************************************************************/
SotStorage::~SotStorage()
{
    delete m_pOwnStg;
    if( m_bDelStm )
        delete m_pStorStm;
}

/*************************************************************************
|*    SotStorage::CreateMemoryStream()
|*
|*    Beschreibung
*************************************************************************/
SvMemoryStream * SotStorage::CreateMemoryStream()
{
    SvMemoryStream * pStm = NULL;
    pStm = new SvMemoryStream( 0x8000, 0x8000 );
    SotStorageRef aStg = new SotStorage( *pStm );
    if( CopyTo( aStg ) )
        aStg->Commit();
    else
    {
        aStg.Clear(); // Storage vorher freigeben
        delete pStm;
        pStm = NULL;
    }
    return pStm;
}

/*************************************************************************
|*    SotStorage::GetStorage()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SotStorage::IsStorageFile( const String & rFileName )
{
    String aName( rFileName );
    INetURLObject aObj( aName );
    if ( aObj.GetProtocol() == INET_PROT_NOT_VALID )
    {
        rtl::OUString aURL;
        ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aURL );
        aObj.SetURL( aURL );
        aName = aObj.GetMainURL( INetURLObject::NO_DECODE );
    }

    SvStream * pStm = ::utl::UcbStreamHelper::CreateStream( aName, STREAM_STD_READ );
    sal_Bool bRet = SotStorage::IsStorageFile( pStm );
    delete pStm;
    return bRet;
}

sal_Bool SotStorage::IsStorageFile( SvStream* pStream )
{
    /** code for new storages must come first! **/
    if ( pStream )
    {
        long nPos = pStream->Tell();
        sal_Bool bRet = UCBStorage::IsStorageFile( pStream );
        if ( !bRet )
            bRet = Storage::IsStorageFile( pStream );
        pStream->Seek( nPos );
        return bRet;
    }
    else
        return sal_False;
}
/*************************************************************************
|*    SotStorage::GetStorage()
|*
|*    Beschreibung
*************************************************************************/
const String & SotStorage::GetName() const
{
    if( !m_aName.Len() )
    {
        DBG_ASSERT( Owner(), "must be owner" );
        if( m_pOwnStg )
            ((SotStorage *)this)->m_aName = m_pOwnStg->GetName();
    }
    return m_aName;
}

void SotStorage::SetName( const String& rName )
{
    // This method is necessary because most storages will not be opened with a FileName, but an external stream instead
    // This stream is a stream opened by a UCP and so aName is only used as a transport for all client code of the SotStorage
    // class that depends on the fact that a root storage has a name
    DBG_ASSERT( !GetName().Len(), "SetName() must not be called when the storage already has a name!" );
    m_aName = rName;
}

/*************************************************************************
|*    SotStorage::ResetError()
|*
|*    Beschreibung
*************************************************************************/
void SotStorage::ResetError()
{
    m_nError = SVSTREAM_OK;
    if( m_pOwnStg )
        m_pOwnStg->ResetError();
}

/*************************************************************************
|*    SotStorage::SetClass()
|*    SotStorage::SetConvertClass()
|*
|*    Beschreibung
*************************************************************************/
void SotStorage::SetClass( const SvGlobalName & rName,
                          sal_uLong nOriginalClipFormat,
                          const String & rUserTypeName )
{
    DBG_ASSERT( Owner(), "must be owner" );
    if( m_pOwnStg )
        m_pOwnStg->SetClass( rName, nOriginalClipFormat, rUserTypeName );
    else
        SetError( SVSTREAM_GENERALERROR );
}

void SotStorage::SetConvertClass( const SvGlobalName & rName,
                                 sal_uLong nOriginalClipFormat,
                                 const String & rUserTypeName )
{
    DBG_ASSERT( Owner(), "must be owner" );
    if( m_pOwnStg )
        m_pOwnStg->SetConvertClass( rName, nOriginalClipFormat, rUserTypeName );
    else
        SetError( SVSTREAM_GENERALERROR );
}

/*************************************************************************
|*    SotStorage::GetClassName()
|*    SotStorage::GetFormat()
|*    SotStorage::GetUserName()
|*    SotStorage::ShouldConvert()
|*
|*    Beschreibung
*************************************************************************/
SvGlobalName SotStorage::GetClassName()
{
    SvGlobalName aGN;
    DBG_ASSERT( Owner(), "must be owner" );
    if( m_pOwnStg )
        aGN = m_pOwnStg->GetClassName();
    else
        SetError( SVSTREAM_GENERALERROR );
    return aGN;
}

sal_uLong SotStorage::GetFormat()
{
    sal_uLong nFormat = 0;
    DBG_ASSERT( Owner(), "must be owner" );
    if( m_pOwnStg )
        nFormat = m_pOwnStg->GetFormat();
    else
        SetError( SVSTREAM_GENERALERROR );
    return nFormat;
}

String SotStorage::GetUserName()
{
    String aName;
    DBG_ASSERT( Owner(), "must be owner" );
    if( m_pOwnStg )
        aName = m_pOwnStg->GetUserName();
    else
        SetError( SVSTREAM_GENERALERROR );
    return aName;
}

sal_Bool SotStorage::ShouldConvert()
{
    DBG_ASSERT( Owner(), "must be owner" );
    if( m_pOwnStg )
        return m_pOwnStg->ShouldConvert();
    else
        SetError( SVSTREAM_GENERALERROR );
    return sal_False;
}

/*************************************************************************
|*    SotStorage::FillInfoList()
|*
|*    Beschreibung
*************************************************************************/
void SotStorage::FillInfoList( SvStorageInfoList * pFillList ) const
{
    DBG_ASSERT( Owner(), "must be owner" );
    if( m_pOwnStg )
        m_pOwnStg->FillInfoList( pFillList );
}

/*************************************************************************
|*    SotStorage::CopyTo()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SotStorage::CopyTo( SotStorage * pDestStg )
{
    DBG_ASSERT( Owner(), "must be owner" );
    DBG_ASSERT( pDestStg->Owner(), "must be owner" );
    if( m_pOwnStg && pDestStg->m_pOwnStg )
    {
        m_pOwnStg->CopyTo( pDestStg->m_pOwnStg );
        SetError( m_pOwnStg->GetError() );
        pDestStg->m_aKey = m_aKey;
        pDestStg->m_nVersion = m_nVersion;
    }
    else
        SetError( SVSTREAM_GENERALERROR );
    return SVSTREAM_OK == GetError();
}

/*************************************************************************
|*    SotStorage::Commit()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SotStorage::Commit()
{
    DBG_ASSERT( Owner(), "must be owner" );
    if( m_pOwnStg )
    {
        if( !m_pOwnStg->Commit() )
            SetError( m_pOwnStg->GetError() );
    }
    else
        SetError( SVSTREAM_GENERALERROR );
    return SVSTREAM_OK == GetError();
}

/*************************************************************************
|*    SotStorage::Revert()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SotStorage::Revert()
{
    DBG_ASSERT( Owner(), "must be owner" );
    if( m_pOwnStg )
    {
        if( !m_pOwnStg->Revert() )
            SetError( m_pOwnStg->GetError() );
    }
    else
        SetError( SVSTREAM_GENERALERROR );
    return SVSTREAM_OK == GetError();
}

SotStorageStream * SotStorage::OpenSotStream( const String & rEleName,
                                             StreamMode nMode,
                                             StorageMode nStorageMode )
{
    DBG_ASSERT( !nStorageMode, "StorageModes ignored" );
    SotStorageStream * pStm = NULL;
    DBG_ASSERT( Owner(), "must be owner" );
    if( m_pOwnStg )
    {
        // volle Ole-Patches einschalten
        // egal was kommt, nur exclusiv gestattet
        nMode |= STREAM_SHARE_DENYALL;
        ErrCode nE = m_pOwnStg->GetError();
        BaseStorageStream * p = m_pOwnStg->OpenStream( rEleName, nMode,
                            (nStorageMode & STORAGE_TRANSACTED) ? sal_False : sal_True );
        pStm = new SotStorageStream( p );

        if( !nE )
            m_pOwnStg->ResetError(); // kein Fehler setzen
        if( nMode & STREAM_TRUNC )
            pStm->SetSize( 0 );
    }
    else
        SetError( SVSTREAM_GENERALERROR );
    return pStm;
}

/*************************************************************************
|*    SotStorage::OpenStorage()
|*
|*    Beschreibung
*************************************************************************/
SotStorage * SotStorage::OpenSotStorage( const String & rEleName,
                                        StreamMode nMode,
                                        StorageMode nStorageMode )
{
    SotStorage * pStor = NULL;
    DBG_ASSERT( Owner(), "must be owner" );
    if( m_pOwnStg )
    {
        nMode |= STREAM_SHARE_DENYALL;
        ErrCode nE = m_pOwnStg->GetError();
        BaseStorage * p = m_pOwnStg->OpenStorage( rEleName, nMode,
                        (nStorageMode & STORAGE_TRANSACTED) ? sal_False : sal_True );
        if( p )
        {
            pStor = new SotStorage( p );
            if( !nE )
                m_pOwnStg->ResetError(); // kein Fehler setzen

            return pStor;
        }
    }

    SetError( SVSTREAM_GENERALERROR );

    return NULL;
}

SotStorage * SotStorage::OpenUCBStorage( const String & rEleName,
                                        StreamMode nMode,
                                        StorageMode nStorageMode )
{
    SotStorage * pStor = NULL;
    DBG_ASSERT( Owner(), "must be owner" );
    if( m_pOwnStg )
    {
        nMode |= STREAM_SHARE_DENYALL;
        ErrCode nE = m_pOwnStg->GetError();
        BaseStorage * p = m_pOwnStg->OpenUCBStorage( rEleName, nMode,
                        (nStorageMode & STORAGE_TRANSACTED) ? sal_False : sal_True );
        pStor = new SotStorage( p );
        if( !nE )
            m_pOwnStg->ResetError(); // kein Fehler setzen
    }
    else
        SetError( SVSTREAM_GENERALERROR );
    return pStor;
}

/*************************************************************************
|*    SotStorage::IsStream()
|*    SotStorage::IsStorage()
|*    SotStorage::IsContained()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SotStorage::IsStorage( const String & rEleName ) const
{
    DBG_ASSERT( Owner(), "must be owner" );
    // ein bisschen schneller
    if( m_pOwnStg )
        return m_pOwnStg->IsStorage( rEleName );
    return sal_False;
}

sal_Bool SotStorage::IsStream( const String & rEleName ) const
{
    DBG_ASSERT( Owner(), "must be owner" );
    // ein bisschen schneller
    if( m_pOwnStg )
        return m_pOwnStg->IsStream( rEleName );
    return sal_False;
}

sal_Bool SotStorage::IsContained( const String & rEleName ) const
{
    DBG_ASSERT( Owner(), "must be owner" );
    // ein bisschen schneller
    if( m_pOwnStg )
        return m_pOwnStg->IsContained( rEleName );
    return sal_False;
}

/*************************************************************************
|*    SotStorage::Remove()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SotStorage::Remove( const String & rEleName )
{
    DBG_ASSERT( Owner(), "must be owner" );
    if( m_pOwnStg )
    {
        m_pOwnStg->Remove( rEleName );
        SetError( m_pOwnStg->GetError() );
    }
    else
        SetError( SVSTREAM_GENERALERROR );
    return SVSTREAM_OK == GetError();
}

/*************************************************************************
|*    SotStorage::Rename()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SotStorage::Rename( const String & rEleName, const String & rNewName )
{
    DBG_ASSERT( Owner(), "must be owner" );
    if( m_pOwnStg )
    {
        m_pOwnStg->Rename( rEleName, rNewName );
        SetError( m_pOwnStg->GetError() );
    }
    else
        SetError( SVSTREAM_GENERALERROR );
    return SVSTREAM_OK == GetError();
}

/*************************************************************************
|*    SotStorage::CopyTo()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SotStorage::CopyTo( const String & rEleName,
                        SotStorage * pNewSt, const String & rNewName )
{
    DBG_ASSERT( Owner(), "must be owner" );
    DBG_ASSERT( pNewSt->Owner(), "must be owner" );
    if( m_pOwnStg )
    {
        m_pOwnStg->CopyTo( rEleName, pNewSt->m_pOwnStg, rNewName );
        SetError( m_pOwnStg->GetError() );
        SetError( pNewSt->GetError() );
    }
    else
        SetError( SVSTREAM_GENERALERROR );
    return SVSTREAM_OK == GetError();
}

/*************************************************************************
|*    SotStorage::MoveTo()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SotStorage::MoveTo( const String & rEleName,
                        SotStorage * pNewSt, const String & rNewName )
{
    DBG_ASSERT( Owner(), "must be owner" );
    DBG_ASSERT( pNewSt->Owner(), "must be owner" );
    if( m_pOwnStg )
    {
        m_pOwnStg->MoveTo( rEleName, pNewSt->m_pOwnStg, rNewName );
        SetError( m_pOwnStg->GetError() );
        SetError( pNewSt->GetError() );
    }
    else
        SetError( SVSTREAM_GENERALERROR );
    return SVSTREAM_OK == GetError();
}

const SvStream* SotStorage::GetSvStream()
{
    const SvStream* pResult = 0;
    DBG_ASSERT( Owner(), "must be owner" );
    if( m_pOwnStg )
        pResult = m_pOwnStg->GetSvStream();
    return pResult;
}

sal_Bool SotStorage::Validate()
{
    DBG_ASSERT( m_bIsRoot, "Validate nur an Rootstorage" );
    if( m_pOwnStg )
        return m_pOwnStg->ValidateFAT();
    else
        return sal_True;
}

sal_Bool SotStorage::SetProperty( const String& rName, const ::com::sun::star::uno::Any& rValue )
{
    UCBStorage* pStg = PTR_CAST( UCBStorage, m_pOwnStg );
    if ( pStg )
    {
        return pStg->SetProperty( rName, rValue );
    }
    else
    {
        DBG_WARNING("W1:Not implemented!");
        return sal_False;
    }
}

sal_Bool SotStorage::GetProperty( const String& rName, ::com::sun::star::uno::Any& rValue )
{
    UCBStorage* pStg = PTR_CAST( UCBStorage, m_pOwnStg );
    if ( pStg )
    {
        return pStg->GetProperty( rName, rValue );
    }
    else if ( rName.CompareToAscii("MediaType") == COMPARE_EQUAL )
    {
        String aStr = SotExchange::GetFormatMimeType( GetFormat() );
        sal_uInt16 nPos = aStr.Search(';');
        if ( nPos != STRING_NOTFOUND )
            aStr = aStr.Copy( 0, nPos );
        rValue <<= (::rtl::OUString) aStr;
        return sal_True;
    }
    else
    {
        DBG_WARNING("W1:Not implemented!");
        return sal_False;
    }
}

sal_Bool SotStorage::IsOLEStorage() const
{
    UCBStorage* pStg = PTR_CAST( UCBStorage, m_pOwnStg );
    return !pStg;
}

sal_Bool SotStorage::IsOLEStorage( const String & rFileName )
{
    return Storage::IsStorageFile( rFileName );
}

sal_Bool SotStorage::IsOLEStorage( SvStream* pStream )
{
    return Storage::IsStorageFile( pStream );
}

void SotStorage::SetKey( const rtl::OString& rKey )
{
    m_aKey = rKey;
    if ( !IsOLEStorage() )
    {
        sal_uInt8 aBuffer[RTL_DIGEST_LENGTH_SHA1];
        rtlDigestError nError = rtl_digest_SHA1( m_aKey.getStr(), m_aKey.getLength(), aBuffer, RTL_DIGEST_LENGTH_SHA1 );
        if ( nError == rtl_Digest_E_None )
        {
            sal_uInt8* pBuffer = aBuffer;
            ::com::sun::star::uno::Sequence < sal_Int8 > aSequ( (sal_Int8*) pBuffer, RTL_DIGEST_LENGTH_SHA1 );
            ::com::sun::star::uno::Any aAny;
            aAny <<= aSequ;
            SetProperty( ::rtl::OUString("EncryptionKey"), aAny );
        }
    }
}

SotStorage* SotStorage::OpenOLEStorage( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage,
                                    const String& rEleName, StreamMode nMode )
{
    sal_Int32 nEleMode = embed::ElementModes::SEEKABLEREAD;
    if ( nMode & STREAM_WRITE )
        nEleMode |= embed::ElementModes::WRITE;
    if ( nMode & STREAM_TRUNC )
        nEleMode |= embed::ElementModes::TRUNCATE;
    if ( nMode & STREAM_NOCREATE )
        nEleMode |= embed::ElementModes::NOCREATE;

    SvStream* pStream = NULL;
    try
    {
        uno::Reference < io::XStream > xStream = xStorage->openStreamElement( rEleName, nEleMode );

        // TODO/LATER: should it be done this way?
        if ( nMode & STREAM_WRITE )
        {
            uno::Reference < beans::XPropertySet > xStreamProps( xStream, uno::UNO_QUERY_THROW );
            xStreamProps->setPropertyValue(
                        ::rtl::OUString(  "MediaType"  ),
                        uno::makeAny( ::rtl::OUString(  "application/vnd.sun.star.oleobject"  ) ) );
        }

           pStream = utl::UcbStreamHelper::CreateStream( xStream );
    }
    catch ( uno::Exception& )
    {
        //TODO/LATER: ErrorHandling
        pStream = new SvMemoryStream;
        pStream->SetError( ERRCODE_IO_GENERAL );
    }

    return new SotStorage( pStream, sal_True );
}

sal_Int32 SotStorage::GetFormatID( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage )
{
    uno::Reference< beans::XPropertySet > xProps( xStorage, uno::UNO_QUERY );
    if ( !xProps.is() )
        return 0;

    ::rtl::OUString aMediaType;
    xProps->getPropertyValue( ::rtl::OUString("MediaType") ) >>= aMediaType;
    if ( !aMediaType.isEmpty() )
    {
        ::com::sun::star::datatransfer::DataFlavor aDataFlavor;
        aDataFlavor.MimeType = aMediaType;
        return SotExchange::GetFormat( aDataFlavor );
    }

    return 0;
}

sal_Int32 SotStorage::GetVersion( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage )
{
    sal_Int32 nSotFormatID = SotStorage::GetFormatID( xStorage );
    switch( nSotFormatID )
    {
        case SOT_FORMATSTR_ID_STARWRITER_8:
        case SOT_FORMATSTR_ID_STARWRITER_8_TEMPLATE:
        case SOT_FORMATSTR_ID_STARWRITERWEB_8:
        case SOT_FORMATSTR_ID_STARWRITERGLOB_8:
        case SOT_FORMATSTR_ID_STARDRAW_8:
        case SOT_FORMATSTR_ID_STARDRAW_8_TEMPLATE:
        case SOT_FORMATSTR_ID_STARIMPRESS_8:
        case SOT_FORMATSTR_ID_STARIMPRESS_8_TEMPLATE:
        case SOT_FORMATSTR_ID_STARCALC_8:
        case SOT_FORMATSTR_ID_STARCALC_8_TEMPLATE:
        case SOT_FORMATSTR_ID_STARCHART_8:
        case SOT_FORMATSTR_ID_STARCHART_8_TEMPLATE:
        case SOT_FORMATSTR_ID_STARMATH_8:
        case SOT_FORMATSTR_ID_STARMATH_8_TEMPLATE:
            return SOFFICE_FILEFORMAT_8;
        case SOT_FORMATSTR_ID_STARWRITER_60:
        case SOT_FORMATSTR_ID_STARWRITERWEB_60:
        case SOT_FORMATSTR_ID_STARWRITERGLOB_60:
        case SOT_FORMATSTR_ID_STARDRAW_60:
        case SOT_FORMATSTR_ID_STARIMPRESS_60:
        case SOT_FORMATSTR_ID_STARCALC_60:
        case SOT_FORMATSTR_ID_STARCHART_60:
        case SOT_FORMATSTR_ID_STARMATH_60:
            return SOFFICE_FILEFORMAT_60;
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
