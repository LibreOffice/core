#include "unotools/ucblockbytes.hxx"

#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#endif
#ifndef  _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_CHAOS_OPENMODE_HPP_
#include <com/sun/star/chaos/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGENOTIFIER_HPP_
#include <com/sun/star/beans/XPropertiesChangeNotifier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATACONTROL_HPP_
#include <com/sun/star/io/XActiveDataControl.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef  _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

#include <ucbhelper/content.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::chaos;
using namespace ::com::sun::star::lang;

class ProgressHandler_Impl: public cppu::OWeakObject, public XProgressHandler
{
    Link                    m_aProgress;

public:
                            ProgressHandler_Impl() {}

    void                    SetProgressLink( const Link& rLink )
                                { m_aProgress = rLink; }

    virtual Any SAL_CALL    queryInterface( const Type & rType )
                                throw (RuntimeException);

    virtual void SAL_CALL   acquire() throw (RuntimeException)
                                { OWeakObject::acquire(); }

    virtual void SAL_CALL   release() throw (RuntimeException)
                                { OWeakObject::release(); }

    virtual void SAL_CALL   push(const Any & rStatus) throw (RuntimeException);

    virtual void SAL_CALL   update(const Any & rStatus)
                                throw (RuntimeException);

    virtual void SAL_CALL   pop() throw (RuntimeException);
};

//----------------------------------------------------------------------------
Any SAL_CALL ProgressHandler_Impl::queryInterface( const Type & rType ) throw (RuntimeException)
{
    Any aRet( ::cppu::queryInterface(   rType,
                                        static_cast< XProgressHandler* >( this )));

    if ( aRet.hasValue() )
        return aRet ;
    else
        return OWeakObject::queryInterface( rType );
}

//----------------------------------------------------------------------------
void SAL_CALL ProgressHandler_Impl::push( const Any & rStatus ) throw (RuntimeException)
{
}

//----------------------------------------------------------------------------
void SAL_CALL ProgressHandler_Impl::update( const Any & rStatus ) throw (RuntimeException)
{
    if ( m_aProgress.IsSet() )
        m_aProgress.Call( 0 );
}

//----------------------------------------------------------------------------
void SAL_CALL ProgressHandler_Impl::pop() throw (RuntimeException)
{
}

class UcbTaskEnvironment : public ::cppu::WeakImplHelper1< XCommandEnvironment >
{
    Reference< XInteractionHandler >    m_xInteractionHandler;
    Reference< XProgressHandler >       m_xProgressHandler;

public:
            UcbTaskEnvironment( const Reference< XInteractionHandler>& rxInteractionHandler,
                                const Reference< XProgressHandler>& rxProgressHandler );

    virtual Reference<XInteractionHandler> SAL_CALL getInteractionHandler() throw (RuntimeException)
    { return m_xInteractionHandler; }

    virtual Reference<XProgressHandler> SAL_CALL getProgressHandler() throw (RuntimeException)
    { return m_xProgressHandler; }
};

class CommandThread_Impl : public ::vos::OThread
{
    Reference< XContent >               m_xContent;
    Reference< XInteractionHandler >    m_xInteract;
    Reference< XProgressHandler >       m_xProgress;
    ::ucb::Content*         m_pContent;

    OpenCommandArgument2    m_aArgument;
    UcbLockBytesRef         m_xLockBytes;
    UCB_Link_HelperRef      m_xLink;
    sal_Bool                m_bCanceled : 1;
    sal_Bool                m_bRunning  : 1;
    sal_Bool                m_bSimple   : 1;

public:
                    CommandThread_Impl( Reference < XContent > xContent,
                                        const OpenCommandArgument2& rArg,
                                        Reference < XInteractionHandler > xInteract,
                                        Reference < XProgressHandler > xProgress,
                                        UCB_Link_HelperRef xLink )
                        : m_xInteract( xInteract )
                        , m_xContent( xContent )
                        , m_xProgress( xProgress )
                        , m_aArgument( rArg )
                        , m_xLink( xLink )
                        , m_bCanceled( sal_False )
                        , m_bRunning( sal_False )
                        , m_bSimple( sal_False )
                    {
                        m_pContent = new ::ucb::Content( xContent, new UcbTaskEnvironment( m_xInteract, m_xProgress ) );
                    }

                    CommandThread_Impl( UcbLockBytesRef xLockBytes )
                        : m_xLockBytes( xLockBytes )
                        , m_bCanceled( sal_False )
                        , m_bRunning( sal_False )
                        , m_bSimple( sal_True )
                    {}

                    ~CommandThread_Impl()
                    {
                        delete m_pContent;
                    }

