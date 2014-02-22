/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <com/sun/star/packages/manifest/ManifestWriter.hpp>
#include <com/sun/star/packages/manifest/ManifestReader.hpp>
#include <com/sun/star/ucb/InteractiveIOException.hpp>

#include <boost/scoped_ptr.hpp>
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
#include <comphelper/classids.hxx>

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
    OUString        m_aURL;
    SvStream*       m_pSvStream;

public:
    FileStreamWrapper_Impl( const OUString& rName );
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


FileStreamWrapper_Impl::FileStreamWrapper_Impl( const OUString& rName )
    : m_aURL( rName )
    , m_pSvStream(0)
{
    
}


FileStreamWrapper_Impl::~FileStreamWrapper_Impl()
{
    if ( m_pSvStream )
    {
        delete m_pSvStream;
#if OSL_DEBUG_LEVEL > 1
        --nOpenFiles;
#endif
    }

    if ( !m_aURL.isEmpty() )
        ::utl::UCBContentHelper::Kill( m_aURL );
}


sal_Int32 SAL_CALL FileStreamWrapper_Impl::readBytes(Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
                throw( NotConnectedException, BufferSizeExceededException, RuntimeException )
{
    if ( m_aURL.isEmpty() )
    {
        aData.realloc( 0 );
        return 0;
    }

    checkConnected();

    if (nBytesToRead < 0)
        throw BufferSizeExceededException(OUString(),static_cast<XWeak*>(this));

    ::osl::MutexGuard aGuard( m_aMutex );

    aData.realloc(nBytesToRead);

    sal_uInt32 nRead = m_pSvStream->Read((void*)aData.getArray(), nBytesToRead);
    checkError();

    
    if (nRead < (sal_uInt32)nBytesToRead)
        aData.realloc( nRead );

    return nRead;
}


sal_Int32 SAL_CALL FileStreamWrapper_Impl::readSomeBytes(Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead) throw( NotConnectedException, BufferSizeExceededException, RuntimeException )
{
    if ( m_aURL.isEmpty() )
    {
        aData.realloc( 0 );
        return 0;
    }

    checkError();

    if (nMaxBytesToRead < 0)
        throw BufferSizeExceededException(OUString(),static_cast<XWeak*>(this));

    if (m_pSvStream->IsEof())
    {
        aData.realloc(0);
        return 0;
    }
    else
        return readBytes(aData, nMaxBytesToRead);
}


void SAL_CALL FileStreamWrapper_Impl::skipBytes(sal_Int32 nBytesToSkip) throw( NotConnectedException, BufferSizeExceededException, RuntimeException )
{
    if ( m_aURL.isEmpty() )
        return;

    ::osl::MutexGuard aGuard( m_aMutex );
    checkError();

    m_pSvStream->SeekRel(nBytesToSkip);
    checkError();
}


sal_Int32 SAL_CALL FileStreamWrapper_Impl::available() throw( NotConnectedException, RuntimeException )
{
    if ( m_aURL.isEmpty() )
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


void SAL_CALL FileStreamWrapper_Impl::closeInput() throw( NotConnectedException, RuntimeException )
{
    if ( m_aURL.isEmpty() )
        return;

    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();
    DELETEZ( m_pSvStream );
#if OSL_DEBUG_LEVEL > 1
    --nOpenFiles;
#endif
    ::utl::UCBContentHelper::Kill( m_aURL );
    m_aURL = "";
}


void SAL_CALL FileStreamWrapper_Impl::seek( sal_Int64 _nLocation ) throw (IllegalArgumentException, IOException, RuntimeException)
{
    if ( m_aURL.isEmpty() )
        return;

    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    m_pSvStream->Seek((sal_uInt32)_nLocation);
    checkError();
}


sal_Int64 SAL_CALL FileStreamWrapper_Impl::getPosition(  ) throw (IOException, RuntimeException)
{
    if ( m_aURL.isEmpty() )
        return 0;

    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    sal_uInt32 nPos = m_pSvStream->Tell();
    checkError();
    return (sal_Int64)nPos;
}


sal_Int64 SAL_CALL FileStreamWrapper_Impl::getLength(  ) throw (IOException, RuntimeException)
{
    if ( m_aURL.isEmpty() )
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


void FileStreamWrapper_Impl::checkConnected()
{
    if ( m_aURL.isEmpty() )
        throw NotConnectedException(OUString(), const_cast<XWeak*>(static_cast<const XWeak*>(this)));
    if ( !m_pSvStream )
    {
        m_pSvStream = ::utl::UcbStreamHelper::CreateStream( m_aURL, STREAM_STD_READ );
#if OSL_DEBUG_LEVEL > 1
        ++nOpenFiles;
#endif
    }
}


void FileStreamWrapper_Impl::checkError()
{
    checkConnected();

    if (m_pSvStream->SvStream::GetError() != ERRCODE_NONE)
        
        throw NotConnectedException(OUString(), const_cast<XWeak*>(static_cast<const XWeak*>(this)));
}

TYPEINIT1( UCBStorageStream, BaseStorageStream );
TYPEINIT1( UCBStorage, BaseStorage );

#define COMMIT_RESULT_FAILURE           0
#define COMMIT_RESULT_NOTHING_TO_DO     1
#define COMMIT_RESULT_SUCCESS           2

#define min( x, y ) (( x < y ) ? x : y)

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




enum RepresentModes {
        nonset,
        svstream,
        xinputstream
};

class UCBStorageStream_Impl : public SvRefBase, public SvStream
{
                                ~UCBStorageStream_Impl();
public:

    virtual sal_uLong           GetData( void* pData, sal_uLong nSize );
    virtual sal_uLong           PutData( const void* pData, sal_uLong nSize );
    virtual sal_uLong           SeekPos( sal_uLong nPos );
    virtual void                SetSize( sal_uLong nSize );
    virtual void                FlushData();
    virtual void                ResetError();

    UCBStorageStream*           m_pAntiImpl;    

    OUString                    m_aOriginalName;
    OUString                    m_aName;        
    OUString                    m_aURL;         
    OUString                    m_aContentType;
    OUString                    m_aOriginalContentType;
    OString                     m_aKey;
    ::ucbhelper::Content*       m_pContent;     
    Reference<XInputStream>     m_rSource;      
    SvStream*                   m_pStream;      
                                                
    OUString                    m_aTempURL;     
    RepresentModes              m_nRepresentMode; 
    long                        m_nError;
    StreamMode                  m_nMode;        
    bool                        m_bSourceRead;  
    bool                        m_bModified;    
    bool                        m_bCommited;    
    bool                        m_bDirect;      
                                                
                                                
    bool                        m_bIsOLEStorage;

                                UCBStorageStream_Impl( const OUString&, StreamMode, UCBStorageStream*, bool, const OString* pKey=0,
                                                       bool bRepair = false, Reference< XProgressHandler > xProgress = Reference< XProgressHandler >() );

    void                        Free();
    bool                        Init();
    bool                        Clear();
    sal_Int16                   Commit();       
    bool                        Revert();       
    BaseStorage*                CreateStorage();
    sal_uLong                   GetSize();

    sal_uLong                   ReadSourceWriteTemporary( sal_uLong aLength ); 
                                                                           
    sal_uLong                   ReadSourceWriteTemporary();                

    sal_uLong                   CopySourceToTemporary();                
                                                                        
                                                                        
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
    UCBStorage*                 m_pAntiImpl;    

    OUString                    m_aOriginalName;
    OUString                    m_aName;        
    OUString                    m_aURL;         
    OUString                    m_aContentType;
    OUString                    m_aOriginalContentType;
    ::ucbhelper::Content*       m_pContent;     
    ::utl::TempFile*            m_pTempFile;    
    SvStream*                   m_pSource;      
    long                        m_nError;
    StreamMode                  m_nMode;        
    bool                        m_bModified;    
    bool                        m_bCommited;    
    bool                        m_bDirect;      
                                                
                                                
    bool                        m_bIsRoot;      
    bool                        m_bDirty;           
    bool                        m_bIsLinked;
    bool                        m_bListCreated;
    sal_uLong                   m_nFormat;
    OUString                    m_aUserTypeName;
    SvGlobalName                m_aClassId;

    UCBStorageElementList_Impl  m_aChildrenList;

    bool                        m_bRepairPackage;
    Reference< XProgressHandler > m_xProgressHandler;

                                UCBStorage_Impl( const ::ucbhelper::Content&, const OUString&, StreamMode, UCBStorage*, bool,
                                                 bool, bool = false, Reference< XProgressHandler > = Reference< XProgressHandler >() );
                                UCBStorage_Impl( const OUString&, StreamMode, UCBStorage*, bool, bool,
                                                 bool = false, Reference< XProgressHandler > = Reference< XProgressHandler >() );
                                UCBStorage_Impl( SvStream&, UCBStorage*, bool );
    void                        Init();
    sal_Int16                   Commit();
    bool                        Revert();
    bool                        Insert( ::ucbhelper::Content *pContent );
    UCBStorage_Impl*            OpenStorage( UCBStorageElement_Impl* pElement, StreamMode nMode, bool bDirect );
    UCBStorageStream_Impl*      OpenStream( UCBStorageElement_Impl*, StreamMode, bool, const OString* pKey=0 );
    void                        SetProps( const Sequence < Sequence < PropertyValue > >& rSequence, const OUString& );
    void                        GetProps( sal_Int32&, Sequence < Sequence < PropertyValue > >& rSequence, const OUString& );
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


struct UCBStorageElement_Impl
{
    OUString                    m_aName;        
    OUString                    m_aOriginalName;
    sal_uLong                   m_nSize;
    bool                        m_bIsFolder;    
    bool                        m_bIsStorage;   
    bool                        m_bIsRemoved;   
    bool                        m_bIsInserted;  
    UCBStorage_ImplRef          m_xStorage;     
    UCBStorageStream_ImplRef    m_xStream;      

                                UCBStorageElement_Impl( const OUString& rName,
                                                        bool bIsFolder = false, sal_uLong nSize = 0 )
                                    : m_aName( rName )
                                    , m_aOriginalName( rName )
                                    , m_nSize( nSize )
                                    , m_bIsFolder( bIsFolder )
                                    , m_bIsStorage( bIsFolder )
                                    , m_bIsRemoved( false )
                                    , m_bIsInserted( false )
                                {
                                }

    ::ucbhelper::Content*       GetContent();
    bool                        IsModified();
    OUString                    GetContentType();
    void                        SetContentType( const OUString& );
    OUString                    GetOriginalContentType();
    bool                        IsLoaded()
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

OUString UCBStorageElement_Impl::GetContentType()
{
    if ( m_xStream.Is() )
        return m_xStream->m_aContentType;
    else if ( m_xStorage.Is() )
        return m_xStorage->m_aContentType;
    else
    {
        OSL_FAIL("Element not loaded!");
        return OUString();
    }
}

void UCBStorageElement_Impl::SetContentType( const OUString& rType )
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

OUString UCBStorageElement_Impl::GetOriginalContentType()
{
    if ( m_xStream.Is() )
        return m_xStream->m_aOriginalContentType;
    else if ( m_xStorage.Is() )
        return m_xStorage->m_aOriginalContentType;
    else
        return OUString();
}

bool UCBStorageElement_Impl::IsModified()
{
    bool bModified = m_bIsRemoved || m_bIsInserted || m_aName != m_aOriginalName;
    if ( bModified )
    {
        if ( m_xStream.Is() )
            bModified = m_xStream->m_aContentType != m_xStream->m_aOriginalContentType;
        else if ( m_xStorage.Is() )
            bModified = m_xStorage->m_aContentType != m_xStorage->m_aOriginalContentType;
    }

    return bModified;
}

UCBStorageStream_Impl::UCBStorageStream_Impl( const OUString& rName, StreamMode nMode, UCBStorageStream* pStream, bool bDirect, const OString* pKey, bool bRepair, Reference< XProgressHandler > xProgress  )
    : m_pAntiImpl( pStream )
    , m_aURL( rName )
    , m_pContent( NULL )
    , m_pStream( NULL )
    , m_nRepresentMode( nonset )
    , m_nError( 0 )
    , m_nMode( nMode )
    , m_bSourceRead( !( nMode & STREAM_TRUNC ) )
    , m_bModified( false )
    , m_bCommited( false )
    , m_bDirect( bDirect )
    , m_bIsOLEStorage( false )
{
    
    INetURLObject aObj( rName );
    m_aName = m_aOriginalName = aObj.GetLastName();
    try
    {
        
        Reference< ::com::sun::star::ucb::XCommandEnvironment > xComEnv;

        OUString aTemp( rName );

        if ( bRepair )
        {
            xComEnv = new ::ucbhelper::CommandEnvironment( Reference< ::com::sun::star::task::XInteractionHandler >(),
                                                     xProgress );
            aTemp += "?repairpackage";
        }

        m_pContent = new ::ucbhelper::Content( aTemp, xComEnv, comphelper::getProcessComponentContext() );

        if ( pKey )
        {
            m_aKey = *pKey;

            
            sal_uInt8 aBuffer[RTL_DIGEST_LENGTH_SHA1];
            rtlDigestError nErr = rtl_digest_SHA1( pKey->getStr(), pKey->getLength(), aBuffer, RTL_DIGEST_LENGTH_SHA1 );
            if ( nErr == rtl_Digest_E_None )
            {
                sal_uInt8* pBuffer = aBuffer;
                ::com::sun::star::uno::Sequence < sal_Int8 > aSequ( (sal_Int8*) pBuffer, RTL_DIGEST_LENGTH_SHA1 );
                ::com::sun::star::uno::Any aAny;
                aAny <<= aSequ;
                m_pContent->setPropertyValue("EncryptionKey", aAny );
            }
        }
    }
    catch (const ContentCreationException&)
    {
        
        SetError( SVSTREAM_CANNOT_MAKE );
    }
    catch (const RuntimeException&)
    {
        
        SetError( ERRCODE_IO_GENERAL );
    }
}

UCBStorageStream_Impl::~UCBStorageStream_Impl()
{
    if( m_rSource.is() )
        m_rSource.clear();

    if( m_pStream )
        delete m_pStream;

    if ( !m_aTempURL.isEmpty() )
        ::utl::UCBContentHelper::Kill( m_aTempURL );

    if( m_pContent )
        delete m_pContent;
}


bool UCBStorageStream_Impl::Init()
{
    if( m_nRepresentMode == xinputstream )
    {
        OSL_FAIL( "XInputStream misuse!" );
        SetError( ERRCODE_IO_ACCESSDENIED );
        return false;
    }

    if( !m_pStream )
    {
        
        

        m_nRepresentMode = svstream; 

        if ( m_aTempURL.isEmpty() )
            m_aTempURL = ::utl::TempFile().GetURL();

        m_pStream = ::utl::UcbStreamHelper::CreateStream( m_aTempURL, STREAM_STD_READWRITE, true /* bFileExists */ );
#if OSL_DEBUG_LEVEL > 1
        ++nOpenFiles;
#endif

        if( !m_pStream )
        {
            OSL_FAIL( "Suspicious temporary stream creation!" );
            SetError( SVSTREAM_CANNOT_MAKE );
            return false;
        }

        SetError( m_pStream->GetError() );
    }

    if( m_bSourceRead && !m_rSource.is() )
    {
        
        

        try
        {
            m_rSource = m_pContent->openStream();
        }
        catch (const Exception&)
        {
            
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
                
                m_bSourceRead = false;
            }
            catch (const Exception&)
            {
                
                m_bSourceRead = false;
                OSL_FAIL( "Can not operate original stream!" );
                SetError( SVSTREAM_CANNOT_MAKE );
            }

            m_pStream->Seek( 0 );
        }
        else
        {
            
            m_bSourceRead = false;
                
        }
    }

    DBG_ASSERT( m_rSource.is() || !m_bSourceRead, "Unreadable source stream!" );

    return true;
}

sal_uLong UCBStorageStream_Impl::ReadSourceWriteTemporary()
{
    
    

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
            OSL_FAIL( OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
            (void)e;
        }
    }

    m_bSourceRead = false;

    return aResult;

}

sal_uLong UCBStorageStream_Impl::ReadSourceWriteTemporary( sal_uLong aLength )
{
    
    

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
                m_bSourceRead = false;
        }
        catch( const Exception & e )
        {
            OSL_FAIL( OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
            (void)e;
        }
    }

    return aResult;
}

sal_uLong UCBStorageStream_Impl::CopySourceToTemporary()
{
    
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



sal_uLong UCBStorageStream_Impl::GetData( void* pData, sal_uLong nSize )
{
    sal_uLong aResult = 0;

    if( !Init() )
        return 0;


    
    aResult = m_pStream->Read( pData, nSize );
    if( m_bSourceRead && aResult < nSize )
    {
        
        

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
            OSL_FAIL( OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
            (void)e;
        }

        if( aResult < nSize )
            m_bSourceRead = false;
    }

    return aResult;
}

sal_uLong UCBStorageStream_Impl::PutData( const void* pData, sal_uLong nSize )
{
    if ( !(m_nMode & STREAM_WRITE) )
    {
        SetError( ERRCODE_IO_ACCESSDENIED );
        return 0; 
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
        
        
        

        if( m_pStream->Tell() > nPos
            || m_pStream->Seek( STREAM_SEEK_TO_END ) > nPos )
        {
            
            aResult = m_pStream->Seek( nPos );
        }
        else
        {
            
            aResult = m_pStream->Tell();

            if( aResult < nPos )
            {
                if( m_bSourceRead )
                {
                    aResult += ReadSourceWriteTemporary( nPos - aResult );
                    if( aResult < nPos )
                        m_bSourceRead = false;

                    DBG_ASSERT( aResult == m_pStream->Tell(), "Error in stream arithmetic!\n" );
                }

                if( (m_nMode & STREAM_WRITE) && !m_bSourceRead && aResult < nPos )
                {
                    
                    
                    
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

    m_bModified = true;

    if( m_bSourceRead )
    {
        sal_uLong aPos = m_pStream->Tell();
        m_pStream->Seek( STREAM_SEEK_TO_END );
        if( m_pStream->Tell() < nSize )
            ReadSourceWriteTemporary( nSize - m_pStream->Tell() );
        m_pStream->Seek( aPos );
    }

    m_pStream->SetStreamSize( nSize );
    m_bSourceRead = false;
}

void  UCBStorageStream_Impl::FlushData()
{
    if( m_pStream )
    {
        CopySourceToTemporary();
        m_pStream->Flush();
    }

    m_bCommited = true;
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
    
    
    UCBStorageStream* pNewStorageStream = new UCBStorageStream( this );
    Storage *pStorage = new Storage( *pNewStorageStream, m_bDirect );

    
    long nTmpErr = pStorage->GetError();
    pStorage->SetError( nTmpErr );

    m_bIsOLEStorage = !nTmpErr;
    return static_cast< BaseStorage* > ( pStorage );
}

sal_Int16 UCBStorageStream_Impl::Commit()
{
    
    
    if ( m_bCommited || m_bIsOLEStorage || m_bDirect )
    {
        
        

        if ( m_bModified )
        {
            try
            {
                CopySourceToTemporary();

                
                Free();

                
                DBG_ASSERT( !m_aTempURL.isEmpty() || ( m_nMode & STREAM_TRUNC ), "No temporary file to read from!");
                if ( m_aTempURL.isEmpty() && !( m_nMode & STREAM_TRUNC ) )
                    throw RuntimeException();

                
                Reference < XInputStream > xStream = new FileStreamWrapper_Impl( m_aTempURL );

                Any aAny;
                InsertCommandArgument aArg;
                aArg.Data = xStream;
                aArg.ReplaceExisting = true;
                aAny <<= aArg;
                m_pContent->executeCommand( OUString("insert"), aAny );

                
                m_aTempURL = "";

                INetURLObject aObj( m_aURL );
                aObj.SetName( m_aName );
                m_aURL = aObj.GetMainURL( INetURLObject::NO_DECODE );
                m_bModified = false;
                m_bSourceRead = true;
            }
            catch (const CommandAbortedException&)
            {
                
                SetError( ERRCODE_IO_GENERAL );
                return COMMIT_RESULT_FAILURE;
            }
            catch (const RuntimeException&)
            {
                
                SetError( ERRCODE_IO_GENERAL );
                return COMMIT_RESULT_FAILURE;
            }
            catch (const Exception&)
            {
                
                SetError( ERRCODE_IO_GENERAL );
                return COMMIT_RESULT_FAILURE;
            }

            m_bCommited = false;
            return COMMIT_RESULT_SUCCESS;
        }
    }

    return COMMIT_RESULT_NOTHING_TO_DO;
}

bool UCBStorageStream_Impl::Revert()
{
    
    if ( m_bCommited )
    {
        OSL_FAIL("Revert while commit is in progress!" );
        return false;                   
    }

    Free();
    if ( !m_aTempURL.isEmpty() )
    {
        ::utl::UCBContentHelper::Kill( m_aTempURL );
        m_aTempURL = "";
    }

    m_bSourceRead = false;
    try
    {
        m_rSource = m_pContent->openStream();
        if( m_rSource.is() )
        {
            if ( m_pAntiImpl && ( m_nMode & STREAM_TRUNC ) )
                
                m_bSourceRead = false;
            else
            {
                m_nMode &= ~STREAM_TRUNC;
                m_bSourceRead = true;
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

    m_bModified = false;
    m_aName = m_aOriginalName;
    m_aContentType = m_aOriginalContentType;
    return ( GetError() == ERRCODE_NONE );
}

bool UCBStorageStream_Impl::Clear()
{
    bool bRet = ( m_pAntiImpl == NULL );
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
        if ( !m_aTempURL.isEmpty() )
            --nOpenFiles;
        else
            --nOpenStreams;
    }
#endif

    m_nRepresentMode = nonset;
    m_rSource.clear();
    DELETEZ( m_pStream );
}

void UCBStorageStream_Impl::PrepareCachedForReopen( StreamMode nMode )
{
    bool isWritable = (( m_nMode & STREAM_WRITE ) != 0 );
    if ( isWritable )
    {
        
        nMode |= STREAM_WRITE;
    }

    m_nMode = nMode;
    Free();

    if ( nMode & STREAM_TRUNC )
    {
        m_bSourceRead = false; 

        if ( !m_aTempURL.isEmpty() )
        {
            ::utl::UCBContentHelper::Kill( m_aTempURL );
            m_aTempURL = "";
        }
    }
}

UCBStorageStream::UCBStorageStream( const OUString& rName, StreamMode nMode, bool bDirect, const OString* pKey, bool bRepair, Reference< XProgressHandler > xProgress )
{
    
    
    pImp = new UCBStorageStream_Impl( rName, nMode, this, bDirect, pKey, bRepair, xProgress );
    pImp->AddRef();             
    StorageBase::m_nMode = pImp->m_nMode;
}

UCBStorageStream::UCBStorageStream( UCBStorageStream_Impl *pImpl )
    : pImp( pImpl )
{
    pImp->AddRef();             
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
    
    return pImp->GetData( pData, nSize );
}

sal_uLong UCBStorageStream::Write( const void* pData, sal_uLong nSize )
{
    return pImp->PutData( pData, nSize );
}

sal_uLong UCBStorageStream::Seek( sal_uLong nPos )
{
    
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
    
    Commit();
}

bool UCBStorageStream::SetSize( sal_uLong nNewSize )
{
    pImp->SetSize( nNewSize );
    return !pImp->GetError();
}

bool UCBStorageStream::Validate( bool bWrite ) const
{
    return ( !bWrite || ( pImp->m_nMode & STREAM_WRITE ) );
}

bool UCBStorageStream::ValidateMode( StreamMode m ) const
{
    
    if( m == ( STREAM_READ | STREAM_TRUNC ) )  
        return true;
    sal_uInt16 nCurMode = 0xFFFF;
    if( ( m & 3 ) == STREAM_READ )
    {
        
        if( ( ( m & STREAM_SHARE_DENYWRITE )
           && ( nCurMode & STREAM_SHARE_DENYWRITE ) )
         || ( ( m & STREAM_SHARE_DENYALL )
           && ( nCurMode & STREAM_SHARE_DENYALL ) ) )
            return true;
    }
    else
    {
        
        
        
        if( ( m & STREAM_SHARE_DENYALL )
         && ( nCurMode & STREAM_SHARE_DENYALL ) )
            return true;
    }

    return true;
}

const SvStream* UCBStorageStream::GetSvStream() const
{
    if( !pImp->Init() )
        return NULL;

    pImp->CopySourceToTemporary();
    return pImp->m_pStream; 
}

SvStream* UCBStorageStream::GetModifySvStream()
{
    return (SvStream*)pImp;
}

bool  UCBStorageStream::Equals( const BaseStorageStream& rStream ) const
{
    
    return ((BaseStorageStream*) this ) == &rStream;
}

bool UCBStorageStream::Commit()
{
    
    pImp->FlushData();
    return true;
}

bool UCBStorageStream::Revert()
{
    return pImp->Revert();
}

bool UCBStorageStream::CopyTo( BaseStorageStream* pDestStm )
{
    if( !pImp->Init() )
        return false;

    UCBStorageStream* pStg = PTR_CAST( UCBStorageStream, pDestStm );
    if ( pStg )
        pStg->pImp->m_aContentType = pImp->m_aContentType;

    pDestStm->SetSize( 0 );
    Seek( STREAM_SEEK_TO_END );
    sal_Int32 n = Tell();
    if( n < 0 )
        return false;

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

    return true;
}

bool UCBStorageStream::SetProperty( const OUString& rName, const ::com::sun::star::uno::Any& rValue )
{
    if ( rName == "Title")
        return false;

    if ( rName == "MediaType")
    {
        OUString aTmp;
        rValue >>= aTmp;
        pImp->m_aContentType = aTmp;
    }

    try
    {
        if ( pImp->m_pContent )
        {
            pImp->m_pContent->setPropertyValue( rName, rValue );
            return true;
        }
    }
    catch (const Exception&)
    {
    }

    return false;
}

sal_uLong UCBStorageStream::GetSize() const
{
    return pImp->GetSize();
}

UCBStorage::UCBStorage( SvStream& rStrm, bool bDirect )
{
    OUString aURL = GetLinkedFile( rStrm );
    if ( !aURL.isEmpty() )
    {
        StreamMode nMode = STREAM_READ;
        if( rStrm.IsWritable() )
            nMode = STREAM_READ | STREAM_WRITE;

        ::ucbhelper::Content aContent( aURL, Reference < XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        pImp = new UCBStorage_Impl( aContent, aURL, nMode, this, bDirect, true );
    }
    else
    {
        
        
        pImp = new UCBStorage_Impl( rStrm, this, bDirect );
    }

    pImp->AddRef();
    pImp->Init();
    StorageBase::m_nMode = pImp->m_nMode;
}

UCBStorage::UCBStorage( const ::ucbhelper::Content& rContent, const OUString& rName, StreamMode nMode, bool bDirect, bool bIsRoot )
{
    
    
    pImp = new UCBStorage_Impl( rContent, rName, nMode, this, bDirect, bIsRoot );
    pImp->AddRef();
    pImp->Init();
    StorageBase::m_nMode = pImp->m_nMode;
}

UCBStorage::UCBStorage( const OUString& rName, StreamMode nMode, bool bDirect, bool bIsRoot, bool bIsRepair, Reference< XProgressHandler > xProgressHandler )
{
    
    
    pImp = new UCBStorage_Impl( rName, nMode, this, bDirect, bIsRoot, bIsRepair, xProgressHandler );
    pImp->AddRef();
    pImp->Init();
    StorageBase::m_nMode = pImp->m_nMode;
}

UCBStorage::UCBStorage( const OUString& rName, StreamMode nMode, bool bDirect, bool bIsRoot )
{
    
    
    pImp = new UCBStorage_Impl( rName, nMode, this, bDirect, bIsRoot, false, Reference< XProgressHandler >() );
    pImp->AddRef();
    pImp->Init();
    StorageBase::m_nMode = pImp->m_nMode;
}

UCBStorage::UCBStorage( UCBStorage_Impl *pImpl )
    : pImp( pImpl )
{
    pImp->m_pAntiImpl = this;
    SetError( pImp->m_nError );
    pImp->AddRef();             
    StorageBase::m_nMode = pImp->m_nMode;
}

UCBStorage::~UCBStorage()
{
    if ( pImp->m_bIsRoot && pImp->m_bDirect && ( !pImp->m_pTempFile || pImp->m_pSource ) )
        
        Commit();

    pImp->m_pAntiImpl = NULL;
    pImp->ReleaseRef();
}

UCBStorage_Impl::UCBStorage_Impl( const ::ucbhelper::Content& rContent, const OUString& rName, StreamMode nMode, UCBStorage* pStorage, bool bDirect, bool bIsRoot, bool bIsRepair, Reference< XProgressHandler > xProgressHandler  )
    : m_pAntiImpl( pStorage )
    , m_pContent( new ::ucbhelper::Content( rContent ) )
    , m_pTempFile( NULL )
    , m_pSource( NULL )
    
    , m_nError( 0 )
    , m_nMode( nMode )
    , m_bModified( false )
    , m_bCommited( false )
    , m_bDirect( bDirect )
    , m_bIsRoot( bIsRoot )
    , m_bDirty( false )
    , m_bIsLinked( true )
    , m_bListCreated( false )
    , m_nFormat( 0 )
    , m_aClassId( SvGlobalName() )
    , m_bRepairPackage( bIsRepair )
    , m_xProgressHandler( xProgressHandler )
{
    OUString aName( rName );
    if( aName.isEmpty() )
    {
        
        DBG_ASSERT( m_bIsRoot, "SubStorage must have a name!" );
        m_pTempFile = new ::utl::TempFile;
        m_pTempFile->EnableKillingFile( true );
        m_aName = m_aOriginalName = aName = m_pTempFile->GetURL();
    }

    m_aURL = rName;
}

UCBStorage_Impl::UCBStorage_Impl( const OUString& rName, StreamMode nMode, UCBStorage* pStorage, bool bDirect, bool bIsRoot, bool bIsRepair, Reference< XProgressHandler > xProgressHandler )
    : m_pAntiImpl( pStorage )
    , m_pContent( NULL )
    , m_pTempFile( NULL )
    , m_pSource( NULL )
    
    , m_nError( 0 )
    , m_nMode( nMode )
    , m_bModified( false )
    , m_bCommited( false )
    , m_bDirect( bDirect )
    , m_bIsRoot( bIsRoot )
    , m_bDirty( false )
    , m_bIsLinked( false )
    , m_bListCreated( false )
    , m_nFormat( 0 )
    , m_aClassId( SvGlobalName() )
    , m_bRepairPackage( bIsRepair )
    , m_xProgressHandler( xProgressHandler )
{
    OUString aName( rName );
    if( aName.isEmpty() )
    {
        
        DBG_ASSERT( m_bIsRoot, "SubStorage must have a name!" );
        m_pTempFile = new ::utl::TempFile;
        m_pTempFile->EnableKillingFile( true );
        m_aName = m_aOriginalName = aName = m_pTempFile->GetURL();
    }

    if ( m_bIsRoot )
    {
        
        OUString aTemp = "vnd.sun.star.pkg:
        aTemp += INetURLObject::encode( aName, INetURLObject::PART_AUTHORITY, '%', INetURLObject::ENCODE_ALL );
        m_aURL = aTemp;

        if ( m_nMode & STREAM_WRITE )
        {
            
            SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( aName, STREAM_STD_READWRITE, m_pTempFile != 0 /* bFileExists */ );
            delete pStream;
        }
    }
    else
    {
        
        m_aURL = rName;
        if ( !m_aURL.startsWith( "vnd.sun.star.pkg:
            m_bIsLinked = true;
    }
}

UCBStorage_Impl::UCBStorage_Impl( SvStream& rStream, UCBStorage* pStorage, bool bDirect )
    : m_pAntiImpl( pStorage )
    , m_pContent( NULL )
    , m_pTempFile( new ::utl::TempFile )
    , m_pSource( &rStream )
    , m_nError( 0 )
    , m_bModified( false )
    , m_bCommited( false )
    , m_bDirect( bDirect )
    , m_bIsRoot( true )
    , m_bDirty( false )
    , m_bIsLinked( false )
    , m_bListCreated( false )
    , m_nFormat( 0 )
    , m_aClassId( SvGlobalName() )
    , m_bRepairPackage( false )
{
    
    
    m_pTempFile->EnableKillingFile( true );
    DBG_ASSERT( !bDirect, "Storage on a stream must not be opened in direct mode!" );

    
    
    
    OUString aTemp = "vnd.sun.star.pkg:
    aTemp += INetURLObject::encode( m_pTempFile->GetURL(), INetURLObject::PART_AUTHORITY, '%', INetURLObject::ENCODE_ALL );
    m_aURL = aTemp;

    
    SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( m_pTempFile->GetURL(), STREAM_STD_READWRITE, true /* bFileExists */ );
    if ( pStream )
    {
        rStream.Seek(0);
        rStream.ReadStream( *pStream );
        pStream->Flush();
        DELETEZ( pStream );
    }

    
    m_pSource->Seek(0);

    
    m_nMode = STREAM_READ;
    if( rStream.IsWritable() )
        m_nMode = STREAM_READ | STREAM_WRITE;
}

void UCBStorage_Impl::Init()
{
    
    INetURLObject aObj( m_aURL );
    if ( m_aName.isEmpty() )
        
        m_aName = m_aOriginalName = aObj.GetLastName();

    
    if ( !m_pContent && !( m_nMode & STORAGE_DISKSPANNED_MODE ) )
        CreateContent();

    if ( m_nMode & STORAGE_DISKSPANNED_MODE )
    {
        
        
        m_aContentType = m_aOriginalContentType = "application/vnd.sun.xml.impress";
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
                    
                    aObj.Append( OUString( "META-INF" ) );
                    aObj.Append( OUString( "manifest.xml" ) );

                    
                    SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( aObj.GetMainURL( INetURLObject::NO_DECODE ), STREAM_STD_READ );
                    
                    if ( pStream )
                    {
                        if ( !pStream->GetError() )
                        {
                            ::utl::OInputStreamWrapper* pHelper = new ::utl::OInputStreamWrapper( *pStream );
                            com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream > xInputStream( pHelper );

                            
                            Reference < ::com::sun::star::packages::manifest::XManifestReader > xReader =
                                ::com::sun::star::packages::manifest::ManifestReader::create(
                                    ::comphelper::getProcessComponentContext() ) ;
                            Sequence < Sequence < PropertyValue > > aProps = xReader->readManifestSequence( xInputStream );

                            
                            xReader = NULL;
                            xInputStream = NULL;
                            SetProps( aProps, OUString() );
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
            
            try {
                Any aAny = m_pContent->getPropertyValue("MediaType");
                OUString aTmp;
                if ( ( aAny >>= aTmp ) && !aTmp.isEmpty() )
                    m_aContentType = m_aOriginalContentType = aTmp;
            }
            catch (const Exception&)
            {
                DBG_ASSERT( false,
                            "getPropertyValue has thrown an exception! Please let developers know the scenario!" );
            }
        }
    }

    if ( !m_aContentType.isEmpty() )
    {
        
        ::com::sun::star::datatransfer::DataFlavor aDataFlavor;
        aDataFlavor.MimeType = m_aContentType;
        m_nFormat = SotExchange::GetFormat( aDataFlavor );

        
        m_aClassId = GetClassId_Impl( m_nFormat );

        
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
        
        Reference< ::com::sun::star::ucb::XCommandEnvironment > xComEnv;

        OUString aTemp( m_aURL );

        if ( m_bRepairPackage )
        {
            xComEnv = new ::ucbhelper::CommandEnvironment( Reference< ::com::sun::star::task::XInteractionHandler >(),
                                                     m_xProgressHandler );
            aTemp += "?repairpackage";
        }

        m_pContent = new ::ucbhelper::Content( aTemp, xComEnv, comphelper::getProcessComponentContext() );
    }
    catch (const ContentCreationException&)
    {
        
        SetError( SVSTREAM_CANNOT_MAKE );
    }
    catch (const RuntimeException&)
    {
        
        SetError( SVSTREAM_CANNOT_MAKE );
    }
}

void UCBStorage_Impl::ReadContent()
{
   if ( m_bListCreated )
        return;

    m_bListCreated = true;

    
    Sequence< OUString > aProps(4);
    aProps[0] = "Title";
    aProps[1] = "IsFolder";
    aProps[2] = "MediaType";
    aProps[3] = "Size";
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
                
                OUString aTitle( xRow->getString(1) );
                OUString aContentType;
                if ( m_bIsLinked )
                {
                    
                    if ( aTitle == "META-INF" )
                        continue;
                }
                else
                {
                    aContentType = xRow->getString(3);
                }

                bool bIsFolder( xRow->getBoolean(2) );
                sal_Int64 nSize = xRow->getLong(4);
                UCBStorageElement_Impl* pElement = new UCBStorageElement_Impl( aTitle, bIsFolder, (sal_uLong) nSize );
                m_aChildrenList.push_back( pElement );

                bool bIsOfficeDocument = m_bIsLinked || ( m_aClassId != SvGlobalName() );
                if ( bIsFolder )
                {
                    if ( m_bIsLinked )
                        OpenStorage( pElement, m_nMode, m_bDirect );
                    if ( pElement->m_xStorage.Is() )
                        pElement->m_xStorage->Init();
                }
                else if ( bIsOfficeDocument )
                {
                    
                    OUString aName( m_aURL + "/" + xRow->getString(1));

                    Reference< ::com::sun::star::ucb::XCommandEnvironment > xComEnv;
                    if ( m_bRepairPackage )
                    {
                        xComEnv = new ::ucbhelper::CommandEnvironment( Reference< ::com::sun::star::task::XInteractionHandler >(),
                                                                m_xProgressHandler );
                        aName += "?repairpackage";
                    }

                    ::ucbhelper::Content aContent( aName, xComEnv, comphelper::getProcessComponentContext() );

                    OUString aMediaType;
                    Any aAny = aContent.getPropertyValue("MediaType");
                    if ( ( aAny >>= aMediaType ) && ( aMediaType == "application/vnd.sun.star.oleobject" ) )
                        pElement->m_bIsStorage = true;
                    else if ( aMediaType.isEmpty() )
                    {
                        
                        OpenStream( pElement, STREAM_STD_READ, m_bDirect );
                        if ( Storage::IsStorageFile( pElement->m_xStream ) )
                            pElement->m_bIsStorage = true;
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
        
        if ( !( m_nMode & STREAM_WRITE ) )
            
            SetError( ERRCODE_IO_GENERAL );
    }
    catch (const RuntimeException&)
    {
        
        SetError( ERRCODE_IO_GENERAL );
    }
    catch (const ResultSetException&)
    {
        
        SetError( ERRCODE_IO_BROKENPACKAGE );
    }
    catch (const SQLException&)
    {
        
        SetError( ERRCODE_IO_WRONGFORMAT );
    }
    catch (const Exception&)
    {
        
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

OUString Find_Impl( const Sequence < Sequence < PropertyValue > >& rSequence, const OUString& rPath )
{
    bool bFound = false;
    for ( sal_Int32 nSeqs=0; nSeqs<rSequence.getLength(); nSeqs++ )
    {
        const Sequence < PropertyValue >& rMyProps = rSequence[nSeqs];
        OUString aType;

        for ( sal_Int32 nProps=0; nProps<rMyProps.getLength(); nProps++ )
        {
            const PropertyValue& rAny = rMyProps[nProps];
            if ( rAny.Name == "FullPath" )
            {
                OUString aTmp;
                if ( ( rAny.Value >>= aTmp ) && aTmp == rPath )
                    bFound = true;
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

    return OUString();
}

void UCBStorage_Impl::SetProps( const Sequence < Sequence < PropertyValue > >& rSequence, const OUString& rPath )
{
    OUString aPath( rPath );
    if ( !m_bIsRoot )
        aPath += m_aName;
    aPath += "/";

    m_aContentType = m_aOriginalContentType = Find_Impl( rSequence, aPath );

    if ( m_bIsRoot )
        
        aPath = "";

    for ( size_t i = 0; i < m_aChildrenList.size(); ++i )
    {
        UCBStorageElement_Impl* pElement = m_aChildrenList[ i ];
        DBG_ASSERT( !pElement->m_bIsFolder || pElement->m_xStorage.Is(), "Storage should be open!" );
        if ( pElement->m_bIsFolder && pElement->m_xStorage.Is() )
            pElement->m_xStorage->SetProps( rSequence, aPath );
        else
        {
            OUString aElementPath( aPath );
            aElementPath += pElement->m_aName;
            pElement->SetContentType( Find_Impl( rSequence, aElementPath ) );
        }
    }

    if ( !m_aContentType.isEmpty() )
    {
        
        ::com::sun::star::datatransfer::DataFlavor aDataFlavor;
        aDataFlavor.MimeType = m_aContentType;
        m_nFormat = SotExchange::GetFormat( aDataFlavor );

        
        m_aClassId = GetClassId_Impl( m_nFormat );

        
        SotExchange::GetFormatDataFlavor( m_nFormat, aDataFlavor );
        m_aUserTypeName = aDataFlavor.HumanPresentableName;
    }
}

void UCBStorage_Impl::GetProps( sal_Int32& nProps, Sequence < Sequence < PropertyValue > >& rSequence, const OUString& rPath )
{
    
    Sequence < PropertyValue > aProps(2);

    
    
    OUString aPath( rPath );
    if ( !m_bIsRoot )
        aPath += m_aName;
    aPath += "/";
    aProps[0].Name = "MediaType";
    aProps[0].Value <<= (OUString ) m_aContentType;
    aProps[1].Name = "FullPath";
    aProps[1].Value <<= (OUString ) aPath;
    rSequence[ nProps++ ] = aProps;

    if ( m_bIsRoot )
        
        aPath = "";

    
    for ( size_t i = 0; i < m_aChildrenList.size(); ++i )
    {
        UCBStorageElement_Impl* pElement = m_aChildrenList[ i ];
        DBG_ASSERT( !pElement->m_bIsFolder || pElement->m_xStorage.Is(), "Storage should be open!" );
        if ( pElement->m_bIsFolder && pElement->m_xStorage.Is() )
            
            pElement->m_xStorage->GetProps( nProps, rSequence, aPath );
        else
        {
            
            OUString aElementPath( aPath );
            aElementPath += pElement->m_aName;
            aProps[0].Name = "MediaType";
            aProps[0].Value <<= (OUString ) pElement->GetContentType();
            aProps[1].Name = "FullPath";
            aProps[1].Value <<= (OUString ) aElementPath;
            rSequence[ nProps++ ] = aProps;
        }
    }
}

UCBStorage_Impl::~UCBStorage_Impl()
{
    
    for ( size_t i = 0, n = m_aChildrenList.size(); i < n; ++i )
        delete m_aChildrenList[ i ];
    m_aChildrenList.clear();

    delete m_pContent;
    delete m_pTempFile;
}

bool UCBStorage_Impl::Insert( ::ucbhelper::Content *pContent )
{
    
    
    bool bRet = false;

    try
    {
        Sequence< ContentInfo > aInfo = pContent->queryCreatableContentsInfo();
        sal_Int32 nCount = aInfo.getLength();
        if ( nCount == 0 )
            return false;

        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            
            const ContentInfo & rCurr = aInfo[i];
            if ( rCurr.Attributes & ContentInfoAttribute::KIND_FOLDER )
            {
                
                const Sequence< Property > & rProps = rCurr.Properties;
                if ( rProps.getLength() != 1 )
                    continue;

                if ( rProps[ 0 ].Name != "Title" )
                    continue;

                Sequence < OUString > aNames(1);
                aNames[0] = "Title";
                Sequence < Any > aValues(1);
                aValues[0] = makeAny( OUString( m_aName ) );

                Content aNewFolder;
                if ( !pContent->insertNewContent( rCurr.Type, aNames, aValues, aNewFolder ) )
                    continue;

                
                DELETEZ( m_pContent );
                m_pContent = new ::ucbhelper::Content( aNewFolder );
                bRet = true;
            }
        }
    }
    catch (const CommandAbortedException&)
    {
        
        SetError( ERRCODE_IO_GENERAL );
    }
    catch (const RuntimeException&)
    {
        
        SetError( ERRCODE_IO_GENERAL );
    }
    catch (const Exception&)
    {
        
        SetError( ERRCODE_IO_GENERAL );
    }

    return bRet;
}

sal_Int16 UCBStorage_Impl::Commit()
{
    
    sal_Int16 nRet = COMMIT_RESULT_NOTHING_TO_DO;

    
    
    if ( ( m_nMode & STREAM_WRITE ) && ( m_bCommited || m_bDirect ) )
    {
        try
        {
            
            for ( size_t i = 0; i < m_aChildrenList.size() && nRet; ++i )
            {
                UCBStorageElement_Impl* pElement = m_aChildrenList[ i ];
                ::ucbhelper::Content* pContent = pElement->GetContent();
                boost::scoped_ptr< ::ucbhelper::Content > xDeleteContent;
                if ( !pContent && pElement->IsModified() )
                {
                    
                    OUString aName( m_aURL );
                    aName += "/";
                    aName += pElement->m_aOriginalName;
                    pContent = new ::ucbhelper::Content( aName, Reference< ::com::sun::star::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
                    xDeleteContent.reset(pContent);  
                }

                if ( pElement->m_bIsRemoved )
                {
                    
                    if ( !pElement->m_bIsInserted )
                    {
                        
                        if( !pElement->m_xStream.Is() || pElement->m_xStream->Clear() )
                        {
                            pContent->executeCommand( OUString("delete"), makeAny( true ) );
                            nRet = COMMIT_RESULT_SUCCESS;
                        }
                        else
                            
                            nRet = COMMIT_RESULT_FAILURE;
                    }
                }
                else
                {
                    sal_Int16 nLocalRet = COMMIT_RESULT_NOTHING_TO_DO;
                    if ( pElement->m_xStorage.Is() )
                    {
                        
                        
                        
                        
                        
                        if ( !pElement->m_bIsInserted || m_bIsLinked || pElement->m_xStorage->Insert( m_pContent ) )
                        {
                            nLocalRet = pElement->m_xStorage->Commit();
                            pContent = pElement->GetContent();
                        }
                    }
                    else if ( pElement->m_xStream.Is() )
                    {
                        
                        nLocalRet = pElement->m_xStream->Commit();
                        if ( pElement->m_xStream->m_bIsOLEStorage )
                        {
                            
                            pElement->m_xStream->m_aContentType = "application/vnd.sun.star.oleobject";
                            Any aValue;
                            aValue <<= true;
                            pElement->m_xStream->m_pContent->setPropertyValue("Encrypted", aValue );
                        }

                        pContent = pElement->GetContent();
                    }

                    if ( pElement->m_aName != pElement->m_aOriginalName )
                    {
                        
                        nLocalRet = COMMIT_RESULT_SUCCESS;
                        Any aAny;
                        aAny <<= (OUString) pElement->m_aName;
                        pContent->setPropertyValue("Title", aAny );
                    }

                    if ( pElement->IsLoaded() && pElement->GetContentType() != pElement->GetOriginalContentType() )
                    {
                        
                        nLocalRet = COMMIT_RESULT_SUCCESS;
                        Any aAny;
                        aAny <<= (OUString) pElement->GetContentType();
                        pContent->setPropertyValue("MediaType", aAny );
                    }

                    if ( nLocalRet != COMMIT_RESULT_NOTHING_TO_DO )
                        nRet = nLocalRet;
                }

                if ( nRet == COMMIT_RESULT_FAILURE )
                    break;
            }
        }
        catch (const ContentCreationException&)
        {
            
            SetError( ERRCODE_IO_NOTEXISTS );
            return COMMIT_RESULT_FAILURE;
        }
        catch (const CommandAbortedException&)
        {
            
            SetError( ERRCODE_IO_GENERAL );
            return COMMIT_RESULT_FAILURE;
        }
        catch (const RuntimeException&)
        {
            
            SetError( ERRCODE_IO_GENERAL );
            return COMMIT_RESULT_FAILURE;
        }
        catch (const Exception&)
        {
            
            SetError( ERRCODE_IO_GENERAL );
            return COMMIT_RESULT_FAILURE;
        }

        if ( m_bIsRoot && m_pContent )
        {
            
            if ( nRet == COMMIT_RESULT_SUCCESS )
            {
                try
                {
                    
                    
                    Any aType;
                    aType <<= (OUString) m_aContentType;
                    m_pContent->setPropertyValue("MediaType", aType );

                    if (  m_bIsLinked )
                    {
                        
                        
                        Content aNewSubFolder;
                        bool bRet = ::utl::UCBContentHelper::MakeFolder( *m_pContent, OUString("META-INF"), aNewSubFolder );
                        if ( bRet )
                        {
                            
                            OUString aURL( aNewSubFolder.getURL() );
                            ::utl::TempFile* pTempFile = new ::utl::TempFile( &aURL );

                            
                            SvStream* pStream = pTempFile->GetStream( STREAM_STD_READWRITE );
                            ::utl::OOutputStreamWrapper* pHelper = new ::utl::OOutputStreamWrapper( *pStream );
                            com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > xOutputStream( pHelper );

                            
                            Reference < ::com::sun::star::packages::manifest::XManifestWriter > xWriter =
                                ::com::sun::star::packages::manifest::ManifestWriter::create(
                                    ::comphelper::getProcessComponentContext() );
                            sal_Int32 nCount = GetObjectCount() + 1;
                            Sequence < Sequence < PropertyValue > > aProps( nCount );
                            sal_Int32 nProps = 0;
                            GetProps( nProps, aProps, OUString() );
                            xWriter->writeManifestSequence( xOutputStream, aProps );

                            
                            Content aSource( pTempFile->GetURL(), Reference < XCommandEnvironment >(), comphelper::getProcessComponentContext() );
                            xWriter = NULL;
                            xOutputStream = NULL;
                            DELETEZ( pTempFile );
                            aNewSubFolder.transferContent( aSource, InsertOperation_MOVE, OUString("manifest.xml"), NameClash::OVERWRITE );
                        }
                    }
                    else
                    {
#if OSL_DEBUG_LEVEL > 1
                        fprintf ( stderr, "Files: %i\n", nOpenFiles );
                        fprintf ( stderr, "Streams: %i\n", nOpenStreams );
#endif
                        
                        Any aAny;
                        m_pContent->executeCommand( OUString("flush"), aAny );
                        if ( m_pSource != 0 )
                        {
                            SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( m_pTempFile->GetURL(), STREAM_STD_READ );
                            m_pSource->SetStreamSize(0);
                            
                            pStream->ReadStream( *m_pSource );
                            DELETEZ( pStream );
                            m_pSource->Seek(0);
                        }
                    }
                }
                catch (const CommandAbortedException&)
                {
                    
                    
                    
                    SetError( ERRCODE_IO_GENERAL );
                    return COMMIT_RESULT_FAILURE;
                }
                catch (const RuntimeException&)
                {
                    
                    
                    
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
                    
                    
                    
                    SetError( ERRCODE_IO_GENERAL );
                    return COMMIT_RESULT_FAILURE;
                }
            }
            else if ( nRet != COMMIT_RESULT_NOTHING_TO_DO )
            {
                
                SetError( ERRCODE_IO_GENERAL );
                return nRet;
            }

            
            
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
                    pInnerElement->m_bIsInserted = false;
                    ++i;
                }
            }
        }

        m_bCommited = false;
    }

    return nRet;
}

bool UCBStorage_Impl::Revert()
{
    for ( size_t i = 0; i < m_aChildrenList.size(); )
    {
        UCBStorageElement_Impl* pElement = m_aChildrenList[ i ];
        pElement->m_bIsRemoved = false;
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
                pElement->m_xStream->m_bCommited = false;
                pElement->m_xStream->Revert();
            }
            else if ( pElement->m_xStorage.Is() )
            {
                pElement->m_xStorage->m_bCommited = false;
                pElement->m_xStorage->Revert();
            }

            pElement->m_aName = pElement->m_aOriginalName;
            pElement->m_bIsRemoved = false;
            ++i;
        }
    }
    return true;
}

const OUString& UCBStorage::GetName() const
{
    return pImp->m_aName; 
}

bool UCBStorage::IsRoot() const
{
    return pImp->m_bIsRoot;
}

void UCBStorage::SetDirty()
{
    pImp->m_bDirty = true;
}

void UCBStorage::SetClass( const SvGlobalName & rClass, sal_uLong nOriginalClipFormat, const OUString & rUserTypeName )
{
    pImp->m_aClassId = rClass;
    pImp->m_nFormat = nOriginalClipFormat;
    pImp->m_aUserTypeName = rUserTypeName;

    
    
    ::com::sun::star::datatransfer::DataFlavor aDataFlavor;
    SotExchange::GetFormatDataFlavor( pImp->m_nFormat, aDataFlavor );
    pImp->m_aContentType = aDataFlavor.MimeType;
}

void UCBStorage::SetClassId( const ClsId& rClsId )
{
    pImp->m_aClassId = SvGlobalName( (const CLSID&) rClsId );
    if ( pImp->m_aClassId == SvGlobalName() )
        return;

    
    
    
    
    
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

void UCBStorage::SetConvertClass( const SvGlobalName & /*rConvertClass*/, sal_uLong /*nOriginalClipFormat*/, const OUString & /*rUserTypeName*/ )
{
    
}

bool UCBStorage::ShouldConvert()
{
    
    return false;
}

SvGlobalName UCBStorage::GetClassName()
{
    return  pImp->m_aClassId;
}

sal_uLong UCBStorage::GetFormat()
{
    return pImp->m_nFormat;
}

OUString UCBStorage::GetUserName()
{
    OSL_FAIL("UserName is not implemented in UCB storages!" );
    return pImp->m_aUserTypeName;
}

void UCBStorage::FillInfoList( SvStorageInfoList* pList ) const
{
    
    for ( size_t i = 0; i < pImp->GetChildrenList().size(); ++i )
    {
        UCBStorageElement_Impl* pElement = pImp->GetChildrenList()[ i ];
        if ( !pElement->m_bIsRemoved )
        {
            
            sal_uLong nSize = pElement->m_nSize;
            if ( pElement->m_xStream.Is() )
                nSize = pElement->m_xStream->GetSize();
            SvStorageInfo aInfo( pElement->m_aName, nSize, pElement->m_bIsStorage );
            pList->push_back( aInfo );
        }
    }
}

bool UCBStorage::CopyStorageElement_Impl( UCBStorageElement_Impl& rElement, BaseStorage* pDest, const OUString& rNew ) const
{
    
    
    
    if ( !rElement.m_bIsStorage )
    {
        
        
        BaseStorageStream* pOtherStream = pDest->OpenStream( rNew, STREAM_WRITE | STREAM_SHARE_DENYALL, pImp->m_bDirect );
        BaseStorageStream* pStream = NULL;
        bool bDeleteStream = false;

        
        if ( rElement.m_xStream.Is() )
            pStream = rElement.m_xStream->m_pAntiImpl;
        if ( !pStream )
        {
            pStream = ( const_cast < UCBStorage* > (this) )->OpenStream( rElement.m_aName, STREAM_STD_READ, pImp->m_bDirect );
            bDeleteStream = true;
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
        
        
        BaseStorage* pStorage = NULL;

        
        bool bDeleteStorage = false;
        if ( rElement.m_xStorage.Is() )
            pStorage = rElement.m_xStorage->m_pAntiImpl;
        if ( !pStorage )
        {
            pStorage = ( const_cast < UCBStorage* > (this) )->OpenStorage( rElement.m_aName, pImp->m_nMode, pImp->m_bDirect );
            bDeleteStorage = true;
        }

        UCBStorage* pUCBDest = PTR_CAST( UCBStorage, pDest );
        UCBStorage* pUCBCopy = PTR_CAST( UCBStorage, pStorage );

        bool bOpenUCBStorage = pUCBDest && pUCBCopy;
        BaseStorage* pOtherStorage = bOpenUCBStorage ?
                pDest->OpenUCBStorage( rNew, STREAM_WRITE | STREAM_SHARE_DENYALL, pImp->m_bDirect ) :
                pDest->OpenOLEStorage( rNew, STREAM_WRITE | STREAM_SHARE_DENYALL, pImp->m_bDirect );

        
        
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

    return Good() && pDest->Good();
}

UCBStorageElement_Impl* UCBStorage::FindElement_Impl( const OUString& rName ) const
{
    DBG_ASSERT( !rName.isEmpty(), "Name is empty!" );
    for ( size_t i = 0, n = pImp->GetChildrenList().size(); i < n; ++i )
    {
        UCBStorageElement_Impl* pElement = pImp->GetChildrenList()[ i ];
        if ( pElement->m_aName == rName && !pElement->m_bIsRemoved )
            return pElement;
    }
    return NULL;
}

bool UCBStorage::CopyTo( BaseStorage* pDestStg ) const
{
    DBG_ASSERT( pDestStg != ((BaseStorage*)this), "Self-Copying is not possible!" );
    if ( pDestStg == ((BaseStorage*)this) )
        return false;

    
    

    
    
    if( pDestStg->ISA( UCBStorage ) )
        pDestStg->SetClass( pImp->m_aClassId, pImp->m_nFormat,
                            pImp->m_aUserTypeName );
    else
        pDestStg->SetClassId( GetClassId() );
    pDestStg->SetDirty();

    bool bRet = true;
    for ( size_t i = 0; i < pImp->GetChildrenList().size() && bRet; ++i )
    {
        UCBStorageElement_Impl* pElement = pImp->GetChildrenList()[ i ];
        if ( !pElement->m_bIsRemoved )
            bRet = CopyStorageElement_Impl( *pElement, pDestStg, pElement->m_aName );
    }

    if( !bRet )
        SetError( pDestStg->GetError() );
    return Good() && pDestStg->Good();
}

bool UCBStorage::CopyTo( const OUString& rElemName, BaseStorage* pDest, const OUString& rNew )
{
    if( rElemName.isEmpty() )
        return false;

    if ( pDest == ((BaseStorage*) this) )
    {
        
        return false;
    }
    else
    {
        
            UCBStorageElement_Impl* pElement = FindElement_Impl( rElemName );
        if ( pElement )
            return CopyStorageElement_Impl( *pElement, pDest, rNew );
        else
        {
            SetError( SVSTREAM_FILE_NOT_FOUND );
            return false;
        }
    }
}

bool UCBStorage::Commit()
{
    
    pImp->m_bCommited = true;
    if ( pImp->m_bIsRoot )
        
        return ( pImp->Commit() != COMMIT_RESULT_FAILURE );
    else
        return true;
}

bool UCBStorage::Revert()
{
    return pImp->Revert();
}

BaseStorageStream* UCBStorage::OpenStream( const OUString& rEleName, StreamMode nMode, bool bDirect, const OString* pKey )
{
    if( rEleName.isEmpty() )
        return NULL;

    
    UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    if ( !pElement )
    {
        
        if( ( nMode & STREAM_NOCREATE ) )
        {
            SetError( ( nMode & STREAM_WRITE ) ? SVSTREAM_CANNOT_MAKE : SVSTREAM_FILE_NOT_FOUND );
            OUString aName( pImp->m_aURL );
            aName += "/";
            aName += rEleName;
            UCBStorageStream* pStream = new UCBStorageStream( aName, nMode, bDirect, pKey, pImp->m_bRepairPackage, pImp->m_xProgressHandler );
            pStream->SetError( GetError() );
            pStream->pImp->m_aName = rEleName;
            return pStream;
        }
        else
        {
            
            pElement = new UCBStorageElement_Impl( rEleName );
            pElement->m_bIsInserted = true;
            pImp->m_aChildrenList.push_back( pElement );
        }
    }

    if ( !pElement->m_bIsFolder )
    {
        
        if ( pElement->m_xStream.Is() )
        {
            
            if ( pElement->m_xStream->m_pAntiImpl )
            {
                OSL_FAIL("Stream is already open!" );
                SetError( SVSTREAM_ACCESS_DENIED );  
                return NULL;
            }
            else
            {
                
                
                OString aKey;
                if ( pKey )
                    aKey = *pKey;
                if ( pElement->m_xStream->m_aKey == aKey )
                {
                    pElement->m_xStream->PrepareCachedForReopen( nMode );

                    return new UCBStorageStream( pElement->m_xStream );
                }
            }
        }

        
        pImp->OpenStream( pElement, nMode, bDirect, pKey );

        
        pElement->m_xStream->m_aName = rEleName;
        return new UCBStorageStream( pElement->m_xStream );
    }

    return NULL;
}

UCBStorageStream_Impl* UCBStorage_Impl::OpenStream( UCBStorageElement_Impl* pElement, StreamMode nMode, bool bDirect, const OString* pKey )
{
    OUString aName( m_aURL );
    aName += "/";
    aName += pElement->m_aOriginalName;
    pElement->m_xStream = new UCBStorageStream_Impl( aName, nMode, NULL, bDirect, pKey, m_bRepairPackage, m_xProgressHandler );
    return pElement->m_xStream;
}

BaseStorage* UCBStorage::OpenUCBStorage( const OUString& rEleName, StreamMode nMode, bool bDirect )
{
    if( rEleName.isEmpty() )
        return NULL;

    return OpenStorage_Impl( rEleName, nMode, bDirect, true );
}

BaseStorage* UCBStorage::OpenOLEStorage( const OUString& rEleName, StreamMode nMode, bool bDirect )
{
    if( rEleName.isEmpty() )
        return NULL;

    return OpenStorage_Impl( rEleName, nMode, bDirect, false );
}

BaseStorage* UCBStorage::OpenStorage( const OUString& rEleName, StreamMode nMode, bool bDirect )
{
    if( rEleName.isEmpty() )
        return NULL;

    return OpenStorage_Impl( rEleName, nMode, bDirect, true );
}

BaseStorage* UCBStorage::OpenStorage_Impl( const OUString& rEleName, StreamMode nMode, bool bDirect, bool bForceUCBStorage )
{
    
    UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    if ( !pElement )
    {
        
        if( ( nMode & STREAM_NOCREATE ) )
        {
            SetError( ( nMode & STREAM_WRITE ) ? SVSTREAM_CANNOT_MAKE : SVSTREAM_FILE_NOT_FOUND );
            OUString aName( pImp->m_aURL );
            aName += "/";
            aName += rEleName;  
            UCBStorage *pStorage = new UCBStorage( aName, nMode, bDirect, false, pImp->m_bRepairPackage, pImp->m_xProgressHandler );
            pStorage->pImp->m_bIsRoot = false;
            pStorage->pImp->m_bListCreated = true; 
            pStorage->SetError( GetError() );
            return pStorage;
        }

        
        
        
        pElement = new UCBStorageElement_Impl( rEleName );
        pElement->m_bIsInserted = true;
        pImp->m_aChildrenList.push_back( pElement );
    }

    if ( !pElement->m_bIsFolder && ( pElement->m_bIsStorage || !bForceUCBStorage ) )
    {
        
        
        
        
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

        pElement->m_bIsStorage = true;
        return pElement->m_xStream->CreateStorage();  
    }
    else if ( pElement->m_xStorage.Is() )
    {
        
        if ( pElement->m_xStorage->m_pAntiImpl )
        {
            OSL_FAIL("Storage is already open!" );
            SetError( SVSTREAM_ACCESS_DENIED );  
        }
        else
        {
            bool bIsWritable = (( pElement->m_xStorage->m_nMode & STREAM_WRITE ) != 0);
            if ( !bIsWritable && (( nMode & STREAM_WRITE ) != 0 ))
            {
                OUString aName( pImp->m_aURL );
                aName += "/";
                aName += pElement->m_aOriginalName;
                UCBStorage* pStorage = new UCBStorage( aName, nMode, bDirect, false, pImp->m_bRepairPackage, pImp->m_xProgressHandler );
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
        
        bool bIsWritable = (( pImp->m_nMode & STREAM_WRITE ) != 0 );
        if ( pImp->m_bIsLinked && pImp->m_bIsRoot && bIsWritable )
        {
            
            INetURLObject aFolderObj( pImp->m_aURL );
            aFolderObj.removeSegment();

            Content aFolder( aFolderObj.GetMainURL( INetURLObject::NO_DECODE ), Reference < XCommandEnvironment >(), comphelper::getProcessComponentContext() );
            pImp->m_pContent = new Content;
            bool bRet = ::utl::UCBContentHelper::MakeFolder( aFolder, pImp->m_aName, *pImp->m_pContent );
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
                pStor->m_bListCreated = true; 

            return new UCBStorage( pStor );
        }
    }

    return NULL;
}

UCBStorage_Impl* UCBStorage_Impl::OpenStorage( UCBStorageElement_Impl* pElement, StreamMode nMode, bool bDirect )
{
    UCBStorage_Impl* pRet = NULL;
    OUString aName( m_aURL );
    aName += "/";
    aName += pElement->m_aOriginalName;  

    pElement->m_bIsStorage = pElement->m_bIsFolder = true;

    if ( m_bIsLinked && !::utl::UCBContentHelper::Exists( aName ) )
    {
        Content aNewFolder;
        bool bRet = ::utl::UCBContentHelper::MakeFolder( *m_pContent, pElement->m_aOriginalName, aNewFolder );
        if ( bRet )
            pRet = new UCBStorage_Impl( aNewFolder, aName, nMode, NULL, bDirect, false, m_bRepairPackage, m_xProgressHandler );
    }
    else
    {
        pRet = new UCBStorage_Impl( aName, nMode, NULL, bDirect, false, m_bRepairPackage, m_xProgressHandler );
    }

    if ( pRet )
    {
        pRet->m_bIsLinked = m_bIsLinked;
        pRet->m_bIsRoot = false;

        
        pRet->m_aName = pElement->m_aOriginalName;
        pElement->m_xStorage = pRet;
    }

    if ( pRet )
        pRet->Init();

    return pRet;
}

bool UCBStorage::IsStorage( const OUString& rEleName ) const
{
    if( rEleName.isEmpty() )
        return false;

    const UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    return ( pElement && pElement->m_bIsStorage );
}

bool UCBStorage::IsStream( const OUString& rEleName ) const
{
    if( rEleName.isEmpty() )
        return false;

    const UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    return ( pElement && !pElement->m_bIsStorage );
}

bool UCBStorage::IsContained( const OUString & rEleName ) const
{
    if( rEleName.isEmpty() )
        return false;
    const UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    return ( pElement != NULL );
}

bool UCBStorage::Remove( const OUString& rEleName )
{
    if( rEleName.isEmpty() )
        return false;

    UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    if ( pElement )
    {
        pElement->m_bIsRemoved = true;
    }
    else
        SetError( SVSTREAM_FILE_NOT_FOUND );

    return ( pElement != NULL );
}

bool UCBStorage::Rename( const OUString& rEleName, const OUString& rNewName )
{
    if( rEleName.isEmpty()|| rNewName.isEmpty() )
        return false;

    UCBStorageElement_Impl *pAlreadyExisting = FindElement_Impl( rNewName );
    if ( pAlreadyExisting )
    {
        SetError( SVSTREAM_ACCESS_DENIED );
        return false;                       
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

bool UCBStorage::MoveTo( const OUString& rEleName, BaseStorage* pNewSt, const OUString& rNewName )
{
    if( rEleName.isEmpty() || rNewName.isEmpty() )
        return false;

    if ( pNewSt == ((BaseStorage*) this) && !FindElement_Impl( rNewName ) )
    {
        return Rename( rEleName, rNewName );
    }
    else
    {
/*
        if ( PTR_CAST( UCBStorage, pNewSt ) )
        {
            
            
            
            
            
                
            
            
            
            
        }
*/
        
        bool bRet = CopyTo( rEleName, pNewSt, rNewName );
        if ( bRet )
            bRet = Remove( rEleName );
        return bRet;
    }
}

bool UCBStorage::ValidateFAT()
{
    
    return true;
}

bool UCBStorage::Validate( bool  bWrite ) const
{
    
    return ( !bWrite || ( pImp->m_nMode & STREAM_WRITE ) );
}

bool UCBStorage::ValidateMode( StreamMode m ) const
{
    
    if( m == ( STREAM_READ | STREAM_TRUNC ) )  
        return true;
    sal_uInt16 nCurMode = 0xFFFF;
    if( ( m & 3 ) == STREAM_READ )
    {
        
        if( ( ( m & STREAM_SHARE_DENYWRITE )
           && ( nCurMode & STREAM_SHARE_DENYWRITE ) )
         || ( ( m & STREAM_SHARE_DENYALL )
           && ( nCurMode & STREAM_SHARE_DENYALL ) ) )
            return true;
    }
    else
    {
        
        
        
        if( ( m & STREAM_SHARE_DENYALL )
         && ( nCurMode & STREAM_SHARE_DENYALL ) )
            return true;
    }

    return true;
}

const SvStream* UCBStorage::GetSvStream() const
{
    
    
    return pImp->m_pSource;
}

bool UCBStorage::Equals( const BaseStorage& rStorage ) const
{
    
    return ((BaseStorage*)this) == &rStorage;
}

bool UCBStorage::IsStorageFile( SvStream* pFile )
{
    if ( !pFile )
        return false;

    sal_uLong nPos = pFile->Tell();
    pFile->Seek( STREAM_SEEK_TO_END );
    if ( pFile->Tell() < 4 )
        return false;

    pFile->Seek(0);
    sal_uInt32 nBytes(0);
    pFile->ReadUInt32( nBytes );

    
    bool bRet = ( nBytes == 0x04034b50 );
    if ( !bRet )
    {
        
        bRet = ( nBytes == 0x08074b50 );
        if ( bRet )
        {
            nBytes = 0;
            pFile->ReadUInt32( nBytes );
            bRet = ( nBytes == 0x04034b50 );
        }
    }

    pFile->Seek( nPos );
    return bRet;
}

bool UCBStorage::IsDiskSpannedFile( SvStream* pFile )
{
    if ( !pFile )
        return false;

    sal_uLong nPos = pFile->Tell();
    pFile->Seek( STREAM_SEEK_TO_END );
    if ( !pFile->Tell() )
        return false;

    pFile->Seek(0);
    sal_uInt32 nBytes;
    pFile->ReadUInt32( nBytes );

    
    bool bRet = ( nBytes == 0x08074b50 );
    if ( bRet )
    {
        pFile->ReadUInt32( nBytes );
        bRet = ( nBytes == 0x04034b50 );
    }

    pFile->Seek( nPos );
    return bRet;
}

OUString UCBStorage::GetLinkedFile( SvStream &rStream )
{
    OUString aString;
    sal_uLong nPos = rStream.Tell();
    rStream.Seek( STREAM_SEEK_TO_END );
    if ( !rStream.Tell() )
        return aString;

    rStream.Seek(0);
    sal_uInt32 nBytes;
    rStream.ReadUInt32( nBytes );
    if( nBytes == 0x04034b50 )
    {
        OString aTmp = read_uInt16_lenPrefixed_uInt8s_ToOString(rStream);
        if (aTmp.match("ContentURL="))
        {
            aString = OStringToOUString(aTmp.copy(11), RTL_TEXTENCODING_UTF8);
        }
    }

    rStream.Seek( nPos );
    return aString;
}

OUString UCBStorage::CreateLinkFile( const OUString& rName )
{
    
    INetURLObject aFolderObj( rName );
    OUString aName = aFolderObj.GetName();
    aFolderObj.removeSegment();
    OUString aFolderURL( aFolderObj.GetMainURL( INetURLObject::NO_DECODE ) );
    ::utl::TempFile* pTempFile = new ::utl::TempFile( &aFolderURL );

    
    SvStream* pStream = pTempFile->GetStream( STREAM_STD_READWRITE | STREAM_TRUNC );

    
    pStream->WriteUInt32( ( sal_uInt32 ) 0x04034b50 );

    
    INetURLObject aObj( rName );
    OUString aTmpName = aObj.GetName();
    OUString aTitle = "content." + aTmpName;

    
    Content aFolder( aFolderURL, Reference < XCommandEnvironment >(), comphelper::getProcessComponentContext() );
    Content aNewFolder;
    bool bRet = ::utl::UCBContentHelper::MakeFolder( aFolder, aTitle, aNewFolder );
    if ( !bRet )
    {
        aFolderObj.insertName( aTitle );
        if ( ::utl::UCBContentHelper::Exists( aFolderObj.GetMainURL( INetURLObject::NO_DECODE ) ) )
        {
            
            
            aTitle += ".";
            for ( sal_Int32 i=0; !bRet; i++ )
            {
                OUString aTmp = aTitle + OUString::number( i );
                bRet = ::utl::UCBContentHelper::MakeFolder( aFolder, aTmp, aNewFolder );
                if ( bRet )
                    aTitle = aTmp;
                else
                {
                    aFolderObj.SetName( aTmp );
                    if ( !::utl::UCBContentHelper::Exists( aFolderObj.GetMainURL( INetURLObject::NO_DECODE ) ) )
                        
                        break;
                }
            }
        }
    }

    if ( bRet )
    {
        
        aObj.SetName( aTitle );
        OUString aURL = aObj.GetMainURL( INetURLObject::NO_DECODE );

        
        OUString aLink = "ContentURL=" + aURL;
        write_uInt16_lenPrefixed_uInt8s_FromOUString(*pStream, aLink, RTL_TEXTENCODING_UTF8);
        pStream->Flush();

        
        Content aSource( pTempFile->GetURL(), Reference < XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        DELETEZ( pTempFile );
        aFolder.transferContent( aSource, InsertOperation_MOVE, aName, NameClash::OVERWRITE );
        return aURL;
    }

    pTempFile->EnableKillingFile( true );
    delete pTempFile;
    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
