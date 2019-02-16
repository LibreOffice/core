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

#include <astscope.hxx>
#include <astbasetype.hxx>
#include <astinterface.hxx>
#include <errorhandler.hxx>
#include <osl/diagnose.h>


static bool isGlobal(const OString& scopedName)
{
    return scopedName.isEmpty() || scopedName.startsWith(":");
}

AstScope::AstScope(NodeType nodeType)
    : m_nodeType(nodeType)
{

}

AstScope::~AstScope()
{

}

AstDeclaration* AstScope::addDeclaration(AstDeclaration* pDecl)
{
    AstDeclaration* pDeclaration = nullptr;

    if ((pDeclaration = lookupForAdd(pDecl)) != nullptr)
    {
        if ( pDecl->hasAncestor(pDeclaration) )
        {
            ErrorHandler::error2(ErrorCode::RedefScope, pDecl, pDeclaration);
            return nullptr;
        }
        if ( (pDecl->getNodeType() == pDeclaration->getNodeType()) &&
             (pDecl->getNodeType() == NT_sequence
              || pDecl->getNodeType() == NT_instantiated_struct) )
        {
            return pDeclaration;
        }
        if ( (pDeclaration->getNodeType() == NT_interface)
             && (pDecl->getNodeType() == NT_interface)
             && !(static_cast<AstInterface*>(pDeclaration)->isDefined()) )
        {
            m_declarations.push_back(pDecl);
            return pDecl;
        }
        if ( (NT_service == m_nodeType) &&
             ( ((pDecl->getNodeType() == NT_interface_member)
                && (pDeclaration->getNodeType() == NT_interface)) ||
               ((pDecl->getNodeType() == NT_service_member)
                && (pDeclaration->getNodeType() == NT_service)) )
            )
        {
            m_declarations.push_back(pDecl);
            return pDecl;
        }

        ErrorHandler::error2(ErrorCode::RedefScope, scopeAsDecl(this), pDecl);
        return nullptr;
    }

    m_declarations.push_back(pDecl);
    return pDecl;
}

sal_uInt16 AstScope::getNodeCount(NodeType nodeType) const
{
    return static_cast<sal_uInt16>(std::count_if(getIteratorBegin(), getIteratorEnd(),
        [&nodeType](const AstDeclaration* pDecl) { return pDecl->getNodeType() == nodeType; }));
}

AstDeclaration* AstScope::lookupByName(const OString& scopedName)
{
    AstDeclaration* pDecl = nullptr;
    AstScope*       pScope = nullptr;
    if (scopedName.isEmpty())
        return nullptr;

    // If name starts with "::" start look up in global scope
    if ( isGlobal(scopedName) )
    {
        pDecl = scopeAsDecl(this);
        if ( !pDecl )
            return nullptr;

        pScope = pDecl->getScope();
        // If this is the global scope ...
        if ( !pScope )
        {
            // look up the scopedName part after "::"
            OString subName = scopedName.copy(2);
            pDecl = lookupByName(subName);
            return pDecl;
            //return pScope->lookupByName();
        }
        // OK, not global scope yet, so simply iterate with parent scope
        pDecl = pScope->lookupByName(scopedName);
        return pDecl;
    }

    // The name does not start with "::"
    // Look up in the local scope and start with the first scope
    sal_Int32 nIndex = scopedName.indexOf(':');
    OString firstScope =  nIndex > 0 ? scopedName.copy(0, nIndex) : scopedName;
    bool    bFindFirstScope = true;
    pDecl = lookupByNameLocal(firstScope);
    if ( !pDecl )
    {
        bFindFirstScope = false;

        // OK, not found. Go down parent scope chain
        pDecl = scopeAsDecl(this);
        if ( pDecl )
        {
            pScope = pDecl->getScope();
            if ( pScope )
                   pDecl = pScope->lookupByName(scopedName);
            else
                pDecl = nullptr;

             // Special case for scope which is an interface. We
             // have to look in the inherited interfaces as well.
            if ( !pDecl && m_nodeType == NT_interface )
                pDecl = lookupInInherited(scopedName);
       }
    }

    if ( bFindFirstScope && (firstScope != scopedName) )
    {
        sal_Int32 i = 0;
        sal_Int32 nOffset = 2;
        do
        {
            pScope = declAsScope(pDecl);
            if( pScope )
            {
                pDecl = pScope->lookupByNameLocal(scopedName.getToken(nOffset, ':', i ));
                nOffset = 1;
            }
            if( !pDecl )
                break;
        } while( i != -1 );

        if ( !pDecl )
        {
            // last try if is not the global scope and the scopeName isn't specify global too
            pDecl = scopeAsDecl(this);
            if ( pDecl && !pDecl->getLocalName().isEmpty() )
            {
                pScope = pDecl->getScope();
                if ( pScope )
                    pDecl = pScope->lookupByName(scopedName);
            } else
            {
                pDecl = nullptr;
            }
        }

    }

    return pDecl;
}

