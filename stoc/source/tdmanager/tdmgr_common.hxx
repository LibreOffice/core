/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

#ifndef _STOC_TDMGR_COMMON_HXX
#define _STOC_TDMGR_COMMON_HXX

#include <rtl/unload.h>

#include "com/sun/star/reflection/XTypeDescription.hpp"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
#define ARLEN(x) (sizeof (x) / sizeof *(x))


namespace css = ::com::sun::star;

namespace stoc_tdmgr
{
    extern rtl_StandardModuleCount g_moduleCount;

struct IncompatibleTypeException
{
    ::rtl::OUString m_cause;
    IncompatibleTypeException( ::rtl::OUString const & cause )
        : m_cause( cause ) {}
};

void check(
    css::uno::Reference<css::reflection::XTypeDescription> const & xNewTD,
    css::uno::Reference<css::reflection::XTypeDescription> const & xExistingTD,
    ::rtl::OUString const & context = ::rtl::OUString() );
/* throw (css::uno::RuntimeException, IncompatibleTypeException) */

} // namespace stoc_tdmgr

#endif /* _STOC_TDMGR_COMMON_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
