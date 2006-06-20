/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ftpinpstr.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:24:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


FTPInputStream::FTPInputStream(FILE* tmpfl)
    : m_tmpfl(tmpfl ? tmpfl : tmpfile())
{
    fseek(m_tmpfl,0,SEEK_END);
//  fpos_t pos;
//  fgetpos(m_tmpfl,&pos);
    long pos = ftell(m_tmpfl);
    rewind(m_tmpfl);
    m_nLength = sal_Int64(pos);
}



FTPInputStream::~FTPInputStream()
{
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
    fread(aData.getArray(),nBytesToRead,1,m_tmpfl);
    epos = ftell(m_tmpfl);

    return sal_Int32(epos-bpos);
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
