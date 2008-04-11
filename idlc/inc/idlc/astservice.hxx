/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: astservice.hxx,v $
 * $Revision: 1.7 $
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
#ifndef _IDLC_ASTSERVICE_HXX_
#define _IDLC_ASTSERVICE_HXX_

#include <idlc/astdeclaration.hxx>
#include <idlc/astscope.hxx>

class AstService : public AstDeclaration
                 , public AstScope
{
public:
    AstService(const ::rtl::OString& name, AstScope* pScope)
        : AstDeclaration(NT_service, name, pScope)
        , AstScope(NT_service)
        , m_defaultConstructor(false)
        {}
    AstService(const NodeType type, const ::rtl::OString& name, AstScope* pScope)
        : AstDeclaration(type, name, pScope)
        , AstScope(type)
        , m_defaultConstructor(false)
        {}
    virtual ~AstService() {}

    virtual sal_Bool dump(RegistryKey& rKey);

    void setDefaultConstructor(bool b) { m_defaultConstructor = b; }

    bool checkLastConstructor() const;

private:
    bool m_defaultConstructor;
};

#endif // _IDLC_ASTSERVICE_HXX_

