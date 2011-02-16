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

#ifndef INCLUDED_BINARYURP_SOURCE_OUTGOINGREQUEST_HXX
#define INCLUDED_BINARYURP_SOURCE_OUTGOINGREQUEST_HXX

#include "sal/config.h"

#include "typelib/typedescription.hxx"

namespace binaryurp {

struct OutgoingRequest {
    enum Kind { KIND_NORMAL, KIND_REQUEST_CHANGE, KIND_COMMIT_CHANGE };

    OutgoingRequest(
        Kind theKind, com::sun::star::uno::TypeDescription const & theMember,
        bool theSetter):
        kind(theKind), member(theMember), setter(theSetter)
    {}

    Kind kind;

    com::sun::star::uno::TypeDescription member;

    bool setter;
};

}

#endif
