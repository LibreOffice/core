/*************************************************************************
 *
 *  $RCSfile: ucblockbytes.cxx,v $
 *
 *  $Revision: 1.45 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-25 11:35:45 $
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

#ifndef _COMPHELPER_CONDITION_HXX_
#include <comphelper/condition.hxx>
#endif
#ifndef _OSL_THREAD_HXX_
#include <osl/thread.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UCBHELPER_INTERATIONREQUEST_HXX
#include <ucbhelper/interactionrequest.hxx>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONABORT_HPP_
#include <com/sun/star/task/XInteractionAbort.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVENETWORKCONNECTEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveNetworkConnectException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDFAILEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandFailedException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_UNSUPPORTEDDATASINKEXCEPTION_HPP_
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEIODEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#endif
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
#ifndef _COM_SUN_STAR_UCB_POSTCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/PostCommandArgument2.hpp>
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
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
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



class Moderator
    : public osl::Thread
{
    // usage restriction:
    // It might be possible, that the call to the interactionhandler and/or
    // progresshandler is done asynchrounsly, while the 'execute' simply
    // returns. This would imply that these class must be refcounted !!!

public:

    Moderator(
        Reference < XContent >& xContent,
        Reference < XInteractionHandler >& xInteract,
        Reference < XProgressHandler >& xProgress,
        const Command& rArg
    )
        throw(
            ContentCreationException,
            RuntimeException
        );

    ~Moderator();


    enum ResultType {
        NORESULT,

        INTERACTIONREQUEST,    // reply expected

        PROGRESSPUSH,
        PROGRESSUPDATE,
        PROGRESSPOP,

        INPUTSTREAM,
        STREAM,

        RESULT,
        TIMEDOUT,
        COMMANDABORTED,
        COMMANDFAILED,
        INTERACTIVEIO,
        UNSUPPORTED,
        GENERAL
    };


    class ConditionRes
        : public comphelper::Condition
    {
    public:

        ConditionRes(osl::Mutex& aMutex,Moderator& aModerator)
            : comphelper::Condition(aMutex),
              m_aModerator(aModerator)
        {
        }

    protected:

        bool applies() const {
            return m_aModerator.m_aResultType != NORESULT;
        }

    private:

        Moderator& m_aModerator;
    };


    struct Result {
        ResultType        type;
        Any               result;
        sal_Int32         ioErrorCode;
    };


    Result getResult(const sal_uInt32 milliSec);


    enum ReplyType {
        NOREPLY,
        EXIT,
        RETRY,
        REQUESTHANDLED
    };


    class ConditionRep
        : public comphelper::Condition
    {
    public:

        ConditionRep(osl::Mutex& aMutex,Moderator& aModerator)
            : comphelper::Condition(aMutex),
              m_aModerator(aModerator)
        {
        }

    protected:

        bool applies() const {
            return m_aModerator.m_aReplyType != NOREPLY;
        }

    private:

        Moderator& m_aModerator;
    };

    void setReply(ReplyType);


    void handle( const Reference<XInteractionRequest >& Request );

    void push( const Any& Status );

    void update( const Any& Status );

    void pop(  );

    void setStream(const Reference< XStream >& aStream);

    void setInputStream(const Reference<XInputStream> &rxInputStream);


protected:

    virtual void SAL_CALL run();

    virtual void SAL_CALL onTerminated();

private:

    osl::Mutex        m_aMutex;

    friend class ConditionRes;

    ConditionRes      m_aRes;
    ResultType        m_aResultType;
    sal_Int32         m_nIOErrorCode;
    Any               m_aResult;

    friend class ConditionRep;

    ConditionRep      m_aRep;
    ReplyType         m_aReplyType;

    Command                           m_aArg;
    ::ucb::Content                    m_aContent;
};


class ModeratorsActiveDataStreamer
    : public ::cppu::WeakImplHelper1<XActiveDataStreamer>
{
public:

    ModeratorsActiveDataStreamer(Moderator &theModerator);

    ~ModeratorsActiveDataStreamer();

    // XActiveDataStreamer
    virtual void SAL_CALL
    setStream(
        const Reference< XStream >& aStream
    )
        throw(
            RuntimeException
        );

    virtual Reference<XStream> SAL_CALL
    getStream (
        void
    ) throw(
        RuntimeException
    )
    {
        osl::MutexGuard aGuard(m_aMutex);
        return m_xStream;
    }


private:

    Moderator& m_aModerator;

    osl::Mutex m_aMutex;
    Reference<XStream> m_xStream;
};



class ModeratorsActiveDataSink
    : public ::cppu::WeakImplHelper1<XActiveDataSink>
{
public:

    ModeratorsActiveDataSink(Moderator &theModerator);

    ~ModeratorsActiveDataSink();

    // XActiveDataSink.
    virtual void SAL_CALL
    setInputStream (
        const Reference<XInputStream> &rxInputStream
    )
        throw(
            RuntimeException
        );

    virtual Reference<XInputStream> SAL_CALL
    getInputStream (
        void
    ) throw(
        RuntimeException
    )
    {
        osl::MutexGuard aGuard(m_aMutex);
        return m_xStream;
    }


private:

    Moderator& m_aModerator;
    osl::Mutex m_aMutex;
    Reference<XInputStream> m_xStream;
};



ModeratorsActiveDataSink::ModeratorsActiveDataSink(Moderator &theModerator)
    : m_aModerator(theModerator)
{
}


ModeratorsActiveDataSink::~ModeratorsActiveDataSink()
{
}

// XActiveDataSink.
void SAL_CALL
ModeratorsActiveDataSink::setInputStream (
    const Reference<XInputStream> &rxInputStream
)
    throw(
        RuntimeException
    )
{
    m_aModerator.setInputStream(rxInputStream);
    osl::MutexGuard aGuard(m_aMutex);
    m_xStream = rxInputStream;
}


ModeratorsActiveDataStreamer::ModeratorsActiveDataStreamer(
    Moderator &theModerator
)
    : m_aModerator(theModerator)
{
}


ModeratorsActiveDataStreamer::~ModeratorsActiveDataStreamer()
{
}

// XActiveDataStreamer.
void SAL_CALL
ModeratorsActiveDataStreamer::setStream (
    const Reference<XStream> &rxStream
)
    throw(
        RuntimeException
    )
{
    m_aModerator.setStream(rxStream);
    osl::MutexGuard aGuard(m_aMutex);
    m_xStream = rxStream;
}



class ModeratorsInteractionHandler
    : public ::cppu::WeakImplHelper1<XInteractionHandler>
{
public:

    ModeratorsInteractionHandler(Moderator &theModerator);

    ~ModeratorsInteractionHandler();

    virtual void SAL_CALL
    handle( const Reference<XInteractionRequest >& Request )
        throw (RuntimeException);

private:

    Moderator& m_aModerator;
};


class ModeratorsProgressHandler
    : public ::cppu::WeakImplHelper1<XProgressHandler>
{
public:

    ModeratorsProgressHandler(Moderator &theModerator);

    ~ModeratorsProgressHandler();

    virtual void SAL_CALL push( const Any& Status )
        throw (
            RuntimeException);

    virtual void SAL_CALL update( const Any& Status )
        throw (RuntimeException);

    virtual void SAL_CALL pop(  )
        throw (RuntimeException);


private:

    Moderator& m_aModerator;
};


ModeratorsProgressHandler::ModeratorsProgressHandler(Moderator &theModerator)
    : m_aModerator(theModerator)
{
}

ModeratorsProgressHandler::~ModeratorsProgressHandler()
{
}


void SAL_CALL ModeratorsProgressHandler::push( const Any& Status )
    throw (
        RuntimeException)
{
    m_aModerator.push(Status);
}


void SAL_CALL ModeratorsProgressHandler::update( const Any& Status )
    throw (RuntimeException)
{
    m_aModerator.update(Status);
}


void SAL_CALL ModeratorsProgressHandler::pop(  )
    throw (RuntimeException)
{
    m_aModerator.pop();
}




ModeratorsInteractionHandler::ModeratorsInteractionHandler(
    Moderator &aModerator)
    : m_aModerator(aModerator)
{
}


ModeratorsInteractionHandler::~ModeratorsInteractionHandler()
{
}


void SAL_CALL
ModeratorsInteractionHandler::handle(
    const Reference<XInteractionRequest >& Request
)
    throw (
        RuntimeException
    )
{
    // wakes up the mainthread
    m_aModerator.handle(Request);
}




Moderator::Moderator(
    Reference < XContent >& xContent,
    Reference < XInteractionHandler >& xInteract,
    Reference < XProgressHandler >& xProgress,
    const Command& rArg
)
    throw(
        ::com::sun::star::ucb::ContentCreationException,
        ::com::sun::star::uno::RuntimeException
    )
    : m_aMutex(),

      m_aRes(m_aMutex,*this),
      m_aResultType(NORESULT),
      m_aResult(),
      m_nIOErrorCode(0),

      m_aRep(m_aMutex,*this),
      m_aReplyType(NOREPLY),

      m_aArg(rArg),
      m_aContent(
          xContent,
          new UcbTaskEnvironment(
              xInteract.is() ? new ModeratorsInteractionHandler(*this) : 0,
              xProgress.is() ? new ModeratorsProgressHandler(*this) : 0
          ))
{
    // now exchange the whole data sink stuff
    // with a thread safe version

    Reference<XInterface> *pxSink;

    PostCommandArgument2 aPostArg;
    OpenCommandArgument2 aOpenArg;

    int dec(2);
    if(m_aArg.Argument >>= aPostArg) {
        pxSink = &aPostArg.Sink;
        dec = 0;
    }
    else if(m_aArg.Argument >>= aOpenArg) {
        pxSink = &aOpenArg.Sink;
        dec = 1;
    }

    if(dec ==2)
        throw ContentCreationException();

    Reference < XActiveDataSink > xActiveSink(*pxSink,UNO_QUERY);
    if(xActiveSink.is())
        *pxSink = Reference<XInterface>(
            (cppu::OWeakObject*)new ModeratorsActiveDataSink(*this));

    Reference<XActiveDataStreamer> xStreamer( *pxSink, UNO_QUERY );
    if ( xStreamer.is() )
        *pxSink = Reference<XInterface>(
            (cppu::OWeakObject*)new ModeratorsActiveDataStreamer(*this));

    if(dec == 0)
        m_aArg.Argument <<= aPostArg;
    else if(dec == 1)
        m_aArg.Argument <<= aOpenArg;
}


Moderator::~Moderator()
{
}


Moderator::Result Moderator::getResult(const sal_uInt32 milliSec)
{
    Result ret;
    try {
        comphelper::ConditionWaiter aWaiter(m_aRes,milliSec);
        ret.type = m_aResultType;
        ret.result = m_aResult;
        ret.ioErrorCode = m_nIOErrorCode;

        // reset
        m_aResultType = NORESULT;
    }
    catch(const comphelper::ConditionWaiter::timedout&)
    {
        ret.type = TIMEDOUT;
    }

    return ret;
}


void Moderator::setReply(ReplyType aReplyType )
{
    comphelper::ConditionModifier aMod(m_aRep);
    m_aReplyType = aReplyType;
}


void Moderator::handle( const Reference<XInteractionRequest >& Request )
{
    ReplyType aReplyType;

    do {
        {
            comphelper::ConditionModifier aMod(m_aRes);
            m_aResultType = INTERACTIONREQUEST;
            m_aResult <<= Request;
        }

        {
            comphelper::ConditionWaiter aWait(m_aRep);
            aReplyType = m_aReplyType;

            // reset
            m_aReplyType = NOREPLY;
        }

        if(aReplyType == EXIT) {
            Sequence<Reference<XInteractionContinuation> > aSeq(
                Request->getContinuations());
            for(sal_Int32 i = 0; i < aSeq.getLength(); ++i) {
                Reference<XInteractionAbort> aRef(aSeq[i],UNO_QUERY);
                if(aRef.is()) {
                    aRef->select();
                }
            }

            // resignal the exitcondition
            setReply(EXIT);
            break;
        }
    } while(aReplyType != REQUESTHANDLED);
}



void Moderator::push( const Any& Status )
{
    {
        comphelper::ConditionModifier aMod(m_aRes);
        m_aResultType = PROGRESSPUSH;
        m_aResult = Status;
    }
    ReplyType aReplyType;
    {
        comphelper::ConditionWaiter aWait(m_aRep);
        aReplyType = m_aReplyType;
        m_aReplyType = NOREPLY;
    }
    if(aReplyType == EXIT)
        setReply(EXIT);
}


void Moderator::update( const Any& Status )
{
    {
        comphelper::ConditionModifier aMod(m_aRes);
        m_aResultType = PROGRESSUPDATE;
        m_aResult = Status;
    }
    ReplyType aReplyType;
    {
        comphelper::ConditionWaiter aWait(m_aRep);
        aReplyType = m_aReplyType;
        m_aReplyType = NOREPLY;
    }
    if(aReplyType == EXIT)
        setReply(EXIT);
}


void Moderator::pop(  )
{
    {
        comphelper::ConditionModifier aMod(m_aRes);
        m_aResultType = PROGRESSPOP;
    }
    ReplyType aReplyType;
    {
        comphelper::ConditionWaiter aWait(m_aRep);
        aReplyType = m_aReplyType;
        m_aReplyType = NOREPLY;
    }
    if(aReplyType == EXIT)
        setReply(EXIT);
}


void Moderator::setStream(const Reference< XStream >& aStream)
{
    {
        comphelper::ConditionModifier aMod(m_aRes);
        m_aResultType = STREAM;
        m_aResult <<= aStream;
    }
    ReplyType aReplyType;
    {
        comphelper::ConditionWaiter aWait(m_aRep);
        aReplyType = m_aReplyType;
        m_aReplyType = NOREPLY;
    }
    if(aReplyType == EXIT)
        setReply(EXIT);
}


void Moderator::setInputStream(const Reference<XInputStream> &rxInputStream)
{
    {
        comphelper::ConditionModifier aMod(m_aRes);
        m_aResultType = INPUTSTREAM;
        m_aResult <<= rxInputStream;
    }
    ReplyType aReplyType;
    {
        comphelper::ConditionWaiter aWait(m_aRep);
        m_aReplyType = NOREPLY;
        m_aReplyType = NOREPLY;
    }
    if(aReplyType == EXIT)
        setReply(EXIT);
}



void SAL_CALL Moderator::run()
{
    ResultType aResultType;
    Any        aResult;
    sal_Int32  nIOErrorCode;

    try
    {
        aResult = m_aContent.executeCommand(m_aArg.Name,m_aArg.Argument);
        aResultType = RESULT;
    }
    catch ( CommandAbortedException )
    {
        aResultType = COMMANDABORTED;
    }
    catch ( CommandFailedException )
    {
        aResultType = COMMANDFAILED;
    }
    catch ( InteractiveIOException& r )
    {
        nIOErrorCode = r.Code;
        aResultType = INTERACTIVEIO;
    }
    catch ( UnsupportedDataSinkException& )
    {
        aResultType = UNSUPPORTED;
    }
    catch ( Exception )
    {
        aResultType = GENERAL;
    }

    {
        comphelper::ConditionModifier aMod(m_aRes);
        m_aResultType = aResultType;
        m_aResult = aResult;
        m_nIOErrorCode = nIOErrorCode;
    }
}



void SAL_CALL Moderator::onTerminated()
{
    {
        comphelper::ConditionWaiter aWaiter(m_aRep);
    }
     delete this;
}


/**
   Function for opening UCB contents synchronously,
   but with handled timeout;
*/

