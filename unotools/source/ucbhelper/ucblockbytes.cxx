/*************************************************************************
 *
 *  $RCSfile: ucblockbytes.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: mba $ $Date: 2001-04-03 15:06:31 $
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

#include <unotools/ucblockbytes.hxx>
#include <comphelper/processfactory.hxx>

#ifndef _COM_SUN_STAR_IO_XACTIVEDATASTREAMER_HPP_
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_DOCUMENTHEADERFIELD_HPP_
#include <com/sun/star/ucb/DocumentHeaderField.hpp>
#endif
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
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
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
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
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
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _TOOLS_INETMSG_HXX
#include <tools/inetmsg.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XTRUNCATE_HPP_
#include <com/sun/star/io/XTruncate.hpp>
#endif

#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/content.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

namespace utl
{

/**
    Helper class for getting a XInputStream when opening a content
 */
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
    virtual void SAL_CALL   addListener ( const Reference<XStreamListener> &rxListener) throw(RuntimeException) {}
    virtual void SAL_CALL   removeListener ( const Reference<XStreamListener> &rxListener) throw(RuntimeException) {}
    virtual void SAL_CALL   start (void) throw(RuntimeException) {}
    virtual void SAL_CALL   terminate (void) throw(RuntimeException)
                            { m_xLockBytes->terminate_Impl(); }

    // XActiveDataSink.
    virtual void SAL_CALL   setInputStream ( const Reference<XInputStream> &rxInputStream) throw(RuntimeException)
                            { m_xLockBytes->setInputStream_Impl (rxInputStream); }
    virtual Reference<XInputStream> SAL_CALL getInputStream (void) throw(RuntimeException)
                            { return m_xLockBytes->getInputStream_Impl(); }
};

/**
    Helper class for getting a XStream when opening a content
 */
class UcbStreamer_Impl : public ::cppu::WeakImplHelper2< XActiveDataStreamer, XActiveDataControl >
{
    Reference < XStream >   m_xStream;
    UcbLockBytesRef         m_xLockBytes;

public:

                            UcbStreamer_Impl( UcbLockBytes* pLockBytes )
                                : m_xLockBytes( pLockBytes )
                            {}

    // XActiveDataControl.
    virtual void SAL_CALL   addListener ( const Reference<XStreamListener> &rxListener) throw(RuntimeException) {}
    virtual void SAL_CALL   removeListener ( const Reference<XStreamListener> &rxListener) throw(RuntimeException) {}
    virtual void SAL_CALL   start (void) throw(RuntimeException) {}
    virtual void SAL_CALL   terminate (void) throw(RuntimeException)
                            { m_xLockBytes->terminate_Impl(); }

    // XActiveDataStreamer
    virtual void SAL_CALL   setStream( const Reference< XStream >& aStream ) throw(RuntimeException)
                            { m_xStream = aStream; m_xLockBytes->setStream_Impl( aStream ); }
    virtual Reference< XStream > SAL_CALL getStream() throw(RuntimeException)
                            { return m_xStream; }
};

/**
    Helper class for progress handling while executing UCB commands
 */
class ProgressHandler_Impl: public ::cppu::WeakImplHelper1< XProgressHandler >
{
    Link                    m_aProgress;

public:
                            ProgressHandler_Impl( const Link& rLink )
                                : m_aProgress( rLink )
                            {}
    // XProgressHandler
    virtual void SAL_CALL   push(const Any & rStatus) throw (RuntimeException) {}
    virtual void SAL_CALL   pop() throw (RuntimeException) {}
    virtual void SAL_CALL   update(const Any & rStatus) throw (RuntimeException)
                            { if ( m_aProgress.IsSet() ) m_aProgress.Call( 0 ); }
};

/**
    Helper class for managing interactions and progress when executing UCB commands
 */
class UcbTaskEnvironment : public ::cppu::WeakImplHelper1< XCommandEnvironment >
{
    Reference< XInteractionHandler >                m_xInteractionHandler;
    Reference< XProgressHandler >                   m_xProgressHandler;

public:
                            UcbTaskEnvironment( const Reference< XInteractionHandler>& rxInteractionHandler,
                                                const Reference< XProgressHandler>& rxProgressHandler )
                                : m_xInteractionHandler( rxInteractionHandler )
                                , m_xProgressHandler( rxProgressHandler )
                            {}


    virtual Reference<XInteractionHandler> SAL_CALL getInteractionHandler() throw (RuntimeException)
    { return m_xInteractionHandler; }

