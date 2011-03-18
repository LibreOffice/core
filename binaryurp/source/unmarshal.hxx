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

#ifndef INCLUDED_BINARYURP_SOURCE_UNMARSHAL_HXX
#define INCLUDED_BINARYURP_SOURCE_UNMARSHAL_HXX

#include "sal/config.h"

#include <vector>

#include "boost/noncopyable.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "rtl/ref.hxx"
#include "sal/types.h"
#include "typelib/typedescription.hxx"

namespace binaryurp {
    class BinaryAny;
    class Bridge;
    struct ReaderState;
}
namespace com { namespace sun { namespace star { namespace uno {
    class TypeDescription;
} } } }
namespace rtl {
    class ByteSequecne;
    class OUString;
}

namespace binaryurp {

class Unmarshal: private boost::noncopyable {
public:
    Unmarshal(
        rtl::Reference< Bridge > const & bridge, ReaderState & state,
        com::sun::star::uno::Sequence< sal_Int8 > const & buffer);

    ~Unmarshal();

    sal_uInt8 read8();

    sal_uInt16 read16();

    sal_uInt32 read32();

    com::sun::star::uno::TypeDescription readType();

    rtl::OUString readOid();

    rtl::ByteSequence readTid();

    BinaryAny readValue(com::sun::star::uno::TypeDescription const & type);

    void done() const;

private:
    void check(sal_Int32 size) const;

    sal_uInt32 readCompressed();

    sal_uInt16 readCacheIndex();

    sal_uInt64 read64();

    rtl::OUString readString();

    BinaryAny readSequence(com::sun::star::uno::TypeDescription const & type);

    void readMemberValues(
        com::sun::star::uno::TypeDescription const & type,
        std::vector< BinaryAny > * values);

    rtl::Reference< Bridge > bridge_;
    ReaderState & state_;
    com::sun::star::uno::Sequence< sal_Int8 > buffer_;
    sal_uInt8 const * data_;
    sal_uInt8 const * end_;
};

}

#endif
