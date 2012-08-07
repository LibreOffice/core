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

#include <boost/unordered_map.hpp>
#include <vector>
#include <string.h>

#include <cppuhelper/weak.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase4.hxx>

#include <com/sun/star/io/XObjectInputStream.hpp>
#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XMarkableStream.hpp>
#include <com/sun/star/io/XConnectable.hpp>
#include <com/sun/star/io/UnexpectedEOFException.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

using namespace ::cppu;
using namespace ::osl;
using namespace ::std;
using namespace ::rtl;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

#include "factreg.hxx"

namespace io_stm {

class ODataInputStream :
    public WeakImplHelper4 <
                              XDataInputStream,
                              XActiveDataSink,
                              XConnectable,
                              XServiceInfo
                           >
{
public:
    ODataInputStream( )
        : m_bValidStream( sal_False )
        {
            g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
        }

    ~ODataInputStream();
public: // XInputStream
    virtual sal_Int32 SAL_CALL readBytes(Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
                                                            throw ( NotConnectedException,
                                                                        BufferSizeExceededException,
                                                                        RuntimeException);
    virtual sal_Int32 SAL_CALL readSomeBytes(Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead)
                                                            throw ( NotConnectedException,
                                                                        BufferSizeExceededException,
                                                                        RuntimeException);
    virtual void SAL_CALL skipBytes(sal_Int32 nBytesToSkip)                 throw ( NotConnectedException,
                                                                        BufferSizeExceededException,
                                                                        RuntimeException);
    virtual sal_Int32 SAL_CALL available(void)                          throw ( NotConnectedException,
                                                                        RuntimeException);
    virtual void SAL_CALL closeInput(void)                          throw ( NotConnectedException,
                                                                        RuntimeException);

public: // XDataInputStream
    virtual sal_Int8 SAL_CALL readBoolean(void) throw (IOException, RuntimeException);
    virtual sal_Int8 SAL_CALL readByte(void) throw (IOException, RuntimeException);
    virtual sal_Unicode SAL_CALL readChar(void) throw (IOException, RuntimeException);
    virtual sal_Int16 SAL_CALL readShort(void) throw (IOException, RuntimeException);
    virtual sal_Int32 SAL_CALL readLong(void) throw (IOException, RuntimeException);
    virtual sal_Int64 SAL_CALL readHyper(void) throw (IOException, RuntimeException);
    virtual float SAL_CALL readFloat(void) throw (IOException, RuntimeException);
    virtual double SAL_CALL readDouble(void) throw (IOException, RuntimeException);
    virtual OUString SAL_CALL readUTF(void) throw (IOException, RuntimeException);



public: // XActiveDataSink
    virtual void SAL_CALL setInputStream(const Reference< XInputStream > & aStream)
        throw (RuntimeException);
    virtual Reference< XInputStream > SAL_CALL getInputStream(void) throw (RuntimeException);

public: // XConnectable
    virtual void SAL_CALL setPredecessor(const Reference < XConnectable >& aPredecessor) throw (RuntimeException);
    virtual Reference < XConnectable > SAL_CALL getPredecessor(void) throw (RuntimeException);
    virtual void SAL_CALL setSuccessor(const Reference < XConnectable >& aSuccessor) throw (RuntimeException);
    virtual Reference < XConnectable > SAL_CALL getSuccessor(void) throw (RuntimeException) ;


public: // XServiceInfo
    OUString                     SAL_CALL getImplementationName() throw ();
    Sequence< OUString >         SAL_CALL getSupportedServiceNames(void) throw ();
    sal_Bool                        SAL_CALL supportsService(const OUString& ServiceName) throw ();

protected:

    Reference < XConnectable >  m_pred;
    Reference < XConnectable >  m_succ;
    Reference < XInputStream >  m_input;
    sal_Bool m_bValidStream;
};

ODataInputStream::~ODataInputStream()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

// XInputStream
sal_Int32 ODataInputStream::readBytes(Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
    throw ( NotConnectedException,
            BufferSizeExceededException,
            RuntimeException)
{
     sal_Int32 nRead;

     if( m_bValidStream )
    {
         nRead = m_input->readBytes( aData , nBytesToRead );
     }
     else
    {
         throw NotConnectedException( );
     }

     return nRead;
}

sal_Int32 ODataInputStream::readSomeBytes(Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead)
    throw ( NotConnectedException,
            BufferSizeExceededException,
            RuntimeException)
{
     sal_Int32 nRead;
     if( m_bValidStream ) {
         nRead = m_input->readSomeBytes( aData , nMaxBytesToRead );
     }
     else {
         throw NotConnectedException( );
     }

     return nRead;
}
void ODataInputStream::skipBytes(sal_Int32 nBytesToSkip)
    throw ( NotConnectedException,
            BufferSizeExceededException,
            RuntimeException)
{
     if( m_bValidStream ) {
         m_input->skipBytes( nBytesToSkip );
     }
     else
    {
         throw NotConnectedException( );
     }
}


sal_Int32 ODataInputStream::available(void)
    throw ( NotConnectedException,
            RuntimeException)
{
     sal_Int32 nAvail;

     if( m_bValidStream )
    {
         nAvail = m_input->available( );
     }
     else
    {
         throw NotConnectedException( );
     }
     return nAvail;
}

void ODataInputStream::closeInput(void )
    throw ( NotConnectedException,
            RuntimeException)
{
     if( m_bValidStream ) {
         m_input->closeInput( );
         setInputStream( Reference< XInputStream > () );
         setPredecessor( Reference < XConnectable >() );
         setSuccessor( Reference < XConnectable >() );
         m_bValidStream = sal_False;
     }
     else
    {
         throw NotConnectedException( );
     }
}




//== XDataInputStream ===========================================

// XDataInputStream
sal_Int8 ODataInputStream::readBoolean(void) throw (IOException, RuntimeException)
{
    return readByte();
}

sal_Int8 ODataInputStream::readByte(void)    throw (IOException, RuntimeException)
{
    Sequence<sal_Int8> aTmp(1);
    if( 1 != readBytes( aTmp, 1 ) )
    {
        throw UnexpectedEOFException();
    }
    return aTmp.getArray()[0];
}

sal_Unicode ODataInputStream::readChar(void) throw (IOException, RuntimeException)
{
    Sequence<sal_Int8> aTmp(2);
    if( 2 != readBytes( aTmp, 2 ) )
    {
        throw UnexpectedEOFException();
    }

    const sal_uInt8 * pBytes = ( const sal_uInt8 * )aTmp.getConstArray();
    return ((sal_Unicode)pBytes[0] << 8) + pBytes[1];
}

sal_Int16 ODataInputStream::readShort(void) throw (IOException, RuntimeException)
{
    Sequence<sal_Int8> aTmp(2);
    if( 2 != readBytes( aTmp, 2 ) )
    {
        throw UnexpectedEOFException();
    }

    const sal_uInt8 * pBytes = ( const sal_uInt8 * ) aTmp.getConstArray();
    return ((sal_Int16)pBytes[0] << 8) + pBytes[1];
}


sal_Int32 ODataInputStream::readLong(void) throw (IOException, RuntimeException)
{
    Sequence<sal_Int8> aTmp(4);
    if( 4 != readBytes( aTmp, 4 ) )
    {
        throw UnexpectedEOFException( );
    }

    const sal_uInt8 * pBytes = ( const sal_uInt8 * ) aTmp.getConstArray();
    return ((sal_Int32)pBytes[0] << 24) + ((sal_Int32)pBytes[1] << 16) + ((sal_Int32)pBytes[2] << 8) + pBytes[3];
}


sal_Int64 ODataInputStream::readHyper(void) throw (IOException, RuntimeException)
{
    Sequence<sal_Int8> aTmp(8);
    if( 8 != readBytes( aTmp, 8 ) )
    {
        throw UnexpectedEOFException( );
    }

    const sal_uInt8 * pBytes = ( const sal_uInt8 * ) aTmp.getConstArray();
    return
        (((sal_Int64)pBytes[0]) << 56) +
        (((sal_Int64)pBytes[1]) << 48) +
        (((sal_Int64)pBytes[2]) << 40) +
        (((sal_Int64)pBytes[3]) << 32) +
        (((sal_Int64)pBytes[4]) << 24) +
        (((sal_Int64)pBytes[5]) << 16) +
        (((sal_Int64)pBytes[6]) << 8) +
        pBytes[7];
}

float ODataInputStream::readFloat(void) throw (IOException, RuntimeException)
{
    union { float f; sal_uInt32 n; } a;
    a.n = readLong();
    return a.f;
}

double ODataInputStream::readDouble(void) throw (IOException, RuntimeException)
{
    sal_uInt32 n = 1;
    union { double d; struct { sal_uInt32 n1; sal_uInt32 n2; } ad; } a;
    if( *(sal_uInt8 *)&n == 1 )
    {
        // little endian
        a.ad.n2 = readLong();
        a.ad.n1 = readLong();
    }
    else
    {
        // big endian
        a.ad.n1 = readLong();
        a.ad.n2 = readLong();
    }
    return a.d;
}

OUString ODataInputStream::readUTF(void) throw (IOException, RuntimeException)
{
    sal_uInt16              nShortLen = (sal_uInt16)readShort();
    sal_Int32               nUTFLen;

    if( ((sal_uInt16)0xffff) == nShortLen )
    {
        // is interpreted as a sign, that string is longer than 64k
        // incompatible to older XDataInputStream-routines, when strings are exactly 64k
        nUTFLen = readLong();
    }
    else
    {
        nUTFLen = ( sal_Int32 ) nShortLen;
    }

    Sequence<sal_Unicode>   aBuffer( nUTFLen );
    sal_Unicode *           pStr = aBuffer.getArray();

    sal_Int32 nCount = 0;
    sal_Int32 nStrLen = 0;
    while( nCount < nUTFLen )
    {
        sal_uInt8 c = (sal_uInt8)readByte();
        sal_uInt8 char2, char3;
        switch( c >> 4 )
        {
            case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
                // 0xxxxxxx
                nCount++;
                pStr[nStrLen++] = c;
                break;

            case 12: case 13:
                // 110x xxxx   10xx xxxx
                nCount += 2;
                if( ! ( nCount <= nUTFLen ) )
                {
                    throw WrongFormatException( );
                }

                char2 = (sal_uInt8)readByte();
                if( ! ( (char2 & 0xC0) == 0x80 ) )
                {
                    throw WrongFormatException( );
                }

                pStr[nStrLen++] = (sal_Unicode(c & 0x1F) << 6) | (char2 & 0x3F);
                break;

            case 14:
            // 1110 xxxx  10xx xxxx  10xx xxxx
                nCount += 3;
                if( !( nCount <= nUTFLen) )
                {
                    throw WrongFormatException( );
                }

                char2 = (sal_uInt8)readByte();
                char3 = (sal_uInt8)readByte();

                if( (((char2 & 0xC0) != 0x80) || ((char3 & 0xC0) != 0x80)) ) {
                    throw WrongFormatException( );
                }
                pStr[nStrLen++] = (sal_Unicode(c & 0x0F) << 12) |
                                (sal_Unicode(char2 & 0x3F) << 6) |
                                (char3 & 0x3F);
                break;

            default:
                // 10xx xxxx,  1111 xxxx
                throw WrongFormatException();
                //throw new UTFDataFormatException();
        }
    }
    return OUString( pStr, nStrLen );
}



// XActiveDataSource
void ODataInputStream::setInputStream(const Reference< XInputStream > & aStream)
    throw (RuntimeException)
{

    if( m_input != aStream ) {
        m_input = aStream;

        Reference < XConnectable > pred( m_input , UNO_QUERY );
        setPredecessor( pred );
    }

    m_bValidStream = m_input.is();
}

Reference< XInputStream > ODataInputStream::getInputStream(void) throw (RuntimeException)
{
    return m_input;
}



// XDataSink
void ODataInputStream::setSuccessor( const Reference < XConnectable > &r ) throw (RuntimeException)
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

Reference < XConnectable > ODataInputStream::getSuccessor() throw (RuntimeException)
{
    return m_succ;
}


// XDataSource
void ODataInputStream::setPredecessor( const Reference < XConnectable > &r )
    throw (RuntimeException)
{
    if( r != m_pred ) {
        m_pred = r;
        if( m_pred.is() ) {
            m_pred->setSuccessor( Reference< XConnectable > (
                (static_cast< XConnectable *  >(this)) ) );
        }
    }
}
Reference < XConnectable > ODataInputStream::getPredecessor() throw (RuntimeException)
{
    return m_pred;
}

// XServiceInfo
OUString ODataInputStream::getImplementationName() throw ()
{
    return ODataInputStream_getImplementationName();
}

// XServiceInfo
sal_Bool ODataInputStream::supportsService(const OUString& ServiceName) throw ()
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

// XServiceInfo
Sequence< OUString > ODataInputStream::getSupportedServiceNames(void) throw ()
{
    return ODataInputStream_getSupportedServiceNames();
}

/***
*
* registration information
*
*
****/

Reference< XInterface > SAL_CALL ODataInputStream_CreateInstance(
    SAL_UNUSED_PARAMETER const Reference < XComponentContext > & )
    throw( Exception)
{
    ODataInputStream *p = new ODataInputStream;
    return Reference< XInterface > ( (OWeakObject * ) p );
}

OUString ODataInputStream_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.io.stm.DataInputStream" ) );
}

Sequence<OUString> ODataInputStream_getSupportedServiceNames(void)
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.io.DataInputStream" ) );
    return aRet;
}




class ODataOutputStream :
    public WeakImplHelper4 <
             XDataOutputStream,
             XActiveDataSource,
             XConnectable,
              XServiceInfo >
{
public:
    ODataOutputStream()
        : m_bValidStream( sal_False )
        {
            g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
        }
    ~ODataOutputStream();

public: // XOutputStream
    virtual void SAL_CALL writeBytes(const Sequence< sal_Int8 >& aData)
        throw ( NotConnectedException,
                BufferSizeExceededException,
                RuntimeException);
    virtual void SAL_CALL flush(void)
        throw ( NotConnectedException,
                BufferSizeExceededException,
                RuntimeException);
    virtual void SAL_CALL closeOutput(void)
        throw ( NotConnectedException,
                BufferSizeExceededException,
                RuntimeException);

public: // XDataOutputStream
    virtual void SAL_CALL writeBoolean(sal_Bool Value) throw (IOException, RuntimeException);
    virtual void SAL_CALL writeByte(sal_Int8 Value) throw (IOException, RuntimeException);
    virtual void SAL_CALL writeChar(sal_Unicode Value) throw (IOException, RuntimeException);
    virtual void SAL_CALL writeShort(sal_Int16 Value) throw (IOException, RuntimeException);
    virtual void SAL_CALL writeLong(sal_Int32 Value) throw (IOException, RuntimeException);
    virtual void SAL_CALL writeHyper(sal_Int64 Value) throw (IOException, RuntimeException);
    virtual void SAL_CALL writeFloat(float Value) throw (IOException, RuntimeException);
    virtual void SAL_CALL writeDouble(double Value) throw (IOException, RuntimeException);
    virtual void SAL_CALL writeUTF(const OUString& Value) throw (IOException, RuntimeException);

public: // XActiveDataSource
    virtual void SAL_CALL setOutputStream(const Reference< XOutputStream > & aStream)
        throw (RuntimeException);
    virtual Reference < XOutputStream > SAL_CALL getOutputStream(void) throw (RuntimeException);

public: // XConnectable
    virtual void SAL_CALL setPredecessor(const Reference < XConnectable >& aPredecessor)
        throw (RuntimeException);
    virtual Reference < XConnectable > SAL_CALL getPredecessor(void)
        throw (RuntimeException);
    virtual void SAL_CALL setSuccessor(const Reference < XConnectable >& aSuccessor)
        throw (RuntimeException);
    virtual Reference < XConnectable > SAL_CALL getSuccessor(void)
        throw (RuntimeException);

public: // XServiceInfo
    OUString                     SAL_CALL getImplementationName() throw ();
    Sequence< OUString >         SAL_CALL getSupportedServiceNames(void) throw ();
    sal_Bool                     SAL_CALL supportsService(const OUString& ServiceName) throw ();

protected:
    Reference < XConnectable >  m_succ;
    Reference < XConnectable >  m_pred;
    Reference<  XOutputStream > m_output;
    sal_Bool m_bValidStream;
};

ODataOutputStream::~ODataOutputStream()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}


