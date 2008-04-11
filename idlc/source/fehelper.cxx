/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fehelper.cxx,v $
 * $Revision: 1.10 $
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
#include <idlc/fehelper.hxx>
#include <idlc/errorhandler.hxx>
#include <idlc/astarray.hxx>
#include "idlc/idlc.hxx"

using namespace ::rtl;

FeDeclarator::FeDeclarator(const OString& name, DeclaratorType declType, AstDeclaration* pComplPart)
    : m_pComplexPart(pComplPart)
    , m_name(name)
    , m_declType(declType)
{
}

FeDeclarator::~FeDeclarator()
{
}

sal_Bool FeDeclarator::checkType(AstDeclaration const * type)
{
    OString tmp(m_name);
    sal_Int32 count = m_name.lastIndexOf( ':' );
    if( count != -1 )
        tmp = m_name.copy( count+1 );

    if (tmp == type->getLocalName())
        return sal_False;
    else
        return sal_True;
}

AstType const * FeDeclarator::compose(AstDeclaration const * pDecl)
{
    AstArray*   pArray;
    AstType*    pType;

    if ( pDecl == 0 )
    {
        return NULL;
    }
    if ( !pDecl->isType() )
    {
        idlc()->error()->noTypeError(pDecl);
        return NULL;
    }
    pType = (AstType*)pDecl;
    if (m_declType == FD_simple || m_pComplexPart == NULL)
        return pType;

    if (m_pComplexPart->getNodeType() == NT_array)
    {
        pArray = (AstArray*)m_pComplexPart;
        pArray->setType(pType);

        // insert array type in global scope
        AstScope* pScope = idlc()->scopes()->bottom();
        if ( pScope )
        {
            AstDeclaration* pDecl2 = pScope->addDeclaration(pArray);
            if ( (AstDeclaration*)pArray != pDecl2 )
            {
                delete m_pComplexPart;
                m_pComplexPart = pDecl2;
            }
        }
        return pArray;
    }

    return NULL; // return through this statement should not happen
}

FeInheritanceHeader::FeInheritanceHeader(
    NodeType nodeType, ::rtl::OString* pName, ::rtl::OString* pInherits,
    std::vector< rtl::OString > * typeParameters)
    : m_nodeType(nodeType)
    , m_pName(pName)
    , m_pInherits(NULL)
{
    if (typeParameters != 0) {
        m_typeParameters = *typeParameters;
    }
    initializeInherits(pInherits);
}

void FeInheritanceHeader::initializeInherits(::rtl::OString* pInherits)
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
                if ( idlc()->error()->checkPublished( pDecl ) )
                {
                    m_pInherits = pDecl;
                }
            }
            else
            {
                idlc()->error()->inheritanceError(
                    getNodeType(), getName(), pDecl);
            }
        }
        else
        {
            idlc()->error()->lookupError(*pInherits);
        }
    }
}
