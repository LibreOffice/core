/*************************************************************************
 *
 *  $RCSfile: ucbstorage.cxx,v $
 *
 *  $Revision: 1.80 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 16:52:52 $
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

#ifndef _COM_SUN_STAR_IO_NOTCONNECTEDEXCEPTION_HPP_
#include <com/sun/star/io/NotConnectedException.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_BUFFERSIZEEXCEEDEDEXCEPTION_HPP_
#include <com/sun/star/io/BufferSizeExceededException.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#include <unotools/tempfile.hxx>
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_RESULTSETEXCEPTION_HPP_
#include <com/sun/star/ucb/ResultSetException.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HDL_
#include <com/sun/star/sdbc/XResultSet.hdl>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DATAFLAVOR_HPP_
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTCREATOR_HPP_
#include <com/sun/star/ucb/XContentCreator.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_CONTENTINFOATTRIBUTE_HPP_
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_MANIFEST_XMANIFESTWRITER_HPP_
#include <com/sun/star/packages/manifest/XManifestWriter.hpp>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_MANIFEST_XMANIFESTREADER_HPP_
#include <com/sun/star/packages/manifest/XManifestReader.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEIODEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#endif

#include <rtl/digest.h>
#include <tools/ref.hxx>
#include <tools/debug.hxx>
#include <unotools/streamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <tools/list.hxx>
#include <tools/urlobj.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase2.hxx>
#include <ucbhelper/commandenvironment.hxx>

#include "stg.hxx"
#include "storinfo.hxx"
#include "storage.hxx"
#include "exchange.hxx"
#include "formats.hxx"
#include "clsids.hxx"

#include "unostorageholder.hxx"

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::sdbc;
using namespace ::ucb;

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

    //DECLARE_UNO3_AGG_DEFAULTS( FileStreamWrapper_Impl, FileInputStreamWrapper_Base);

    virtual void SAL_CALL seek( sal_Int64 _nLocation ) throw ( IllegalArgumentException, IOException, RuntimeException);
    virtual sal_Int64 SAL_CALL getPosition(  ) throw ( IOException, RuntimeException);
    virtual sal_Int64 SAL_CALL getLength(  ) throw ( IOException, RuntimeException);
    virtual sal_Int32   SAL_CALL    readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead) throw( NotConnectedException, BufferSizeExceededException, RuntimeException );
    virtual sal_Int32   SAL_CALL    readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead) throw( NotConnectedException, BufferSizeExceededException, RuntimeException );
    virtual void        SAL_CALL    skipBytes(sal_Int32 nBytesToSkip) throw( NotConnectedException, BufferSizeExceededException, RuntimeException);
    virtual sal_Int32   SAL_CALL    available() throw( NotConnectedException, RuntimeException );
    virtual void        SAL_CALL    closeInput() throw( NotConnectedException, RuntimeException );

protected:
    void checkConnected();
    void checkError();
};

//------------------------------------------------------------------
FileStreamWrapper_Impl::FileStreamWrapper_Impl( const String& rName )
    : m_pSvStream(0)
    , m_aURL( rName )
{
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
    ::osl::MutexGuard aGuard( m_aMutex );
    checkError();

#ifdef DBG_UTIL
    sal_uInt32 nCurrentPos = m_pSvStream->Tell();
#endif

    m_pSvStream->SeekRel(nBytesToSkip);
    checkError();

#ifdef DBG_UTIL
    nCurrentPos = m_pSvStream->Tell();
#endif
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL FileStreamWrapper_Impl::available() throw( NotConnectedException, RuntimeException )
{
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
    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    m_pSvStream->Seek((sal_uInt32)_nLocation);
    checkError();
}

//------------------------------------------------------------------------------
sal_Int64 SAL_CALL FileStreamWrapper_Impl::getPosition(  ) throw (IOException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    sal_uInt32 nPos = m_pSvStream->Tell();
    checkError();
    return (sal_Int64)nPos;
}

//------------------------------------------------------------------------------
sal_Int64 SAL_CALL FileStreamWrapper_Impl::getLength(  ) throw (IOException, RuntimeException)
{
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
        DBG_ERROR( "Unknown UCB storage format!" );
        return 0;
    }
}


SvGlobalName GetClassId_Impl( sal_Int32 nFormat )
{
    switch ( nFormat )
    {
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
            //DBG_ERROR( "Unknown UCB storage format!" );
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

    virtual ULONG               GetData( void* pData, ULONG nSize );
    virtual ULONG               PutData( const void* pData, ULONG nSize );
    virtual ULONG               SeekPos( ULONG nPos );
    virtual void                SetSize( ULONG nSize );
    virtual void                FlushData();
    virtual void                ResetError();

    UCBStorageStream*           m_pAntiImpl;    // only valid if an external reference exists

    String                      m_aOriginalName;// the original name before accessing the stream
    String                      m_aName;        // the actual name ( changed with a Rename command at the parent )
    String                      m_aURL;         // the full path name to create the content
    String                      m_aContentType;
    String                      m_aOriginalContentType;
    ByteString                  m_aKey;
    ::ucb::Content*             m_pContent;     // the content that provides the data
    Reference<XInputStream>     m_rSource;      // the stream covering the original data of the content
    SvStream*                   m_pStream;      // the stream worked on; for readonly streams it is the original stream of the content
                                                // for read/write streams it's a copy into a temporary file
    String                      m_aTempURL;     // URL of this temporary stream
    RepresentModes              m_nRepresentMode; // should it be used as XInputStream or as SvStream
    long                        m_nError;
    StreamMode                  m_nMode;        // open mode ( read/write/trunc/nocreate/sharing )
    BOOL                        m_bSourceRead;  // Source still contains useful information
    BOOL                        m_bModified;    // only modified streams will be sent to the original content
    BOOL                        m_bCommited;    // sending the streams is coordinated by the root storage of the package
    BOOL                        m_bDirect;      // the storage and its streams are opened in direct mode; for UCBStorages
                                                // this means that the root storage does an autocommit when its external
                                                // reference is destroyed
    BOOL                        m_bIsOLEStorage;// an OLEStorage on a UCBStorageStream makes this an Autocommit-stream

                                UCBStorageStream_Impl( const String&, StreamMode, UCBStorageStream*, BOOL, const ByteString* pKey=0, BOOL bRepair = FALSE, Reference< XProgressHandler > xProgress = Reference< XProgressHandler >() );

    void                        Free();
    BOOL                        Init();
    BOOL                        Clear();
    sal_Int16                   Commit();       // if modified and commited: transfer an XInputStream to the content
    BOOL                        Revert();       // discard all changes
    BaseStorage*                CreateStorage();// create an OLE Storage on the UCBStorageStream
    ULONG                       GetSize();

    ULONG                       ReadSourceWriteTemporary( ULONG aLength ); // read aLength from source and copy to temporary,
                                                                           // no seeking is produced
    ULONG                       ReadSourceWriteTemporary();                // read source till the end and copy to temporary,
                                                                           // no seeking is produced
    ULONG                       CopySourceToTemporary( ULONG aLength ); // same as ReadSourceWriteToTemporary( aLength )
                                                                        // but the writing is done at the end of temporary
                                                                        // pointer position is not changed
    ULONG                       CopySourceToTemporary();                // same as ReadSourceWriteToTemporary()
                                                                        // but the writing is done at the end of temporary
                                                                        // pointer position is not changed
    Reference<XInputStream>     GetXInputStream();                      // return XInputStream, after that
                                                                        // this class is close to be unusable
                                                                        // since it can not read and write
    void                        SetError( long nError );
    void                        PrepareCachedForReopen( StreamMode nMode );
};

SV_DECL_IMPL_REF( UCBStorageStream_Impl );

struct UCBStorageElement_Impl;
DECLARE_LIST( UCBStorageElementList_Impl, UCBStorageElement_Impl* );

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
    ::ucb::Content*             m_pContent;     // the content that provides the storage elements
    ::utl::TempFile*            m_pTempFile;    // temporary file, only for storages on stream
    SvStream*                   m_pSource;      // original stream, only for storages on a stream
    //SvStream*                   m_pStream;      // the corresponding editable stream, only for storage on a stream
    long                        m_nError;
    StreamMode                  m_nMode;        // open mode ( read/write/trunc/nocreate/sharing )
    BOOL                        m_bModified;    // only modified elements will be sent to the original content
    BOOL                        m_bCommited;    // sending the streams is coordinated by the root storage of the package
    BOOL                        m_bDirect;      // the storage and its streams are opened in direct mode; for UCBStorages
                                                // this means that the root storage does an autocommit when its external
                                                // reference is destroyed
    BOOL                        m_bIsRoot;      // marks this storage as root storages that manages all oommits and reverts
    BOOL                        m_bDirty;       // ???
    BOOL                        m_bIsLinked;
    BOOL                        m_bListCreated;
    ULONG                       m_nFormat;
    String                      m_aUserTypeName;
    SvGlobalName                m_aClassId;

    UCBStorageElementList_Impl  m_aChildrenList;

    BOOL                        m_bRepairPackage;
    Reference< XProgressHandler > m_xProgressHandler;

    UNOStorageHolderList*       m_pUNOStorageHolderList;
                                UCBStorage_Impl( const ::ucb::Content&, const String&, StreamMode, UCBStorage*, BOOL, BOOL, BOOL = FALSE, Reference< XProgressHandler > = Reference< XProgressHandler >() );
                                UCBStorage_Impl( const String&, StreamMode, UCBStorage*, BOOL, BOOL, BOOL = FALSE, Reference< XProgressHandler > = Reference< XProgressHandler >() );
                                UCBStorage_Impl( SvStream&, UCBStorage*, BOOL );
    void                        Init();
    sal_Int16                   Commit();
    BOOL                        Revert();
    BOOL                        Insert( ::ucb::Content *pContent );
    UCBStorage_Impl*            OpenStorage( UCBStorageElement_Impl* pElement, StreamMode nMode, BOOL bDirect );
    UCBStorageStream_Impl*      OpenStream( UCBStorageElement_Impl*, StreamMode, BOOL, const ByteString* pKey=0 );
    void                        SetProps( const Sequence < Sequence < PropertyValue > >& rSequence, const String& );
    void                        GetProps( sal_Int32&, Sequence < Sequence < PropertyValue > >& rSequence, const String& );
    sal_Int32                   GetObjectCount();
    void                        ReadContent();
    void                        CreateContent();
    ::ucb::Content*             GetContent()
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
    ULONG                       m_nSize;
    BOOL                        m_bIsFolder;    // Only TRUE when it is a UCBStorage !
    BOOL                        m_bIsStorage;   // Also TRUE when it is an OLEStorage !
    BOOL                        m_bIsRemoved;   // element will be removed on commit
    BOOL                        m_bIsInserted;  // element will be removed on revert
    UCBStorage_ImplRef          m_xStorage;     // reference to the "real" storage
    UCBStorageStream_ImplRef    m_xStream;      // reference to the "real" stream

                                UCBStorageElement_Impl( const ::rtl::OUString& rName,
                                            const ::rtl::OUString rMediaType = ::rtl::OUString(),
                                            BOOL bIsFolder = FALSE, ULONG nSize = 0 )
                                    : m_aName( rName )
                                    , m_aOriginalName( rName )
                                    , m_nSize( nSize )
                                    , m_bIsFolder( bIsFolder )
                                    , m_bIsStorage( bIsFolder )
                                    , m_bIsRemoved( FALSE )
                                    , m_bIsInserted( FALSE )
                                {
                                }

    ::ucb::Content*             GetContent();
    BOOL                        IsModified();
    String                      GetContentType();
    void                        SetContentType( const String& );
    String                      GetOriginalContentType();
    BOOL                        IsLoaded()
                                { return m_xStream.Is() || m_xStorage.Is(); }
};

::ucb::Content* UCBStorageElement_Impl::GetContent()
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
        DBG_ERROR("Element not loaded!");
        return String();
    }
}

void UCBStorageElement_Impl::SetContentType( const String& rType )
{
    if ( m_xStream.Is() )
        m_xStream->m_aContentType = m_xStream->m_aOriginalContentType = rType;
    else if ( m_xStorage.Is() )
        m_xStorage->m_aContentType = m_xStorage->m_aOriginalContentType = rType;
    else
        DBG_ERROR("Element not loaded!");
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

BOOL UCBStorageElement_Impl::IsModified()
{
    BOOL bModified = m_bIsRemoved || m_bIsInserted || m_aName != m_aOriginalName;
    if ( bModified )
    {
        if ( m_xStream.Is() )
            bModified = m_xStream->m_aContentType != m_xStream->m_aOriginalContentType;
        else if ( m_xStorage.Is() )
            bModified = m_xStorage->m_aContentType != m_xStorage->m_aOriginalContentType;
    }

    return bModified;
}

UCBStorageStream_Impl::UCBStorageStream_Impl( const String& rName, StreamMode nMode, UCBStorageStream* pStream, BOOL bDirect, const ByteString* pKey, BOOL bRepair, Reference< XProgressHandler > xProgress  )
    : m_pAntiImpl( pStream )
    , m_bModified( FALSE )
    , m_bCommited( FALSE )
    , m_bIsOLEStorage( FALSE )
    , m_bSourceRead( !( nMode & STREAM_TRUNC ) )
    , m_bDirect( bDirect )
    , m_aURL( rName )
    , m_nError( 0 )
    , m_nMode( nMode )
    , m_pContent( NULL )
    , m_pStream( NULL )
    , m_nRepresentMode( nonset )
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
            xComEnv = new ::ucb::CommandEnvironment( Reference< ::com::sun::star::task::XInteractionHandler >(),
                                                     xProgress );
            aTemp += rtl::OUString::createFromAscii("?repairpackage");
        }

        m_pContent = new ::ucb::Content( aTemp, xComEnv );

        if ( pKey )
        {
            m_aKey = *pKey;

            // stream is encrypted and should be decrypted (without setting the key we'll get the raw data)
            sal_uInt8 aBuffer[RTL_DIGEST_LENGTH_SHA1];
            rtlDigestError nErr = rtl_digest_SHA1( pKey->GetBuffer(), pKey->Len(), aBuffer, RTL_DIGEST_LENGTH_SHA1 );
            if ( nErr == rtl_Digest_E_None )
            {
                sal_uInt8* pBuffer = aBuffer;
                ::com::sun::star::uno::Sequence < sal_Int8 > aSequ( (sal_Int8*) pBuffer, RTL_DIGEST_LENGTH_SHA1 );
                ::com::sun::star::uno::Any aAny;
                aAny <<= aSequ;
                m_pContent->setPropertyValue( ::rtl::OUString::createFromAscii("EncryptionKey"), aAny );
            }
        }
    }
    catch ( ContentCreationException& )
    {
        // content could not be created
        SetError( SVSTREAM_CANNOT_MAKE );
    }
    catch ( RuntimeException& )
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
        DBG_ERROR( "Misuse of the XInputstream!" );
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
                aResult = new ::utl::OInputStreamWrapper( m_pStream, TRUE );
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
            catch ( Exception& )
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

BOOL UCBStorageStream_Impl::Init()
{
    if( m_nRepresentMode == xinputstream )
    {
        DBG_ERROR( "XInputStream misuse!" );
        SetError( ERRCODE_IO_ACCESSDENIED );
        return FALSE;
    }

    if( !m_pStream )
    {
        // no temporary stream was created
        // create one

        m_nRepresentMode = svstream; // can not be used as XInputStream

        if ( !m_aTempURL.Len() )
            m_aTempURL = ::utl::TempFile().GetURL();

        m_pStream = ::utl::UcbStreamHelper::CreateStream( m_aTempURL, STREAM_STD_READWRITE );
#if OSL_DEBUG_LEVEL > 1
        ++nOpenFiles;
#endif

        if( !m_pStream )
        {
            DBG_ERROR( "Suspicious temporary stream creation!" );
            SetError( SVSTREAM_CANNOT_MAKE );
            return FALSE;
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
        catch ( Exception& )
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
            catch( BufferSizeExceededException& )
            {
                // the temporary stream already contain all the data
                m_bSourceRead = FALSE;
            }
            catch( Exception& )
            {
                // something is really wrong
                m_bSourceRead = FALSE;
                DBG_ERROR( "Can not operate original stream!" );
                SetError( SVSTREAM_CANNOT_MAKE );
            }

            m_pStream->Seek( 0 );
        }
        else
        {
            // if the new file is edited than no source exist
            m_bSourceRead = FALSE;
               //SetError( SVSTREAM_CANNOT_MAKE );
        }
    }

    DBG_ASSERT( m_rSource.is() || !m_bSourceRead, "Unreadable source stream!" );

    return sal_True;
}

ULONG UCBStorageStream_Impl::ReadSourceWriteTemporary()
{
    // read source stream till the end and copy all the data to
    // the current position of the temporary stream

    ULONG aResult = 0;

    if( m_bSourceRead )
    {
        Sequence<sal_Int8> aData(32000);

        try
        {
            ULONG aReaded;
            do
            {
                aReaded = m_rSource->readBytes( aData, 32000 );
                aResult += m_pStream->Write( aData.getArray(), aReaded );
            } while( aReaded == 32000 );
        }
#if OSL_DEBUG_LEVEL > 1
        catch( Exception & e )
        {
            DBG_ASSERT( FALSE, ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
#else
        catch( Exception & )
        {
#endif
        }
    }

    m_bSourceRead = FALSE;

    return aResult;

}

ULONG UCBStorageStream_Impl::ReadSourceWriteTemporary( ULONG aLength )
{
    // read aLength bite from the source stream and copy them to the current
    // position of the temporary stream

    ULONG aResult = 0;

    if( m_bSourceRead )
    {
        Sequence<sal_Int8> aData(32000);

        try
        {

            ULONG aReaded = 32000;

            for( ULONG pInd = 0; pInd < aLength && aReaded == 32000 ; pInd += 32000 )
            {
                ULONG aToCopy = min( aLength - pInd, 32000 );
                aReaded = m_rSource->readBytes( aData, aToCopy );
                aResult += m_pStream->Write( aData.getArray(), aReaded );
            }

            if( aResult < aLength )
                m_bSourceRead = FALSE;
        }
#if OSL_DEBUG_LEVEL > 1
        catch( Exception & e )
        {
            DBG_ASSERT( FALSE, ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
#else
        catch( Exception & )
        {
#endif
        }
    }

    return aResult;
}

ULONG UCBStorageStream_Impl::CopySourceToTemporary()
{
    // current position of the temporary stream is not changed
    ULONG aResult = 0;

    if( m_bSourceRead )
    {
        ULONG aPos = m_pStream->Tell();
        m_pStream->Seek( STREAM_SEEK_TO_END );
        aResult = ReadSourceWriteTemporary();
        m_pStream->Seek( aPos );
    }

    return aResult;

}

ULONG UCBStorageStream_Impl::CopySourceToTemporary( ULONG aLength )
{
    // current position of the temporary stream is not changed
    ULONG aResult = 0;

    if( m_bSourceRead )
    {
        ULONG aPos = m_pStream->Tell();
        m_pStream->Seek( STREAM_SEEK_TO_END );
        aResult = ReadSourceWriteTemporary( aLength );
        m_pStream->Seek( aPos );
    }

    return aResult;

}

// UCBStorageStream_Impl must have a SvStream interface, because it then can be used as underlying stream
// of an OLEStorage; so every write access caused by storage operations marks the UCBStorageStream as modified
ULONG UCBStorageStream_Impl::GetData( void* pData, ULONG nSize )
{
    ULONG aResult = 0;

    if( !Init() )
        return 0;


    ULONG aStartPos = m_pStream->Tell();

    // read data that is in temporary stream
    aResult = m_pStream->Read( pData, nSize );
    if( m_bSourceRead && aResult < nSize )
    {
        // read the tail of the data from original stream
        // copy this tail to the temporary stream

        ULONG aToRead = nSize - aResult;
        pData = (void*)( (char*)pData + aResult );

        try
        {
            Sequence<sal_Int8> aData( aToRead );
            ULONG aReaded = m_rSource->readBytes( aData, aToRead );
            aResult += m_pStream->Write( (void*)aData.getArray(), aReaded );
            int p = m_pStream->GetError();
            memcpy( pData, aData.getArray(), aReaded );
        }
#if OSL_DEBUG_LEVEL > 1
        catch( Exception & e )
        {
            DBG_ASSERT( FALSE, ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
#else
        catch( Exception & )
        {
#endif
        }

        if( aResult < nSize )
            m_bSourceRead = FALSE;
    }

    return aResult;
}

ULONG UCBStorageStream_Impl::PutData( const void* pData, ULONG nSize )
{
    if ( !(m_nMode & STREAM_WRITE) )
    {
        SetError( ERRCODE_IO_ACCESSDENIED );
        return 0; // ?mav?
    }

    if( !nSize || !Init() )
        return 0;

    ULONG aResult = m_pStream->Write( pData, nSize );

    m_bModified = aResult > 0;

    return aResult;

}

ULONG UCBStorageStream_Impl::SeekPos( ULONG nPos )
{
    if( !Init() )
        return 0;

    ULONG aResult;

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
                        m_bSourceRead = FALSE;

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

void  UCBStorageStream_Impl::SetSize( ULONG nSize )
{
    if ( !(m_nMode & STREAM_WRITE) )
    {
        SetError( ERRCODE_IO_ACCESSDENIED );
        return;
    }

    if( !Init() )
        return;

    m_bModified = TRUE;

    if( m_bSourceRead )
    {
        ULONG aPos = m_pStream->Tell();
        m_pStream->Seek( STREAM_SEEK_TO_END );
        if( m_pStream->Tell() < nSize )
            ReadSourceWriteTemporary( nSize - m_pStream->Tell() );
        m_pStream->Seek( aPos );
    }

    m_pStream->SetStreamSize( nSize );
    m_bSourceRead = FALSE;
}

void  UCBStorageStream_Impl::FlushData()
{
    if( m_pStream )
    {
        CopySourceToTemporary();
        m_pStream->Flush();
    }

    m_bCommited = TRUE;
}

void UCBStorageStream_Impl::SetError( long nErr )
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

ULONG UCBStorageStream_Impl::GetSize()
{
    if( !Init() )
        return 0;

    ULONG nPos = m_pStream->Tell();
    m_pStream->Seek( STREAM_SEEK_TO_END );
    ReadSourceWriteTemporary();
    ULONG nRet = m_pStream->Tell();
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

                // create wrapper to stream that is only used while reading inside package component
                DBG_ASSERT( m_aTempURL.Len(), "No temporary file to read from!");
                Reference < XInputStream > xStream = new FileStreamWrapper_Impl( m_aTempURL );

                Any aAny;
                InsertCommandArgument aArg;
                aArg.Data = xStream;
                aArg.ReplaceExisting = sal_True;
                aAny <<= aArg;
                m_pContent->executeCommand( ::rtl::OUString::createFromAscii("insert"), aAny );

                // wrapper now controls lifetime of temporary file
                m_aTempURL.Erase();

                INetURLObject aObj( m_aURL );
                aObj.SetName( m_aName );
                m_aURL = aObj.GetMainURL( INetURLObject::NO_DECODE );
                m_bModified = FALSE;
                m_bSourceRead = TRUE;
            }
            catch ( CommandAbortedException& )
            {
                // any command wasn't executed successfully - not specified
                SetError( ERRCODE_IO_GENERAL );
                return COMMIT_RESULT_FAILURE;
            }
            catch ( RuntimeException& )
            {
                // any other error - not specified
                SetError( ERRCODE_IO_GENERAL );
                return COMMIT_RESULT_FAILURE;
            }
            catch ( Exception& )
            {
                // any other error - not specified
                SetError( ERRCODE_IO_GENERAL );
                return COMMIT_RESULT_FAILURE;
            }

            m_bCommited = FALSE;
            return COMMIT_RESULT_SUCCESS;
        }
    }

    return COMMIT_RESULT_NOTHING_TO_DO;
}

BOOL UCBStorageStream_Impl::Revert()
{
    // if an OLEStorage is created on this stream, no "revert" is neccessary because OLEStorages do nothing on "Revert" !
    if ( m_bCommited )
    {
        DBG_ERROR("Revert while commit is in progress!" )
        return FALSE;                   //  ???
    }

    Free();
    if ( m_aTempURL.Len() )
    {
        ::utl::UCBContentHelper::Kill( m_aTempURL );
        m_aTempURL.Erase();
    }

    m_bSourceRead = FALSE;
    try
    {
        m_rSource = m_pContent->openStream();
        if( m_rSource.is() )
        {
            if ( m_pAntiImpl && ( m_nMode & STREAM_TRUNC ) )
                // stream is in use and should be truncated
                m_bSourceRead = FALSE;
            else
            {
                m_nMode &= ~STREAM_TRUNC;
                m_bSourceRead = TRUE;
            }
        }
        else
            SetError( SVSTREAM_CANNOT_MAKE );
    }
    catch ( ContentCreationException& )
    {
        SetError( ERRCODE_IO_GENERAL );
    }
    catch ( RuntimeException& )
    {
        SetError( ERRCODE_IO_GENERAL );
    }
    catch ( Exception& )
    {
    }

    m_bModified = FALSE;
    m_aName = m_aOriginalName;
    m_aContentType = m_aOriginalContentType;
    return ( GetError() == ERRCODE_NONE );
}

BOOL UCBStorageStream_Impl::Clear()
{
    BOOL bRet = ( m_pAntiImpl == NULL );
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
    sal_Bool bIsWritable = ( m_nMode & STREAM_WRITE );
    if ( bIsWritable )
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

UCBStorageStream::UCBStorageStream( const String& rName, StreamMode nMode, BOOL bDirect, const ByteString* pKey )
{
    // pImp must be initialized in the body, because otherwise the vtable of the stream is not initialized
    // to class UCBStorageStream !
    pImp = new UCBStorageStream_Impl( rName, nMode, this, bDirect, pKey );
    pImp->AddRef();             // use direct refcounting because in header file only a pointer should be used
    StorageBase::nMode = pImp->m_nMode;
}

UCBStorageStream::UCBStorageStream( const String& rName, StreamMode nMode, BOOL bDirect, const ByteString* pKey, BOOL bRepair, Reference< XProgressHandler > xProgress )
{
    // pImp must be initialized in the body, because otherwise the vtable of the stream is not initialized
    // to class UCBStorageStream !
    pImp = new UCBStorageStream_Impl( rName, nMode, this, bDirect, pKey, bRepair, xProgress );
    pImp->AddRef();             // use direct refcounting because in header file only a pointer should be used
    StorageBase::nMode = pImp->m_nMode;
}

UCBStorageStream::UCBStorageStream( UCBStorageStream_Impl *pImpl )
    : pImp( pImpl )
{
    pImp->AddRef();             // use direct refcounting because in header file only a pointer should be used
    pImp->m_pAntiImpl = this;
    SetError( pImp->m_nError );
    StorageBase::nMode = pImp->m_nMode;
}

UCBStorageStream::~UCBStorageStream()
{
    if ( pImp->m_nMode & STREAM_WRITE )
        pImp->Flush();
    pImp->m_pAntiImpl = NULL;
    pImp->Free();
    pImp->ReleaseRef();
}

ULONG UCBStorageStream::Read( void * pData, ULONG nSize )
{
    //return pImp->m_pStream->Read( pData, nSize );
    return pImp->GetData( pData, nSize );
}

ULONG UCBStorageStream::Write( const void* pData, ULONG nSize )
{
/*
    // mba: does occur in writer !
    if ( pImp->m_bCommited )
    {
        DBG_ERROR("Writing while commit is in progress!" )
        return 0;
    }
*/
    pImp->m_bModified = TRUE;
    //return pImp->m_pStream->Write( pData, nSize );
    return pImp->PutData( pData, nSize );
}