// XOutputStream
void ODataOutputStream::writeBytes(const Sequence< sal_Int8 >& aData)
    throw ( NotConnectedException,
            BufferSizeExceededException,
            RuntimeException)
{
    if( m_bValidStream )
    {
        m_output->writeBytes( aData );
    }
    else {
        throw NotConnectedException( );
    }
}

void ODataOutputStream::flush(void)
    throw ( NotConnectedException,
            BufferSizeExceededException,
            RuntimeException)
{
    if( m_bValidStream )
    {
        m_output->flush();
    }
    else
    {
        throw NotConnectedException();
    }

}


void ODataOutputStream::closeOutput(void)
    throw ( NotConnectedException,
            BufferSizeExceededException,
            RuntimeException)
{
    if( m_bValidStream )
    {
        m_output->closeOutput();
        setOutputStream( Reference< XOutputStream > () );
        setPredecessor( Reference < XConnectable >() );
        setSuccessor( Reference < XConnectable >() );
    }
    else
    {
        throw NotConnectedException();
    }
}

// XDataOutputStream
void ODataOutputStream::writeBoolean(sal_Bool Value)
    throw ( IOException,
            RuntimeException)
{
    if( Value )
    {
        writeByte( 1 );
    }
    else
    {
        writeByte( 0 );
    }
}


