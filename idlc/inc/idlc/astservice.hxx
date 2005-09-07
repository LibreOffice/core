/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: astservice.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:00:43 $
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
#ifndef _IDLC_ASTSERVICE_HXX_
#define _IDLC_ASTSERVICE_HXX_

#ifndef _IDLC_ASTDECLARATION_HXX_
#include <idlc/astdeclaration.hxx>
#endif
#ifndef _IDLC_ASTSCOPE_HXX_
#include <idlc/astscope.hxx>
#endif

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

private:
    bool m_defaultConstructor;
};

#endif // _IDLC_ASTSERVICE_HXX_

