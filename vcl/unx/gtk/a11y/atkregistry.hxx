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

#ifndef __ATK_REGISTRY_HXX__
#define __ATK_REGISTRY_HXX__

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <atk/atk.h>

AtkObject * ooo_wrapper_registry_get(const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxAccessible);

void ooo_wrapper_registry_add(const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxAccessible, AtkObject *obj);

void ooo_wrapper_registry_remove(::com::sun::star::accessibility::XAccessible *pAccessible);

#endif // __ATK_REGISTRY_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