void ODataOutputStream::writeByte(sal_Int8 Value)
    throw ( IOException,
            RuntimeException)
{
    Sequence<sal_Int8> aTmp( 1 );
    aTmp.getArray()[0] = Value;
    writeBytes( aTmp );
}

void ODataOutputStream::writeChar(sal_Unicode Value)
    throw ( IOException,
            RuntimeException)
{
    Sequence<sal_Int8> aTmp( 2 );
    sal_Int8 * pBytes = ( sal_Int8 * ) aTmp.getArray();
    pBytes[0] = sal_Int8(Value >> 8);
    pBytes[1] = sal_Int8(Value);
    writeBytes( aTmp );
}


void ODataOutputStream::writeShort(sal_Int16 Value)
    throw ( IOException,
            RuntimeException)
{
    Sequence<sal_Int8> aTmp( 2 );
    sal_Int8 * pBytes = aTmp.getArray();
    pBytes[0] = sal_Int8(Value >> 8);
    pBytes[1] = sal_Int8(Value);
    writeBytes( aTmp );
}

void ODataOutputStream::writeLong(sal_Int32 Value)
    throw ( IOException,
            RuntimeException)
{
    Sequence<sal_Int8> aTmp( 4 );
    sal_Int8 * pBytes = aTmp.getArray();
    pBytes[0] = sal_Int8(Value >> 24);
    pBytes[1] = sal_Int8(Value >> 16);
    pBytes[2] = sal_Int8(Value >> 8);
    pBytes[3] = sal_Int8(Value);
    writeBytes( aTmp );
}