static sal_Bool _UCBOpenContentSync(
    UcbLockBytesRef xLockBytes,
    Reference < XContent > xContent,
    const Command& rArg,
    Reference < XInterface > xSink,
    Reference < XInteractionHandler > xInteract,
    Reference < XProgressHandler > xProgress,
    UcbLockBytesHandlerRef xHandler );


static sal_Bool UCBOpenContentSync(
    UcbLockBytesRef xLockBytes,
    Reference < XContent > xContent,
    const Command& rArg,
    Reference < XInterface > xSink,
    Reference < XInteractionHandler > xInteract,
    Reference < XProgressHandler > xProgress,
    UcbLockBytesHandlerRef xHandler )
{
    // http protocol must be handled in a special way:
    //        during the opening process the input stream may change
    //        only the last inputstream after notifying the document
    //        headers is valid

    Reference<XContentIdentifier> xContId(
        xContent.is() ? xContent->getIdentifier() : 0 );

    rtl::OUString aScheme;
    if(xContId.is())
        aScheme = xContId->getContentProviderScheme();

    // now determine wether we use a timeout or not;
    if( ! aScheme.equalsIgnoreAsciiCaseAscii("http")                &&
        ! aScheme.equalsIgnoreAsciiCaseAscii("vnd.sun.star.webdav") &&
        ! aScheme.equalsIgnoreAsciiCaseAscii("ftp"))
        return _UCBOpenContentSync(
            xLockBytes,xContent,rArg,xSink,xInteract,xProgress,xHandler);

    if (aScheme.compareToAscii(
            "http") != COMPARE_EQUAL )
        xLockBytes->SetStreamValid_Impl();

    Reference< XPropertiesChangeListener > xListener;
    Reference< XPropertiesChangeNotifier > xProps(xContent,UNO_QUERY);
    if(xProps.is()) {
        xListener =
            new UcbPropertiesChangeListener_Impl(xLockBytes);
        xProps->addPropertiesChangeListener(
            Sequence< ::rtl::OUString >(),
            xListener);
    }

    Any aResult;
    bool bException(false);
    bool bAborted(false);
    bool bResultAchieved(false);

    Moderator* pMod = 0;
    try {
        pMod = new Moderator(xContent,xInteract,xProgress,rArg);
        pMod->create();
    } catch(const ContentCreationException&) {
        bResultAchieved = bException = true;
        xLockBytes->SetError( ERRCODE_IO_GENERAL );
    }

    sal_uInt32 nTimeout(5000); // initially 5000 milliSec
    while(!bResultAchieved) {

        Moderator::Result res;
        // try to get the result for with timeout
        res = pMod->getResult(nTimeout);

        switch(res.type) {
        case Moderator::PROGRESSPUSH:
            {
                if(xProgress.is())
                    xProgress->push(res.result);
                pMod->setReply(Moderator::REQUESTHANDLED);
                break;
            }
        case Moderator::PROGRESSUPDATE:
            {
                if(xProgress.is())
                    xProgress->update(res.result);
                pMod->setReply(Moderator::REQUESTHANDLED);
                break;
            }
        case Moderator::PROGRESSPOP:
            {
                if(xProgress.is())
                    xProgress->pop();
                pMod->setReply(Moderator::REQUESTHANDLED);
                break;
            }
        case Moderator::STREAM:
            {
                Reference<XStream> result;
                if(res.result >>= result) {
                    Reference < XActiveDataStreamer > xStreamer(
                        xSink, UNO_QUERY
                    );

                    if(xStreamer.is())
                        xStreamer->setStream(result);
                }
                pMod->setReply(Moderator::REQUESTHANDLED);
                break;
            }
        case Moderator::INPUTSTREAM:
            {
                Reference<XInputStream> result;
                res.result >>= result;
                Reference < XActiveDataSink > xActiveSink(
                    xSink, UNO_QUERY
                );

                if(xActiveSink.is())
                    xActiveSink->setInputStream(result);
                pMod->setReply(Moderator::REQUESTHANDLED);
                break;
            }
        case Moderator::TIMEDOUT:
            {
                Reference<XInteractionRetry> xRet;
                if(xInteract.is()) {
                    InteractiveNetworkConnectException aExcep;
                    INetURLObject aURL(
                        xContId.is() ?
                        xContId->getContentIdentifier() :
                        rtl::OUString() );
                    aExcep.Server = aURL.GetHost();
                    aExcep.Classification = InteractionClassification_ERROR;
                    aExcep.Message =
                        rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "server not responding after five seconds"));
                    Any request;
                    request <<= aExcep;
                    ucbhelper::InteractionRequest *ir =
                        new ucbhelper::InteractionRequest(request);
                    Reference<XInteractionRequest> xIR(ir);
                    Sequence<Reference<XInteractionContinuation> > aSeq(2);
                    ucbhelper::InteractionRetry *retryP =
                        new ucbhelper::InteractionRetry(ir);
                    aSeq[0] = retryP;
                    ucbhelper::InteractionAbort *abortP =
                        new ucbhelper::InteractionAbort(ir);
                    aSeq[1] = abortP;

                    ir->setContinuations(aSeq);
                    xInteract->handle(xIR);
                    rtl::Reference< ucbhelper::InteractionContinuation > ref
                        = ir->getSelection();
                    if(ref.is()) {
                        Reference<XInterface> xInt(ref.get());
                        xRet = Reference<XInteractionRetry>(xInt,UNO_QUERY);
                    }
                }

                if(!xRet.is()) {
                    bAborted = true;
                    xLockBytes->SetError(ERRCODE_ABORT);
                }

                break;
            }
        case Moderator::INTERACTIONREQUEST:
            {
                Reference<XInteractionRequest> Request;
                res.result >>= Request;
                xInteract->handle(Request);
                pMod->setReply(Moderator::REQUESTHANDLED);
                break;
            }
        case Moderator::RESULT:
            {
                bResultAchieved = true;
                aResult = res.result;
                break;
            }
        case Moderator::COMMANDABORTED:
            {
                bAborted = true;
                xLockBytes->SetError( ERRCODE_ABORT );
                break;
            }
        case Moderator::COMMANDFAILED:
            {
                bAborted = true;
                xLockBytes->SetError( ERRCODE_ABORT );
                break;
            }
        case Moderator::INTERACTIVEIO:
            {
                bException = true;
                if ( res.ioErrorCode == IOErrorCode_ACCESS_DENIED ||
                     res.ioErrorCode == IOErrorCode_LOCKING_VIOLATION )
                    xLockBytes->SetError( ERRCODE_IO_ACCESSDENIED );
                else if ( res.ioErrorCode == IOErrorCode_NOT_EXISTING )
                    xLockBytes->SetError( ERRCODE_IO_NOTEXISTS );
                else if ( res.ioErrorCode == IOErrorCode_CANT_READ )
                    xLockBytes->SetError( ERRCODE_IO_CANTREAD );
                else
                    xLockBytes->SetError( ERRCODE_IO_GENERAL );
                break;
            }
        case Moderator::UNSUPPORTED:
            {
                bException = true;
                xLockBytes->SetError( ERRCODE_IO_NOTSUPPORTED );
                break;
            }
        default:
            {
                bException = true;
                xLockBytes->SetError( ERRCODE_IO_GENERAL );
                break;
            }
        }

        bResultAchieved |= bException;
        bResultAchieved |= bAborted;
        if(nTimeout == 5000) nTimeout *= 2;
    }

    if(pMod) pMod->setReply(Moderator::EXIT);

    if ( bAborted || bException )
    {
        if( xHandler.Is() )
            xHandler->Handle( UcbLockBytesHandler::CANCEL, xLockBytes );

        Reference < XActiveDataSink > xActiveSink( xSink, UNO_QUERY );
        if ( xActiveSink.is() )
            xActiveSink->setInputStream( Reference < XInputStream >() );

        Reference < XActiveDataStreamer > xStreamer( xSink, UNO_QUERY );
        if ( xStreamer.is() )
            xStreamer->setStream( Reference < XStream >() );
    }

    Reference < XActiveDataControl > xControl( xSink, UNO_QUERY );
    if ( xControl.is() )
        xControl->terminate();

    if ( xProps.is() )
        xProps->removePropertiesChangeListener(
            Sequence< ::rtl::OUString >(),
            xListener );

    return ( bAborted || bException );
}

