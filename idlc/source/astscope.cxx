/*************************************************************************
 *
 *  $RCSfile: astscope.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 09:27:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _IDLC_ASTSCOPE_HXX_
#include <idlc/astscope.hxx>
#endif
#ifndef _IDLC_ASTBASETYPE_HXX_
#include <idlc/astbasetype.hxx>
#endif
#ifndef _IDLC_ASTINERFACE_HXX_
#include <idlc/astinterface.hxx>
#endif
#ifndef _IDLC_ERRORHANDLER_HXX_
#include <idlc/errorhandler.hxx>
#endif


using namespace ::rtl;

sal_Bool isGlobal(const OString& scopedName)
{
    if ((scopedName.getLength() == 0) || (scopedName.indexOf(':') == 0))
    {
        return sal_True;
    }
    return sal_False;
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
        if (pDecl->getNodeType() == NT_union_branch )
        {
            m_declarations.push_back(pDecl);
            return pDecl;
        }
        if ( pDecl->hasAncestor(pDeclaration) )
        {
            idlc()->error()->error2(EIDL_REDEF_SCOPE, pDecl, pDeclaration);
            return NULL;
        }
        if ( (pDecl->getNodeType() == pDeclaration->getNodeType()) &&
             ((pDecl->getNodeType() == NT_sequence) || (pDeclaration->getNodeType() == NT_array)) )
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
             ((pDecl->getNodeType() == NT_interface_member)
              && (pDeclaration->getNodeType() == NT_interface)) ||
             ((pDecl->getNodeType() == NT_service_member)
              && (pDeclaration->getNodeType() == NT_service)))
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
    DeclList::iterator iter = getIteratorBegin();
    DeclList::iterator end = getIteratorEnd();
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
    if (scopedName.getLength() == 0)
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
    sal_Bool    bFindFirstScope = sal_True;
    pDecl = lookupByNameLocal(firstScope);
    if ( !pDecl )
    {
        bFindFirstScope = sal_False;

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
        sal_Int32 nIndex = 0;
        sal_Int32 nOffset = 2;
        do
        {
            pScope = declAsScope(pDecl);
            if( pScope )
            {
                pDecl = pScope->lookupByNameLocal(scopedName.getToken(nOffset, ':', nIndex ));
                nOffset = 1;
            }
            if( !pDecl )
                break;
        } while( nIndex != -1 );

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

AstDeclaration* AstScope::lookupByNameLocal(const OString& name)
{
    DeclList::iterator iter = getIteratorBegin();
    DeclList::iterator end = getIteratorEnd();
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

AstDeclaration* AstScope::lookupInInherited(const OString& scopedName)
{
    AstDeclaration* pDecl = NULL;
    AstInterface* pInterface = (AstInterface*)this;

    if ( !pInterface )
        return NULL;

    // Can't look in an interface which was not yet defined
    if ( !pInterface->getScope() )
    {
        idlc()->error()->forwardLookupError(pInterface, scopedName);
    }

    // OK, loop through inherited interfaces. Stop when you find it
    if ( pInterface->nInheritedInterfaces() > 0 )
    {
        DeclList::const_iterator iter = pInterface->getInheritedInterfaces().begin();
        DeclList::const_iterator end = pInterface->getInheritedInterfaces().end();

        while ( iter != end )
        {
            if ( pDecl = ((AstInterface*)(*iter))->lookupByNameLocal(scopedName) )
                return pDecl;
            if ( pDecl = ((AstInterface*)(*iter))->lookupInInherited(scopedName) )
                return pDecl;
            ++iter;
        }
    }
    // Not found
    return NULL;
}

AstDeclaration* AstScope::lookupPrimitiveType(ExprType type)
{
    AstDeclaration* pDecl = NULL;
    AstScope*       pScope = NULL;
    AstBaseType*    pBaseType = NULL;
    OString         typeName;
    pDecl = scopeAsDecl(this);
    if ( !pDecl )
        return NULL;
    pScope = pDecl->getScope();
    if ( pScope)
        return pScope->lookupPrimitiveType(type);

    switch (type)
    {
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
        pBaseType = (AstBaseType*)pDecl;

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

    if ( (pRetDecl == NULL) &&
         m_nodeType == NT_interface )
        pRetDecl = lookupInInherited(pDecl->getLocalName());

   return pRetDecl;
}
