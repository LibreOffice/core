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

#ifndef INCLUDED_BINARYURP_SOURCE_PROXY_HXX
#define INCLUDED_BINARYURP_SOURCE_PROXY_HXX

#include <sal/config.h>

#include <atomic>
#include <cstddef>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <typelib/typedescription.h>
#include <typelib/typedescription.hxx>
#include <uno/any2.h>
#include <uno/dispatcher.h>

namespace binaryurp { class Bridge; }
namespace com::sun::star::uno { class UnoInterfaceReference; }

namespace binaryurp {

class Proxy: public uno_Interface {
public:
    Proxy(
        rtl::Reference< Bridge > const & bridge, OUString const & oid,
        com::sun::star::uno::TypeDescription const & type);

    const OUString& getOid() const { return oid_;}

    const com::sun::star::uno::TypeDescription& getType() const { return type_;}

    void do_acquire();

    void do_release();

    void do_free();

    void do_dispatch(
        typelib_TypeDescription const * member, void * returnValue,
        void ** arguments, uno_Any ** exception) const;

    static bool isProxy(
        rtl::Reference< Bridge > const & bridge,
        com::sun::star::uno::UnoInterfaceReference const & object,
        OUString * oid);

private:
    Proxy(const Proxy&) = delete;
    Proxy& operator=(const Proxy&) = delete;

    ~Proxy();

    void do_dispatch_throw(
        typelib_TypeDescription const * member, void * returnValue,
        void ** arguments, uno_Any ** exception) const;

    bool isProxy(rtl::Reference< Bridge > const & bridge, OUString * oid)
        const;

    rtl::Reference< Bridge > bridge_;
    OUString oid_;
    com::sun::star::uno::TypeDescription type_;
    std::atomic<std::size_t> references_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