/**
    Function for opening UCB contents synchronously
 */
static sal_Bool _UCBOpenContentSync(
    UcbLockBytesRef xLockBytes,
    Reference < XContent > xContent,
    const Command& rArg,
    Reference < XInterface > xSink,
    Reference < XInteractionHandler > xInteract,
    Reference < XProgressHandler > xProgress,
    UcbLockBytesHandlerRef xHandler )
{
    ::ucb::Content aContent( xContent, new UcbTaskEnvironment( xInteract, xProgress ) );
    Reference < XContentIdentifier > xIdent = xContent->getIdentifier();
    ::rtl::OUString aScheme = xIdent->getContentProviderScheme();

    // http protocol must be handled in a special way: during the opening process the input stream may change
    // only the last inputstream after notifying the document headers is valid
    if ( aScheme.compareToAscii("http") != COMPARE_EQUAL )
        xLockBytes->SetStreamValid_Impl();

    Reference< XPropertiesChangeListener > xListener = new UcbPropertiesChangeListener_Impl( xLockBytes );
    Reference< XPropertiesChangeNotifier > xProps ( xContent, UNO_QUERY );
    if ( xProps.is() )
        xProps->addPropertiesChangeListener( Sequence< ::rtl::OUString >(), xListener );

    Any aResult;
    bool bException = false;
    bool bAborted = false;

    try
    {
        aResult = aContent.executeCommand( rArg.Name, rArg.Argument );
    }
    catch ( CommandAbortedException )
    {
        bAborted = true;
        xLockBytes->SetError( ERRCODE_ABORT );
    }
    catch ( CommandFailedException )
    {
        bAborted = true;
        xLockBytes->SetError( ERRCODE_ABORT );
    }
    catch ( InteractiveIOException& r )
    {
        bException = true;
        if ( r.Code == IOErrorCode_ACCESS_DENIED || r.Code == IOErrorCode_LOCKING_VIOLATION )
            xLockBytes->SetError( ERRCODE_IO_ACCESSDENIED );
        else if ( r.Code == IOErrorCode_NOT_EXISTING )
            xLockBytes->SetError( ERRCODE_IO_NOTEXISTS );
        else if ( r.Code == IOErrorCode_CANT_READ )
            xLockBytes->SetError( ERRCODE_IO_CANTREAD );
        else
            xLockBytes->SetError( ERRCODE_IO_GENERAL );
    }
    catch ( UnsupportedDataSinkException& )
    {
        bException = true;
        xLockBytes->SetError( ERRCODE_IO_NOTSUPPORTED );
    }
    catch ( Exception )
    {
        bException = true;
        xLockBytes->SetError( ERRCODE_IO_GENERAL );
    }

    if ( bAborted || bException )
    {
        if( xHandler.Is() )
            xHandler->Handle( UcbLockBytesHandler::CANCEL, xLockBytes );

        Reference < XActiveDataSink > xActiveSink( xSink, UNO_QUERY );
        if ( xActiveSink.is() )
            xActiveSink->setInputStream( Reference < XInputStream >() );

        Reference < XActiveDataStreamer > xStreamer( xSink, UNO_QUERY );
        if ( xStreamer.is() )
            xStreamer->setStream( Reference < XStream >() );
    }

    Reference < XActiveDataControl > xControl( xSink, UNO_QUERY );
    if ( xControl.is() )
        xControl->terminate();


    if ( xProps.is() )
        xProps->removePropertiesChangeListener( Sequence< ::rtl::OUString >(), xListener );

    return ( bAborted || bException );
}


