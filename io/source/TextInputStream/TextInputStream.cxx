/*************************************************************************
 *
 *  $RCSfile: TextInputStream.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-12 15:50:30 $
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


#include <string.h>
#include <osl/mutex.hxx>
#include <osl/diagnose.h>

#include <uno/mapping.hxx>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase2.hxx>

#include <rtl/textenc.h>
#include <rtl/tencinfo.h>

#include <com/sun/star/io/XTextInputStream.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>


#define IMPLEMENTATION_NAME "com.sun.star.comp.io.TextInputStream"
#define SERVICE_NAME "com.sun.star.io.TextInputStream"

using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::registry;


namespace io_TextStream
{

//===========================================================================
// Implementation XTextInputStream

typedef WeakImplHelper2< XTextInputStream, XActiveDataSink > TextInputStreamHelper;
class OCommandEnvironment;

#define INITIAL_UNICODE_BUFFER_CAPACITY     0x100
#define READ_BYTE_COUNT                     0x100

class OTextInputStream : public TextInputStreamHelper
{
    Reference< XInputStream > mxStream;

    // Encoding
    OUString mEncoding;
    sal_Bool mbEncodingInitialized;
    rtl_TextToUnicodeConverter  mConvText2Unicode;
    rtl_TextToUnicodeContext    mContextText2Unicode;
    Sequence<sal_Int8>          mSeqSource;

    // Internal buffer for characters that are already converted successfully
    sal_Unicode* mpBuffer;
    sal_Int32 mnBufferSize;
    sal_Int32 mnCharsInBuffer;
    sal_Bool mbReachedEOF;

    void implResizeBuffer( void );
    OUString implReadString( const Sequence< sal_Unicode >& Delimiters,
        sal_Bool bRemoveDelimiter, sal_Bool bFindLineEnd )
            throw(IOException, RuntimeException);
    sal_Int32 implReadNext() throw(IOException, RuntimeException);

public:
    OTextInputStream();
    virtual ~OTextInputStream();

    // Methods XTextInputStream
    virtual OUString SAL_CALL readLine(  )
        throw(IOException, RuntimeException);
    virtual OUString SAL_CALL readString( const Sequence< sal_Unicode >& Delimiters, sal_Bool bRemoveDelimiter )
        throw(IOException, RuntimeException);
    virtual sal_Bool SAL_CALL isEOF(  )
        throw(IOException, RuntimeException);
    virtual void SAL_CALL setEncoding( const OUString& Encoding ) throw(RuntimeException);

    // Methods XInputStream
    virtual sal_Int32 SAL_CALL readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException);
    virtual sal_Int32 SAL_CALL readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException);
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException);
    virtual sal_Int32 SAL_CALL available(  )
        throw(NotConnectedException, IOException, RuntimeException);
    virtual void SAL_CALL closeInput(  )
        throw(NotConnectedException, IOException, RuntimeException);

    // Methods XActiveDataSink
    virtual void SAL_CALL setInputStream( const Reference< XInputStream >& aStream )
        throw(RuntimeException);
    virtual Reference< XInputStream > SAL_CALL getInputStream()
        throw(RuntimeException);
};

OTextInputStream::OTextInputStream()
    : mpBuffer( NULL ), mnBufferSize( 0 ), mnCharsInBuffer( 0 ), mbReachedEOF( sal_False )
{
    mbEncodingInitialized = false;
}

OTextInputStream::~OTextInputStream()
{
    if( mbEncodingInitialized )
    {
        rtl_destroyUnicodeToTextContext( mConvText2Unicode, mContextText2Unicode );
        rtl_destroyUnicodeToTextConverter( mConvText2Unicode );
    }
}

void OTextInputStream::implResizeBuffer( void )
{
    sal_Int32 mnNewBufferSize = mnBufferSize * 2;
    sal_Unicode* pNewBuffer = new sal_Unicode[ mnNewBufferSize ];
    memcpy( pNewBuffer, mpBuffer, mnCharsInBuffer * sizeof( sal_Unicode ) );
    mpBuffer = pNewBuffer;
    mnBufferSize = mnNewBufferSize;
}


//===========================================================================
// XTextInputStream

OUString OTextInputStream::readLine(  )
    throw(IOException, RuntimeException)
{
    static Sequence< sal_Unicode > aDummySeq;
    return implReadString( aDummySeq, sal_True, sal_True );
}

OUString OTextInputStream::readString( const Sequence< sal_Unicode >& Delimiters, sal_Bool bRemoveDelimiter )
        throw(IOException, RuntimeException)
{
    return implReadString( Delimiters, sal_True, sal_False );
}

sal_Bool OTextInputStream::isEOF()
    throw(IOException, RuntimeException)
{
    sal_Bool bRet = sal_False;
    if( mnCharsInBuffer == 0 && mbReachedEOF )
        bRet = sal_True;
    return bRet;
}


OUString OTextInputStream::implReadString( const Sequence< sal_Unicode >& Delimiters,
                                           sal_Bool bRemoveDelimiter, sal_Bool bFindLineEnd )
        throw(IOException, RuntimeException)
{
    OUString aRetStr;
    if( !mbEncodingInitialized )
    {
        OUString aUtf8Str( RTL_CONSTASCII_USTRINGPARAM("utf8") );
        setEncoding( aUtf8Str );
    }
    if( !mbEncodingInitialized )
        return aRetStr;

    if( !mpBuffer )
    {
        mnBufferSize = INITIAL_UNICODE_BUFFER_CAPACITY;
        mpBuffer = new sal_Unicode[ mnBufferSize ];
    }

    // Only for bFindLineEnd
    sal_Unicode cLineEndChar1 = 0x13;
    sal_Unicode cLineEndChar2 = 0x10;

    sal_Int32 nBufferReadPos = 0;
    sal_Int32 nCopyLen = 0;
    sal_Bool bFound = sal_False;
    sal_Bool bFoundFirstLineEndChar = sal_False;
    sal_Unicode cFirstLineEndChar;
    const sal_Unicode* pDelims = Delimiters.getConstArray();
    const sal_Int32 nDelimCount = Delimiters.getLength();
    while( !bFound )
    {
        // Still characters available?
        if( nBufferReadPos == mnCharsInBuffer )
        {
            // Already reached EOF? Then we can't read any more
            if( mbReachedEOF )
                break;

            // No, so read new characters
            if( !implReadNext() )
                break;

            /*
            sal_Int32 nFreeBufferSize = mnBufferSize - nBufferReadPos;
            if( nFreeBufferSize < READ_BYTE_COUNT )
                implResizeBuffer();

            try
            {
                Sequence< sal_Int8 > aData;
                sal_Int32 nBytesToRead = READ_BYTE_COUNT;
                sal_Int32 nRead = mxStream->readSomeBytes( aData, nBytesToRead );
                if( nRead < nBytesToRead )
                    bEOF = sal_True;

                // Try to convert
                sal_uInt32 uiInfo;
                sal_Size nSrcCvtBytes = 0;
                sal_Size nTargetCount = 0;
                sal_Size nSourceCount = 0;
                while( sal_True )
                {
                    const sal_Int8 *pbSource = aData.getConstArray();

                    //// the whole source size
                    //sal_Int32 nSourceSize = seqText.getLength() + m_seqSource.getLength();
                    //Sequence<sal_Unicode>     seqUnicode ( nSourceSize );
//
                    //const sal_Int8 *pbSource = seqText.getConstArray();
                    //sal_Int8 *pbTempMem = 0;
//
                    //if( m_seqSource.getLength() ) {
                        //// put old rest and new byte sequence into one array
                        //pbTempMem = new sal_Int8[ nSourceSize ];
                        //memcpy( pbTempMem , m_seqSource.getConstArray() , m_seqSource.getLength() );
                        //memcpy( &(pbTempMem[ m_seqSource.getLength() ]) , seqText.getConstArray() , seqText.getLength() );
                        //pbSource = pbTempMem;
//
                        //// set to zero again
                        //m_seqSource = Sequence< sal_Int8 >();
                    //}

                    // All invalid characters are transformed to the unicode undefined char
                    nTargetCount += rtl_convertTextToUnicode(
                                        mConvText2Unicode,
                                        mContextText2Unicode,
                                        (const sal_Char*) &( pbSource[nSourceCount] ),
                                        nRead - nSourceCount,
                                        mpBuffer + nBufferReadPos + nTargetCount,
                                        mnBufferSize - nBufferReadPos - nTargetCount,
                                        RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT   |
                                        RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                                        RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT,
                                        &uiInfo,
                                        &nSrcCvtBytes );
                    nSourceCount += nSrcCvtBytes;

                    sal_Bool bCont = sal_False;
                    if( uiInfo & RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL )
                    {
                        implResizeBuffer();
                        bCont = sal_True;
                    }

                    if( uiInfo & RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL )
                    {
                        // read next byte
                        static Sequence< sal_Int8 > aOneByteSeq( 1 );
                        sal_Int32 nRead = mxStream->readSomeBytes( aData, 1 );
                        if( nRead == 0 )
                        {
                            bEOF = sal_True;

                            // return what we have
                            // TODO
                        }
                        sal_Int32 nOldLen = aData.getLength();
                        aData.realloc( nOldLen + 1 );
                        aData.getArray()[ nOldLen ] = aOneByteSeq.getConstArray()[ 0 ];
                        pbSource = aData.getConstArray();
                        bCont = sal_True;
                    }

                    if( bCont )
                        continue;
                    break;
                }

                mnCharsInBuffer += nTargetCount;
            }
            catch( NotConnectedException& e1 )
            {
                e1;
                throw IOException();
                //throw IOException( L"OTextInputStream::implReadString failed" );
            }
            catch( BufferSizeExceededException& e2 )
            {
                e2;
                throw IOException();
            }
            */
        }

        // Now there should be characters available
        // (otherwise the loop should have been breaked before)
        sal_Unicode c = mpBuffer[ nBufferReadPos++ ];

        if( bFindLineEnd )
        {
            if( bFoundFirstLineEndChar )
            {
                bFound = sal_True;
                nCopyLen = nBufferReadPos - 2;
                if( c == cLineEndChar1 || c == cLineEndChar2 )
                {
                    // Same line end char -> new line break
                    if( c == cFirstLineEndChar )
                    {
                        nBufferReadPos--;
                    }
                }
            }
            else if( c == cLineEndChar1 || c == cLineEndChar2 )
            {
                bFoundFirstLineEndChar = sal_True;
                cFirstLineEndChar = c;
            }
        }
        else
        {
            for( sal_Int32 i = 0 ; i < nDelimCount ; i++ )
            {
                if( c == pDelims[ i ] )
                {
                    bFound = sal_True;
                    nCopyLen = nBufferReadPos;
                    if( bRemoveDelimiter )
                        nCopyLen--;
                }
            }
        }
    }

    // Create string
    if( nCopyLen )
        aRetStr = OUString( mpBuffer, nCopyLen );

    // Copy rest of buffer
    memmove( mpBuffer, mpBuffer + nBufferReadPos,
        (mnCharsInBuffer - nBufferReadPos) * sizeof( sal_Unicode ) );
    mnCharsInBuffer -= nBufferReadPos;

    return aRetStr;
}


