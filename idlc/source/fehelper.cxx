/*************************************************************************
 *
 *  $RCSfile: fehelper.cxx,v $
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
#ifndef _IDLC_FEHELPER_HXX_
#include <idlc/fehelper.hxx>
#endif
#ifndef _IDLC_ERRORHANDLER_HXX_
#include <idlc/errorhandler.hxx>
#endif
#ifndef _IDLC_ASTARRAY_HXX_
#include <idlc/astarray.hxx>
#endif

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

sal_Bool FeDeclarator::checkType(AstDeclaration *type)
{
    OString tmp(m_name);
    sal_uInt32 count = 0;
    if ( (count = m_name.getTokenCount(':')) > 0 )
        tmp = m_name.getToken(count-1, ':');

    if (tmp == type->getLocalName())
        return sal_False;
    else
        return sal_True;
}

AstType* FeDeclarator::compose(AstDeclaration* pDecl)
{
    AstArray*   pArray;
    AstType*    pType;

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
        AstDeclaration* pDecl = NULL;
        if ( pScope )
        {
            pDecl = pScope->addDeclaration(pArray);
            if ( (AstDeclaration*)pArray != pDecl )
            {
                delete m_pComplexPart;
                m_pComplexPart = pDecl;
            }
        }
        return pArray;
    }

    return NULL; // return through this statement should not happen
}

FeInheritanceHeader::FeInheritanceHeader(NodeType nodeType, ::rtl::OString* pName, StringList* pInherits)
    : m_nodeType(nodeType)
    , m_pName(pName)
    , m_pInherits(NULL)
{
    initializeInherits(pInherits);
}

sal_Bool FeInheritanceHeader::initializeInherits(StringList* pInherits)
{
    if ( pInherits && (pInherits->size() > 0) )
    {
        StringList::iterator iter = pInherits->begin();
        StringList::iterator end = pInherits->end();
        AstScope* pScope = idlc()->scopes()->topNonNull();
        AstDeclaration* pDecl = NULL;

        m_pInherits = new DeclList();
        while (pScope && iter != end)
        {
            pDecl = pScope->lookupByName(*iter);
            if ( pDecl )
            {
                m_pInherits->push_back(pDecl);
            } else
            {
                idlc()->error()->lookupError(*iter);
                return sal_False;
            }
            iter++;
        }
    }
    return sal_True;
}

sal_Bool FeInterfaceHeader::initializeInherits(StringList* pInherits)
{
    if ( !pInherits )
        return sal_True;

    if ( !FeInheritanceHeader::initializeInherits(pInherits) )
        return sal_False;

    DeclList::iterator iter = m_pInherits->begin();
    DeclList::iterator end  = m_pInherits->end();

    while ( iter != end )
    {
        AstInterface* pIface = ((AstInterface*)*iter);
        if ( ((*iter)->getNodeType() != NT_interface) ||
             !((AstInterface*)*iter)->isDefined() )
        {
            idlc()->error()->inheritanceError(getName(), *iter);
            return sal_False;
        }
        iter++;
    }
    return sal_True;
}
