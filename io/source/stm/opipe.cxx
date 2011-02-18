/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_io.hxx"

// streams
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XConnectable.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/factory.hxx>

#include <cppuhelper/implbase4.hxx>      // OWeakObject

#include <osl/conditn.hxx>
#include <osl/mutex.hxx>

#include <limits>
#include <string.h>

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;

#include "factreg.hxx"
#include "streamhelper.hxx"

// Implementation and service names
#define IMPLEMENTATION_NAME "com.sun.star.comp.io.stm.Pipe"
#define SERVICE_NAME "com.sun.star.io.Pipe"

namespace io_stm{

class OPipeImpl :
    public WeakImplHelper4< XInputStream , XOutputStream , XConnectable , XServiceInfo >
{
public:
    OPipeImpl( );
    ~OPipeImpl();

public: // XInputStream
    virtual sal_Int32 SAL_CALL readBytes(Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
        throw(  NotConnectedException,
                BufferSizeExceededException,
                RuntimeException );
    virtual sal_Int32 SAL_CALL readSomeBytes(Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead)
        throw( NotConnectedException,
               BufferSizeExceededException,
               RuntimeException );
    virtual void SAL_CALL skipBytes(sal_Int32 nBytesToSkip)
        throw( NotConnectedException,
               BufferSizeExceededException,
               RuntimeException );
    virtual sal_Int32 SAL_CALL available(void)
        throw( NotConnectedException,
               RuntimeException );
    virtual void SAL_CALL closeInput(void)
        throw( NotConnectedException,
               RuntimeException );

public: // XOutputStream

    virtual void SAL_CALL writeBytes(const Sequence< sal_Int8 >& aData)
        throw( NotConnectedException,
               BufferSizeExceededException,
               RuntimeException );
    virtual void SAL_CALL flush(void)
        throw( NotConnectedException,
               BufferSizeExceededException,
               RuntimeException );
    virtual void SAL_CALL closeOutput(void)
        throw( NotConnectedException,
               BufferSizeExceededException,
               RuntimeException );

public: // XConnectable
    virtual void SAL_CALL setPredecessor(const Reference< XConnectable >& aPredecessor)
        throw( RuntimeException );
    virtual Reference< XConnectable > SAL_CALL getPredecessor(void) throw( RuntimeException );
    virtual void SAL_CALL setSuccessor(const Reference < XConnectable > & aSuccessor)
        throw( RuntimeException );
    virtual Reference < XConnectable > SAL_CALL getSuccessor(void) throw( RuntimeException ) ;


public: // XServiceInfo
    OUString                    SAL_CALL getImplementationName() throw(  );
    Sequence< OUString >         SAL_CALL getSupportedServiceNames(void) throw(  );
    sal_Bool                        SAL_CALL supportsService(const OUString& ServiceName) throw(  );

private:

    Reference < XConnectable >  m_succ;
    Reference < XConnectable >  m_pred;

    sal_Int32 m_nBytesToSkip;

    sal_Int8  *m_p;

    sal_Bool m_bOutputStreamClosed;
    sal_Bool m_bInputStreamClosed;

    oslCondition m_conditionBytesAvail;
    Mutex     m_mutexAccess;
    I_FIFO      *m_pFIFO;
};



OPipeImpl::OPipeImpl()
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    m_nBytesToSkip  = 0;

    m_bOutputStreamClosed   = sal_False;
    m_bInputStreamClosed    = sal_False;

    m_pFIFO = new MemFIFO;
    m_conditionBytesAvail = osl_createCondition();
}

OPipeImpl::~OPipeImpl()
{
    osl_destroyCondition( m_conditionBytesAvail );
    delete m_pFIFO;
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}


sal_Int32 OPipeImpl::readBytes(Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
    throw( NotConnectedException, BufferSizeExceededException,RuntimeException )
{
    while( sal_True )
    {
        { // start guarded section
            MutexGuard guard( m_mutexAccess );
            if( m_bInputStreamClosed )
            {
                throw NotConnectedException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "Pipe::readBytes NotConnectedException" )),
                    *this );
            }
            sal_Int32 nOccupiedBufferLen = m_pFIFO->getSize();

            if( m_bOutputStreamClosed && nBytesToRead > nOccupiedBufferLen )
            {
                nBytesToRead = nOccupiedBufferLen;
            }

            if( nOccupiedBufferLen < nBytesToRead )
            {
                // wait outside guarded section
                osl_resetCondition( m_conditionBytesAvail );
            }
            else {
                // necessary bytes are available
                m_pFIFO->read( aData , nBytesToRead );
                return nBytesToRead;
            }
        } // end guarded section

