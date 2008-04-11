/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: inheritedinterface.hxx,v $
 * $Revision: 1.5 $
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

#ifndef INCLUDED_idlc_inc_idlc_inheritedinterface_hxx
#define INCLUDED_idlc_inc_idlc_inheritedinterface_hxx

#include "idlc/idlc.hxx"

#include "rtl/ustring.hxx"

class AstInterface;
class AstType;

class InheritedInterface {
public:
    InheritedInterface(
        AstType const * theInterface, bool theOptional,
        rtl::OUString const & theDocumentation):
        interface(theInterface), optional(theOptional),
        documentation(theDocumentation) {}

    AstType const * getInterface() const { return interface; }

    AstInterface const * getResolved() const
    { return resolveInterfaceTypedefs(interface); }

    bool isOptional() const { return optional; }

    rtl::OUString getDocumentation() const { return documentation; }

private:
    AstType const * interface;
    bool optional;
    rtl::OUString documentation;
};

#endif
