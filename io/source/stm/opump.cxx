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


#include <sal/log.hxx>

#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/io/XConnectable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/thread.h>
#include <mutex>

using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;

namespace io_stm {

    namespace {

    class Pump : public WeakImplHelper<
          XActiveDataSource, XActiveDataSink, XActiveDataControl, XConnectable, XServiceInfo >
    {
        std::mutex                              m_aMutex;
        oslThread                               m_aThread;

        Reference< XConnectable >               m_xPred;
        Reference< XConnectable >               m_xSucc;
        Reference< XInputStream >               m_xInput;
        Reference< XOutputStream >              m_xOutput;
        comphelper::OInterfaceContainerHelper4<XStreamListener>  m_cnt;
        bool                                m_closeFired;

        void run();
        static void static_run( void* pObject );

        void close();
        void fireClose();
        void fireStarted();
        void fireTerminated();
        void fireError( const Any &a );

    public:
        Pump();
        virtual ~Pump() override;

        // XActiveDataSource
        virtual void SAL_CALL setOutputStream( const Reference< css::io::XOutputStream >& xOutput ) override;
        virtual Reference< css::io::XOutputStream > SAL_CALL getOutputStream() override;

        // XActiveDataSink
        virtual void SAL_CALL setInputStream( const Reference< css::io::XInputStream >& xStream ) override;
        virtual Reference< css::io::XInputStream > SAL_CALL getInputStream() override;

        // XActiveDataControl
        virtual void SAL_CALL addListener( const Reference< css::io::XStreamListener >& xListener ) override;
        virtual void SAL_CALL removeListener( const Reference< css::io::XStreamListener >& xListener ) override;
        virtual void SAL_CALL start() override;
        virtual void SAL_CALL terminate() override;

        // XConnectable
        virtual void SAL_CALL setPredecessor( const Reference< css::io::XConnectable >& xPred ) override;
        virtual Reference< css::io::XConnectable > SAL_CALL getPredecessor() override;
        virtual void SAL_CALL setSuccessor( const Reference< css::io::XConnectable >& xSucc ) override;
        virtual Reference< css::io::XConnectable > SAL_CALL getSuccessor() override;

    public: // XServiceInfo
        virtual OUString    SAL_CALL getImplementationName() override;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
        virtual sal_Bool     SAL_CALL supportsService(const OUString& ServiceName) override;
    };

    }

Pump::Pump() : m_aThread( nullptr ),
               m_closeFired( false )
{
}

Pump::~Pump()
{
    // exit gracefully
    if( m_aThread )
    {
        osl_joinWithThread( m_aThread );
        osl_destroyThread( m_aThread );
    }
}

void Pump::fireError( const  Any & exception )
{
    std::unique_lock guard( m_aMutex );
    comphelper::OInterfaceIteratorHelper4<XStreamListener> iter( guard, m_cnt );
    guard.unlock();
    while( iter.hasMoreElements() )
    {
        try
        {
            iter.next()->error( exception );
        }
        catch ( const RuntimeException &e )
        {
            SAL_WARN("io.streams","com.sun.star.comp.stoc.Pump: unexpected exception during calling listeners" << e);
        }
    }
}

void Pump::fireClose()
{
    bool bFire = false;
    {
        std::unique_lock guard( m_aMutex );
        if( ! m_closeFired  )
        {
            m_closeFired = true;
            bFire = true;
        }
    }

    if( !bFire )
        return;

    std::unique_lock guard( m_aMutex );
    comphelper::OInterfaceIteratorHelper4<XStreamListener> iter( guard, m_cnt );
    guard.unlock();
    while( iter.hasMoreElements() )
    {
        try
        {
            iter.next()->closed( );
        }
        catch ( const RuntimeException &e )
        {
            SAL_WARN("io.streams","com.sun.star.comp.stoc.Pump: unexpected exception during calling listeners" << e);
        }
    }
}

void Pump::fireStarted()
{
    std::unique_lock guard( m_aMutex );
    comphelper::OInterfaceIteratorHelper4<XStreamListener> iter( guard, m_cnt );
    guard.unlock();
    while( iter.hasMoreElements() )
    {
        try
        {
            iter.next()->started( );
        }
        catch ( const RuntimeException &e )
        {
            SAL_WARN("io.streams","com.sun.star.comp.stoc.Pump: unexpected exception during calling listeners" << e);
        }
    }
}

void Pump::fireTerminated()
{
    std::unique_lock guard( m_aMutex );
    comphelper::OInterfaceIteratorHelper4<XStreamListener> iter( guard, m_cnt );
    guard.unlock();
    while( iter.hasMoreElements() )
    {
        try
        {
            iter.next()->terminated();
        }
        catch ( const RuntimeException &e )
        {
            SAL_WARN("io.streams","com.sun.star.comp.stoc.Pump: unexpected exception during calling listeners" << e);
        }
    }
}


void Pump::close()
{
    // close streams and release references
    Reference< XInputStream > rInput;
    Reference< XOutputStream > rOutput;
    {
        std::unique_lock guard( m_aMutex );
        rInput = m_xInput;
        m_xInput.clear();

        rOutput = m_xOutput;
        m_xOutput.clear();
        m_xSucc.clear();
        m_xPred.clear();
    }
    if( rInput.is() )
    {
        try
        {
            rInput->closeInput();
        }
        catch( Exception & )
        {
            // go down calm
        }
    }
    if( rOutput.is() )
    {
        try
        {
            rOutput->closeOutput();
        }
        catch( Exception & )
        {
            // go down calm
        }
    }
}

void Pump::static_run( void* pObject )
{
    osl_setThreadName("io_stm::Pump::run()");
    static_cast<Pump*>(pObject)->run();
    static_cast<Pump*>(pObject)->release();
}

void Pump::run()
{
    try
    {
        fireStarted();
        try
        {
            Reference< XInputStream > rInput;
            Reference< XOutputStream > rOutput;
            {
                std::unique_lock aGuard( m_aMutex );
                rInput = m_xInput;
                rOutput = m_xOutput;
            }

            if( ! rInput.is() )
            {
                throw NotConnectedException( "no input stream set", getXWeak() );
            }
            Sequence< sal_Int8 > aData;
            while( rInput->readSomeBytes( aData, 65536 ) )
            {
                if( ! rOutput.is() )
                {
                    throw NotConnectedException( "no output stream set", getXWeak() );
                }
                rOutput->writeBytes( aData );
                osl_yieldThread();
            }
        }
        catch ( const IOException & e )
        {
            fireError( Any( e ) );
        }
        catch ( const RuntimeException & e )
        {
            fireError( Any( e ) );
        }
        catch ( const Exception & e )
        {
            fireError( Any( e ) );
        }

        close();
        fireClose();
    }
    catch ( const css::uno::Exception &e )
    {
        // we are the last on the stack.
        // this is to avoid crashing the program, when e.g. a bridge crashes
        SAL_WARN("io.streams","com.sun.star.comp.stoc.Pump: unexpected exception during calling listeners" << e);
    }
}


/*
 * XConnectable
 */

void Pump::setPredecessor( const Reference< XConnectable >& xPred )
{
    std::unique_lock aGuard( m_aMutex );
    m_xPred = xPred;
}


Reference< XConnectable > Pump::getPredecessor()
{
    std::unique_lock aGuard( m_aMutex );
    return m_xPred;
}


void Pump::setSuccessor( const Reference< XConnectable >& xSucc )
{
    std::unique_lock aGuard( m_aMutex );
    m_xSucc = xSucc;
}


Reference< XConnectable > Pump::getSuccessor()
{
    std::unique_lock aGuard( m_aMutex );
    return m_xSucc;
}


/*
 * XActiveDataControl
 */

void Pump::addListener( const Reference< XStreamListener >& xListener )
{
    std::unique_lock aGuard( m_aMutex );
    m_cnt.addInterface( aGuard, xListener );
}


void Pump::removeListener( const Reference< XStreamListener >& xListener )
{
    std::unique_lock aGuard( m_aMutex );
    m_cnt.removeInterface( aGuard, xListener );
}


void Pump::start()
{
    std::unique_lock aGuard( m_aMutex );
    m_aThread = osl_createSuspendedThread(Pump::static_run,this);
    if( !m_aThread )
    {
        throw RuntimeException(
            "Pump::start Couldn't create worker thread",
            *this);
    }

    // will be released by OPump::static_run
    acquire();
    osl_resumeThread( m_aThread );

}


void Pump::terminate()
{
    close();

    // wait for the worker to die
    if( m_aThread )
        osl_joinWithThread( m_aThread );

    fireTerminated();
    fireClose();
}


/*
 * XActiveDataSink
 */

void Pump::setInputStream( const Reference< XInputStream >& xStream )
{
    std::unique_lock aGuard( m_aMutex );
    m_xInput = xStream;
    Reference< XConnectable > xConnect( xStream, UNO_QUERY );
    if( xConnect.is() )
        xConnect->setSuccessor( this );
    // data transfer starts in XActiveDataControl::start
}


Reference< XInputStream > Pump::getInputStream()
{
    std::unique_lock aGuard( m_aMutex );
    return m_xInput;
}


/*
 * XActiveDataSource
 */

void Pump::setOutputStream( const Reference< XOutputStream >& xOut )
{
    std::unique_lock aGuard( m_aMutex );
    m_xOutput = xOut;
    Reference< XConnectable > xConnect( xOut, UNO_QUERY );
    if( xConnect.is() )
        xConnect->setPredecessor( this );
    // data transfer starts in XActiveDataControl::start
}

Reference< XOutputStream > Pump::getOutputStream()
{
    std::unique_lock aGuard( m_aMutex );
    return m_xOutput;
}

// XServiceInfo
OUString Pump::getImplementationName()
{
    return "com.sun.star.comp.io.Pump";
}

// XServiceInfo
sal_Bool Pump::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > Pump::getSupportedServiceNames()
{
    return { "com.sun.star.io.Pump" };
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
io_Pump_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new io_stm::Pump());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
