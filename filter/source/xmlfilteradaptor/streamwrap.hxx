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


#ifndef _OSL_FILE_WRAPPER_HXX_
#define _OSL_FILE_WRAPPER_HXX_

#include <osl/mutex.hxx>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <cppuhelper/implbase1.hxx>

#define DECLARE_UNO3_AGG_DEFAULTS(classname, baseclass) \
virtual void            SAL_CALL acquire() throw() { baseclass::acquire(); } \
virtual void            SAL_CALL release() throw() { baseclass::release(); }    \
virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException) \
{ return baseclass::queryInterface(_rType); } \
void            SAL_CALL PUT_SEMICOLON_AT_THE_END()

namespace osl
{
    class File;
}

namespace foo
{
    namespace stario    = ::com::sun::star::io;
    namespace staruno   = ::com::sun::star::uno;

//==================================================================
//= OOutputStreamWrapper
//==================================================================
typedef ::cppu::WeakImplHelper1<stario::XOutputStream> OutputStreamWrapper_Base;
    // needed for some compilers
class OOutputStreamWrapper : public OutputStreamWrapper_Base
{
    ::osl::File&        rStream;

public:
    OOutputStreamWrapper(::osl::File& _rStream) :rStream(_rStream) { }

// UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OOutputStreamWrapper, OutputStreamWrapper_Base);

// stario::XOutputStream
    virtual void SAL_CALL writeBytes(const staruno::Sequence< sal_Int8 >& aData) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void SAL_CALL flush() throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void SAL_CALL closeOutput() throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
};

}   // namespace utl


#endif // _UTL_STREAM_WRAPPER_HXX_