AstDeclaration* AstScope::lookupByNameLocal(const OString& name) const
{
    for (auto const& declaration : m_declarations)
    {
        if ( declaration->getLocalName() == name )
            return declaration;
    }
    return nullptr;
}

AstDeclaration* AstScope::lookupInInherited(const OString& scopedName) const
{
    const AstInterface* pInterface = dynamic_cast<const AstInterface*>(this);

    if ( !pInterface )
        return nullptr;

    // Can't look in an interface which was not yet defined
    if ( !pInterface->getScope() )
    {
        ErrorHandler::forwardLookupError(pInterface, scopedName);
    }

    // OK, loop through inherited interfaces. Stop when you find it
    for (auto const& elem : pInterface->getAllInheritedInterfaces())
    {
        AstInterface const * resolved = elem.getResolved();
        AstDeclaration* pDecl = resolved->lookupByNameLocal(scopedName);
        if ( pDecl )
            return pDecl;
        pDecl = resolved->lookupInInherited(scopedName);
        if ( pDecl )
            return pDecl;
    }
    // Not found
    return nullptr;
}

AstDeclaration* AstScope::lookupPrimitiveType(ExprType type)
{
    AstDeclaration* pDecl = nullptr;
    AstScope*       pScope = nullptr;
    OString         typeName;
    pDecl = scopeAsDecl(this);
    if ( !pDecl )
        return nullptr;
    pScope = pDecl->getScope();
    if ( pScope)
        return pScope->lookupPrimitiveType(type);

    switch (type)
    {
        case ET_none:
            OSL_ASSERT(false);
            break;
        case ET_short:
            typeName = OString("short");
            break;
        case ET_ushort:
            typeName = OString("unsigned short");
            break;
        case ET_long:
            typeName = OString("long");
            break;
        case ET_ulong:
            typeName = OString("unsigned long");
            break;
        case ET_hyper:
            typeName = OString("hyper");
            break;
        case ET_uhyper:
            typeName = OString("unsigned hyper");
            break;
        case ET_float:
            typeName = OString("float");
            break;
        case ET_double:
            typeName = OString("double");
            break;
        case ET_char:
            typeName = OString("char");
            break;
        case ET_byte:
            typeName = OString("byte");
            break;
        case ET_boolean:
            typeName = OString("boolean");
            break;
        case ET_any:
            typeName = OString("any");
            break;
        case ET_void:
            typeName = OString("void");
            break;
        case ET_type:
            typeName = OString("type");
            break;
        case ET_string:
            typeName = OString("string");
            break;
    }

    pDecl = lookupByNameLocal(typeName);

    if ( pDecl && (pDecl->getNodeType() == NT_predefined) )
    {
        AstBaseType* pBaseType = static_cast<AstBaseType*>(pDecl);

        if ( pBaseType->getExprType() == type )
            return pDecl;
    }

    return nullptr;
}

AstDeclaration* AstScope::lookupForAdd(AstDeclaration const * pDecl) const
{
    if ( !pDecl )
        return nullptr;

    AstDeclaration* pRetDecl = lookupByNameLocal(pDecl->getLocalName());

    return pRetDecl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