//----------------------------------------------------------------------------
static void copyInputToOutput( const Reference< XInputStream >& aIn, const Reference< XOutputStream >& aOut )
{
    const sal_Int32 nConstBufferSize = 32000;

    sal_Int32 nRead;
    Sequence < sal_Int8 > aSequence ( nConstBufferSize );

    do
    {
        nRead = aIn->readBytes ( aSequence, nConstBufferSize );
        if ( nRead < nConstBufferSize )
        {
            Sequence < sal_Int8 > aTempBuf ( aSequence.getConstArray(), nRead );
            aOut->writeBytes ( aTempBuf );
        }
        else
            aOut->writeBytes ( aSequence );
    }
    while ( nRead == nConstBufferSize );
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
    }

    if ( !m_xInputStream.is() && m_xOutputStream.is() )
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

Reference < XInputStream > UcbLockBytes::getInputStream()
{
    vos::OClearableGuard aGuard( m_aMutex );
    m_bDontClose = sal_True;
    return m_xInputStream;
}

//----------------------------------------------------------------------------

sal_Bool UcbLockBytes::setStream_Impl( const Reference<XStream>& aStream )
{
    vos::OClearableGuard aGuard( m_aMutex );
    if ( aStream.is() )
    {
        m_xOutputStream = aStream->getOutputStream();
        setInputStream_Impl( aStream->getInputStream(), sal_False );
        m_xSeekable = Reference < XSeekable > ( aStream, UNO_QUERY );
    }
    else
    {
        m_xOutputStream = Reference < XOutputStream >();
        setInputStream_Impl( Reference < XInputStream >() );
    }

    return m_xInputStream.is();
}

