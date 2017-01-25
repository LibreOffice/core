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

#include <sal/config.h>

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
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <ucbhelper/content.hxx>

#include <memory>

using namespace ::com::sun::star;

SvLockBytesRef MakeLockBytes_Impl( const OUString & rName, StreamMode nMode )
{
    SvLockBytesRef xLB;
    if( !rName.isEmpty() )
    {
        SvStream * pFileStm = new SvFileStream( rName, nMode );
        xLB = new SvLockBytes( pFileStm, true );
    }
    else
    {
        SvStream * pCacheStm = new SvMemoryStream();
        xLB = new SvLockBytes( pCacheStm, true );
    }
    return xLB;
}

SotStorageStream::SotStorageStream( const OUString & rName, StreamMode nMode )
    : SvStream( MakeLockBytes_Impl( rName, nMode ).get() )
    , pOwnStm( nullptr )
{
    if( nMode & StreamMode::WRITE )
        m_isWritable = true;
    else
        m_isWritable = false;
}

SotStorageStream::SotStorageStream( BaseStorageStream * pStm )
{
    if( pStm )
    {
        if( StreamMode::WRITE & pStm->GetMode() )
            m_isWritable = true;
        else
            m_isWritable = false;

        pOwnStm = pStm;
        SetError( pStm->GetError() );
        pStm->ResetError();
    }
    else
    {
        pOwnStm = nullptr;
        m_isWritable = true;
        SetError( SVSTREAM_INVALID_PARAMETER );
    }
}

SotStorageStream::~SotStorageStream()
{
    Flush(); //SetBufferSize(0);
    delete pOwnStm;
}

void SotStorageStream::ResetError()
{
    SvStream::ResetError();
    if( pOwnStm )
         pOwnStm->ResetError();
}

std::size_t SotStorageStream::GetData(void* pData, std::size_t const nSize)
{
    std::size_t nRet = 0;

    if( pOwnStm )
    {
        nRet = pOwnStm->Read( pData, nSize );
        SetError( pOwnStm->GetError() );
    }
    else
        nRet = SvStream::GetData( pData, nSize );

    return nRet;
}

std::size_t SotStorageStream::PutData(const void* pData, std::size_t const nSize)
{
    std::size_t nRet = 0;

    if( pOwnStm )
    {
        nRet = pOwnStm->Write( pData, nSize );
        SetError( pOwnStm->GetError() );
    }
    else
        nRet = SvStream::PutData( pData, nSize );
    return nRet;
}

sal_uInt64 SotStorageStream::SeekPos(sal_uInt64 nPos)
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

void SotStorageStream::SetSize(sal_uInt64 const nNewSize)
{
    sal_uInt64 const nPos = Tell();
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
}

sal_uInt32 SotStorageStream::GetSize() const
{
    sal_uLong nPos = Tell();
    const_cast<SotStorageStream *>(this)->Seek( STREAM_SEEK_TO_END );
    sal_uLong nSize = Tell();
    const_cast<SotStorageStream *>(this)->Seek( nPos );
    return nSize;
}

sal_uInt64 SotStorageStream::remainingSize()
{
    if (pOwnStm)
        return pOwnStm->GetSize() - Tell();

    return SvStream::remainingSize();
}

void SotStorageStream::CopyTo( SotStorageStream * pDestStm )
{
    Flush(); // alle Daten schreiben
    pDestStm->ClearBuffer();
    if( !pOwnStm || !pDestStm->pOwnStm )
    {
        // Wenn Ole2 oder nicht nur eigene StorageStreams
        sal_uLong nPos = Tell();    // Position merken
        Seek( 0L );
        pDestStm->SetSize( 0 ); // Ziel-Stream leeren

        std::unique_ptr<sal_uInt8[]> pMem(new sal_uInt8[ 8192 ]);
        sal_uLong  nRead;
        while (0 != (nRead = ReadBytes(pMem.get(), 8192)))
        {
            if (nRead != pDestStm->WriteBytes(pMem.get(), nRead))
            {
                SetError( SVSTREAM_GENERALERROR );
                break;
            }
        }
        pMem.reset();
        // Position setzen
        pDestStm->Seek( nPos );
        Seek( nPos );
    }
    else
    {
        pOwnStm->CopyTo( pDestStm->pOwnStm );
        SetError( pOwnStm->GetError() );
    }
}

