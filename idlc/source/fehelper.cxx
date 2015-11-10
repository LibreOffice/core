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

#include <idlc/fehelper.hxx>
#include <idlc/errorhandler.hxx>
#include "idlc/idlc.hxx"

FeDeclarator::FeDeclarator(const OString& name, DeclaratorType declType, AstDeclaration* pComplPart)
    : m_pComplexPart(pComplPart)
    , m_name(name)
    , m_declType(declType)
{
}

FeDeclarator::~FeDeclarator()
{
}

bool FeDeclarator::checkType(AstDeclaration const * type)
{
    OString tmp(m_name);
    sal_Int32 count = m_name.lastIndexOf( ':' );
    if( count != -1 )
        tmp = m_name.copy( count+1 );

    if (tmp == type->getLocalName())
        return false;
    else
        return true;
}

AstType const * FeDeclarator::compose(AstDeclaration const * pDecl)
{
    const AstType*    pType;

    if ( pDecl == nullptr )
    {
        return nullptr;
    }
    if ( !pDecl->isType() )
    {
        ErrorHandler::noTypeError(pDecl);
        return nullptr;
    }
    pType = static_cast<const AstType*>(pDecl);
    if (m_declType == FD_simple || m_pComplexPart == nullptr)
        return pType;

    return nullptr; // return through this statement should not happen
}

FeInheritanceHeader::FeInheritanceHeader(
    NodeType nodeType, OString* pName, OString* pInherits,
    std::vector< OString > * typeParameters)
    : m_nodeType(nodeType)
    , m_pName(pName)
    , m_pInherits(nullptr)
{
    if (typeParameters != nullptr) {
        m_typeParameters = *typeParameters;
    }
    initializeInherits(pInherits);
}

void FeInheritanceHeader::initializeInherits(OString* pInherits)
{
    if ( pInherits )
    {
        AstScope* pScope = idlc()->scopes()->topNonNull();
        AstDeclaration* pDecl = pScope->lookupByName(*pInherits);
        if ( pDecl )
        {
            AstDeclaration const * resolved = resolveTypedefs(pDecl);
            if ( resolved->getNodeType() == getNodeType()
                 && (resolved->getNodeType() != NT_interface
                     || static_cast< AstInterface const * >(
                         resolved)->isDefined()) )
            {
                if ( ErrorHandler::checkPublished( pDecl ) )
                {
                    m_pInherits = pDecl;
                }
            }
            else
            {
                ErrorHandler::inheritanceError(
                    getNodeType(), getName(), pDecl);
            }
        }
        else
        {
            ErrorHandler::lookupError(*pInherits);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
