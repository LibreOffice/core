/*************************************************************************
 *
 *  $RCSfile: astdeclaration.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-15 12:30:43 $
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
#ifndef _IDLC_ASTDECLARATION_HXX_
#include <idlc/astdeclaration.hxx>
#endif
#ifndef _IDLC_ASTSCOPE_HXX_
#include <idlc/astscope.hxx>
#endif
#ifndef _RTL_STRBUF_HXX_
#include <rtl/strbuf.hxx>
#endif

using namespace ::rtl;

static OString sGlobal("::");

static OString convertName(const OString& name)
{
    OStringBuffer nameBuffer(name.getLength()+1);
    sal_Int32 count = name.getTokenCount(':');
    if ( count )
    {
        sal_Int32 offset = (name.indexOf("::") == 0 ? 2 : 0);
        for (sal_Int32 i=offset; i < count; i+=2 )
        {
            nameBuffer.append('/');
            nameBuffer.append(name.getToken(i, ':'));
        }
    } else
    {
        nameBuffer.append('/');
        nameBuffer.append(name);
    }
    return nameBuffer.makeStringAndClear();
}

AstDeclaration::AstDeclaration(NodeType type, const OString& name, AstScope* pScope)
    : m_nodeType(type)
    , m_localName(name)
    , m_pScope(pScope)
    , m_bIsAdded(sal_False)
    , m_bImported(sal_False)
    , m_bInMainFile(sal_False)
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
}


AstDeclaration::~AstDeclaration()
{

}

void AstDeclaration::setName(const ::rtl::OString& name)
{
    sal_Int32 count = name.getTokenCount(':');
    if ( count > 0 )
    {
        m_localName = name.getToken(count-1, ':');
        m_scopedName = name;
    } else if ( m_pScope )
    {
        m_localName = name;
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
        m_localName = name;
        m_scopedName = name;
    }
    m_fullName = convertName(m_scopedName);
}

sal_Bool AstDeclaration::isType()
{
    sal_Bool bIsType = sal_False;

    switch (m_nodeType)
    {
        case NT_interface:
        case NT_struct:
        case NT_union:
        case NT_enum:
        case NT_sequence:
        case NT_array:
        case NT_typedef:
        case NT_predefined:
            bIsType = sal_True;
    }

    return bIsType;
}

sal_Bool AstDeclaration::hasAncestor(AstDeclaration* pDecl)
{
    if (this == pDecl)
        return sal_True;
    if ( !m_pScope )
        return sal_False;
    return scopeAsDecl(m_pScope)->hasAncestor(pDecl);
}

sal_Bool AstDeclaration::dump(RegistryKey& rKey, RegistryTypeWriterLoader* pLoader)
{
    AstScope* pScope = declAsScope(this);
    sal_Bool bRet = sal_True;

    if ( pScope )
    {
        DeclList::iterator iter = pScope->getIteratorBegin();
        DeclList::iterator end = pScope->getIteratorEnd();
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
                        bRet = pDecl->dump(rKey, pLoader);
                }
            }
            iter++;
        }
    }
    return bRet;
}
