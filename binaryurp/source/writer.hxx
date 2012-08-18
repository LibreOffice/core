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

#ifndef INCLUDED_BINARYURP_SOURCE_WRITER_HXX
#define INCLUDED_BINARYURP_SOURCE_WRITER_HXX

#include "sal/config.h"

#include <deque>
#include <vector>

#include "osl/conditn.hxx"
#include "osl/mutex.hxx"
#include "rtl/byteseq.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "salhelper/thread.hxx"
#include "typelib/typedescription.hxx"
#include "uno/dispatcher.hxx"

#include "binaryany.hxx"
#include "marshal.hxx"
#include "writerstate.hxx"

namespace binaryurp { class Bridge; }

namespace binaryurp {

class Writer: public salhelper::Thread
{
public:
    explicit Writer(rtl::Reference< Bridge > const & bridge);

    // Only called from Bridge::reader_ thread, and only before Bridge::writer_
    // thread is unblocked:
    void sendDirectRequest(
        rtl::ByteSequence const & tid, OUString const & oid,
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
        rtl::ByteSequence const & tid, OUString const & oid,
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

    virtual void execute();

    void sendRequest(
        rtl::ByteSequence const & tid, OUString const & oid,
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
            rtl::ByteSequence const & theTid, OUString const & theOid,
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

        OUString oid; // request

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
    OUString lastOid_;
    rtl::ByteSequence lastTid_;
    osl::Condition unblocked_;
    osl::Condition items_;

    osl::Mutex mutex_;
    std::deque< Item > queue_;
    bool stop_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