sal_Int32 OTextInputStream::implReadNext()
        throw(IOException, RuntimeException)
{
    sal_Int32 nFreeBufferSize = mnBufferSize - mnCharsInBuffer;
    if( nFreeBufferSize < READ_BYTE_COUNT )
        implResizeBuffer();
    nFreeBufferSize = mnBufferSize - mnCharsInBuffer;

    try
    {
        Sequence< sal_Int8 > aData;
        sal_Int32 nBytesToRead = READ_BYTE_COUNT;
        sal_Int32 nRead = mxStream->readSomeBytes( aData, nBytesToRead );
        if( nRead < nBytesToRead )
            mbReachedEOF = sal_True;

        // Try to convert
        sal_uInt32 uiInfo;
        sal_Size nSrcCvtBytes = 0;
        sal_Size nTargetCount = 0;
        sal_Size nSourceCount = 0;
        while( sal_True )
        {
            const sal_Int8 *pbSource = aData.getConstArray();

            //// the whole source size
            //sal_Int32 nSourceSize = seqText.getLength() + m_seqSource.getLength();
            //Sequence<sal_Unicode>     seqUnicode ( nSourceSize );
//
            //const sal_Int8 *pbSource = seqText.getConstArray();
            //sal_Int8 *pbTempMem = 0;
//
            //if( m_seqSource.getLength() ) {
                //// put old rest and new byte sequence into one array
                //pbTempMem = new sal_Int8[ nSourceSize ];
                //memcpy( pbTempMem , m_seqSource.getConstArray() , m_seqSource.getLength() );
                //memcpy( &(pbTempMem[ m_seqSource.getLength() ]) , seqText.getConstArray() , seqText.getLength() );
                //pbSource = pbTempMem;
//
                //// set to zero again
                //m_seqSource = Sequence< sal_Int8 >();
            //}

            // All invalid characters are transformed to the unicode undefined char
            nTargetCount += rtl_convertTextToUnicode(
                                mConvText2Unicode,
                                mContextText2Unicode,
                                (const sal_Char*) &( pbSource[nSourceCount] ),
                                nRead - nSourceCount,
                                mpBuffer + mnCharsInBuffer + nTargetCount,
                                nFreeBufferSize - nTargetCount,
                                RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT   |
                                RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                                RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT,
                                &uiInfo,
                                &nSrcCvtBytes );
            nSourceCount += nSrcCvtBytes;

            sal_Bool bCont = sal_False;
            if( uiInfo & RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL )
            {
                implResizeBuffer();
                bCont = sal_True;
            }

            if( uiInfo & RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL )
            {
                // read next byte
                static Sequence< sal_Int8 > aOneByteSeq( 1 );
                sal_Int32 nRead = mxStream->readSomeBytes( aData, 1 );
                if( nRead == 0 )
                {
                    mbReachedEOF = sal_True;
                    break;
                }
                sal_Int32 nOldLen = aData.getLength();
                aData.realloc( nOldLen + 1 );
                aData.getArray()[ nOldLen ] = aOneByteSeq.getConstArray()[ 0 ];
                pbSource = aData.getConstArray();
                bCont = sal_True;
            }

            if( bCont )
                continue;
            break;
        }

        mnCharsInBuffer += nTargetCount;
        return nTargetCount;
    }
    catch( NotConnectedException& e1 )
    {
        e1;
        throw IOException();
        //throw IOException( L"OTextInputStream::implReadString failed" );
    }
    catch( BufferSizeExceededException& e2 )
    {
        e2;
        throw IOException();
    }
}


