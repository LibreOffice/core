/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: astmodule.hxx,v $
 * $Revision: 1.4 $
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
#ifndef _IDLC_ASTMODULE_HXX_
#define _IDLC_ASTMODULE_HXX_

#include <idlc/astdeclaration.hxx>
#include <idlc/astscope.hxx>

class AstModule : public AstDeclaration
                , public AstScope
{
public:
    AstModule(const ::rtl::OString& name, AstScope* pScope)
        : AstDeclaration(NT_module, name, pScope)
        , AstScope(NT_module)
    {}
    AstModule(NodeType type, const ::rtl::OString& name, AstScope* pScope)
        : AstDeclaration(type, name, pScope)
        , AstScope(type)
    {}
    virtual ~AstModule() {}

    virtual sal_Bool dump(RegistryKey& rKey);
};

#endif // _IDLC_ASTMODULE_HXX_