    virtual Reference<XProgressHandler> SAL_CALL    getProgressHandler() throw (RuntimeException)
    { return m_xProgressHandler; }
};


/**
    Helper class for property change notifies when executing UCB commands
*/
class UcbPropertiesChangeListener_Impl : public ::cppu::WeakImplHelper1< XPropertiesChangeListener >
{
public:
    UcbLockBytesRef         m_xLockBytes;

                            UcbPropertiesChangeListener_Impl( UcbLockBytesRef rRef )
                                : m_xLockBytes( rRef )
                            {}

    virtual void SAL_CALL   disposing ( const EventObject &rEvent) throw(RuntimeException) {}
    virtual void SAL_CALL   propertiesChange ( const Sequence<PropertyChangeEvent> &rEvent) throw(RuntimeException);
};

void SAL_CALL UcbPropertiesChangeListener_Impl::propertiesChange ( const Sequence<PropertyChangeEvent> &rEvent) throw(RuntimeException)
{
    sal_Int32 i, n = rEvent.getLength();
    for (i = 0; i < n; i++)
    {
        PropertyChangeEvent evt (rEvent[i]);
        if (evt.PropertyName == ::rtl::OUString::createFromAscii ("DocumentHeader"))
        {
            Sequence<DocumentHeaderField> aHead;
            if (evt.NewValue >>= aHead)
            {
                sal_Int32 k, m = aHead.getLength();
                for (k = 0; k < m; k++)
                {
                    String aName( aHead[k].Name );
                    String aValue( aHead[k].Value );

                    if (aName.CompareIgnoreCaseToAscii("Expires") == COMPARE_EQUAL)
                    {
                        DateTime aExpires (0, 0);
                        if (INetRFC822Message::ParseDateField (aValue, aExpires))
                        {
                            aExpires.ConvertToLocalTime();
                            m_xLockBytes->SetExpireDate_Impl( aExpires );
                        }
                    }
                }
            }

            m_xLockBytes->SetStreamValid_Impl();
        }
        else if (evt.PropertyName == rtl::OUString::createFromAscii ("PresentationURL"))
        {
            ::rtl::OUString aUrl;
            if (evt.NewValue >>= aUrl)
            {
                ::rtl::OUString aBad (::rtl::OUString::createFromAscii ("private:"));
                if (!(aUrl.compareTo (aBad, aBad.getLength()) == 0))
                {
                    // URL changed (Redirection).
                    m_xLockBytes->SetRealURL_Impl( aUrl );
                }
            }
        }
        else if (evt.PropertyName == ::rtl::OUString::createFromAscii ("MediaType"))
        {
            ::rtl::OUString aContentType;
            if (evt.NewValue >>= aContentType)
                m_xLockBytes->SetContentType_Impl( aContentType );
        }
    }
}

/**
    Helper class for opening UCB contents synchronously or asynchronously
 */
class CommandThread_Impl : public ::vos::OThread
{
public:

    Reference < XContent >                  m_xContent;
    Reference < XInteractionHandler >       m_xInteract;
    Reference < XProgressHandler >          m_xProgress;
    Reference < XPropertiesChangeListener > m_xListener;
    ::ucb::Content*                         m_pContent;
    OpenCommandArgument2                    m_aArgument;
    UcbLockBytesRef                         m_xLockBytes;
    UcbLockBytesHandlerRef                  m_xHandler;
    sal_Bool                                m_bCanceled : 1;
    sal_Bool                                m_bRunning  : 1;

                    CommandThread_Impl( UcbLockBytesRef xLockBytes,
                                        Reference < XContent > xContent,
                                        const OpenCommandArgument2& rArg,
                                        Reference < XInteractionHandler > xInteract,
                                        Reference < XProgressHandler > xProgress,
                                        UcbLockBytesHandlerRef xRef );

                    ~CommandThread_Impl();

    virtual void SAL_CALL   onTerminated();
    virtual void SAL_CALL   run();
    void                    Cancel();
    sal_Bool                DoIt();
};

