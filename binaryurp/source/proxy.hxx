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

#ifndef INCLUDED_BINARYURP_SOURCE_PROXY_HXX
#define INCLUDED_BINARYURP_SOURCE_PROXY_HXX

#include "sal/config.h"

#include "boost/noncopyable.hpp"
#include "osl/interlck.h"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "typelib/typedescription.h"
#include "typelib/typedescription.hxx"
#include "uno/any2.h"
#include "uno/dispatcher.h"

namespace binaryurp { class Bridge; }
namespace com { namespace sun { namespace star { namespace uno {
    class UnoInterfaceReference;
} } } }

namespace binaryurp {

class Proxy: public uno_Interface, private boost::noncopyable {
public:
    Proxy(
        rtl::Reference< Bridge > const & bridge, rtl::OUString const & oid,
        com::sun::star::uno::TypeDescription const & type);

    rtl::OUString getOid() const;

    com::sun::star::uno::TypeDescription getType() const;

    void do_acquire();

    void do_release();

    void do_free();

    void do_dispatch(
        typelib_TypeDescription const * member, void * returnValue,
        void ** arguments, uno_Any ** exception) const;

    static bool isProxy(
        rtl::Reference< Bridge > const & bridge,
        com::sun::star::uno::UnoInterfaceReference const & object,
        rtl::OUString * oid);

private:
    ~Proxy();

    void do_dispatch_throw(
        typelib_TypeDescription const * member, void * returnValue,
        void ** arguments, uno_Any ** exception) const;

    bool isProxy(rtl::Reference< Bridge > const & bridge, rtl::OUString * oid)
        const;

    rtl::Reference< Bridge > bridge_;
    rtl::OUString oid_;
    com::sun::star::uno::TypeDescription type_;
    oslInterlockedCount references_;
};

}

#endif
