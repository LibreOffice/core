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

#include "sal/config.h"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "uno/current_context.h"
#include "uno/dispatcher.hxx"
#include "uno/lbnames.h"

#include "currentcontext.hxx"

namespace binaryurp {

namespace {

namespace css = com::sun::star;

}

namespace current_context {

css::uno::UnoInterfaceReference get() {
    css::uno::UnoInterfaceReference cc;
    if (!uno_getCurrentContext(
            reinterpret_cast< void ** >(&cc.m_pUnoI),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO)).pData, 0))
    {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("uno_getCurrentContext failed")),
            css::uno::Reference< css::uno::XInterface >());
    }
    return cc;
}

void set(css::uno::UnoInterfaceReference const & value) {
    css::uno::UnoInterfaceReference cc(value);
    if (!uno_setCurrentContext(
            cc.m_pUnoI,
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO)).pData, 0))
    {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("uno_setCurrentContext failed")),
            css::uno::Reference< css::uno::XInterface >());
    }
}

}

}