//----------------------------------------------------------------------------
CommandThread_Impl::CommandThread_Impl( UcbLockBytesRef xLockBytes,
                    Reference < XContent > xContent,
                    const OpenCommandArgument2& rArg,
                    Reference < XInteractionHandler > xInteract,
                    Reference < XProgressHandler > xProgress,
                    UcbLockBytesHandlerRef xRef )
    : m_xInteract( xInteract )
    , m_xContent( xContent )
    , m_xProgress( xProgress )
    , m_xLockBytes( xLockBytes )
    , m_xHandler( xRef )
    , m_aArgument( rArg )
    , m_bCanceled( sal_False )
    , m_bRunning( sal_False )
{
    m_xLockBytes->setCommandThread_Impl( this );
    m_pContent = new ::ucb::Content( xContent, new UcbTaskEnvironment( m_xInteract, m_xProgress ) );
    Reference < XContentIdentifier > xIdent = xContent->getIdentifier();
    ::rtl::OUString aScheme = xIdent->getContentProviderScheme();

    // http protocol must be handled in a special way: during the opening process the input stream may change
    // only the last inputstream after notifying the document headers is valid
    if ( aScheme.compareToAscii("http") != COMPARE_EQUAL )
        m_xLockBytes->SetStreamValid_Impl();

    m_xListener = new UcbPropertiesChangeListener_Impl( m_xLockBytes );
    Reference< XPropertiesChangeNotifier > xProps ( xContent, UNO_QUERY );
    if ( xProps.is() )
        xProps->addPropertiesChangeListener( Sequence< ::rtl::OUString >(), m_xListener );
}

CommandThread_Impl::~CommandThread_Impl()
{
    m_xLockBytes->setCommandThread_Impl(0);
    Reference< XPropertiesChangeNotifier > xProps ( m_pContent->get(), UNO_QUERY );
    if ( xProps.is() )
        xProps->removePropertiesChangeListener( Sequence< ::rtl::OUString >(), m_xListener );
    delete m_pContent;
}

void CommandThread_Impl::run()
{
    m_bRunning = sal_True;

    if( !m_bCanceled && schedule() )
        DoIt();

    m_bRunning = sal_False;
}

sal_Bool CommandThread_Impl::DoIt()
{
    // create content and execute command
    Any aParam;
    aParam <<= m_aArgument;
    Any aResult;
    bool bException = false;
    bool bAborted = false;

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
        if( m_xHandler.Is() )
            m_xHandler->Handle( UcbLockBytesHandler::CANCEL, m_xLockBytes );

        Reference < XActiveDataSink > xSink( m_aArgument.Sink, UNO_QUERY );
        if ( xSink.is() )
            xSink->setInputStream( Reference < XInputStream >() );

        Reference < XActiveDataStreamer > xStreamer( m_aArgument.Sink, UNO_QUERY );
        if ( xStreamer.is() )
            xStreamer->setStream( Reference < XStream >() );
    }

    Reference < XActiveDataControl > xControl( m_aArgument.Sink, UNO_QUERY );
    if ( xControl.is() )
        xControl->terminate();

    return ( bAborted || bException );
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

    if ( m_bRunning && m_pContent )
    {
        m_pContent->abortCommand();
        m_bRunning = sal_False;
    }
}

//----------------------------------------------------------------------------
UcbLockBytes::UcbLockBytes( UcbLockBytesHandler* pHandler )
    : m_xInputStream (NULL)
    , m_pCommandThread( NULL )
    , m_bTerminated  (sal_False)
    , m_bStreamValid  (sal_False)
    , m_bDontClose( sal_False )
    , m_xHandler( pHandler )
    , m_nError( ERRCODE_NONE )
{
}

//----------------------------------------------------------------------------
UcbLockBytes::~UcbLockBytes()
{
    if ( !m_bDontClose )
    {
        if ( m_xInputStream.is() )
        {
            try
            {
                m_xInputStream->closeInput();
            }
            catch ( RuntimeException const & )
            {}
            catch ( IOException const & )
            {}
        }

        if ( m_xOutputStream.is() )
        {
            try
            {
                m_xOutputStream->closeOutput();
            }
            catch ( RuntimeException const & )
            {}
            catch ( IOException const & )
            {}
        }
    }
}

//----------------------------------------------------------------------------

sal_Bool UcbLockBytes::setStream_Impl( const Reference<XStream>& aStream )
{
    vos::OClearableGuard aGuard( m_aMutex );
    if ( aStream.is() )
    {
        m_xOutputStream = aStream->getOutputStream();
        setInputStream_Impl( aStream->getInputStream() );
        m_xSeekable = Reference < XSeekable > ( aStream, UNO_QUERY );
    }
    else
    {
        m_xOutputStream = Reference < XOutputStream >();
        setInputStream_Impl( Reference < XInputStream >() );
    }

    return m_xInputStream.is();
}