ULONG UCBStorageStream::Seek( ULONG nPos )
{
    //return pImp->m_pStream->Seek( nPos );
    return pImp->Seek( nPos );
}

ULONG UCBStorageStream::Tell()
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

BOOL UCBStorageStream::SetSize( ULONG nNewSize )
{
/*
    if ( pImp->m_bCommited )
    {
        DBG_ERROR("Changing stream size while commit is in progress!" )
        return FALSE;
    }
*/
    pImp->m_bModified = TRUE;
    //return pImp->m_pStream->SetStreamSize( nNewSize );
    pImp->SetSize( nNewSize );
    return !pImp->GetError();
}

BOOL UCBStorageStream::Validate( BOOL bWrite ) const
{
    return ( !bWrite || ( pImp->m_nMode & STREAM_WRITE ) );
}

BOOL UCBStorageStream::ValidateMode( StreamMode m ) const
{
    // ???
    if( m == ( STREAM_READ | STREAM_TRUNC ) )  // from stg.cxx
        return TRUE;
    USHORT nCurMode = 0xFFFF;
    if( ( m & 3 ) == STREAM_READ )
    {
        // only SHARE_DENYWRITE or SHARE_DENYALL allowed
        if( ( ( m & STREAM_SHARE_DENYWRITE )
           && ( nCurMode & STREAM_SHARE_DENYWRITE ) )
         || ( ( m & STREAM_SHARE_DENYALL )
           && ( nCurMode & STREAM_SHARE_DENYALL ) ) )
            return TRUE;
    }
    else
    {
        // only SHARE_DENYALL allowed
        // storages open in r/o mode are OK, since only
        // the commit may fail
        if( ( m & STREAM_SHARE_DENYALL )
         && ( nCurMode & STREAM_SHARE_DENYALL ) )
            return TRUE;
    }

    return TRUE;
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

BOOL  UCBStorageStream::Equals( const BaseStorageStream& rStream ) const
{
    // ???
    return ((BaseStorageStream*) this ) == &rStream;
}

BOOL UCBStorageStream::Commit()
{
    // mark this stream for sending it on root commit
    pImp->FlushData();
    return TRUE;
}

BOOL UCBStorageStream::Revert()
{
    return pImp->Revert();
}

BOOL UCBStorageStream::CopyTo( BaseStorageStream* pDestStm )
{
    if( !pImp->Init() )
        return FALSE;

    UCBStorageStream* pStg = PTR_CAST( UCBStorageStream, pDestStm );
    if ( pStg )
        pStg->pImp->m_aContentType = pImp->m_aContentType;

    pDestStm->SetSize( 0 );
    Seek( STREAM_SEEK_TO_END );
    INT32 n = Tell();
    if( n < 0 )
        return FALSE;

    if( pDestStm->SetSize( n ) && n )
    {
        BYTE* p = new BYTE[ 4096 ];
        Seek( 0L );
        pDestStm->Seek( 0L );
        while( n )
        {
            UINT32 nn = n;
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

    return TRUE;
}

BOOL UCBStorageStream::SetProperty( const String& rName, const ::com::sun::star::uno::Any& rValue )
{
    if ( rName.CompareToAscii("Title") == COMPARE_EQUAL )
        return FALSE;

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
            return TRUE;
        }
    }
    catch ( Exception& )
    {
    }

    return FALSE;
}

BOOL UCBStorageStream::GetProperty( const String& rName, ::com::sun::star::uno::Any& rValue )
{
    try
    {
        if ( pImp->m_pContent )
        {
            rValue = pImp->m_pContent->getPropertyValue( rName );
            return TRUE;
        }
    }
    catch ( Exception& )
    {
    }

    return FALSE;
}

UCBStorage::UCBStorage( SvStream& rStrm, BOOL bDirect )
{
    String aURL = GetLinkedFile( rStrm );
    if ( aURL.Len() )
    {
        StreamMode nMode = STREAM_READ;
        if( rStrm.IsWritable() )
            nMode = STREAM_READ | STREAM_WRITE;

        ::ucb::Content aContent( aURL, Reference < XCommandEnvironment >() );
        pImp = new UCBStorage_Impl( aContent, aURL, nMode, this, bDirect, TRUE );
    }
    else
    {
        // pImp must be initialized in the body, because otherwise the vtable of the stream is not initialized
        // to class UCBStorage !
        pImp = new UCBStorage_Impl( rStrm, this, bDirect );
    }

    pImp->AddRef();
    pImp->Init();
    StorageBase::nMode = pImp->m_nMode;
}

UCBStorage::UCBStorage( const ::ucb::Content& rContent, const String& rName, StreamMode nMode, BOOL bDirect, BOOL bIsRoot )
{
    // pImp must be initialized in the body, because otherwise the vtable of the stream is not initialized
    // to class UCBStorage !
    pImp = new UCBStorage_Impl( rContent, rName, nMode, this, bDirect, bIsRoot );
    pImp->AddRef();
    pImp->Init();
    StorageBase::nMode = pImp->m_nMode;
}

UCBStorage::UCBStorage( const String& rName, StreamMode nMode, BOOL bDirect, BOOL bIsRoot, BOOL bIsRepair, Reference< XProgressHandler > xProgressHandler )
{
    // pImp must be initialized in the body, because otherwise the vtable of the stream is not initialized
    // to class UCBStorage !
    pImp = new UCBStorage_Impl( rName, nMode, this, bDirect, bIsRoot, bIsRepair, xProgressHandler );
    pImp->AddRef();
    pImp->Init();
    StorageBase::nMode = pImp->m_nMode;
}

UCBStorage::UCBStorage( const String& rName, StreamMode nMode, BOOL bDirect, BOOL bIsRoot )
{
    // pImp must be initialized in the body, because otherwise the vtable of the stream is not initialized
    // to class UCBStorage !
    pImp = new UCBStorage_Impl( rName, nMode, this, bDirect, bIsRoot, sal_False, Reference< XProgressHandler >() );
    pImp->AddRef();
    pImp->Init();
    StorageBase::nMode = pImp->m_nMode;
}

UCBStorage::UCBStorage( UCBStorage_Impl *pImpl )
    : pImp( pImpl )
{
    pImp->m_pAntiImpl = this;
    SetError( pImp->m_nError );
    pImp->AddRef();             // use direct refcounting because in header file only a pointer should be used
    StorageBase::nMode = pImp->m_nMode;
}

UCBStorage::~UCBStorage()
{
    if ( pImp->m_bIsRoot && pImp->m_bDirect && ( !pImp->m_pTempFile || pImp->m_pSource ) )
        // DirectMode is simulated with an AutoCommit
        Commit();

    pImp->m_pAntiImpl = NULL;
    pImp->ReleaseRef();
}

UCBStorage_Impl::UCBStorage_Impl( const ::ucb::Content& rContent, const String& rName, StreamMode nMode, UCBStorage* pStorage, BOOL bDirect, BOOL bIsRoot, BOOL bIsRepair, Reference< XProgressHandler > xProgressHandler  )
    : m_pAntiImpl( pStorage )
    , m_pTempFile( NULL )
    , m_pContent( new ::ucb::Content( rContent ) )
    , m_pSource( NULL )
    //, m_pStream( NULL )
    , m_nError( 0 )
    , m_nMode( nMode )
    , m_nFormat( 0 )
    , m_bIsRoot( bIsRoot )
    , m_bDirect( bDirect )
    , m_bModified( FALSE )
    , m_bCommited( FALSE )
    , m_bDirty( FALSE )
    , m_bIsLinked( TRUE )
    , m_bListCreated( FALSE )
    , m_aClassId( SvGlobalName() )
    , m_bRepairPackage( bIsRepair )
    , m_xProgressHandler( xProgressHandler )
    , m_pUNOStorageHolderList( NULL )

{
    String aName( rName );
    if( !aName.Len() )
    {
        // no name given = use temporary name!
        DBG_ASSERT( m_bIsRoot, "SubStorage must have a name!" );
        m_pTempFile = new ::utl::TempFile;
        m_pTempFile->EnableKillingFile( TRUE );
        m_aName = m_aOriginalName = aName = m_pTempFile->GetURL();
    }

    m_aURL = rName;
}

UCBStorage_Impl::UCBStorage_Impl( const String& rName, StreamMode nMode, UCBStorage* pStorage, BOOL bDirect, BOOL bIsRoot, BOOL bIsRepair, Reference< XProgressHandler > xProgressHandler )
    : m_pAntiImpl( pStorage )
    , m_pTempFile( NULL )
    , m_pContent( NULL )
    , m_pSource( NULL )
    //, m_pStream( NULL )
    , m_nError( 0 )
    , m_nMode( nMode )
    , m_nFormat( 0 )
    , m_bIsRoot( bIsRoot )
    , m_bDirect( bDirect )
    , m_bModified( FALSE )
    , m_bCommited( FALSE )
    , m_bDirty( FALSE )
    , m_bIsLinked( FALSE )
    , m_bListCreated( FALSE )
    , m_aClassId( SvGlobalName() )
    , m_bRepairPackage( bIsRepair )
    , m_xProgressHandler( xProgressHandler )
    , m_pUNOStorageHolderList( NULL )
{
    String aName( rName );
    if( !aName.Len() )
    {
        // no name given = use temporary name!
        DBG_ASSERT( m_bIsRoot, "SubStorage must have a name!" );
        m_pTempFile = new ::utl::TempFile;
        m_pTempFile->EnableKillingFile( TRUE );
        m_aName = m_aOriginalName = aName = m_pTempFile->GetURL();
    }

    if ( m_bIsRoot )
    {
        // create the special package URL for the package content
        String aTemp = String::CreateFromAscii("vnd.sun.star.pkg://");
        aTemp += INetURLObject::encode( aName, INetURLObject::PART_AUTHORITY, '%', INetURLObject::ENCODE_ALL );
        m_aURL = aTemp;

        if ( m_nMode & STREAM_WRITE )
        {
            // the root storage opens the package, so make sure that there is any
            SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( aName, STREAM_STD_READWRITE );
            delete pStream;
        }
    }
    else
    {
        // substorages are opened like streams: the URL is a "child URL" of the root package URL
        m_aURL = rName;
    }
}

UCBStorage_Impl::UCBStorage_Impl( SvStream& rStream, UCBStorage* pStorage, BOOL bDirect )
    : m_pAntiImpl( pStorage )
    , m_pTempFile( new ::utl::TempFile )
    , m_pContent( NULL )
    , m_pSource( &rStream )
    , m_nError( 0 )
    , m_bIsRoot( TRUE )
    , m_bDirect( bDirect )
    , m_nFormat( 0 )
    , m_bDirty( FALSE )
    , m_bIsLinked( FALSE )
    , m_bListCreated( FALSE )
    , m_bModified( FALSE )
    , m_bCommited( FALSE )
    , m_aClassId( SvGlobalName() )
    , m_bRepairPackage( FALSE )
    , m_pUNOStorageHolderList( NULL )
{
    // opening in direct mode is too fuzzy because the data is transferred to the stream in the Commit() call,
    // which will be called in the storages' dtor
    m_pTempFile->EnableKillingFile( TRUE );
    DBG_ASSERT( !bDirect, "Storage on a stream must not be opened in direct mode!" );

    // UCBStorages work on a content, so a temporary file for a content must be created, even if the stream is only
    // accessed readonly
    // the root storage opens the package; create the special package URL for the package content
    String aTemp = String::CreateFromAscii("vnd.sun.star.pkg://");
    aTemp += INetURLObject::encode( m_pTempFile->GetURL(), INetURLObject::PART_AUTHORITY, '%', INetURLObject::ENCODE_ALL );
    m_aURL = aTemp;

    // copy data into the temporary file
    SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( m_pTempFile->GetURL(), STREAM_STD_READWRITE );
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
        m_aContentType = m_aOriginalContentType = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("application/vnd.sun.xml.impress") );
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
                    aObj.Append( String( RTL_CONSTASCII_USTRINGPARAM("META-INF") ) );
                    aObj.Append( String( RTL_CONSTASCII_USTRINGPARAM("manifest.xml") ) );

                    // create input stream
                    SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( aObj.GetMainURL( INetURLObject::NO_DECODE ), STREAM_STD_READ );
                    ::utl::OInputStreamWrapper* pHelper = new ::utl::OInputStreamWrapper( *pStream );
                    com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream > xInputStream( pHelper );

                    // create a manifest reader object that will read in the manifest from the stream
                    Reference < ::com::sun::star::packages::manifest::XManifestReader > xReader =
                        Reference< ::com::sun::star::packages::manifest::XManifestReader >
                            ( ::comphelper::getProcessServiceFactory()->createInstance(
                                ::rtl::OUString::createFromAscii( "com.sun.star.packages.manifest.ManifestReader" )), UNO_QUERY) ;
                    Sequence < Sequence < PropertyValue > > aProps = xReader->readManifestSequence( xInputStream );

                    // cleanup
                    xReader = NULL;
                    xInputStream = NULL;
                    delete pStream;
                    SetProps( aProps, String() );
                }
            }
            else
                ReadContent();
        }
        else
        {
            // get the manifest information from the package
            Any aAny = m_pContent->getPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ) );
            rtl::OUString aTmp;
            if ( ( aAny >>= aTmp ) && aTmp.getLength() )
                m_aContentType = m_aOriginalContentType = aTmp;
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
            xComEnv = new ::ucb::CommandEnvironment( Reference< ::com::sun::star::task::XInteractionHandler >(),
                                                     m_xProgressHandler );
            aTemp += rtl::OUString::createFromAscii("?repairpackage");
        }

        m_pContent = new ::ucb::Content( aTemp, xComEnv );
    }
    catch ( ContentCreationException& )
    {
        // content could not be created
        SetError( SVSTREAM_CANNOT_MAKE );
    }
    catch ( RuntimeException& )
    {
        // any other error - not specified
        SetError( SVSTREAM_CANNOT_MAKE );
    }
}

