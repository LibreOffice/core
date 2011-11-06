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



#ifndef ODMA_INPUTSTREAM_HXX
#define ODMA_INPUTSTREAM_HXX

#include <osl/mutex.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <cppuhelper/implbase5.hxx>

#include "rtl/ref.hxx"

namespace ucbhelper
{
    class Content;
}
namespace odma
{
    typedef ::cppu::WeakImplHelper5<    ::com::sun::star::io::XInputStream,
                                        ::com::sun::star::io::XStream,
                                        ::com::sun::star::io::XTruncate,
                                        ::com::sun::star::io::XSeekable,
                                        ::com::sun::star::io::XOutputStream> OOdmaStreamBase;

    class ContentProvider;
    class ContentProperties;
    class OOdmaStream : public OOdmaStreamBase
    {
        ::osl::Mutex    m_aMutex;
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream>  m_xOutput;
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XTruncate>      m_xTruncate;
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>   m_xInput;
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XSeekable>      m_xInputSeek;

        ::rtl::Reference<ContentProperties> m_aProp;
        ::ucbhelper::Content*               m_pContent;
        ContentProvider*                    m_pProvider;
        sal_Bool                            m_bInputStreamCalled;
        sal_Bool                            m_bOutputStreamCalled;
        sal_Bool                            m_bModified;

        void ensureInputStream()  throw( ::com::sun::star::io::IOException );
        void ensureOutputStream()  throw( ::com::sun::star::io::IOException );
        void SAL_CALL closeStream() throw(  ::com::sun::star::io::NotConnectedException,
                                            ::com::sun::star::io::IOException,
                                            ::com::sun::star::uno::RuntimeException );
    public:
        OOdmaStream(::ucbhelper::Content* _pContent,
                    ContentProvider* _pProvider,
                    const ::rtl::Reference<ContentProperties>& _rProp);
        virtual ~OOdmaStream();
        // com::sun::star::io::XInputStream
        virtual sal_Int32 SAL_CALL readBytes( ::com::sun::star::uno::Sequence<sal_Int8>& aData, sal_Int32 nBytesToRead )
            throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException,
                  ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

        virtual sal_Int32 SAL_CALL readSomeBytes( ::com::sun::star::uno::Sequence<sal_Int8>& aData, sal_Int32 nMaxBytesToRead )
            throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException,
                  ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
            throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException,
                  ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

        virtual sal_Int32 SAL_CALL available(  )
            throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL closeInput(  )
            throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

        // com::sun::star::io::XStream
        virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream > SAL_CALL getInputStream(  ) throw( com::sun::star::uno::RuntimeException );
        virtual com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > SAL_CALL getOutputStream(  ) throw( com::sun::star::uno::RuntimeException );

        // com::sun::star::io::XOutputStream
        void SAL_CALL writeBytes( const com::sun::star::uno::Sequence< sal_Int8 >& aData )
                    throw( com::sun::star::io::NotConnectedException,
                           com::sun::star::io::BufferSizeExceededException,
                           com::sun::star::io::IOException,
                           com::sun::star::uno::RuntimeException);

        void SAL_CALL flush()
                    throw( com::sun::star::io::NotConnectedException,
                           com::sun::star::io::BufferSizeExceededException,
                           com::sun::star::io::IOException,
                           com::sun::star::uno::RuntimeException);
        void SAL_CALL closeOutput()
                    throw( com::sun::star::io::NotConnectedException,
                           com::sun::star::io::IOException,
                           com::sun::star::uno::RuntimeException );
        // XTruncate
        virtual void SAL_CALL truncate( void )
            throw( com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );
        // XSeekable
        void SAL_CALL seek(sal_Int64 location )
            throw( com::sun::star::lang::IllegalArgumentException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        sal_Int64 SAL_CALL getPosition()
            throw( com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        sal_Int64 SAL_CALL getLength()
            throw( com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );
    };
}
#endif // ODMA_INPUTSTREAM_HXX
