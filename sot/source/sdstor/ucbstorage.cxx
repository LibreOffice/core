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

#include <tools/ref.hxx>
#include <tools/debug.hxx>
#include <unotools/streamhelper.hxx>
#include <tools/list.hxx>
#include <tools/urlobj.hxx>

#include "stg.hxx"
#include "storinfo.hxx"
#include "exchange.hxx"
#include "formats.hxx"
#include "clsids.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::sdbc;
using namespace ::ucb;

TYPEINIT1( UCBStorageStream, BaseStorageStream );
TYPEINIT1( UCBStorage, BaseStorage );

#define COMMIT_RESULT_FAILURE           0
#define COMMIT_RESULT_NOTHING_TO_DO     1
#define COMMIT_RESULT_SUCCESS           2

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
            DBG_ERROR( "Unknown UCB storage format!" );
            return SvGlobalName();
    }
}

// All storage and streams are refcounted internally; outside of this classes they are only accessible through a handle
// class, that uses the refcounted object as impl-class.

class UCBStorageStream_Impl : public SvRefBase, public SvStream
{
                                ~UCBStorageStream_Impl();
protected:

    virtual ULONG               GetData( void* pData, ULONG nSize );
    virtual ULONG               PutData( const void* pData, ULONG nSize );
    virtual ULONG               SeekPos( ULONG nPos );
    virtual void                SetSize( ULONG nSize );
    virtual void                FlushData();
    virtual void                ResetError();

public:
    UCBStorageStream*           m_pAntiImpl;    // only valid if an external reference exists

    String                      m_aOriginalName;// the original name before accessing the stream
    String                      m_aName;        // the actual name ( changed with a Rename command at the parent )
    String                      m_aURL;         // the full path name to create the content
    String                      m_aContentType;
    String                      m_aOriginalContentType;
    ::ucb::Content*             m_pContent;     // the content that provides the data
    ::utl::TempFile*            m_pTempFile;    // temporary file for transacted mode
    SvStream*                   m_pSource;      // the stream covering the original data of the content
    SvStream*                   m_pStream;      // the stream worked on; for readonly streams it is the same as m_pSource,
                                                // for read/write streams it's a copy
    StreamMode                  m_nMode;        // open mode ( read/write/trunc/nocreate/sharing )
    BOOL                        m_bModified;    // only modified streams will be sent to the original content
    BOOL                        m_bCommited;    // sending the streams is coordinated by the root storage of the package
    BOOL                        m_bDirect;      // the storage and its streams are opened in direct mode; for UCBStorages
                                                // this means that the root storage does an autocommit when its external
                                                // reference is destroyed
    BOOL                        m_bIsOLEStorage;// an OLEStorage on a UCBStorageStream makes this an Autocommit-stream

                                UCBStorageStream_Impl( const String&, StreamMode, UCBStorageStream*, BOOL );

    sal_Int16                   Commit();       // if modified and commited: transfer an XInputStream to the content
    BOOL                        Revert();       // discard all changes
    BaseStorage*                CreateStorage();// create an OLE Storage on the UCBStorageStream
    ULONG                       GetSize();
    void                        SwitchToWritable( StreamMode, BOOL );
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
    SvStream*                   m_pSource;      // origonal stream, only for storages on a stream
    SvStream*                   m_pStream;      // the corresponding editable stream, only for storage on a stream
    StreamMode                  m_nMode;        // open mode ( read/write/trunc/nocreate/sharing )
    BOOL                        m_bModified;    // only modified elements will be sent to the original content
    BOOL                        m_bCommited;    // sending the streams is coordinated by the root storage of the package
    BOOL                        m_bDirect;      // the storage and its streams are opened in direct mode; for UCBStorages
                                                // this means that the root storage does an autocommit when its external
                                                // reference is destroyed
    BOOL                        m_bIsRoot;      // marks this storage as root storages that manages all oommits and reverts
    BOOL                        m_bDirty;       // ???
    ULONG                       m_nFormat;
    String                      m_aUserTypeName;
    SvGlobalName                m_aClassId;

