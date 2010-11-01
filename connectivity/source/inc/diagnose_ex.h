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

#ifndef CONNECTIVITY_DIAGNOSE_EX_H
#define CONNECTIVITY_DIAGNOSE_EX_H

#if OSL_DEBUG_LEVEL > 0
    #define OSL_VERIFY_RES( expression, fail_message ) \
        OSL_ENSURE( expression, fail_message )
    #define OSL_VERIFY_EQUALS( expression, compare, fail_message ) \
        OSL_ENSURE( expression == compare, fail_message )
#else
    #define OSL_VERIFY_RES( expression, fail_message ) \
        (void)(expression)
    #define OSL_VERIFY_EQUALS( expression, compare, fail_message ) \
        (void)(expression)
#endif

#define OSL_UNUSED( expression ) \
    (void)(expression)

#endif // CONNECTIVITY_DIAGNOSE_EX_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
