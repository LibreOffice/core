/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