void UCBStorage_Impl::ReadContent()
{
   if ( m_bListCreated )
        return;

    m_bListCreated = TRUE;

    // create cursor for access to children
    Sequence< ::rtl::OUString > aProps(4);
    ::rtl::OUString* pProps = aProps.getArray();
    pProps[0] = ::rtl::OUString::createFromAscii( "Title" );
    pProps[1] = ::rtl::OUString::createFromAscii( "IsFolder" );
    pProps[2] = ::rtl::OUString::createFromAscii( "MediaType" );
    pProps[3] = ::rtl::OUString::createFromAscii( "Size" );
    ::ucb::ResultSetInclude eInclude = ::ucb::INCLUDE_FOLDERS_AND_DOCUMENTS;

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
                    if( aTitle.equalsAscii("META-INF") )
                        continue;
                }
                else
                {
                    aContentType = xRow->getString(3);
                }

                BOOL bIsFolder( xRow->getBoolean(2) );
                sal_Int64 nSize = xRow->getLong(4);
                UCBStorageElement_Impl* pElement = new UCBStorageElement_Impl( aTitle, aContentType, bIsFolder, (ULONG) nSize );
                m_aChildrenList.Insert( pElement, LIST_APPEND );

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
                        xComEnv = new ::ucb::CommandEnvironment( Reference< ::com::sun::star::task::XInteractionHandler >(),
                                                                 m_xProgressHandler );
                           aName += String( RTL_CONSTASCII_USTRINGPARAM( "?repairpackage" ) );
                    }

                    ::ucb::Content aContent( aName, xComEnv );

                    ::rtl::OUString aMediaType;
                    Any aAny = aContent.getPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ) );
                    if ( ( aAny >>= aMediaType ) && ( aMediaType.compareToAscii("application/vnd.sun.star.oleobject") == 0 ) )
                        pElement->m_bIsStorage = TRUE;
                    else if ( !aMediaType.getLength() )
                    {
                        // older files didn't have that special content type, so they must be detected
                        OpenStream( pElement, STREAM_STD_READ, m_bDirect );
                        if ( Storage::IsStorageFile( pElement->m_xStream ) )
                            pElement->m_bIsStorage = TRUE;
                        else
                            pElement->m_xStream->Free();
                    }
                }
            }
        }
    }
    catch ( InteractiveIOException& r )
    {
        if ( r.Code != IOErrorCode_NOT_EXISTING )
            SetError( ERRCODE_IO_GENERAL );
    }
    catch ( CommandAbortedException& )
    {
        // any command wasn't executed successfully - not specified
        if ( !( m_nMode & STREAM_WRITE ) )
            // if the folder was just inserted and not already commited, this is not an error!
            SetError( ERRCODE_IO_GENERAL );
    }
    catch ( RuntimeException& )
    {
        // any other error - not specified
        SetError( ERRCODE_IO_GENERAL );
    }
    catch ( ResultSetException& )
    {
        // means that the package file is broken
        SetError( ERRCODE_IO_BROKENPACKAGE );
    }
    catch ( SQLException& )
    {
        // means that the file can be broken
        SetError( ERRCODE_IO_WRONGFORMAT );
    }
    catch ( Exception& )
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
    sal_Int32 nCount = m_aChildrenList.Count();
    UCBStorageElement_Impl* pElement = m_aChildrenList.First();
    while ( pElement )
    {
        DBG_ASSERT( !pElement->m_bIsFolder || pElement->m_xStorage.Is(), "Storage should be open!" );
        if ( pElement->m_bIsFolder && pElement->m_xStorage.Is() )
            nCount += pElement->m_xStorage->GetObjectCount();
        pElement = m_aChildrenList.Next();
    }

    return nCount;
}