        // wait for new data outside guarded section!
        osl_waitCondition( m_conditionBytesAvail , 0 );
    }
}


sal_Int32 OPipeImpl::readSomeBytes(Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead)
    throw( NotConnectedException,
           BufferSizeExceededException,
           RuntimeException )
{
    while( sal_True ) {
        {
            MutexGuard guard( m_mutexAccess );
            if( m_bInputStreamClosed )
            {
                throw NotConnectedException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "Pipe::readSomeBytes NotConnectedException" )),
                    *this );
            }
            if( m_pFIFO->getSize() )
            {
                sal_Int32 nSize = Min( nMaxBytesToRead , m_pFIFO->getSize() );
                aData.realloc( nSize );
                m_pFIFO->read( aData , nSize );
                return nSize;
            }

            if( m_bOutputStreamClosed )
            {
                // no bytes in buffer anymore
                return 0;
            }
        }

        osl_waitCondition( m_conditionBytesAvail , 0 );
    }
}


void OPipeImpl::skipBytes(sal_Int32 nBytesToSkip)
    throw( NotConnectedException,
           BufferSizeExceededException,
           RuntimeException )
{
    MutexGuard guard( m_mutexAccess );
    if( m_bInputStreamClosed )
    {
        throw NotConnectedException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Pipe::skipBytes NotConnectedException" ) ),
            *this );
    }

    if( nBytesToSkip < 0
        || (nBytesToSkip
            > std::numeric_limits< sal_Int32 >::max() - m_nBytesToSkip) )
    {
        throw BufferSizeExceededException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Pipe::skipBytes BufferSizeExceededException" )),
            *this );
    }
    m_nBytesToSkip += nBytesToSkip;

    nBytesToSkip = Min( m_pFIFO->getSize() , m_nBytesToSkip );
    m_pFIFO->skip( nBytesToSkip );
    m_nBytesToSkip -= nBytesToSkip;
}


sal_Int32 OPipeImpl::available(void)
    throw( NotConnectedException,
           RuntimeException )
 {
    MutexGuard guard( m_mutexAccess );
    if( m_bInputStreamClosed )
    {
        throw NotConnectedException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Pipe::available NotConnectedException" ) ),
            *this );
    }
    return m_pFIFO->getSize();
}

void OPipeImpl::closeInput(void)
    throw( NotConnectedException,
           RuntimeException)
{
    MutexGuard guard( m_mutexAccess );

    m_bInputStreamClosed = sal_True;

    delete m_pFIFO;
    m_pFIFO = 0;

    // readBytes may throw an exception
    osl_setCondition( m_conditionBytesAvail );

    setSuccessor( Reference< XConnectable > () );
    return;
}


