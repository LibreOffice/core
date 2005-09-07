/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inheritedinterface.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:04:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
