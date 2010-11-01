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

#ifndef INCLUDED_CONFIGMGR_SOURCE_TYPE_HXX
#define INCLUDED_CONFIGMGR_SOURCE_TYPE_HXX

#include "sal/config.h"

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
    class Type;
} } } }

namespace configmgr {

enum Type {
    TYPE_ERROR, TYPE_NIL, TYPE_ANY, TYPE_BOOLEAN, TYPE_SHORT, TYPE_INT,
    TYPE_LONG, TYPE_DOUBLE, TYPE_STRING, TYPE_HEXBINARY, TYPE_BOOLEAN_LIST,
    TYPE_SHORT_LIST, TYPE_INT_LIST, TYPE_LONG_LIST, TYPE_DOUBLE_LIST,
    TYPE_STRING_LIST, TYPE_HEXBINARY_LIST };

bool isListType(Type type);

Type elementType(Type type);

com::sun::star::uno::Type mapType(Type type);

Type getDynamicType(com::sun::star::uno::Any const & value);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
