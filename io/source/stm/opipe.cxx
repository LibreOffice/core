/*************************************************************************
 *
 *  $RCSfile: opipe.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:24:18 $
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

// streams
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XConnectable.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/factory.hxx>

#include <cppuhelper/implbase4.hxx>      // OWeakObject

#include <osl/conditn.hxx>
#include <osl/mutex.hxx>

#include <assert.h>
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
#define MAX_BUFFER_SIZE     0x80000000


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

    // DEBUG
    inline void checkInvariant();

    Reference < XConnectable >  m_succ;
    Reference < XConnectable >  m_pred;

    sal_Int32 m_nBytesToSkip;

    sal_Int8  *m_p;

    sal_Bool m_bOutputStreamClosed;
    sal_Bool m_bInputStreamClosed;

    oslCondition m_conditionBytesAvail;
    Mutex     m_mutexAccess;
    IFIFO       *m_pFIFO;
};



OPipeImpl::OPipeImpl()
{
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
}


// These invariants must hold when entering a guarded method or leaving a guarded method.
void OPipeImpl::checkInvariant()
{

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
                throw NotConnectedException();
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

    // this point is never reached
    return 0;
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
                throw NotConnectedException();
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

    // this point is never reached
    return 0;
}


void OPipeImpl::skipBytes(sal_Int32 nBytesToSkip)
    throw( NotConnectedException,
           BufferSizeExceededException,
           RuntimeException )
{
    MutexGuard guard( m_mutexAccess );
    if( nBytesToSkip + m_nBytesToSkip > MAX_BUFFER_SIZE || 0 > nBytesToSkip + m_nBytesToSkip )
    {
        throw BufferSizeExceededException();
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
    checkInvariant();
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
    checkInvariant();

    if( m_bOutputStreamClosed )
    {
        throw NotConnectedException();
    }

    if( m_bInputStreamClosed )
    {
        throw NotConnectedException();
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
    catch ( IFIFO_OutOfBoundsException & )
    {
        throw BufferSizeExceededException();
    }
    catch ( IFIFO_OutOfMemoryException & )
    {
        throw BufferSizeExceededException();
    }

    // readBytes may check again if enough bytes are available
    osl_setCondition( m_conditionBytesAvail );

    checkInvariant();
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
    Sequence<OUString> seq(1);
    seq.getArray()[0] = OPipeImpl_getServiceName();
    return seq;
}





/* implementation functions
*
*
*/


Reference < XInterface > SAL_CALL OPipeImpl_CreateInstance(
    const Reference < XMultiServiceFactory > & rSMgr ) throw(Exception)
{
    OPipeImpl *p = new OPipeImpl;

    return Reference < XInterface > ( SAL_STATIC_CAST( OWeakObject * , p ) );
}


OUString OPipeImpl_getServiceName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
}

OUString    OPipeImpl_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
}

Sequence<OUString> OPipeImpl_getSupportedServiceNames(void)
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = OPipeImpl_getServiceName();

    return aRet;
}
}


