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

#include <tools/ref.hxx>
#include <tools/debug.hxx>
#include <unotools/streamhelper.hxx>
#include <tools/list.hxx>

#include "stg.hxx"
#include "stgelem.hxx"
#include "storinfo.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::sdbc;

TYPEINIT1( UCBStorageStream, BaseStorageStream );
TYPEINIT1( UCBStorage, BaseStorage );

// All storage and streams are refcounted internally; outside of this classes they are only accessible through a handle
// class, that uses the refcounted object as impl-class.

class UCBStorageStream_Impl : public SvRefBase
{
                                ~UCBStorageStream_Impl();
public:
    UCBStorageStream*           m_pAntiImpl;    // only valid if external references exist

    ::ucb::Content*             m_pContent;     // the content that provides the data
    String                      m_aOriginalName;// the original name before accessing the stream
    String                      m_aName;        // the actual name ( changed with a Rename command at the parent )
    String                      m_aContentType;
    String                      m_aOriginalContentType;
    StreamMode                  m_nMode;        // open mode ( read/write/trunc/nocreate/sharing )
    BOOL                        m_bModified;    // only modified streams will be sent to the original content
    BOOL                        m_bCommited;    // sending the streams is coordinated by the root storage of the package

    ::utl::TempFile*            m_pTempFile;    // temporary file for transacted mode
    SvStream*                   m_pSource;      // the stream covering the original data of the content
    SvStream*                   m_pStream;      // the corresponding editable stream
    BOOL                        m_bIsOLEStorage;// an OLEStorage on a UCBStorageStream makes this an Autocommit-stream

                                UCBStorageStream_Impl( const String&, StreamMode, UCBStorageStream* );

    BOOL                        Commit();       // if modified and commited: transfer an XInputStream to the content
    BaseStorage*                CreateStorage();// create an OLE Storage on the UCBStorageStream
    ULONG                       GetSize();
};

SV_DECL_IMPL_REF( UCBStorageStream_Impl );

struct UCBStorageElement_Impl;
DECLARE_LIST( UCBStorageElementList_Impl, UCBStorageElement_Impl* );

class UCBStorage_Impl : public SvRefBase
{
                                ~UCBStorage_Impl();
public:
    UCBStorage*                 m_pAntiImpl;    // only valid if external references exist

    ::ucb::Content*             m_pContent;
    String                      m_aOriginalName;// the original name before accessing the storage
    String                      m_aName;        // the actual name ( changed with a Rename command at the parent )
    String                      m_aContentType;
    String                      m_aOriginalContentType;
    StreamMode                  m_nMode;        // open mode ( read/write/trunc/nocreate/sharing )
    BOOL                        m_bModified;    // only modified streams will be sent to the original content
    BOOL                        m_bCommited;    // sending the streams is coordinated by the root storage of the package

    ::utl::TempFile*            m_pTempFile;    // temporary file for storage on stream
    SvStream*                   m_pSource;      // the stream covering the original data of the content
    SvStream*                   m_pStream;      // the corresponding editable stream

    SvGlobalName                m_aGlobalName;
    ULONG                       m_nFormat;
    String                      m_aUserTypeName;
    ClsId                       m_aClassId;
    BOOL                        m_bIsRoot;
    BOOL                        m_bDirty;  // ???

    UCBStorageElementList_Impl  m_aChildrenList;

                                UCBStorage_Impl( const String&, StreamMode, UCBStorage* );
                                UCBStorage_Impl( SvStream&, UCBStorage* );
    void                        Init( const String&, StreamMode, UCBStorage* );
    BOOL                        Commit();
};

SV_DECL_IMPL_REF( UCBStorage_Impl );

// this struct contains all neccessary information on an element inside a UCBStorage
struct UCBStorageElement_Impl
{
    String                      m_aName;        // the actual URL relative to the root "folder"
    String                      m_aOriginalName;// the original name in the content
    String                      m_aContentType; // MediaType, can be used to check if a stream "is" a OLEStorage
    String                      m_aOriginalContentType;
    ULONG                       m_nSize;
    BOOL                        m_bIsFolder;    // Only TRUE when it is a UCBStorage !
    BOOL                        m_bIsStorage;   // Also TRUE when it is an OLEStorage !
    BOOL                        m_bIsRemoved;   // element will be removed on commit
    UCBStorage_ImplRef          m_xStorage;     // reference to the "real" storage
    UCBStorageStream_ImplRef    m_xStream;      // reference to the "real" stream

