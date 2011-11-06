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

#include "idlc/aststructinstance.hxx"

#include "idlc/asttype.hxx"
#include "idlc/idlctypes.hxx"

#include "rtl/strbuf.hxx"
#include "rtl/string.hxx"

namespace {

rtl::OString createName(
    AstType const * typeTemplate, DeclList const * typeArguments)
{
    rtl::OStringBuffer buf(typeTemplate->getScopedName());
    if (typeArguments != 0) {
        buf.append('<');
        for (DeclList::const_iterator i(typeArguments->begin());
             i != typeArguments->end(); ++i)
        {
            if (i != typeArguments->begin()) {
                buf.append(',');
            }
            if (*i != 0) {
                buf.append((*i)->getScopedName());
            }
        }
        buf.append('>');
    }
    return buf.makeStringAndClear();
}

}

AstStructInstance::AstStructInstance(
    AstType const * typeTemplate, DeclList const * typeArguments,
    AstScope * scope):
    AstType(
        NT_instantiated_struct, createName(typeTemplate, typeArguments), scope),
    m_typeTemplate(typeTemplate), m_typeArguments(*typeArguments)
{}
