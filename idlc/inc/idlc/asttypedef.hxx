/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: asttypedef.hxx,v $
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
#ifndef _IDLC_ASTTYPEDEF_HXX_
#define _IDLC_ASTTYPEDEF_HXX_

#include <idlc/asttype.hxx>

class AstTypeDef : public AstType
{
public:
    AstTypeDef(
        AstType const * baseType, rtl::OString const & name, AstScope * scope):
        AstType(NT_typedef, name, scope), m_pBaseType(baseType) {}

    virtual ~AstTypeDef() {}

    AstType const * getBaseType() const
        { return m_pBaseType; }

    virtual bool isUnsigned() const
    { return m_pBaseType != 0 && m_pBaseType->isUnsigned(); }

    virtual sal_Bool dump(RegistryKey& rKey);
private:
    AstType const * m_pBaseType;
};

#endif // _IDLC_ASTTYPEDEF_HXX_

