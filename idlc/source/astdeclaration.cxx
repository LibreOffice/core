/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: astdeclaration.cxx,v $
 * $Revision: 1.12 $
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
#include <idlc/astdeclaration.hxx>
#include <idlc/astscope.hxx>
#include <rtl/strbuf.hxx>

using namespace ::rtl;

static OString sGlobal("::");

static OString convertName(const OString& name)
{
    OStringBuffer nameBuffer(name.getLength()+1);
    sal_Int32 nIndex = 0;
    do
    {
        OString token( name.getToken( 0, ':', nIndex ) );
        if( token.getLength() )
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
    , m_bImported(sal_False)
    , m_bIsAdded(sal_False)
    , m_bInMainFile(sal_False)
    , m_bPredefined(false)
{
    if ( m_pScope )
    {
        AstDeclaration* pDecl = scopeAsDecl(m_pScope);
        if (pDecl)
        {
            m_scopedName = pDecl->getScopedName();
            if (m_scopedName.getLength() > 0)
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
        m_bInMainFile = sal_True;
    } else
    {
        m_fileName = idlc()->getFileName();
        m_bImported = sal_True;
    }

    if ( idlc()->isDocValid() )
        m_documentation = OStringToOUString(idlc()->getDocumentation(), RTL_TEXTENCODING_UTF8);

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
        m_fileName = OString();
        m_bInMainFile = sal_False;
    }
}

void AstDeclaration::setName(const ::rtl::OString& name)
{
    m_scopedName = name;
    sal_Int32 nIndex = name.lastIndexOf( ':' );
    m_localName = name.copy( nIndex+1 );

// Huh ? There is always at least one token

//  sal_Int32 count = name.getTokenCount(':');

//  if ( count > 0 )
//  {
//      m_localName = name.getToken(count-1, ':');
//      m_scopedName = name;
//  } else if ( m_pScope )
//  {
//      m_localName = name;
//      AstDeclaration* pDecl = scopeAsDecl(m_pScope);
//      if (pDecl)
//      {
//          m_scopedName = pDecl->getScopedName();
//          if (m_scopedName.getLength() > 0)
//              m_scopedName += sGlobal;
//          m_scopedName += m_localName;
//      }
//  } else
//  {
//      m_localName = name;
//      m_scopedName = name;
//  }
    m_fullName = convertName(m_scopedName);
}

bool AstDeclaration::isType() const {
    switch (m_nodeType) {
    case NT_interface:
    case NT_instantiated_struct:
    case NT_union:
    case NT_enum:
    case NT_sequence:
    case NT_array:
    case NT_typedef:
    case NT_predefined:
    case NT_type_parameter:
        return true;

    default:
        OSL_ASSERT(m_nodeType != NT_struct); // see AstStruct::isType
        return false;
    }
}

sal_Bool AstDeclaration::hasAncestor(AstDeclaration* pDecl)
{
    if (this == pDecl)
        return sal_True;
    if ( !m_pScope )
        return sal_False;
    return scopeAsDecl(m_pScope)->hasAncestor(pDecl);
}

sal_Bool AstDeclaration::dump(RegistryKey& rKey)
{
    AstScope* pScope = declAsScope(this);
    sal_Bool bRet = sal_True;

    if ( pScope )
    {
        DeclList::const_iterator iter = pScope->getIteratorBegin();
        DeclList::const_iterator end = pScope->getIteratorEnd();
        AstDeclaration* pDecl = NULL;
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
                    case NT_union:
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

