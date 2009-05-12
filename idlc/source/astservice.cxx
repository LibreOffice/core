/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: astservice.cxx,v $
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
                    if (r1 != r2 || t1->getScopedName() != t2->getScopedName())
                    {
                        return false;
                    }
                }
                return true;
            }
        }
    }
    return false;
}
