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

#include <astdeclaration.hxx>
#include <astscope.hxx>
#include <rtl/strbuf.hxx>
#include <osl/diagnose.h>

const OStringLiteral sGlobal("::");

static OString convertName(const OString& name)
{
    OStringBuffer nameBuffer(name.getLength()+1);
    sal_Int32 nIndex = 0;
    do
    {
        OString token( name.getToken( 0, ':', nIndex ) );
        if( !token.isEmpty() )
        {
            nameBuffer.append('/');
            nameBuffer.append( token );
        }
    } while( nIndex != -1 );
    return nameBuffer.makeStringAndClear();
}

AstDeclaration::AstDeclaration(NodeType type, const OString& name, AstScope* pScope)
    : m_localName(name)
    , m_pScope(pScope)
    , m_nodeType(type)
    , m_bImported(false)
    , m_bInMainFile(false)
    , m_bPredefined(false)
    , m_lineNumber(0)
{
    if ( m_pScope )
    {
        AstDeclaration* pDecl = scopeAsDecl(m_pScope);
        if (pDecl)
        {
            m_scopedName = pDecl->getScopedName();
            if (!m_scopedName.isEmpty())
                m_scopedName += sGlobal;
            m_scopedName += m_localName;
        }
    } else
    {
        m_scopedName = m_localName;
    }
    m_fullName = convertName(m_scopedName);

    if ( idlc()->getFileName() == idlc()->getRealFileName() )
    {
        m_fileName = idlc()->getMainFileName();
        m_bInMainFile = true;
    } else
    {
        m_fileName = idlc()->getFileName();
        m_bImported = true;
    }

    m_documentation = idlc()->processDocumentation();

    m_bPublished = idlc()->isPublished();
}


AstDeclaration::~AstDeclaration()
{

}

void AstDeclaration::setPredefined(bool bPredefined)
{
    m_bPredefined = bPredefined;
    if ( m_bPredefined )
    {
        m_fileName.clear();
        m_bInMainFile = false;
    }
}

bool AstDeclaration::isType() const {
    switch (m_nodeType) {
    case NT_interface:
    case NT_instantiated_struct:
    case NT_enum:
    case NT_sequence:
    case NT_typedef:
    case NT_predefined:
    case NT_type_parameter:
        return true;

    default:
        OSL_ASSERT(m_nodeType != NT_struct); // see AstStruct::isType
        return false;
    }
}

bool AstDeclaration::hasAncestor(AstDeclaration* pDecl)
{
    if (this == pDecl)
        return true;
    if ( !m_pScope )
        return false;
    return scopeAsDecl(m_pScope)->hasAncestor(pDecl);
}

bool AstDeclaration::dump(RegistryKey& rKey)
{
    AstScope* pScope = declAsScope(this);
    bool bRet = true;

    if ( pScope )
    {
        DeclList::const_iterator iter = pScope->getIteratorBegin();
        DeclList::const_iterator end = pScope->getIteratorEnd();
        AstDeclaration* pDecl = nullptr;
        while ( iter != end && bRet)
        {
            pDecl = *iter;
            if ( pDecl->isInMainfile() )
            {
                switch ( pDecl->getNodeType() )
                {
                    case NT_module:
                    case NT_constants:
                    case NT_interface:
                    case NT_struct:
                    case NT_exception:
                    case NT_enum:
                    case NT_typedef:
                    case NT_service:
                    case NT_singleton:
                        bRet = pDecl->dump(rKey);
                        break;
                    default:
                        break;
                }
            }

            ++iter;
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
