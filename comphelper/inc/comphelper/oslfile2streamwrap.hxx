/*************************************************************************
 *
 *  $RCSfile: oslfile2streamwrap.hxx,v $
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
#ifndef _COMPHELPER_STREAM_OSLFILEWRAPPER_HXX_
#define _COMPHELPER_STREAM_OSLFILEWRAPPER_HXX_

#ifndef _OSL_MUTEX_HXX_ //autogen wg. ::osl::Mutex
#include <osl/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif


namespace comphelper
{
    namespace stario    = ::com::sun::star::io;
    namespace staruno   = ::com::sun::star::uno;

//==================================================================
// FmUnoIOStream,
// stream zum schreiben un lesen von Daten, basieren  auf File
//==================================================================
typedef ::cppu::WeakImplHelper1<stario::XInputStream> InputStreamWrapper_Base;
    // needed for some compilers
class OSLInputStreamWrapper : public InputStreamWrapper_Base
{
    ::osl::Mutex    m_aMutex;
    ::osl::File*    m_pFile;
    sal_Bool        m_bFileOwner : 1;

public:
    OSLInputStreamWrapper(::osl::File& _rStream);
    OSLInputStreamWrapper(::osl::File* pStream, sal_Bool bOwner=sal_False);
    virtual ~OSLInputStreamWrapper();

// UNO Anbindung
    virtual void            SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)
        { InputStreamWrapper_Base::acquire(); }
    virtual void            SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)
        { InputStreamWrapper_Base::release(); }
    virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException)
        { return InputStreamWrapper_Base::queryInterface(_rType); }

// stario::XInputStream
    virtual sal_Int32   SAL_CALL    readBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual sal_Int32   SAL_CALL    readSomeBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void        SAL_CALL    skipBytes(sal_Int32 nBytesToSkip) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual sal_Int32   SAL_CALL    available() throw(stario::NotConnectedException, staruno::RuntimeException);
    virtual void        SAL_CALL    closeInput() throw(stario::NotConnectedException, staruno::RuntimeException);
};

//==================================================================
// FmUnoOutStream,
// Datensenke fuer Files
//==================================================================
typedef ::cppu::WeakImplHelper1<stario::XOutputStream> OutputStreamWrapper_Base;
    // needed for some compilers
class OSLOutputStreamWrapper : public OutputStreamWrapper_Base
{
    ::osl::File&        rFile;

public:
    OSLOutputStreamWrapper(::osl::File& _rFile) :rFile(_rFile) { }

// UNO Anbindung
    virtual void            SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)
        { OutputStreamWrapper_Base::acquire(); }
    virtual void            SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)
        { OutputStreamWrapper_Base::release(); }
    virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException)
        { return OutputStreamWrapper_Base::queryInterface(_rType); }

// stario::XOutputStream
    virtual void SAL_CALL writeBytes(const staruno::Sequence< sal_Int8 >& aData) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void SAL_CALL flush() throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void SAL_CALL closeOutput() throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
};

}   // namespace comphelper


#endif // _COMPHELPER_STREAM_OSLFILEWRAPPER_HXX_

