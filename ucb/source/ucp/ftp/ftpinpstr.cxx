/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include "ftpinpstr.hxx"
#ifndef _RTL_ALLOC_H
#include <rtl/alloc.h>
#endif
#ifndef STD_ALGORITHM
#include <algorithm>
#define STD_ALGORITHM
#endif

using namespace ftp;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;


FTPInputStream::FTPInputStream( oslFileHandle tmpfl )
    : m_tmpfl(tmpfl)
    , m_nLength( 0 )
{
    if ( !m_tmpfl )
        osl_createTempFile( NULL, &m_tmpfl, NULL );
    OSL_ENSURE( m_tmpfl, "input stream without tempfile!" );

    if ( osl_setFilePos( m_tmpfl, osl_Pos_End, 0 ) == osl_File_E_None )
    {
        sal_uInt64 nFileSize = 0;
        if ( osl_getFilePos( m_tmpfl, &nFileSize ) == osl_File_E_None )
            m_nLength = nFileSize;
        osl_setFilePos( m_tmpfl, osl_Pos_Absolut, 0 );
    }
}

FTPInputStream::~FTPInputStream()
{
    if ( 0 != m_tmpfl)
        osl_closeFile(m_tmpfl);
}

sal_Int32 SAL_CALL FTPInputStream::readBytes(Sequence< sal_Int8 >& aData,
                                             sal_Int32 nBytesToRead)
    throw(NotConnectedException,
          BufferSizeExceededException,
          IOException,
          RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);

    sal_uInt64 nBeforePos( 0 );
    sal_uInt64 nBytesRequested( nBytesToRead );
    sal_uInt64 nBytesRead( 0 );

    osl_getFilePos( m_tmpfl, &nBeforePos );

    if ( 0 == ( nBytesRequested = std::min< sal_uInt64 >( m_nLength - nBeforePos, nBytesRequested ) ) )
        return 0;

    if ( 0 <= nBytesToRead && aData.getLength() < nBytesToRead )
        aData.realloc( nBytesToRead );

    if ( osl_readFile( m_tmpfl, aData.getArray(), nBytesRequested, &nBytesRead ) != osl_File_E_None )
        throw IOException();

    return sal_Int32( nBytesRead );
}


sal_Int32 SAL_CALL FTPInputStream::readSomeBytes( Sequence< sal_Int8 >& aData,
                                                  sal_Int32 nMaxBytesToRead )
    throw( NotConnectedException,
           BufferSizeExceededException,
           IOException,
           RuntimeException)
{
    return readBytes(aData,nMaxBytesToRead);
}



void SAL_CALL FTPInputStream::skipBytes(sal_Int32 nBytesToSkip)
    throw(NotConnectedException,
          BufferSizeExceededException,
          IOException,
          RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);
    if(!m_tmpfl)
        throw IOException();

    osl_setFilePos( m_tmpfl, osl_Pos_Current, nBytesToSkip );
}



sal_Int32 SAL_CALL FTPInputStream::available(void)
    throw(NotConnectedException,
          IOException,
          RuntimeException)
{
    return sal::static_int_cast<sal_Int32>(m_nLength - getPosition());
}



void SAL_CALL FTPInputStream::closeInput(void)
    throw(NotConnectedException,
          IOException,
          RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);
    if(m_tmpfl)
        osl_closeFile(m_tmpfl),m_tmpfl = 0;
}



void SAL_CALL FTPInputStream::seek(sal_Int64 location)
    throw( IllegalArgumentException,
           IOException,
           RuntimeException )
{
    osl::MutexGuard aGuard(m_aMutex);
    if(!m_tmpfl)
        throw IOException();

    osl_setFilePos( m_tmpfl, osl_Pos_Absolut, location );
}



sal_Int64 SAL_CALL
FTPInputStream::getPosition(
    void )
    throw( IOException,
           RuntimeException )
{
    osl::MutexGuard aGuard(m_aMutex);
    if(!m_tmpfl)
        throw IOException();

    sal_uInt64 nFilePos = 0;
    osl_getFilePos( m_tmpfl, &nFilePos );
    return nFilePos;
}



sal_Int64 SAL_CALL FTPInputStream::getLength(
    void
) throw(
    IOException,RuntimeException
)
{
    return m_nLength;
}
