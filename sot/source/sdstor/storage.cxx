/*************************************************************************
 *
 *  $RCSfile: storage.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-24 16:42:38 $
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

#include <osl/file.hxx>
#include <stg.hxx>
#include <storinfo.hxx>
#include <storage.hxx>

#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _TOOLS_FSYS_HXX
#include <tools/fsys.hxx>
#endif
#ifndef _CACHESTR_HXX //autogen
#include <tools/cachestr.hxx>
#endif
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <unotools/localfilehelper.hxx>
#pragma hdrstop

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
SO2_IMPL_INVARIANT(SotStorageStream)


void SotStorageStream::TestMemberObjRef( BOOL /*bFree*/ )
{
}

#ifdef TEST_INVARIANT
void SotStorageStream::TestMemberInvariant( BOOL /*bPrint*/ )
{
}
#endif

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
        xLB = new SvLockBytes( pFileStm, TRUE );
    }
    else
    {
        SvStream * pCacheStm = new SvCacheStream();
        xLB = new SvLockBytes( pCacheStm, TRUE );
    }
    return xLB;
}

SotStorageStream::SotStorageStream( const String & rName, StreamMode nMode,
                                  StorageMode nStorageMode)
    : SvStream( MakeLockBytes_Impl( rName, nMode ) )
    , pOwnStm( NULL )
{
    if( nMode & STREAM_WRITE )
        bIsWritable = TRUE;
    else
        bIsWritable = FALSE;

    DBG_ASSERT( !nStorageMode,"StorageModes ignored" )
}

SotStorageStream::SotStorageStream( BaseStorageStream * pStm )
{
    if( STREAM_WRITE & pStm->GetMode() )
        bIsWritable = TRUE;
    else
        bIsWritable = FALSE;

    pOwnStm = pStm;
    SetError( pStm->GetError() );
    pStm->ResetError();
}

SotStorageStream::SotStorageStream()
    : pOwnStm( NULL )
{
    // ??? wenn Init virtuell ist, entsprechen setzen
    bIsWritable = TRUE;
}

/************************************************************************
|*    SotStorageStream::~SotStorageStream()
|*
|*    Beschreibung
*************************************************************************/
SotStorageStream::~SotStorageStream()
{

    SetBufferSize( 0 );
    // Hack, wegen Fehler in den SD-Storages
    // Commit();
    delete pOwnStm;
}