    virtual void SAL_CALL   onTerminated();
    virtual void SAL_CALL   run();
    void                    Cancel();
};

//----------------------------------------------------------------------------
void CommandThread_Impl::run()
{
    if ( m_bSimple )
    {
        m_bRunning = sal_True;
        m_xLockBytes->terminate_Impl();
        m_bRunning = sal_False;
        return;
    }

    m_bRunning = sal_True;

    if( !m_bCanceled && schedule() )
    {
        Any aParam;
        aParam <<= m_aArgument;
        Any aResult;
        bool bException = false;
        bool bAborted = false;

        m_pContent = new ::ucb::Content( m_xContent, new UcbTaskEnvironment( m_xInteract, m_xProgress ) );

        try
        {
            aResult = m_pContent->executeCommand( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("open") ), aParam );
        }
        catch ( CommandAbortedException )
        {
            bAborted = true;
        }
        catch ( Exception )
        {
            bException = true;
        }

        if ( bAborted || bException )
        {
            if( m_xLink.Is() )
                m_xLink->Cancel();

            Reference < XInputStream > aDummy;
            Reference < XActiveDataSink > ::query(m_aArgument.Sink)->setInputStream( aDummy );
        }

        Reference < XActiveDataControl > ::query(m_aArgument.Sink)->terminate();
    }

    m_bRunning = sal_False;
}

//----------------------------------------------------------------------------
void CommandThread_Impl::onTerminated()
{
    delete this;
}

//----------------------------------------------------------------------------
void CommandThread_Impl::Cancel()
{
    if ( m_bCanceled )
        return;

    m_bCanceled = sal_True;

    if ( m_bRunning && !m_bSimple )
    {
        m_pContent->abortCommand();
        m_bRunning = sal_False;
    }
}

//----------------------------------------------------------------------------
UcbLockBytes::~UcbLockBytes()
{
    if ( !m_bDontClose && m_xInputStream.is() )
        m_xInputStream->closeInput();
}

//----------------------------------------------------------------------------
sal_Bool UcbLockBytes::setInputStream_Impl( const Reference<XInputStream> &rxInputStream )
{
    BOOL bRet;

    vos::OClearableGuard aGuard( m_aMutex );

    if ( !m_bDontClose && m_xInputStream.is() )
        m_xInputStream->closeInput();

    m_xInputStream = rxInputStream;
    bRet = m_xInputStream.is();
    aGuard.clear();

    m_aInitialized.set();

    return bRet;
}

//----------------------------------------------------------------------------
void UcbLockBytes::terminate_Impl()
{
    m_pCommandThread = NULL;

    Reference<XInputStream> xStream = getInputStream_Impl();
    Reference<XSeekable> xSeekable (xStream, UNO_QUERY);

    if ( xSeekable.is() )
        m_nSize = sal_uInt32(xSeekable->getLength());

    m_bTerminated = sal_True;

    m_aInitialized.set();
    m_aTerminated.set();

    m_aLinkList->Done();
}

//----------------------------------------------------------------------------
void UcbLockBytes::SetSynchronMode (BOOL bSynchron)
{
    SvLockBytes::SetSynchronMode (bSynchron);
}

//----------------------------------------------------------------------------
ErrCode UcbLockBytes::ReadAt ( ULONG nPos, void *pBuffer, ULONG nCount, ULONG *pRead) const
{
    Reference<XInputStream> xStream = getInputStream_Impl();

    if ( !xStream.is() )
    {
        if ( m_bTerminated )
            return ERRCODE_IO_CANTREAD;
        else
            return ERRCODE_IO_PENDING;
    }

    if ( pRead )
        *pRead = 0;

    Reference < XSeekable > xSeekable( xStream, UNO_QUERY );
    if ( !xSeekable.is() )
        return ERRCODE_IO_CANTREAD;

    try
    {
        xSeekable->seek( nPos );
    }
    catch ( IOException )
    {
        return ERRCODE_IO_CANTSEEK;
    }

    Sequence<sal_Int8> aData;
    sal_Int32          nSize;

    nCount = VOS_MIN(nCount, 0x7FFFFFFF);
    try
    {
        if ( !m_bTerminated && !IsSynchronMode() )
        {
            sal_Int64 nLen = xSeekable->getLength();
            if ( nPos + nCount > nLen )
                return ERRCODE_IO_PENDING;
        }

        nSize = xStream->readBytes( aData, sal_Int32(nCount) );
    }
    catch (IOException)
    {
        return ERRCODE_IO_CANTREAD;
    }

    rtl_copyMemory (pBuffer, aData.getConstArray(), nSize);
    if (pRead)
        *pRead = ULONG(nSize);

    sal_uInt32 nRead = VOS_MAX(m_nRead, nPos + nSize);
    SAL_CONST_CAST(UcbLockBytes*, this)->m_nRead = nRead;

    return ERRCODE_NONE;
}

