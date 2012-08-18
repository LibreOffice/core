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

#ifndef INCLUDED_BINARYURP_SOURCE_INCOMINGREQUEST_HXX
#define INCLUDED_BINARYURP_SOURCE_INCOMINGREQUEST_HXX

#include "sal/config.h"

#include <vector>

#include "boost/noncopyable.hpp"
#include "rtl/byteseq.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typedescription.hxx"
#include "uno/dispatcher.hxx"

namespace binaryurp {
    class BinaryAny;
    class Bridge;
}

namespace binaryurp {

class IncomingRequest: private boost::noncopyable {
public:
    IncomingRequest(
        rtl::Reference< Bridge > const & bridge, rtl::ByteSequence const & tid,
        OUString const & oid,
        com::sun::star::uno::UnoInterfaceReference const & object,
        com::sun::star::uno::TypeDescription const & type,
        sal_uInt16 functionId, bool synchronous,
        com::sun::star::uno::TypeDescription const & member, bool setter,
        std::vector< BinaryAny > const & inArguments, bool currentContextMode,
        com::sun::star::uno::UnoInterfaceReference const & currentContext);

    ~IncomingRequest();

    void execute() const;

private:
    bool execute_throw(
        BinaryAny * returnValue, std::vector< BinaryAny > * outArguments) const;

    rtl::Reference< Bridge > bridge_;
    rtl::ByteSequence tid_;
    OUString oid_; // initial object queryInterface; release
    com::sun::star::uno::UnoInterfaceReference object_;
    com::sun::star::uno::TypeDescription type_;
    sal_uInt16 functionId_;
    bool synchronous_;
    com::sun::star::uno::TypeDescription member_;
    bool setter_;
    std::vector< BinaryAny > inArguments_;
    bool currentContextMode_;
    com::sun::star::uno::UnoInterfaceReference currentContext_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
