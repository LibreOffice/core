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


// streams
#include <com/sun/star/io/XPipe.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XConnectable.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <osl/conditn.hxx>
#include <osl/mutex.hxx>

#include <limits>
#include <string.h>

using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;

#include "services.hxx"
#include "streamhelper.hxx"

// Implementation and service names
#define IMPLEMENTATION_NAME "com.sun.star.comp.io.stm.Pipe"

namespace io_stm{

class OPipeImpl :
    public WeakImplHelper< XPipe , XConnectable , XServiceInfo >
{
public:
    OPipeImpl( );
    virtual ~OPipeImpl();

public: // XInputStream
    virtual sal_Int32 SAL_CALL readBytes(Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
        throw(  NotConnectedException,
                BufferSizeExceededException,
                RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL readSomeBytes(Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead)
        throw( NotConnectedException,
               BufferSizeExceededException,
               RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL skipBytes(sal_Int32 nBytesToSkip)
        throw( NotConnectedException,
               BufferSizeExceededException,
               RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL available()
        throw( NotConnectedException,
               RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL closeInput()
        throw( NotConnectedException,
               RuntimeException, std::exception ) SAL_OVERRIDE;

public: // XOutputStream

    virtual void SAL_CALL writeBytes(const Sequence< sal_Int8 >& aData)
        throw( NotConnectedException,
               BufferSizeExceededException,
               RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL flush()
        throw( NotConnectedException,
               BufferSizeExceededException,
               RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL closeOutput()
        throw( NotConnectedException,
               BufferSizeExceededException,
               RuntimeException, std::exception ) SAL_OVERRIDE;

public: // XConnectable
    virtual void SAL_CALL setPredecessor(const Reference< XConnectable >& aPredecessor)
        throw( RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual Reference< XConnectable > SAL_CALL getPredecessor() throw( RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL setSuccessor(const Reference < XConnectable > & aSuccessor)
        throw( RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual Reference < XConnectable > SAL_CALL getSuccessor() throw( RuntimeException, std::exception ) SAL_OVERRIDE ;


public: // XServiceInfo
    OUString                    SAL_CALL getImplementationName() throw(std::exception  ) SAL_OVERRIDE;
    Sequence< OUString >         SAL_CALL getSupportedServiceNames() throw(std::exception  ) SAL_OVERRIDE;
    sal_Bool                        SAL_CALL supportsService(const OUString& ServiceName) throw(std::exception  ) SAL_OVERRIDE;

private:

    Reference < XConnectable >  m_succ;
    Reference < XConnectable >  m_pred;

    sal_Int32 m_nBytesToSkip;

    bool m_bOutputStreamClosed;
    bool m_bInputStreamClosed;

    osl::Condition m_conditionBytesAvail;
    Mutex          m_mutexAccess;
    MemFIFO       *m_pFIFO;
};



OPipeImpl::OPipeImpl()
{
    m_nBytesToSkip  = 0;

    m_bOutputStreamClosed   = false;
    m_bInputStreamClosed    = false;

    m_pFIFO = new MemFIFO;
}

OPipeImpl::~OPipeImpl()
{
    delete m_pFIFO;
}


sal_Int32 OPipeImpl::readBytes(Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
    throw( NotConnectedException, BufferSizeExceededException,RuntimeException, std::exception )
{
    while( true )
    {
        { // start guarded section
            MutexGuard guard( m_mutexAccess );
            if( m_bInputStreamClosed )
            {
                throw NotConnectedException(
                    "Pipe::readBytes NotConnectedException",
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
                m_conditionBytesAvail.reset();
            }
            else {
                // necessary bytes are available
                m_pFIFO->read( aData , nBytesToRead );
                return nBytesToRead;
            }
        } // end guarded section

        // wait for new data outside guarded section!
        m_conditionBytesAvail.wait();
    }
}


sal_Int32 OPipeImpl::readSomeBytes(Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead)
    throw( NotConnectedException,
           BufferSizeExceededException,
           RuntimeException, std::exception )
{
    while( true ) {
        {
            MutexGuard guard( m_mutexAccess );
            if( m_bInputStreamClosed )
            {
                throw NotConnectedException(
                    "Pipe::readSomeBytes NotConnectedException",
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

        m_conditionBytesAvail.wait();
    }
}


void OPipeImpl::skipBytes(sal_Int32 nBytesToSkip)
    throw( NotConnectedException,
           BufferSizeExceededException,
           RuntimeException, std::exception )
{
    MutexGuard guard( m_mutexAccess );
    if( m_bInputStreamClosed )
    {
        throw NotConnectedException(
            "Pipe::skipBytes NotConnectedException",
            *this );
    }

    if( nBytesToSkip < 0
        || (nBytesToSkip
            > std::numeric_limits< sal_Int32 >::max() - m_nBytesToSkip) )
    {
        throw BufferSizeExceededException(
            "Pipe::skipBytes BufferSizeExceededException",
            *this );
    }
    m_nBytesToSkip += nBytesToSkip;

    nBytesToSkip = Min( m_pFIFO->getSize() , m_nBytesToSkip );
    m_pFIFO->skip( nBytesToSkip );
    m_nBytesToSkip -= nBytesToSkip;
}


sal_Int32 OPipeImpl::available()
    throw( NotConnectedException,
           RuntimeException, std::exception )
 {
    MutexGuard guard( m_mutexAccess );
    if( m_bInputStreamClosed )
    {
        throw NotConnectedException(
            "Pipe::available NotConnectedException",
            *this );
    }
    return m_pFIFO->getSize();
}

void OPipeImpl::closeInput()
    throw( NotConnectedException,
           RuntimeException, std::exception)
{
    MutexGuard guard( m_mutexAccess );

    m_bInputStreamClosed = true;

    delete m_pFIFO;
    m_pFIFO = 0;

    // readBytes may throw an exception
    m_conditionBytesAvail.set();

    setSuccessor( Reference< XConnectable > () );
    return;
}


void OPipeImpl::writeBytes(const Sequence< sal_Int8 >& aData)
    throw( NotConnectedException,
           BufferSizeExceededException,
           RuntimeException, std::exception)
{
    MutexGuard guard( m_mutexAccess );

    if( m_bOutputStreamClosed )
    {
        throw NotConnectedException(
            "Pipe::writeBytes NotConnectedException (outputstream)",
            *this );
    }

    if( m_bInputStreamClosed )
    {
        throw NotConnectedException(
            "Pipe::writeBytes NotConnectedException (inputstream)",
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
            "Pipe::writeBytes BufferSizeExceededException",
            *this );
    }
    catch ( I_FIFO_OutOfMemoryException & )
    {
        throw BufferSizeExceededException(
            "Pipe::writeBytes BufferSizeExceededException",
            *this );
    }

    // readBytes may check again if enough bytes are available
    m_conditionBytesAvail.set();
}


void OPipeImpl::flush()
    throw( NotConnectedException,
           BufferSizeExceededException,
           RuntimeException, std::exception)
{
    // nothing to do for a pipe
    return;
}

void OPipeImpl::closeOutput()
    throw( NotConnectedException,
           BufferSizeExceededException,
           RuntimeException, std::exception)
{
    MutexGuard guard( m_mutexAccess );

    m_bOutputStreamClosed = true;
    m_conditionBytesAvail.set();
    setPredecessor( Reference < XConnectable > () );
    return;
}


void OPipeImpl::setSuccessor( const Reference < XConnectable >  &r )
    throw( RuntimeException, std::exception )
{
     /// if the references match, nothing needs to be done
     if( m_succ != r ) {
         /// store the reference for later use
         m_succ = r;

         if( m_succ.is() )
         {
              m_succ->setPredecessor(
                  Reference< XConnectable > ( (static_cast< XConnectable *  >(this)) ) );
         }
     }
}

Reference < XConnectable > OPipeImpl::getSuccessor()    throw( RuntimeException, std::exception )
{
    return m_succ;
}


// XDataSource
void OPipeImpl::setPredecessor( const Reference < XConnectable > &r )
    throw( RuntimeException, std::exception )
{
    if( r != m_pred ) {
        m_pred = r;
        if( m_pred.is() ) {
            m_pred->setSuccessor(
                Reference < XConnectable > ( (static_cast< XConnectable *  >(this)) ) );
        }
    }
}

Reference < XConnectable > OPipeImpl::getPredecessor() throw( RuntimeException, std::exception )
{
    return m_pred;
}




// XServiceInfo
OUString OPipeImpl::getImplementationName() throw(std::exception  )
{
    return OPipeImpl_getImplementationName();
}

// XServiceInfo
sal_Bool OPipeImpl::supportsService(const OUString& ServiceName) throw(std::exception  )
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > OPipeImpl::getSupportedServiceNames() throw(std::exception  )
{
    return OPipeImpl_getSupportedServiceNames();
}

/* implementation functions
*
*
*/


Reference < XInterface > SAL_CALL OPipeImpl_CreateInstance(
    SAL_UNUSED_PARAMETER const Reference < XComponentContext > & )
    throw(Exception)
{
    OPipeImpl *p = new OPipeImpl;

    return Reference < XInterface > ( (static_cast< OWeakObject *  >(p)) );
}


OUString    OPipeImpl_getImplementationName()
{
    return OUString( IMPLEMENTATION_NAME );
}

Sequence<OUString> OPipeImpl_getSupportedServiceNames()
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = "com.sun.star.io.Pipe";
    return aRet;
}
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