sal_Bool UcbLockBytes::setInputStream_Impl( const Reference<XInputStream> &rxInputStream, sal_Bool bSetXSeekable )
{
    BOOL bRet;

    vos::OClearableGuard aGuard( m_aMutex );
    if ( !m_bDontClose && m_xInputStream.is() )
        m_xInputStream->closeInput();

    m_xInputStream = rxInputStream;

    if( bSetXSeekable )
    {
        m_xSeekable = Reference < XSeekable > ( rxInputStream, UNO_QUERY );
        if( !m_xSeekable.is() && rxInputStream.is() )
        {
            Reference < XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
            Reference< XOutputStream > rxTempOut = Reference < XOutputStream > (
                                xFactory->createInstance ( ::rtl::OUString::createFromAscii( "com.sun.star.io.TempFile" ) ),
                                UNO_QUERY );

            if( rxTempOut.is() )
            {
                copyInputToOutput( rxInputStream, rxTempOut );
                m_xInputStream = Reference< XInputStream >( rxTempOut, UNO_QUERY );
                m_xSeekable = Reference < XSeekable > ( rxTempOut, UNO_QUERY );
            }
        }
    }

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
    m_bTerminated = sal_True;
    m_aInitialized.set();
    m_aTerminated.set();

    if ( GetError() == ERRCODE_NONE && !m_xInputStream.is() )
    {
        DBG_ERROR("No InputStream, but no error set!" );
        SetError( ERRCODE_IO_NOTEXISTS );
    }

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
        pThis->m_aInitialized.wait();
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
    catch (com::sun::star::lang::IllegalArgumentException)
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
        delete[] pBuffer;
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
        pThis->m_aInitialized.wait();
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
    // is alive only for compatibility reasons
    OSL_ENSURE( m_bTerminated, "UcbLockBytes is not thread safe so it can be used only syncronously!\n" );
}

