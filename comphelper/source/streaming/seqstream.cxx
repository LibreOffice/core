/*************************************************************************
 *
 *  $RCSfile: seqstream.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-09-29 11:28:15 $
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

#ifndef _COMPHELPER_SEQSTREAM_HXX
#include <comphelper/seqstream.hxx>
#endif

#include <memory.h> // for memcpy

namespace comphelper
{

//---------------------------------------------------------------------------------------------
// class SequenceInputStream
//---------------------------------------------------------------------------------------------

//------------------------------------------------------------------
SequenceInputStream::SequenceInputStream(const ByteSequence& rData)
:   m_aData(rData)
,   m_nPos(0)
{
}

// checks if closed, returns available size, not mutex-protected
//------------------------------------------------------------------
inline sal_Int32 SequenceInputStream::avail()
{
    if (m_nPos == -1)
        throw stario::NotConnectedException(::rtl::OUString(), *this);

    return m_aData.getLength() - m_nPos;
}

// com::sun::star::io::XInputStream
//------------------------------------------------------------------
sal_Int32 SAL_CALL SequenceInputStream::readBytes( staruno::Sequence<sal_Int8>& aData, sal_Int32 nBytesToRead )
    throw(stario::NotConnectedException, stario::BufferSizeExceededException,
          stario::IOException, staruno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nAvail = avail();

    if (nBytesToRead < 0)
        throw stario::BufferSizeExceededException(::rtl::OUString(),*this);

    if (nAvail < nBytesToRead)
        nBytesToRead = nAvail;

    aData.realloc(nBytesToRead);
    memcpy(aData.getArray(), m_aData.getConstArray() + m_nPos, nBytesToRead);
    m_nPos += nBytesToRead;

    return nBytesToRead;
}

//------------------------------------------------------------------
sal_Int32 SAL_CALL SequenceInputStream::readSomeBytes( staruno::Sequence<sal_Int8>& aData, sal_Int32 nMaxBytesToRead )
    throw(stario::NotConnectedException, stario::BufferSizeExceededException,
          stario::IOException, staruno::RuntimeException)
{
    // all data is available at once
    return readBytes(aData, nMaxBytesToRead);
}

//------------------------------------------------------------------
void SAL_CALL SequenceInputStream::skipBytes( sal_Int32 nBytesToSkip )
    throw(stario::NotConnectedException, stario::BufferSizeExceededException,
          stario::IOException, staruno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nAvail = avail();

    if (nBytesToSkip < 0)
        throw stario::BufferSizeExceededException(::rtl::OUString(),*this);

    if (nAvail < nBytesToSkip)
        nBytesToSkip = nAvail;

    m_nPos += nBytesToSkip;
}

//------------------------------------------------------------------
sal_Int32 SAL_CALL SequenceInputStream::available(  )
    throw(stario::NotConnectedException, stario::IOException, staruno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return avail();
}

//------------------------------------------------------------------
void SAL_CALL SequenceInputStream::closeInput(  )
    throw(stario::NotConnectedException, stario::IOException, staruno::RuntimeException)
{
    if (m_nPos == -1)
        throw stario::NotConnectedException(::rtl::OUString(), *this);

    m_nPos = -1;
}
} // namespace comphelper