void ODataOutputStream::writeHyper(sal_Int64 Value)
    throw ( IOException,
            RuntimeException)
{
    Sequence<sal_Int8> aTmp( 8 );
    sal_Int8 * pBytes = aTmp.getArray();
    pBytes[0] = sal_Int8(Value >> 56);
    pBytes[1] = sal_Int8(Value >> 48);
    pBytes[2] = sal_Int8(Value >> 40);
    pBytes[3] = sal_Int8(Value >> 32);
    pBytes[4] = sal_Int8(Value >> 24);
    pBytes[5] = sal_Int8(Value >> 16);
    pBytes[6] = sal_Int8(Value >> 8);
    pBytes[7] = sal_Int8(Value);
    writeBytes( aTmp );
}


void ODataOutputStream::writeFloat(float Value)
    throw ( IOException,
            RuntimeException)
{
    union { float f; sal_uInt32 n; } a;
    a.f = Value;
    writeLong( a.n );
}

void ODataOutputStream::writeDouble(double Value)
    throw ( IOException,
            RuntimeException)
{
    sal_uInt32 n = 1;
    union { double d; struct { sal_uInt32 n1; sal_uInt32 n2; } ad; } a;
    a.d = Value;
    if( *(sal_Int8 *)&n == 1 )
    {
        // little endian
        writeLong( a.ad.n2 );
        writeLong( a.ad.n1 );
    }
    else
    {
        // big endian
        writeLong( a.ad.n1 );
        writeLong( a.ad.n2 );
    }
}

void ODataOutputStream::writeUTF(const OUString& Value)
    throw ( IOException,
            RuntimeException)
{
    sal_Int32 nStrLen = Value.getLength();
    const sal_Unicode * pStr = Value.getStr();
    sal_Int32 nUTFLen = 0;
    sal_Int32 i;

    for( i = 0 ; i < nStrLen ; i++ )
    {
        sal_uInt16 c = pStr[i];
        if( (c >= 0x0001) && (c <= 0x007F) )
        {
            nUTFLen++;
        }
        else if( c > 0x07FF )
        {
            nUTFLen += 3;
        }
        else
        {
            nUTFLen += 2;
        }
    }


    // compatibility mode for older implementations, where it was not possible
    // to write blocks bigger than 64 k. Note that there is a tradeoff. Blocks,
    // that are exactly 64k long can not be read by older routines when written
    // with these routines and the other way round !!!!!
    if( nUTFLen >= 0xFFFF ) {
        writeShort( (sal_Int16)-1 );
        writeLong( nUTFLen );
    }
    else {
        writeShort( ((sal_uInt16)nUTFLen) );
    }
    for( i = 0 ; i < nStrLen ; i++ )
    {
        sal_uInt16 c = pStr[i];
        if( (c >= 0x0001) && (c <= 0x007F) )
        {
            writeByte(sal_Int8(c));
        }
        else if( c > 0x07FF )
        {
            writeByte(sal_Int8(0xE0 | ((c >> 12) & 0x0F)));
            writeByte(sal_Int8(0x80 | ((c >>  6) & 0x3F)));
            writeByte(sal_Int8(0x80 | ((c >>  0) & 0x3F)));
        }
        else
        {
            writeByte(sal_Int8(0xC0 | ((c >>  6) & 0x1F)));
            writeByte(sal_Int8(0x80 | ((c >>  0) & 0x3F)));
        }
    }
}

