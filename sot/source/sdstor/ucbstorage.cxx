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

#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/io/BufferSizeExceededException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <ucbhelper/content.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/ResultSetException.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <com/sun/star/ucb/ContentInfo.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/packages/manifest/XManifestWriter.hpp>
#include <com/sun/star/packages/manifest/XManifestReader.hpp>
#include <com/sun/star/ucb/InteractiveIOException.hpp>

#include <rtl/digest.h>
#include <tools/ref.hxx>
#include <tools/debug.hxx>
#include <unotools/streamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase2.hxx>
#include <ucbhelper/commandenvironment.hxx>

#include "sot/stg.hxx"
#include "sot/storinfo.hxx"
#include <sot/storage.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include "sot/clsids.hxx"

#include <vector>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::sdbc;
using namespace ::ucbhelper;

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
static int nOpenFiles=0;
static int nOpenStreams=0;
#endif

typedef ::cppu::WeakImplHelper2 < XInputStream, XSeekable > FileInputStreamWrapper_Base;
class FileStreamWrapper_Impl : public FileInputStreamWrapper_Base
{
protected:
    ::osl::Mutex    m_aMutex;
    String          m_aURL;
    SvStream*       m_pSvStream;

public:
    FileStreamWrapper_Impl( const String& rName );
    virtual ~FileStreamWrapper_Impl();

    virtual void SAL_CALL seek( sal_Int64 _nLocation ) throw ( IllegalArgumentException, IOException, RuntimeException);
    virtual sal_Int64 SAL_CALL getPosition(  ) throw ( IOException, RuntimeException);
    virtual sal_Int64 SAL_CALL getLength(  ) throw ( IOException, RuntimeException);
    virtual sal_Int32 SAL_CALL readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead) throw( NotConnectedException, BufferSizeExceededException, RuntimeException );
    virtual sal_Int32 SAL_CALL readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead) throw( NotConnectedException, BufferSizeExceededException, RuntimeException );
    virtual void      SAL_CALL skipBytes(sal_Int32 nBytesToSkip) throw( NotConnectedException, BufferSizeExceededException, RuntimeException);
    virtual sal_Int32 SAL_CALL available() throw( NotConnectedException, RuntimeException );
    virtual void      SAL_CALL closeInput() throw( NotConnectedException, RuntimeException );

protected:
    void checkConnected();
    void checkError();
};

//------------------------------------------------------------------
FileStreamWrapper_Impl::FileStreamWrapper_Impl( const String& rName )
    : m_aURL( rName )
    , m_pSvStream(0)
{
    // if no URL is provided the stream is empty
}