//----------------------------------------------------------------------------
IMPL_LINK( UcbLockBytes, DataAvailHdl, void*, EMPTYARG )
{
    if ( hasInputStream_Impl() && m_xHandler.Is() )
        m_xHandler->Handle( UcbLockBytesHandler::DATA_AVAILABLE, this );

    return 0;
}

UcbLockBytesRef UcbLockBytes::CreateInputLockBytes( const Reference< XInputStream >& xInputStream )
{
    if( !xInputStream.is() )
        return NULL;;

    UcbLockBytesRef xLockBytes = new UcbLockBytes();
    xLockBytes->setDontClose_Impl();
    xLockBytes->setInputStream_Impl( xInputStream );
    xLockBytes->terminate_Impl();
    return xLockBytes;
}

#if SUPD<640
UcbLockBytesRef UcbLockBytes::CreateLockBytes( const Reference < XContent >& xContent, const Sequence < PropertyValue >& rProps,
        const Reference < XInputStream >& xPostData, const Reference < XInteractionHandler >& xInteractionHandler, UcbLockBytesHandler* pHandler )
#else
UcbLockBytesRef UcbLockBytes::CreateLockBytes( const Reference < XContent >& xContent, const ::rtl::OUString& rReferer, const ::rtl::OUString& rMediaType,
        const Reference < XInputStream >& xPostData, const Reference < XInteractionHandler >& xInteractionHandler, UcbLockBytesHandler* pHandler )
