/*************************************************************************
 *
 *  $RCSfile: seqstream.hxx,v $
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
#define _COMPHELPER_SEQSTREAM_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

namespace comphelper
{

    namespace staruno = ::com::sun::star::uno;
    namespace stario  = ::com::sun::star::io;

    typedef staruno::Sequence<sal_Int8> ByteSequence;

//==================================================================
// SequenceInputStream
// stream for reading data from a sequence of bytes
//==================================================================


class SequenceInputStream
: public ::cppu::WeakImplHelper1< stario::XInputStream >
{
    ::osl::Mutex    m_aMutex;
    ByteSequence    m_aData;
    sal_Int32       m_nPos;

public:
    SequenceInputStream(const ByteSequence& rData);

// com::sun::star::io::XInputStream
    virtual sal_Int32 SAL_CALL readBytes( staruno::Sequence<sal_Int8>& aData, sal_Int32 nBytesToRead )
        throw(stario::NotConnectedException, stario::BufferSizeExceededException,
              stario::IOException, staruno::RuntimeException);

    virtual sal_Int32 SAL_CALL readSomeBytes( staruno::Sequence<sal_Int8>& aData, sal_Int32 nMaxBytesToRead )
        throw(stario::NotConnectedException, stario::BufferSizeExceededException,
              stario::IOException, staruno::RuntimeException);

    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
        throw(stario::NotConnectedException, stario::BufferSizeExceededException,
              stario::IOException, staruno::RuntimeException);

    virtual sal_Int32 SAL_CALL available(  )
        throw(stario::NotConnectedException, stario::IOException, staruno::RuntimeException);

    virtual void SAL_CALL closeInput(  )
        throw(stario::NotConnectedException, stario::IOException, staruno::RuntimeException);

private:
    inline sal_Int32 avail();
};

} // namespace comphelper

#endif //_COMPHELPER_SEQSTREAM_HXX


