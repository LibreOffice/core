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

#if ! defined INCLUDED_DP_RESOURCE_H
#define INCLUDED_DP_RESOURCE_H

#include "tools/resmgr.hxx"
#include "tools/string.hxx"
#include "tools/resid.hxx"
#include "com/sun/star/lang/Locale.hpp"
#include "dp_misc.h"
#include <memory>
#include "dp_misc_api.hxx"

namespace dp_misc {

//==============================================================================
ResId getResId( sal_uInt16 id );

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC String getResourceString( sal_uInt16 id );

template <typename Unique, sal_uInt16 id>
struct StaticResourceString :
        public ::rtl::StaticWithInit<const ::rtl::OUString, Unique> {
    const ::rtl::OUString operator () () { return getResourceString(id); }
};

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
::com::sun::star::lang::Locale toLocale( ::rtl::OUString const & slang );

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
::com::sun::star::lang::Locale getOfficeLocale();

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
::rtl::OUString getOfficeLocaleString();

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