// XActiveDataSource
void ODataOutputStream::setOutputStream(const Reference< XOutputStream > & aStream)
    throw (RuntimeException)
{
    if( m_output != aStream ) {
        m_output = aStream;
        m_bValidStream = m_output.is();

        Reference < XConnectable > succ( m_output , UNO_QUERY );
        setSuccessor( succ );
    }
}

Reference< XOutputStream > ODataOutputStream::getOutputStream(void)
    throw (RuntimeException)
{
    return m_output;
}




// XDataSink
void ODataOutputStream::setSuccessor( const Reference < XConnectable > &r )
    throw (RuntimeException)
{
     /// if the references match, nothing needs to be done
     if( m_succ != r )
     {
         /// store the reference for later use
         m_succ = r;

         if( m_succ.is() )
         {
              /// set this instance as the sink !
              m_succ->setPredecessor( Reference < XConnectable > (
                  (static_cast< XConnectable *  >(this)) ));
         }
     }
}
Reference < XConnectable > ODataOutputStream::getSuccessor()    throw (RuntimeException)
{
    return m_succ;
}


// XDataSource
void ODataOutputStream::setPredecessor( const Reference < XConnectable > &r )   throw (RuntimeException)
{
    if( r != m_pred ) {
        m_pred = r;
        if( m_pred.is() ) {
            m_pred->setSuccessor( Reference< XConnectable > (
                (static_cast< XConnectable *  >(this)) ));
        }
    }
}
Reference < XConnectable > ODataOutputStream::getPredecessor()  throw (RuntimeException)
{
    return m_pred;
}



// XServiceInfo
OUString ODataOutputStream::getImplementationName() throw ()
{
    return ODataOutputStream_getImplementationName();
}

// XServiceInfo
sal_Bool ODataOutputStream::supportsService(const OUString& ServiceName) throw ()
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

// XServiceInfo
Sequence< OUString > ODataOutputStream::getSupportedServiceNames(void) throw ()
{
    return ODataOutputStream_getSupportedServiceNames();
}




Reference< XInterface > SAL_CALL ODataOutputStream_CreateInstance(
    SAL_UNUSED_PARAMETER const Reference < XComponentContext > & )
    throw(Exception)
{
    ODataOutputStream *p = new ODataOutputStream;
    Reference< XInterface > xService = *p;
    return xService;
}


OUString ODataOutputStream_getImplementationName()
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.io.stm.DataOutputStream" ) );
}

Sequence<OUString> ODataOutputStream_getSupportedServiceNames(void)
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] =  OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.io.DataOutputStream" ) );
    return aRet;
}

//--------------------------------------
struct equalObjectContainer_Impl
{
    sal_Int32 operator()(const Reference< XInterface > & s1,
                         const Reference< XInterface > & s2) const
      {
        return s1 == s2;
    }
};

//-----------------------------------------------------------------------------
struct hashObjectContainer_Impl
{
    size_t operator()(const Reference< XInterface > & xRef) const
    {
        return (size_t)xRef.get();
    }
};

typedef boost::unordered_map
<
    Reference< XInterface >,
    sal_Int32,
    hashObjectContainer_Impl,
    equalObjectContainer_Impl
> ObjectContainer_Impl;