/*
OUString OTextInputStream::implConvert( const Sequence<sal_Int8> &seqText )
{
    sal_uInt32 uiInfo;
    sal_Size nSrcCvtBytes   = 0;
    sal_Size nTargetCount   = 0;
    sal_Size nSourceCount   = 0;

    // the whole source size
    sal_Int32 nSourceSize = seqText.getLength() + m_seqSource.getLength();
    Sequence<sal_Unicode>   seqUnicode ( nSourceSize );

    const sal_Int8 *pbSource = seqText.getConstArray();
    sal_Int8 *pbTempMem = 0;

    if( m_seqSource.getLength() ) {
        // put old rest and new byte sequence into one array
        pbTempMem = new sal_Int8[ nSourceSize ];
        memcpy( pbTempMem , m_seqSource.getConstArray() , m_seqSource.getLength() );
        memcpy( &(pbTempMem[ m_seqSource.getLength() ]) , seqText.getConstArray() , seqText.getLength() );
        pbSource = pbTempMem;

        // set to zero again
        m_seqSource = Sequence< sal_Int8 >();
    }

    while( sal_True ) {

        // All invalid characters are transformed to the unicode undefined char
        nTargetCount +=     rtl_convertTextToUnicode(
                                    m_convText2Unicode,
                                    m_contextText2Unicode,
                                    ( const sal_Char * ) &( pbSource[nSourceCount] ),
                                    nSourceSize - nSourceCount ,
                                    &( seqUnicode.getArray()[ nTargetCount ] ),
                                    seqUnicode.getLength() - nTargetCount,
                                    RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT   |
                                    RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                                    RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT,
                                    &uiInfo,
                                    &nSrcCvtBytes );
        nSourceCount += nSrcCvtBytes;

        if( uiInfo & RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL ) {
            // save necessary bytes for next conversion
            seqUnicode.realloc( seqUnicode.getLength() * 2 );
            continue;
        }
        break;
    }
    if( uiInfo & RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL ) {
        m_seqSource.realloc( nSourceSize - nSourceCount );
        memcpy( m_seqSource.getArray() , &(pbSource[nSourceCount]) , nSourceSize-nSourceCount );
    }


    if( pbTempMem ) {
        delete pbTempMem;
    }

    // set to correct unicode size
    seqUnicode.realloc( nTargetCount );

    return seqUnicode;
}
*/

