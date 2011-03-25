/*************************************************************************
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
************************************************************************/

#include "precompiled_test.hxx"
#include "sal/config.h"

#include <limits>
#include <string>

#include "boost/noncopyable.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "cppuhelper/exc_hlp.hxx"
#include "osl/thread.h"
#include "rtl/string.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "preextstl.h"
#include "cppunit/Message.h"
#include "cppunit/Protector.h"
#include "postextstl.h"

namespace {

namespace css = com::sun::star;

// Best effort conversion:
std::string convert(rtl::OUString const & s16) {
    rtl::OString s8(rtl::OUStringToOString(s16, osl_getThreadTextEncoding()));
    return std::string(
        s8.getStr(),
        ((static_cast< sal_uInt32 >(s8.getLength())
          > std::numeric_limits< std::string::size_type >::max())
         ? std::numeric_limits< std::string::size_type >::max()
         : static_cast< std::string::size_type >(s8.getLength())));
}

class Prot: public CppUnit::Protector, private boost::noncopyable {
public:
    Prot() {}

    virtual ~Prot() {}

    virtual bool protect(
        CppUnit::Functor const & functor,
        CppUnit::ProtectorContext const & context);
};

bool Prot::protect(
    CppUnit::Functor const & functor, CppUnit::ProtectorContext const & context)
{
    try {
        return functor();
    } catch (css::uno::Exception & e) {
        css::uno::Any a(cppu::getCaughtException());
        reportError(
            context,
            CppUnit::Message(
                convert(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "uncaught exception of type "))
                    + a.getValueTypeName()),
                convert(e.Message)));
    }
    return false;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT CppUnit::Protector * SAL_CALL
unoexceptionprotector() {
    return new Prot;
}
