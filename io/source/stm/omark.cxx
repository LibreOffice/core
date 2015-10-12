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


#include <map>
#include <vector>

#include <com/sun/star/io/XMarkableStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XConnectable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <osl/mutex.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>

#include <string.h>


using namespace ::std;
using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

#include "services.hxx"
#include "streamhelper.hxx"

namespace io_stm {

/***********************
*
* OMarkableOutputStream.
*
* This object allows to set marks in an outputstream. It is allowed to jump back to the marks and
* rewrite the some bytes.
*
*         The object must buffer the data since the last mark set. Flush will not
*         have any effect. As soon as the last mark has been removed, the object may write the data
*         through to the chained object.
*
**********************/
class OMarkableOutputStream :
    public WeakImplHelper< XOutputStream ,
                            XActiveDataSource ,
                            XMarkableStream ,
                            XConnectable,
                            XServiceInfo
                          >
{
public:
    OMarkableOutputStream(  );
    virtual ~OMarkableOutputStream();

public: // XOutputStream
    virtual void SAL_CALL writeBytes(const Sequence< sal_Int8 >& aData)
        throw ( NotConnectedException,
                BufferSizeExceededException,
                RuntimeException, std::exception) override;
    virtual void SAL_CALL flush()
        throw ( NotConnectedException,
                BufferSizeExceededException,
                RuntimeException, std::exception) override;
    virtual void SAL_CALL closeOutput()
        throw ( NotConnectedException,
                BufferSizeExceededException,
                RuntimeException, std::exception) override;

public: // XMarkable
    virtual sal_Int32 SAL_CALL createMark()
        throw (IOException, RuntimeException, std::exception) override;
    virtual void SAL_CALL deleteMark(sal_Int32 Mark)
        throw (IOException,
               IllegalArgumentException,
               RuntimeException, std::exception) override;
    virtual void SAL_CALL jumpToMark(sal_Int32 nMark)
        throw (IOException,
               IllegalArgumentException,
               RuntimeException, std::exception) override;
    virtual void SAL_CALL jumpToFurthest()
        throw (IOException, RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL offsetToMark(sal_Int32 nMark)
        throw (IOException,
               IllegalArgumentException,
               RuntimeException, std::exception) override;

public: // XActiveDataSource
    virtual void SAL_CALL setOutputStream(const Reference < XOutputStream > & aStream)
        throw (RuntimeException, std::exception) override;
    virtual Reference < XOutputStream > SAL_CALL getOutputStream()
        throw (RuntimeException, std::exception) override;

public: // XConnectable
    virtual void SAL_CALL setPredecessor(const Reference < XConnectable > & aPredecessor)
        throw (RuntimeException, std::exception) override;
    virtual Reference < XConnectable > SAL_CALL getPredecessor() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setSuccessor(const Reference < XConnectable >& aSuccessor)
        throw (RuntimeException, std::exception) override;
    virtual Reference<  XConnectable >  SAL_CALL getSuccessor() throw (RuntimeException, std::exception) override;

public: // XServiceInfo
    OUString                     SAL_CALL getImplementationName() throw (std::exception) override;
    Sequence< OUString >         SAL_CALL getSupportedServiceNames() throw (std::exception) override;
    sal_Bool                        SAL_CALL supportsService(const OUString& ServiceName) throw (std::exception) override;

private:
    // helper methods
    void checkMarksAndFlush() throw( NotConnectedException, BufferSizeExceededException);

    Reference< XConnectable > m_succ;
    Reference< XConnectable > m_pred;

    Reference< XOutputStream >  m_output;
    bool m_bValidStream;

    MemRingBuffer *m_pBuffer;
    map<sal_Int32,sal_Int32,less< sal_Int32 > > m_mapMarks;
    sal_Int32 m_nCurrentPos;
    sal_Int32 m_nCurrentMark;

    Mutex m_mutex;
};

OMarkableOutputStream::OMarkableOutputStream( )
    : m_bValidStream(false)
    , m_nCurrentPos(0)
    , m_nCurrentMark(0)
{
    m_pBuffer = new MemRingBuffer;
}

OMarkableOutputStream::~OMarkableOutputStream()
{
    delete m_pBuffer;
}


// XOutputStream
void OMarkableOutputStream::writeBytes(const Sequence< sal_Int8 >& aData)
    throw ( NotConnectedException,
            BufferSizeExceededException,
            RuntimeException, std::exception)
{
    if( m_bValidStream ) {
        if( m_mapMarks.empty() && ( m_pBuffer->getSize() == 0 ) ) {
            // no mark and  buffer active, simple write through
            m_output->writeBytes( aData );
        }
        else {
            MutexGuard guard( m_mutex );
            // new data must be buffered
            try
            {
                m_pBuffer->writeAt( m_nCurrentPos , aData );
                m_nCurrentPos += aData.getLength();
            }
            catch( IRingBuffer_OutOfBoundsException & )
            {
                throw BufferSizeExceededException();
            }
            catch( IRingBuffer_OutOfMemoryException & )
            {
                throw BufferSizeExceededException();
            }
            checkMarksAndFlush();
        }
    }
    else {
        throw NotConnectedException();
    }
}

void OMarkableOutputStream::flush()
    throw ( NotConnectedException,
            BufferSizeExceededException,
            RuntimeException, std::exception)
{
    Reference< XOutputStream > output;
    {
        MutexGuard guard( m_mutex );
        output = m_output;
    }

    // Markable cannot flush buffered data, because the data may get rewritten,
    // however one can forward the flush to the chained stream to give it
    // a chance to write data buffered in the chained stream.
    if( output.is() )
    {
        output->flush();
    }
}

void OMarkableOutputStream::closeOutput()
    throw ( NotConnectedException,
            BufferSizeExceededException,
            RuntimeException, std::exception)
{
    if( m_bValidStream ) {
        MutexGuard guard( m_mutex );
        // all marks must be cleared and all

        if( ! m_mapMarks.empty() )
        {
            m_mapMarks.clear();
         }
        m_nCurrentPos = m_pBuffer->getSize();
          checkMarksAndFlush();

        m_output->closeOutput();

        setOutputStream( Reference< XOutputStream > () );
        setPredecessor( Reference < XConnectable >() );
        setSuccessor( Reference< XConnectable > () );
    }
    else {
        throw NotConnectedException();
    }
}


sal_Int32 OMarkableOutputStream::createMark()
    throw ( IOException,
            RuntimeException, std::exception)
{
    MutexGuard guard( m_mutex );
    sal_Int32 nMark = m_nCurrentMark;

    m_mapMarks[nMark] = m_nCurrentPos;

    m_nCurrentMark ++;
    return nMark;
}

void OMarkableOutputStream::deleteMark(sal_Int32 Mark)
    throw( IOException,
           IllegalArgumentException,
           RuntimeException, std::exception)
{
    MutexGuard guard( m_mutex );
    map<sal_Int32,sal_Int32,less<sal_Int32> >::iterator ii = m_mapMarks.find( Mark );

    if( ii == m_mapMarks.end() ) {
        OUStringBuffer buf( 128 );
        buf.append( "MarkableOutputStream::deleteMark unknown mark (" );
        buf.append( Mark );
        buf.append( ")");
        throw IllegalArgumentException( buf.makeStringAndClear(), *this, 0);
    }
    else {
        m_mapMarks.erase( ii );
        checkMarksAndFlush();
    }
}

void OMarkableOutputStream::jumpToMark(sal_Int32 nMark)
    throw (IOException,
           IllegalArgumentException,
           RuntimeException, std::exception)
{
    MutexGuard guard( m_mutex );
    map<sal_Int32,sal_Int32,less<sal_Int32> >::iterator ii = m_mapMarks.find( nMark );

    if( ii == m_mapMarks.end() ) {
        OUStringBuffer buf( 128 );
        buf.append( "MarkableOutputStream::jumpToMark unknown mark (" );
        buf.append( nMark );
        buf.append( ")");
        throw IllegalArgumentException( buf.makeStringAndClear(), *this, 0);
    }
    else {
        m_nCurrentPos = (*ii).second;
    }
}

void OMarkableOutputStream::jumpToFurthest()
    throw (IOException,
           RuntimeException, std::exception)
{
    MutexGuard guard( m_mutex );
    m_nCurrentPos = m_pBuffer->getSize();
    checkMarksAndFlush();
}

sal_Int32 OMarkableOutputStream::offsetToMark(sal_Int32 nMark)
    throw (IOException,
           IllegalArgumentException,
           RuntimeException, std::exception)
{

    MutexGuard guard( m_mutex );
    map<sal_Int32,sal_Int32,less<sal_Int32> >::const_iterator ii = m_mapMarks.find( nMark );

    if( ii == m_mapMarks.end() )
    {
        OUStringBuffer buf( 128 );
        buf.append( "MarkableOutputStream::offsetToMark unknown mark (" );
        buf.append( nMark );
        buf.append( ")");
        throw IllegalArgumentException( buf.makeStringAndClear(), *this, 0);
    }
    return m_nCurrentPos - (*ii).second;
}



// XActiveDataSource2
void OMarkableOutputStream::setOutputStream(const Reference < XOutputStream >& aStream)
    throw (RuntimeException, std::exception)
{
    if( m_output != aStream ) {
        m_output = aStream;

        Reference < XConnectable > succ( m_output , UNO_QUERY );
        setSuccessor( succ );
    }
    m_bValidStream = m_output.is();
}

Reference< XOutputStream > OMarkableOutputStream::getOutputStream() throw (RuntimeException, std::exception)
{
    return m_output;
}



void OMarkableOutputStream::setSuccessor( const Reference< XConnectable > &r )
    throw (RuntimeException, std::exception)
{
     /// if the references match, nothing needs to be done
     if( m_succ != r ) {
         /// store the reference for later use
         m_succ = r;

         if( m_succ.is() ) {
              m_succ->setPredecessor( Reference < XConnectable > (
                  (static_cast< XConnectable *  >(this)) ) );
         }
     }
}
Reference <XConnectable > OMarkableOutputStream::getSuccessor()     throw (RuntimeException, std::exception)
{
    return m_succ;
}


// XDataSource
void OMarkableOutputStream::setPredecessor( const Reference< XConnectable > &r )
    throw (RuntimeException, std::exception)
{
    if( r != m_pred ) {
        m_pred = r;
        if( m_pred.is() ) {
            m_pred->setSuccessor( Reference < XConnectable > (
                (static_cast< XConnectable *  >(this )) ) );
        }
    }
}
Reference < XConnectable > OMarkableOutputStream::getPredecessor() throw (RuntimeException, std::exception)
{
    return m_pred;
}


// private methods

void OMarkableOutputStream::checkMarksAndFlush() throw(     NotConnectedException,
                                                            BufferSizeExceededException)
{
    map<sal_Int32,sal_Int32,less<sal_Int32> >::iterator ii;

    // find the smallest mark
    sal_Int32 nNextFound = m_nCurrentPos;
    for( ii = m_mapMarks.begin() ; ii != m_mapMarks.end() ; ++ii ) {
        if( (*ii).second <= nNextFound )  {
            nNextFound = (*ii).second;
        }
    }

    if( nNextFound ) {
        // some data must be released !
        m_nCurrentPos -= nNextFound;
        for( ii = m_mapMarks.begin() ; ii != m_mapMarks.end() ; ++ii ) {
            (*ii).second -= nNextFound;
        }

        Sequence<sal_Int8> seq(nNextFound);
        m_pBuffer->readAt( 0 , seq , nNextFound );
        m_pBuffer->forgetFromStart( nNextFound );

        // now write data through to streams
        m_output->writeBytes( seq );
    }
    else {
        // nothing to do. There is a mark or the current cursor position, that prevents
        // releasing data !
    }
}


// XServiceInfo
OUString OMarkableOutputStream::getImplementationName() throw (std::exception)
{
    return OMarkableOutputStream_getImplementationName();
}

// XServiceInfo
sal_Bool OMarkableOutputStream::supportsService(const OUString& ServiceName) throw (std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > OMarkableOutputStream::getSupportedServiceNames() throw (std::exception)
{
    return OMarkableOutputStream_getSupportedServiceNames();
}

/*------------------------
*
* external binding
*
*------------------------*/
Reference< XInterface > SAL_CALL OMarkableOutputStream_CreateInstance(
    SAL_UNUSED_PARAMETER const Reference < XComponentContext > & )
    throw(Exception)
{
    OMarkableOutputStream *p = new OMarkableOutputStream( );

    return Reference < XInterface > ( static_cast<OWeakObject *>(p) );
}

OUString    OMarkableOutputStream_getImplementationName()
{
    return OUString("com.sun.star.comp.io.stm.MarkableOutputStream");
}

Sequence<OUString> OMarkableOutputStream_getSupportedServiceNames()
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = "com.sun.star.io.MarkableOutputStream";

    return aRet;
}








// XMarkableInputStream



class OMarkableInputStream :
    public WeakImplHelper
    <
             XInputStream,
             XActiveDataSink,
             XMarkableStream,
             XConnectable,
             XServiceInfo
    >
{
public:
    OMarkableInputStream(  );
    virtual ~OMarkableInputStream();


public: // XInputStream
    virtual sal_Int32 SAL_CALL readBytes(Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
        throw ( NotConnectedException,
                BufferSizeExceededException,
                RuntimeException, std::exception) override ;
    virtual sal_Int32 SAL_CALL readSomeBytes(Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead)
        throw ( NotConnectedException,
                BufferSizeExceededException,
                RuntimeException, std::exception) override;
    virtual void SAL_CALL skipBytes(sal_Int32 nBytesToSkip)
        throw ( NotConnectedException,
                BufferSizeExceededException,
                RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL available()
        throw ( NotConnectedException,
                RuntimeException, std::exception) override;
    virtual void SAL_CALL closeInput() throw (NotConnectedException, RuntimeException, std::exception) override;

public: // XMarkable
    virtual sal_Int32 SAL_CALL createMark()
        throw (IOException, RuntimeException, std::exception) override;
    virtual void SAL_CALL deleteMark(sal_Int32 Mark)
        throw (IOException, IllegalArgumentException, RuntimeException, std::exception) override;
    virtual void SAL_CALL jumpToMark(sal_Int32 nMark)
        throw (IOException, IllegalArgumentException, RuntimeException, std::exception) override;
    virtual void SAL_CALL jumpToFurthest()
        throw (IOException, RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL offsetToMark(sal_Int32 nMark)
        throw (IOException, IllegalArgumentException,RuntimeException, std::exception) override;

public: // XActiveDataSink
    virtual void SAL_CALL setInputStream(const Reference < XInputStream > & aStream)
        throw (RuntimeException, std::exception) override;
    virtual Reference < XInputStream > SAL_CALL getInputStream()
        throw (RuntimeException, std::exception) override;

public: // XConnectable
    virtual void SAL_CALL setPredecessor(const Reference < XConnectable > & aPredecessor)
        throw (RuntimeException, std::exception) override;
    virtual Reference < XConnectable > SAL_CALL getPredecessor()
        throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setSuccessor(const Reference < XConnectable > & aSuccessor)
        throw (RuntimeException, std::exception) override;
    virtual Reference < XConnectable > SAL_CALL getSuccessor() throw (RuntimeException, std::exception) override;

public: // XServiceInfo
    OUString                     SAL_CALL getImplementationName() throw (std::exception) override;
    Sequence< OUString >         SAL_CALL getSupportedServiceNames() throw (std::exception) override;
    sal_Bool                         SAL_CALL  supportsService(const OUString& ServiceName) throw (std::exception) override;

private:
    void checkMarksAndFlush();

    Reference < XConnectable >  m_succ;
    Reference < XConnectable >  m_pred;

    Reference< XInputStream > m_input;
    bool m_bValidStream;

    MemRingBuffer *m_pBuffer;
    map<sal_Int32,sal_Int32,less< sal_Int32 > > m_mapMarks;
    sal_Int32 m_nCurrentPos;
    sal_Int32 m_nCurrentMark;

    Mutex m_mutex;
};

OMarkableInputStream::OMarkableInputStream()
    : m_bValidStream(false)
    , m_nCurrentPos(0)
    , m_nCurrentMark(0)
{
    m_pBuffer = new MemRingBuffer;
}

OMarkableInputStream::~OMarkableInputStream()
{
    if( m_pBuffer ) {
        delete m_pBuffer;
    }
}




// XInputStream

sal_Int32 OMarkableInputStream::readBytes(Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
    throw ( NotConnectedException,
            BufferSizeExceededException,
            RuntimeException, std::exception)
{
    sal_Int32 nBytesRead;

    if( m_bValidStream ) {
        MutexGuard guard( m_mutex );
        if( m_mapMarks.empty() && ! m_pBuffer->getSize() ) {
            // normal read !
            nBytesRead = m_input->readBytes( aData, nBytesToRead );
        }
        else {
            // read from buffer
            sal_Int32 nRead;

            // read enough bytes into buffer
            if( m_pBuffer->getSize() - m_nCurrentPos < nBytesToRead  ) {
                sal_Int32 nToRead = nBytesToRead - ( m_pBuffer->getSize() - m_nCurrentPos );
                nRead = m_input->readBytes( aData , nToRead );

                OSL_ASSERT( aData.getLength() == nRead );

                try
                {
                    m_pBuffer->writeAt( m_pBuffer->getSize() , aData );
                }
                catch( IRingBuffer_OutOfMemoryException & ) {
                    throw BufferSizeExceededException();
                }
                catch( IRingBuffer_OutOfBoundsException & ) {
                    throw BufferSizeExceededException();
                }

                if( nRead < nToRead ) {
                    nBytesToRead = nBytesToRead - (nToRead-nRead);
                }
            }

            OSL_ASSERT( m_pBuffer->getSize() - m_nCurrentPos >= nBytesToRead  );

            m_pBuffer->readAt( m_nCurrentPos , aData , nBytesToRead );

            m_nCurrentPos += nBytesToRead;
            nBytesRead = nBytesToRead;
        }
    }
    else {
        throw NotConnectedException(
            "MarkableInputStream::readBytes NotConnectedException",
            *this );
    }
    return nBytesRead;
}


sal_Int32 OMarkableInputStream::readSomeBytes(Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead)
    throw ( NotConnectedException,
            BufferSizeExceededException,
            RuntimeException, std::exception)
{

    sal_Int32 nBytesRead;
    if( m_bValidStream ) {
        MutexGuard guard( m_mutex );
        if( m_mapMarks.empty() && ! m_pBuffer->getSize() ) {
            // normal read !
            nBytesRead = m_input->readSomeBytes( aData, nMaxBytesToRead );
        }
        else {
            // read from buffer
            sal_Int32 nRead = 0;
            sal_Int32 nInBuffer = m_pBuffer->getSize() - m_nCurrentPos;
            sal_Int32 nAdditionalBytesToRead = Min(nMaxBytesToRead-nInBuffer,m_input->available());
            nAdditionalBytesToRead = Max(0 , nAdditionalBytesToRead );

            // read enough bytes into buffer
            if( 0 == nInBuffer ) {
                nRead = m_input->readSomeBytes( aData , nMaxBytesToRead );
            }
            else if( nAdditionalBytesToRead ) {
                nRead = m_input->readBytes( aData , nAdditionalBytesToRead );
            }

            if( nRead ) {
                aData.realloc( nRead );
                try
                {
                    m_pBuffer->writeAt( m_pBuffer->getSize() , aData );
                }
                catch( IRingBuffer_OutOfMemoryException & )
                {
                    throw BufferSizeExceededException();
                }
                catch( IRingBuffer_OutOfBoundsException &  )
                {
                    throw BufferSizeExceededException();
                }
            }

            nBytesRead = Min( nMaxBytesToRead , nInBuffer + nRead );

            // now take everything from buffer !
            m_pBuffer->readAt( m_nCurrentPos , aData , nBytesRead );

            m_nCurrentPos += nBytesRead;
        }
    }
    else
    {
        throw NotConnectedException(
            "MarkableInputStream::readSomeBytes NotConnectedException",
            *this );
    }
    return nBytesRead;


}


void OMarkableInputStream::skipBytes(sal_Int32 nBytesToSkip)
    throw ( NotConnectedException,
            BufferSizeExceededException,
            RuntimeException, std::exception)
{
    if ( nBytesToSkip < 0 )
        throw BufferSizeExceededException(
            "precondition not met: XInputStream::skipBytes: non-negative integer required!",
            *this
        );

    // this method is blocking
    Sequence<sal_Int8> seqDummy( nBytesToSkip );
    readBytes( seqDummy , nBytesToSkip );
}

sal_Int32 OMarkableInputStream::available() throw (NotConnectedException, RuntimeException, std::exception)
{
    sal_Int32 nAvail;
    if( m_bValidStream ) {
        MutexGuard guard( m_mutex );
        nAvail = m_input->available() + ( m_pBuffer->getSize() - m_nCurrentPos );
    }
    else
    {
        throw NotConnectedException(
            "MarkableInputStream::available NotConnectedException",
            *this );
    }

    return nAvail;
}


void OMarkableInputStream::closeInput() throw (NotConnectedException, RuntimeException, std::exception)
{
    if( m_bValidStream ) {
        MutexGuard guard( m_mutex );

        m_input->closeInput();

        setInputStream( Reference< XInputStream > () );
        setPredecessor( Reference< XConnectable > () );
        setSuccessor( Reference< XConnectable >() );

        delete m_pBuffer;
        m_pBuffer = 0;
        m_nCurrentPos = 0;
        m_nCurrentMark = 0;
    }
    else {
        throw NotConnectedException(
            "MarkableInputStream::closeInput NotConnectedException",
            *this );
    }
}

// XMarkable

sal_Int32 OMarkableInputStream::createMark()            throw (IOException, RuntimeException, std::exception)
{
    MutexGuard guard( m_mutex );
    sal_Int32 nMark = m_nCurrentMark;

    m_mapMarks[nMark] = m_nCurrentPos;

    m_nCurrentMark ++;
    return nMark;
}

void OMarkableInputStream::deleteMark(sal_Int32 Mark)       throw (IOException, IllegalArgumentException, RuntimeException, std::exception)
{
    MutexGuard guard( m_mutex );
    map<sal_Int32,sal_Int32,less<sal_Int32> >::iterator ii = m_mapMarks.find( Mark );

    if( ii == m_mapMarks.end() ) {
        OUStringBuffer buf( 128 );
        buf.append( "MarkableInputStream::deleteMark unknown mark (" );
        buf.append( Mark );
        buf.append( ")");
        throw IllegalArgumentException( buf.makeStringAndClear(), *this , 0 );
    }
    else {
        m_mapMarks.erase( ii );
        checkMarksAndFlush();
    }
}

void OMarkableInputStream::jumpToMark(sal_Int32 nMark)
    throw (IOException,
           IllegalArgumentException,
           RuntimeException, std::exception)
{
    MutexGuard guard( m_mutex );
    map<sal_Int32,sal_Int32,less<sal_Int32> >::iterator ii = m_mapMarks.find( nMark );

    if( ii == m_mapMarks.end() )
    {
        OUStringBuffer buf( 128 );
        buf.append( "MarkableInputStream::jumpToMark unknown mark (" );
        buf.append( nMark );
        buf.append( ")");
        throw IllegalArgumentException( buf.makeStringAndClear(), *this , 0 );
    }
    else
    {
        m_nCurrentPos = (*ii).second;
    }
}

void OMarkableInputStream::jumpToFurthest()         throw (IOException, RuntimeException, std::exception)
{
    MutexGuard guard( m_mutex );
    m_nCurrentPos = m_pBuffer->getSize();
    checkMarksAndFlush();
}

sal_Int32 OMarkableInputStream::offsetToMark(sal_Int32 nMark)
     throw (IOException,
           IllegalArgumentException,
           RuntimeException, std::exception)
{
    MutexGuard guard( m_mutex );
    map<sal_Int32,sal_Int32,less<sal_Int32> >::const_iterator ii = m_mapMarks.find( nMark );

    if( ii == m_mapMarks.end() )
    {
        OUStringBuffer buf( 128 );
        buf.append( "MarkableInputStream::offsetToMark unknown mark (" );
        buf.append( nMark );
        buf.append( ")");
        throw IllegalArgumentException( buf.makeStringAndClear(), *this , 0 );
    }
    return m_nCurrentPos - (*ii).second;
}







// XActiveDataSource
void OMarkableInputStream::setInputStream(const Reference< XInputStream > & aStream)
    throw (RuntimeException, std::exception)
{

    if( m_input != aStream ) {
        m_input = aStream;

        Reference < XConnectable >  pred( m_input , UNO_QUERY );
        setPredecessor( pred );
    }

    m_bValidStream = m_input.is();

}

Reference< XInputStream > OMarkableInputStream::getInputStream() throw (RuntimeException, std::exception)
{
    return m_input;
}



// XDataSink
void OMarkableInputStream::setSuccessor( const Reference< XConnectable > &r )
    throw (RuntimeException, std::exception)
{
     /// if the references match, nothing needs to be done
     if( m_succ != r ) {
         /// store the reference for later use
         m_succ = r;

         if( m_succ.is() ) {
              /// set this instance as the sink !
              m_succ->setPredecessor( Reference< XConnectable > (
                  (static_cast< XConnectable *  >(this)) ) );
         }
     }
}

Reference < XConnectable >  OMarkableInputStream::getSuccessor() throw (RuntimeException, std::exception)
{
    return m_succ;
}


// XDataSource
void OMarkableInputStream::setPredecessor( const Reference < XConnectable >  &r )
    throw (RuntimeException, std::exception)
{
    if( r != m_pred ) {
        m_pred = r;
        if( m_pred.is() ) {
            m_pred->setSuccessor( Reference< XConnectable > (
                (static_cast< XConnectable *  >(this)) ) );
        }
    }
}
Reference< XConnectable >  OMarkableInputStream::getPredecessor() throw (RuntimeException, std::exception)
{
    return m_pred;
}




void OMarkableInputStream::checkMarksAndFlush()
{
    map<sal_Int32,sal_Int32,less<sal_Int32> >::iterator ii;

    // find the smallest mark
    sal_Int32 nNextFound = m_nCurrentPos;
    for( ii = m_mapMarks.begin() ; ii != m_mapMarks.end() ; ++ii ) {
        if( (*ii).second <= nNextFound )  {
            nNextFound = (*ii).second;
        }
    }

    if( nNextFound ) {
        // some data must be released !
        m_nCurrentPos -= nNextFound;
        for( ii = m_mapMarks.begin() ; ii != m_mapMarks.end() ; ++ii ) {
            (*ii).second -= nNextFound;
        }

        m_pBuffer->forgetFromStart( nNextFound );

    }
    else {
        // nothing to do. There is a mark or the current cursor position, that prevents
        // releasing data !
    }
}

// XServiceInfo
OUString OMarkableInputStream::getImplementationName() throw (std::exception)
{
    return OMarkableInputStream_getImplementationName();
}

// XServiceInfo
sal_Bool OMarkableInputStream::supportsService(const OUString& ServiceName) throw (std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > OMarkableInputStream::getSupportedServiceNames() throw (std::exception)
{
    return OMarkableInputStream_getSupportedServiceNames();
}

/*------------------------
*
* external binding
*
*------------------------*/
Reference < XInterface > SAL_CALL OMarkableInputStream_CreateInstance(
    SAL_UNUSED_PARAMETER const Reference < XComponentContext > & )
    throw(Exception)
{
    OMarkableInputStream *p = new OMarkableInputStream( );
    return Reference< XInterface > ( static_cast<OWeakObject *>(p) );
}

OUString    OMarkableInputStream_getImplementationName()
{
    return OUString("com.sun.star.comp.io.stm.MarkableInputStream");
}

Sequence<OUString> OMarkableInputStream_getSupportedServiceNames()
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = "com.sun.star.io.MarkableInputStream";
    return aRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
