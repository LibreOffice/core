/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: astenum.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:06:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _IDLC_ASTENUM_HXX_
#include <idlc/astenum.hxx>
#endif

#include "registry/version.h"
#include "registry/writer.hxx"

using namespace ::rtl;

AstEnum::AstEnum(const ::rtl::OString& name, AstScope* pScope)
    : AstType(NT_enum, name, pScope)
    , AstScope(NT_enum)
    , m_enumValueCount(0)
{
}

AstEnum::~AstEnum()
{
}

AstConstant* AstEnum::checkValue(AstExpression* pExpr)
{
    DeclList::const_iterator iter = getIteratorBegin();
    DeclList::const_iterator end = getIteratorEnd();
    AstConstant*        pConst = NULL;
    AstDeclaration*     pDecl = NULL;

    while ( iter != end)
    {
        pDecl = *iter;
        pConst = (AstConstant*)pDecl;

        if (pConst->getConstValue()->compare(pExpr))
            return pConst;

        ++iter;
    }

    if ( pExpr->getExprValue()->u.lval > m_enumValueCount )
        m_enumValueCount = pExpr->getExprValue()->u.lval + 1;

    return NULL;
}

sal_Bool AstEnum::dump(RegistryKey& rKey)
{
    RegistryKey localKey;
    if (rKey.createKey( OStringToOUString(getFullName(), RTL_TEXTENCODING_UTF8 ), localKey))
    {
        fprintf(stderr, "%s: warning, could not create key '%s' in '%s'\n",
                idlc()->getOptions()->getProgramName().getStr(),
                getFullName().getStr(), OUStringToOString(rKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
        return sal_False;
    }

    OUString emptyStr;
    sal_uInt16 nConst = getNodeCount(NT_enum_val);
    if ( nConst > 0 )
    {
        typereg::Writer aBlob(
            m_bPublished ? TYPEREG_VERSION_1 : TYPEREG_VERSION_0,
            getDocumentation(), emptyStr, RT_TYPE_ENUM, m_bPublished,
            OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8), 0,
            nConst, 0, 0);

        DeclList::const_iterator iter = getIteratorBegin();
        DeclList::const_iterator end = getIteratorEnd();
        AstDeclaration* pDecl = NULL;
        sal_uInt16 index = 0;
        while ( iter != end )
        {
            pDecl = *iter;
            if ( pDecl->getNodeType() == NT_enum_val )
                ((AstConstant*)pDecl)->dumpBlob(aBlob, index++, false);

            ++iter;
        }

        sal_uInt32 aBlobSize;
        void const * pBlob = aBlob.getBlob(&aBlobSize);

        if (localKey.setValue(emptyStr, RG_VALUETYPE_BINARY,
                                (RegValue)pBlob, aBlobSize))
        {
            fprintf(stderr, "%s: warning, could not set value of key \"%s\" in %s\n",
                    idlc()->getOptions()->getProgramName().getStr(),
                    getFullName().getStr(), OUStringToOString(localKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
            return sal_False;
        }
    }

    return sal_True;
}

AstDeclaration* AstEnum::addDeclaration(AstDeclaration* pDecl)
{
    AstScope* pScope = getScope();
    // add enum value to enclosing scope of the enum
//  pDecl->setName(scopeAsDecl(pScope)->getScopedName() + "::" + pDecl->getLocalName());
//  pScope->addDeclaration(pDecl);

    return AstScope::addDeclaration(pDecl);
}