void OPipeImpl::writeBytes(const Sequence< sal_Int8 >& aData)
    throw( NotConnectedException,
           BufferSizeExceededException,
           RuntimeException)
{
    MutexGuard guard( m_mutexAccess );

    if( m_bOutputStreamClosed )
    {
        throw NotConnectedException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Pipe::writeBytes NotConnectedException (outputstream)" )),
            *this );
    }

    if( m_bInputStreamClosed )
    {
        throw NotConnectedException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Pipe::writeBytes NotConnectedException (inputstream)" )),
            *this );
    }

    // check skipping
    sal_Int32 nLen = aData.getLength();
    if( m_nBytesToSkip  && m_nBytesToSkip >= nLen  ) {
        // all must be skipped - forget whole call
        m_nBytesToSkip -= nLen;
        return;
    }

    // adjust buffersize if necessary

    try
    {
        if( m_nBytesToSkip )
        {
            Sequence< sal_Int8 > seqCopy( nLen - m_nBytesToSkip );
            memcpy( seqCopy.getArray() , &( aData.getConstArray()[m_nBytesToSkip] ) , nLen-m_nBytesToSkip );
            m_pFIFO->write( seqCopy );
        }
        else
        {
            m_pFIFO->write( aData );
        }
        m_nBytesToSkip = 0;
    }
    catch ( I_FIFO_OutOfBoundsException & )
    {
        throw BufferSizeExceededException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Pipe::writeBytes BufferSizeExceededException" )),
            *this );
    }
    catch ( I_FIFO_OutOfMemoryException & )
    {
        throw BufferSizeExceededException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Pipe::writeBytes BufferSizeExceededException" )),
            *this );
    }

    // readBytes may check again if enough bytes are available
    osl_setCondition( m_conditionBytesAvail );
}


void OPipeImpl::flush(void)
    throw( NotConnectedException,
           BufferSizeExceededException,
           RuntimeException)
{
    // nothing to do for a pipe
    return;
}

void OPipeImpl::closeOutput(void)
    throw( NotConnectedException,
           BufferSizeExceededException,
           RuntimeException)
{
    MutexGuard guard( m_mutexAccess );

    m_bOutputStreamClosed = sal_True;
    osl_setCondition( m_conditionBytesAvail );
    setPredecessor( Reference < XConnectable > () );
    return;
}


void OPipeImpl::setSuccessor( const Reference < XConnectable >  &r )
    throw( RuntimeException )
{
     /// if the references match, nothing needs to be done
     if( m_succ != r ) {
         /// store the reference for later use
         m_succ = r;

         if( m_succ.is() )
         {
              m_succ->setPredecessor(
                  Reference< XConnectable > ( SAL_STATIC_CAST( XConnectable * , this ) ) );
         }
     }
}

Reference < XConnectable > OPipeImpl::getSuccessor()    throw( RuntimeException )
{
    return m_succ;
}


// XDataSource
void OPipeImpl::setPredecessor( const Reference < XConnectable > &r )
    throw( RuntimeException )
{
    if( r != m_pred ) {
        m_pred = r;
        if( m_pred.is() ) {
            m_pred->setSuccessor(
                Reference < XConnectable > ( SAL_STATIC_CAST( XConnectable * , this ) ) );
        }
    }
}

Reference < XConnectable > OPipeImpl::getPredecessor() throw( RuntimeException )
{
    return m_pred;
}




// XServiceInfo
OUString OPipeImpl::getImplementationName() throw(  )
{
    return OPipeImpl_getImplementationName();
}

// XServiceInfo
sal_Bool OPipeImpl::supportsService(const OUString& ServiceName) throw(  )
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

// XServiceInfo
Sequence< OUString > OPipeImpl::getSupportedServiceNames(void) throw(  )
{
    return OPipeImpl_getSupportedServiceNames();
}





/* implementation functions
*
*
*/


Reference < XInterface > SAL_CALL OPipeImpl_CreateInstance(
    const Reference < XComponentContext > & ) throw(Exception)
{
    OPipeImpl *p = new OPipeImpl;

    return Reference < XInterface > ( SAL_STATIC_CAST( OWeakObject * , p ) );
}


OUString    OPipeImpl_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM ( IMPLEMENTATION_NAME ) );
}

Sequence<OUString> OPipeImpl_getSupportedServiceNames(void)
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICE_NAME ));
    return aRet;
}
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