sal_Bool UcbLockBytes::setInputStream_Impl( const Reference<XInputStream> &rxInputStream )
{
    BOOL bRet;

    vos::OClearableGuard aGuard( m_aMutex );
    if ( !m_bDontClose && m_xInputStream.is() )
        m_xInputStream->closeInput();

    m_xInputStream = rxInputStream;
    m_xSeekable = Reference < XSeekable > ( rxInputStream, UNO_QUERY );
    bRet = m_xInputStream.is();
    aGuard.clear();

    if ( m_bStreamValid && m_xInputStream.is() )
        m_aInitialized.set();

    return bRet;
}

void UcbLockBytes::SetStreamValid_Impl()
{
    m_bStreamValid = sal_True;
    if ( m_xInputStream.is() )
        m_aInitialized.set();
}

//----------------------------------------------------------------------------
void UcbLockBytes::terminate_Impl()
{
    m_pCommandThread = NULL;
    m_bTerminated = sal_True;
    m_aInitialized.set();
    m_aTerminated.set();

    if ( GetError() == ERRCODE_NONE && !m_xInputStream.is() )
        SetError( ERRCODE_IO_NOTEXISTS );

    if ( m_xHandler.Is() )
        m_xHandler->Handle( UcbLockBytesHandler::DONE, this );
}

//----------------------------------------------------------------------------
void UcbLockBytes::SetSynchronMode (BOOL bSynchron)
{
    SvLockBytes::SetSynchronMode (bSynchron);
}

//----------------------------------------------------------------------------
ErrCode UcbLockBytes::ReadAt ( ULONG nPos, void *pBuffer, ULONG nCount, ULONG *pRead) const
{
    if ( IsSynchronMode() )
    {
        UcbLockBytes* pThis = const_cast < UcbLockBytes* >( this );
        if ( m_xHandler.Is() )
            m_xHandler->Handle( UcbLockBytesHandler::BEFOREWAIT, pThis );
        pThis->m_aInitialized.wait();
        if ( m_xHandler.Is() )
            m_xHandler->Handle( UcbLockBytesHandler::AFTERWAIT, pThis );
    }

    Reference <XInputStream> xStream = getInputStream_Impl();
    if ( !xStream.is() )
    {
        if ( m_bTerminated )
            return ERRCODE_IO_CANTREAD;
        else
            return ERRCODE_IO_PENDING;
    }

    if ( pRead )
        *pRead = 0;

    Reference <XSeekable> xSeekable = getSeekable_Impl();
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

    return ERRCODE_NONE;
}

//----------------------------------------------------------------------------
ErrCode UcbLockBytes::WriteAt ( ULONG nPos, const void *pBuffer, ULONG nCount, ULONG *pWritten)
{
    if ( pWritten )
        *pWritten = 0;

    DBG_ASSERT( IsSynchronMode(), "Writing is only possible in SynchronMode!" );
    DBG_ASSERT( m_aInitialized.check(), "Writing bevor stream is ready!" );

    Reference <XSeekable> xSeekable = getSeekable_Impl();
    Reference <XOutputStream> xOutputStream = getOutputStream_Impl();
    if ( !xOutputStream.is() || !xSeekable.is() )
        return ERRCODE_IO_CANTWRITE;

    try
    {
        xSeekable->seek( nPos );
    }
    catch ( IOException )
    {
        return ERRCODE_IO_CANTSEEK;
    }

    sal_Int8* pData = (sal_Int8*) pBuffer;
    Sequence<sal_Int8> aData( pData, nCount );
    try
    {
        xOutputStream->writeBytes( aData );
        if ( pWritten )
            *pWritten = nCount;
    }
    catch ( Exception )
    {
        return ERRCODE_IO_CANTWRITE;
    }

    return ERRCODE_NONE;
}

//----------------------------------------------------------------------------
ErrCode UcbLockBytes::Flush() const
{
    Reference <XOutputStream > xOutputStream = getOutputStream_Impl();
    if ( !xOutputStream.is() )
        return ERRCODE_IO_CANTWRITE;
    xOutputStream->flush();
    return ERRCODE_NONE;
}

