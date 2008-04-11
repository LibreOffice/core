/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: aststruct.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _IDLC_ASTSTRUCT_HXX_
#define _IDLC_ASTSTRUCT_HXX_

#include <idlc/asttype.hxx>
#include <idlc/astscope.hxx>
#include "idlc/idlctypes.hxx"

class AstStruct;
typedef ::std::vector< AstStruct* > InheritedTypes;

class AstStruct : public AstType
                , public AstScope
{
public:
    AstStruct(
        const ::rtl::OString& name,
        std::vector< rtl::OString > const & typeParameters,
        AstStruct* pBaseType, AstScope* pScope);

    AstStruct(const NodeType type,
              const ::rtl::OString& name,
              AstStruct* pBaseType,
              AstScope* pScope);
    virtual ~AstStruct();

    AstStruct* getBaseType()
        { return m_pBaseType; }

    DeclList::size_type getTypeParameterCount() const
    { return m_typeParameters.size(); }

    AstDeclaration const * findTypeParameter(rtl::OString const & name) const;

    virtual bool isType() const;

    virtual sal_Bool dump(RegistryKey& rKey);
private:
    AstStruct* m_pBaseType;
    DeclList m_typeParameters;
};

#endif // _IDLC_ASTSTRUCT_HXX_

