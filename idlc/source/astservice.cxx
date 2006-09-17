/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: astservice.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 08:12:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2006 by Sun Microsystems, Inc.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_idlc.hxx"

#include "sal/config.h"

#include "idlc/astmember.hxx"
#include "idlc/astoperation.hxx"
#include "idlc/astservice.hxx"
#include "idlc/asttype.hxx"

bool AstService::checkLastConstructor() const {
    AstOperation const * last = static_cast< AstOperation const * >(getLast());
    for (DeclList::const_iterator i(getIteratorBegin()); i != getIteratorEnd();
         ++i)
    {
        if (*i != last && (*i)->getNodeType() == NT_operation) {
            AstOperation const * ctor = static_cast< AstOperation * >(*i);
            if (ctor->isVariadic() && last->isVariadic()) {
                return true;
            }
            sal_uInt32 n = ctor->nMembers();
            if (n == last->nMembers()) {
                for (DeclList::const_iterator i1(ctor->getIteratorBegin()),
                         i2(last->getIteratorBegin());
                     i1 != ctor->getIteratorEnd(); ++i1, ++i2)
                {
                    sal_Int32 r1;
                    AstDeclaration const * t1 = deconstructAndResolveTypedefs(
                        static_cast< AstMember * >(*i1)->getType(), &r1);
                    sal_Int32 r2;
                    AstDeclaration const * t2 = deconstructAndResolveTypedefs(
                        static_cast< AstMember * >(*i2)->getType(), &r2);
                    if (r1 == r2 && t1->getScopedName() == t2->getScopedName())
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}
