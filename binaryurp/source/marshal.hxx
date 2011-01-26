/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2011 Oracle and/or its affiliates.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
*
************************************************************************/

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