                                UCBStorageElement_Impl( const ::rtl::OUString& rName, const ::rtl::OUString rMediaType,
                                            BOOL bIsFolder, ULONG nSize )
                                    : m_aName( rName )
                                    , m_aOriginalName( rName )
                                    , m_aContentType( rMediaType )
                                    , m_aOriginalContentType( rMediaType )
                                    , m_nSize( nSize )
                                    , m_bIsFolder( bIsFolder )
                                    , m_bIsStorage( FALSE )
                                    , m_bIsRemoved( FALSE )
                                {
                                    // Detection for OLEStorage is needed !
                                }
};

UCBStorageStream_Impl::UCBStorageStream_Impl( const String& rName, StreamMode nMode, UCBStorageStream* pStream )
    : m_pAntiImpl( pStream )
    , m_bModified( FALSE )
    , m_bCommited( FALSE )
    , m_aName( rName )
    , m_aOriginalName( rName )
    , m_nMode( nMode )
{
    // create the content
    m_pContent = new ::ucb::Content( rName, Reference< ::com::sun::star::ucb::XCommandEnvironment > () );

    // open it using the given StreamMode
    // is the case "New Content - stream must be created new" handled correctly ?? In this case m_pSource is empty or NULL!
    m_pSource = ::utl::UcbStreamHelper::CreateStream( rName, nMode );

    // create a temporary Stream for transacted access ( only neccessary for write access )
    if ( nMode & STREAM_WRITE )
    {
        m_pTempFile = new ::utl::TempFile;
        m_pTempFile->EnableKillingFile( TRUE );
        m_pStream = m_pTempFile->GetStream( nMode );

        // copy the original stream into the temporary stream ( only transacted mode is supported )
        *m_pSource >> *m_pStream;
    }
    else
    {
        m_pTempFile = NULL;
        m_pStream = m_pSource;
    }
}

UCBStorageStream_Impl::~UCBStorageStream_Impl()
{
    delete m_pTempFile;
    delete m_pSource;
    delete m_pContent;
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
    // create an OLEStorage
    // it must have the root attribute because otherwise it would probably not write before my root is commited
    // is this correct ?
    // or should we open the storage in direct mode ?
    // or kreep it and store using a copy like in SfxMedium ?
    // perhaps we could rearrange the storage on commit ot this stream ?
    return new Storage( *m_pStream, FALSE );
}

BOOL UCBStorageStream_Impl::Commit()
{
    // send stream to the original content
    // the  parent storage is responsible for the correct handling of deleted contents
    // all access should be disabled until the root storage has sent a "Commit" command to its content
    if ( m_bCommited || m_bIsOLEStorage )
    {
        // modified streams with OLEStorages on it have autocommit
        if ( m_bModified )
        {
            try
            {
                Reference < XInputStream > xStream = new ::utl::OInputStreamHelper( new SvLockBytes( m_pStream ), 8192 );
                Any aAny;
                InsertCommandArgument aArg;
                aArg.Data = xStream;
                aArg.ReplaceExisting = sal_True;
                aAny <<= aArg;
                m_pContent->executeCommand( ::rtl::OUString::createFromAscii("Insert"), aAny );
                m_bModified = FALSE;
            }
            catch ( ... )
            {
                if ( m_pAntiImpl )
                    m_pAntiImpl->SetError( ERRCODE_IO_GENERAL );
                return FALSE;
            }

            m_bCommited = FALSE;
            return TRUE;
        }
    }

    return TRUE;
}

UCBStorageStream::UCBStorageStream( const String& rName, StreamMode nMode )
    : pImp( new UCBStorageStream_Impl( rName, nMode, this ) )
{
    pImp->AddRef();             // use direct refcounting because in header file only a pointer should be used
}

UCBStorageStream::UCBStorageStream( UCBStorageStream_Impl *pImpl )
    : pImp( pImpl )
{
    pImp->AddRef();             // use direct refcounting because in header file only a pointer should be used
}

UCBStorageStream::~UCBStorageStream()
{
    pImp->ReleaseRef();
}

ULONG UCBStorageStream::Read( void * pData, ULONG nSize )
{
    return pImp->m_pStream->Read( pData, nSize );
}