void OTextInputStream::setEncoding( const OUString& Encoding )
    throw(RuntimeException)
{
    OString aOEncodingStr = OUStringToOString( Encoding, RTL_TEXTENCODING_ASCII_US );
    rtl_TextEncoding encoding = rtl_getTextEncodingFromMimeCharset( aOEncodingStr.getStr() );
    if( RTL_TEXTENCODING_DONTKNOW == encoding )
        return;

    mbEncodingInitialized = true;
    mConvText2Unicode = rtl_createTextToUnicodeConverter( encoding );
    mContextText2Unicode = rtl_createTextToUnicodeContext( mConvText2Unicode );
    mEncoding = Encoding;
}

//===========================================================================
// XInputStream

sal_Int32 OTextInputStream::readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
    throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    return mxStream->readBytes( aData, nBytesToRead );
}

sal_Int32 OTextInputStream::readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
    throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    return mxStream->readSomeBytes( aData, nMaxBytesToRead );
}

void OTextInputStream::skipBytes( sal_Int32 nBytesToSkip )
    throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    mxStream->skipBytes( nBytesToSkip );
}

sal_Int32 OTextInputStream::available(  )
    throw(NotConnectedException, IOException, RuntimeException)
{
    return mxStream->available();
}

void OTextInputStream::closeInput(  )
    throw(NotConnectedException, IOException, RuntimeException)
{
    mxStream->closeInput();
}




