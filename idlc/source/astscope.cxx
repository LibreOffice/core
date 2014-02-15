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

#include <idlc/astscope.hxx>
#include <idlc/astbasetype.hxx>
#include <idlc/astinterface.hxx>
#include <idlc/errorhandler.hxx>


using namespace ::rtl;

bool isGlobal(const OString& scopedName)
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
    AstDeclaration* pDeclaration = NULL;

    if ((pDeclaration = lookupForAdd(pDecl)) != NULL)
    {
        if ( pDecl->hasAncestor(pDeclaration) )
        {
            idlc()->error()->error2(EIDL_REDEF_SCOPE, pDecl, pDeclaration);
            return NULL;
        }
        if ( (pDecl->getNodeType() == pDeclaration->getNodeType()) &&
             (pDecl->getNodeType() == NT_sequence
              || pDecl->getNodeType() == NT_instantiated_struct) )
        {
            return pDeclaration;
        }
        if ( (pDeclaration->getNodeType() == NT_interface)
             && (pDecl->getNodeType() == NT_interface)
             && !((AstInterface*)pDeclaration)->isDefined() )
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

        idlc()->error()->error2(EIDL_REDEF_SCOPE, scopeAsDecl(this), pDecl);
        return NULL;
    }

    m_declarations.push_back(pDecl);
    return pDecl;
}

sal_uInt16 AstScope::getNodeCount(NodeType nodeType)
{
    DeclList::const_iterator iter = getIteratorBegin();
    DeclList::const_iterator end = getIteratorEnd();
    AstDeclaration* pDecl = NULL;
    sal_uInt16 count = 0;

    while ( iter != end )
    {
        pDecl = *iter;
        if ( pDecl->getNodeType() == nodeType )
            count++;
        ++iter;
    }
    return count;
}

AstDeclaration* AstScope::lookupByName(const OString& scopedName)
{
    AstDeclaration* pDecl = NULL;
    AstScope*       pScope = NULL;
    if (scopedName.isEmpty())
        return NULL;

    // If name starts with "::" start look up in global scope
    if ( isGlobal(scopedName) )
    {
        pDecl = scopeAsDecl(this);
        if ( !pDecl )
            return NULL;

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
                pDecl = NULL;

             // Special case for scope which is an interface. We
             // have to look in the inherited interfaces as well.
            if ( !pDecl )
            {
                if (m_nodeType == NT_interface)
                    pDecl = lookupInInherited(scopedName);
            }
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
            if ( pDecl && (pDecl->getLocalName() != "") )
            {
                pScope = pDecl->getScope();
                if ( pScope )
                    pDecl = pScope->lookupByName(scopedName);
            } else
            {
                pDecl = NULL;
            }
        }

    }

    return pDecl;
}

AstDeclaration* AstScope::lookupByNameLocal(const OString& name) const
{
    DeclList::const_iterator iter(m_declarations.begin());
    DeclList::const_iterator end(m_declarations.end());
    AstDeclaration* pDecl = NULL;

    while ( iter != end )
    {
        pDecl = *iter;
        if ( pDecl->getLocalName() == name )
            return pDecl;
        ++iter;
    }
    return NULL;
}

AstDeclaration* AstScope::lookupInInherited(const OString& scopedName) const
{
    AstInterface* pInterface = (AstInterface*)this;

    if ( !pInterface )
        return NULL;

    // Can't look in an interface which was not yet defined
    if ( !pInterface->getScope() )
    {
        idlc()->error()->forwardLookupError(pInterface, scopedName);
    }

    // OK, loop through inherited interfaces. Stop when you find it
    AstInterface::InheritedInterfaces::const_iterator iter(
        pInterface->getAllInheritedInterfaces().begin());
    AstInterface::InheritedInterfaces::const_iterator end(
        pInterface->getAllInheritedInterfaces().end());
    while ( iter != end )
    {
        AstInterface const * resolved = iter->getResolved();
        AstDeclaration* pDecl = resolved->lookupByNameLocal(scopedName);
        if ( pDecl )
            return pDecl;
        pDecl = resolved->lookupInInherited(scopedName);
        if ( pDecl )
            return pDecl;
        ++iter;
    }
    // Not found
    return NULL;
}

AstDeclaration* AstScope::lookupPrimitiveType(ExprType type)
{
    AstDeclaration* pDecl = NULL;
    AstScope*       pScope = NULL;
    OString         typeName;
    pDecl = scopeAsDecl(this);
    if ( !pDecl )
        return NULL;
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
        AstBaseType* pBaseType = (AstBaseType*)pDecl;

        if ( pBaseType->getExprType() == type )
            return pDecl;
    }

    return NULL;
}

AstDeclaration* AstScope::lookupForAdd(AstDeclaration* pDecl)
{
    if ( !pDecl )
        return NULL;

    AstDeclaration* pRetDecl = lookupByNameLocal(pDecl->getLocalName());

   return pRetDecl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
