/*************************************************************************
 *
 *  $RCSfile: odata.cxx,v $
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
#include <hash_map>
#include <vector>

#include <com/sun/star/io/XObjectInputStream.hpp>
#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XMarkableStream.hpp>
#include <com/sun/star/io/XConnectable.hpp>
#include <com/sun/star/io/UnexpectedEOFException.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/weak.hxx>      // OWeakObject
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <osl/mutex.hxx>

#include <assert.h>
#include <string.h>


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
    ODataInputStream( const Reference< XMultiServiceFactory > &r  ) :
        m_bValidStream( sal_False ),
        m_rFactory( r )
        {}

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
    Reference < XMultiServiceFactory > m_rFactory;

    Reference < XConnectable >  m_pred;
    Reference < XConnectable >  m_succ;

    Reference < XInputStream >  m_input;
    sal_Bool m_bValidStream;
};

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
    throw WrongFormatException( );
    return 0;
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
                  SAL_STATIC_CAST( XConnectable * , this ) ) );
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
                SAL_STATIC_CAST( XConnectable * , this ) ) );
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
    Sequence<OUString> seq(1);
    seq.getArray()[0] = ODataInputStream_getServiceName();
    return seq;
}




/***
*
* registration information
*
*
****/

Reference< XInterface > SAL_CALL ODataInputStream_CreateInstance( const Reference < XMultiServiceFactory > & rSMgr ) throw( Exception)
{
    ODataInputStream *p = new ODataInputStream( rSMgr );
    return Reference< XInterface > ( (OWeakObject * ) p );
}


OUString ODataInputStream_getServiceName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.io.DataInputStream" ) );
}

OUString ODataInputStream_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.io.stm.DataInputStream" ) );
}

Sequence<OUString> ODataInputStream_getSupportedServiceNames(void)
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = ODataInputStream_getServiceName();

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
    ODataOutputStream( const Reference< XMultiServiceFactory >  &r) :
        m_rFactory(r),
        m_bValidStream( sal_False )
        {}

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
    Reference < XMultiServiceFactory >  m_rFactory;
    Reference < XConnectable >  m_succ;
    Reference < XConnectable >  m_pred;

Reference<  XOutputStream > m_output;
    sal_Bool m_bValidStream;
};



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
    throw WrongFormatException();
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
    sal_Int32 nStrLen = Value.len();
    const sal_Unicode * pStr = Value.getStr();
    sal_Int32 nUTFLen = 0;

    for( sal_Int32 i = 0 ; i < nStrLen ; i++ )
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
            //written += 2;
        }
        else
        {
            writeByte(sal_Int8(0xC0 | ((c >>  6) & 0x1F)));
            writeByte(sal_Int8(0x80 | ((c >>  0) & 0x3F)));
            //written += 1;
        }
    }
    //written += strlen + 2;
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
                  SAL_STATIC_CAST( XConnectable * , this  ) ));
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
                SAL_STATIC_CAST( XConnectable * , this  ) ));
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

    Sequence<OUString> seq(1);
    seq.getArray()[0] = ODataOutputStream_getServiceName();
    return seq;
}




Reference< XInterface > SAL_CALL ODataOutputStream_CreateInstance( const Reference < XMultiServiceFactory > & rSMgr ) throw(Exception)
{
    ODataOutputStream *p = new ODataOutputStream( rSMgr );
    Reference< XInterface > xService = *p;
    return xService;
}

OUString ODataOutputStream_getServiceName()
{
    return OUString::createFromAscii( "com.sun.star.io.DataOutputStream" );
}

OUString ODataOutputStream_getImplementationName()
{
    return OUString::createFromAscii( "com.sun.star.comp.io.stm.DataOutputStream" );
}

Sequence<OUString> ODataOutputStream_getSupportedServiceNames(void)
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = ODataOutputStream_getServiceName();

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

typedef hash_map
<
    Reference< XInterface >,
    sal_Int32,
    hashObjectContainer_Impl,
    equalObjectContainer_Impl
> ObjectContainer_Impl;

//------------------------------
//------------------------------
//------------------------------
//  struct equalXPersistObjectRef_Impl
//  {
//      sal_Int32 operator()(const Reference< XPersistObject > & s1, c
//                           const Reference< XPersistObject > & s2) const
//      {
//          return s1 == s2;
//      }
//  };

//  //-----------------------------------------------------------------------------
//  struct hashXPersistObjectRef_Impl
//  {
//      size_t operator()(const Reference< XPersistObject > & xRef) const
//      {
//          return (size_t)(XInterface *)xRef;
//      }
//  };


