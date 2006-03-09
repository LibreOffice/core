/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: asttypedef.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-03-09 10:48:31 $
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
#ifndef _IDLC_ASTTYPEDEF_HXX_
#define _IDLC_ASTTYPEDEF_HXX_

#ifndef _IDLC_ASTTYPE_HXX_
#include <idlc/asttype.hxx>
#endif

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