::rtl::OUString Find_Impl( const Sequence < Sequence < PropertyValue > >& rSequence, const ::rtl::OUString& rPath )
{
    BOOL bFound = FALSE;
    for ( sal_Int32 nSeqs=0; nSeqs<rSequence.getLength(); nSeqs++ )
    {
        const Sequence < PropertyValue >& rMyProps = rSequence[nSeqs];
        ::rtl::OUString aType;

        for ( sal_Int32 nProps=0; nProps<rMyProps.getLength(); nProps++ )
        {
            const PropertyValue& rAny = rMyProps[nProps];
            if ( rAny.Name.equalsAscii("FullPath") )
            {
                rtl::OUString aTmp;
                if ( ( rAny.Value >>= aTmp ) && aTmp == rPath )
                    bFound = TRUE;
                if ( aType.getLength() )
                    break;
            }
            else if ( rAny.Name.equalsAscii("MediaType") )
            {
                if ( ( rAny.Value >>= aType ) && aType.getLength() && bFound )
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

    UCBStorageElement_Impl* pElement = m_aChildrenList.First();
    while ( pElement )
    {
        DBG_ASSERT( !pElement->m_bIsFolder || pElement->m_xStorage.Is(), "Storage should be open!" );
        if ( pElement->m_bIsFolder && pElement->m_xStorage.Is() )
            pElement->m_xStorage->SetProps( rSequence, aPath );
        else
        {
            String aElementPath( aPath );
            aElementPath += pElement->m_aName;
            pElement->SetContentType( Find_Impl( rSequence, aElementPath ) );
        }

        pElement = m_aChildrenList.Next();
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
    aProps[0].Name = ::rtl::OUString::createFromAscii("MediaType");
    aProps[0].Value <<= (::rtl::OUString ) m_aContentType;
    aProps[1].Name = ::rtl::OUString::createFromAscii("FullPath");
    aProps[1].Value <<= (::rtl::OUString ) aPath;
    rSequence[ nProps++ ] = aProps;

    if ( m_bIsRoot )
        // the "FullPath" of a child always starts without '/'
        aPath.Erase();

    // now the properties of my elements
    UCBStorageElement_Impl* pElement = m_aChildrenList.First();
    while ( pElement )
    {
        DBG_ASSERT( !pElement->m_bIsFolder || pElement->m_xStorage.Is(), "Storage should be open!" );
        if ( pElement->m_bIsFolder && pElement->m_xStorage.Is() )
            // storages add there properties by themselves ( see above )
            pElement->m_xStorage->GetProps( nProps, rSequence, aPath );
        else
        {
            // properties of streams
            Sequence < PropertyValue > aProps(2);
            String aElementPath( aPath );
            aElementPath += pElement->m_aName;
            aProps[0].Name = ::rtl::OUString::createFromAscii("MediaType");
            aProps[0].Value <<= (::rtl::OUString ) pElement->GetContentType();
            aProps[1].Name = ::rtl::OUString::createFromAscii("FullPath");
            aProps[1].Value <<= (::rtl::OUString ) aElementPath;
            rSequence[ nProps++ ] = aProps;
        }

        pElement = m_aChildrenList.Next();
    }
}

UCBStorage_Impl::~UCBStorage_Impl()
{
    if ( m_pUNOStorageHolderList )
    {
        for ( UNOStorageHolderList::iterator aIter = m_pUNOStorageHolderList->begin();
              aIter != m_pUNOStorageHolderList->end(); aIter++ )
            if ( *aIter )
            {
                (*aIter)->InternalDispose();
                (*aIter)->release();
                (*aIter) = NULL;
            }

        m_pUNOStorageHolderList->clear();
        DELETEZ( m_pUNOStorageHolderList );
    }

    // first delete elements!
    UCBStorageElement_Impl* pElement = m_aChildrenList.First();
    while ( pElement )
    {
        delete pElement;
        pElement = m_aChildrenList.Next();
    }

    m_aChildrenList.Clear();
    delete m_pContent;
    delete m_pTempFile;
}

BOOL UCBStorage_Impl::Insert( ::ucb::Content *pContent )
{
    // a new substorage is inserted into a UCBStorage ( given by the parameter pContent )
    // it must be inserted with a title and a type
    BOOL bRet = FALSE;
    Reference< XContentCreator > xCreator = Reference< XContentCreator >( pContent->get(), UNO_QUERY );
    if ( !xCreator.is() )
        return sal_False;

    try
    {
        Sequence< ContentInfo > aInfo = xCreator->queryCreatableContentsInfo();
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

                if ( !rProps[ 0 ].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
                    continue;

                Sequence < ::rtl::OUString > aNames(1);
                ::rtl::OUString* pNames = aNames.getArray();
                pNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) );
                Sequence < Any > aValues(1);
                Any* pValues = aValues.getArray();
                pValues[0] = makeAny( ::rtl::OUString( m_aName ) );

                Content aNewFolder;
                if ( !pContent->insertNewContent( rCurr.Type, aNames, aValues, aNewFolder ) )
                    continue;

                // remove old content, create an "empty" new one and initialize it with the new inserted
                DELETEZ( m_pContent );
                m_pContent = new ::ucb::Content( aNewFolder );
                bRet = TRUE;
            }
        }
    }
    catch ( CommandAbortedException& )
    {
        // any command wasn't executed successfully - not specified
        SetError( ERRCODE_IO_GENERAL );
    }
    catch ( RuntimeException& )
    {
        // any other error - not specified
        SetError( ERRCODE_IO_GENERAL );
    }
    catch ( Exception& )
    {
        // any other error - not specified
        SetError( ERRCODE_IO_GENERAL );
    }

    return bRet;
}

sal_Int16 UCBStorage_Impl::Commit()
{
    // send all changes to the package
    UCBStorageElement_Impl* pElement = m_aChildrenList.First();
    sal_Int16 nRet = COMMIT_RESULT_NOTHING_TO_DO;

    // there is nothing to do if the storage has been opened readonly or if it was opened in transacted mode and no
    // commit command has been sent
    if ( ( m_nMode & STREAM_WRITE ) && ( m_bCommited || m_bDirect ) )
    {
        try
        {
            // all errors will be caught in the "catch" statement outside the loop
            while ( pElement && nRet )
            {
                ::ucb::Content* pContent = pElement->GetContent();
                BOOL bDeleteContent = FALSE;
                if ( !pContent && pElement->IsModified() )
                {
                    // if the element has never been opened, no content has been created until now
                    bDeleteContent = TRUE;  // remember to delete it later
                    String aName( m_aURL );
                    aName += '/';
                    aName += pElement->m_aOriginalName;
                    pContent = new ::ucb::Content( aName, Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
                }

                if ( pElement->m_bIsRemoved )
                {
                    // was it inserted, then removed (so there would be nothing to do!)
                    if ( !pElement->m_bIsInserted )
                    {
                        // first remove all open stream handles
                        if( !pElement->m_xStream.Is() || pElement->m_xStream->Clear() )
                        {
                            pContent->executeCommand( ::rtl::OUString::createFromAscii("delete"), makeAny( sal_Bool( sal_True ) ) );
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
                            pElement->m_xStream->m_aContentType = String::CreateFromAscii("application/vnd.sun.star.oleobject");
                            Any aValue;
                            aValue <<= (BOOL) TRUE;
                            pElement->m_xStream->m_pContent->setPropertyValue(String::CreateFromAscii("Encrypted"), aValue );
                        }

                        pContent = pElement->GetContent();
                    }

                    if ( pElement->m_aName != pElement->m_aOriginalName )
                    {
                        // name ( title ) of the element was changed
                        nLocalRet = COMMIT_RESULT_SUCCESS;
                        Any aAny;
                        aAny <<= (rtl::OUString) pElement->m_aName;
                        pContent->setPropertyValue( ::rtl::OUString::createFromAscii("Title"), aAny );
                    }

                    if ( pElement->IsLoaded() && pElement->GetContentType() != pElement->GetOriginalContentType() )
                    {
                        // mediatype of the element was changed
                        nLocalRet = COMMIT_RESULT_SUCCESS;
                        Any aAny;
                        aAny <<= (rtl::OUString) pElement->GetContentType();
                        pContent->setPropertyValue( ::rtl::OUString::createFromAscii("MediaType"), aAny );
                    }

                    if ( nLocalRet != COMMIT_RESULT_NOTHING_TO_DO )
                        nRet = nLocalRet;
                }

                if ( bDeleteContent )
                    // content was created inside the loop
                    delete pContent;

                if ( nRet == COMMIT_RESULT_FAILURE )
                    break;

                pElement = m_aChildrenList.Next();
            }
        }
        catch ( ContentCreationException& )
        {
            // content could not be created
            SetError( ERRCODE_IO_NOTEXISTS );
            return COMMIT_RESULT_FAILURE;
        }
        catch ( CommandAbortedException& )
        {
            // any command wasn't executed successfully - not specified
            SetError( ERRCODE_IO_GENERAL );
            return COMMIT_RESULT_FAILURE;
        }
        catch ( RuntimeException& )
        {
            // any other error - not specified
            SetError( ERRCODE_IO_GENERAL );
            return COMMIT_RESULT_FAILURE;
        }
        catch ( Exception& )
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
                    m_pContent->setPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ), aType );

                    if (  m_bIsLinked )
                    {
                        // write a manifest file
                        // first create a subfolder "META-inf"
                        Content aNewSubFolder;
                        BOOL bRet = ::utl::UCBContentHelper::MakeFolder( *m_pContent, String::CreateFromAscii("META-INF"), aNewSubFolder );
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
                                        ::rtl::OUString::createFromAscii( "com.sun.star.packages.manifest.ManifestWriter" )), UNO_QUERY) ;
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
                            aNewSubFolder.transferContent( aSource, InsertOperation_MOVE, ::rtl::OUString::createFromAscii("manifest.xml"), NameClash::OVERWRITE );
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
                        m_pContent->executeCommand( ::rtl::OUString::createFromAscii("flush"), aAny );
                        if ( m_pSource != 0 )
                        {
                            SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( m_pTempFile->GetURL(), STREAM_STD_READ );
                            m_pSource->Seek(0);
                            *pStream >> *m_pSource;
                            DELETEZ( pStream );
                            m_pSource->Seek(0);
                        }
                    }
                }
                catch ( CommandAbortedException& )
                {
                    // how to tell the content : forget all changes ?!
                    // or should we assume that the content does it by itself because he throwed an exception ?!
                    // any command wasn't executed successfully - not specified
                    SetError( ERRCODE_IO_GENERAL );
                    return COMMIT_RESULT_FAILURE;
                }
                catch ( RuntimeException& )
                {
                    // how to tell the content : forget all changes ?!
                    // or should we assume that the content does it by itself because he throwed an exception ?!
                    // any other error - not specified
                    SetError( ERRCODE_IO_GENERAL );
                    return COMMIT_RESULT_FAILURE;
                }
                catch ( InteractiveIOException& r )
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
                catch ( Exception& )
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
            UCBStorageElement_Impl* pElement = m_aChildrenList.First();
            BOOL bRet = TRUE;
            while ( pElement && bRet )
            {
                UCBStorageElement_Impl* pNext = m_aChildrenList.Next();
                if ( pElement->m_bIsRemoved )
                {
                    // is this correct use of our list class ?!
                    m_aChildrenList.Remove( pElement );
                }
                else
                {
                    pElement->m_aOriginalName = pElement->m_aName;
                    pElement->m_bIsInserted = FALSE;
                }

                pElement = pNext;
            }
        }

        m_bCommited = FALSE;
    }

    return nRet;
}