//  typedef NAMESPACE_STD(hash_map)
//  <
//      XPersistObjectRef,
//      sal_Int32,
//      hashXPersistObjectRef_Impl,
//      equalXPersistObjectRef_Impl
//  > XPersistObjectRefHashMap_Impl;

/**
 */
/*---------------------------------------------
*
*
*
*
*--------------------------------------------*/
class OObjectOutputStream :
            public ODataOutputStream,
            public XObjectOutputStream,
            public XMarkableStream
{
public:
    OObjectOutputStream( const Reference < XMultiServiceFactory > &r ) :
                        ODataOutputStream(r) , m_nMaxId(0) , m_bValidMarkable(sal_False) {}

public:
    Any     SAL_CALL queryInterface( const Type &type );
    void    SAL_CALL acquire()                       { ODataOutputStream::acquire(); }
    void    SAL_CALL release()                       { ODataOutputStream::release(); }

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
    virtual void SAL_CALL writeHyper(Hyper Value) throw (IOException, RuntimeException)
                { ODataOutputStream::writeHyper( Value ); }
    virtual void SAL_CALL writeFloat(float Value) throw (IOException, RuntimeException)
                { ODataOutputStream::writeFloat( Value ); }
    virtual void SAL_CALL writeDouble(double Value) throw (IOException, RuntimeException)
                { ODataOutputStream::writeDouble( Value ); }
    virtual void SAL_CALL writeUTF(const OUString& Value) throw (IOException, RuntimeException)
                { ODataOutputStream::writeUTF( Value );}

    // XObjectOutputStream
    virtual void SAL_CALL writeObject( const Reference< XPersistObject > & r );

public: // XMarkableStream
    virtual sal_Int32 SAL_CALL createMark(void)                 throw (IOException, RuntimeException);
    virtual void SAL_CALL deleteMark(sal_Int32 Mark)            throw (IOException, IllegalArgumentException, RuntimeException);
    virtual void SAL_CALL jumpToMark(sal_Int32 nMark)       throw (IOException, IllegalArgumentException, RuntimeException);
    virtual void SAL_CALL jumpToFurthest(void)          throw (IOException, RuntimeException);
    virtual sal_Int32 SAL_CALL offsetToMark(sal_Int32 nMark)
        throw (IOException, IllegalArgumentException, RuntimeException);

public: //XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL
            getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
            getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

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


Any OObjectOutputStream::queryInterface( const Type &aType )
{
    Any a = ::cppu::queryInterface(
        aType ,
        SAL_STATIC_CAST( XMarkableStream * , this ),
        SAL_STATIC_CAST( XObjectOutputStream * , this ) );
    if( a.hasValue() )
    {
        return a;
    }

    return ODataOutputStream::queryInterface( aType );

}
void OObjectOutputStream::writeObject( const Reference< XPersistObject > & xPObj )
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
            SAL_STATIC_CAST( XObjectOutputStream * , this ) ) );

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
        Reference< XInterface > rTry = m_output;
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




Reference< XInterface > SAL_CALL OObjectOutputStream_CreateInstance( const Reference < XMultiServiceFactory > & rSMgr )
    throw(Exception)
{
    OObjectOutputStream *p = new OObjectOutputStream( rSMgr );
    return  Reference< XInterface > ( SAL_STATIC_CAST( OWeakObject * , p ) );
}

OUString OObjectOutputStream_getServiceName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.io.ObjectOutputStream" ) );
}

OUString OObjectOutputStream_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.io.stm.ObjectOutputStream" ) );
}

Sequence<OUString> OObjectOutputStream_getSupportedServiceNames(void)
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = OObjectOutputStream_getServiceName();
    return aRet;
}

Sequence< Type > SAL_CALL OObjectOutputStream::getTypes(void) throw( RuntimeException )
{
    static OTypeCollection *pCollection = 0;
    if( ! pCollection )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pCollection )
        {
            static OTypeCollection collection(
                getCppuType( (Reference< XMarkableStream > * ) 0 ),
                getCppuType( (Reference< XObjectOutputStream > * ) 0 ),
                ODataOutputStream::getTypes() );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}

Sequence< sal_Int8 > SAL_CALL OObjectOutputStream::getImplementationId(  ) throw( RuntimeException)
{
    static OImplementationId *pId = 0;
    if( ! pId )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pId )
        {
            static OImplementationId id( sal_False );
            pId = &id;
        }
    }
    return (*pId).getImplementationId();
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
    Sequence<OUString> seq(1);
    seq.getArray()[0] = OObjectOutputStream_getServiceName();
    return seq;
}





