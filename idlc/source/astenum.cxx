/*************************************************************************
 *
 *  $RCSfile: astenum.cxx,v $
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
#ifndef _IDLC_ASTENUM_HXX_
#include <idlc/astenum.hxx>
#endif

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
    DeclList::iterator  iter = getIteratorBegin();
    DeclList::iterator  end = getIteratorEnd();
    AstConstant*        pConst = NULL;
    AstDeclaration*     pDecl = NULL;

    while ( iter != end)
    {
        pDecl = *iter;
        pConst = (AstConstant*)pDecl;

        if (pConst->getConstValue()->compare(pExpr))
            return pConst;

        iter++;
    }

    if ( pExpr->getExprValue()->u.lval > m_enumValueCount )
        m_enumValueCount = pExpr->getExprValue()->u.lval + 1;

    return NULL;
}

sal_Bool AstEnum::dump(RegistryKey& rKey, RegistryTypeWriterLoader* pLoader)
{
    RegistryKey localKey;
    if (rKey.createKey( OStringToOUString(getFullName(), RTL_TEXTENCODING_UTF8 ), localKey))
    {
        fprintf(stderr, "%s: warning, could not create key '%s' in '%s'\n",
                idlc()->getOptions()->getProgramName().getStr(),
                getFullName().getStr(), rKey.getRegistryName().getStr());
        return sal_False;
    }

    sal_uInt16 nConst = getNodeCount(NT_enum_val);
    if ( nConst > 0 )
    {
        RegistryTypeWriter aBlob(pLoader->getApi(),
                                 RT_TYPE_ENUM,
                                 OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8),
                                 OUString(), nConst, 0, 0);

        aBlob.setDoku( getDocumentation() );
        aBlob.setFileName( OStringToOUString(getFileName(), RTL_TEXTENCODING_UTF8));

        DeclList::iterator iter = getIteratorBegin();
        DeclList::iterator end = getIteratorEnd();
        AstDeclaration* pDecl = NULL;
        sal_uInt16 index = 0;
        while ( iter != end )
        {
            pDecl = *iter;
            if ( pDecl->getNodeType() == NT_enum_val )
                ((AstConstant*)pDecl)->dumpBlob(aBlob, index++);

            iter++;
        }

        const sal_uInt8* pBlob = aBlob.getBlop();
        sal_uInt32       aBlobSize = aBlob.getBlopSize();

        if (localKey.setValue(OUString(), RG_VALUETYPE_BINARY,
                                (RegValue)pBlob, aBlobSize))
        {
            fprintf(stderr, "%s: warning, could not set value of key \"%s\" in %s\n",
                    idlc()->getOptions()->getProgramName().getStr(),
                    getFullName(), localKey.getRegistryName());
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
