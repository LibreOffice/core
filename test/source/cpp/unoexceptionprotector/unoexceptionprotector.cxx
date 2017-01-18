/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
