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


#include <stdio.h>

#include <sal/log.hxx>

#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/io/XConnectable.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include <uno/dispatcher.h>
#include <uno/mapping.hxx>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/mutex.hxx>
#include <osl/thread.h>


using namespace osl;
using namespace std;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::io;

#include "services.hxx"

namespace io_stm {

    class Pump : public WeakImplHelper5<
          XActiveDataSource, XActiveDataSink, XActiveDataControl, XConnectable, XServiceInfo >
    {
        Mutex                                   m_aMutex;
        oslThread                               m_aThread;

        Reference< XConnectable >               m_xPred;
        Reference< XConnectable >               m_xSucc;
        Reference< XInputStream >               m_xInput;
        Reference< XOutputStream >              m_xOutput;
        OInterfaceContainerHelper               m_cnt;
        sal_Bool                                m_closeFired;

        void run();
        static void static_run( void* pObject );

        void close();
        void fireClose();
        void fireStarted();
        void fireTerminated();
        void fireError( const Any &a );

    public:
        Pump();
        virtual ~Pump();

        // XActiveDataSource
        virtual void SAL_CALL setOutputStream( const Reference< ::com::sun::star::io::XOutputStream >& xOutput ) throw();
        virtual Reference< ::com::sun::star::io::XOutputStream > SAL_CALL getOutputStream() throw();

        // XActiveDataSink
        virtual void SAL_CALL setInputStream( const Reference< ::com::sun::star::io::XInputStream >& xStream ) throw();
        virtual Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream() throw();

        // XActiveDataControl
        virtual void SAL_CALL addListener( const Reference< ::com::sun::star::io::XStreamListener >& xListener ) throw();
        virtual void SAL_CALL removeListener( const Reference< ::com::sun::star::io::XStreamListener >& xListener ) throw();
        virtual void SAL_CALL start() throw( RuntimeException );
        virtual void SAL_CALL terminate() throw();

        // XConnectable
        virtual void SAL_CALL setPredecessor( const Reference< ::com::sun::star::io::XConnectable >& xPred ) throw();
        virtual Reference< ::com::sun::star::io::XConnectable > SAL_CALL getPredecessor() throw();
        virtual void SAL_CALL setSuccessor( const Reference< ::com::sun::star::io::XConnectable >& xSucc ) throw();
        virtual Reference< ::com::sun::star::io::XConnectable > SAL_CALL getSuccessor() throw();

    public: // XServiceInfo
        virtual OUString    SAL_CALL getImplementationName() throw(  );
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(void) throw(  );
        virtual sal_Bool     SAL_CALL supportsService(const OUString& ServiceName) throw(  );
    };

Pump::Pump() : m_aThread( 0 ),
               m_cnt( m_aMutex ),
               m_closeFired( sal_False )
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
    OInterfaceIteratorHelper iter( m_cnt );
    while( iter.hasMoreElements() )
    {
        try
        {
            static_cast< XStreamListener * > ( iter.next() )->error( exception );
        }
        catch ( const RuntimeException &e )
        {
            SAL_WARN("io.streams","com.sun.star.comp.stoc.Pump: unexpected exception during calling listeners" << e.Message);
        }
    }
}

void Pump::fireClose()
{
    sal_Bool bFire = sal_False;
    {
        MutexGuard guard( m_aMutex );
        if( ! m_closeFired  )
        {
            m_closeFired = sal_True;
            bFire = sal_True;
        }
    }

    if( bFire )
    {
        OInterfaceIteratorHelper iter( m_cnt );
        while( iter.hasMoreElements() )
        {
            try
            {
                static_cast< XStreamListener * > ( iter.next() )->closed( );
            }
            catch ( const RuntimeException &e )
            {
                SAL_WARN("io.streams","com.sun.star.comp.stoc.Pump: unexpected exception during calling listeners" << e.Message);
            }
        }
    }
}

void Pump::fireStarted()
{
    OInterfaceIteratorHelper iter( m_cnt );
    while( iter.hasMoreElements() )
    {
        try
        {
            static_cast< XStreamListener * > ( iter.next() )->started( );
        }
        catch ( const RuntimeException &e )
        {
            SAL_WARN("io.streams","com.sun.star.comp.stoc.Pump: unexpected exception during calling listeners" << e.Message);
        }
    }
}

void Pump::fireTerminated()
{
    OInterfaceIteratorHelper iter( m_cnt );
    while( iter.hasMoreElements() )
    {
        try
        {
            static_cast< XStreamListener * > ( iter.next() )->terminated();
        }
        catch ( const RuntimeException &e )
        {
            SAL_WARN("io.streams","com.sun.star.comp.stoc.Pump: unexpected exception during calling listeners" << e.Message);
        }
    }
}



