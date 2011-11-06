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


#ifndef _COMPHELPER_STREAM_OSLFILEWRAPPER_HXX_
#define _COMPHELPER_STREAM_OSLFILEWRAPPER_HXX_

#include <osl/mutex.hxx>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/implbase1.hxx>
#include <osl/file.hxx>
#include "comphelper/comphelperdllapi.h"

namespace comphelper
{
    namespace stario    = ::com::sun::star::io;
    namespace staruno   = ::com::sun::star::uno;

//==================================================================
// FmUnoIOStream,
// stream zum schreiben un lesen von Daten, basieren  auf File
//==================================================================
struct InputStreamWrapper_Base : public ::cppu::WeakImplHelper1<stario::XInputStream>
{};

class COMPHELPER_DLLPUBLIC OSLInputStreamWrapper : public InputStreamWrapper_Base
{
    ::osl::Mutex    m_aMutex;
    ::osl::File*    m_pFile;
    sal_Bool        m_bFileOwner : 1;

public:
    OSLInputStreamWrapper(::osl::File& _rStream);
    OSLInputStreamWrapper(::osl::File* pStream, sal_Bool bOwner=sal_False);
    virtual ~OSLInputStreamWrapper();

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
struct OutputStreamWrapper_Base : public ::cppu::WeakImplHelper1<stario::XOutputStream>
{};

class COMPHELPER_DLLPUBLIC OSLOutputStreamWrapper : public OutputStreamWrapper_Base
{
    ::osl::File&        rFile;

public:
    OSLOutputStreamWrapper(::osl::File& _rFile) :rFile(_rFile) { }

// stario::XOutputStream
    virtual void SAL_CALL writeBytes(const staruno::Sequence< sal_Int8 >& aData) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void SAL_CALL flush() throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void SAL_CALL closeOutput() throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
};

}   // namespace comphelper


#endif // _COMPHELPER_STREAM_OSLFILEWRAPPER_HXX_

