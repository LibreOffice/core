/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
        std::vector< unsigned char > * buffer, OUString const & oid);

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
