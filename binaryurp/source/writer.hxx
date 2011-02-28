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

#ifndef INCLUDED_BINARYURP_SOURCE_WRITER_HXX
#define INCLUDED_BINARYURP_SOURCE_WRITER_HXX

#include "sal/config.h"

#include <cstddef>
#include <deque>
#include <vector>

#include "boost/noncopyable.hpp"
#include "osl/conditn.hxx"
#include "osl/mutex.hxx"
#include "osl/thread.hxx"
#include "rtl/byteseq.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "salhelper/simplereferenceobject.hxx"
#include "typelib/typedescription.hxx"
#include "uno/dispatcher.hxx"

#include "binaryany.hxx"
#include "marshal.hxx"
#include "writerstate.hxx"

namespace binaryurp { class Bridge; }

namespace binaryurp {

class Writer:
    public osl::Thread, public salhelper::SimpleReferenceObject,
    private boost::noncopyable
{
public:
    static void * operator new(std::size_t size)
    { return Thread::operator new(size); }

    static void operator delete(void * pointer)
    { Thread::operator delete(pointer); }

    explicit Writer(rtl::Reference< Bridge > const & bridge);

    // Only called from Bridge::reader_ thread, and only before Bridge::writer_
    // thread is unblocked:
    void sendDirectRequest(
        rtl::ByteSequence const & tid, rtl::OUString const & oid,
        com::sun::star::uno::TypeDescription const & type,
        com::sun::star::uno::TypeDescription const & member,
        std::vector< BinaryAny > const & inArguments);

    // Only called from Bridge::reader_ thread, and only before Bridge::writer_
    // thread is unblocked:
    void sendDirectReply(
        rtl::ByteSequence const & tid,
        com::sun::star::uno::TypeDescription const & member,
        bool exception, BinaryAny const & returnValue,
        std::vector< BinaryAny > const & outArguments);

    void queueRequest(
        rtl::ByteSequence const & tid, rtl::OUString const & oid,
        com::sun::star::uno::TypeDescription const & type,
        com::sun::star::uno::TypeDescription const & member,
        std::vector< BinaryAny > const & inArguments);

    void queueReply(
        rtl::ByteSequence const & tid,
        com::sun::star::uno::TypeDescription const & member, bool setter,
        bool exception, BinaryAny const & returnValue,
        std::vector< BinaryAny > const & outArguments,
        bool setCurrentContextMode);

    void unblock();

    void stop();

private:
    virtual ~Writer();

    virtual void SAL_CALL run();

    virtual void SAL_CALL onTerminated();

    void sendRequest(
        rtl::ByteSequence const & tid, rtl::OUString const & oid,
        com::sun::star::uno::TypeDescription const & type,
        com::sun::star::uno::TypeDescription const & member,
        std::vector< BinaryAny > const & inArguments, bool currentContextMode,
        com::sun::star::uno::UnoInterfaceReference const & currentContext);

    void sendReply(
        rtl::ByteSequence const & tid,
        com::sun::star::uno::TypeDescription const & member, bool setter,
        bool exception, BinaryAny const & returnValue,
        std::vector< BinaryAny > const & outArguments);

    void sendMessage(std::vector< unsigned char > const & buffer);

    struct Item {
        Item();

        // Request:
        Item(
            rtl::ByteSequence const & theTid, rtl::OUString const & theOid,
            com::sun::star::uno::TypeDescription const & theType,
            com::sun::star::uno::TypeDescription const & theMember,
            std::vector< BinaryAny > const & inArguments,
            com::sun::star::uno::UnoInterfaceReference const &
                theCurrentContext);

        // Reply:
        Item(
            rtl::ByteSequence const & theTid,
            com::sun::star::uno::TypeDescription const & theMember,
            bool theSetter, bool theException, BinaryAny const & theReturnValue,
            std::vector< BinaryAny > const & outArguments,
            bool theSetCurrentContextMode);

        bool request;

        rtl::ByteSequence tid; // request + reply

        rtl::OUString oid; // request

        com::sun::star::uno::TypeDescription type; // request

        com::sun::star::uno::TypeDescription member; // request + reply

        bool setter; // reply

        std::vector< BinaryAny > arguments;
            // request: inArguments; reply: outArguments

        bool exception; // reply

        BinaryAny returnValue; // reply

        com::sun::star::uno::UnoInterfaceReference currentContext; // request

        bool setCurrentContextMode; // reply
    };

    rtl::Reference< Bridge > bridge_;
    WriterState state_;
    Marshal marshal_;
    com::sun::star::uno::TypeDescription lastType_;
    rtl::OUString lastOid_;
    rtl::ByteSequence lastTid_;
    osl::Condition unblocked_;
    osl::Condition items_;

    osl::Mutex mutex_;
    std::deque< Item > queue_;
    bool stop_;
};

}

#endif