class OObjectInputStream :
    public ODataInputStream,
    public XObjectInputStream,
    public XMarkableStream
{
public:
    OObjectInputStream( const Reference < XMultiServiceFactory > &r) :
        ODataInputStream(r),
        m_bValidMarkable(sal_False)
        {}
public:
    Any     SAL_CALL queryInterface( const Type &type );
    void    SAL_CALL acquire()                       { ODataInputStream::acquire(); }
    void    SAL_CALL release()                       { ODataInputStream::release(); }

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
    virtual Hyper SAL_CALL readHyper(void) throw (IOException, RuntimeException)
                { return ODataInputStream::readHyper(); }
    virtual float SAL_CALL readFloat(void) throw (IOException, RuntimeException)
                { return ODataInputStream::readFloat(); }
    virtual double SAL_CALL readDouble(void) throw (IOException, RuntimeException)
        { return ODataInputStream::readDouble(); }
    virtual OUString SAL_CALL readUTF(void) throw (IOException, RuntimeException)
                { return ODataInputStream::readUTF(); }

public: // XObjectInputStream
    virtual Reference< XPersistObject > SAL_CALL readObject( );

public: // XMarkableStream
    virtual sal_Int32 SAL_CALL createMark(void)
        throw (IOException, RuntimeException);
    virtual void SAL_CALL deleteMark(sal_Int32 Mark)            throw (IOException, IllegalArgumentException, RuntimeException);
    virtual void SAL_CALL jumpToMark(sal_Int32 nMark)       throw (IOException, IllegalArgumentException, RuntimeException);
    virtual void SAL_CALL jumpToFurthest(void)          throw (IOException, RuntimeException);
    virtual sal_Int32 SAL_CALL offsetToMark(sal_Int32 nMark)
        throw (IOException, IllegalArgumentException, RuntimeException);

public: //XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL
            getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
            getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

public: // XServiceInfo
    OUString                     SAL_CALL getImplementationName() throw ();
    Sequence< OUString >         SAL_CALL getSupportedServiceNames(void) throw ();
    sal_Bool                     SAL_CALL supportsService(const OUString& ServiceName) throw ();

private:
    void connectToMarkable();
private:
    sal_Bool                m_bValidMarkable;
    Reference < XMarkableStream > m_rMarkable;
    vector < Reference<  XPersistObject > > m_aPersistVector;

};


Any OObjectInputStream::queryInterface( const Type &aType )
{
    Any a = ::cppu::queryInterface(
        aType ,
        SAL_STATIC_CAST( XMarkableStream * , this ),
        SAL_STATIC_CAST( XObjectInputStream * , this ) );
    if( a.hasValue() )
    {
        return a;
    }

    return ODataInputStream::queryInterface( aType );

}

Reference< XPersistObject >  OObjectInputStream::readObject()
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
        if( aName.len() )
        {
            // load the object
            Reference< XInterface > x = m_rFactory->createInstance( aName );
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
                    SAL_STATIC_CAST( XObjectInputStream *, this ) ) );
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
        Reference< XInterface > rTry = m_input;
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


Sequence< Type > SAL_CALL OObjectInputStream::getTypes(void) throw( RuntimeException )
{
    static OTypeCollection *pCollection = 0;
    if( ! pCollection )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pCollection )
        {
            static OTypeCollection collection(
                getCppuType( (Reference< XMarkableStream > * ) 0 ),
                getCppuType( (Reference< XObjectInputStream > * ) 0 ),
                ODataInputStream::getTypes() );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}

Sequence< sal_Int8 > SAL_CALL OObjectInputStream::getImplementationId(  ) throw( RuntimeException)
{
    static OImplementationId *pId = 0;
    if( ! pId )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pId )
        {
            static OImplementationId id( sal_False );
            pId = &id;
        }
    }
    return (*pId).getImplementationId();
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
    Sequence<OUString> seq(1);
    seq.getArray()[0] = OObjectInputStream_getServiceName();
    return seq;
}




Reference< XInterface > SAL_CALL OObjectInputStream_CreateInstance( const Reference < XMultiServiceFactory > & rSMgr ) throw(Exception)
{
    OObjectInputStream *p = new OObjectInputStream( rSMgr );
    return Reference< XInterface> (
        SAL_STATIC_CAST( OWeakObject *, p ) );
}

OUString OObjectInputStream_getServiceName()
{
    return OUString::createFromAscii( "com.sun.star.io.ObjectInputStream" );
}

OUString OObjectInputStream_getImplementationName()
{
    return OUString::createFromAscii( "com.sun.star.comp.io.stm.ObjectInputStream" );
}

Sequence<OUString> OObjectInputStream_getSupportedServiceNames(void)
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = OObjectInputStream_getServiceName();

    return aRet;
}

}