BOOL UCBStorage_Impl::Revert()
{
    UCBStorageElement_Impl* pElement = m_aChildrenList.First();
    BOOL bRet = TRUE;
    while ( pElement && bRet )
    {
        pElement->m_bIsRemoved = FALSE;
        if ( pElement->m_bIsInserted )
        {
            m_aChildrenList.Remove( pElement );  // correct usage of list ???
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
            pElement->m_bIsRemoved = FALSE;
        }

        pElement = m_aChildrenList.Next();
    }

    return bRet;
}

const String& UCBStorage::GetName() const
{
    return pImp->m_aName;               // pImp->m_aURL ?!
}

BOOL UCBStorage::IsRoot() const
{
    return pImp->m_bIsRoot;
}

void UCBStorage::SetDirty()
{
    pImp->m_bDirty = TRUE;
}

void UCBStorage::SetClass( const SvGlobalName & rClass, ULONG nOriginalClipFormat, const String & rUserTypeName )
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

void UCBStorage::SetConvertClass( const SvGlobalName & rConvertClass, ULONG nOriginalClipFormat, const String & rUserTypeName )
{
    // ???
}

BOOL UCBStorage::ShouldConvert()
{
    // ???
    return FALSE;
}

SvGlobalName UCBStorage::GetClassName()
{
    return  pImp->m_aClassId;
}