    UCBStorageElementList_Impl  m_aChildrenList;

                                UCBStorage_Impl( const String&, StreamMode, UCBStorage*, BOOL, BOOL );
                                UCBStorage_Impl( SvStream&, UCBStorage*, BOOL );
    void                        Init();
    sal_Int16                   Commit();
    BOOL                        Revert();
    BOOL                        Insert( ::ucb::Content *pContent );
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
    String                      GetOriginalContentType();
};

::ucb::Content* UCBStorageElement_Impl::GetContent()
{
    if ( m_xStream.Is() )
        return m_xStream->m_pContent;
    else if ( m_xStorage.Is() )
        return m_xStorage->m_pContent;
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
        return String();
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

UCBStorageStream_Impl::UCBStorageStream_Impl( const String& rName, StreamMode nMode, UCBStorageStream* pStream, BOOL bDirect )
    : m_pAntiImpl( pStream )
    , m_bModified( FALSE )
    , m_bCommited( FALSE )
    , m_bIsOLEStorage( FALSE )
    , m_bDirect( bDirect )
    , m_aURL( rName )
    , m_nMode( nMode )
    , m_pContent( NULL )
    , m_pSource( NULL )
    , m_pStream( NULL )
    , m_pTempFile( NULL )
{
    // name is last segment in URL
    INetURLObject aObj( rName );
    m_aName = m_aOriginalName = aObj.GetLastName();
    BOOL bFailure = FALSE;
    try
    {
        // create the content
        m_pContent = new ::ucb::Content( rName, Reference< ::com::sun::star::ucb::XCommandEnvironment > () );

        // open it using ( readonly, because writing is never done directly into the original stream )
        m_pSource = ::utl::UcbStreamHelper::CreateStream( rName, STREAM_STD_READ );

        // create a temporary Stream for transacted access ( only neccessary for write access )
        if ( nMode & STREAM_WRITE )
        {
            m_pTempFile = new ::utl::TempFile;
            m_pTempFile->EnableKillingFile( TRUE );
            m_pStream = m_pTempFile->GetStream( nMode );

            // copy the original stream into the temporary stream ( only transacted mode is supported )
            if ( m_pSource->GetError() == ERRCODE_IO_NOTEXISTS )
                m_pSource->ResetError();
            else {
                *m_pSource >> *m_pStream;
                m_pStream->Flush();
            }
        }
        else
        {
            // use original stream if only read access is desired
            m_pStream = m_pSource;
        }

        m_pSource->Seek(0);
        m_pStream->Seek(0);
        m_pAntiImpl->SetError( m_pSource->GetError() );
    }
    catch ( ContentCreationException& )
    {
        // content could not be created
        bFailure = TRUE;
        m_pAntiImpl->SetError( SVSTREAM_CANNOT_MAKE );
    }
    catch ( RuntimeException& )
    {
        // any other error - not specified
        m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
    }

    if ( bFailure )
    {
        // creation of content failed, make sure that this does not lead to a crash
        m_pTempFile = new ::utl::TempFile;
        m_pTempFile->EnableKillingFile( TRUE );
        m_pStream = m_pTempFile->GetStream( nMode );
        m_pStream->Seek(0);
        m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
    }
}

UCBStorageStream_Impl::~UCBStorageStream_Impl()
{
    // deleting the TempFile also kills the stream m_pStream ( TempFile is owner )
    delete m_pTempFile;
    delete m_pSource;
    delete m_pContent;
}

void UCBStorageStream_Impl::SwitchToWritable( StreamMode nMode, BOOL bDirect )
{
    if ( !(m_nMode & STREAM_WRITE) )
    {
        // actually this stream is opened readonly
        if ( !m_pSource )
        {
            // stream has been opened for writing earlier and m_pSource was released on last Commit - just get it
            DBG_ASSERT( m_pTempFile && m_pStream, "Suspicious Mode!" );
            m_pSource = ::utl::UcbStreamHelper::CreateStream( m_aURL, STREAM_STD_READ );
        }
        else
        {
            // stream has never been opened for writing - so make a copy of then stream
            DBG_ASSERT( !m_pTempFile && m_pSource == m_pStream, "Suspicious Mode!" );
            m_pTempFile = new ::utl::TempFile;
            m_pTempFile->EnableKillingFile( TRUE );
            m_pStream = m_pTempFile->GetStream( nMode );

            // copy the original stream into the temporary stream ( only transacted mode is supported )
            if ( m_pSource->GetError() == ERRCODE_IO_NOTEXISTS )
                m_pSource->ResetError();
            else {
                *m_pSource >> *m_pStream;
                m_pStream->Flush();
            }
        }
    }

    m_nMode = nMode;
}

// UCBStorageStream_Impl must have a SvStream interface, because it then can be used as underlying stream
// of an OLEStorage; so every write access caused by storage operations marks the UCBStorageStream as modified
ULONG UCBStorageStream_Impl::GetData( void* pData, ULONG nSize )
{
    return m_pStream->Read( pData, nSize );
}

ULONG UCBStorageStream_Impl::PutData( const void* pData, ULONG nSize )
{
    m_bModified = TRUE;
    return m_pStream->Write( pData, nSize );
}

ULONG UCBStorageStream_Impl::SeekPos( ULONG nPos )
{
    return m_pStream->Seek( nPos );
}

void  UCBStorageStream_Impl::SetSize( ULONG nSize )
{
    m_bModified = TRUE;
    m_pStream->SetStreamSize( nSize );
}

void  UCBStorageStream_Impl::FlushData()
{
    m_pStream->Flush();
}

void  UCBStorageStream_Impl::ResetError()
{
    if ( m_pAntiImpl )
        m_pAntiImpl->ResetError();
}

ULONG UCBStorageStream_Impl::GetSize()
{
    ULONG nPos = m_pStream->Tell();
    m_pStream->Seek( STREAM_SEEK_TO_END );
    ULONG nRet = m_pStream->Tell();
    m_pStream->Seek( nPos );
    return nRet;
}

BaseStorage* UCBStorageStream_Impl::CreateStorage()
{
    // create an OLEStorage on a SvStream ( = this )
    // it gets the root attribute because otherwise it would probably not write before my root is commited
    Storage *pStorage = new Storage( *this, TRUE );
    m_bIsOLEStorage = !pStorage->GetError();
    return pStorage;
}

sal_Int16 UCBStorageStream_Impl::Commit()
{
    // send stream to the original content
    // the  parent storage is responsible for the correct handling of deleted contents
    DBG_ASSERT( m_pStream, "Suspicious Commit!" );
    if ( m_bCommited || m_bIsOLEStorage || m_bDirect )
    {
        // modified streams with OLEStorages on it have autocommit; it is assumed that the OLEStorage
        // was commited as well ( if not opened in direct mode )
        if ( m_bModified )
        {
            try
            {
                // First a "HandsOff" : release object before commiting new data to it
                DELETEZ( m_pSource );

                // better create new LockBytes, because streams are not refcounted
                Reference < XInputStream > xStream = new ::utl::OInputStreamHelper( new SvLockBytes( m_pStream ), 8192 );
                Any aAny;
                InsertCommandArgument aArg;
                aArg.Data = xStream;
                aArg.ReplaceExisting = sal_True;
                aAny <<= aArg;
                m_pContent->executeCommand( ::rtl::OUString::createFromAscii("insert"), aAny );
                m_bModified = FALSE;

                INetURLObject aObj( m_aURL );
                aObj.SetName( m_aName );
                m_aURL = aObj.GetMainURL();
            }
            catch ( CommandAbortedException& )
            {
                // any command wasn't executed successfully - not specified
                if ( m_pAntiImpl )
                    m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
                return COMMIT_RESULT_FAILURE;
            }
            catch ( RuntimeException& )
            {
                // any other error - not specified
                if ( m_pAntiImpl )
                    m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
                return COMMIT_RESULT_FAILURE;
            }
            catch ( Exception& )
            {
                // any other error - not specified
                if ( m_pAntiImpl )
                    m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
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
    DBG_ASSERT( m_pStream && m_pSource != m_pStream, "Suspicious Revert!" );
    if ( m_bCommited )
    {
        DBG_ERROR("Revert while commit is in progress!" )
        return FALSE;                   //  ???
    }

    // discard all changes, get the original stream data
    if ( m_bModified )
    {
        if ( !m_pSource )
            // SourceStream was released on last Commit
            m_pSource = ::utl::UcbStreamHelper::CreateStream( m_aURL, STREAM_STD_READ );

        *m_pSource >> *m_pStream;
        m_pStream->Seek(0);
        m_pSource->Seek(0);
        m_bModified = FALSE;
    }

    m_aName = m_aOriginalName;
    m_aContentType = m_aOriginalContentType;
    return ( m_pStream->GetError() != ERRCODE_NONE );
}

UCBStorageStream::UCBStorageStream( const String& rName, StreamMode nMode, BOOL bDirect )
{
    // pImp must be initialized in the body, because otherwise the vtable of the stream is not initialized
    // to class UCBStorageStream !
    pImp = new UCBStorageStream_Impl( rName, nMode, this, bDirect );
    pImp->AddRef();             // use direct refcounting because in header file only a pointer should be used
    StorageBase::nMode = pImp->m_nMode;
}

UCBStorageStream::UCBStorageStream( UCBStorageStream_Impl *pImpl )
    : pImp( pImpl )
{
    pImp->AddRef();             // use direct refcounting because in header file only a pointer should be used
    StorageBase::nMode = pImp->m_nMode;
}

UCBStorageStream::~UCBStorageStream()
{
    pImp->m_pAntiImpl = NULL;
    pImp->ReleaseRef();
}

ULONG UCBStorageStream::Read( void * pData, ULONG nSize )
{
    return pImp->m_pStream->Read( pData, nSize );
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
    return pImp->m_pStream->Write( pData, nSize );
}

ULONG UCBStorageStream::Seek( ULONG nPos )
{
    return pImp->m_pStream->Seek( nPos );
}

ULONG UCBStorageStream::Tell()
{
    return pImp->m_pStream->Tell();
}

void UCBStorageStream::Flush()
{
    // streams are never really transacted, so flush also means commit !
//    Commit();
    pImp->m_pStream->Flush();
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
    return pImp->m_pStream->SetStreamSize( nNewSize );
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
    return pImp->m_pStream;
}

BOOL  UCBStorageStream::Equals( const BaseStorageStream& rStream ) const
{
    // ???
    return ((BaseStorageStream*) this ) == &rStream;
}

BOOL UCBStorageStream::Commit()
{
    // mark this stream for sending it on root commit
    pImp->m_pStream->Flush();
    pImp->m_bCommited = TRUE;
    return TRUE;
}

BOOL UCBStorageStream::Revert()
{
    return pImp->Revert();
}

BOOL UCBStorageStream::CopyTo( BaseStorageStream* pDestStm )
{
    pDestStm->SetSize( 0 );
    Seek( STREAM_SEEK_TO_END );
    INT32 n = Tell();
    if( pDestStm->SetSize( n ) && n )
    {
        void* p = new BYTE[ 4096 ];
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

        delete p;
    }

    return TRUE;
}

BOOL UCBStorageStream::SetProperty( const String& rName, const ::com::sun::star::uno::Any& rValue )
{
    if ( rName.CompareToAscii("Title") == COMPARE_EQUAL )
        return FALSE;

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
    // pImp must be initialized in the body, because otherwise the vtable of the stream is not initialized
    // to class UCBStorage !
    pImp = new UCBStorage_Impl( rStrm, this, bDirect );
    pImp->AddRef();
    pImp->Init();
    StorageBase::nMode = pImp->m_nMode;
}

UCBStorage::UCBStorage( const String& rName, StreamMode nMode, BOOL bDirect, BOOL bIsRoot )
{
    // pImp must be initialized in the body, because otherwise the vtable of the stream is not initialized
    // to class UCBStorage !
    pImp = new UCBStorage_Impl( rName, nMode, this, bDirect, bIsRoot );
    pImp->AddRef();
    pImp->Init();
    StorageBase::nMode = pImp->m_nMode;
}

UCBStorage::UCBStorage( UCBStorage_Impl *pImpl )
    : pImp( pImpl )
{
    pImp->AddRef();             // use direct refcounting because in header file only a pointer should be used
    StorageBase::nMode = pImp->m_nMode;
}

UCBStorage::~UCBStorage()
{
    if ( pImp->m_bIsRoot && pImp->m_bDirect )
        // DirectMode is simulated with an AutoCommit
        Commit();

    pImp->m_pAntiImpl = NULL;
    pImp->ReleaseRef();
}

UCBStorage_Impl::UCBStorage_Impl( const String& rName, StreamMode nMode, UCBStorage* pStorage, BOOL bDirect, BOOL bIsRoot )
    : m_pAntiImpl( pStorage )
    , m_pTempFile( NULL )
    , m_pContent( NULL )
    , m_pSource( NULL )
    , m_pStream( NULL )
    , m_nMode( nMode )
    , m_nFormat( 0 )
    , m_bIsRoot( bIsRoot )
    , m_bDirect( bDirect )
    , m_bModified( FALSE )
    , m_bCommited( FALSE )
    , m_bDirty( FALSE )
    , m_aClassId( SvGlobalName() )
{
    String aName( rName );
    if( !aName.Len() )
    {
        // no name given = use temporary name!
        DBG_ASSERT( m_bIsRoot, "SubStorage must have a name!" );
        m_pTempFile = new ::utl::TempFile;
        m_aName = m_aOriginalName = aName = m_pTempFile->GetURL();
    }

    if ( m_bIsRoot )
    {
        // the root storage opens the package; create the special package URL for the package content
        String aTemp = String::CreateFromAscii("vnd.sun.star.pkg://");
        aTemp += INetURLObject::encode( aName, INetURLObject::PART_AUTHORITY, '%', INetURLObject::ENCODE_ALL );
        m_aURL = aTemp;
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
    , m_bIsRoot( TRUE )
    , m_bDirect( bDirect )
    , m_nFormat( 0 )
    , m_bDirty( FALSE )
    , m_bModified( FALSE )
    , m_bCommited( FALSE )
    , m_aClassId( SvGlobalName() )
{
    // UCBStorages work on a content, so a temporary file for a content must be created, even if the stream is only
    // accessed readonly
    // the root storage opens the package; create the special package URL for the package content
    String aTemp = String::CreateFromAscii("vnd.sun.star.pkg://");
    aTemp += INetURLObject::encode( m_pTempFile->GetURL(), INetURLObject::PART_AUTHORITY, '%', INetURLObject::ENCODE_ALL );
    m_aURL = aTemp;

    // copy data into the temporary file
    m_pStream = m_pTempFile->GetStream( STREAM_STD_READWRITE );
    rStream >> *m_pStream;
    m_pStream->Flush();
    m_pStream->Seek(0);
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

    try
    {
        // create content; where to put StreamMode ?! ( already done when opening the file of the package ? )
        m_pContent = new ::ucb::Content( m_aURL, Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
    }
    catch ( ContentCreationException& )
    {
        // content could not be created
        m_pAntiImpl->SetError( SVSTREAM_CANNOT_MAKE );
    }
    catch ( RuntimeException& )
    {
        // any other error - not specified
        m_pAntiImpl->SetError( SVSTREAM_CANNOT_MAKE );
    }

    if ( m_pContent )
    {
        Any aAny = m_pContent->getPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ) );
        rtl::OUString aTmp;
        if ( ( aAny >>= aTmp ) && aTmp.getLength() )
        {
            m_aContentType = m_aOriginalContentType = aTmp;

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
            Reference< XResultSet > xResultSet = m_pContent->createCursor( aProps, eInclude );
            Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
            Reference< XRow > xRow( xResultSet, UNO_QUERY );
            if ( xResultSet.is() )
            {
                while ( xResultSet->next() )
                {
                    // insert all into the children list
                    ::rtl::OUString aTitle( xRow->getString(1) );
                    BOOL bIsFolder( xRow->getBoolean(2) );
                    sal_Int64 nSize = xRow->getLong(4);
                    ::rtl::OUString aContentType= xRow->getString(3);
                    UCBStorageElement_Impl* pElement = new UCBStorageElement_Impl( aTitle, aContentType, bIsFolder, (ULONG) nSize );
                    m_aChildrenList.Insert( pElement, LIST_APPEND );
                    if ( !bIsFolder )
                    {
                        // will be replaced by a detection using the MediaType
                        BaseStorageStream* pStream = m_pAntiImpl->OpenStream( xRow->getString(1), STREAM_STD_READ, m_bDirect );
                        if ( Storage::IsStorageFile( const_cast < SvStream* > ( pStream->GetSvStream() ) ) )
                            pElement->m_bIsStorage = TRUE;
                        delete pStream;
                    }
                }
            }
        }
        catch ( CommandAbortedException& )
        {
            // any command wasn't executed successfully - not specified
            m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
        }
        catch ( RuntimeException& )
        {
            // any other error - not specified
            m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
        }
        catch ( SQLException& )
        {
            // any other error - not specified
            m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
        }
        catch ( Exception& )
        {
            // any other error - not specified
            m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
        }
    }
}

UCBStorage_Impl::~UCBStorage_Impl()
{
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
    Sequence < ::rtl::OUString > aNames(1);
    Sequence < Any > aValues(1);
    ::rtl::OUString* pNames = aNames.getArray();
    pNames[0] = ::rtl::OUString::createFromAscii("Title");
    Any* pValues = aValues.getArray();
    pValues[0] <<= ::rtl::OUString( m_aName );
    ::rtl::OUString aType = ::rtl::OUString::createFromAscii( "application/vnd.sun.star.pkg-folder" );

    // remove old content, create an "empty" new one and initialize it by inserting
    DELETEZ( m_pContent );
    m_pContent = new ::ucb::Content;
    BOOL bRet = FALSE;
    try
    {
        bRet = pContent->insertNewContent( aType, aNames, aValues, *m_pContent );
    }
    catch ( CommandAbortedException& )
    {
        // any command wasn't executed successfully - not specified
        if ( m_pAntiImpl )
            m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
    }
    catch ( RuntimeException& )
    {
        // any other error - not specified
        if ( m_pAntiImpl )
            m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
    }
    catch ( Exception& )
    {
        // any other error - not specified
        if ( m_pAntiImpl )
            m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
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
            while ( pElement && nRet )
            {
                ::ucb::Content* pContent = pElement->GetContent();
                BOOL bDeleteContent = FALSE;
                if ( !pContent && pElement->IsModified() )
                {
                    // if the element has never been opened, no content has been created until now
                    bDeleteContent = TRUE;
                    String aName( m_aURL );
                    aName += '/';
                    aName += pElement->m_aOriginalName;
                    pContent = new ::ucb::Content( aName, Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
                }

                if ( pElement->m_bIsRemoved )
                {
                    // errors will be caught in the "catch" statement outside the loop
                    nRet = COMMIT_RESULT_SUCCESS;
                    Any aAny;
                    pContent->executeCommand( ::rtl::OUString::createFromAscii("delete"), aAny );
                }
                else
                {
                    sal_Int16 nLocalRet = COMMIT_RESULT_NOTHING_TO_DO;
                    if ( pElement->m_xStorage.Is() )
                    {
                        if ( !pElement->m_bIsInserted || pElement->m_xStorage->Insert( m_pContent ) )
                        {
                            nLocalRet = pElement->m_xStorage->Commit();
                            pContent = pElement->GetContent();
                        }
                    }
                    else if ( pElement->m_xStream.Is() )
                    {
                        nLocalRet = pElement->m_xStream->Commit();
                        pContent = pElement->GetContent();
                    }

                    if ( pElement->m_aName != pElement->m_aOriginalName )
                    {
                        // errors will be caught in the "catch" statement outside the loop
                        nRet = COMMIT_RESULT_SUCCESS;
                        Any aAny;
                        aAny <<= (rtl::OUString) pElement->m_aName;
                        pContent->setPropertyValue( ::rtl::OUString::createFromAscii("Title"), aAny );
                    }

                    if ( pElement->GetContentType() != pElement->GetOriginalContentType() )
                    {
                        // errors will be caught in the "catch" statement outside the loop
                        nRet = COMMIT_RESULT_SUCCESS;
                        Any aAny;
                        aAny <<= (rtl::OUString) pElement->GetContentType();
                        pContent->setPropertyValue( ::rtl::OUString::createFromAscii("MediaType"), aAny );
                    }

                    if ( nLocalRet != COMMIT_RESULT_NOTHING_TO_DO )
                        nRet = nLocalRet;
                }

                if ( bDeleteContent )
                    delete pContent;

                pElement = m_aChildrenList.Next();
            }
        }
        catch ( ContentCreationException& )
        {
            // content could not be created
            if ( m_pAntiImpl )
                m_pAntiImpl->SetError( ERRCODE_IO_NOTEXISTS );
            return COMMIT_RESULT_FAILURE;
        }
        catch ( CommandAbortedException& )
        {
            // any command wasn't executed successfully - not specified
            if ( m_pAntiImpl )
                m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
            return COMMIT_RESULT_FAILURE;
        }
        catch ( RuntimeException& )
        {
            // any other error - not specified
            if ( m_pAntiImpl )
                m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
            return COMMIT_RESULT_FAILURE;
        }
        catch ( Exception& )
        {
            // any other error - not specified
            if ( m_pAntiImpl )
                m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
            return COMMIT_RESULT_FAILURE;
        }

        if ( m_bIsRoot )
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

                    DELETEZ( m_pSource );
                    Any aAny;
                    m_pContent->executeCommand( ::rtl::OUString::createFromAscii("flush"), aAny );
                    if ( m_pStream && m_pSource )
                    {
                        *m_pStream >> *m_pSource;
                        m_pStream->Seek(0);
                        m_pSource->Seek(0);
                    }
                }
                catch ( CommandAbortedException& )
                {
                    // how to tell the content : forget all changes ?!
                    // or should we assume that the content does it by itself because he throwed an exception ?!
                    // any command wasn't executed successfully - not specified
                    if ( m_pAntiImpl )
                        m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
                    return COMMIT_RESULT_FAILURE;
                }
                catch ( RuntimeException& )
                {
                    // how to tell the content : forget all changes ?!
                    // or should we assume that the content does it by itself because he throwed an exception ?!
                    // any other error - not specified
                    if ( m_pAntiImpl )
                        m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
                    return COMMIT_RESULT_FAILURE;
                }
                catch ( Exception& )
                {
                    // how to tell the content : forget all changes ?!
                    // or should we assume that the content does it by itself because he throwed an exception ?!
                    // any other error - not specified
                    if ( m_pAntiImpl )
                        m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
                    return COMMIT_RESULT_FAILURE;
                }
            }
            else
            {
                // how to tell the content : forget all changes ?!
                if ( m_pAntiImpl && !m_pAntiImpl->GetError() )
                    m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
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
                pElement->m_xStream->Revert();
            else if ( pElement->m_xStorage.Is() )
                pElement->m_xStorage->Revert();

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
    ::com::sun::star::datatransfer::DataFlavor aDataFlavor;
    SotExchange::GetFormatDataFlavor( pImp->m_nFormat, aDataFlavor );
    pImp->m_aUserTypeName = aDataFlavor.HumanPresentableName;
    pImp->m_aContentType = aDataFlavor.MimeType;
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
    UCBStorageElement_Impl* pElement = pImp->m_aChildrenList.First();
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
            pStream = ( const_cast < UCBStorage* > (this) )->OpenStream( rElement.m_aName, pImp->m_nMode, pImp->m_bDirect );
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
                pDest->OpenStorage( rNew, STREAM_WRITE | STREAM_SHARE_DENYALL, pImp->m_bDirect );

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
    UCBStorageElement_Impl* pElement = pImp->m_aChildrenList.First();
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
    UCBStorageElement_Impl* pElement = pImp->m_aChildrenList.First();
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

BaseStorageStream* UCBStorage::OpenStream( const String& rEleName, StreamMode nMode, BOOL bDirect )
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
            UCBStorageStream* pStream = new UCBStorageStream( aName, nMode, bDirect );
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
            }
            else
            {
                BOOL bIsWritable = ( pElement->m_xStream->m_nMode & STREAM_WRITE );
                if ( !bIsWritable && ( nMode & STREAM_WRITE ) )
                    pElement->m_xStream->SwitchToWritable( nMode, bDirect );
//              DBG_ASSERT( bDirect == pElement->m_xStream->m_bDirect, "Wrong DirectMode!" );
                return new UCBStorageStream( pElement->m_xStream );
            }
        }
        else
        {
            // stream is opened the first time
            String aName( pImp->m_aURL );
            aName += '/';
            aName += pElement->m_aOriginalName;
            UCBStorageStream* pStream = new UCBStorageStream( aName, nMode, bDirect );
            pElement->m_xStream = pStream->pImp;

            // if name has been changed before creating the stream: set name!
            pStream->pImp->m_aName = rEleName;
            return pStream;
        }
    }

    return NULL;
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

    return OpenStorage_Impl( rEleName, nMode, bDirect, FALSE );
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
            UCBStorage *pStorage = new UCBStorage( aName, nMode, bDirect, FALSE );
            pStorage->pImp->m_bIsRoot = FALSE;
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
        BaseStorageStream* pStr = OpenStream( rEleName, nMode, bDirect );
        UCBStorageStream* pStream = PTR_CAST( UCBStorageStream, pStr );
        if ( !pStream )
        {
            SetError( ( nMode & STREAM_WRITE ) ? SVSTREAM_CANNOT_MAKE : SVSTREAM_FILE_NOT_FOUND );
            return NULL;
        }

        pElement->m_bIsStorage = TRUE;
        pElement->m_xStream = pStream->pImp;
        delete pStream;
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
                UCBStorage* pStorage = new UCBStorage( aName, nMode, bDirect, FALSE );
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
        String aName( pImp->m_aURL );
        aName += '/';
        aName += pElement->m_aOriginalName;  //  ???
        pElement->m_bIsStorage = pElement->m_bIsFolder = TRUE;
        UCBStorage *pStorage = new UCBStorage( aName, nMode, bDirect, FALSE );
        pStorage->pImp->m_bIsRoot = FALSE;

        // if name has been changed before creating the stream: set name!
        pStorage->pImp->m_aName = rEleName;
        pElement->m_xStorage = pStorage->pImp;
        return pStorage;
    }

    return NULL;
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
    return pImp->m_pStream;
}

BOOL UCBStorage::Equals( const BaseStorage& rStorage ) const
{
    // ???
    return ((BaseStorage*)this) == &rStorage;
}

BOOL UCBStorage::IsStorageFile( SvStream* pFile )
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
    BOOL bRet = ( nBytes == 0x04034b50 );         // magic Bytes!
    pFile->Seek( nPos );
    return bRet;
}

BOOL UCBStorage::SetProperty( const String& rName, const ::com::sun::star::uno::Any& rValue )
{
    if ( rName.CompareToAscii("Title") == COMPARE_EQUAL )
        return FALSE;

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

BOOL UCBStorage::GetProperty( const String& rName, ::com::sun::star::uno::Any& rValue )
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

