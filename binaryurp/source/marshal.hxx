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



#ifndef INCLUDED_BINARYURP_SOURCE_MARSHAL_HXX
#define INCLUDED_BINARYURP_SOURCE_MARSHAL_HXX

#include "sal/config.h"

#include <vector>

#include "boost/noncopyable.hpp"
#include "rtl/byteseq.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typedescription.hxx"

namespace binaryurp {
    class BinaryAny;
    class Bridge;
    struct WriterState;
}

namespace binaryurp {

class Marshal: private boost::noncopyable {
public:
    Marshal(rtl::Reference< Bridge > const & bridge, WriterState & state);

    ~Marshal();

    static void write8(std::vector< unsigned char > * buffer, sal_uInt8 value);

    static void write16(
        std::vector< unsigned char > * buffer, sal_uInt16 value);

    static void write32(
        std::vector< unsigned char > * buffer, sal_uInt32 value);

    void writeValue(
        std::vector< unsigned char > * buffer,
        com::sun::star::uno::TypeDescription const & type,
        BinaryAny const & value);

    void writeType(
        std::vector< unsigned char > * buffer,
        com::sun::star::uno::TypeDescription const & value);

    void writeOid(
        std::vector< unsigned char > * buffer, rtl::OUString const & oid);

    void writeTid(
        std::vector< unsigned char > * buffer, rtl::ByteSequence const & tid);

private:
    void writeValue(
        std::vector< unsigned char > * buffer,
        com::sun::star::uno::TypeDescription const & type, void const * value);

    void writeMemberValues(
        std::vector< unsigned char > * buffer,
        com::sun::star::uno::TypeDescription const & type,
        void const * aggregateValue);

    rtl::Reference< Bridge > bridge_;
    WriterState & state_;
};

}

#endif