ULONG UCBStorage::GetFormat()
{
    return pImp->m_nFormat;
}

String UCBStorage::GetUserName()
{
    DBG_ERROR("UserName is not implemented in UCB storages!" );
    return pImp->m_aUserTypeName;
}

void UCBStorage::FillInfoList( SvStorageInfoList* pList ) const
{
    // put information in childrenlist into StorageInfoList
    UCBStorageElement_Impl* pElement = pImp->GetChildrenList().First();
    while ( pElement )
    {
        if ( !pElement->m_bIsRemoved )
        {
            // problem: what about the size of a substorage ?!
            ULONG nSize = pElement->m_nSize;
            if ( pElement->m_xStream.Is() )
                nSize = pElement->m_xStream->GetSize();
            SvStorageInfo aInfo( pElement->m_aName, nSize, pElement->m_bIsStorage );
            pList->Append( aInfo );
        }

        pElement = pImp->m_aChildrenList.Next();
    }
}

BOOL UCBStorage::CopyStorageElement_Impl( UCBStorageElement_Impl& rElement, BaseStorage* pDest, const String& rNew ) const
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
        BOOL bDeleteStream = FALSE;

        // if stream is already open, it is allowed to copy it, so be aware of this
        if ( rElement.m_xStream.Is() )
            pStream = rElement.m_xStream->m_pAntiImpl;
        if ( !pStream )
        {
            pStream = ( const_cast < UCBStorage* > (this) )->OpenStream( rElement.m_aName, STREAM_STD_READ, pImp->m_bDirect );
            bDeleteStream = TRUE;
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
        BOOL bDeleteStorage = FALSE;
        if ( rElement.m_xStorage.Is() )
            pStorage = rElement.m_xStorage->m_pAntiImpl;
        if ( !pStorage )
        {
            pStorage = ( const_cast < UCBStorage* > (this) )->OpenStorage( rElement.m_aName, pImp->m_nMode, pImp->m_bDirect );
            bDeleteStorage = TRUE;
        }

        UCBStorage* pUCBDest = PTR_CAST( UCBStorage, pDest );
        UCBStorage* pUCBCopy = PTR_CAST( UCBStorage, pStorage );

        BOOL bOpenUCBStorage = pUCBDest && pUCBCopy;
        BaseStorage* pOtherStorage = bOpenUCBStorage ?
                pDest->OpenUCBStorage( rNew, STREAM_WRITE | STREAM_SHARE_DENYALL, pImp->m_bDirect ) :
                pDest->OpenOLEStorage( rNew, STREAM_WRITE | STREAM_SHARE_DENYALL, pImp->m_bDirect );

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

    return BOOL( Good() && pDest->Good() );
}