//----------------------------------------------------------------------------
ErrCode UcbLockBytes::WriteAt ( ULONG, const void*, ULONG, ULONG *pWritten)
{
    if ( pWritten )
        *pWritten = 0;
    return ERRCODE_IO_CANTWRITE;
}

//----------------------------------------------------------------------------
ErrCode UcbLockBytes::Flush (void) const
{
    return ERRCODE_NONE;
}

//----------------------------------------------------------------------------
ErrCode UcbLockBytes::SetSize (ULONG)
{
    return ERRCODE_IO_NOTSUPPORTED;
}

//----------------------------------------------------------------------------
ErrCode UcbLockBytes::Stat( SvLockBytesStat *pStat, SvLockBytesStatFlag) const
{
    if (!pStat)
        return ERRCODE_IO_INVALIDPARAMETER;

    Reference<XInputStream> xStream = getInputStream_Impl();

    if (!xStream.is())
        return ERRCODE_IO_INVALIDACCESS;

    Reference<XSeekable> xSeekable (xStream, UNO_QUERY);
    if (!xSeekable.is())
        return ERRCODE_IO_INVALIDACCESS;

    try
    {
        pStat->nSize = ULONG(xSeekable->getLength());
    }
    catch (IOException)
    {
        return ERRCODE_IO_CANTTELL;
    }

    if (!m_bTerminated)
        return ERRCODE_IO_PENDING;
    else
        return ERRCODE_NONE;
}

//----------------------------------------------------------------------------
void UcbLockBytes::Cancel()
{
    if ( m_bTerminated )
        return;

    if ( m_pCommandThread )
    {
        m_pCommandThread->Cancel();
        m_pCommandThread = NULL;
    }
}

//----------------------------------------------------------------------------
IMPL_LINK( UcbLockBytes, DataAvailHdl, void*, EMPTYARG )
{
    if ( hasInputStream_Impl() )
        m_aLinkList->DataAvail();

    return 0;
}

//----------------------------------------------------------------------------
// class UcbDataSink_Impl
//----------------------------------------------------------------------------
class UcbDataSink_Impl : public ::cppu::WeakImplHelper2< XActiveDataControl, XActiveDataSink >
{
    UcbLockBytesRef         m_xLockBytes;

public:
                            UcbDataSink_Impl( UcbLockBytes* pLockBytes )
                                : m_xLockBytes( pLockBytes )
                            {}

    SvLockBytes*            getLockBytes (void)
                            { return m_xLockBytes; }

    // XActiveDataControl.
    virtual void SAL_CALL   addListener ( const Reference<XStreamListener> &rxListener) throw(RuntimeException);
    virtual void SAL_CALL   removeListener ( const Reference<XStreamListener> &rxListener) throw(RuntimeException);
    virtual void SAL_CALL   start (void) throw(RuntimeException);
    virtual void SAL_CALL   terminate (void) throw(RuntimeException);

    // XActiveDataSink.
    virtual void SAL_CALL   setInputStream ( const Reference<XInputStream> &rxInputStream) throw(RuntimeException);
    virtual Reference<XInputStream> SAL_CALL getInputStream (void) throw(RuntimeException);
};

void SAL_CALL UcbDataSink_Impl::addListener ( const Reference<XStreamListener> &rxListener) throw(RuntimeException)
{
    // not supported
}

void SAL_CALL UcbDataSink_Impl::removeListener ( const Reference<XStreamListener> &rxListener) throw(RuntimeException)
{
    // not supported
}

void SAL_CALL UcbDataSink_Impl::start (void) throw(RuntimeException)
{
}

void SAL_CALL UcbDataSink_Impl::terminate (void) throw(RuntimeException)
{
    m_xLockBytes->terminate_Impl();
}

void SAL_CALL UcbDataSink_Impl::setInputStream ( const Reference<XInputStream> &rxInputStream) throw(RuntimeException)
{
    m_xLockBytes->setInputStream_Impl (rxInputStream);
}

Reference<XInputStream> SAL_CALL UcbDataSink_Impl::getInputStream (void) throw(RuntimeException)
{
    return m_xLockBytes->getInputStream_Impl();
}

//-------------------------------------------------------------------------
UcbTaskEnvironment::UcbTaskEnvironment( const Reference< XInteractionHandler >& rxInteractionHandler,
                                        const Reference< XProgressHandler >& rxProgressHandler )
    : m_xInteractionHandler( rxInteractionHandler )
    , m_xProgressHandler( rxProgressHandler )
{
}