class OObjectOutputStream:
    public ImplInheritanceHelper2<
            ODataOutputStream, /* parent */
            XObjectOutputStream, XMarkableStream >
{
public:
    OObjectOutputStream()
        : m_nMaxId(0) ,
          m_bValidMarkable(sal_False)
        {
            g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
        }

    ~OObjectOutputStream();

public:
    // XOutputStream
    virtual void SAL_CALL writeBytes(const Sequence< sal_Int8 >& aData)
        throw ( NotConnectedException,
                BufferSizeExceededException,
                RuntimeException)
        { ODataOutputStream::writeBytes( aData ); }

    virtual void SAL_CALL flush(void)
        throw ( NotConnectedException,
                BufferSizeExceededException,
                RuntimeException)
        { ODataOutputStream::flush(); }

    virtual void SAL_CALL closeOutput(void)
        throw ( NotConnectedException,
                BufferSizeExceededException,
                RuntimeException)
        { ODataOutputStream::closeOutput(); }

public:
    // XDataOutputStream
    virtual void SAL_CALL writeBoolean(sal_Bool Value) throw (IOException, RuntimeException)
                { ODataOutputStream::writeBoolean( Value ); }
    virtual void SAL_CALL writeByte(sal_Int8 Value) throw (IOException, RuntimeException)
                { ODataOutputStream::writeByte( Value ); }
    virtual void SAL_CALL writeChar(sal_Unicode Value) throw (IOException, RuntimeException)
                { ODataOutputStream::writeChar( Value ); }
    virtual void SAL_CALL writeShort(sal_Int16 Value) throw (IOException, RuntimeException)
                { ODataOutputStream::writeShort( Value ); }
    virtual void SAL_CALL writeLong(sal_Int32 Value) throw (IOException, RuntimeException)
                { ODataOutputStream::writeLong( Value ); }
    virtual void SAL_CALL writeHyper(sal_Int64 Value) throw (IOException, RuntimeException)
                { ODataOutputStream::writeHyper( Value ); }
    virtual void SAL_CALL writeFloat(float Value) throw (IOException, RuntimeException)
                { ODataOutputStream::writeFloat( Value ); }
    virtual void SAL_CALL writeDouble(double Value) throw (IOException, RuntimeException)
                { ODataOutputStream::writeDouble( Value ); }
    virtual void SAL_CALL writeUTF(const OUString& Value) throw (IOException, RuntimeException)
                { ODataOutputStream::writeUTF( Value );}

    // XObjectOutputStream
        virtual void SAL_CALL writeObject( const Reference< XPersistObject > & r ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

public: // XMarkableStream
    virtual sal_Int32 SAL_CALL createMark(void)                 throw (IOException, RuntimeException);
    virtual void SAL_CALL deleteMark(sal_Int32 Mark)            throw (IOException, IllegalArgumentException, RuntimeException);
    virtual void SAL_CALL jumpToMark(sal_Int32 nMark)       throw (IOException, IllegalArgumentException, RuntimeException);
    virtual void SAL_CALL jumpToFurthest(void)          throw (IOException, RuntimeException);
    virtual sal_Int32 SAL_CALL offsetToMark(sal_Int32 nMark)
        throw (IOException, IllegalArgumentException, RuntimeException);

public: // XServiceInfo
    OUString                   SAL_CALL   getImplementationName() throw ();
    Sequence< OUString >       SAL_CALL   getSupportedServiceNames(void) throw ();
    sal_Bool                   SAL_CALL   supportsService(const OUString& ServiceName) throw ();

private:
    void connectToMarkable();
private:
    ObjectContainer_Impl                m_mapObject;
    sal_Int32                           m_nMaxId;
    Reference< XMarkableStream >        m_rMarkable;
    sal_Bool                            m_bValidMarkable;
};

OObjectOutputStream::~OObjectOutputStream()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

void OObjectOutputStream::writeObject( const Reference< XPersistObject > & xPObj ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{

    connectToMarkable();
    sal_Bool bWriteObj = sal_False;
    // create Mark to write length of info
    sal_uInt32 nInfoLenMark = m_rMarkable->createMark();

    // length of the info data (is later rewritten)
    OObjectOutputStream::writeShort( 0 );

    // write the object identifier
    if( xPObj.is() )
    {
        Reference< XInterface > rX( xPObj , UNO_QUERY );

        ObjectContainer_Impl::const_iterator aIt
            = m_mapObject.find( rX );
        if( aIt == m_mapObject.end() )
        {
            // insert new object in hash table
            m_mapObject[ rX ] = ++m_nMaxId;
            ODataOutputStream::writeLong( m_nMaxId );
            ODataOutputStream::writeUTF( xPObj->getServiceName() );
            bWriteObj = sal_True;
        }
        else
        {
            ODataOutputStream::writeLong( (*aIt).second );
            OUString aName;
            ODataOutputStream::writeUTF( aName );
        }
    }
    else
    {
        ODataOutputStream::writeLong( 0 );
        OUString aName;
        ODataOutputStream::writeUTF( aName );
    }

    sal_uInt32 nObjLenMark = m_rMarkable->createMark();
    ODataOutputStream::writeLong( 0 );

    sal_Int32 nInfoLen = m_rMarkable->offsetToMark( nInfoLenMark );
    m_rMarkable->jumpToMark( nInfoLenMark );
    // write length of the info data
    ODataOutputStream::writeShort( (sal_Int16)nInfoLen );
    // jump to the end of the stream
    m_rMarkable->jumpToFurthest();

    if( bWriteObj )
        xPObj->write( Reference< XObjectOutputStream > (
            (static_cast< XObjectOutputStream *  >(this)) ) );

    sal_Int32 nObjLen = m_rMarkable->offsetToMark( nObjLenMark ) -4;
    m_rMarkable->jumpToMark( nObjLenMark );
    // write length of the info data
    ODataOutputStream::writeLong( nObjLen );
    // jump to the end of the stream
    m_rMarkable->jumpToFurthest();

    m_rMarkable->deleteMark( nObjLenMark );
    m_rMarkable->deleteMark( nInfoLenMark );
}



void OObjectOutputStream::connectToMarkable(void)
{
    if( ! m_bValidMarkable ) {
        if( ! m_bValidStream )
        {
            throw NotConnectedException();
        }

        // find the markable stream !
        Reference< XInterface > rTry(m_output);
        while( sal_True ) {
            if( ! rTry.is() )
            {
                throw NotConnectedException();
            }
            Reference < XMarkableStream > markable( rTry , UNO_QUERY );
            if( markable.is() )
            {
                m_rMarkable = markable;
                break;
            }
            Reference < XActiveDataSource > source( rTry , UNO_QUERY );
            rTry = source;
        }
        m_bValidMarkable = sal_True;
    }
}


sal_Int32 OObjectOutputStream::createMark(void)
    throw (IOException, RuntimeException)
{
    connectToMarkable();    // throws an exception, if a markable is not connected !

    return m_rMarkable->createMark();
}

void OObjectOutputStream::deleteMark(sal_Int32 Mark)
    throw (IOException, IllegalArgumentException, RuntimeException)
{
    if( ! m_bValidMarkable )
    {
        throw NotConnectedException();
    }
    m_rMarkable->deleteMark( Mark );
}

void OObjectOutputStream::jumpToMark(sal_Int32 nMark)
    throw (IOException, IllegalArgumentException, RuntimeException)
{
    if( ! m_bValidMarkable )
    {
        throw NotConnectedException();
    }
    m_rMarkable->jumpToMark( nMark );
}


void OObjectOutputStream::jumpToFurthest(void)
    throw (IOException, RuntimeException)
{
    connectToMarkable();
    m_rMarkable->jumpToFurthest();
}

sal_Int32 OObjectOutputStream::offsetToMark(sal_Int32 nMark)
    throw (IOException, IllegalArgumentException, RuntimeException)
{
    if( ! m_bValidMarkable )
    {
        throw NotConnectedException();
    }
    return m_rMarkable->offsetToMark( nMark );
}




Reference< XInterface > SAL_CALL OObjectOutputStream_CreateInstance(
    SAL_UNUSED_PARAMETER const Reference < XComponentContext > & )
    throw(Exception)
{
    OObjectOutputStream *p = new OObjectOutputStream;
    return  Reference< XInterface > ( (static_cast< OWeakObject *  >(p)) );
}

OUString OObjectOutputStream_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.io.stm.ObjectOutputStream" ) );
}

Sequence<OUString> OObjectOutputStream_getSupportedServiceNames(void)
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.io.ObjectOutputStream" ) );
    return aRet;
}