bool SotStorageStream::Commit()
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

bool SotStorageStream::SetProperty( const OUString& rName, const css::uno::Any& rValue )
{
    UCBStorageStream* pStg =  dynamic_cast<UCBStorageStream*>( pOwnStm );
    if ( pStg )
    {
        return pStg->SetProperty( rName, rValue );
    }
    else
    {
        OSL_FAIL("Not implemented!");
        return false;
    }
}

/************************************************************************
|*
|*    SotStorage::SotStorage()
|*
|*    Beschreibung      Es muss ein I... Objekt an SvObject uebergeben
|*                      werden, da es sonst selbst ein IUnknown anlegt und
|*                      festlegt, dass alle weiteren I... Objekte mit
|*                      delete zerstoert werden (Owner() == true).
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
#define INIT_SotStorage()                     \
    : m_pOwnStg( nullptr )                       \
    , m_pStorStm( nullptr )                      \
    , m_nError( SVSTREAM_OK )                 \
    , m_bIsRoot( false )                      \
    , m_bDelStm( false )                      \
    , m_nVersion( SOFFICE_FILEFORMAT_CURRENT )

#define ERASEMASK  ( StreamMode::TRUNC | StreamMode::WRITE | StreamMode::SHARE_DENYALL )

SotStorage::SotStorage( const OUString & rName, StreamMode nMode )
    INIT_SotStorage()
{
    m_aName = rName; // Namen merken
    CreateStorage( true, nMode );
    if ( IsOLEStorage() )
        m_nVersion = SOFFICE_FILEFORMAT_50;
}

void SotStorage::CreateStorage( bool bForceUCBStorage, StreamMode nMode )
{
    DBG_ASSERT( !m_pStorStm && !m_pOwnStg, "Use only in ctor!" );
    if( !m_aName.isEmpty() )
    {
        // named storage
        if( ( ( nMode & ERASEMASK ) == ERASEMASK ) )
            ::utl::UCBContentHelper::Kill( m_aName );

        INetURLObject aObj( m_aName );
        if ( aObj.GetProtocol() == INetProtocol::NotValid )
        {
            OUString aURL;
            osl::FileBase::getFileURLFromSystemPath( m_aName, aURL );
            aObj.SetURL( aURL );
            m_aName = aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        }

        // check the stream
        m_pStorStm = ::utl::UcbStreamHelper::CreateStream( m_aName, nMode );
        if ( m_pStorStm && m_pStorStm->GetError() )
            DELETEZ( m_pStorStm );

        if ( m_pStorStm )
        {
            // try as UCBStorage, next try as OLEStorage
            bool bIsUCBStorage = UCBStorage::IsStorageFile( m_pStorStm );
            if ( !bIsUCBStorage && bForceUCBStorage )
                // if UCBStorage has priority, it should not be used only if it is really an OLEStorage
                bIsUCBStorage = !Storage::IsStorageFile( m_pStorStm );

            if ( bIsUCBStorage )
            {
                if ( !(UCBStorage::GetLinkedFile( *m_pStorStm ).isEmpty()) )
                {
                    // detect special unpacked storages
                    m_pOwnStg = new UCBStorage( *m_pStorStm, true );
                    m_bDelStm = true;
                }
                else
                {
                    // UCBStorage always works directly on the UCB content, so discard the stream first
                    DELETEZ( m_pStorStm );
                    m_pOwnStg = new UCBStorage( m_aName, nMode, true, true/*bIsRoot*/ );
                }
            }
            else
            {
                // OLEStorage can be opened with a stream
                m_pOwnStg = new Storage( *m_pStorStm, true );
                m_bDelStm = true;
            }
        }
        else if ( bForceUCBStorage )
        {
            m_pOwnStg = new UCBStorage( m_aName, nMode, true, true/*bIsRoot*/ );
            SetError( ERRCODE_IO_NOTSUPPORTED );
        }
        else
        {
            m_pOwnStg = new Storage( m_aName, nMode, true );
            SetError( ERRCODE_IO_NOTSUPPORTED );
        }
    }
    else
    {
        // temporary storage
        if ( bForceUCBStorage )
            m_pOwnStg = new UCBStorage( m_aName, nMode, true, true/*bIsRoot*/ );
        else
            m_pOwnStg = new Storage( m_aName, nMode, true );
        m_aName = m_pOwnStg->GetName();
    }

    SetError( m_pOwnStg->GetError() );

    SignAsRoot( m_pOwnStg->IsRoot() );
}