#endif
{
    if( !xContent.is() )
        return NULL;;

    UcbLockBytesRef xLockBytes = new UcbLockBytes( pHandler );
    xLockBytes->SetSynchronMode( !pHandler );
    Reference< XActiveDataControl > xSink = (XActiveDataControl*) new UcbDataSink_Impl( xLockBytes );

#if SUPD<640
    if ( rProps.getLength() )
    {
        Reference < XCommandProcessor > xProcessor( xContent, UNO_QUERY );
        Command aCommand;
        aCommand.Name     = ::rtl::OUString::createFromAscii("setPropertyValues");
        aCommand.Handle   = -1; /* unknown */
        aCommand.Argument <<= rProps;
        xProcessor->execute( aCommand, 0, Reference < XCommandEnvironment >() );
    }

    PostCommandArgument aArgument;
    aArgument.Source = xPostData;
    aArgument.Sink = xSink;

#else
    PostCommandArgument2 aArgument;
    aArgument.Source = xPostData;
    aArgument.Sink = xSink;
    aArgument.MediaType = rMediaType;
    aArgument.Referer = rReferer;
#endif

    Command aCommand;
    aCommand.Name = ::rtl::OUString::createFromAscii ("post");
    aCommand.Argument <<= aArgument;

    Reference< XProgressHandler > xProgressHdl = new ProgressHandler_Impl( LINK( &xLockBytes, UcbLockBytes, DataAvailHdl ) );

    sal_Bool bError = UCBOpenContentSync( xLockBytes,
                                          xContent,
                                          aCommand,
                                          xSink,
                                          xInteractionHandler,
                                          xProgressHdl,
                                          pHandler );

       if ( xLockBytes->GetError() == ERRCODE_NONE && ( bError || !xLockBytes->getInputStream().is() ) )
    {
        DBG_ERROR("No InputStream, but no error set!" );
           xLockBytes->SetError( ERRCODE_IO_GENERAL );
    }

    return xLockBytes;
}

