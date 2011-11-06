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
#include <stdio.h>

using namespace ftp;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;


FTPInputStream::FTPInputStream(FILE* tmpfl)
    : m_tmpfl(tmpfl ? tmpfl : tmpfile())
{
    fseek(m_tmpfl,0,SEEK_END);
//      fpos_t pos;
//      fgetpos(m_tmpfl,&pos);
    long pos = ftell(m_tmpfl);
    rewind(m_tmpfl);
    m_nLength = sal_Int64(pos);
}



FTPInputStream::~FTPInputStream()
{
    if ( 0 != m_tmpfl)
        fclose(m_tmpfl);
}


Any SAL_CALL FTPInputStream::queryInterface(
    const Type& rType
)
    throw(
        RuntimeException
    )
{
    Any aRet = ::cppu::queryInterface(rType,
                                      SAL_STATIC_CAST( XInputStream*,this ),
                                      SAL_STATIC_CAST( XSeekable*,this ) );

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}



void SAL_CALL FTPInputStream::acquire( void ) throw() {
    OWeakObject::acquire();
}



void SAL_CALL FTPInputStream::release( void ) throw() {
    OWeakObject::release();
}


sal_Int32 SAL_CALL FTPInputStream::readBytes(Sequence< sal_Int8 >& aData,
                                             sal_Int32 nBytesToRead)
    throw(NotConnectedException,
          BufferSizeExceededException,
          IOException,
          RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);

    if(0 <= nBytesToRead && aData.getLength() < nBytesToRead)
        aData.realloc(nBytesToRead);

//     fpos_t bpos,epos;

//     fgetpos(m_tmpfl,&bpos);
//     fread(aData.getArray(),nBytesToRead,1,m_tmpfl);
//     fgetpos(m_tmpfl,&epos);
    long bpos,epos;

    bpos = ftell(m_tmpfl);
    if (fread(aData.getArray(),nBytesToRead,1,m_tmpfl) != 1)
        throw IOException();

    epos = ftell(m_tmpfl);

    return sal_Int32(epos-bpos);
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

    fseek(m_tmpfl,long(nBytesToSkip),SEEK_CUR);
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
        fclose(m_tmpfl),m_tmpfl = 0;
}



void SAL_CALL FTPInputStream::seek(sal_Int64 location)
    throw( IllegalArgumentException,
           IOException,
           RuntimeException )
{
    osl::MutexGuard aGuard(m_aMutex);
    if(!m_tmpfl)
        throw IOException();

    fseek(m_tmpfl,long(location),SEEK_SET);
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

//     fpos_t pos;
//     fgetpos(m_tmpfl,&pos);
    long pos;
    pos = ftell(m_tmpfl);
    return sal_Int64(pos);
}



sal_Int64 SAL_CALL FTPInputStream::getLength(
    void
) throw(
    IOException,RuntimeException
)
{
    return m_nLength;
}
