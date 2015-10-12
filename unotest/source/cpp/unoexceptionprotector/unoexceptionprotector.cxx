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

#include <string>
#include <iostream>

#include "boost/noncopyable.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "cppuhelper/exc_hlp.hxx"
#include "cppunit/Message.h"
#include "osl/thread.h"
#include "rtl/string.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "cppunittester/protectorfactory.hxx"

namespace {

// Best effort conversion:
std::string convert(OUString const & s16) {
    OString s8(OUStringToOString(s16, osl_getThreadTextEncoding()));
    static_assert(sizeof (sal_Int32) <= sizeof (std::string::size_type), "got to be at least equal");
        // ensure following cast is legitimate
    return std::string(
        s8.getStr(), static_cast< std::string::size_type >(s8.getLength()));
}

class Prot : public CppUnit::Protector, private boost::noncopyable
{
public:
    Prot() {}

    virtual ~Prot() {}

    virtual bool protect(
        CppUnit::Functor const & functor,
        CppUnit::ProtectorContext const & context) override;
};

bool Prot::protect(
    CppUnit::Functor const & functor, CppUnit::ProtectorContext const & context)
{
    try {
        return functor();
    } catch (const css::uno::Exception &e) {
        css::uno::Any a(cppu::getCaughtException());
        reportError(
            context,
            CppUnit::Message(
                convert(
                    "An uncaught exception of type " + a.getValueTypeName()),
                convert(e.Message)));
    }
    return false;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT CppUnit::Protector * SAL_CALL
unoexceptionprotector() {
    return new Prot;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