SotStorage::SotStorage( bool bUCBStorage, const OUString & rName, StreamMode nMode )
    INIT_SotStorage()
{
    m_aName = rName;
    CreateStorage( bUCBStorage, nMode );
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
    const ErrCode nErr = m_pOwnStg ? m_pOwnStg->GetError() : SVSTREAM_CANNOT_MAKE;
    SetError( nErr );
    if ( IsOLEStorage() )
        m_nVersion = SOFFICE_FILEFORMAT_50;
}

SotStorage::SotStorage( bool bUCBStorage, SvStream & rStm )
    INIT_SotStorage()
{
    SetError( rStm.GetError() );

    // try as UCBStorage, next try as OLEStorage
    if ( UCBStorage::IsStorageFile( &rStm ) || bUCBStorage )
        m_pOwnStg = new UCBStorage( rStm, false );
    else
        m_pOwnStg = new Storage( rStm, false );

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
        m_pOwnStg = new UCBStorage( rStm, false );
    else
        m_pOwnStg = new Storage( rStm, false );

    SetError( m_pOwnStg->GetError() );

    if ( IsOLEStorage() )
        m_nVersion = SOFFICE_FILEFORMAT_50;

    SignAsRoot( m_pOwnStg->IsRoot() );
}

SotStorage::SotStorage( SvStream * pStm, bool bDelete )
    INIT_SotStorage()
{
    SetError( pStm->GetError() );

    // try as UCBStorage, next try as OLEStorage
    if ( UCBStorage::IsStorageFile( pStm ) )
        m_pOwnStg = new UCBStorage( *pStm, false );
    else
        m_pOwnStg = new Storage( *pStm, false );

    SetError( m_pOwnStg->GetError() );

    m_pStorStm = pStm;
    m_bDelStm = bDelete;
    if ( IsOLEStorage() )
        m_nVersion = SOFFICE_FILEFORMAT_50;

    SignAsRoot( m_pOwnStg->IsRoot() );
}

SotStorage::~SotStorage()
{
    delete m_pOwnStg;
    if( m_bDelStm )
        delete m_pStorStm;
}

SvMemoryStream * SotStorage::CreateMemoryStream()
{
    SvMemoryStream * pStm = nullptr;
    pStm = new SvMemoryStream( 0x8000, 0x8000 );
    tools::SvRef<SotStorage> aStg = new SotStorage( *pStm );
    if( CopyTo( aStg.get() ) )
    {
        aStg->Commit();
    }
    else
    {
        aStg.clear(); // Storage vorher freigeben
        delete pStm;
        pStm = nullptr;
    }
    return pStm;
}

bool SotStorage::IsStorageFile( const OUString & rFileName )
{
    OUString aName( rFileName );
    INetURLObject aObj( aName );
    if ( aObj.GetProtocol() == INetProtocol::NotValid )
    {
        OUString aURL;
        osl::FileBase::getFileURLFromSystemPath( aName, aURL );
        aObj.SetURL( aURL );
        aName = aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    }

    std::unique_ptr<SvStream> pStm(::utl::UcbStreamHelper::CreateStream( aName, StreamMode::STD_READ ));
    bool bRet = SotStorage::IsStorageFile( pStm.get() );
    return bRet;
}

bool SotStorage::IsStorageFile( SvStream* pStream )
{
    /** code for new storages must come first! **/
    if ( pStream )
    {
        long nPos = pStream->Tell();
        bool bRet = UCBStorage::IsStorageFile( pStream );
        if ( !bRet )
            bRet = Storage::IsStorageFile( pStream );
        pStream->Seek( nPos );
        return bRet;
    }
    else
        return false;
}

const OUString & SotStorage::GetName() const
{
    if( m_aName.isEmpty() )
    {
        if( m_pOwnStg )
            const_cast<SotStorage *>(this)->m_aName = m_pOwnStg->GetName();
    }
    return m_aName;
}

void SotStorage::SetClass( const SvGlobalName & rName,
                           SotClipboardFormatId nOriginalClipFormat,
                           const OUString & rUserTypeName )
{
    if( m_pOwnStg )
        m_pOwnStg->SetClass( rName, nOriginalClipFormat, rUserTypeName );
    else
        SetError( SVSTREAM_GENERALERROR );
}

