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



#ifndef INCLUDED_BINARYURP_SOURCE_READER_HXX
#define INCLUDED_BINARYURP_SOURCE_READER_HXX

#include "sal/config.h"

#include <cstddef>

#include "boost/noncopyable.hpp"
#include "osl/thread.hxx"
#include "rtl/byteseq.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "salhelper/simplereferenceobject.hxx"
#include "typelib/typedescription.hxx"

#include "readerstate.hxx"

namespace binaryurp {
    class BinaryAny;
    class Bridge;
    class Unmarshal;
}

namespace binaryurp {

class Reader:
    public osl::Thread, public salhelper::SimpleReferenceObject,
    private boost::noncopyable
{
public:
    static void * operator new(std::size_t size)
    { return Thread::operator new(size); }

    static void operator delete(void * pointer)
    { Thread::operator delete(pointer); }

    explicit Reader(rtl::Reference< Bridge > const & bridge);

private:
    virtual ~Reader();

    virtual void SAL_CALL run();

    virtual void SAL_CALL onTerminated();

    void readMessage(Unmarshal & unmarshal);

    void readReplyMessage(Unmarshal & unmarshal, sal_uInt8 flags1);

    rtl::ByteSequence getTid(Unmarshal & unmarshal, bool newTid) const;

    rtl::Reference< Bridge > bridge_;
    com::sun::star::uno::TypeDescription lastType_;
    rtl::OUString lastOid_;
    rtl::ByteSequence lastTid_;
    ReaderState state_;
};

}

#endif
