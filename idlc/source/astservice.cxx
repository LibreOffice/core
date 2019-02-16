/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <sal/config.h>

#include <astmember.hxx>
#include <astoperation.hxx>
#include <astservice.hxx>
#include <asttype.hxx>

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
                return std::equal(ctor->getIteratorBegin(), ctor->getIteratorEnd(), last->getIteratorBegin(),
                    [](AstDeclaration* a, AstDeclaration* b) {
                        sal_Int32 r1;
                        AstDeclaration const * t1 = deconstructAndResolveTypedefs(static_cast< AstMember * >(a)->getType(), &r1);
                        sal_Int32 r2;
                        AstDeclaration const * t2 = deconstructAndResolveTypedefs(static_cast< AstMember * >(b)->getType(), &r2);
                        return r1 == r2 && t1->getScopedName() == t2->getScopedName();
                    });
            }
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