void Pump::close()
{
    // close streams and release references
    Reference< XInputStream > rInput;
    Reference< XOutputStream > rOutput;
    {
        MutexGuard guard( m_aMutex );
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
    ((Pump*)pObject)->run();
    ((Pump*)pObject)->release();
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
                Guard< Mutex > aGuard( m_aMutex );
                rInput = m_xInput;
                rOutput = m_xOutput;
            }

            if( ! rInput.is() )
            {
                NotConnectedException exception(
                    OUString("no input stream set") , Reference<XInterface>((OWeakObject*)this) );
                throw exception;
            }
            Sequence< sal_Int8 > aData;
            while( rInput->readSomeBytes( aData, 65536 ) )
            {
                if( ! rOutput.is() )
                {
                    NotConnectedException exception(
                        OUString("no output stream set") , Reference<XInterface>( (OWeakObject*)this) );
                    throw exception;
                }
                rOutput->writeBytes( aData );
                osl_yieldThread();
            }
        }
        catch ( const IOException & e )
        {
            fireError( makeAny( e ) );
        }
        catch ( const RuntimeException & e )
        {
            fireError( makeAny( e ) );
        }
        catch ( const Exception & e )
        {
            fireError( makeAny( e ) );
        }

        close();
        fireClose();
    }
    catch ( const com::sun::star::uno::Exception &e )
    {
        // we are the last on the stack.
        // this is to avoid crashing the program, when e.g. a bridge crashes
        SAL_WARN("io.streams","com.sun.star.comp.stoc.Pump: unexpected exception during calling listeners" << e.Message);
    }
}



/*
 * XConnectable
 */

void Pump::setPredecessor( const Reference< XConnectable >& xPred ) throw()
{
    Guard< Mutex > aGuard( m_aMutex );
    m_xPred = xPred;
}



Reference< XConnectable > Pump::getPredecessor() throw()
{
    Guard< Mutex > aGuard( m_aMutex );
    return m_xPred;
}



void Pump::setSuccessor( const Reference< XConnectable >& xSucc ) throw()
{
    Guard< Mutex > aGuard( m_aMutex );
    m_xSucc = xSucc;
}



Reference< XConnectable > Pump::getSuccessor() throw()
{
    Guard< Mutex > aGuard( m_aMutex );
    return m_xSucc;
}



/*
 * XActiveDataControl
 */

void Pump::addListener( const Reference< XStreamListener >& xListener ) throw()
{
    m_cnt.addInterface( xListener );
}



void Pump::removeListener( const Reference< XStreamListener >& xListener ) throw()
{
    m_cnt.removeInterface( xListener );
}



void Pump::start() throw( RuntimeException )
{
    Guard< Mutex > aGuard( m_aMutex );
    m_aThread = osl_createSuspendedThread((oslWorkerFunction)Pump::static_run,this);
    if( m_aThread )
    {
        // will be released by OPump::static_run
        acquire();
        osl_resumeThread( m_aThread );
    }
    else
    {
        throw RuntimeException(
            OUString("Pump::start Couldn't create worker thread"),
            *this);
    }
}



void Pump::terminate() throw()
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

void Pump::setInputStream( const Reference< XInputStream >& xStream ) throw()
{
    Guard< Mutex > aGuard( m_aMutex );
    m_xInput = xStream;
    Reference< XConnectable > xConnect( xStream, UNO_QUERY );
    if( xConnect.is() )
        xConnect->setSuccessor( this );
    // data transfer starts in XActiveDataControl::start
}



Reference< XInputStream > Pump::getInputStream() throw()
{
    Guard< Mutex > aGuard( m_aMutex );
    return m_xInput;
}



/*
 * XActiveDataSource
 */

void Pump::setOutputStream( const Reference< XOutputStream >& xOut ) throw()
{
    Guard< Mutex > aGuard( m_aMutex );
    m_xOutput = xOut;
    Reference< XConnectable > xConnect( xOut, UNO_QUERY );
    if( xConnect.is() )
        xConnect->setPredecessor( this );
    // data transfer starts in XActiveDataControl::start
}

Reference< XOutputStream > Pump::getOutputStream() throw()
{
    Guard< Mutex > aGuard( m_aMutex );
    return m_xOutput;
}

// XServiceInfo
OUString Pump::getImplementationName() throw(  )
{
    return OPumpImpl_getImplementationName();
}

// XServiceInfo
sal_Bool Pump::supportsService(const OUString& ServiceName) throw(  )
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > Pump::getSupportedServiceNames(void) throw(  )
{
    return OPumpImpl_getSupportedServiceNames();
}


Reference< XInterface > SAL_CALL OPumpImpl_CreateInstance(
    SAL_UNUSED_PARAMETER const Reference< XComponentContext > & )
    throw (Exception)
{
    return Reference< XInterface >( *new Pump );
}

OUString OPumpImpl_getImplementationName()
{
    return OUString("com.sun.star.comp.io.Pump");
}

Sequence<OUString> OPumpImpl_getSupportedServiceNames(void)
{
    OUString s("com.sun.star.io.Pump");
    Sequence< OUString > seq( &s , 1 );
    return seq;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