/*************************************************************************
|*    SotStorageStream::SyncSvStream()
|*
|*    Beschreibung: Der SvStream wird auf den Zustand des Standard-Streams
|*                  gesetzt. Der Puffer des SvStreams wird weggeworfen.
*************************************************************************/
void SotStorageStream::SyncSvStream()
{
    ULONG nPos = 0;
    if( pOwnStm )
    {
        pOwnStm->Flush();
        nPos = pOwnStm->Tell();
        SetError( pOwnStm->GetError() );
        SvStream::SyncSvStream( nPos );
    }
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
ULONG SotStorageStream::GetData( void* pData, ULONG nSize )
{
    ULONG nRet = 0;

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
ULONG SotStorageStream::PutData( const void* pData, ULONG nSize )
{
    ULONG nRet = 0;

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
ULONG SotStorageStream::SeekPos( ULONG nPos )
{
    ULONG nRet = 0;

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
void SotStorageStream::SetSize( ULONG nNewSize )
{
    ULONG   nPos = Tell();
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
UINT32 SotStorageStream::GetSize() const
{
    ULONG nPos = Tell();
    ((SotStorageStream *)this)->Seek( STREAM_SEEK_TO_END );
    ULONG nSize = Tell();
    ((SotStorageStream *)this)->Seek( nPos );
    return nSize;
}

/*************************************************************************
|*    SotStorageStream::CopyTo()
|*
|*    Beschreibung
*************************************************************************/
BOOL SotStorageStream::CopyTo( SotStorageStream * pDestStm )
{
    Flush(); // alle Daten schreiben
    pDestStm->ClearBuffer();
    if( !pOwnStm || !pDestStm->pOwnStm )
    { // Wenn Ole2 oder nicht nur eigene StorageStreams

        ULONG nPos = Tell();    // Position merken
        pDestStm->SetSize( 0 ); // Ziel-Stream leeren

        void * pMem = new BYTE[ 8192 ];
        ULONG  nRead;
        while( 0 != (nRead = Read( pMem, 8192 )) )
        {
            if( nRead != pDestStm->Write( pMem, nRead ) )
            {
                SetError( SVSTREAM_GENERALERROR );
                break;
            }
        }
        delete pMem;
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
BOOL SotStorageStream::Commit()
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

BOOL SotStorageStream::Revert()
{
    if( !pOwnStm )
    {
        pOwnStm->Revert();
        SetError( pOwnStm->GetError() );
    }
    return GetError() == SVSTREAM_OK;
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
SO2_IMPL_INVARIANT(SotStorage)


/************************************************************************
|*
|*    SotStorage::Tes*()
|*
|*    Beschreibung
*************************************************************************/
void SotStorage::TestMemberObjRef( BOOL /*bFree*/ )
{
}

#ifdef TEST_INVARIANT
void SotStorage::TestMemberInvariant( BOOL bPrint )
{
}
#endif

/************************************************************************
|*
|*    SotStorage::SotStorage()
|*
|*    Beschreibung      Es muss ein I... Objekt an SvObject uebergeben
|*                      werden, da es sonst selbst ein IUnknown anlegt und
|*                      festlegt, dass alle weiteren I... Objekte mit
|*                      delete zerstoert werden (Owner() == TRUE).
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
|*    Ersterstellung    MM 23.06.94
|*    Letzte Aenderung  MM 23.06.94
|*
*************************************************************************/
#define INIT_SotStorage()                   \
    : nError( SVSTREAM_OK )                 \
    , bIsRoot( FALSE )                      \
    , bDelStm( FALSE )                      \
    , nVersion( SOFFICE_FILEFORMAT_NOW )    \
    , pOwnStg( NULL )   \
    , pStorStm( NULL )

SotStorage::SotStorage()
    INIT_SotStorage()
{
    // ??? What's this ???
}

#define ERASEMASK  ( STREAM_TRUNC | STREAM_WRITE | STREAM_SHARE_DENYALL )

SotStorage::SotStorage( const String & rName, StreamMode nMode, StorageMode nStorageMode )
    INIT_SotStorage()
{
    aName = rName; // Namen merken
    if( aName.Len() )
    {
        // named storage
        if( ( ( nMode & ERASEMASK ) == ERASEMASK ) )
            // ???
            osl::File::remove( rName );

        String aURL;
        INetURLObject aObj( rName );
        if ( aObj.GetProtocol() == INET_PROT_NOT_VALID )
        {
            String aURL;
            ::utl::LocalFileHelper::ConvertPhysicalNameToURL( rName, aURL );
            aObj.SetURL( aURL );
        }

//        pStorStm = ::utl::UcbStreamHelper::CreateStream( aObj.GetMainURL(), nMode );
        pStorStm = new SvFileStream( aObj.GetMainURL(), nMode );
        if ( pStorStm )
        {
            // try as UCBStorage, next try as OLEStorage
            if ( UCBStorage::IsStorageFile( pStorStm ) )
            {
                // UCBStorage always works directly on the UCB content, so discard the stream first
                DELETEZ( pStorStm );
                DBG_ASSERT( (nStorageMode & STORAGE_TRANSACTED), "No direct mode supported!" );
                pOwnStg = new UCBStorage( rName, nMode );
            }
            else
            {
                // OLEStorage can be opened with a stream
                pOwnStg = new Storage( *pStorStm, (nStorageMode & STORAGE_TRANSACTED) ? FALSE : TRUE );
                bDelStm = TRUE;
            }
        }
        else
        {
            pOwnStg = new Storage( rName, nMode, (nStorageMode & STORAGE_TRANSACTED) ? FALSE : TRUE );
            SetError( ERRCODE_IO_NOTSUPPORTED );
        }
    }
    else
    {
        // temporary storage
        // try as UCBStorage, next try as OLEStorage ( how to check which one is desired ?! )
        pOwnStg = new Storage( rName, nMode, (nStorageMode & STORAGE_TRANSACTED) ? FALSE : TRUE );
        aName = pOwnStg->GetName();
    }

    ULONG nErr = pOwnStg->GetError();
    SetError( nErr );
}

SotStorage::SotStorage( BaseStorage * pStor )
    INIT_SotStorage()
{
    aName = pStor->GetName(); // Namen merken
    SignAsRoot( pStor->IsRoot() );
    SetError( pStor->GetError() );
    pOwnStg = pStor;
    ULONG nErr = pOwnStg->GetError();
    SetError( nErr );
}

SotStorage::SotStorage( SvStream & rStm )
    INIT_SotStorage()
{
    SetError( rStm.GetError() );

    // try as UCBStorage, next try as OLEStorage
    if ( UCBStorage::IsStorageFile( pStorStm ) )
        pOwnStg = new UCBStorage( rStm, FALSE );
    else
        pOwnStg = new Storage( rStm, FALSE );
}

SotStorage::SotStorage( SvStream * pStm, BOOL bDelete )
    INIT_SotStorage()
{
    SetError( pStm->GetError() );

    // try as UCBStorage, next try as OLEStorage
    if ( UCBStorage::IsStorageFile( pStorStm ) )
        pOwnStg = new UCBStorage( *pStm, FALSE );
    else
        pOwnStg = new Storage( *pStm, FALSE );

    ULONG nError = pOwnStg->GetError();
    if ( nError != SVSTREAM_OK )
        SetError( nError );

    pStorStm = pStm;
    bDelStm = bDelete;
}

/*************************************************************************
|*    SotStorage::~SotStorage()
|*
|*    Beschreibung
*************************************************************************/
SotStorage::~SotStorage()
{
    delete pOwnStg;
    if( bDelStm )
        delete pStorStm;
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
    }
    return pStm;
}

/*************************************************************************
|*    SotStorage::GetStorage()
|*
|*    Beschreibung
*************************************************************************/
BOOL SotStorage::IsStorageFile( const String & rFileName )
{
    /** code for new storages must come first! **/
    return Storage::IsStorageFile( rFileName );
}

BOOL SotStorage::IsStorageFile( SvStream* pStream )
{
    /** code for new storages must come first! **/
    if ( pStream )
    {
        long nPos = pStream->Tell();
        BOOL bRet = Storage::IsStorageFile( pStream );
        pStream->Seek( nPos );
        return bRet;
    }
    else
        return FALSE;
}
/*************************************************************************
|*    SotStorage::GetStorage()
|*
|*    Beschreibung
*************************************************************************/
const String & SotStorage::GetName() const
{
    if( !aName.Len() )
    {
        DBG_ASSERT( Owner(), "must be owner" )
        if( pOwnStg )
            ((SotStorage *)this)->aName = pOwnStg->GetName();
    }
    return aName;
}

void SotStorage::SetName( const String& rName )
{
    // This method is necessary because most storages will not be opened with a FileName, but an external stream instead
    // This stream is a stream opened by a UCP and so aName is only used as a transport for all client code of the SotStorage
    // class that depends on the fact that a root storage has a name
    DBG_ASSERT( !GetName().Len(), "SetName() must not be called when the storage already has a name!" );
    aName = rName;
}

/*************************************************************************
|*    SotStorage::ResetError()
|*
|*    Beschreibung
*************************************************************************/
void SotStorage::ResetError()
{
    nError = SVSTREAM_OK;
    if( pOwnStg )
        pOwnStg->ResetError();
}

/*************************************************************************
|*    SotStorage::SetClass()
|*    SotStorage::SetConvertClass()
|*
|*    Beschreibung
*************************************************************************/
void SotStorage::SetClass( const SvGlobalName & rName,
                          ULONG nOriginalClipFormat,
                          const String & rUserTypeName )
{
    DBG_ASSERT( Owner(), "must be owner" )
    if( pOwnStg )
        pOwnStg->SetClass( rName, nOriginalClipFormat, rUserTypeName );
    else
        SetError( SVSTREAM_GENERALERROR );
}

void SotStorage::SetConvertClass( const SvGlobalName & rName,
                                 ULONG nOriginalClipFormat,
                                 const String & rUserTypeName )
{
    DBG_ASSERT( Owner(), "must be owner" )
    if( pOwnStg )
        pOwnStg->SetConvertClass( rName, nOriginalClipFormat, rUserTypeName );
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
    DBG_ASSERT( Owner(), "must be owner" )
    if( pOwnStg )
        aGN = pOwnStg->GetClassName();
    else
        SetError( SVSTREAM_GENERALERROR );
    return aGN;
}

ULONG SotStorage::GetFormat()
{
    ULONG nFormat = 0;
    DBG_ASSERT( Owner(), "must be owner" )
    if( pOwnStg )
        nFormat = pOwnStg->GetFormat();
    else
        SetError( SVSTREAM_GENERALERROR );
    return nFormat;
}

String SotStorage::GetUserName()
{
    String aName;
    DBG_ASSERT( Owner(), "must be owner" )
    if( pOwnStg )
        aName = pOwnStg->GetUserName();
    else
        SetError( SVSTREAM_GENERALERROR );
    return aName;
}

BOOL SotStorage::ShouldConvert()
{
    DBG_ASSERT( Owner(), "must be owner" )
    if( pOwnStg )
        return pOwnStg->ShouldConvert();
    else
        SetError( SVSTREAM_GENERALERROR );
    return FALSE;
}

/*************************************************************************
|*    SotStorage::FillInfoList()
|*
|*    Beschreibung
*************************************************************************/
void SotStorage::FillInfoList( SvStorageInfoList * pFillList ) const
{
    DBG_ASSERT( Owner(), "must be owner" )
    if( pOwnStg )
        pOwnStg->FillInfoList( pFillList );
}

/*************************************************************************
|*    SotStorage::CopyTo()
|*
|*    Beschreibung
*************************************************************************/
BOOL SotStorage::CopyTo( SotStorage * pDestStg )
{
    DBG_ASSERT( Owner(), "must be owner" )
    DBG_ASSERT( pDestStg->Owner(), "must be owner" )
    if( pOwnStg && pDestStg->pOwnStg )
    {
        pOwnStg->CopyTo( pDestStg->pOwnStg );
        SetError( pOwnStg->GetError() );
        pDestStg->aKey = aKey;
        pDestStg->nVersion = nVersion;
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
BOOL SotStorage::Commit()
{
    DBG_ASSERT( Owner(), "must be owner" )
    if( pOwnStg )
    {
        if( !pOwnStg->Commit() )
            SetError( pOwnStg->GetError() );
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
BOOL SotStorage::Revert()
{
    DBG_ASSERT( Owner(), "must be owner" )
    if( pOwnStg )
    {
        if( !pOwnStg->Revert() )
            SetError( pOwnStg->GetError() );
    }
    else
        SetError( SVSTREAM_GENERALERROR );
    return SVSTREAM_OK == GetError();
}

/*************************************************************************
|*    SotStorage::OpenStream()
|*
|*    Beschreibung
*************************************************************************/
SotStorageStream * SotStorage::OpenSotStream( const String & rEleName,
                                             StreamMode nMode,
                                             StorageMode nStorageMode )
{
    DBG_ASSERT( !nStorageMode, "StorageModes ignored" )

    SotStorageStream * pStm = NULL;
    DBG_ASSERT( Owner(), "must be owner" )
    if( pOwnStg )
    {
        // volle Ole-Patches einschalten
        // egal was kommt, nur exclusiv gestattet
        nMode |= STREAM_SHARE_DENYALL;
        ErrCode nE = pOwnStg->GetError();
        BaseStorageStream * p = pOwnStg->OpenStream( rEleName, nMode,
                            (nStorageMode & STORAGE_TRANSACTED) ? FALSE : TRUE );
        pStm = new SotStorageStream( p );
        if( !nE )
            pOwnStg->ResetError(); // kein Fehler setzen
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
    DBG_ASSERT( Owner(), "must be owner" )
    if( pOwnStg )
    {
        nMode |= STREAM_SHARE_DENYALL;
        ErrCode nE = pOwnStg->GetError();
        BaseStorage * p = pOwnStg->OpenStorage( rEleName, nMode,
                        (nStorageMode & STORAGE_TRANSACTED) ? FALSE : TRUE );
        pStor = new SotStorage( p );
        if( !nE )
            pOwnStg->ResetError(); // kein Fehler setzen
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
BOOL SotStorage::IsStorage( const String & rEleName ) const
{
    DBG_ASSERT( Owner(), "must be owner" )
    // ein bisschen schneller
    if( pOwnStg )
        return pOwnStg->IsStorage( rEleName );
    return FALSE;
}

BOOL SotStorage::IsStream( const String & rEleName ) const
{
    DBG_ASSERT( Owner(), "must be owner" )
    // ein bisschen schneller
    if( pOwnStg )
        return pOwnStg->IsStream( rEleName );
    return FALSE;
}

BOOL SotStorage::IsContained( const String & rEleName ) const
{
    DBG_ASSERT( Owner(), "must be owner" )
    // ein bisschen schneller
    if( pOwnStg )
        return pOwnStg->IsContained( rEleName );
    return FALSE;
}

/*************************************************************************
|*    SotStorage::Remove()
|*
|*    Beschreibung
*************************************************************************/
BOOL SotStorage::Remove( const String & rEleName )
{
    DBG_ASSERT( Owner(), "must be owner" )
    if( pOwnStg )
    {
        pOwnStg->Remove( rEleName );
        SetError( pOwnStg->GetError() );
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
BOOL SotStorage::Rename( const String & rEleName, const String & rNewName )
{
    DBG_ASSERT( Owner(), "must be owner" )
    if( pOwnStg )
    {
        pOwnStg->Rename( rEleName, rNewName );
        SetError( pOwnStg->GetError() );
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
BOOL SotStorage::CopyTo( const String & rEleName,
                        SotStorage * pNewSt, const String & rNewName )
{
    DBG_ASSERT( Owner(), "must be owner" )
    DBG_ASSERT( pNewSt->Owner(), "must be owner" )
    if( pOwnStg )
    {
        pOwnStg->CopyTo( rEleName, pNewSt->pOwnStg, rNewName );
        SetError( pOwnStg->GetError() );
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
BOOL SotStorage::MoveTo( const String & rEleName,
                        SotStorage * pNewSt, const String & rNewName )
{
    DBG_ASSERT( Owner(), "must be owner" )
    DBG_ASSERT( pNewSt->Owner(), "must be owner" )
    if( pOwnStg )
    {
        pOwnStg->MoveTo( rEleName, pNewSt->pOwnStg, rNewName );
        SetError( pOwnStg->GetError() );
        SetError( pNewSt->GetError() );
    }
    else
        SetError( SVSTREAM_GENERALERROR );
    return SVSTREAM_OK == GetError();
}

const SvStream* SotStorage::GetSvStream()
{
    const SvStream* pResult = 0;
    DBG_ASSERT( Owner(), "must be owner" )
    if( pOwnStg )
        pResult = pOwnStg->GetSvStream();
    return pResult;
}

SvStream* SotStorage::GetTargetSvStream() const
{
    SvStream* pResult = 0;
    DBG_ASSERT( Owner(), "must be owner" )
    if( pOwnStg )
        pResult = (SvStream*)(pOwnStg->GetSvStream());
    return pResult;
}


BOOL SotStorage::Validate()
{
    DBG_ASSERT( bIsRoot, "Validate nur an Rootstorage" );
    if( pOwnStg )
        return pOwnStg->ValidateFAT();
    else
        return TRUE;
}


