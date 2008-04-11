/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: astinterfacemember.hxx,v $
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
#ifndef _IDLC_ASTINTERFACEMEMBER_HXX_
#define _IDLC_ASTINTERFACEMEMBER_HXX_

#include <idlc/astinterface.hxx>

class AstInterfaceMember : public AstDeclaration
{
public:
    AstInterfaceMember(const sal_uInt32 flags, AstInterface* pRealInterface,
                       const ::rtl::OString& name, AstScope* pScope)
        : AstDeclaration(NT_interface_member, name, pScope)
        , m_flags(flags)
        , m_pRealInterface(pRealInterface)
        {}
    virtual ~AstInterfaceMember() {}

    AstInterface* getRealInterface()
        { return m_pRealInterface; }
    sal_Bool isOptional()
        { return ((m_flags & AF_OPTIONAL) == AF_OPTIONAL); }
private:
    const sal_uInt32    m_flags;
    AstInterface*       m_pRealInterface;
};

#endif // _IDLC_ASTINTERFACEMEMBER_HXX_