//----------------------------------------------------------------------------
ErrCode UcbLockBytes::SetSize (ULONG nNewSize)
{
    SvLockBytesStat aStat;
    Stat( &aStat, (SvLockBytesStatFlag) 0 );
    ULONG nSize = aStat.nSize;

    if ( nSize > nNewSize )
    {
        Reference < XTruncate > xTrunc( getOutputStream_Impl(), UNO_QUERY );
        if ( xTrunc.is() )
        {
            xTrunc->truncate();
            nSize = 0;
        }
        else
            DBG_WARNING("Not truncatable!");
    }

    if ( nSize < nNewSize )
    {
        ULONG nDiff = nNewSize-nSize, nCount=0;
        BYTE* pBuffer = new BYTE[ nDiff ];
        WriteAt( nSize, pBuffer, nDiff, &nCount );
        delete pBuffer;
        if ( nCount != nDiff )
            return ERRCODE_IO_CANTWRITE;
    }

    return ERRCODE_NONE;
}

//----------------------------------------------------------------------------
ErrCode UcbLockBytes::Stat( SvLockBytesStat *pStat, SvLockBytesStatFlag) const
{
    if ( IsSynchronMode() )
    {
        UcbLockBytes* pThis = const_cast < UcbLockBytes* >( this );
        if ( m_xHandler.Is() )
            m_xHandler->Handle( UcbLockBytesHandler::BEFOREWAIT, pThis );
        pThis->m_aInitialized.wait();
        if ( m_xHandler.Is() )
            m_xHandler->Handle( UcbLockBytesHandler::AFTERWAIT, pThis );
    }

    if (!pStat)
        return ERRCODE_IO_INVALIDPARAMETER;

    Reference <XInputStream> xStream = getInputStream_Impl();
    Reference <XSeekable> xSeekable = getSeekable_Impl();

    if ( !xStream.is() )
    {
        if ( m_bTerminated )
            return ERRCODE_IO_INVALIDACCESS;
        else
            return ERRCODE_IO_PENDING;
    }
    else if( !xSeekable.is() )
        return ERRCODE_IO_CANTTELL;

    try
    {
        pStat->nSize = ULONG(xSeekable->getLength());
    }
    catch (IOException)
    {
        return ERRCODE_IO_CANTTELL;
    }

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
    if ( hasInputStream_Impl() && m_xHandler.Is() )
        m_xHandler->Handle( UcbLockBytesHandler::DATA_AVAILABLE, this );

    return 0;
}

UcbLockBytesRef UcbLockBytes::CreateInputLockBytes( const Reference< XInputStream > xInputStream )
{
    if( !xInputStream.is() )
        return NULL;;

    UcbLockBytesRef xLockBytes = new UcbLockBytes();
    xLockBytes->setDontClose_Impl();
    xLockBytes->setInputStream_Impl( xInputStream );
    xLockBytes->terminate_Impl();
    return xLockBytes;
}

UcbLockBytesRef UcbLockBytes::CreateLockBytes( const Reference < XContent > xContent, StreamMode eOpenMode, UcbLockBytesHandler* pHandler )
{
    if( !xContent.is() )
        return NULL;;

    Reference< XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
    Reference< XInteractionHandler > xInteractionHandler = Reference< XInteractionHandler > (
                xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uui.InteractionHandler") ) ), UNO_QUERY );

    UcbLockBytesRef xLockBytes = new UcbLockBytes( pHandler );
    xLockBytes->SetSynchronMode( !pHandler );
    Reference< XActiveDataControl > xSink;
    if ( eOpenMode & STREAM_WRITE )
        xSink = (XActiveDataControl*) new UcbStreamer_Impl( xLockBytes );
    else
        xSink = (XActiveDataControl*) new UcbDataSink_Impl( xLockBytes );

    OpenCommandArgument2 aArgument;
    aArgument.Sink = xSink;
    aArgument.Mode = OpenMode::DOCUMENT;

    Reference< XProgressHandler > xProgressHdl = new ProgressHandler_Impl( LINK( &xLockBytes, UcbLockBytes, DataAvailHdl ) );
    CommandThread_Impl* pThread = new CommandThread_Impl( xLockBytes, xContent, aArgument, xInteractionHandler, xProgressHdl, pHandler );

    if ( ( eOpenMode & STREAM_WRITE ) || !pHandler )
    {
        // first try read/write mode
        sal_Bool bError = pThread->DoIt();
        if ( bError || !xLockBytes->getInputStream_Impl().is() )
            xLockBytes->SetError( ERRCODE_IO_NOTEXISTS );
        delete pThread;
        return xLockBytes;
    }
    else
    {
        pThread->create();
    }

    return xLockBytes;
}

};
