/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fehelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:04:00 $
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
#ifndef _IDLC_FEHELPER_HXX_
#define _IDLC_FEHELPER_HXX_

#ifndef _IDLC_ASTTYPE_HXX_
#include <idlc/asttype.hxx>
#endif
#ifndef _IDLC_ASTINTERFACE_HXX_
#include <idlc/astinterface.hxx>
#endif

#include <vector>

class FeDeclarator
{
public:
    // Enum to denote types of declarators
    enum DeclaratorType
    {
        FD_simple,      // Simple declarator
        FD_complex      // Complex declarator (complex_part field used)
    };

    FeDeclarator(const ::rtl::OString& name, DeclaratorType declType, AstDeclaration* pComplPart);
    virtual ~FeDeclarator();

    AstDeclaration* getComplexPart()
        { return m_pComplexPart; }
    const ::rtl::OString& getName()
        { return m_name; }
    DeclaratorType  getDeclType()
        { return m_declType; }

    sal_Bool checkType(AstDeclaration const * pType);
    AstType const * compose(AstDeclaration const * pDecl);
private:
    AstDeclaration* m_pComplexPart;
    ::rtl::OString  m_name;
    DeclaratorType  m_declType;
};

typedef ::std::list< FeDeclarator* > FeDeclList;

class FeInheritanceHeader
{
public:
    FeInheritanceHeader(
        NodeType nodeType, ::rtl::OString* pName, ::rtl::OString* pInherits,
        std::vector< rtl::OString > * typeParameters);

    virtual ~FeInheritanceHeader()
    {
        if ( m_pName )
            delete m_pName;
     }

    NodeType getNodeType()
        { return m_nodeType; }
    ::rtl::OString* getName()
        { return m_pName; }
    AstDeclaration* getInherits()
        { return m_pInherits; }

    std::vector< rtl::OString > const & getTypeParameters() const
    { return m_typeParameters; }

private:
    void initializeInherits(::rtl::OString* pinherits);

    NodeType        m_nodeType;
    ::rtl::OString* m_pName;
    AstDeclaration* m_pInherits;
    std::vector< rtl::OString > m_typeParameters;
};

#endif // _IDLC_FEHELPER_HXX_