UcbLockBytesRef UcbLockBytes::CreateInputLockBytes( const Reference < XContent > xContent, UCB_Link_HelperRef xLink )
{
    if( !xContent.is() )
        return NULL;;
/*
    Reference<XPropertiesChangeNotifier> xProps ( xContent, UNO_QUERY );
    if ( xProps.is() )
    {
        m_aContentType = getContentType_Impl (xProps);
        if( !m_aContentType.Len() )
            m_aContentType = CONTENT_TYPE_STR_APP_OCTSTREAM;
        m_bMimeAvail = sal_True;
        xProps->addPropertiesChangeListener ( Sequence< OUString >(), this );
    }
*/
    Reference< XMultiServiceFactory > xFactory = ::utl::getProcessServiceFactory();
    Reference< XInteractionHandler > xInteractionHandler = Reference< XInteractionHandler > (
                xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uui.InteractionHandler") ) ), UNO_QUERY );

    OpenCommandArgument2 aArgument;
    String aCmd( RTL_CONSTASCII_USTRINGPARAM( "Open" ) );
    UcbLockBytesRef xLockBytes = new UcbLockBytes( xLink );
    Reference < XActiveDataSink > xSink = new UcbDataSink_Impl( xLockBytes );
    aArgument.Sink = xSink;
    aArgument.Mode = OpenMode::DOCUMENT;

    ProgressHandler_Impl            *pProgressHdl = new ProgressHandler_Impl();
    Reference< XProgressHandler >    xProgressHdl = pProgressHdl;
    pProgressHdl->SetProgressLink( LINK( &xLockBytes, UcbLockBytes, DataAvailHdl ) );

    ::vos::OThread *pThread = new CommandThread_Impl( xContent, aArgument, xInteractionHandler, xProgressHdl, xLink );
    xLockBytes->setCommandThread_Impl( (CommandThread_Impl*) pThread );
    pThread->create();

    return xLockBytes;
}

UcbLockBytesRef UcbLockBytes::CreateInputLockBytes( const Reference< XInputStream > xInputStream,
                                                 UCB_Link_HelperRef xLink )
{
    if( !xInputStream.is() )
        return NULL;;

    String aCmd( RTL_CONSTASCII_USTRINGPARAM( "Open" ) );
    UcbLockBytesRef xLockBytes = new UcbLockBytes( xLink );
    Reference < XActiveDataSink > xSink = new UcbDataSink_Impl( xLockBytes );

    xLockBytes->setDontClose_Impl();
    xLockBytes->setInputStream_Impl( xInputStream );

    // we have to create a thread here so ::terminate() will be called
    // asynchronously
    ::vos::OThread *pThread = new CommandThread_Impl( xLockBytes );
    pThread->create();

    return xLockBytes;
}

void UCB_Link_Helper::SetDoneLink( const Link& rLink )
{
    ::vos::OGuard aGuard( maMutex );
    mbSet = TRUE;
    maDoneLink = rLink;
}

//----------------------------------------------------------------------------
void UCB_Link_Helper::SetDataAvailLink( const Link& rLink )
{
    ::vos::OGuard aGuard( maMutex );
    mbSet = TRUE;
    maDataAvailLink = rLink;
}

//----------------------------------------------------------------------------
void UCB_Link_Helper::SetCancelLink( const Link& rLink )
{
    ::vos::OGuard aGuard( maMutex );
    mbSet = TRUE;
    maCancelLink = rLink;
}

//----------------------------------------------------------------------------
void UCB_Link_Helper::Done()
{
    ::vos::OGuard aGuard( maMutex );

    if ( maDoneLink.IsSet() )
        maDoneLink.Call( 0 );
}

//----------------------------------------------------------------------------
void UCB_Link_Helper::DataAvail()
{
    ::vos::OGuard aGuard( maMutex );

    if ( maDataAvailLink.IsSet() )
        maDataAvailLink.Call( 0 );
}

//----------------------------------------------------------------------------
void UCB_Link_Helper::Cancel()
{
    ::vos::OGuard aGuard( maMutex );

    if ( maCancelLink.IsSet() )
        maCancelLink.Call( 0 );
}

//----------------------------------------------------------------------------
void UCB_Link_Helper::Clear()
{
    ::vos::OGuard aGuard( maMutex );

    if ( mbSet )
    {
        maDoneLink = Link();
        maDataAvailLink = Link();
        maCancelLink = Link();
        mbSet = FALSE;
    }
}