// XServiceInfo
OUString OObjectOutputStream::getImplementationName() throw ()
{
    return ODataInputStream_getImplementationName();
}

// XServiceInfo
sal_Bool OObjectOutputStream::supportsService(const OUString& ServiceName) throw ()
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

// XServiceInfo
Sequence< OUString > OObjectOutputStream::getSupportedServiceNames(void) throw ()
{
    return OObjectOutputStream_getSupportedServiceNames();
}

class OObjectInputStream:
    public ImplInheritanceHelper2<
        ODataInputStream, /* parent */
        XObjectInputStream, XMarkableStream >
{
public:
    OObjectInputStream( const Reference < XComponentContext > &r)
        : m_rSMgr( r->getServiceManager() )
        , m_rCxt( r )
        , m_bValidMarkable(sal_False)
        {
            g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
        }
    ~OObjectInputStream();

public: // XInputStream
    virtual sal_Int32 SAL_CALL readBytes(Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
        throw ( NotConnectedException,
                BufferSizeExceededException,
                RuntimeException)
        { return ODataInputStream::readBytes( aData , nBytesToRead ); }

    virtual sal_Int32 SAL_CALL readSomeBytes(Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead)
        throw ( NotConnectedException,
                BufferSizeExceededException,
                RuntimeException)
        { return ODataInputStream::readSomeBytes( aData, nMaxBytesToRead ); }

    virtual void SAL_CALL skipBytes(sal_Int32 nBytesToSkip)
        throw ( NotConnectedException,
                BufferSizeExceededException,
                RuntimeException)
        { ODataInputStream::skipBytes( nBytesToSkip ); }

    virtual sal_Int32 SAL_CALL available(void)
        throw ( NotConnectedException,
                RuntimeException)
        { return ODataInputStream::available(); }

    virtual void SAL_CALL closeInput(void)
        throw ( NotConnectedException,
                RuntimeException)
        { ODataInputStream::closeInput(); }

public: // XDataInputStream
    virtual sal_Int8 SAL_CALL readBoolean(void) throw (IOException, RuntimeException)
                { return ODataInputStream::readBoolean(); }
    virtual sal_Int8 SAL_CALL readByte(void) throw (IOException, RuntimeException)
                { return ODataInputStream::readByte(); }
    virtual sal_Unicode SAL_CALL readChar(void) throw (IOException, RuntimeException)
                { return ODataInputStream::readChar(); }
    virtual sal_Int16 SAL_CALL readShort(void) throw (IOException, RuntimeException)
                { return ODataInputStream::readShort(); }
    virtual sal_Int32 SAL_CALL readLong(void) throw (IOException, RuntimeException)
                { return ODataInputStream::readLong(); }
    virtual sal_Int64 SAL_CALL readHyper(void) throw (IOException, RuntimeException)
                { return ODataInputStream::readHyper(); }
    virtual float SAL_CALL readFloat(void) throw (IOException, RuntimeException)
                { return ODataInputStream::readFloat(); }
    virtual double SAL_CALL readDouble(void) throw (IOException, RuntimeException)
        { return ODataInputStream::readDouble(); }
    virtual OUString SAL_CALL readUTF(void) throw (IOException, RuntimeException)
                { return ODataInputStream::readUTF(); }

public: // XObjectInputStream
        virtual Reference< XPersistObject > SAL_CALL readObject( ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

public: // XMarkableStream
    virtual sal_Int32 SAL_CALL createMark(void)
        throw (IOException, RuntimeException);
    virtual void SAL_CALL deleteMark(sal_Int32 Mark)            throw (IOException, IllegalArgumentException, RuntimeException);
    virtual void SAL_CALL jumpToMark(sal_Int32 nMark)       throw (IOException, IllegalArgumentException, RuntimeException);
    virtual void SAL_CALL jumpToFurthest(void)          throw (IOException, RuntimeException);
    virtual sal_Int32 SAL_CALL offsetToMark(sal_Int32 nMark)
        throw (IOException, IllegalArgumentException, RuntimeException);

public: // XServiceInfo
    OUString                     SAL_CALL getImplementationName() throw ();
    Sequence< OUString >         SAL_CALL getSupportedServiceNames(void) throw ();
    sal_Bool                     SAL_CALL supportsService(const OUString& ServiceName) throw ();

private:
    void connectToMarkable();
private:
    Reference < XMultiComponentFactory > m_rSMgr;
    Reference < XComponentContext >     m_rCxt;
    sal_Bool                m_bValidMarkable;
    Reference < XMarkableStream > m_rMarkable;
    vector < Reference<  XPersistObject > > m_aPersistVector;

};

OObjectInputStream::~OObjectInputStream()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

Reference< XPersistObject >  OObjectInputStream::readObject() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    // check if chain contains a XMarkableStream
    connectToMarkable();

    Reference< XPersistObject > xLoadedObj;

    // create Mark to skip newer versions
    sal_uInt32 nMark = m_rMarkable->createMark();
    // length of the data
    sal_Int32 nLen = (sal_uInt16) ODataInputStream::readShort();
    if( nLen < 0xc )
    {
        throw WrongFormatException();
    }

    // read the object identifier
    sal_uInt32 nId = readLong();

    // the name of the persist model
    // MM ???
    OUString aName = readUTF();

    // Read the length of the object
    sal_Int32 nObjLen = readLong();
    if( ( 0 == nId && 0 != nObjLen ) )
    {
        throw WrongFormatException();
    }

    // skip data of new version
    skipBytes( nLen - m_rMarkable->offsetToMark( nMark ) );

    sal_Bool bLoadSuccesfull = sal_True;
    if( nId )
    {
        if( !aName.isEmpty() )
        {
            // load the object
            Reference< XInterface > x = m_rSMgr->createInstanceWithContext( aName, m_rCxt );
            xLoadedObj = Reference< XPersistObject >( x, UNO_QUERY );
            if( xLoadedObj.is() )
            {
                sal_uInt32 nSize = m_aPersistVector.size();
                if( nSize <= nId )
                {
                    // grow to the right size
                    Reference< XPersistObject > xEmpty;
                    m_aPersistVector.insert( m_aPersistVector.end(), (long)(nId - nSize + 1), xEmpty );
                }

                m_aPersistVector[nId] = xLoadedObj;
                xLoadedObj->read( Reference< XObjectInputStream >(
                    (static_cast< XObjectInputStream * >(this)) ) );
            }
            else
            {
                // no service with this name could be instantiated
                bLoadSuccesfull = sal_False;
            }
        }
        else {
            if( m_aPersistVector.size() < nId )
            {
                // id unknown, load failure !
                bLoadSuccesfull = sal_False;
            }
            else
            {
                // Object has alread been read,
                xLoadedObj = m_aPersistVector[nId];
            }
        }
    }

    // skip to the position behind the object
    skipBytes( nObjLen + nLen - m_rMarkable->offsetToMark( nMark ) );
    m_rMarkable->deleteMark( nMark );

    if( ! bLoadSuccesfull )
    {
        throw WrongFormatException();
    }
    return xLoadedObj;
}


void OObjectInputStream::connectToMarkable()
{
    if( ! m_bValidMarkable ) {
        if( ! m_bValidStream )
        {
            throw NotConnectedException( );
        }

        // find the markable stream !
        Reference< XInterface > rTry(m_input);
        while( sal_True ) {
            if( ! rTry.is() )
            {
                throw NotConnectedException( );
            }
            Reference<  XMarkableStream > markable( rTry , UNO_QUERY );
            if( markable.is() )
            {
                m_rMarkable = markable;
                break;
            }
            Reference < XActiveDataSink > sink( rTry , UNO_QUERY );
            rTry = sink;
        }
        m_bValidMarkable = sal_True;
    }
}

sal_Int32 OObjectInputStream::createMark(void)              throw (IOException, RuntimeException)
{
    connectToMarkable();    // throws an exception, if a markable is not connected !

    return m_rMarkable->createMark();
}

void OObjectInputStream::deleteMark(sal_Int32 Mark)         throw (IOException, IllegalArgumentException, RuntimeException)
{
    if( ! m_bValidMarkable )
    {
        throw NotConnectedException();
    }
    m_rMarkable->deleteMark( Mark );
}

void OObjectInputStream::jumpToMark(sal_Int32 nMark)        throw (IOException, IllegalArgumentException, RuntimeException)
{
    if( ! m_bValidMarkable )
    {
        throw NotConnectedException();
    }
    m_rMarkable->jumpToMark( nMark );
}
void OObjectInputStream::jumpToFurthest(void)           throw (IOException, RuntimeException)
{
    connectToMarkable();
    m_rMarkable->jumpToFurthest();
}

sal_Int32 OObjectInputStream::offsetToMark(sal_Int32 nMark)
    throw (IOException, IllegalArgumentException, RuntimeException)
{
    if( ! m_bValidMarkable )
    {
        throw NotConnectedException();
    }
    return m_rMarkable->offsetToMark( nMark );
}

// XServiceInfo
OUString OObjectInputStream::getImplementationName() throw ()
{
    return OObjectInputStream_getImplementationName();
}

// XServiceInfo
sal_Bool OObjectInputStream::supportsService(const OUString& ServiceName) throw ()
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

// XServiceInfo
Sequence< OUString > OObjectInputStream::getSupportedServiceNames(void) throw ()
{
    return OObjectInputStream_getSupportedServiceNames();
}




Reference< XInterface > SAL_CALL OObjectInputStream_CreateInstance( const Reference < XComponentContext > & rCtx ) throw(Exception)
{
    OObjectInputStream *p = new OObjectInputStream( rCtx );
    return Reference< XInterface> ( (static_cast< OWeakObject * >(p)) );
}

OUString OObjectInputStream_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.io.stm.ObjectInputStream" ) );
}

Sequence<OUString> OObjectInputStream_getSupportedServiceNames(void)
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.io.ObjectInputStream" ) );
    return aRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
