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
