/*************************************************************************
 *
 *  $RCSfile: astdeclaration.hxx,v $
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
#ifndef _IDLC_ASTDECLARATION_HXX_
#define _IDLC_ASTDECLARATION_HXX_

#ifndef _IDLC_IDLC_HXX_
#include <idlc/idlc.hxx>
#endif
#ifndef _REGISTRY_REGISTRY_HXX_
#include <registry/registry.hxx>
#endif
#ifndef _REGISTRY_REFLWRIT_HXX_
#include <registry/reflwrit.hxx>
#endif

class AstScope;

// Enum defining the different kinds of Ast nodes
enum NodeType
{
    NT_object,              // Denotes an object
    NT_service,             // Denotes an servcie
    NT_interface_member,    // Denotes an interface which is exported from object
    NT_service_member,      // Denotes an service which is exported from object
    NT_observes,            // Denotes an observed interface
    NT_needs,               // Denotes an needed service
    NT_module,              // Denotes a module
    NT_root,                // Denotes the root of AST
    NT_interface,           // Denotes an interface
    NT_constants,           // Denotes a constant group
    NT_const,               // Denotes a constant
    NT_exception,           // Denotes an exception
    NT_attribute,           // Denotes an attribute
    NT_property,            // Denotes an property
    NT_operation,           // Denotes an operation
    NT_parameter,           // Denotes an op. parameter
    NT_union,               // Denotes a union
    NT_union_branch,        // Denotes a union branch
    NT_struct,              // Denotes a structure
    NT_member,              // Denotes a member in structure, exception
    NT_enum,                // Denotes an enumeration
    NT_enum_val,            // Denotes an enum. value
    NT_array,               // Denotes an IDL array
    NT_sequence,            // Denotes an IDL sequence
    NT_typedef,             // Denotes a typedef
    NT_predefined           // Denotes a predefined type
};

class AstDeclaration
{
public:
    // Constructors
    AstDeclaration(const NodeType type, const ::rtl::OString& name, AstScope* pScope);
    virtual ~AstDeclaration();

    // Data access
    void setName(const ::rtl::OString& name);
    const ::rtl::OString& getLocalName()
        { return m_localName; }
    const ::rtl::OString&   getScopedName()
        { return m_scopedName; }
    const ::rtl::OString&   getFullName()
        { return m_fullName; }
    virtual const sal_Char* getRelativName()
        { return m_fullName.getStr()+1; }
    AstScope* getScope()
        { return m_pScope; }
    void setScope(AstScope* pSc)
        { m_pScope = pSc; }
    const NodeType getNodeType()
        { return m_nodeType; }
    sal_Bool isInMainfile()
        { return m_bInMainFile; }
    void setInMainfile(sal_Bool bInMainfile)
        { m_bInMainFile = bInMainfile; }
    sal_Bool isImported()
        { return m_bImported; }
    void setImported(sal_Bool bImported)
        { m_bImported = bImported; }
    sal_Int32 getLineNumber()
        { return m_lineNumber; }
    void setLineNumber(sal_Int32 lineNumber)
        { m_lineNumber = lineNumber; }
    const ::rtl::OString& getFileName()
        { return m_fileName; }
    void setFileName(const ::rtl::OString& rFileName)
        { m_fileName = rFileName; }
    const ::rtl::OUString& getDocumentation()
        { return m_documentation; }
    void setDocumentation(const ::rtl::OUString& rDocumentation)
        { m_documentation = rDocumentation; }
    sal_Bool isAdded()
        { return m_bIsAdded; }
    void markAsAdded()
        { m_bIsAdded = sal_True; }

    sal_Bool isType();
    sal_Bool hasAncestor(AstDeclaration* pDecl);

    virtual sal_Bool dump(RegistryKey& rKey, RegistryTypeWriterLoader* pLoader);
protected:
    ::rtl::OString      m_localName;
    ::rtl::OString      m_scopedName;       // full qualified name
    ::rtl::OString      m_fullName;         // full qualified name with '/' as seperator
    AstScope*           m_pScope;
    const NodeType      m_nodeType;
    sal_Bool            m_bImported;        // imported ?
    sal_Bool            m_bIsAdded;         // mark declaration as added in scope
    sal_Bool            m_bInMainFile;      // defined in main file
    sal_Int32           m_lineNumber;       // line number defined in
    ::rtl::OString      m_fileName;         // fileName defined in
    ::rtl::OUString     m_documentation;    // fileName defined in
};

#endif // _IDLC_ASTDECLARATION_HXX_

