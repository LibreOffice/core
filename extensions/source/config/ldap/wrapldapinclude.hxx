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

#ifdef WNT
#pragma warning (push,1)
#pragma warning (disable:4668)
#endif

#ifdef WITH_OPENLDAP
#include <ldap.h>
#ifndef LDAP_API
#    define LDAP_API(rt) rt
#endif
#ifndef LDAP_CALL
#    define LDAP_CALL
#endif
#else
#ifndef LDAP_INCLUDED
#define LDAP_INCLUDED
#include <ldap/ldap.h>
#endif // LDAP_INCLUDED
#endif

#ifdef WNT
#pragma warning (pop)
#endif // WNT

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
