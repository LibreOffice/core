/*************************************************************************
 *
 *  $RCSfile: opump.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: svesik $ $Date: 2000-11-23 14:42:26 $
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

#include <stdio.h>

#include <osl/diagnose.h>

#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/io/XConnectable.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include <uno/dispatcher.h>
#include <uno/mapping.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/factory.hxx>
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <list>

using namespace osl;
using namespace std;
using namespace rtl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::io;

namespace io_stm {

    class Pump : public WeakImplHelper4<
          XActiveDataSource, XActiveDataSink, XActiveDataControl, XConnectable >
    {
        Mutex                                   m_aMutex;
        oslThread                               m_aThread;

        Reference< XConnectable >               m_xPred;
        Reference< XConnectable >               m_xSucc;
        Reference< XInputStream >               m_xInput;
        Reference< XOutputStream >              m_xOutput;
        list< Reference< XStreamListener > >    m_aListeners;

        void run();
        static void static_run( void* pObject );

        void close();
        void fireError( list< Reference< XStreamListener > > & , Any & exception);

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
        virtual void SAL_CALL start() throw();
        virtual void SAL_CALL terminate() throw();

        // XConnectable
        virtual void SAL_CALL setPredecessor( const Reference< ::com::sun::star::io::XConnectable >& xPred ) throw();
        virtual Reference< ::com::sun::star::io::XConnectable > SAL_CALL getPredecessor() throw();
        virtual void SAL_CALL setSuccessor( const Reference< ::com::sun::star::io::XConnectable >& xSucc ) throw();
        virtual Reference< ::com::sun::star::io::XConnectable > SAL_CALL getSuccessor() throw();

    };

Pump::Pump() : m_aThread( NULL )
{
}

Pump::~Pump()
{
    // exit gracefully
    osl_joinWithThread( m_aThread );
    osl_freeThreadHandle( m_aThread );
}

void Pump::fireError( list< Reference< XStreamListener > > &aList , Any & exception )
{
    list< Reference< XStreamListener > > aLocalListeners = aList;
    list< Reference< XStreamListener > >::iterator it;
    for( it = aLocalListeners.begin();
         it != aLocalListeners.end(); ++it )
    {
        try
        {
            (*it)->error( exception );
        }
        catch ( RuntimeException &e )
        {
            OString sMessage = OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US );
            OSL_ENSHURE( !"com.sun.star.comp.stoc.Pump: unexpected exception during calling listeners", sMessage.getStr() );
        }
    }
}

void Pump::close()
{
    // close streams and release references

    if( m_xInput.is() )
    {
        m_xInput->closeInput();
        m_xInput = Reference< XInputStream >();
    }
    if( m_xOutput.is() )
    {
        m_xOutput->closeOutput();
        m_xOutput = Reference< XOutputStream >();
    }
    m_aListeners = list< Reference< XStreamListener > >();
    m_xSucc = Reference< XConnectable >();
    m_xPred = Reference< XConnectable >();
}

void Pump::static_run( void* pObject )
{
    ((Pump*)pObject)->run();
}

void Pump::run()
{
    Guard< Mutex > aGuard( m_aMutex );

    try
    {
        list< Reference< XStreamListener > >::iterator it;
        for( it = m_aListeners.begin(); it != m_aListeners.end(); ++it )
        {
            (*it)->started();
        }

        try
        {
            Sequence< sal_Int8 > aData;
            long nBytes;
            while( nBytes = m_xInput->readBytes( aData, 65536 ) )
            {
                m_xOutput->writeBytes( aData );
                if( nBytes < 65536 ) // marks EOF or error
                    break;
                osl_yieldThread();
            }

        }
        catch ( IOException & e )
        {
            Any aException;
            aException <<= e;
            fireError( m_aListeners , aException );
        }
        catch ( RuntimeException & e )
        {
            Any aException;
            aException <<= e;
            fireError( m_aListeners , aException );
        }
        catch ( Exception & e )
        {
            Any aException;
            aException <<= e;
            fireError( m_aListeners , aException );
        }

        // listeners may remove themselves when called this way
        list< Reference< XStreamListener > > aLocalListeners = m_aListeners;
        close();

        for( it = aLocalListeners.begin();
             it != aLocalListeners.end(); ++it )
        {
            try
            {
                (*it)->closed();
            }
            catch ( RuntimeException &e )
            {
                OString sMessage = OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US );
                OSL_ENSHURE( !"com.sun.star.comp.stoc.Pump: unexpected exception during calling listeners", sMessage.getStr() );
            }
        }
    }
    catch ( com::sun::star::uno::Exception &e )
    {
        // we are the last on the stack.
        // this is to avoid crashing the program, when e.g. a bridge crashes
        OString sMessage = OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US );
        OSL_ENSHURE( !"com.sun.star.comp.stoc.Pump: unexpected exception", sMessage.getStr() );
    }
}

// ------------------------------------------------------------

/*
 * XConnectable
 */

