/*************************************************************************
 *
 *  $RCSfile: fehelper.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-15 12:23:01 $
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
#ifndef _IDLC_FEHELPER_HXX_
#define _IDLC_FEHELPER_HXX_

#ifndef _IDLC_ASTTYPE_HXX_
#include <idlc/asttype.hxx>
#endif
#ifndef _IDLC_ASTINTERFACE_HXX_
#include <idlc/astinterface.hxx>
#endif

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

    sal_Bool checkType(AstDeclaration *pType);
    AstType* compose(AstDeclaration* pDecl);
private:
    AstDeclaration* m_pComplexPart;
    ::rtl::OString  m_name;
    DeclaratorType  m_declType;
};

typedef ::std::list< FeDeclarator* > FeDeclList;

class FeInheritanceHeader
{
public:
    FeInheritanceHeader(NodeType nodeType, ::rtl::OString* pName, StringList* pInherits);

    virtual ~FeInheritanceHeader()
    {
        if ( m_pName )
            delete m_pName;
        if ( m_pInherits )
            delete m_pInherits;
     }

    void setNodeType(NodeType nodeType)
        { m_nodeType = nodeType; }
    NodeType getNodeType()
        { return m_nodeType; }
    void setName(::rtl::OString* pName)
        { m_pName = pName; }
    ::rtl::OString* getName()
        { return m_pName; }
    void setInheritsAsStringList(StringList* pInherits)
        { initializeInherits(pInherits); }
    void setInherits(DeclList* pInherits)
        { m_pInherits = pInherits; }
    DeclList* getInherits()
        { return m_pInherits; }
    sal_uInt32 nInherits()
        {
            if ( m_pInherits )
                return m_pInherits->size();
            else
                return 0;
        }
protected:
    virtual sal_Bool initializeInherits(StringList* pinherits);

    NodeType        m_nodeType;
    ::rtl::OString* m_pName;
    DeclList*       m_pInherits;
};

class FeInterfaceHeader : public FeInheritanceHeader
{
public:
    FeInterfaceHeader( ::rtl::OString* pName, StringList* pInherits)
        : FeInheritanceHeader(NT_interface, pName, pInherits)
    {}
    FeInterfaceHeader(NodeType nodeType, ::rtl::OString* pName, StringList* pInherits)
        : FeInheritanceHeader(nodeType, pName, pInherits)
    {}

    virtual ~FeInterfaceHeader()
    {}

protected:
    virtual sal_Bool initializeInherits(StringList* pinherits);
};

#endif // _IDLC_FEHELPER_HXX_

