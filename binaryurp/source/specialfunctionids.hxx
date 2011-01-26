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

#ifndef INCLUDED_BINARYURP_SOURCE_SPECIALFUNCTIONIDS_HXX
#define INCLUDED_BINARYURP_SOURCE_SPECIALFUNCTIONIDS_HXX

#include "sal/config.h"

namespace binaryurp {

enum SpecialFunctionIds {
    SPECIAL_FUNCTION_ID_QUERY_INTERFACE = 0,

    SPECIAL_FUNCTION_ID_RESERVED = 1,

    SPECIAL_FUNCTION_ID_RELEASE = 2,

    SPECIAL_FUNCTION_ID_REQUEST_CHANGE = 4,

    SPECIAL_FUNCTION_ID_COMMIT_CHANGE = 5
};

}

#endif