void Pump::setPredecessor( const Reference< XConnectable >& xPred ) throw()
{
    Guard< Mutex > aGuard( m_aMutex );

    m_xPred = xPred;
}

// ------------------------------------------------------------

Reference< XConnectable > Pump::getPredecessor() throw()
{
    Guard< Mutex > aGuard( m_aMutex );

    return m_xPred;
}

// ------------------------------------------------------------

void Pump::setSuccessor( const Reference< XConnectable >& xSucc ) throw()
{
    Guard< Mutex > aGuard( m_aMutex );

    m_xSucc = xSucc;
}

// ------------------------------------------------------------

Reference< XConnectable > Pump::getSuccessor() throw()
{
    Guard< Mutex > aGuard( m_aMutex );

    return m_xSucc;
}

// -----------------------------------------------------------------

/*
 * XActiveDataControl
 */

void Pump::addListener( const Reference< XStreamListener >& xListener ) throw()
{
    Guard< Mutex > aGuard( m_aMutex );

    m_aListeners.push_back( xListener );
}

// ------------------------------------------------------------

void Pump::removeListener( const Reference< XStreamListener >& xListener ) throw()
{
    Guard< Mutex > aGuard( m_aMutex );

    m_aListeners.remove( xListener );
}

// ------------------------------------------------------------

void Pump::start() throw()
{
    Guard< Mutex > aGuard( m_aMutex );

    m_aThread = osl_createThread(
        (oslWorkerFunction)Pump::static_run,
        (void*)this
        );
}

// ------------------------------------------------------------

void Pump::terminate() throw()
{
    osl_joinWithThread( m_aThread );

    Guard< Mutex > aGuard( m_aMutex );

    // listeners may remove themselves when called this way
    list< Reference< XStreamListener > > aLocalListeners = m_aListeners;
    close();

    for( list< Reference< XStreamListener > >::iterator it = aLocalListeners.begin();
         it != aLocalListeners.end(); ++it )
    {
        (*it)->terminated();
    }
}

// ------------------------------------------------------------

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

// ------------------------------------------------------------

Reference< XInputStream > Pump::getInputStream() throw()
{
    Guard< Mutex > aGuard( m_aMutex );

    return m_xInput;
}

// ------------------------------------------------------------

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

// ------------------------------------------------------------

Reference< XOutputStream > Pump::getOutputStream() throw()
{
    Guard< Mutex > aGuard( m_aMutex );

    return m_xOutput;
}


Reference< XInterface > SAL_CALL OPumpImpl_CreateInstance( const Reference< XMultiServiceFactory > & rSMgr ) throw (Exception)
{
    return Reference< XInterface >( *new Pump );
}

OUString OPumpImpl_getServiceName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.io.Pump" ) );
}

OUString OPumpImpl_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.io.Pump") );
}

Sequence<OUString> OPumpImpl_getSupportedServiceNames(void)
{
    OUString s = OPumpImpl_getServiceName();
    Sequence< OUString > seq( &s , 1 );
    return seq;
}

}