UCBStorageElement_Impl* UCBStorage::FindElement_Impl( const String& rName ) const
{
    DBG_ASSERT( rName.Len(), "Name is empty!" );
    UCBStorageElement_Impl* pElement = pImp->GetChildrenList().First();
    while ( pElement )
    {
        if ( pElement->m_aName == rName && !pElement->m_bIsRemoved )
            break;
        pElement = pImp->m_aChildrenList.Next();
    }

    return pElement;
}

BOOL UCBStorage::CopyTo( BaseStorage* pDestStg ) const
{
    DBG_ASSERT( pDestStg != ((BaseStorage*)this), "Self-Copying is not possible!" );
    if ( pDestStg == ((BaseStorage*)this) )
        return FALSE;

    // perhaps it's also a problem if one storage is a parent of the other ?!
    // or if not: could be optimized ?!

    pDestStg->SetClassId( GetClassId() );
    pDestStg->SetDirty();

    BOOL bRet = TRUE;
    UCBStorageElement_Impl* pElement = pImp->GetChildrenList().First();
    while ( pElement && bRet )
    {
        if ( !pElement->m_bIsRemoved )
            bRet = CopyStorageElement_Impl( *pElement, pDestStg, pElement->m_aName );
        pElement = pImp->m_aChildrenList.Next();
    }

    if( !bRet )
        SetError( pDestStg->GetError() );
    return BOOL( Good() && pDestStg->Good() );
}

BOOL UCBStorage::CopyTo( const String& rElemName, BaseStorage* pDest, const String& rNew )
{
    if( !rElemName.Len() )
        return NULL;

    if ( pDest == ((BaseStorage*) this) )
    {
        // can't double an element
        return FALSE;
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
            return FALSE;
        }
    }
}

BOOL UCBStorage::Commit()
{
    // mark this storage for sending it on root commit
    pImp->m_bCommited = TRUE;
    if ( pImp->m_bIsRoot )
        // the root storage coordinates commiting by sending a Commit command to its content
        return ( pImp->Commit() != COMMIT_RESULT_FAILURE );
    else
        return TRUE;
}

BOOL UCBStorage::Revert()
{
    return pImp->Revert();
}

