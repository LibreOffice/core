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

#ifndef INCLUDED_BINARYURP_SOURCE_READER_HXX
#define INCLUDED_BINARYURP_SOURCE_READER_HXX

#include "sal/config.h"

#include "rtl/byteseq.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "salhelper/thread.hxx"
#include "typelib/typedescription.hxx"

#include "readerstate.hxx"

namespace binaryurp {
    class Bridge;
    class Unmarshal;
}

namespace binaryurp {

class Reader: public salhelper::Thread {
public:
    explicit Reader(rtl::Reference< Bridge > const & bridge);

private:
    virtual ~Reader();

    virtual void execute();

    void readMessage(Unmarshal & unmarshal);

    void readReplyMessage(Unmarshal & unmarshal, sal_uInt8 flags1);

    rtl::ByteSequence getTid(Unmarshal & unmarshal, bool newTid) const;

    rtl::Reference< Bridge > bridge_;
    com::sun::star::uno::TypeDescription lastType_;
    OUString lastOid_;
    rtl::ByteSequence lastTid_;
    ReaderState state_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
