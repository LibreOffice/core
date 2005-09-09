/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ftpinpstr.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:36:17 $
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
#define _FTP_FTPINPSTR_HXX_


#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <stdio.h>


namespace ftp {


    /** Implements a seekable InputStream
     *  working on a buffer.
     */


    namespace css = com::sun::star;


    class FTPInputStream
        : public cppu::OWeakObject,
          public com::sun::star::io::XInputStream,
          public com::sun::star::io::XSeekable
    {
    public:

        /** Defines the storage kind found
         *  on which the inputstream acts.
         */

        FTPInputStream(FILE* tmpfl = 0);

        ~FTPInputStream();

        virtual css::uno::Any SAL_CALL queryInterface(const css::uno::Type& rType)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL acquire(void) throw();

        virtual void SAL_CALL release(void) throw();

        virtual sal_Int32 SAL_CALL
        readBytes(css::uno::Sequence< sal_Int8 >& aData,
                  sal_Int32 nBytesToRead)
            throw( css::io::NotConnectedException,
                   css::io::BufferSizeExceededException,
                   css::io::IOException,
                   css::uno::RuntimeException);

        virtual sal_Int32 SAL_CALL
        readSomeBytes(css::uno::Sequence< sal_Int8 >& aData,
                      sal_Int32 nMaxBytesToRead )
            throw( css::io::NotConnectedException,
                   css::io::BufferSizeExceededException,
                   css::io::IOException,
                   css::uno::RuntimeException);

        virtual void SAL_CALL
        skipBytes(sal_Int32 nBytesToSkip)
            throw(css::io::NotConnectedException,
                  css::io::BufferSizeExceededException,
                  css::io::IOException,
                  css::uno::RuntimeException );

        virtual sal_Int32 SAL_CALL
        available(void)
            throw(css::io::NotConnectedException,
                  css::io::IOException,
                  css::uno::RuntimeException );

        virtual void SAL_CALL
        closeInput(void)
            throw(css::io::NotConnectedException,
                  css::io::IOException,
                  css::uno::RuntimeException);


        /** XSeekable
         */

        virtual void SAL_CALL
        seek(sal_Int64 location)
            throw(css::lang::IllegalArgumentException,
                  css::io::IOException,
                  css::uno::RuntimeException);


        virtual sal_Int64 SAL_CALL
        getPosition(void)
            throw(css::io::IOException,
                  css::uno::RuntimeException);


        virtual sal_Int64 SAL_CALL
        getLength(void)
            throw(css::io::IOException,
                  css::uno::RuntimeException);

        // additional

//          void append(const void* pBuffer,size_t size,size_t nmemb);

    private:

        osl::Mutex m_aMutex;
        FILE* m_tmpfl;
        sal_Int64 m_nLength;
    };


}

#endif