BaseStorageStream* UCBStorage::OpenStream( const String& rEleName, StreamMode nMode, BOOL bDirect, const ByteString* pKey )
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
            pElement->m_bIsInserted = TRUE;
            pImp->m_aChildrenList.Insert( pElement, LIST_APPEND );
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
                DBG_ERROR("Stream is already open!" );
                SetError( SVSTREAM_ACCESS_DENIED );  // ???
                return NULL;
            }
            else
            {
                // check if stream is opened with the same keyword as before
                // if not, generate a new stream because it could be encrypted vs. decrypted!
                ByteString aKey;
                if ( pKey )
                    aKey = *pKey;
                if ( pElement->m_xStream->m_aKey == aKey )
                {
                    pElement->m_xStream->PrepareCachedForReopen( nMode );

    //              DBG_ASSERT( bDirect == pElement->m_xStream->m_bDirect, "Wrong DirectMode!" );
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

UCBStorageStream_Impl* UCBStorage_Impl::OpenStream( UCBStorageElement_Impl* pElement, StreamMode nMode, BOOL bDirect, const ByteString* pKey )
{
    String aName( m_aURL );
    aName += '/';
    aName += pElement->m_aOriginalName;
    pElement->m_xStream = new UCBStorageStream_Impl( aName, nMode, NULL, bDirect, pKey, m_bRepairPackage, m_xProgressHandler );
    return pElement->m_xStream;
}

BaseStorage* UCBStorage::OpenUCBStorage( const String& rEleName, StreamMode nMode, BOOL bDirect )
{
    if( !rEleName.Len() )
        return NULL;

    return OpenStorage_Impl( rEleName, nMode, bDirect, TRUE );
}

BaseStorage* UCBStorage::OpenOLEStorage( const String& rEleName, StreamMode nMode, BOOL bDirect )
{
    if( !rEleName.Len() )
        return NULL;

    return OpenStorage_Impl( rEleName, nMode, bDirect, FALSE );
}

BaseStorage* UCBStorage::OpenStorage( const String& rEleName, StreamMode nMode, BOOL bDirect )
{
    if( !rEleName.Len() )
        return NULL;

    return OpenStorage_Impl( rEleName, nMode, bDirect, TRUE );
}

BaseStorage* UCBStorage::OpenStorage_Impl( const String& rEleName, StreamMode nMode, BOOL bDirect, BOOL bForceUCBStorage )
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
            UCBStorage *pStorage = new UCBStorage( aName, nMode, bDirect, FALSE, pImp->m_bRepairPackage, pImp->m_xProgressHandler );
            pStorage->pImp->m_bIsRoot = FALSE;
            pStorage->pImp->m_bListCreated = sal_True; // the storage is pretty new, nothing to read
            pStorage->SetError( GetError() );
            return pStorage;
        }

        // create a new UCBStorageElement and insert it into the list
        // problem: perhaps an OLEStorage should be created ?!
        // Because nothing is known about the element that should be created, an external parameter is needed !
        pElement = new UCBStorageElement_Impl( rEleName );
        pElement->m_bIsInserted = TRUE;
        pImp->m_aChildrenList.Insert( pElement, LIST_APPEND );
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

        pElement->m_bIsStorage = TRUE;
        return pElement->m_xStream->CreateStorage();  // can only be created in transacted mode
    }
    else if ( pElement->m_xStorage.Is() )
    {
        // storage has already been opened; if it has no external reference, it may be opened another time
        if ( pElement->m_xStorage->m_pAntiImpl )
        {
            DBG_ERROR("Storage is already open!" );
            SetError( SVSTREAM_ACCESS_DENIED );  // ???
        }
        else
        {
            BOOL bIsWritable = ( pElement->m_xStorage->m_nMode & STREAM_WRITE );
            if ( !bIsWritable && ( nMode & STREAM_WRITE ) )
            {
                String aName( pImp->m_aURL );
                aName += '/';
                aName += pElement->m_aOriginalName;
                UCBStorage* pStorage = new UCBStorage( aName, nMode, bDirect, FALSE, pImp->m_bRepairPackage, pImp->m_xProgressHandler );
                pElement->m_xStorage = pStorage->pImp;
                return pStorage;
            }
            else
            {
//                    DBG_ASSERT( bDirect == pElement->m_xStorage->m_bDirect, "Wrong DirectMode!" );
                return new UCBStorage( pElement->m_xStorage );
            }
        }
    }
    else if ( !pElement->m_xStream.Is() )
    {
        // storage is opened the first time
        BOOL bIsWritable = ( pImp->m_nMode & STREAM_WRITE );
        if ( pImp->m_bIsLinked && pImp->m_bIsRoot && bIsWritable )
        {
            // make sure that the root storage object has been created before substorages will be created
            INetURLObject aFolderObj( pImp->m_aURL );
            String aName = aFolderObj.GetName();
            aFolderObj.removeSegment();

            Content aFolder( aFolderObj.GetMainURL( INetURLObject::NO_DECODE ), Reference < XCommandEnvironment >() );
            pImp->m_pContent = new Content;
            BOOL bRet = ::utl::UCBContentHelper::MakeFolder( aFolder, pImp->m_aName, *pImp->m_pContent );
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

UCBStorage_Impl* UCBStorage_Impl::OpenStorage( UCBStorageElement_Impl* pElement, StreamMode nMode, BOOL bDirect )
{
    UCBStorage_Impl* pRet = NULL;
    String aName( m_aURL );
    aName += '/';
    aName += pElement->m_aOriginalName;  //  ???

    pElement->m_bIsStorage = pElement->m_bIsFolder = TRUE;

    if ( m_bIsLinked && !::utl::UCBContentHelper::Exists( aName ) )
    {
        Content aNewFolder;
        BOOL bRet = ::utl::UCBContentHelper::MakeFolder( *m_pContent, pElement->m_aOriginalName, aNewFolder );
        if ( bRet )
            pRet = new UCBStorage_Impl( aNewFolder, aName, nMode, NULL, bDirect, FALSE, m_bRepairPackage, m_xProgressHandler );
    }
    else
    {
        pRet = new UCBStorage_Impl( aName, nMode, NULL, bDirect, FALSE, m_bRepairPackage, m_xProgressHandler );
        pRet->m_bIsLinked = m_bIsLinked;
    }

    if ( pRet )
    {
        pRet->m_bIsRoot = FALSE;

        // if name has been changed before creating the stream: set name!
        pRet->m_aName = pElement->m_aOriginalName;
        pElement->m_xStorage = pRet;
    }

    if ( pRet )
        pRet->Init();

    return pRet;
}

BOOL UCBStorage::IsStorage( const String& rEleName ) const
{
    if( !rEleName.Len() )
        return FALSE;

    const UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    return ( pElement && pElement->m_bIsStorage );
}

BOOL UCBStorage::IsStream( const String& rEleName ) const
{
    if( !rEleName.Len() )
        return FALSE;

    const UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    return ( pElement && !pElement->m_bIsStorage );
}

BOOL UCBStorage::IsContained( const String & rEleName ) const
{
    if( !rEleName.Len() )
        return FALSE;
    const UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    return ( pElement != NULL );
}

BOOL UCBStorage::Remove( const String& rEleName )
{
    if( !rEleName.Len() )
        return FALSE;

    UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    if ( pElement )
    {
        pElement->m_bIsRemoved = TRUE;
    }
    else
        SetError( SVSTREAM_FILE_NOT_FOUND );

    return ( pElement != NULL );
}

BOOL UCBStorage::Rename( const String& rEleName, const String& rNewName )
{
    if( !rEleName.Len()|| !rNewName.Len() )
        return FALSE;

    UCBStorageElement_Impl *pAlreadyExisting = FindElement_Impl( rNewName );
    if ( pAlreadyExisting )
    {
        SetError( SVSTREAM_ACCESS_DENIED );
        return FALSE;                       // can't change to a name that is already used
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

BOOL UCBStorage::MoveTo( const String& rEleName, BaseStorage* pNewSt, const String& rNewName )
{
    if( !rEleName.Len() || !rNewName.Len() )
        return FALSE;

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
        BOOL bRet = CopyTo( rEleName, pNewSt, rNewName );
        if ( bRet )
            bRet = Remove( rEleName );
        return bRet;
    }
}

BOOL UCBStorage::ValidateFAT()
{
    // ???
    return TRUE;
}

BOOL UCBStorage::Validate( BOOL  bWrite ) const
{
    // ???
    return ( !bWrite || ( pImp->m_nMode & STREAM_WRITE ) );
}

BOOL UCBStorage::ValidateMode( StreamMode m ) const
{
    // ???
    if( m == ( STREAM_READ | STREAM_TRUNC ) )  // from stg.cxx
        return TRUE;
    USHORT nCurMode = 0xFFFF;
    if( ( m & 3 ) == STREAM_READ )
    {
        // only SHARE_DENYWRITE or SHARE_DENYALL allowed
        if( ( ( m & STREAM_SHARE_DENYWRITE )
           && ( nCurMode & STREAM_SHARE_DENYWRITE ) )
         || ( ( m & STREAM_SHARE_DENYALL )
           && ( nCurMode & STREAM_SHARE_DENYALL ) ) )
            return TRUE;
    }
    else
    {
        // only SHARE_DENYALL allowed
        // storages open in r/o mode are OK, since only
        // the commit may fail
        if( ( m & STREAM_SHARE_DENYALL )
         && ( nCurMode & STREAM_SHARE_DENYALL ) )
            return TRUE;
    }

    return TRUE;
}

const SvStream* UCBStorage::GetSvStream() const
{
    // this would cause a complete download of the file
    // as it looks, this method is NOT used inside of SOT, only exported by class SotStorage - but for what ???
    return pImp->m_pSource;
}

BOOL UCBStorage::Equals( const BaseStorage& rStorage ) const
{
    // ???
    return ((BaseStorage*)this) == &rStorage;
}

BOOL UCBStorage::IsStorageFile( const String& rFileName )
{
    String aFileURL = rFileName;
    INetURLObject aObj( aFileURL );
    if ( aObj.GetProtocol() == INET_PROT_NOT_VALID )
    {
        ::utl::LocalFileHelper::ConvertPhysicalNameToURL( rFileName, aFileURL );
        aObj.SetURL( aFileURL );
        aFileURL = aObj.GetMainURL( INetURLObject::NO_DECODE );
    }

    SvStream * pStm = ::utl::UcbStreamHelper::CreateStream( aFileURL, STREAM_STD_READ );
    BOOL bRet = UCBStorage::IsStorageFile( pStm );
    delete pStm;
    return bRet;
}

BOOL UCBStorage::IsStorageFile( SvStream* pFile )
{
    if ( !pFile )
        return FALSE;

    ULONG nPos = pFile->Tell();
    pFile->Seek( STREAM_SEEK_TO_END );
    if ( pFile->Tell() < 4 )
        return FALSE;

    pFile->Seek(0);
    UINT32 nBytes;
    *pFile >> nBytes;

    // search for the magic bytes
    BOOL bRet = ( nBytes == 0x04034b50 );
    if ( !bRet )
    {
        // disk spanned file have an additional header in front of the usual one
        bRet = ( nBytes == 0x08074b50 );
        if ( bRet )
        {
            *pFile >> nBytes;
            bRet = ( nBytes == 0x04034b50 );
        }
    }

    pFile->Seek( nPos );
    return bRet;
}

BOOL UCBStorage::IsDiskSpannedFile( SvStream* pFile )
{
    if ( !pFile )
        return FALSE;

    ULONG nPos = pFile->Tell();
    pFile->Seek( STREAM_SEEK_TO_END );
    if ( !pFile->Tell() )
        return FALSE;

    pFile->Seek(0);
    UINT32 nBytes;
    *pFile >> nBytes;

    // disk spanned file have an additional header in front of the usual one
    BOOL bRet = ( nBytes == 0x08074b50 );
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
    ULONG nPos = rStream.Tell();
    rStream.Seek( STREAM_SEEK_TO_END );
    if ( !rStream.Tell() )
        return aString;

    rStream.Seek(0);
    UINT32 nBytes;
    rStream >> nBytes;
    if( nBytes == 0x04034b50 )
    {
        ByteString aTmp;
        rStream.ReadByteString( aTmp );
        if ( aTmp.CompareTo( "ContentURL=", 11 ) == COMPARE_EQUAL )
        {
            aTmp.Erase( 0, 11 );
            aString = String( aTmp, RTL_TEXTENCODING_UTF8 );
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
    *pStream << ( UINT32 ) 0x04034b50;

    // assemble a new folder name in the destination folder
    INetURLObject aObj( rName );
    String aTmp = aObj.GetName();
    String aTitle = String::CreateFromAscii( "content." );
    aTitle += aTmp;

    // create a folder and store its URL
    Content aFolder( aFolderURL, Reference < XCommandEnvironment >() );
    Content aNewFolder;
    BOOL bRet = ::utl::UCBContentHelper::MakeFolder( aFolder, aTitle, aNewFolder );
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
        String aLink = String::CreateFromAscii("ContentURL=");
        aLink += aURL;
        pStream->WriteByteString( aLink, RTL_TEXTENCODING_UTF8 );
        pStream->Flush();

        // move the stream to its desired location
        Content aSource( pTempFile->GetURL(), Reference < XCommandEnvironment >() );
        DELETEZ( pTempFile );
        aFolder.transferContent( aSource, InsertOperation_MOVE, aName, NameClash::OVERWRITE );
        return aURL;
    }

    pTempFile->EnableKillingFile( TRUE );
    delete pTempFile;
    return String();
}

BOOL UCBStorage::SetProperty( const String& rName, const ::com::sun::star::uno::Any& rValue )
{
    if ( rName.CompareToAscii("Title") == COMPARE_EQUAL )
        return FALSE;

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
            return TRUE;
        }
    }
    catch ( Exception& )
    {
    }

    return FALSE;
}

BOOL UCBStorage::GetProperty( const String& rName, ::com::sun::star::uno::Any& rValue )
{
    try
    {
        if ( pImp->GetContent() )
        {
            rValue = pImp->m_pContent->getPropertyValue( rName );
            return TRUE;
        }
    }
    catch ( Exception& )
    {
    }

    return FALSE;
}

BOOL UCBStorage::GetProperty( const String& rEleName, const String& rName, ::com::sun::star::uno::Any& rValue )
{
    UCBStorageElement_Impl *pEle = FindElement_Impl( rEleName );
    if ( !pEle )
        return FALSE;

    if ( !pEle->m_bIsFolder )
    {
        if ( !pEle->m_xStream.Is() )
            pImp->OpenStream( pEle, pImp->m_nMode, pImp->m_bDirect );
        if ( pEle->m_xStream->m_nError )
        {
            pEle->m_xStream.Clear();
            return FALSE;
        }

        try
        {
            if ( pEle->m_xStream->m_pContent )
            {
                rValue = pEle->m_xStream->m_pContent->getPropertyValue( rName );
                return TRUE;
            }
        }
        catch ( Exception& )
        {
        }
    }
    else
    {
        if ( !pEle->m_xStorage.Is() )
            pImp->OpenStorage( pEle, pImp->m_nMode, pImp->m_bDirect );
        if ( pEle->m_xStorage->m_nError )
        {
            pEle->m_xStorage.Clear();
            return FALSE;
        }

        try
        {
            if ( pEle->m_xStorage->GetContent() )
            {
                rValue = pEle->m_xStorage->m_pContent->getPropertyValue( rName );
                return TRUE;
            }
        }
        catch ( Exception& )
        {
        }
    }

    return FALSE;
}

UNOStorageHolderList* UCBStorage::GetUNOStorageHolderList()
{
    if ( !pImp->m_pUNOStorageHolderList )
        pImp->m_pUNOStorageHolderList = new UNOStorageHolderList;

    return pImp->m_pUNOStorageHolderList;
}

