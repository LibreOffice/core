/*************************************************************************
 *
 *  $RCSfile: ftpinpstr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: abi $ $Date: 2002-08-28 07:23:12 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _FTP_FTPINPSTR_HXX_
#include "ftpinpstr.hxx"
#endif
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


FTPInputStream::FTPInputStream()
    : m_nMaxLen(1024*1024),
      m_nLen(0),
      m_nWritePos(0),
      m_nReadPos(0),
      m_pBuffer(0),
      m_pFile(0)  { }



FTPInputStream::~FTPInputStream() {
    rtl_freeMemory(m_pBuffer);
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


/** nBytesToRead < 0
    returns zero written bytes.
*/

sal_Int32 SAL_CALL FTPInputStream::readBytes(Sequence< sal_Int8 >& aData,
                                             sal_Int32 nBytesToRead)
    throw(NotConnectedException,
          BufferSizeExceededException,
          IOException,
          RuntimeException) {
    osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 curr =
        std::min(nBytesToRead,sal_Int32(m_nWritePos)-sal_Int32(m_nReadPos));

    if(0 <= curr && aData.getLength() < curr)
        aData.realloc(curr);

    for(sal_Int32 k = 0; k < curr; )
        aData[k++] = static_cast<sal_Int8*>(m_pBuffer)[m_nReadPos++];

    return curr > 0 ? curr : 0;
}


sal_Int32 SAL_CALL FTPInputStream::readSomeBytes( Sequence< sal_Int8 >& aData,sal_Int32 nMaxBytesToRead )
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
          RuntimeException) {
    osl::MutexGuard aGuard(m_aMutex);
    if(nBytesToSkip < 0)
        throw IOException();
    m_nReadPos += nBytesToSkip;
    if(m_nReadPos > m_nWritePos)   // Can't skip behind the end of the current write-position.
        m_nReadPos = m_nWritePos;
}



sal_Int32 SAL_CALL FTPInputStream::available(void)
    throw(NotConnectedException,
          IOException,
          RuntimeException) {
      osl::MutexGuard aGuard(m_aMutex);
      return std::max(sal_Int32(m_nWritePos)-sal_Int32(m_nReadPos),sal_Int32(0));
}



void SAL_CALL FTPInputStream::closeInput(void)
    throw(NotConnectedException,
          IOException,
          RuntimeException) {
    // fclose(m_pFile);
}



void SAL_CALL FTPInputStream::seek(sal_Int64 location)
    throw( IllegalArgumentException,
           IOException,
           RuntimeException ) {
    osl::MutexGuard aGuard(m_aMutex);
    if(location < 0)
        throw IllegalArgumentException();

    m_nReadPos = sal_uInt32(location);
    if(m_nReadPos > m_nWritePos)   // Can't seek behind the end
        //                         // of the current write-position.
        m_nReadPos = m_nWritePos;
}



sal_Int64 SAL_CALL
FTPInputStream::getPosition(
    void )
    throw( IOException,
           RuntimeException )
{
    osl::MutexGuard aGuard(m_aMutex);
    return sal_Int64(m_nReadPos);
}



sal_Int64 SAL_CALL FTPInputStream::getLength(
    void
) throw(
    IOException,RuntimeException
)
{
    osl::MutexGuard aGuard(m_aMutex);
    return sal_Int64(m_nWritePos);
}


const void* FTPInputStream::getBuffer(
    void
) const
    throw(
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    return m_pBuffer;
}


//  void
//  FTPInputStream::append(
//      const void* pBuffer,
//      size_t size,
//      size_t nmemb
//  ) throw()
//  {
//      if(!m_pFile)
//          m_pFile = tmpfile();

//      fwrite(pBuffer,size,nmemb,m_pFile);
//  }

void
FTPInputStream::append(
    const void* pBuffer,
    size_t size,
    size_t nmemb
) throw()
{
    osl::MutexGuard aGuard(m_aMutex);
    sal_uInt32 nLen = size*nmemb;
    sal_uInt32 tmp(nLen + m_nWritePos);

    if(m_nLen < tmp) { // enlarge in steps of multiples of 1K
        do {
            m_nLen+=1024;
            } while(m_nLen < tmp);

        m_pBuffer = rtl_reallocateMemory(m_pBuffer,m_nLen);
    }

    rtl_copyMemory(static_cast<sal_Int8*>(m_pBuffer)+m_nWritePos,
                   pBuffer,nLen);
    m_nWritePos = tmp;
}


void FTPInputStream::reset() throw()
{
    osl::MutexGuard aGuard(m_aMutex);
    m_nLen = 0;
    m_nWritePos = 0;
    m_nReadPos = 0;
    rtl_freeMemory(m_pBuffer),m_pBuffer = 0;
}


void
FTPInputStream::append2File(
    const void* pBuffer,
    size_t size,
    size_t nmemb
) throw()
{
    fwrite(pBuffer,size,nmemb,m_pFile);
}
