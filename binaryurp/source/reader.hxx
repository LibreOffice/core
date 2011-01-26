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
