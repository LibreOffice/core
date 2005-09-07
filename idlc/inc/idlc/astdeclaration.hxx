/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: astdeclaration.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:56:00 $
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
#ifndef _IDLC_ASTDECLARATION_HXX_
#define _IDLC_ASTDECLARATION_HXX_

#ifndef _IDLC_IDLC_HXX_
#include <idlc/idlc.hxx>
#endif
#ifndef _REGISTRY_REGISTRY_HXX_
#include <registry/registry.hxx>
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
    NT_struct,              // Denotes either a plain struct type, or a
                            // polymorphic struct type template
    NT_type_parameter,      // Denotes a type parameter of a polymorphic struct
                            // type template
    NT_instantiated_struct, // Denotes an instantiated polymorphic struct type
    NT_member,              // Denotes a member in structure, exception
    NT_enum,                // Denotes an enumeration
    NT_enum_val,            // Denotes an enum. value
    NT_array,               // Denotes an IDL array
    NT_sequence,            // Denotes an IDL sequence
    NT_typedef,             // Denotes a typedef
    NT_predefined,          // Denotes a predefined type
    NT_singleton            // Denotes a singleton
};

class AstDeclaration
{
public:
    // Constructors
    AstDeclaration(NodeType type, const ::rtl::OString& name, AstScope* pScope);
    virtual ~AstDeclaration();

    // Data access
    void setName(const ::rtl::OString& name);
    const ::rtl::OString& getLocalName() const
        { return m_localName; }
    const ::rtl::OString&   getScopedName() const
        { return m_scopedName; }
    const ::rtl::OString&   getFullName()
        { return m_fullName; }
    virtual const sal_Char* getRelativName() const
        { return m_fullName.getStr()+1; }
    AstScope* getScope()
        { return m_pScope; }
    void setScope(AstScope* pSc)
        { m_pScope = pSc; }
    NodeType getNodeType() const
        { return m_nodeType; }
    sal_Bool isInMainfile() const
        { return m_bInMainFile; }
    void setInMainfile(sal_Bool bInMainfile)
        { m_bInMainFile = bInMainfile; }
    sal_Bool isImported() const
        { return m_bImported; }
    void setImported(sal_Bool bImported)
        { m_bImported = bImported; }
    sal_Int32 getLineNumber() const
        { return m_lineNumber; }
    void setLineNumber(sal_Int32 lineNumber)
        { m_lineNumber = lineNumber; }
    const ::rtl::OString& getFileName() const
        { return m_fileName; }
    void setFileName(const ::rtl::OString& rFileName)
        { m_fileName = rFileName; }
    const ::rtl::OUString& getDocumentation() const
        { return m_documentation; }
    void setDocumentation(const ::rtl::OUString& rDocumentation)
        { m_documentation = rDocumentation; }
    sal_Bool isAdded()
        { return m_bIsAdded; }
    void markAsAdded()
        { m_bIsAdded = sal_True; }

    virtual bool isType() const;

    sal_Bool hasAncestor(AstDeclaration* pDecl);

    bool isPublished() const { return m_bPublished; }

    virtual sal_Bool dump(RegistryKey& rKey);
protected:
    ::rtl::OString      m_localName;
    ::rtl::OString      m_scopedName;       // full qualified name
    ::rtl::OString      m_fullName;         // full qualified name with '/' as seperator
    AstScope*           m_pScope;
    const NodeType      m_nodeType;
    sal_Bool            m_bImported;        // imported ?
    sal_Bool            m_bIsAdded;         // mark declaration as added in scope
    sal_Bool            m_bInMainFile;      // defined in main file
    bool                m_bPublished;
    sal_Int32           m_lineNumber;       // line number defined in
    ::rtl::OString      m_fileName;         // fileName defined in
    ::rtl::OUString     m_documentation;    // fileName defined in
};

#endif // _IDLC_ASTDECLARATION_HXX_