SvGlobalName SotStorage::GetClassName()
{
    SvGlobalName aGN;
    if( m_pOwnStg )
        aGN = m_pOwnStg->GetClassName();
    else
        SetError( SVSTREAM_GENERALERROR );
    return aGN;
}

SotClipboardFormatId SotStorage::GetFormat()
{
    SotClipboardFormatId nFormat = SotClipboardFormatId::NONE;
    if( m_pOwnStg )
        nFormat = m_pOwnStg->GetFormat();
    else
        SetError( SVSTREAM_GENERALERROR );
    return nFormat;
}

OUString SotStorage::GetUserName()
{
    OUString aName;
    if( m_pOwnStg )
        aName = m_pOwnStg->GetUserName();
    else
        SetError( SVSTREAM_GENERALERROR );
    return aName;
}

void SotStorage::FillInfoList( SvStorageInfoList * pFillList ) const
{
    if( m_pOwnStg )
        m_pOwnStg->FillInfoList( pFillList );
}

bool SotStorage::CopyTo( SotStorage * pDestStg )
{
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

bool SotStorage::Commit()
{
    if( m_pOwnStg )
    {
        if( !m_pOwnStg->Commit() )
            SetError( m_pOwnStg->GetError() );
    }
    else
        SetError( SVSTREAM_GENERALERROR );

    return SVSTREAM_OK == GetError();
}

SotStorageStream * SotStorage::OpenSotStream( const OUString & rEleName,
                                              StreamMode nMode )
{
    SotStorageStream * pStm = nullptr;
    if( m_pOwnStg )
    {
        // volle Ole-Patches einschalten
        // egal was kommt, nur exclusiv gestattet
        nMode |= StreamMode::SHARE_DENYALL;
        ErrCode nE = m_pOwnStg->GetError();
        BaseStorageStream * p = m_pOwnStg->OpenStream( rEleName, nMode );
        pStm = new SotStorageStream( p );

        if( !nE )
            m_pOwnStg->ResetError(); // kein Fehler setzen
        if( nMode & StreamMode::TRUNC )
            pStm->SetSize( 0 );
    }
    else
        SetError( SVSTREAM_GENERALERROR );

    return pStm;
}

SotStorage * SotStorage::OpenSotStorage( const OUString & rEleName,
                                         StreamMode nMode,
                                         bool transacted )
{
    if( m_pOwnStg )
    {
        nMode |= StreamMode::SHARE_DENYALL;
        ErrCode nE = m_pOwnStg->GetError();
        BaseStorage * p = m_pOwnStg->OpenStorage(rEleName, nMode, !transacted);
        if( p )
        {
            SotStorage * pStor = new SotStorage( p );
            if( !nE )
                m_pOwnStg->ResetError(); // kein Fehler setzen

            return pStor;
        }
    }

    SetError( SVSTREAM_GENERALERROR );

    return nullptr;
}

bool SotStorage::IsStorage( const OUString & rEleName ) const
{
    // ein bisschen schneller
    if( m_pOwnStg )
        return m_pOwnStg->IsStorage( rEleName );

    return false;
}

bool SotStorage::IsStream( const OUString & rEleName ) const
{
    // ein bisschen schneller
    if( m_pOwnStg )
        return m_pOwnStg->IsStream( rEleName );

    return false;
}

bool SotStorage::IsContained( const OUString & rEleName ) const
{
    // ein bisschen schneller
    if( m_pOwnStg )
        return m_pOwnStg->IsContained( rEleName );

    return false;
}

bool SotStorage::Remove( const OUString & rEleName )
{
    if( m_pOwnStg )
    {
        m_pOwnStg->Remove( rEleName );
        SetError( m_pOwnStg->GetError() );
    }
    else
        SetError( SVSTREAM_GENERALERROR );

    return SVSTREAM_OK == GetError();
}

bool SotStorage::CopyTo( const OUString & rEleName,
                         SotStorage * pNewSt, const OUString & rNewName )
{
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

bool SotStorage::Validate()
{
    DBG_ASSERT( m_bIsRoot, "Validate nur an Rootstorage" );
    if( m_pOwnStg )
        return m_pOwnStg->ValidateFAT();
    else
        return true;
}

bool SotStorage::IsOLEStorage() const
{
    UCBStorage* pStg =  dynamic_cast<UCBStorage*>( m_pOwnStg );
    return !pStg;
}

bool SotStorage::IsOLEStorage( const OUString & rFileName )
{
    return Storage::IsStorageFile( rFileName );
}

bool SotStorage::IsOLEStorage( SvStream* pStream )
{
    return Storage::IsStorageFile( pStream );
}

SotStorage* SotStorage::OpenOLEStorage( const css::uno::Reference < css::embed::XStorage >& xStorage,
                                        const OUString& rEleName, StreamMode nMode )
{
    sal_Int32 nEleMode = embed::ElementModes::SEEKABLEREAD;
    if ( nMode & StreamMode::WRITE )
        nEleMode |= embed::ElementModes::WRITE;
    if ( nMode & StreamMode::TRUNC )
        nEleMode |= embed::ElementModes::TRUNCATE;
    if ( nMode & StreamMode::NOCREATE )
        nEleMode |= embed::ElementModes::NOCREATE;

    SvStream* pStream = nullptr;
    try
    {
        uno::Reference < io::XStream > xStream = xStorage->openStreamElement( rEleName, nEleMode );

        // TODO/LATER: should it be done this way?
        if ( nMode & StreamMode::WRITE )
        {
            uno::Reference < beans::XPropertySet > xStreamProps( xStream, uno::UNO_QUERY_THROW );
            xStreamProps->setPropertyValue( "MediaType",
                                            uno::makeAny( OUString(  "application/vnd.sun.star.oleobject"  ) ) );
        }

           pStream = utl::UcbStreamHelper::CreateStream( xStream );
    }
    catch ( uno::Exception& )
    {
        //TODO/LATER: ErrorHandling
        pStream = new SvMemoryStream;
        pStream->SetError( ERRCODE_IO_GENERAL );
    }

    return new SotStorage( pStream, true );
}

SotClipboardFormatId SotStorage::GetFormatID( const css::uno::Reference < css::embed::XStorage >& xStorage )
{
    uno::Reference< beans::XPropertySet > xProps( xStorage, uno::UNO_QUERY );
    if ( !xProps.is() )
        return SotClipboardFormatId::NONE;

    OUString aMediaType;
    xProps->getPropertyValue("MediaType") >>= aMediaType;
    if ( !aMediaType.isEmpty() )
    {
        css::datatransfer::DataFlavor aDataFlavor;
        aDataFlavor.MimeType = aMediaType;
        return SotExchange::GetFormat( aDataFlavor );
    }

    return SotClipboardFormatId::NONE;
}

sal_Int32 SotStorage::GetVersion( const css::uno::Reference < css::embed::XStorage >& xStorage )
{
    SotClipboardFormatId nSotFormatID = SotStorage::GetFormatID( xStorage );
    switch( nSotFormatID )
    {
    case SotClipboardFormatId::STARWRITER_8:
    case SotClipboardFormatId::STARWRITER_8_TEMPLATE:
    case SotClipboardFormatId::STARWRITERWEB_8:
    case SotClipboardFormatId::STARWRITERGLOB_8:
    case SotClipboardFormatId::STARWRITERGLOB_8_TEMPLATE:
    case SotClipboardFormatId::STARDRAW_8:
    case SotClipboardFormatId::STARDRAW_8_TEMPLATE:
    case SotClipboardFormatId::STARIMPRESS_8:
    case SotClipboardFormatId::STARIMPRESS_8_TEMPLATE:
    case SotClipboardFormatId::STARCALC_8:
    case SotClipboardFormatId::STARCALC_8_TEMPLATE:
    case SotClipboardFormatId::STARCHART_8:
    case SotClipboardFormatId::STARCHART_8_TEMPLATE:
    case SotClipboardFormatId::STARMATH_8:
    case SotClipboardFormatId::STARMATH_8_TEMPLATE:
        return SOFFICE_FILEFORMAT_8;
    case SotClipboardFormatId::STARWRITER_60:
    case SotClipboardFormatId::STARWRITERWEB_60:
    case SotClipboardFormatId::STARWRITERGLOB_60:
    case SotClipboardFormatId::STARDRAW_60:
    case SotClipboardFormatId::STARIMPRESS_60:
    case SotClipboardFormatId::STARCALC_60:
    case SotClipboardFormatId::STARCHART_60:
    case SotClipboardFormatId::STARMATH_60:
        return SOFFICE_FILEFORMAT_60;
    default: break;
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
