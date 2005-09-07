/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: aststruct.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:01:49 $
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
#ifndef _IDLC_ASTSTRUCT_HXX_
#define _IDLC_ASTSTRUCT_HXX_

#ifndef _IDLC_ASTTYPE_HXX_
#include <idlc/asttype.hxx>
#endif
#ifndef _IDLC_ASTSCOPE_HXX_
#include <idlc/astscope.hxx>
#endif
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