ULONG UCBStorageStream::Write( const void* pData, ULONG nSize )
{
    if ( pImp->m_bCommited )
    {
        DBG_ERROR("Revert while commit is in progress!" )
        return 0;
    }

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

void  UCBStorageStream::Flush()
{
    // streams are never really transacted, so flush also means commit !
    pImp->m_pStream->Flush();
    Commit();
}

BOOL  UCBStorageStream::SetSize( ULONG nNewSize )
{
    if ( pImp->m_bCommited )
    {
        DBG_ERROR("Revert while commit is in progress!" )
        return FALSE;
    }

    pImp->m_bModified = TRUE;
    return TRUE;
    return pImp->m_pStream->SetStreamSize( nNewSize );
}

BOOL  UCBStorageStream::Validate( BOOL bWrite ) const
{
    return ( !bWrite || ( pImp->m_nMode & STREAM_WRITE ) );
}

BOOL  UCBStorageStream::ValidateMode( StreamMode m ) const
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
    // Write or revert access should be disabled until the pImp->bModified is cleared!
    // mark this stream for sending it on root commit
    pImp->m_bCommited = TRUE;
    return TRUE;
}

BOOL UCBStorageStream::Revert()
{
    if ( pImp->m_bCommited )
    {
        DBG_ERROR("Revert while commit is in progress!" )
        return FALSE;                   //  ???
    }

    // discard all changes, get the original stream data
    if ( pImp->m_bModified )
    {
        *pImp->m_pSource >> *pImp->m_pStream;
        pImp->m_bModified = FALSE;
    }

    pImp->m_aName = pImp->m_aOriginalName;
    pImp->m_aContentType = pImp->m_aOriginalContentType;
    return ( GetError() == ERRCODE_NONE );
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

UCBStorage::UCBStorage( SvStream& rStrm, BOOL bDirect )
    : pImp( new UCBStorage_Impl( rStrm, this ) )
{
    pImp->AddRef();
}

UCBStorage::UCBStorage( const String& rName, StreamMode nMode )
    : pImp( new UCBStorage_Impl( rName, nMode, this ) )
{
    pImp->AddRef();
}

UCBStorage::UCBStorage( UCBStorage_Impl *pImpl )
    : pImp( pImpl )
{
    pImp->AddRef();             // use direct refcounting because in header file only a pointer should be used
}

UCBStorage::~UCBStorage()
{
    pImp->ReleaseRef();
}

UCBStorage_Impl::UCBStorage_Impl( const String& rName, StreamMode nMode, UCBStorage* pStorage )
    : m_pAntiImpl( pStorage )
    , m_aName( rName )
    , m_aOriginalName( rName )
    , m_pTempFile( NULL )
    , m_pSource( NULL )
    , m_pStream( NULL )
    , m_bIsRoot( TRUE )
{
    Init( rName, nMode, pStorage );
}

UCBStorage_Impl::UCBStorage_Impl( SvStream& rStream, UCBStorage* pStorage )
    : m_pAntiImpl( pStorage )
    , m_pTempFile( new ::utl::TempFile )
    , m_pSource( &rStream )
    , m_bIsRoot( TRUE )
{
    // UCBStorages work on a content, so a temporary file for a content must be created
    m_aName = m_aOriginalName = m_pTempFile->GetURL();

    // copy data into the temporary file
    m_pStream = m_pTempFile->GetStream( STREAM_STD_READWRITE );
    rStream >> *m_pStream;

    // check opening mode
    StreamMode nMode = STREAM_READ;
    if( rStream.IsWritable() )
        nMode = STREAM_READ | STREAM_WRITE;

    // proceed as usual
    Init( m_aName, nMode, pStorage );
}

void UCBStorage_Impl::Init( const String& rName, StreamMode nMode, UCBStorage* pStorage )
{
    // create content; where to put StreamMode ?! ( already done when opening the file of the package ? )
    m_pContent = new ::ucb::Content( rName, Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
    try
    {
        // create cursor with only URLs of children in the resultset
        // maybe also "MediaType" or "IsFolder" to ease the creation of substorages or streams and also
        // make methods like IsStream or IsStorage and FillInfoList possible
        Sequence< ::rtl::OUString > aProps(4);
        ::rtl::OUString* pProps = aProps.getArray();
        pProps[0] == ::rtl::OUString::createFromAscii( "Url" );
        pProps[1] == ::rtl::OUString::createFromAscii( "IsFolder" );
        pProps[2] == ::rtl::OUString::createFromAscii( "MediaType" );
        pProps[3] == ::rtl::OUString::createFromAscii( "Size" );
        ::ucb::ResultSetInclude eInclude = ::ucb::INCLUDE_FOLDERS_AND_DOCUMENTS;
        Reference< XResultSet > xResultSet = m_pContent->createCursor( aProps, eInclude );
        Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
        while ( xResultSet->next() )
        {
            // insert all into the children list
            ::rtl::OUString aId = xContentAccess->queryContentIdentifierString();
            BOOL bIsFolder = TRUE;
            ULONG nSize = 0;
            ::rtl::OUString aContentType;
            m_aChildrenList.Insert( new UCBStorageElement_Impl( aId, aContentType, bIsFolder, nSize ), LIST_APPEND );
        }
    }
    catch( ... )
    {
        m_pAntiImpl->SetError( SVSTREAM_CANNOT_MAKE );
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

const String& UCBStorage::GetName() const
{
    return pImp->m_aName;
}

BOOL UCBStorage::IsRoot() const
{
    return pImp->m_bIsRoot;
}

void UCBStorage::SetClassId( const ClsId& rClsId )
{
    pImp->m_aClassId = rClsId;
}

const ClsId& UCBStorage::GetClassId() const
{
    return pImp->m_aClassId;
}

void UCBStorage::SetDirty()
{
    pImp->m_bDirty = TRUE;
}

void UCBStorage::SetClass( const SvGlobalName & rClass, ULONG nOriginalClipFormat, const String & rUserTypeName )
{
    pImp->m_aGlobalName = rClass;
    pImp->m_nFormat = nOriginalClipFormat;
    pImp->m_aUserTypeName = rUserTypeName;
}

void UCBStorage::SetConvertClass( const SvGlobalName & rConvertClass, ULONG nOriginalClipFormat, const String & rUserTypeName )
{
    // ???
}

SvGlobalName UCBStorage::GetClassName()
{
    return pImp->m_aGlobalName;
}

ULONG UCBStorage::GetFormat()
{
    return pImp->m_nFormat;
}

String UCBStorage::GetUserName()
{
    return pImp->m_aUserTypeName;
}

BOOL UCBStorage::ShouldConvert()
{
    return FALSE;
}

void UCBStorage::FillInfoList( SvStorageInfoList* pList ) const
{
    // put information in childrenlist into StorageInfoList
    UCBStorageElement_Impl* pElement = pImp->m_aChildrenList.First();
    while ( pElement  )
    {
        ULONG nSize = pElement->m_nSize;
        if ( pElement->m_xStream.Is() )
            nSize = pElement->m_xStream->GetSize();
        SvStorageInfo aInfo( pElement->m_aName, nSize, pElement->m_bIsStorage );
        pList->Append( aInfo );
        pElement = pImp->m_aChildrenList.Next();
    }
}

BOOL UCBStorage::CopyStorageElement_Impl( UCBStorageElement_Impl& rElement, BaseStorage* pDest, const String& rNew ) const
{
    // insert stream or storage into the list or stream of the destination storage
    // not into the content, this will be done on commit !
    // be aware of name changes !
    if ( !rElement.m_bIsFolder )
    {
        // copy the streams data
        // the destination stream must not be open
        BaseStorageStream* pOtherStream = pDest->OpenStream( rElement.m_aName, STREAM_WRITE | STREAM_SHARE_DENYALL, FALSE );
        BaseStorageStream* pStream = NULL;

        // if stream is already open, it is allowed to copy it, so be aware of this
        BOOL bDeleteStream = FALSE;
        if ( rElement.m_xStream.Is() )
            pStream = rElement.m_xStream->m_pAntiImpl;
        if ( !pStream )
        {
            pStream = ( const_cast < UCBStorage* > (this) )->OpenStream( rElement.m_aName, pImp->m_nMode, FALSE );
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
        BaseStorage* pOtherStorage = pDest->OpenStorage( rElement.m_aName, STREAM_WRITE | STREAM_SHARE_DENYALL, FALSE );
        BaseStorage* pStorage = NULL;

        // if stream is already open, it is allowed to copy it, so be aware of this
        BOOL bDeleteStorage = FALSE;
        if ( rElement.m_xStorage.Is() )
            pStorage = rElement.m_xStorage->m_pAntiImpl;
        if ( !pStorage )
        {
            pStorage = ( const_cast < UCBStorage* > (this) )->OpenStorage( rElement.m_aName, pImp->m_nMode, FALSE );
            bDeleteStorage = TRUE;
        }

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
        if ( pElement->m_aName == rName )
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

    pDestStg->SetClassId( GetClassId() );
    pDestStg->SetDirty();

    BOOL bRet = TRUE;
    UCBStorageElement_Impl* pElement = pImp->m_aChildrenList.First();
    while ( pElement && ( bRet = CopyStorageElement_Impl( *pElement, pDestStg, pElement->m_aName ) ) )
        pElement = pImp->m_aChildrenList.Next();

    if( !bRet )
        SetError( pDestStg->GetError() );
    return BOOL( Good() && pDestStg->Good() );

}

BOOL UCBStorage::CopyTo( const String& rElemName, BaseStorage* pDest, const String& rNew )
{
    if( !rElemName.Len() )
        return NULL;

    // perhaps an optimization is possible if pDest is the same storage or belongs to the same root as this ?!
    // if ( pDest == ((BaseStorage*) this) )

    UCBStorageElement_Impl* pElement = FindElement_Impl( rElemName );
    if ( pElement )
        return CopyStorageElement_Impl( *pElement, pDest, rNew );
    else
    {
        SetError( SVSTREAM_FILE_NOT_FOUND );
        return FALSE;
    }
}

BOOL UCBStorage::Commit()
{
    // mark this stream for sending it on root commit
    pImp->m_bCommited = TRUE;
    if ( pImp->m_bIsRoot )
        // the root storage coordinates commiting by sending a Commit command to its content
        return pImp->Commit();
    else
        return TRUE;
}

BOOL UCBStorage_Impl::Commit()
{
    UCBStorageElement_Impl* pElement = m_aChildrenList.First();
    BOOL bRet = TRUE;
    if ( m_bCommited )
    {
        while ( pElement && bRet )
        {
            if ( pElement->m_bIsRemoved )
            {
                // execute command "delete"
            }
            else
            {
                // check also for changes in name, content type
                if ( pElement->m_aName != pElement->m_aOriginalName )
                {
                    // set property value "Title"
                }

                if ( pElement->m_aContentType != pElement->m_aOriginalContentType )
                {
                    // set property value "MediaType"
                }

                if ( pElement->m_xStorage.Is() )
                    bRet = pElement->m_xStorage->Commit();
                else if ( pElement->m_xStream.Is() )
                    bRet = pElement->m_xStream->Commit();
            }

            pElement = m_aChildrenList.Next();
        }

        if ( m_bIsRoot )
        {
            if ( bRet )
            {
                Any aAny;
                try
                {
                    m_pContent->executeCommand( ::rtl::OUString::createFromAscii("commit"), aAny );
                    if ( m_pSource )
                        *m_pStream >> *m_pSource;
                }
                catch ( ... )
                {
                    return FALSE;
                }
            }
            else
            {
                // how to tell the content : forget all changes ?!
                // and also: must all commited elements be reverted ?!
                return FALSE;
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
                    pElement->m_aOriginalContentType = pElement->m_aContentType;
                }

                pElement = pNext;
            }
        }

        m_bCommited = FALSE;
    }

    return bRet;
}

BOOL UCBStorage::Revert()
{
    // revert all elements ( be aware of changes in names, content types and the "removed" flag )
    return SVSTREAM_OK == GetError();
}

BaseStorageStream* UCBStorage::OpenStream( const String& rEleName, StreamMode nMode, BOOL bDirect )
{
    DBG_ASSERT( !bDirect, "No direct mode supported!" );
    if( !rEleName.Len() )
        return NULL;

    // try to find the storage element
    UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    if ( !pElement )
    {
        // element does not exist, check if creation is allowed
        if( !( nMode & STREAM_NOCREATE ) )
        {
            // create a new UCBStorageElement and insert it into the list
            pElement = new UCBStorageElement_Impl( rEleName, String(), FALSE, 0 );
            pImp->m_aChildrenList.Insert( pElement, LIST_APPEND );
        }
        else
            SetError( ( nMode & STREAM_WRITE ) ? SVSTREAM_CANNOT_MAKE : SVSTREAM_FILE_NOT_FOUND );
    }

    // Should we allow for opening storages as streams ???
    if ( pElement && !pElement->m_bIsStorage )
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
                return new UCBStorageStream( pElement->m_xStream );
        }
        else
        {
            // stream is opened the first time
            UCBStorageStream* pStream = new UCBStorageStream( pElement->m_aOriginalName, nMode );
            pElement->m_xStream = pStream->pImp;

            // if name has been changed before creating the stream: set name!
            pStream->pImp->m_aName = rEleName;
            return pStream;
        }
    }

    return NULL;
}

BaseStorage* UCBStorage::OpenStorage( const String& rEleName, StreamMode nMode, BOOL bDirect )
{
    DBG_ASSERT( !bDirect, "No direct mode supported!" );
    if( !rEleName.Len() )
        return NULL;

    // try to find the storage element
    UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    if ( !pElement )
    {
        // element does not exist, check if creation is allowed
        if( !( nMode & STREAM_NOCREATE ) )
        {
            // create a new UCBStorageElement and insert it into the list
            // problem: perhaps an OLEStorage should be created ?!
            pElement = new UCBStorageElement_Impl( rEleName, String(), TRUE, 0 );
            pImp->m_aChildrenList.Insert( pElement, LIST_APPEND );
        }
        else
            SetError( ( nMode & STREAM_WRITE ) ? SVSTREAM_CANNOT_MAKE : SVSTREAM_FILE_NOT_FOUND );
    }

    // pure streams can't be opened as storages
    if ( pElement && pElement->m_bIsStorage )
    {
        if ( !pElement->m_bIsFolder )
        {
            // Here we need some code to create OLE storages on a stream ( see ctor of SotStorage )
            // Such a storage will be created on a UCBStorageStream; it will write into the stream
            // if it is opened in direct mode or when it is committed. In this case the stream will be
            // modified and then it MUST be treated as commited.
            UCBStorageStream* pStream = PTR_CAST( UCBStorageStream, OpenStream( rEleName, nMode, bDirect ) );
            if ( !pStream )
            {
                SetError( ( nMode & STREAM_WRITE ) ? SVSTREAM_CANNOT_MAKE : SVSTREAM_FILE_NOT_FOUND );
                return NULL;
            }

            pElement->m_xStream = pStream->pImp;
            delete pStream;
            return pStream->pImp->CreateStorage();  // can only be created in transacted mode
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
                return new UCBStorage( pElement->m_xStorage );
        }
        else
        {
            // storage is opened the first time
            UCBStorage *pStorage = new UCBStorage( pElement->m_aOriginalName, nMode );
            pStorage->pImp->m_bIsRoot = FALSE;

            // if name has been changed before creating the stream: set name!
            pStorage->pImp->m_aName = rEleName;
            pElement->m_xStorage = pStorage->pImp;
            return pStorage;
        }
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
        if ( pElement->m_bIsRemoved )
            return FALSE;                   // already removed
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
        // if there is an existing element marked for removing with name rNewName, it can be made anonymous
        if ( !pAlreadyExisting->m_bIsRemoved )
        {
            SetError( SVSTREAM_ACCESS_DENIED );
            return FALSE;                       // can't change to a name that is already used
        }
    }

    UCBStorageElement_Impl *pElement = FindElement_Impl( rEleName );
    if ( pElement )
    {
        if ( pAlreadyExisting )
            pAlreadyExisting->m_aName.Erase();
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

    // perhaps an optimization is possible if pDest is the same storage or belongs to the same root as this ?!
    // if ( pDest == ((BaseStorage*) this) )

    // MoveTo is done by first copying to the new destination and then removing the old element
    BOOL bRet = CopyTo( rEleName, pNewSt, rNewName );
    if ( bRet )
        bRet = Remove( rEleName );
    return bRet;
}

BOOL UCBStorage::ValidateFAT()
{
    // ???
    return TRUE;
}

BOOL UCBStorage::Validate( BOOL ) const
{
    // ???
    return TRUE;
}

BOOL UCBStorage::ValidateMode( StreamMode ) const
{
    // ???
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