//===========================================================================
// XActiveDataSink

void OTextInputStream::setInputStream( const Reference< XInputStream >& aStream )
    throw(RuntimeException)
{
    mxStream = aStream;
}

Reference< XInputStream > OTextInputStream::getInputStream()
    throw(RuntimeException)
{
    return mxStream;
}


Reference< XInterface > SAL_CALL TextInputStream_CreateInstance( const Reference< XMultiServiceFactory > &)
{
    return Reference < XInterface >( ( OWeakObject * ) new OTextInputStream() );
}


Sequence< OUString > TextInputStream_getSupportedServiceNames()
{
    static Sequence < OUString > *pNames = 0;
    if( ! pNames )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( !pNames )
        {
            static Sequence< OUString > seqNames(1);
            seqNames.getArray()[0] = OUString::createFromAscii( SERVICE_NAME );
            pNames = &seqNames;
        }
    }
    return *pNames;
}


}


//==================================================================================================
// Component exports

extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString::createFromAscii("/" IMPLEMENTATION_NAME "/UNO/SERVICES" ) ) );

            const Sequence< OUString > & rSNL = io_TextStream::TextInputStream_getSupportedServiceNames();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLEMENTATION_NAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            io_TextStream::TextInputStream_CreateInstance,
            io_TextStream::TextInputStream_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}