UcbLockBytesRef UcbLockBytes::CreateLockBytes( const Reference < XContent >& xContent, const Sequence < PropertyValue >& rProps,
        StreamMode eOpenMode, const Reference < XInteractionHandler >& xInteractionHandler, UcbLockBytesHandler* pHandler )
{
    if( !xContent.is() )
        return NULL;;

    UcbLockBytesRef xLockBytes = new UcbLockBytes( pHandler );
    xLockBytes->SetSynchronMode( !pHandler );
    Reference< XActiveDataControl > xSink;
    if ( eOpenMode & STREAM_WRITE )
        xSink = (XActiveDataControl*) new UcbStreamer_Impl( xLockBytes );
    else
        xSink = (XActiveDataControl*) new UcbDataSink_Impl( xLockBytes );

    if ( rProps.getLength() )
    {
        Reference < XCommandProcessor > xProcessor( xContent, UNO_QUERY );
        Command aCommand;
        aCommand.Name     = ::rtl::OUString::createFromAscii("setPropertyValues");
        aCommand.Handle   = -1; /* unknown */
        aCommand.Argument <<= rProps;
        xProcessor->execute( aCommand, 0, Reference < XCommandEnvironment >() );
    }

    OpenCommandArgument2 aArgument;
    aArgument.Sink = xSink;
    aArgument.Mode = OpenMode::DOCUMENT;

    Command aCommand;
    aCommand.Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("open") );
    aCommand.Argument <<= aArgument;

    Reference< XProgressHandler > xProgressHdl = new ProgressHandler_Impl( LINK( &xLockBytes, UcbLockBytes, DataAvailHdl ) );

    sal_Bool bError = UCBOpenContentSync( xLockBytes,
                                          xContent,
                                          aCommand,
                                          xSink,
                                          xInteractionHandler,
                                          xProgressHdl,
                                          pHandler );

    if ( xLockBytes->GetError() == ERRCODE_NONE && ( bError || !xLockBytes->getInputStream().is() ) )
    {
        DBG_ERROR("No InputStream, but no error set!" );
           xLockBytes->SetError( ERRCODE_IO_GENERAL );
    }

    return xLockBytes;
}

};
