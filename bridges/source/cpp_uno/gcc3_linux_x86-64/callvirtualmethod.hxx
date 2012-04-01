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

#ifndef INCLUDED_BRIDGES_SOURCE_CPP_UNO_GCC3_LINUX_X86_64_CALLVIRTUALMETHOD_HXX
#define INCLUDED_BRIDGES_SOURCE_CPP_UNO_GCC3_LINUX_X86_64_CALLVIRTUALMETHOD_HXX

#include "sal/config.h"

#include "cppu/macros.hxx"
#include "sal/types.h"
#include "typelib/typedescription.h"

namespace CPPU_CURRENT_NAMESPACE {

void callVirtualMethod(
    void * pThis, sal_uInt32 nVtableIndex, void * pRegisterReturn,
    typelib_TypeDescriptionReference * pReturnTypeRef, bool bSimpleReturn,
    sal_uInt64 *pStack, sal_uInt32 nStack, sal_uInt64 *pGPR, sal_uInt32 nGPR,
    double * pFPR, sal_uInt32 nFPR);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