//------------------------------------------------------------------
FileStreamWrapper_Impl::~FileStreamWrapper_Impl()
{
    if ( m_pSvStream )
    {
        delete m_pSvStream;
#if OSL_DEBUG_LEVEL > 1
        --nOpenFiles;
#endif
    }

    if ( m_aURL.Len() )
        ::utl::UCBContentHelper::Kill( m_aURL );
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL FileStreamWrapper_Impl::readBytes(Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
                throw( NotConnectedException, BufferSizeExceededException, RuntimeException )
{
    if ( !m_aURL.Len() )
    {
        aData.realloc( 0 );
        return 0;
    }

    checkConnected();

    if (nBytesToRead < 0)
        throw BufferSizeExceededException(::rtl::OUString(),static_cast<XWeak*>(this));

    ::osl::MutexGuard aGuard( m_aMutex );

    aData.realloc(nBytesToRead);

    sal_uInt32 nRead = m_pSvStream->Read((void*)aData.getArray(), nBytesToRead);
    checkError();

    // Wenn gelesene Zeichen < MaxLength, Sequence anpassen
    if (nRead < (sal_uInt32)nBytesToRead)
        aData.realloc( nRead );

    return nRead;
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL FileStreamWrapper_Impl::readSomeBytes(Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead) throw( NotConnectedException, BufferSizeExceededException, RuntimeException )
{
    if ( !m_aURL.Len() )
    {
        aData.realloc( 0 );
        return 0;
    }

    checkError();

    if (nMaxBytesToRead < 0)
        throw BufferSizeExceededException(::rtl::OUString(),static_cast<XWeak*>(this));

    if (m_pSvStream->IsEof())
    {
        aData.realloc(0);
        return 0;
    }
    else
        return readBytes(aData, nMaxBytesToRead);
}

//------------------------------------------------------------------------------
void SAL_CALL FileStreamWrapper_Impl::skipBytes(sal_Int32 nBytesToSkip) throw( NotConnectedException, BufferSizeExceededException, RuntimeException )
{
    if ( !m_aURL.Len() )
        return;

    ::osl::MutexGuard aGuard( m_aMutex );
    checkError();

    m_pSvStream->SeekRel(nBytesToSkip);
    checkError();
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL FileStreamWrapper_Impl::available() throw( NotConnectedException, RuntimeException )
{
    if ( !m_aURL.Len() )
        return 0;

    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    sal_uInt32 nPos = m_pSvStream->Tell();
    checkError();

    m_pSvStream->Seek(STREAM_SEEK_TO_END);
    checkError();

    sal_Int32 nAvailable = (sal_Int32)m_pSvStream->Tell() - nPos;
    m_pSvStream->Seek(nPos);
    checkError();

    return nAvailable;
}

//------------------------------------------------------------------------------
void SAL_CALL FileStreamWrapper_Impl::closeInput() throw( NotConnectedException, RuntimeException )
{
    if ( !m_aURL.Len() )
        return;

    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();
    DELETEZ( m_pSvStream );
#if OSL_DEBUG_LEVEL > 1
    --nOpenFiles;
#endif
    ::utl::UCBContentHelper::Kill( m_aURL );
    m_aURL.Erase();
}

//------------------------------------------------------------------------------
void SAL_CALL FileStreamWrapper_Impl::seek( sal_Int64 _nLocation ) throw (IllegalArgumentException, IOException, RuntimeException)
{
    if ( !m_aURL.Len() )
        return;

    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    m_pSvStream->Seek((sal_uInt32)_nLocation);
    checkError();
}

//------------------------------------------------------------------------------
sal_Int64 SAL_CALL FileStreamWrapper_Impl::getPosition(  ) throw (IOException, RuntimeException)
{
    if ( !m_aURL.Len() )
        return 0;

    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    sal_uInt32 nPos = m_pSvStream->Tell();
    checkError();
    return (sal_Int64)nPos;
}

//------------------------------------------------------------------------------
sal_Int64 SAL_CALL FileStreamWrapper_Impl::getLength(  ) throw (IOException, RuntimeException)
{
    if ( !m_aURL.Len() )
        return 0;

    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    sal_uInt32 nCurrentPos = m_pSvStream->Tell();
    checkError();

    m_pSvStream->Seek(STREAM_SEEK_TO_END);
    sal_uInt32 nEndPos = m_pSvStream->Tell();
    m_pSvStream->Seek(nCurrentPos);

    checkError();

    return (sal_Int64)nEndPos;
}

//------------------------------------------------------------------------------
void FileStreamWrapper_Impl::checkConnected()
{
    if ( !m_aURL.Len() )
        throw NotConnectedException(::rtl::OUString(), const_cast<XWeak*>(static_cast<const XWeak*>(this)));
    if ( !m_pSvStream )
    {
        m_pSvStream = ::utl::UcbStreamHelper::CreateStream( m_aURL, STREAM_STD_READ );
#if OSL_DEBUG_LEVEL > 1
        ++nOpenFiles;
#endif
    }
}

//------------------------------------------------------------------------------
void FileStreamWrapper_Impl::checkError()
{
    checkConnected();

    if (m_pSvStream->SvStream::GetError() != ERRCODE_NONE)
        // TODO: really evaluate the error
        throw NotConnectedException(::rtl::OUString(), const_cast<XWeak*>(static_cast<const XWeak*>(this)));
}

TYPEINIT1( UCBStorageStream, BaseStorageStream );
TYPEINIT1( UCBStorage, BaseStorage );

#define COMMIT_RESULT_FAILURE           0
#define COMMIT_RESULT_NOTHING_TO_DO     1
#define COMMIT_RESULT_SUCCESS           2

#define min( x, y ) (( x < y ) ? x : y)
#define max( x, y ) (( x > y ) ? x : y)

sal_Int32 GetFormatId_Impl( SvGlobalName aName )
{
    if ( aName == SvGlobalName( SO3_SW_CLASSID_60 ) )
        return SOT_FORMATSTR_ID_STARWRITER_60;
    if ( aName == SvGlobalName( SO3_SWWEB_CLASSID_60 ) )
        return SOT_FORMATSTR_ID_STARWRITERWEB_60;
    if ( aName == SvGlobalName( SO3_SWGLOB_CLASSID_60 ) )
        return SOT_FORMATSTR_ID_STARWRITERGLOB_60;
    if ( aName == SvGlobalName( SO3_SDRAW_CLASSID_60 ) )
        return SOT_FORMATSTR_ID_STARDRAW_60;
    if ( aName == SvGlobalName( SO3_SIMPRESS_CLASSID_60 ) )
        return SOT_FORMATSTR_ID_STARIMPRESS_60;
    if ( aName == SvGlobalName( SO3_SC_CLASSID_60 ) )
        return SOT_FORMATSTR_ID_STARCALC_60;
    if ( aName == SvGlobalName( SO3_SCH_CLASSID_60 ) )
        return SOT_FORMATSTR_ID_STARCHART_60;
    if ( aName == SvGlobalName( SO3_SM_CLASSID_60 ) )
        return SOT_FORMATSTR_ID_STARMATH_60;
    if ( aName == SvGlobalName( SO3_OUT_CLASSID ) ||
         aName == SvGlobalName( SO3_APPLET_CLASSID ) ||
         aName == SvGlobalName( SO3_PLUGIN_CLASSID ) ||
         aName == SvGlobalName( SO3_IFRAME_CLASSID ) )
        // allowed, but not supported
        return 0;
    else
    {
        OSL_FAIL( "Unknown UCB storage format!" );
        return 0;
    }
}


SvGlobalName GetClassId_Impl( sal_Int32 nFormat )
{
    switch ( nFormat )
    {
        case SOT_FORMATSTR_ID_STARWRITER_8 :
        case SOT_FORMATSTR_ID_STARWRITER_8_TEMPLATE :
            return SvGlobalName( SO3_SW_CLASSID_60 );
        case SOT_FORMATSTR_ID_STARWRITERWEB_8 :
            return SvGlobalName( SO3_SWWEB_CLASSID_60 );
        case SOT_FORMATSTR_ID_STARWRITERGLOB_8 :
            return SvGlobalName( SO3_SWGLOB_CLASSID_60 );
        case SOT_FORMATSTR_ID_STARDRAW_8 :
        case SOT_FORMATSTR_ID_STARDRAW_8_TEMPLATE :
            return SvGlobalName( SO3_SDRAW_CLASSID_60 );
        case SOT_FORMATSTR_ID_STARIMPRESS_8 :
        case SOT_FORMATSTR_ID_STARIMPRESS_8_TEMPLATE :
            return SvGlobalName( SO3_SIMPRESS_CLASSID_60 );
        case SOT_FORMATSTR_ID_STARCALC_8 :
        case SOT_FORMATSTR_ID_STARCALC_8_TEMPLATE :
            return SvGlobalName( SO3_SC_CLASSID_60 );
        case SOT_FORMATSTR_ID_STARCHART_8 :
        case SOT_FORMATSTR_ID_STARCHART_8_TEMPLATE :
            return SvGlobalName( SO3_SCH_CLASSID_60 );
        case SOT_FORMATSTR_ID_STARMATH_8 :
        case SOT_FORMATSTR_ID_STARMATH_8_TEMPLATE :
            return SvGlobalName( SO3_SM_CLASSID_60 );
        case SOT_FORMATSTR_ID_STARWRITER_60 :
            return SvGlobalName( SO3_SW_CLASSID_60 );
        case SOT_FORMATSTR_ID_STARWRITERWEB_60 :
            return SvGlobalName( SO3_SWWEB_CLASSID_60 );
        case SOT_FORMATSTR_ID_STARWRITERGLOB_60 :
            return SvGlobalName( SO3_SWGLOB_CLASSID_60 );
        case SOT_FORMATSTR_ID_STARDRAW_60 :
            return SvGlobalName( SO3_SDRAW_CLASSID_60 );
        case SOT_FORMATSTR_ID_STARIMPRESS_60 :
            return SvGlobalName( SO3_SIMPRESS_CLASSID_60 );
        case SOT_FORMATSTR_ID_STARCALC_60 :
            return SvGlobalName( SO3_SC_CLASSID_60 );
        case SOT_FORMATSTR_ID_STARCHART_60 :
            return SvGlobalName( SO3_SCH_CLASSID_60 );
        case SOT_FORMATSTR_ID_STARMATH_60 :
            return SvGlobalName( SO3_SM_CLASSID_60 );
        default :
            return SvGlobalName();
    }
}

// All storage and streams are refcounted internally; outside of this classes they are only accessible through a handle
// class, that uses the refcounted object as impl-class.

enum RepresentModes {
        nonset,
        svstream,
        xinputstream
};

class UCBStorageStream_Impl : public SvRefBase, public SvStream
{
                                ~UCBStorageStream_Impl();
public:

    virtual sal_uLong               GetData( void* pData, sal_uLong nSize );
    virtual sal_uLong               PutData( const void* pData, sal_uLong nSize );
    virtual sal_uLong               SeekPos( sal_uLong nPos );
    virtual void                SetSize( sal_uLong nSize );
    virtual void                FlushData();
    virtual void                ResetError();

    UCBStorageStream*           m_pAntiImpl;    // only valid if an external reference exists

    String                      m_aOriginalName;// the original name before accessing the stream
    String                      m_aName;        // the actual name ( changed with a Rename command at the parent )
    String                      m_aURL;         // the full path name to create the content
    String                      m_aContentType;
    String                      m_aOriginalContentType;
    rtl::OString                m_aKey;
    ::ucbhelper::Content*       m_pContent;     // the content that provides the data
    Reference<XInputStream>     m_rSource;      // the stream covering the original data of the content
    SvStream*                   m_pStream;      // the stream worked on; for readonly streams it is the original stream of the content
                                                // for read/write streams it's a copy into a temporary file
    String                      m_aTempURL;     // URL of this temporary stream
    RepresentModes              m_nRepresentMode; // should it be used as XInputStream or as SvStream
    long                        m_nError;
    StreamMode                  m_nMode;        // open mode ( read/write/trunc/nocreate/sharing )
    sal_Bool                        m_bSourceRead;  // Source still contains useful information
    sal_Bool                        m_bModified;    // only modified streams will be sent to the original content
    sal_Bool                        m_bCommited;    // sending the streams is coordinated by the root storage of the package
    sal_Bool                        m_bDirect;      // the storage and its streams are opened in direct mode; for UCBStorages
                                                // this means that the root storage does an autocommit when its external
                                                // reference is destroyed
    sal_Bool                        m_bIsOLEStorage;// an OLEStorage on a UCBStorageStream makes this an Autocommit-stream

                                UCBStorageStream_Impl( const String&, StreamMode, UCBStorageStream*, sal_Bool, const rtl::OString* pKey=0, sal_Bool bRepair = sal_False, Reference< XProgressHandler > xProgress = Reference< XProgressHandler >() );

    void                        Free();
    sal_Bool                        Init();
    sal_Bool                        Clear();
    sal_Int16                   Commit();       // if modified and commited: transfer an XInputStream to the content
    sal_Bool                        Revert();       // discard all changes
    BaseStorage*                CreateStorage();// create an OLE Storage on the UCBStorageStream
    sal_uLong                       GetSize();

    sal_uLong                       ReadSourceWriteTemporary( sal_uLong aLength ); // read aLength from source and copy to temporary,
                                                                           // no seeking is produced
    sal_uLong                       ReadSourceWriteTemporary();                // read source till the end and copy to temporary,

    sal_uLong                       CopySourceToTemporary();                // same as ReadSourceWriteToTemporary()
                                                                        // but the writing is done at the end of temporary
                                                                        // pointer position is not changed
    Reference<XInputStream>     GetXInputStream();                      // return XInputStream, after that
                                                                        // this class is close to be unusable
                                                                        // since it can not read and write
    using SvStream::SetError;
    void                        SetError( sal_uInt32 nError );
    void                        PrepareCachedForReopen( StreamMode nMode );
};

SV_DECL_IMPL_REF( UCBStorageStream_Impl );

struct UCBStorageElement_Impl;
typedef ::std::vector< UCBStorageElement_Impl* > UCBStorageElementList_Impl;

class UCBStorage_Impl : public SvRefBase
{
                                ~UCBStorage_Impl();
public:
    UCBStorage*                 m_pAntiImpl;    // only valid if external references exists

    String                      m_aOriginalName;// the original name before accessing the storage
    String                      m_aName;        // the actual name ( changed with a Rename command at the parent )
    String                      m_aURL;         // the full path name to create the content
    String                      m_aContentType;
    String                      m_aOriginalContentType;
    ::ucbhelper::Content*       m_pContent;     // the content that provides the storage elements
    ::utl::TempFile*            m_pTempFile;    // temporary file, only for storages on stream
    SvStream*                   m_pSource;      // original stream, only for storages on a stream
    long                        m_nError;
    StreamMode                  m_nMode;        // open mode ( read/write/trunc/nocreate/sharing )
    sal_Bool                        m_bModified;    // only modified elements will be sent to the original content
    sal_Bool                        m_bCommited;    // sending the streams is coordinated by the root storage of the package
    sal_Bool                        m_bDirect;      // the storage and its streams are opened in direct mode; for UCBStorages
                                                // this means that the root storage does an autocommit when its external
                                                // reference is destroyed
    sal_Bool                        m_bIsRoot;      // marks this storage as root storages that manages all oommits and reverts
    sal_Bool                        m_bDirty;           // ???
    sal_Bool                        m_bIsLinked;
    sal_Bool                        m_bListCreated;
    sal_uLong                       m_nFormat;
    String                      m_aUserTypeName;
    SvGlobalName                m_aClassId;

    UCBStorageElementList_Impl  m_aChildrenList;

    sal_Bool                        m_bRepairPackage;
    Reference< XProgressHandler > m_xProgressHandler;

                                UCBStorage_Impl( const ::ucbhelper::Content&, const String&, StreamMode, UCBStorage*, sal_Bool, sal_Bool, sal_Bool = sal_False, Reference< XProgressHandler > = Reference< XProgressHandler >() );
                                UCBStorage_Impl( const String&, StreamMode, UCBStorage*, sal_Bool, sal_Bool, sal_Bool = sal_False, Reference< XProgressHandler > = Reference< XProgressHandler >() );
                                UCBStorage_Impl( SvStream&, UCBStorage*, sal_Bool );
    void                        Init();
    sal_Int16                   Commit();
    sal_Bool                        Revert();
    sal_Bool                        Insert( ::ucbhelper::Content *pContent );
    UCBStorage_Impl*            OpenStorage( UCBStorageElement_Impl* pElement, StreamMode nMode, sal_Bool bDirect );
    UCBStorageStream_Impl*      OpenStream( UCBStorageElement_Impl*, StreamMode, sal_Bool, const rtl::OString* pKey=0 );
    void                        SetProps( const Sequence < Sequence < PropertyValue > >& rSequence, const String& );
    void                        GetProps( sal_Int32&, Sequence < Sequence < PropertyValue > >& rSequence, const String& );
    sal_Int32                   GetObjectCount();
    void                        ReadContent();
    void                        CreateContent();
    ::ucbhelper::Content*       GetContent()
                                { if ( !m_pContent ) CreateContent(); return m_pContent; }
    UCBStorageElementList_Impl& GetChildrenList()
                                {
                                  long nError = m_nError;
                                  ReadContent();
                                  if ( m_nMode & STREAM_WRITE )
                                  {
                                    m_nError = nError;
                                    if ( m_pAntiImpl )
                                    {
                                        m_pAntiImpl->ResetError();
                                        m_pAntiImpl->SetError( nError );
                                    }
                                  }

                                  return m_aChildrenList;
                                }

    void                        SetError( long nError );
};

SV_DECL_IMPL_REF( UCBStorage_Impl );

// this struct contains all neccessary information on an element inside a UCBStorage
struct UCBStorageElement_Impl
{
    String                      m_aName;        // the actual URL relative to the root "folder"
    String                      m_aOriginalName;// the original name in the content
    sal_uLong                       m_nSize;
    sal_Bool                        m_bIsFolder;    // Only sal_True when it is a UCBStorage !
    sal_Bool                        m_bIsStorage;   // Also sal_True when it is an OLEStorage !
    sal_Bool                        m_bIsRemoved;   // element will be removed on commit
    sal_Bool                        m_bIsInserted;  // element will be removed on revert
    UCBStorage_ImplRef          m_xStorage;     // reference to the "real" storage
    UCBStorageStream_ImplRef    m_xStream;      // reference to the "real" stream

                                UCBStorageElement_Impl( const ::rtl::OUString& rName,
                                            sal_Bool bIsFolder = sal_False, sal_uLong nSize = 0 )
                                    : m_aName( rName )
                                    , m_aOriginalName( rName )
                                    , m_nSize( nSize )
                                    , m_bIsFolder( bIsFolder )
                                    , m_bIsStorage( bIsFolder )
                                    , m_bIsRemoved( sal_False )
                                    , m_bIsInserted( sal_False )
                                {
                                }

    ::ucbhelper::Content*       GetContent();
    sal_Bool                        IsModified();
    String                      GetContentType();
    void                        SetContentType( const String& );
    String                      GetOriginalContentType();
    sal_Bool                        IsLoaded()
                                { return m_xStream.Is() || m_xStorage.Is(); }
};

::ucbhelper::Content* UCBStorageElement_Impl::GetContent()
{
    if ( m_xStream.Is() )
        return m_xStream->m_pContent;
    else if ( m_xStorage.Is() )
        return m_xStorage->GetContent();
    else
        return NULL;
}

String UCBStorageElement_Impl::GetContentType()
{
    if ( m_xStream.Is() )
        return m_xStream->m_aContentType;
    else if ( m_xStorage.Is() )
        return m_xStorage->m_aContentType;
    else
    {
        OSL_FAIL("Element not loaded!");
        return String();
    }
}

void UCBStorageElement_Impl::SetContentType( const String& rType )
{
    if ( m_xStream.Is() ) {
        m_xStream->m_aContentType = m_xStream->m_aOriginalContentType = rType;
    }
    else if ( m_xStorage.Is() ) {
        m_xStorage->m_aContentType = m_xStorage->m_aOriginalContentType = rType;
    }
    else {
        OSL_FAIL("Element not loaded!");
    }
}

String UCBStorageElement_Impl::GetOriginalContentType()
{
    if ( m_xStream.Is() )
        return m_xStream->m_aOriginalContentType;
    else if ( m_xStorage.Is() )
        return m_xStorage->m_aOriginalContentType;
    else
        return String();
}

sal_Bool UCBStorageElement_Impl::IsModified()
{
    sal_Bool bModified = m_bIsRemoved || m_bIsInserted || m_aName != m_aOriginalName;
    if ( bModified )
    {
        if ( m_xStream.Is() )
            bModified = m_xStream->m_aContentType != m_xStream->m_aOriginalContentType;
        else if ( m_xStorage.Is() )
            bModified = m_xStorage->m_aContentType != m_xStorage->m_aOriginalContentType;
    }

    return bModified;
}

UCBStorageStream_Impl::UCBStorageStream_Impl( const String& rName, StreamMode nMode, UCBStorageStream* pStream, sal_Bool bDirect, const rtl::OString* pKey, sal_Bool bRepair, Reference< XProgressHandler > xProgress  )
    : m_pAntiImpl( pStream )
    , m_aURL( rName )
    , m_pContent( NULL )
    , m_pStream( NULL )
    , m_nRepresentMode( nonset )
    , m_nError( 0 )
    , m_nMode( nMode )
    , m_bSourceRead( !( nMode & STREAM_TRUNC ) )
    , m_bModified( sal_False )
    , m_bCommited( sal_False )
    , m_bDirect( bDirect )
    , m_bIsOLEStorage( sal_False )
{
    // name is last segment in URL
    INetURLObject aObj( rName );
    m_aName = m_aOriginalName = aObj.GetLastName();
    try
    {
        // create the content
        Reference< ::com::sun::star::ucb::XCommandEnvironment > xComEnv;

        ::rtl::OUString aTemp( rName );

        if ( bRepair )
        {
            xComEnv = new ::ucbhelper::CommandEnvironment( Reference< ::com::sun::star::task::XInteractionHandler >(),
                                                     xProgress );
            aTemp += rtl::OUString("?repairpackage");
        }

        m_pContent = new ::ucbhelper::Content( aTemp, xComEnv );

        if ( pKey )
        {
            m_aKey = *pKey;

            // stream is encrypted and should be decrypted (without setting the key we'll get the raw data)
            sal_uInt8 aBuffer[RTL_DIGEST_LENGTH_SHA1];
            rtlDigestError nErr = rtl_digest_SHA1( pKey->getStr(), pKey->getLength(), aBuffer, RTL_DIGEST_LENGTH_SHA1 );
            if ( nErr == rtl_Digest_E_None )
            {
                sal_uInt8* pBuffer = aBuffer;
                ::com::sun::star::uno::Sequence < sal_Int8 > aSequ( (sal_Int8*) pBuffer, RTL_DIGEST_LENGTH_SHA1 );
                ::com::sun::star::uno::Any aAny;
                aAny <<= aSequ;
                m_pContent->setPropertyValue( ::rtl::OUString("EncryptionKey"), aAny );
            }
        }
    }
    catch (const ContentCreationException&)
    {
        // content could not be created
        SetError( SVSTREAM_CANNOT_MAKE );
    }
    catch (const RuntimeException&)
    {
        // any other error - not specified
        SetError( ERRCODE_IO_GENERAL );
    }
}

UCBStorageStream_Impl::~UCBStorageStream_Impl()
{
    if( m_rSource.is() )
        m_rSource = Reference< XInputStream >();

    if( m_pStream )
        delete m_pStream;

    if ( m_aTempURL.Len() )
        ::utl::UCBContentHelper::Kill( m_aTempURL );

    if( m_pContent )
        delete m_pContent;
}


Reference<XInputStream> UCBStorageStream_Impl::GetXInputStream()
{
    Reference< XInputStream > aResult;

    if( m_pAntiImpl && m_nRepresentMode != nonset )
    {
        OSL_FAIL( "Misuse of the XInputstream!" );
        SetError( ERRCODE_IO_ACCESSDENIED );
    }
    else
    {
        if( m_bModified )
        {
            // use wrapper around temporary stream
            if( Init() )
            {
                CopySourceToTemporary();

                // owner transfer of stream to wrapper
                aResult = new ::utl::OInputStreamWrapper( m_pStream, sal_True );
                m_pStream->Seek(0);

                if( aResult.is() )
                {
                    // temporary stream can not be used here any more
                    // and can not be opened untill wrapper is closed
                    // stream is deleted by wrapper after use
                    m_pStream = NULL;
                    m_nRepresentMode = xinputstream;
                }
            }
        }
        else
        {
            Free();

            // open a new instance of XInputStream
            try
            {
                aResult = m_pContent->openStream();
            }
            catch (const Exception&)
            {
                // usually means that stream could not be opened
            }

            if( aResult.is() )
                m_nRepresentMode = xinputstream;
            else
                SetError( ERRCODE_IO_ACCESSDENIED );
        }
    }

    return aResult;
}

sal_Bool UCBStorageStream_Impl::Init()
{
    if( m_nRepresentMode == xinputstream )
    {
        OSL_FAIL( "XInputStream misuse!" );
        SetError( ERRCODE_IO_ACCESSDENIED );
        return sal_False;
    }

    if( !m_pStream )
    {
        // no temporary stream was created
        // create one

        m_nRepresentMode = svstream; // can not be used as XInputStream

        if ( !m_aTempURL.Len() )
            m_aTempURL = ::utl::TempFile().GetURL();

        m_pStream = ::utl::UcbStreamHelper::CreateStream( m_aTempURL, STREAM_STD_READWRITE, sal_True /* bFileExists */ );
#if OSL_DEBUG_LEVEL > 1
        ++nOpenFiles;
#endif

        if( !m_pStream )
        {
            OSL_FAIL( "Suspicious temporary stream creation!" );
            SetError( SVSTREAM_CANNOT_MAKE );
            return sal_False;
        }

        SetError( m_pStream->GetError() );
    }

    if( m_bSourceRead && !m_rSource.is() )
    {
        // source file contain usefull information and is not opened
        // open it from the point of noncopied data

        try
        {
            m_rSource = m_pContent->openStream();
        }
        catch (const Exception&)
        {
            // usually means that stream could not be opened
        }

        if( m_rSource.is() )
        {
            m_pStream->Seek( STREAM_SEEK_TO_END );

            try
            {
                m_rSource->skipBytes( m_pStream->Tell() );
            }
            catch (const BufferSizeExceededException&)
            {
                // the temporary stream already contain all the data
                m_bSourceRead = sal_False;
            }
            catch (const Exception&)
            {
                // something is really wrong
                m_bSourceRead = sal_False;
                OSL_FAIL( "Can not operate original stream!" );
                SetError( SVSTREAM_CANNOT_MAKE );
            }

            m_pStream->Seek( 0 );
        }
        else
        {
            // if the new file is edited than no source exist
            m_bSourceRead = sal_False;
                //SetError( SVSTREAM_CANNOT_MAKE );
        }
    }

    DBG_ASSERT( m_rSource.is() || !m_bSourceRead, "Unreadable source stream!" );

    return sal_True;
}

sal_uLong UCBStorageStream_Impl::ReadSourceWriteTemporary()
{
    // read source stream till the end and copy all the data to
    // the current position of the temporary stream

    sal_uLong aResult = 0;

    if( m_bSourceRead )
    {
        Sequence<sal_Int8> aData(32000);

        try
        {
            sal_uLong aReaded;
            do
            {
                aReaded = m_rSource->readBytes( aData, 32000 );
                aResult += m_pStream->Write( aData.getArray(), aReaded );
            } while( aReaded == 32000 );
        }
        catch (const Exception &e)
        {
            OSL_FAIL( ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
            (void)e;
        }
    }

    m_bSourceRead = sal_False;

    return aResult;

}

sal_uLong UCBStorageStream_Impl::ReadSourceWriteTemporary( sal_uLong aLength )
{
    // read aLength bite from the source stream and copy them to the current
    // position of the temporary stream

    sal_uLong aResult = 0;

    if( m_bSourceRead )
    {
        Sequence<sal_Int8> aData(32000);

        try
        {

            sal_uLong aReaded = 32000;

            for( sal_uLong pInd = 0; pInd < aLength && aReaded == 32000 ; pInd += 32000 )
            {
                sal_uLong aToCopy = min( aLength - pInd, 32000 );
                aReaded = m_rSource->readBytes( aData, aToCopy );
                aResult += m_pStream->Write( aData.getArray(), aReaded );
            }

            if( aResult < aLength )
                m_bSourceRead = sal_False;
        }
        catch( const Exception & e )
        {
            OSL_FAIL( ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
            (void)e;
        }
    }

    return aResult;
}

sal_uLong UCBStorageStream_Impl::CopySourceToTemporary()
{
    // current position of the temporary stream is not changed
    sal_uLong aResult = 0;

    if( m_bSourceRead )
    {
        sal_uLong aPos = m_pStream->Tell();
        m_pStream->Seek( STREAM_SEEK_TO_END );
        aResult = ReadSourceWriteTemporary();
        m_pStream->Seek( aPos );
    }

    return aResult;

}

// UCBStorageStream_Impl must have a SvStream interface, because it then can be used as underlying stream
// of an OLEStorage; so every write access caused by storage operations marks the UCBStorageStream as modified
sal_uLong UCBStorageStream_Impl::GetData( void* pData, sal_uLong nSize )
{
    sal_uLong aResult = 0;

    if( !Init() )
        return 0;


    // read data that is in temporary stream
    aResult = m_pStream->Read( pData, nSize );
    if( m_bSourceRead && aResult < nSize )
    {
        // read the tail of the data from original stream
        // copy this tail to the temporary stream

        sal_uLong aToRead = nSize - aResult;
        pData = (void*)( (char*)pData + aResult );

        try
        {
            Sequence<sal_Int8> aData( aToRead );
            sal_uLong aReaded = m_rSource->readBytes( aData, aToRead );
            aResult += m_pStream->Write( (void*)aData.getArray(), aReaded );
            memcpy( pData, aData.getArray(), aReaded );
        }
        catch (const Exception &e)
        {
            OSL_FAIL( ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
            (void)e;
        }

        if( aResult < nSize )
            m_bSourceRead = sal_False;
    }

    return aResult;
}

sal_uLong UCBStorageStream_Impl::PutData( const void* pData, sal_uLong nSize )
{
    if ( !(m_nMode & STREAM_WRITE) )
    {
        SetError( ERRCODE_IO_ACCESSDENIED );
        return 0; // ?mav?
    }

    if( !nSize || !Init() )
        return 0;

    sal_uLong aResult = m_pStream->Write( pData, nSize );

    m_bModified = aResult > 0;

    return aResult;

}

sal_uLong UCBStorageStream_Impl::SeekPos( sal_uLong nPos )
{
    if( !Init() )
        return 0;

    sal_uLong aResult;

    if( nPos == STREAM_SEEK_TO_END )
    {
        m_pStream->Seek( STREAM_SEEK_TO_END );
        ReadSourceWriteTemporary();
        aResult = m_pStream->Tell();
    }
    else
    {
        // the problem is that even if nPos is larger the the length
        // of the stream the stream pointer will be moved to this position
        // so we have to check if temporary stream does not contain required position

        if( m_pStream->Tell() > nPos
            || m_pStream->Seek( STREAM_SEEK_TO_END ) > nPos )
        {
            // no copiing is required
            aResult = m_pStream->Seek( nPos );
        }
        else
        {
            // the temp stream pointer points to the end now
            aResult = m_pStream->Tell();

            if( aResult < nPos )
            {
                if( m_bSourceRead )
                {
                    aResult += ReadSourceWriteTemporary( nPos - aResult );
                    if( aResult < nPos )
                        m_bSourceRead = sal_False;

                    DBG_ASSERT( aResult == m_pStream->Tell(), "Error in stream arithmetic!\n" );
                }

                if( (m_nMode & STREAM_WRITE) && !m_bSourceRead && aResult < nPos )
                {
                    // it means that all the Source stream was copied already
                    // but the required position still was not reached
                    // for writable streams it should be done
                    m_pStream->SetStreamSize( nPos );
                    aResult = m_pStream->Seek( STREAM_SEEK_TO_END );
                    DBG_ASSERT( aResult == nPos, "Error in stream arithmetic!\n" );
                }
            }
        }
    }

    return aResult;
}

void  UCBStorageStream_Impl::SetSize( sal_uLong nSize )
{
    if ( !(m_nMode & STREAM_WRITE) )
    {
        SetError( ERRCODE_IO_ACCESSDENIED );
        return;
    }

    if( !Init() )
        return;

    m_bModified = sal_True;

    if( m_bSourceRead )
    {
        sal_uLong aPos = m_pStream->Tell();
        m_pStream->Seek( STREAM_SEEK_TO_END );
        if( m_pStream->Tell() < nSize )
            ReadSourceWriteTemporary( nSize - m_pStream->Tell() );
        m_pStream->Seek( aPos );
    }

    m_pStream->SetStreamSize( nSize );
    m_bSourceRead = sal_False;
}

void  UCBStorageStream_Impl::FlushData()
{
    if( m_pStream )
    {
        CopySourceToTemporary();
        m_pStream->Flush();
    }

    m_bCommited = sal_True;
}

void UCBStorageStream_Impl::SetError( sal_uInt32 nErr )
{
    if ( !m_nError )
    {
        m_nError = nErr;
        SvStream::SetError( nErr );
        if ( m_pAntiImpl ) m_pAntiImpl->SetError( nErr );
    }
}

void  UCBStorageStream_Impl::ResetError()
{
    m_nError = 0;
    SvStream::ResetError();
    if ( m_pAntiImpl )
        m_pAntiImpl->ResetError();
}

sal_uLong UCBStorageStream_Impl::GetSize()
{
    if( !Init() )
        return 0;

    sal_uLong nPos = m_pStream->Tell();
    m_pStream->Seek( STREAM_SEEK_TO_END );
    ReadSourceWriteTemporary();
    sal_uLong nRet = m_pStream->Tell();
    m_pStream->Seek( nPos );

    return nRet;
}

BaseStorage* UCBStorageStream_Impl::CreateStorage()
{
    // create an OLEStorage on a SvStream ( = this )
    // it gets the root attribute because otherwise it would probably not write before my root is commited
    UCBStorageStream* pNewStorageStream = new UCBStorageStream( this );
    Storage *pStorage = new Storage( *pNewStorageStream, m_bDirect );

    // GetError() call cleares error code for OLE storages, must be changed in future
    long nTmpErr = pStorage->GetError();
    pStorage->SetError( nTmpErr );

    m_bIsOLEStorage = !nTmpErr;
    return static_cast< BaseStorage* > ( pStorage );
}

sal_Int16 UCBStorageStream_Impl::Commit()
{
    // send stream to the original content
    // the  parent storage is responsible for the correct handling of deleted contents
    if ( m_bCommited || m_bIsOLEStorage || m_bDirect )
    {
        // modified streams with OLEStorages on it have autocommit; it is assumed that the OLEStorage
        // was commited as well ( if not opened in direct mode )

        if ( m_bModified )
        {
            try
            {
                CopySourceToTemporary();

                // release all stream handles
                Free();

                // the temporary file does not exist only for truncated streams
                DBG_ASSERT( m_aTempURL.Len() || ( m_nMode & STREAM_TRUNC ), "No temporary file to read from!");
                if ( !m_aTempURL.Len() && !( m_nMode & STREAM_TRUNC ) )
                    throw RuntimeException();

                // create wrapper to stream that is only used while reading inside package component
                Reference < XInputStream > xStream = new FileStreamWrapper_Impl( m_aTempURL );

                Any aAny;
                InsertCommandArgument aArg;
                aArg.Data = xStream;
                aArg.ReplaceExisting = sal_True;
                aAny <<= aArg;
                m_pContent->executeCommand( ::rtl::OUString("insert"), aAny );

                // wrapper now controls lifetime of temporary file
                m_aTempURL.Erase();

                INetURLObject aObj( m_aURL );
                aObj.SetName( m_aName );
                m_aURL = aObj.GetMainURL( INetURLObject::NO_DECODE );
                m_bModified = sal_False;
                m_bSourceRead = sal_True;
            }
            catch (const CommandAbortedException&)
            {
                // any command wasn't executed successfully - not specified
                SetError( ERRCODE_IO_GENERAL );
                return COMMIT_RESULT_FAILURE;
            }
            catch (const RuntimeException&)
            {
                // any other error - not specified
                SetError( ERRCODE_IO_GENERAL );
                return COMMIT_RESULT_FAILURE;
            }
            catch (const Exception&)
            {
                // any other error - not specified
                SetError( ERRCODE_IO_GENERAL );
                return COMMIT_RESULT_FAILURE;
            }

            m_bCommited = sal_False;
            return COMMIT_RESULT_SUCCESS;
        }
    }

    return COMMIT_RESULT_NOTHING_TO_DO;
}

sal_Bool UCBStorageStream_Impl::Revert()
{
    // if an OLEStorage is created on this stream, no "revert" is neccessary because OLEStorages do nothing on "Revert" !
    if ( m_bCommited )
    {
        OSL_FAIL("Revert while commit is in progress!" );
        return sal_False;                   //  ???
    }

    Free();
    if ( m_aTempURL.Len() )
    {
        ::utl::UCBContentHelper::Kill( m_aTempURL );
        m_aTempURL.Erase();
    }

    m_bSourceRead = sal_False;
    try
    {
        m_rSource = m_pContent->openStream();
        if( m_rSource.is() )
        {
            if ( m_pAntiImpl && ( m_nMode & STREAM_TRUNC ) )
                // stream is in use and should be truncated
                m_bSourceRead = sal_False;
            else
            {
                m_nMode &= ~STREAM_TRUNC;
                m_bSourceRead = sal_True;
            }
        }
        else
            SetError( SVSTREAM_CANNOT_MAKE );
    }
    catch (const ContentCreationException&)
    {
        SetError( ERRCODE_IO_GENERAL );
    }
    catch (const RuntimeException&)
    {
        SetError( ERRCODE_IO_GENERAL );
    }
    catch (const Exception&)
    {
    }

    m_bModified = sal_False;
    m_aName = m_aOriginalName;
    m_aContentType = m_aOriginalContentType;
    return ( GetError() == ERRCODE_NONE );
}

sal_Bool UCBStorageStream_Impl::Clear()
{
    sal_Bool bRet = ( m_pAntiImpl == NULL );
    DBG_ASSERT( bRet, "Removing used stream!" );
    if( bRet )
    {
        Free();
    }

    return bRet;
}

void UCBStorageStream_Impl::Free()
{
#if OSL_DEBUG_LEVEL > 1
    if ( m_pStream )
    {
        if ( m_aTempURL.Len() )
            --nOpenFiles;
        else
            --nOpenStreams;
    }
#endif

    m_nRepresentMode = nonset;
    m_rSource = Reference< XInputStream >();
    DELETEZ( m_pStream );
}

void UCBStorageStream_Impl::PrepareCachedForReopen( StreamMode nMode )
{
    sal_Bool isWritable = (( m_nMode & STREAM_WRITE ) != 0 );
    if ( isWritable )
    {
        // once stream was writable, never reset to readonly
        nMode |= STREAM_WRITE;
    }

    m_nMode = nMode;
    Free();

    if ( nMode & STREAM_TRUNC )
    {
        m_bSourceRead = 0; // usually it should be 0 already but just in case...

        if ( m_aTempURL.Len() )
        {
            ::utl::UCBContentHelper::Kill( m_aTempURL );
            m_aTempURL.Erase();
        }
    }
}

UCBStorageStream::UCBStorageStream( const String& rName, StreamMode nMode, sal_Bool bDirect, const rtl::OString* pKey, sal_Bool bRepair, Reference< XProgressHandler > xProgress )
{
    // pImp must be initialized in the body, because otherwise the vtable of the stream is not initialized
    // to class UCBStorageStream !
    pImp = new UCBStorageStream_Impl( rName, nMode, this, bDirect, pKey, bRepair, xProgress );
    pImp->AddRef();             // use direct refcounting because in header file only a pointer should be used
    StorageBase::m_nMode = pImp->m_nMode;
}

UCBStorageStream::UCBStorageStream( UCBStorageStream_Impl *pImpl )
    : pImp( pImpl )
{
    pImp->AddRef();             // use direct refcounting because in header file only a pointer should be used
    pImp->m_pAntiImpl = this;
    SetError( pImp->m_nError );
    StorageBase::m_nMode = pImp->m_nMode;
}

UCBStorageStream::~UCBStorageStream()
{
    if ( pImp->m_nMode & STREAM_WRITE )
        pImp->Flush();
    pImp->m_pAntiImpl = NULL;
    pImp->Free();
    pImp->ReleaseRef();
}

sal_uLong UCBStorageStream::Read( void * pData, sal_uLong nSize )
{
    //return pImp->m_pStream->Read( pData, nSize );
    return pImp->GetData( pData, nSize );
}

sal_uLong UCBStorageStream::Write( const void* pData, sal_uLong nSize )
{
    return pImp->PutData( pData, nSize );
}

sal_uLong UCBStorageStream::Seek( sal_uLong nPos )
{
    //return pImp->m_pStream->Seek( nPos );
    return pImp->Seek( nPos );
}

sal_uLong UCBStorageStream::Tell()
{
    if( !pImp->Init() )
        return 0;
    return pImp->m_pStream->Tell();
}

void UCBStorageStream::Flush()
{
    // streams are never really transacted, so flush also means commit !
    Commit();
}

sal_Bool UCBStorageStream::SetSize( sal_uLong nNewSize )
{
    pImp->SetSize( nNewSize );
    return !pImp->GetError();
}

sal_Bool UCBStorageStream::Validate( sal_Bool bWrite ) const
{
    return ( !bWrite || ( pImp->m_nMode & STREAM_WRITE ) );
}

sal_Bool UCBStorageStream::ValidateMode( StreamMode m ) const
{
    // ???
    if( m == ( STREAM_READ | STREAM_TRUNC ) )  // from stg.cxx
        return sal_True;
    sal_uInt16 nCurMode = 0xFFFF;
    if( ( m & 3 ) == STREAM_READ )
    {
        // only SHARE_DENYWRITE or SHARE_DENYALL allowed
        if( ( ( m & STREAM_SHARE_DENYWRITE )
           && ( nCurMode & STREAM_SHARE_DENYWRITE ) )
         || ( ( m & STREAM_SHARE_DENYALL )
           && ( nCurMode & STREAM_SHARE_DENYALL ) ) )
            return sal_True;
    }
    else
    {
        // only SHARE_DENYALL allowed
        // storages open in r/o mode are OK, since only
        // the commit may fail
        if( ( m & STREAM_SHARE_DENYALL )
         && ( nCurMode & STREAM_SHARE_DENYALL ) )
            return sal_True;
    }

    return sal_True;
}

const SvStream* UCBStorageStream::GetSvStream() const
{
    if( !pImp->Init() )
        return NULL;

    pImp->CopySourceToTemporary();
    return pImp->m_pStream; // should not live longer then pImp!!!
}

SvStream* UCBStorageStream::GetModifySvStream()
{
    return (SvStream*)pImp;
}

Reference< XInputStream > UCBStorageStream::GetXInputStream() const
{
    return pImp->GetXInputStream();
}

sal_Bool  UCBStorageStream::Equals( const BaseStorageStream& rStream ) const
{
    // ???
    return ((BaseStorageStream*) this ) == &rStream;
}

sal_Bool UCBStorageStream::Commit()
{
    // mark this stream for sending it on root commit
    pImp->FlushData();
    return sal_True;
}

sal_Bool UCBStorageStream::Revert()
{
    return pImp->Revert();
}

sal_Bool UCBStorageStream::CopyTo( BaseStorageStream* pDestStm )
{
    if( !pImp->Init() )
        return sal_False;

    UCBStorageStream* pStg = PTR_CAST( UCBStorageStream, pDestStm );
    if ( pStg )
        pStg->pImp->m_aContentType = pImp->m_aContentType;

    pDestStm->SetSize( 0 );
    Seek( STREAM_SEEK_TO_END );
    sal_Int32 n = Tell();
    if( n < 0 )
        return sal_False;

    if( pDestStm->SetSize( n ) && n )
    {
        sal_uInt8* p = new sal_uInt8[ 4096 ];
        Seek( 0L );
        pDestStm->Seek( 0L );
        while( n )
        {
            sal_uInt32 nn = n;
            if( nn > 4096 )
                nn = 4096;
            if( Read( p, nn ) != nn )
                break;
            if( pDestStm->Write( p, nn ) != nn )
                break;
            n -= nn;
        }

        delete[] p;
    }

    return sal_True;
}

sal_Bool UCBStorageStream::SetProperty( const String& rName, const ::com::sun::star::uno::Any& rValue )
{
    if ( rName.CompareToAscii("Title") == COMPARE_EQUAL )
        return sal_False;

    if ( rName.CompareToAscii("MediaType") == COMPARE_EQUAL )
    {
        ::rtl::OUString aTmp;
        rValue >>= aTmp;
        pImp->m_aContentType = aTmp;
    }

    try
    {
        if ( pImp->m_pContent )
        {
            pImp->m_pContent->setPropertyValue( rName, rValue );
            return sal_True;
        }
    }
    catch (const Exception&)
    {
    }

    return sal_False;
}

sal_Bool UCBStorageStream::GetProperty( const String& rName, ::com::sun::star::uno::Any& rValue )
{
    try
    {
        if ( pImp->m_pContent )
        {
            rValue = pImp->m_pContent->getPropertyValue( rName );
            return sal_True;
        }
    }
    catch (const Exception&)
    {
    }

    return sal_False;
}

sal_uLong UCBStorageStream::GetSize() const
{
    return pImp->GetSize();
}

UCBStorage::UCBStorage( SvStream& rStrm, sal_Bool bDirect )
{
    String aURL = GetLinkedFile( rStrm );
    if ( aURL.Len() )
    {
        StreamMode nMode = STREAM_READ;
        if( rStrm.IsWritable() )
            nMode = STREAM_READ | STREAM_WRITE;

        ::ucbhelper::Content aContent( aURL, Reference < XCommandEnvironment >() );
        pImp = new UCBStorage_Impl( aContent, aURL, nMode, this, bDirect, sal_True );
    }
    else
    {
        // pImp must be initialized in the body, because otherwise the vtable of the stream is not initialized
        // to class UCBStorage !
        pImp = new UCBStorage_Impl( rStrm, this, bDirect );
    }

    pImp->AddRef();
    pImp->Init();
    StorageBase::m_nMode = pImp->m_nMode;
}

UCBStorage::UCBStorage( const ::ucbhelper::Content& rContent, const String& rName, StreamMode nMode, sal_Bool bDirect, sal_Bool bIsRoot )
{
    // pImp must be initialized in the body, because otherwise the vtable of the stream is not initialized
    // to class UCBStorage !
    pImp = new UCBStorage_Impl( rContent, rName, nMode, this, bDirect, bIsRoot );
    pImp->AddRef();
    pImp->Init();
    StorageBase::m_nMode = pImp->m_nMode;
}

UCBStorage::UCBStorage( const String& rName, StreamMode nMode, sal_Bool bDirect, sal_Bool bIsRoot, sal_Bool bIsRepair, Reference< XProgressHandler > xProgressHandler )
{
    // pImp must be initialized in the body, because otherwise the vtable of the stream is not initialized
    // to class UCBStorage !
    pImp = new UCBStorage_Impl( rName, nMode, this, bDirect, bIsRoot, bIsRepair, xProgressHandler );
    pImp->AddRef();
    pImp->Init();
    StorageBase::m_nMode = pImp->m_nMode;
}

UCBStorage::UCBStorage( const String& rName, StreamMode nMode, sal_Bool bDirect, sal_Bool bIsRoot )
{
    // pImp must be initialized in the body, because otherwise the vtable of the stream is not initialized
    // to class UCBStorage !
    pImp = new UCBStorage_Impl( rName, nMode, this, bDirect, bIsRoot, sal_False, Reference< XProgressHandler >() );
    pImp->AddRef();
    pImp->Init();
    StorageBase::m_nMode = pImp->m_nMode;
}

UCBStorage::UCBStorage( UCBStorage_Impl *pImpl )
    : pImp( pImpl )
{
    pImp->m_pAntiImpl = this;
    SetError( pImp->m_nError );
    pImp->AddRef();             // use direct refcounting because in header file only a pointer should be used
    StorageBase::m_nMode = pImp->m_nMode;
}

UCBStorage::~UCBStorage()
{
    if ( pImp->m_bIsRoot && pImp->m_bDirect && ( !pImp->m_pTempFile || pImp->m_pSource ) )
        // DirectMode is simulated with an AutoCommit
        Commit();

    pImp->m_pAntiImpl = NULL;
    pImp->ReleaseRef();
}

UCBStorage_Impl::UCBStorage_Impl( const ::ucbhelper::Content& rContent, const String& rName, StreamMode nMode, UCBStorage* pStorage, sal_Bool bDirect, sal_Bool bIsRoot, sal_Bool bIsRepair, Reference< XProgressHandler > xProgressHandler  )
    : m_pAntiImpl( pStorage )
    , m_pContent( new ::ucbhelper::Content( rContent ) )
    , m_pTempFile( NULL )
    , m_pSource( NULL )
    //, m_pStream( NULL )
    , m_nError( 0 )
    , m_nMode( nMode )
    , m_bModified( sal_False )
    , m_bCommited( sal_False )
    , m_bDirect( bDirect )
    , m_bIsRoot( bIsRoot )
    , m_bDirty( sal_False )
    , m_bIsLinked( sal_True )
    , m_bListCreated( sal_False )
    , m_nFormat( 0 )
    , m_aClassId( SvGlobalName() )
    , m_bRepairPackage( bIsRepair )
    , m_xProgressHandler( xProgressHandler )
{
    String aName( rName );
    if( !aName.Len() )
    {
        // no name given = use temporary name!
        DBG_ASSERT( m_bIsRoot, "SubStorage must have a name!" );
        m_pTempFile = new ::utl::TempFile;
        m_pTempFile->EnableKillingFile( sal_True );
        m_aName = m_aOriginalName = aName = m_pTempFile->GetURL();
    }

    m_aURL = rName;
}

UCBStorage_Impl::UCBStorage_Impl( const String& rName, StreamMode nMode, UCBStorage* pStorage, sal_Bool bDirect, sal_Bool bIsRoot, sal_Bool bIsRepair, Reference< XProgressHandler > xProgressHandler )
    : m_pAntiImpl( pStorage )
    , m_pContent( NULL )
    , m_pTempFile( NULL )
    , m_pSource( NULL )
    //, m_pStream( NULL )
    , m_nError( 0 )
    , m_nMode( nMode )
    , m_bModified( sal_False )
    , m_bCommited( sal_False )
    , m_bDirect( bDirect )
    , m_bIsRoot( bIsRoot )
    , m_bDirty( sal_False )
    , m_bIsLinked( sal_False )
    , m_bListCreated( sal_False )
    , m_nFormat( 0 )
    , m_aClassId( SvGlobalName() )
    , m_bRepairPackage( bIsRepair )
    , m_xProgressHandler( xProgressHandler )
{
    String aName( rName );
    if( !aName.Len() )
    {
        // no name given = use temporary name!
        DBG_ASSERT( m_bIsRoot, "SubStorage must have a name!" );
        m_pTempFile = new ::utl::TempFile;
        m_pTempFile->EnableKillingFile( sal_True );
        m_aName = m_aOriginalName = aName = m_pTempFile->GetURL();
    }

    if ( m_bIsRoot )
    {
        // create the special package URL for the package content
        String aTemp = rtl::OUString("vnd.sun.star.pkg://");
        aTemp += String(INetURLObject::encode( aName, INetURLObject::PART_AUTHORITY, '%', INetURLObject::ENCODE_ALL ));
        m_aURL = aTemp;

        if ( m_nMode & STREAM_WRITE )
        {
            // the root storage opens the package, so make sure that there is any
            SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( aName, STREAM_STD_READWRITE, m_pTempFile != 0 /* bFileExists */ );
            delete pStream;
        }
    }
    else
    {
        // substorages are opened like streams: the URL is a "child URL" of the root package URL
        m_aURL = rName;
        if ( m_aURL.CompareToAscii( "vnd.sun.star.pkg://", 19 ) != 0 )
            m_bIsLinked = sal_True;
    }
}

UCBStorage_Impl::UCBStorage_Impl( SvStream& rStream, UCBStorage* pStorage, sal_Bool bDirect )
    : m_pAntiImpl( pStorage )
    , m_pContent( NULL )
    , m_pTempFile( new ::utl::TempFile )
    , m_pSource( &rStream )
    , m_nError( 0 )
    , m_bModified( sal_False )
    , m_bCommited( sal_False )
    , m_bDirect( bDirect )
    , m_bIsRoot( sal_True )
    , m_bDirty( sal_False )
    , m_bIsLinked( sal_False )
    , m_bListCreated( sal_False )
    , m_nFormat( 0 )
    , m_aClassId( SvGlobalName() )
    , m_bRepairPackage( sal_False )
{
    // opening in direct mode is too fuzzy because the data is transferred to the stream in the Commit() call,
    // which will be called in the storages' dtor
    m_pTempFile->EnableKillingFile( sal_True );
    DBG_ASSERT( !bDirect, "Storage on a stream must not be opened in direct mode!" );

    // UCBStorages work on a content, so a temporary file for a content must be created, even if the stream is only
    // accessed readonly
    // the root storage opens the package; create the special package URL for the package content
    String aTemp = rtl::OUString("vnd.sun.star.pkg://");
    aTemp += String(INetURLObject::encode( m_pTempFile->GetURL(), INetURLObject::PART_AUTHORITY, '%', INetURLObject::ENCODE_ALL ));
    m_aURL = aTemp;

    // copy data into the temporary file
    SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( m_pTempFile->GetURL(), STREAM_STD_READWRITE, sal_True /* bFileExists */ );
    if ( pStream )
    {
        rStream.Seek(0);
        rStream >> *pStream;
        pStream->Flush();
        DELETEZ( pStream );
    }

    // close stream and let content access the file
    m_pSource->Seek(0);

    // check opening mode
    m_nMode = STREAM_READ;
    if( rStream.IsWritable() )
        m_nMode = STREAM_READ | STREAM_WRITE;
}

void UCBStorage_Impl::Init()
{
    // name is last segment in URL
    INetURLObject aObj( m_aURL );
    if ( !m_aName.Len() )
        // if the name was not already set to a temp name
        m_aName = m_aOriginalName = aObj.GetLastName();

    // don't create the content for disk spanned files, avoid too early access to directory and/or manifest
    if ( !m_pContent && !( m_nMode & STORAGE_DISKSPANNED_MODE ) )
        CreateContent();

    if ( m_nMode & STORAGE_DISKSPANNED_MODE )
    {
        // Hack! Avoid access to the manifest file until mediatype is not available in the first segment of a
        // disk spanned file
        m_aContentType = m_aOriginalContentType = ::rtl::OUString( "application/vnd.sun.xml.impress" );
    }
    else if ( m_pContent )
    {
        if ( m_bIsLinked )
        {
            if( m_bIsRoot )
            {
                ReadContent();
                if ( m_nError == ERRCODE_NONE )
                {
                    // read the manifest.xml file
                    aObj.Append( String( "META-INF" ) );
                    aObj.Append( String( "manifest.xml" ) );

                    // create input stream
                    SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( aObj.GetMainURL( INetURLObject::NO_DECODE ), STREAM_STD_READ );
                    // no stream means no manifest.xml
                    if ( pStream )
                    {
                        if ( !pStream->GetError() )
                        {
                            ::utl::OInputStreamWrapper* pHelper = new ::utl::OInputStreamWrapper( *pStream );
                            com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream > xInputStream( pHelper );

                            // create a manifest reader object that will read in the manifest from the stream
                            Reference < ::com::sun::star::packages::manifest::XManifestReader > xReader =
                                Reference< ::com::sun::star::packages::manifest::XManifestReader >
                                    ( ::comphelper::getProcessServiceFactory()->createInstance(
                                        ::rtl::OUString("com.sun.star.packages.manifest.ManifestReader")), UNO_QUERY) ;
                            Sequence < Sequence < PropertyValue > > aProps = xReader->readManifestSequence( xInputStream );

                            // cleanup
                            xReader = NULL;
                            xInputStream = NULL;
                            SetProps( aProps, String() );
                        }

                        delete pStream;
                    }
                }
            }
            else
                ReadContent();
        }
        else
        {
            // get the manifest information from the package
            try {
                Any aAny = m_pContent->getPropertyValue( ::rtl::OUString("MediaType") );
                rtl::OUString aTmp;
                if ( ( aAny >>= aTmp ) && !aTmp.isEmpty() )
                    m_aContentType = m_aOriginalContentType = aTmp;
            }
            catch (const Exception&)
            {
                DBG_ASSERT( sal_False,
                            "getPropertyValue has thrown an exception! Please let developers know the scenario!" );
            }
        }
    }

    if ( m_aContentType.Len() )
    {
        // get the clipboard format using the content type
        ::com::sun::star::datatransfer::DataFlavor aDataFlavor;
        aDataFlavor.MimeType = m_aContentType;
        m_nFormat = SotExchange::GetFormat( aDataFlavor );

        // get the ClassId using the clipboard format ( internal table )
        m_aClassId = GetClassId_Impl( m_nFormat );

        // get human presentable name using the clipboard format
        SotExchange::GetFormatDataFlavor( m_nFormat, aDataFlavor );
        m_aUserTypeName = aDataFlavor.HumanPresentableName;

        if( m_pContent && !m_bIsLinked && m_aClassId != SvGlobalName() )
            ReadContent();
    }
}

void UCBStorage_Impl::CreateContent()
{
    try
    {
        // create content; where to put StreamMode ?! ( already done when opening the file of the package ? )
        Reference< ::com::sun::star::ucb::XCommandEnvironment > xComEnv;

        ::rtl::OUString aTemp( m_aURL );

        if ( m_bRepairPackage )
        {
            xComEnv = new ::ucbhelper::CommandEnvironment( Reference< ::com::sun::star::task::XInteractionHandler >(),
                                                     m_xProgressHandler );
            aTemp += rtl::OUString("?repairpackage");
        }

        m_pContent = new ::ucbhelper::Content( aTemp, xComEnv );
    }
    catch (const ContentCreationException&)
    {
        // content could not be created
        SetError( SVSTREAM_CANNOT_MAKE );
    }
    catch (const RuntimeException&)
    {
        // any other error - not specified
        SetError( SVSTREAM_CANNOT_MAKE );
    }
}

void UCBStorage_Impl::ReadContent()
{
   if ( m_bListCreated )
        return;

    m_bListCreated = sal_True;

    // create cursor for access to children
    Sequence< ::rtl::OUString > aProps(4);
    ::rtl::OUString* pProps = aProps.getArray();
    pProps[0] = ::rtl::OUString("Title");
    pProps[1] = ::rtl::OUString("IsFolder");
    pProps[2] = ::rtl::OUString("MediaType");
    pProps[3] = ::rtl::OUString("Size");
    ::ucbhelper::ResultSetInclude eInclude = ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS;

    try
    {
        GetContent();
        if ( !m_pContent )
            return;

        Reference< XResultSet > xResultSet = m_pContent->createCursor( aProps, eInclude );
        Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
        Reference< XRow > xRow( xResultSet, UNO_QUERY );
        if ( xResultSet.is() )
        {
            while ( xResultSet->next() )
            {
                // insert all into the children list
                ::rtl::OUString aTitle( xRow->getString(1) );
                ::rtl::OUString aContentType;
                if ( m_bIsLinked )
                {
                    // unpacked storages have to deal with the meta-inf folder by themselves
                    if ( aTitle == "META-INF" )
                        continue;
                }
                else
                {
                    aContentType = xRow->getString(3);
                }

                sal_Bool bIsFolder( xRow->getBoolean(2) );
                sal_Int64 nSize = xRow->getLong(4);
                UCBStorageElement_Impl* pElement = new UCBStorageElement_Impl( aTitle, bIsFolder, (sal_uLong) nSize );
                m_aChildrenList.push_back( pElement );

                sal_Bool bIsOfficeDocument = m_bIsLinked || ( m_aClassId != SvGlobalName() );
                if ( bIsFolder )
                {
                    if ( m_bIsLinked )
                        OpenStorage( pElement, m_nMode, m_bDirect );
                    if ( pElement->m_xStorage.Is() )
                        pElement->m_xStorage->Init();
                }
                else if ( bIsOfficeDocument )
                {
                    // streams can be external OLE objects, so they are now folders, but storages!
                    String aName( m_aURL );
                    aName += '/';
                    aName += String( xRow->getString(1) );

                    Reference< ::com::sun::star::ucb::XCommandEnvironment > xComEnv;
                    if ( m_bRepairPackage )
                    {
                        xComEnv = new ::ucbhelper::CommandEnvironment( Reference< ::com::sun::star::task::XInteractionHandler >(),
                                                                m_xProgressHandler );
                            aName += String(  "?repairpackage"  );
                    }

                    ::ucbhelper::Content aContent( aName, xComEnv );

                    ::rtl::OUString aMediaType;
                    Any aAny = aContent.getPropertyValue( ::rtl::OUString("MediaType") );
                    if ( ( aAny >>= aMediaType ) && ( aMediaType.compareToAscii("application/vnd.sun.star.oleobject") == 0 ) )
                        pElement->m_bIsStorage = sal_True;
                    else if ( aMediaType.isEmpty() )
                    {
                        // older files didn't have that special content type, so they must be detected
                        OpenStream( pElement, STREAM_STD_READ, m_bDirect );
                        if ( Storage::IsStorageFile( pElement->m_xStream ) )
                            pElement->m_bIsStorage = sal_True;
                        else
                            pElement->m_xStream->Free();
                    }
                }
            }
        }
    }
    catch (const InteractiveIOException& r)
    {
        if ( r.Code != IOErrorCode_NOT_EXISTING )
            SetError( ERRCODE_IO_GENERAL );
    }
    catch (const CommandAbortedException&)
    {
        // any command wasn't executed successfully - not specified
        if ( !( m_nMode & STREAM_WRITE ) )
            // if the folder was just inserted and not already commited, this is not an error!
            SetError( ERRCODE_IO_GENERAL );
    }
    catch (const RuntimeException&)
    {
        // any other error - not specified
        SetError( ERRCODE_IO_GENERAL );
    }
    catch (const ResultSetException&)
    {
        // means that the package file is broken
        SetError( ERRCODE_IO_BROKENPACKAGE );
    }
    catch (const SQLException&)
    {
        // means that the file can be broken
        SetError( ERRCODE_IO_WRONGFORMAT );
    }
    catch (const Exception&)
    {
        // any other error - not specified
        SetError( ERRCODE_IO_GENERAL );
    }
}

void UCBStorage_Impl::SetError( long nError )
{
    if ( !m_nError )
    {
        m_nError = nError;
        if ( m_pAntiImpl ) m_pAntiImpl->SetError( nError );
    }
}

sal_Int32 UCBStorage_Impl::GetObjectCount()
{
    sal_Int32 nCount = m_aChildrenList.size();
    for ( size_t i = 0; i < m_aChildrenList.size(); ++i )
    {
        UCBStorageElement_Impl* pElement = m_aChildrenList[ i ];
        DBG_ASSERT( !pElement->m_bIsFolder || pElement->m_xStorage.Is(), "Storage should be open!" );
        if ( pElement->m_bIsFolder && pElement->m_xStorage.Is() )
            nCount += pElement->m_xStorage->GetObjectCount();
    }

    return nCount;
}

::rtl::OUString Find_Impl( const Sequence < Sequence < PropertyValue > >& rSequence, const ::rtl::OUString& rPath )
{
    sal_Bool bFound = sal_False;
    for ( sal_Int32 nSeqs=0; nSeqs<rSequence.getLength(); nSeqs++ )
    {
        const Sequence < PropertyValue >& rMyProps = rSequence[nSeqs];
        ::rtl::OUString aType;

        for ( sal_Int32 nProps=0; nProps<rMyProps.getLength(); nProps++ )
        {
            const PropertyValue& rAny = rMyProps[nProps];
            if ( rAny.Name == "FullPath" )
            {
                rtl::OUString aTmp;
                if ( ( rAny.Value >>= aTmp ) && aTmp == rPath )
                    bFound = sal_True;
                if ( !aType.isEmpty() )
                    break;
            }
            else if ( rAny.Name == "MediaType" )
            {
                if ( ( rAny.Value >>= aType ) && !aType.isEmpty() && bFound )
                    break;
            }
        }

        if ( bFound )
            return aType;
    }

    return ::rtl::OUString();
}

void UCBStorage_Impl::SetProps( const Sequence < Sequence < PropertyValue > >& rSequence, const String& rPath )
{
    String aPath( rPath );
    if ( !m_bIsRoot )
        aPath += m_aName;
    aPath += '/';

    m_aContentType = m_aOriginalContentType = Find_Impl( rSequence, aPath );

    if ( m_bIsRoot )
        // the "FullPath" of a child always starts without '/'
        aPath.Erase();

    for ( size_t i = 0; i < m_aChildrenList.size(); ++i )
    {
        UCBStorageElement_Impl* pElement = m_aChildrenList[ i ];
        DBG_ASSERT( !pElement->m_bIsFolder || pElement->m_xStorage.Is(), "Storage should be open!" );
        if ( pElement->m_bIsFolder && pElement->m_xStorage.Is() )
            pElement->m_xStorage->SetProps( rSequence, aPath );
        else
        {
            String aElementPath( aPath );
            aElementPath += pElement->m_aName;
            pElement->SetContentType( Find_Impl( rSequence, aElementPath ) );
        }
    }

    if ( m_aContentType.Len() )
    {
        // get the clipboard format using the content type
        ::com::sun::star::datatransfer::DataFlavor aDataFlavor;
        aDataFlavor.MimeType = m_aContentType;
        m_nFormat = SotExchange::GetFormat( aDataFlavor );

        // get the ClassId using the clipboard format ( internal table )
        m_aClassId = GetClassId_Impl( m_nFormat );

        // get human presentable name using the clipboard format
        SotExchange::GetFormatDataFlavor( m_nFormat, aDataFlavor );
        m_aUserTypeName = aDataFlavor.HumanPresentableName;
    }
}

void UCBStorage_Impl::GetProps( sal_Int32& nProps, Sequence < Sequence < PropertyValue > >& rSequence, const String& rPath )
{
    // first my own properties
    Sequence < PropertyValue > aProps(2);

    // first property is the "FullPath" name
    // it's '/' for the root storage and m_aName for each element, followed by a '/' if it's a folder
    String aPath( rPath );
    if ( !m_bIsRoot )
        aPath += m_aName;
    aPath += '/';
    aProps[0].Name = ::rtl::OUString("MediaType");
    aProps[0].Value <<= (::rtl::OUString ) m_aContentType;
    aProps[1].Name = ::rtl::OUString("FullPath");
    aProps[1].Value <<= (::rtl::OUString ) aPath;
    rSequence[ nProps++ ] = aProps;

    if ( m_bIsRoot )
        // the "FullPath" of a child always starts without '/'
        aPath.Erase();

    // now the properties of my elements
    for ( size_t i = 0; i < m_aChildrenList.size(); ++i )
    {
        UCBStorageElement_Impl* pElement = m_aChildrenList[ i ];
        DBG_ASSERT( !pElement->m_bIsFolder || pElement->m_xStorage.Is(), "Storage should be open!" );
        if ( pElement->m_bIsFolder && pElement->m_xStorage.Is() )
            // storages add there properties by themselves ( see above )
            pElement->m_xStorage->GetProps( nProps, rSequence, aPath );
        else
        {
            // properties of streams
            String aElementPath( aPath );
            aElementPath += pElement->m_aName;
            aProps[0].Name = ::rtl::OUString("MediaType");
            aProps[0].Value <<= (::rtl::OUString ) pElement->GetContentType();
            aProps[1].Name = ::rtl::OUString("FullPath");
            aProps[1].Value <<= (::rtl::OUString ) aElementPath;
            rSequence[ nProps++ ] = aProps;
        }
    }
}

UCBStorage_Impl::~UCBStorage_Impl()
{
    // first delete elements!
    for ( size_t i = 0, n = m_aChildrenList.size(); i < n; ++i )
        delete m_aChildrenList[ i ];
    m_aChildrenList.clear();

    delete m_pContent;
    delete m_pTempFile;
}

sal_Bool UCBStorage_Impl::Insert( ::ucbhelper::Content *pContent )
{
    // a new substorage is inserted into a UCBStorage ( given by the parameter pContent )
    // it must be inserted with a title and a type
    sal_Bool bRet = sal_False;

    try
    {
        Sequence< ContentInfo > aInfo = pContent->queryCreatableContentsInfo();
        sal_Int32 nCount = aInfo.getLength();
        if ( nCount == 0 )
            return sal_False;

        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            // Simply look for the first KIND_FOLDER...
            const ContentInfo & rCurr = aInfo[i];
            if ( rCurr.Attributes & ContentInfoAttribute::KIND_FOLDER )
            {
                // Make sure the only required bootstrap property is "Title",
                const Sequence< Property > & rProps = rCurr.Properties;
                if ( rProps.getLength() != 1 )
                    continue;

                if ( rProps[ 0 ].Name != "Title" )
                    continue;

                Sequence < ::rtl::OUString > aNames(1);
                ::rtl::OUString* pNames = aNames.getArray();
                pNames[0] = ::rtl::OUString(  "Title"  );
                Sequence < Any > aValues(1);
                Any* pValues = aValues.getArray();
                pValues[0] = makeAny( ::rtl::OUString( m_aName ) );

                Content aNewFolder;
                if ( !pContent->insertNewContent( rCurr.Type, aNames, aValues, aNewFolder ) )
                    continue;

                // remove old content, create an "empty" new one and initialize it with the new inserted
                DELETEZ( m_pContent );
                m_pContent = new ::ucbhelper::Content( aNewFolder );
                bRet = sal_True;
            }
        }
    }
    catch (const CommandAbortedException&)
    {
        // any command wasn't executed successfully - not specified
        SetError( ERRCODE_IO_GENERAL );
    }
    catch (const RuntimeException&)
    {
        // any other error - not specified
        SetError( ERRCODE_IO_GENERAL );
    }
    catch (const Exception&)
    {
        // any other error - not specified
        SetError( ERRCODE_IO_GENERAL );
    }

    return bRet;
}

sal_Int16 UCBStorage_Impl::Commit()
{
    // send all changes to the package
    sal_Int16 nRet = COMMIT_RESULT_NOTHING_TO_DO;

    // there is nothing to do if the storage has been opened readonly or if it was opened in transacted mode and no
    // commit command has been sent
    if ( ( m_nMode & STREAM_WRITE ) && ( m_bCommited || m_bDirect ) )
    {
        try
        {
            // all errors will be caught in the "catch" statement outside the loop
            for ( size_t i = 0; i < m_aChildrenList.size() && nRet; ++i )
            {
                UCBStorageElement_Impl* pElement = m_aChildrenList[ i ];
                ::ucbhelper::Content* pContent = pElement->GetContent();
                sal_Bool bDeleteContent = sal_False;
                if ( !pContent && pElement->IsModified() )
                {
                    // if the element has never been opened, no content has been created until now
                    bDeleteContent = sal_True;  // remember to delete it later
                    String aName( m_aURL );
                    aName += '/';
                    aName += pElement->m_aOriginalName;
                    pContent = new ::ucbhelper::Content( aName, Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
                }

                if ( pElement->m_bIsRemoved )
                {
                    // was it inserted, then removed (so there would be nothing to do!)
                    if ( !pElement->m_bIsInserted )
                    {
                        // first remove all open stream handles
                        if( !pElement->m_xStream.Is() || pElement->m_xStream->Clear() )
                        {
                            pContent->executeCommand( ::rtl::OUString("delete"), makeAny( sal_Bool( sal_True ) ) );
                            nRet = COMMIT_RESULT_SUCCESS;
                        }
                        else
                            // couldn't release stream because there are external references to it
                            nRet = COMMIT_RESULT_FAILURE;
                    }
                }
                else
                {
                    sal_Int16 nLocalRet = COMMIT_RESULT_NOTHING_TO_DO;
                    if ( pElement->m_xStorage.Is() )
                    {
                        // element is a storage
                        // do a commit in the following cases:
                        //  - if storage is already inserted, and changed
                        //  - storage is not in a package
                        //  - it's a new storage, try to insert and commit if successful inserted
                        if ( !pElement->m_bIsInserted || m_bIsLinked || pElement->m_xStorage->Insert( m_pContent ) )
                        {
                            nLocalRet = pElement->m_xStorage->Commit();
                            pContent = pElement->GetContent();
                        }
                    }
                    else if ( pElement->m_xStream.Is() )
                    {
                        // element is a stream
                        nLocalRet = pElement->m_xStream->Commit();
                        if ( pElement->m_xStream->m_bIsOLEStorage )
                        {
                            // OLE storage should be stored encrytped, if the storage uses encryption
                            pElement->m_xStream->m_aContentType = rtl::OUString("application/vnd.sun.star.oleobject");
                            Any aValue;
                            aValue <<= (sal_Bool) sal_True;
                            pElement->m_xStream->m_pContent->setPropertyValue(rtl::OUString("Encrypted"), aValue );
                        }

                        pContent = pElement->GetContent();
                    }

                    if ( pElement->m_aName != pElement->m_aOriginalName )
                    {
                        // name ( title ) of the element was changed
                        nLocalRet = COMMIT_RESULT_SUCCESS;
                        Any aAny;
                        aAny <<= (rtl::OUString) pElement->m_aName;
                        pContent->setPropertyValue( ::rtl::OUString("Title"), aAny );
                    }

                    if ( pElement->IsLoaded() && pElement->GetContentType() != pElement->GetOriginalContentType() )
                    {
                        // mediatype of the element was changed
                        nLocalRet = COMMIT_RESULT_SUCCESS;
                        Any aAny;
                        aAny <<= (rtl::OUString) pElement->GetContentType();
                        pContent->setPropertyValue( ::rtl::OUString("MediaType"), aAny );
                    }

                    if ( nLocalRet != COMMIT_RESULT_NOTHING_TO_DO )
                        nRet = nLocalRet;
                }

                if ( bDeleteContent )
                    // content was created inside the loop
                    delete pContent;

                if ( nRet == COMMIT_RESULT_FAILURE )
                    break;
            }
        }
        catch (const ContentCreationException&)
        {
            // content could not be created
            SetError( ERRCODE_IO_NOTEXISTS );
            return COMMIT_RESULT_FAILURE;
        }
        catch (const CommandAbortedException&)
        {
            // any command wasn't executed successfully - not specified
            SetError( ERRCODE_IO_GENERAL );
            return COMMIT_RESULT_FAILURE;
        }
        catch (const RuntimeException&)
        {
            // any other error - not specified
            SetError( ERRCODE_IO_GENERAL );
            return COMMIT_RESULT_FAILURE;
        }
        catch (const Exception&)
        {
            // any other error - not specified
            SetError( ERRCODE_IO_GENERAL );
            return COMMIT_RESULT_FAILURE;
        }

        if ( m_bIsRoot && m_pContent )
        {
            // the root storage must flush the root package content
            if ( nRet == COMMIT_RESULT_SUCCESS )
            {
                try
                {
                    // commit the media type to the JAR file
                    // clipboard format and ClassId will be retrieved from the media type when the file is loaded again
                    Any aType;
                    aType <<= (rtl::OUString) m_aContentType;
                    m_pContent->setPropertyValue( ::rtl::OUString("MediaType"), aType );

                    if (  m_bIsLinked )
                    {
                        // write a manifest file
                        // first create a subfolder "META-inf"
                        Content aNewSubFolder;
                        sal_Bool bRet = ::utl::UCBContentHelper::MakeFolder( *m_pContent, rtl::OUString("META-INF"), aNewSubFolder );
                        if ( bRet )
                        {
                            // create a stream to write the manifest file - use a temp file
                            String aURL( aNewSubFolder.getURL() );
                            ::utl::TempFile* pTempFile = new ::utl::TempFile( &aURL );

                            // get the stream from the temp file and create an output stream wrapper
                            SvStream* pStream = pTempFile->GetStream( STREAM_STD_READWRITE );
                            ::utl::OOutputStreamWrapper* pHelper = new ::utl::OOutputStreamWrapper( *pStream );
                            com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > xOutputStream( pHelper );

                            // create a manifest writer object that will fill the stream
                            Reference < ::com::sun::star::packages::manifest::XManifestWriter > xWriter =
                                Reference< ::com::sun::star::packages::manifest::XManifestWriter >
                                    ( ::comphelper::getProcessServiceFactory()->createInstance(
                                        ::rtl::OUString("com.sun.star.packages.manifest.ManifestWriter")), UNO_QUERY) ;
                            sal_Int32 nCount = GetObjectCount() + 1;
                            Sequence < Sequence < PropertyValue > > aProps( nCount );
                            sal_Int32 nProps = 0;
                            GetProps( nProps, aProps, String() );
                            xWriter->writeManifestSequence( xOutputStream, aProps );

                            // move the stream to its desired location
                            Content aSource( pTempFile->GetURL(), Reference < XCommandEnvironment >() );
                            xWriter = NULL;
                            xOutputStream = NULL;
                            DELETEZ( pTempFile );
                            aNewSubFolder.transferContent( aSource, InsertOperation_MOVE, ::rtl::OUString("manifest.xml"), NameClash::OVERWRITE );
                        }
                    }
                    else
                    {
#if OSL_DEBUG_LEVEL > 1
                        fprintf ( stderr, "Files: %i\n", nOpenFiles );
                        fprintf ( stderr, "Streams: %i\n", nOpenStreams );
#endif
                        // force writing
                        Any aAny;
                        m_pContent->executeCommand( ::rtl::OUString("flush"), aAny );
                        if ( m_pSource != 0 )
                        {
                            SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( m_pTempFile->GetURL(), STREAM_STD_READ );
                            m_pSource->SetStreamSize(0);
                            // m_pSource->Seek(0);
                            *pStream >> *m_pSource;
                            DELETEZ( pStream );
                            m_pSource->Seek(0);
                        }
                    }
                }
                catch (const CommandAbortedException&)
                {
                    // how to tell the content : forget all changes ?!
                    // or should we assume that the content does it by itself because he throwed an exception ?!
                    // any command wasn't executed successfully - not specified
                    SetError( ERRCODE_IO_GENERAL );
                    return COMMIT_RESULT_FAILURE;
                }
                catch (const RuntimeException&)
                {
                    // how to tell the content : forget all changes ?!
                    // or should we assume that the content does it by itself because he throwed an exception ?!
                    // any other error - not specified
                    SetError( ERRCODE_IO_GENERAL );
                    return COMMIT_RESULT_FAILURE;
                }
                catch (const InteractiveIOException& r)
                {
                    if ( r.Code == IOErrorCode_ACCESS_DENIED || r.Code == IOErrorCode_LOCKING_VIOLATION )
                        SetError( ERRCODE_IO_ACCESSDENIED );
                    else if ( r.Code == IOErrorCode_NOT_EXISTING )
                        SetError( ERRCODE_IO_NOTEXISTS );
                    else if ( r.Code == IOErrorCode_CANT_READ )
                        SetError( ERRCODE_IO_CANTREAD );
                    else if ( r.Code == IOErrorCode_CANT_WRITE )
                        SetError( ERRCODE_IO_CANTWRITE );
                    else
                        SetError( ERRCODE_IO_GENERAL );

                    return COMMIT_RESULT_FAILURE;
                }
                catch (const Exception&)
                {
                    // how to tell the content : forget all changes ?!
                    // or should we assume that the content does it by itself because he throwed an exception ?!
                    // any other error - not specified
                    SetError( ERRCODE_IO_GENERAL );
                    return COMMIT_RESULT_FAILURE;
                }
            }
            else if ( nRet != COMMIT_RESULT_NOTHING_TO_DO )
            {
                // how to tell the content : forget all changes ?! Should we ?!
                SetError( ERRCODE_IO_GENERAL );
                return nRet;
            }

            // after successfull root commit all elements names and types are adjusted and all removed elements
            // are also removed from the lists
            for ( size_t i = 0; i < m_aChildrenList.size(); )
            {
                UCBStorageElement_Impl* pInnerElement = m_aChildrenList[ i ];
                if ( pInnerElement->m_bIsRemoved )
                {
                    UCBStorageElementList_Impl::iterator it = m_aChildrenList.begin();
                    ::std::advance( it, i );
                    delete *it;
                    m_aChildrenList.erase( it );
                }
                else
                {
                    pInnerElement->m_aOriginalName = pInnerElement->m_aName;
                    pInnerElement->m_bIsInserted = sal_False;
                    ++i;
                }
            }
        }

        m_bCommited = sal_False;
    }

    return nRet;
}

sal_Bool UCBStorage_Impl::Revert()
{
    for ( size_t i = 0; i < m_aChildrenList.size(); )
    {
        UCBStorageElement_Impl* pElement = m_aChildrenList[ i ];
        pElement->m_bIsRemoved = sal_False;
        if ( pElement->m_bIsInserted )
        {
            UCBStorageElementList_Impl::iterator it = m_aChildrenList.begin();
            ::std::advance( it, i );
            delete *it;
            m_aChildrenList.erase( it );
        }
        else
        {
            if ( pElement->m_xStream.Is() )
            {
                pElement->m_xStream->m_bCommited = sal_False;
                pElement->m_xStream->Revert();
            }
            else if ( pElement->m_xStorage.Is() )
            {
                pElement->m_xStorage->m_bCommited = sal_False;
                pElement->m_xStorage->Revert();
            }

            pElement->m_aName = pElement->m_aOriginalName;
            pElement->m_bIsRemoved = sal_False;
            ++i;
        }
    }
    return sal_True;
}

const String& UCBStorage::GetName() const
{
    return pImp->m_aName; // pImp->m_aURL ?!
}

sal_Bool UCBStorage::IsRoot() const
{
    return pImp->m_bIsRoot;
}

void UCBStorage::SetDirty()
{
    pImp->m_bDirty = sal_True;
}

void UCBStorage::SetClass( const SvGlobalName & rClass, sal_uLong nOriginalClipFormat, const String & rUserTypeName )
{
    pImp->m_aClassId = rClass;
    pImp->m_nFormat = nOriginalClipFormat;
    pImp->m_aUserTypeName = rUserTypeName;

    // in UCB storages only the content type will be stored, all other information can be reconstructed
    // ( see the UCBStorage_Impl::Init() method )
    ::com::sun::star::datatransfer::DataFlavor aDataFlavor;
    SotExchange::GetFormatDataFlavor( pImp->m_nFormat, aDataFlavor );
    pImp->m_aContentType = aDataFlavor.MimeType;
}

void UCBStorage::SetClassId( const ClsId& rClsId )
{
    pImp->m_aClassId = SvGlobalName( (const CLSID&) rClsId );
    if ( pImp->m_aClassId == SvGlobalName() )
        return;

    // in OLE storages the clipboard format an the user name will be transferred when a storage is copied because both are
    // stored in one the substreams
    // UCB storages store the content type information as content type in the manifest file and so this information must be
    // kept up to date, and also the other type information that is hold only at runtime because it can be reconstructed from
    // the content type
    pImp->m_nFormat = GetFormatId_Impl( pImp->m_aClassId );
    if ( pImp->m_nFormat )
    {
        ::com::sun::star::datatransfer::DataFlavor aDataFlavor;
        SotExchange::GetFormatDataFlavor( pImp->m_nFormat, aDataFlavor );
        pImp->m_aUserTypeName = aDataFlavor.HumanPresentableName;
        pImp->m_aContentType = aDataFlavor.MimeType;
    }
}

const ClsId& UCBStorage::GetClassId() const
{
    return ( const ClsId& ) pImp->m_aClassId.GetCLSID();
}

void UCBStorage::SetConvertClass( const SvGlobalName & /*rConvertClass*/, sal_uLong /*nOriginalClipFormat*/, const String & /*rUserTypeName*/ )
{
    // ???
}

sal_Bool UCBStorage::ShouldConvert()
{
    // ???
    return sal_False;
}

SvGlobalName UCBStorage::GetClassName()
{
    return  pImp->m_aClassId;
}

sal_uLong UCBStorage::GetFormat()
{
    return pImp->m_nFormat;
}

String UCBStorage::GetUserName()
{
    OSL_FAIL("UserName is not implemented in UCB storages!" );
    return pImp->m_aUserTypeName;
}

void UCBStorage::FillInfoList( SvStorageInfoList* pList ) const
{
    // put information in childrenlist into StorageInfoList
    for ( size_t i = 0; i < pImp->GetChildrenList().size(); ++i )
    {
        UCBStorageElement_Impl* pElement = pImp->GetChildrenList()[ i ];
        if ( !pElement->m_bIsRemoved )
        {
            // problem: what about the size of a substorage ?!
            sal_uLong nSize = pElement->m_nSize;
            if ( pElement->m_xStream.Is() )
                nSize = pElement->m_xStream->GetSize();
            SvStorageInfo aInfo( pElement->m_aName, nSize, pElement->m_bIsStorage );
            pList->push_back( aInfo );
        }
    }
}

sal_Bool UCBStorage::CopyStorageElement_Impl( UCBStorageElement_Impl& rElement, BaseStorage* pDest, const String& rNew ) const
{
    // insert stream or storage into the list or stream of the destination storage
    // not into the content, this will be done on commit !
    // be aware of name changes !
    if ( !rElement.m_bIsStorage )
    {
        // copy the streams data
        // the destination stream must not be open
        BaseStorageStream* pOtherStream = pDest->OpenStream( rNew, STREAM_WRITE | STREAM_SHARE_DENYALL, pImp->m_bDirect );
        BaseStorageStream* pStream = NULL;
        sal_Bool bDeleteStream = sal_False;

        // if stream is already open, it is allowed to copy it, so be aware of this
        if ( rElement.m_xStream.Is() )
            pStream = rElement.m_xStream->m_pAntiImpl;
        if ( !pStream )
        {
            pStream = ( const_cast < UCBStorage* > (this) )->OpenStream( rElement.m_aName, STREAM_STD_READ, pImp->m_bDirect );
            bDeleteStream = sal_True;
        }

        pStream->CopyTo( pOtherStream );
        SetError( pStream->GetError() );
        if( pOtherStream->GetError() )
            pDest->SetError( pOtherStream->GetError() );
        else
            pOtherStream->Commit();

        if ( bDeleteStream )
            delete pStream;
        delete pOtherStream;
    }
    else
    {
        // copy the storage content
        // the destination storage must not be open
        BaseStorage* pStorage = NULL;

        // if stream is already open, it is allowed to copy it, so be aware of this
        sal_Bool bDeleteStorage = sal_False;
        if ( rElement.m_xStorage.Is() )
            pStorage = rElement.m_xStorage->m_pAntiImpl;
        if ( !pStorage )
        {
            pStorage = ( const_cast < UCBStorage* > (this) )->OpenStorage( rElement.m_aName, pImp->m_nMode, pImp->m_bDirect );
            bDeleteStorage = sal_True;
        }

        UCBStorage* pUCBDest = PTR_CAST( UCBStorage, pDest );
        UCBStorage* pUCBCopy = PTR_CAST( UCBStorage, pStorage );

        sal_Bool bOpenUCBStorage = pUCBDest && pUCBCopy;
        BaseStorage* pOtherStorage = bOpenUCBStorage ?
                pDest->OpenUCBStorage( rNew, STREAM_WRITE | STREAM_SHARE_DENYALL, pImp->m_bDirect ) :
                pDest->OpenOLEStorage( rNew, STREAM_WRITE | STREAM_SHARE_DENYALL, pImp->m_bDirect );

        // For UCB storages, the class id and the format id may differ,
        // do passing the class id is not sufficient.
        if( bOpenUCBStorage )
            pOtherStorage->SetClass( pStorage->GetClassName(),
                                     pStorage->GetFormat(),
                                     pUCBCopy->pImp->m_aUserTypeName );
        else
            pOtherStorage->SetClassId( pStorage->GetClassId() );
        pStorage->CopyTo( pOtherStorage );
        SetError( pStorage->GetError() );
        if( pOtherStorage->GetError() )
            pDest->SetError( pOtherStorage->GetError() );
        else
            pOtherStorage->Commit();

        if ( bDeleteStorage )
            delete pStorage;
        delete pOtherStorage;
    }

    return sal_Bool( Good() && pDest->Good() );
}

UCBStorageElement_Impl* UCBStorage::FindElement_Impl( const String& rName ) const
{
    DBG_ASSERT( rName.Len(), "Name is empty!" );
    for ( size_t i = 0, n = pImp->GetChildrenList().size(); i < n; ++i )
    {
        UCBStorageElement_Impl* pElement = pImp->GetChildrenList()[ i ];
        if ( pElement->m_aName == rName && !pElement->m_bIsRemoved )
            return pElement;
    }
    return NULL;
}

sal_Bool UCBStorage::CopyTo( BaseStorage* pDestStg ) const
{
    DBG_ASSERT( pDestStg != ((BaseStorage*)this), "Self-Copying is not possible!" );
    if ( pDestStg == ((BaseStorage*)this) )
        return sal_False;

    // perhaps it's also a problem if one storage is a parent of the other ?!
    // or if not: could be optimized ?!

    // For UCB storages, the class id and the format id may differ,
    // do passing the class id is not sufficient.
    if( pDestStg->ISA( UCBStorage ) )
        pDestStg->SetClass( pImp->m_aClassId, pImp->m_nFormat,
                            pImp->m_aUserTypeName );
    else
        pDestStg->SetClassId( GetClassId() );
    pDestStg->SetDirty();

    sal_Bool bRet = sal_True;
    for ( size_t i = 0; i < pImp->GetChildrenList().size() && bRet; ++i )
    {
        UCBStorageElement_Impl* pElement = pImp->GetChildrenList()[ i ];
        if ( !pElement->m_bIsRemoved )
            bRet = CopyStorageElement_Impl( *pElement, pDestStg, pElement->m_aName );
    }

    if( !bRet )
        SetError( pDestStg->GetError() );
    return sal_Bool( Good() && pDestStg->Good() );
}

sal_Bool UCBStorage::CopyTo( const String& rElemName, BaseStorage* pDest, const String& rNew )
{
    if( !rElemName.Len() )
        return sal_False;

    if ( pDest == ((BaseStorage*) this) )
    {
        // can't double an element
        return sal_False;
    }
    else
    {
        // for copying no optimization is usefull, because in every case the stream data must be copied
            UCBStorageElement_Impl* pElement = FindElement_Impl( rElemName );
        if ( pElement )
            return CopyStorageElement_Impl( *pElement, pDest, rNew );
        else
        {
            SetError( SVSTREAM_FILE_NOT_FOUND );
            return sal_False;
        }
    }
}

sal_Bool UCBStorage::Commit()
{
    // mark this storage for sending it on root commit
    pImp->m_bCommited = sal_True;
    if ( pImp->m_bIsRoot )
        // the root storage coordinates commiting by sending a Commit command to its content
        return ( pImp->Commit() != COMMIT_RESULT_FAILURE );
    else
        return sal_True;
}

sal_Bool UCBStorage::Revert()
{
    return pImp->Revert();
}

BaseStorageStream* UCBStorage::OpenStream( const String& rEleName, StreamMode nMode, sal_Bool bDirect, const rtl::OString* pKey )
{
    if( !rEleName.Len() )
        return NULL;

    // try to find the storage element
    UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    if ( !pElement )
    {
        // element does not exist, check if creation is allowed
        if( ( nMode & STREAM_NOCREATE ) )
        {
            SetError( ( nMode & STREAM_WRITE ) ? SVSTREAM_CANNOT_MAKE : SVSTREAM_FILE_NOT_FOUND );
            String aName( pImp->m_aURL );
            aName += '/';
            aName += rEleName;
            UCBStorageStream* pStream = new UCBStorageStream( aName, nMode, bDirect, pKey, pImp->m_bRepairPackage, pImp->m_xProgressHandler );
            pStream->SetError( GetError() );
            pStream->pImp->m_aName = rEleName;
            return pStream;
        }
        else
        {
            // create a new UCBStorageElement and insert it into the list
            pElement = new UCBStorageElement_Impl( rEleName );
            pElement->m_bIsInserted = sal_True;
            pImp->m_aChildrenList.push_back( pElement );
        }
    }

    if ( pElement && !pElement->m_bIsFolder )
    {
        // check if stream is already created
        if ( pElement->m_xStream.Is() )
        {
            // stream has already been created; if it has no external reference, it may be opened another time
            if ( pElement->m_xStream->m_pAntiImpl )
            {
                OSL_FAIL("Stream is already open!" );
                SetError( SVSTREAM_ACCESS_DENIED );  // ???
                return NULL;
            }
            else
            {
                // check if stream is opened with the same keyword as before
                // if not, generate a new stream because it could be encrypted vs. decrypted!
                rtl::OString aKey;
                if ( pKey )
                    aKey = *pKey;
                if ( pElement->m_xStream->m_aKey == aKey )
                {
                    pElement->m_xStream->PrepareCachedForReopen( nMode );

                    return new UCBStorageStream( pElement->m_xStream );
                }
            }
        }

        // stream is opened the first time
        pImp->OpenStream( pElement, nMode, bDirect, pKey );

        // if name has been changed before creating the stream: set name!
        pElement->m_xStream->m_aName = rEleName;
        return new UCBStorageStream( pElement->m_xStream );
    }

    return NULL;
}

UCBStorageStream_Impl* UCBStorage_Impl::OpenStream( UCBStorageElement_Impl* pElement, StreamMode nMode, sal_Bool bDirect, const rtl::OString* pKey )
{
    String aName( m_aURL );
    aName += '/';
    aName += pElement->m_aOriginalName;
    pElement->m_xStream = new UCBStorageStream_Impl( aName, nMode, NULL, bDirect, pKey, m_bRepairPackage, m_xProgressHandler );
    return pElement->m_xStream;
}

BaseStorage* UCBStorage::OpenUCBStorage( const String& rEleName, StreamMode nMode, sal_Bool bDirect )
{
    if( !rEleName.Len() )
        return NULL;

    return OpenStorage_Impl( rEleName, nMode, bDirect, sal_True );
}

BaseStorage* UCBStorage::OpenOLEStorage( const String& rEleName, StreamMode nMode, sal_Bool bDirect )
{
    if( !rEleName.Len() )
        return NULL;

    return OpenStorage_Impl( rEleName, nMode, bDirect, sal_False );
}

BaseStorage* UCBStorage::OpenStorage( const String& rEleName, StreamMode nMode, sal_Bool bDirect )
{
    if( !rEleName.Len() )
        return NULL;

    return OpenStorage_Impl( rEleName, nMode, bDirect, sal_True );
}

BaseStorage* UCBStorage::OpenStorage_Impl( const String& rEleName, StreamMode nMode, sal_Bool bDirect, sal_Bool bForceUCBStorage )
{
    // try to find the storage element
    UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    if ( !pElement )
    {
        // element does not exist, check if creation is allowed
        if( ( nMode & STREAM_NOCREATE ) )
        {
            SetError( ( nMode & STREAM_WRITE ) ? SVSTREAM_CANNOT_MAKE : SVSTREAM_FILE_NOT_FOUND );
            String aName( pImp->m_aURL );
            aName += '/';
            aName += rEleName;  //  ???
            UCBStorage *pStorage = new UCBStorage( aName, nMode, bDirect, sal_False, pImp->m_bRepairPackage, pImp->m_xProgressHandler );
            pStorage->pImp->m_bIsRoot = sal_False;
            pStorage->pImp->m_bListCreated = sal_True; // the storage is pretty new, nothing to read
            pStorage->SetError( GetError() );
            return pStorage;
        }

        // create a new UCBStorageElement and insert it into the list
        // problem: perhaps an OLEStorage should be created ?!
        // Because nothing is known about the element that should be created, an external parameter is needed !
        pElement = new UCBStorageElement_Impl( rEleName );
        pElement->m_bIsInserted = sal_True;
        pImp->m_aChildrenList.push_back( pElement );
    }

    if ( !pElement->m_bIsFolder && ( pElement->m_bIsStorage || !bForceUCBStorage ) )
    {
        // create OLE storages on a stream ( see ctor of SotStorage )
        // Such a storage will be created on a UCBStorageStream; it will write into the stream
        // if it is opened in direct mode or when it is committed. In this case the stream will be
        // modified and then it MUST be treated as commited.
        if ( !pElement->m_xStream.Is() )
        {
            BaseStorageStream* pStr = OpenStream( rEleName, nMode, bDirect );
            UCBStorageStream* pStream = PTR_CAST( UCBStorageStream, pStr );
            if ( !pStream )
            {
                SetError( ( nMode & STREAM_WRITE ) ? SVSTREAM_CANNOT_MAKE : SVSTREAM_FILE_NOT_FOUND );
                return NULL;
            }

            pElement->m_xStream = pStream->pImp;
            delete pStream;
        }

        pElement->m_xStream->PrepareCachedForReopen( nMode );
        pElement->m_xStream->Init();

        pElement->m_bIsStorage = sal_True;
        return pElement->m_xStream->CreateStorage();  // can only be created in transacted mode
    }
    else if ( pElement->m_xStorage.Is() )
    {
        // storage has already been opened; if it has no external reference, it may be opened another time
        if ( pElement->m_xStorage->m_pAntiImpl )
        {
            OSL_FAIL("Storage is already open!" );
            SetError( SVSTREAM_ACCESS_DENIED );  // ???
        }
        else
        {
            sal_Bool bIsWritable = (( pElement->m_xStorage->m_nMode & STREAM_WRITE ) != 0);
            if ( !bIsWritable && (( nMode & STREAM_WRITE ) != 0 ))
            {
                String aName( pImp->m_aURL );
                aName += '/';
                aName += pElement->m_aOriginalName;
                UCBStorage* pStorage = new UCBStorage( aName, nMode, bDirect, sal_False, pImp->m_bRepairPackage, pImp->m_xProgressHandler );
                pElement->m_xStorage = pStorage->pImp;
                return pStorage;
            }
            else
            {
                return new UCBStorage( pElement->m_xStorage );
            }
        }
    }
    else if ( !pElement->m_xStream.Is() )
    {
        // storage is opened the first time
        sal_Bool bIsWritable = (( pImp->m_nMode & STREAM_WRITE ) != 0 );
        if ( pImp->m_bIsLinked && pImp->m_bIsRoot && bIsWritable )
        {
            // make sure that the root storage object has been created before substorages will be created
            INetURLObject aFolderObj( pImp->m_aURL );
            String aName = aFolderObj.GetName();
            aFolderObj.removeSegment();

            Content aFolder( aFolderObj.GetMainURL( INetURLObject::NO_DECODE ), Reference < XCommandEnvironment >() );
            pImp->m_pContent = new Content;
            sal_Bool bRet = ::utl::UCBContentHelper::MakeFolder( aFolder, pImp->m_aName, *pImp->m_pContent );
            if ( !bRet )
            {
                SetError( SVSTREAM_CANNOT_MAKE );
                return NULL;
            }
        }

        UCBStorage_Impl* pStor = pImp->OpenStorage( pElement, nMode, bDirect );
        if ( pStor )
        {
            if ( pElement->m_bIsInserted )
                pStor->m_bListCreated = sal_True; // the storage is pretty new, nothing to read

            return new UCBStorage( pStor );
        }
    }

    return NULL;
}

UCBStorage_Impl* UCBStorage_Impl::OpenStorage( UCBStorageElement_Impl* pElement, StreamMode nMode, sal_Bool bDirect )
{
    UCBStorage_Impl* pRet = NULL;
    String aName( m_aURL );
    aName += '/';
    aName += pElement->m_aOriginalName;  //  ???

    pElement->m_bIsStorage = pElement->m_bIsFolder = sal_True;

    if ( m_bIsLinked && !::utl::UCBContentHelper::Exists( aName ) )
    {
        Content aNewFolder;
        sal_Bool bRet = ::utl::UCBContentHelper::MakeFolder( *m_pContent, pElement->m_aOriginalName, aNewFolder );
        if ( bRet )
            pRet = new UCBStorage_Impl( aNewFolder, aName, nMode, NULL, bDirect, sal_False, m_bRepairPackage, m_xProgressHandler );
    }
    else
    {
        pRet = new UCBStorage_Impl( aName, nMode, NULL, bDirect, sal_False, m_bRepairPackage, m_xProgressHandler );
    }

    if ( pRet )
    {
        pRet->m_bIsLinked = m_bIsLinked;
        pRet->m_bIsRoot = sal_False;

        // if name has been changed before creating the stream: set name!
        pRet->m_aName = pElement->m_aOriginalName;
        pElement->m_xStorage = pRet;
    }

    if ( pRet )
        pRet->Init();

    return pRet;
}

sal_Bool UCBStorage::IsStorage( const String& rEleName ) const
{
    if( !rEleName.Len() )
        return sal_False;

    const UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    return ( pElement && pElement->m_bIsStorage );
}

sal_Bool UCBStorage::IsStream( const String& rEleName ) const
{
    if( !rEleName.Len() )
        return sal_False;

    const UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    return ( pElement && !pElement->m_bIsStorage );
}

sal_Bool UCBStorage::IsContained( const String & rEleName ) const
{
    if( !rEleName.Len() )
        return sal_False;
    const UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    return ( pElement != NULL );
}

sal_Bool UCBStorage::Remove( const String& rEleName )
{
    if( !rEleName.Len() )
        return sal_False;

    UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    if ( pElement )
    {
        pElement->m_bIsRemoved = sal_True;
    }
    else
        SetError( SVSTREAM_FILE_NOT_FOUND );

    return ( pElement != NULL );
}

sal_Bool UCBStorage::Rename( const String& rEleName, const String& rNewName )
{
    if( !rEleName.Len()|| !rNewName.Len() )
        return sal_False;

    UCBStorageElement_Impl *pAlreadyExisting = FindElement_Impl( rNewName );
    if ( pAlreadyExisting )
    {
        SetError( SVSTREAM_ACCESS_DENIED );
        return sal_False;                       // can't change to a name that is already used
    }

    UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    if ( pElement )
    {
        pElement->m_aName = rNewName;
    }
    else
        SetError( SVSTREAM_FILE_NOT_FOUND );

    return pElement != NULL;
}

sal_Bool UCBStorage::MoveTo( const String& rEleName, BaseStorage* pNewSt, const String& rNewName )
{
    if( !rEleName.Len() || !rNewName.Len() )
        return sal_False;

    if ( pNewSt == ((BaseStorage*) this) && !FindElement_Impl( rNewName ) )
    {
        return Rename( rEleName, rNewName );
    }
    else
    {
/*
        if ( PTR_CAST( UCBStorage, pNewSt ) )
        {
            // because the element is moved, not copied, a special optimization is possible :
            // first copy the UCBStorageElement; flag old element as "Removed" and new as "Inserted",
            // clear original name/type of the new element
            // if moved element is open: copy content, but change absolute URL ( and those of all children of the element! ),
            // clear original name/type of new content, keep the old original stream/storage, but forget its working streams,
                // close original UCBContent and original stream, only the TempFile and its stream may remain unchanged, but now
            // belong to the new content
            // if original and editable stream are identical ( readonly element ), it has to be copied to the editable
            // stream of the destination object
            // Not implemented at the moment ( risky?! ), perhaps later
        }
*/
        // MoveTo is done by first copying to the new destination and then removing the old element
        sal_Bool bRet = CopyTo( rEleName, pNewSt, rNewName );
        if ( bRet )
            bRet = Remove( rEleName );
        return bRet;
    }
}

sal_Bool UCBStorage::ValidateFAT()
{
    // ???
    return sal_True;
}

sal_Bool UCBStorage::Validate( sal_Bool  bWrite ) const
{
    // ???
    return ( !bWrite || ( pImp->m_nMode & STREAM_WRITE ) );
}

sal_Bool UCBStorage::ValidateMode( StreamMode m ) const
{
    // ???
    if( m == ( STREAM_READ | STREAM_TRUNC ) )  // from stg.cxx
        return sal_True;
    sal_uInt16 nCurMode = 0xFFFF;
    if( ( m & 3 ) == STREAM_READ )
    {
        // only SHARE_DENYWRITE or SHARE_DENYALL allowed
        if( ( ( m & STREAM_SHARE_DENYWRITE )
           && ( nCurMode & STREAM_SHARE_DENYWRITE ) )
         || ( ( m & STREAM_SHARE_DENYALL )
           && ( nCurMode & STREAM_SHARE_DENYALL ) ) )
            return sal_True;
    }
    else
    {
        // only SHARE_DENYALL allowed
        // storages open in r/o mode are OK, since only
        // the commit may fail
        if( ( m & STREAM_SHARE_DENYALL )
         && ( nCurMode & STREAM_SHARE_DENYALL ) )
            return sal_True;
    }

    return sal_True;
}

const SvStream* UCBStorage::GetSvStream() const
{
    // this would cause a complete download of the file
    // as it looks, this method is NOT used inside of SOT, only exported by class SotStorage - but for what ???
    return pImp->m_pSource;
}

sal_Bool UCBStorage::Equals( const BaseStorage& rStorage ) const
{
    // ???
    return ((BaseStorage*)this) == &rStorage;
}

sal_Bool UCBStorage::IsStorageFile( SvStream* pFile )
{
    if ( !pFile )
        return sal_False;

    sal_uLong nPos = pFile->Tell();
    pFile->Seek( STREAM_SEEK_TO_END );
    if ( pFile->Tell() < 4 )
        return sal_False;

    pFile->Seek(0);
    sal_uInt32 nBytes(0);
    *pFile >> nBytes;

    // search for the magic bytes
    sal_Bool bRet = ( nBytes == 0x04034b50 );
    if ( !bRet )
    {
        // disk spanned file have an additional header in front of the usual one
        bRet = ( nBytes == 0x08074b50 );
        if ( bRet )
        {
            nBytes = 0;
            *pFile >> nBytes;
            bRet = ( nBytes == 0x04034b50 );
        }
    }

    pFile->Seek( nPos );
    return bRet;
}

sal_Bool UCBStorage::IsDiskSpannedFile( SvStream* pFile )
{
    if ( !pFile )
        return sal_False;

    sal_uLong nPos = pFile->Tell();
    pFile->Seek( STREAM_SEEK_TO_END );
    if ( !pFile->Tell() )
        return sal_False;

    pFile->Seek(0);
    sal_uInt32 nBytes;
    *pFile >> nBytes;

    // disk spanned file have an additional header in front of the usual one
    sal_Bool bRet = ( nBytes == 0x08074b50 );
    if ( bRet )
    {
        *pFile >> nBytes;
        bRet = ( nBytes == 0x04034b50 );
    }

    pFile->Seek( nPos );
    return bRet;
}

String UCBStorage::GetLinkedFile( SvStream &rStream )
{
    String aString;
    sal_uLong nPos = rStream.Tell();
    rStream.Seek( STREAM_SEEK_TO_END );
    if ( !rStream.Tell() )
        return aString;

    rStream.Seek(0);
    sal_uInt32 nBytes;
    rStream >> nBytes;
    if( nBytes == 0x04034b50 )
    {
        rtl::OString aTmp = read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rStream);
        if (aTmp.matchL(RTL_CONSTASCII_STRINGPARAM("ContentURL=")))
        {
            aString = rtl::OStringToOUString(aTmp.copy(11), RTL_TEXTENCODING_UTF8);
        }
    }

    rStream.Seek( nPos );
    return aString;
}

String UCBStorage::CreateLinkFile( const String& rName )
{
    // create a stream to write the link file - use a temp file, because it may be no file content
    INetURLObject aFolderObj( rName );
    String aName = aFolderObj.GetName();
    aFolderObj.removeSegment();
    String aFolderURL( aFolderObj.GetMainURL( INetURLObject::NO_DECODE ) );
    ::utl::TempFile* pTempFile = new ::utl::TempFile( &aFolderURL );

    // get the stream from the temp file
    SvStream* pStream = pTempFile->GetStream( STREAM_STD_READWRITE | STREAM_TRUNC );

    // write header
    *pStream << ( sal_uInt32 ) 0x04034b50;

    // assemble a new folder name in the destination folder
    INetURLObject aObj( rName );
    String aTmpName = aObj.GetName();
    String aTitle = rtl::OUString( "content." );
    aTitle += aTmpName;

    // create a folder and store its URL
    Content aFolder( aFolderURL, Reference < XCommandEnvironment >() );
    Content aNewFolder;
    sal_Bool bRet = ::utl::UCBContentHelper::MakeFolder( aFolder, aTitle, aNewFolder );
    if ( !bRet )
    {
        aFolderObj.insertName( aTitle );
        if ( ::utl::UCBContentHelper::Exists( aFolderObj.GetMainURL( INetURLObject::NO_DECODE ) ) )
        {
            // Hack, because already existing files give the same CommandAbortedException as any other error !
            // append a number until the name can be used for a new folder
            aTitle += '.';
            for ( sal_Int32 i=0; !bRet; i++ )
            {
                String aTmp( aTitle );
                aTmp += String::CreateFromInt32( i );
                bRet = ::utl::UCBContentHelper::MakeFolder( aFolder, aTmp, aNewFolder );
                if ( bRet )
                    aTitle = aTmp;
                else
                {
                    aFolderObj.SetName( aTmp );
                    if ( !::utl::UCBContentHelper::Exists( aFolderObj.GetMainURL( INetURLObject::NO_DECODE ) ) )
                        // Hack, because already existing files give the same CommandAbortedException as any other error !
                        break;
                }
            }
        }
    }

    if ( bRet )
    {
        // get the URL
        aObj.SetName( aTitle );
        String aURL = aObj.GetMainURL( INetURLObject::NO_DECODE );

        // store it as key/value pair
        String aLink = rtl::OUString("ContentURL=");
        aLink += aURL;
        write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(*pStream, aLink, RTL_TEXTENCODING_UTF8);
        pStream->Flush();

        // move the stream to its desired location
        Content aSource( pTempFile->GetURL(), Reference < XCommandEnvironment >() );
        DELETEZ( pTempFile );
        aFolder.transferContent( aSource, InsertOperation_MOVE, aName, NameClash::OVERWRITE );
        return aURL;
    }

    pTempFile->EnableKillingFile( sal_True );
    delete pTempFile;
    return String();
}

sal_Bool UCBStorage::SetProperty( const String& rName, const ::com::sun::star::uno::Any& rValue )
{
    if ( rName.CompareToAscii("Title") == COMPARE_EQUAL )
        return sal_False;

    if ( rName.CompareToAscii("MediaType") == COMPARE_EQUAL )
    {
        ::rtl::OUString aTmp;
        rValue >>= aTmp;
        pImp->m_aContentType = aTmp;
    }

    try
    {
        if ( pImp->GetContent() )
        {
            pImp->m_pContent->setPropertyValue( rName, rValue );
            return sal_True;
        }
    }
    catch (const Exception&)
    {
    }

    return sal_False;
}

sal_Bool UCBStorage::GetProperty( const String& rName, ::com::sun::star::uno::Any& rValue )
{
    try
    {
        if ( pImp->GetContent() )
        {
            rValue = pImp->m_pContent->getPropertyValue( rName );
            return sal_True;
        }
    }
    catch (const Exception&)
    {
    }

    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
