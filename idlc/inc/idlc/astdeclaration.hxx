/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _IDLC_ASTDECLARATION_HXX_
#define _IDLC_ASTDECLARATION_HXX_

#include <idlc/idlc.hxx>
#include <registry/registry.hxx>

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
    NT_struct,              // Denotes either a plain struct type, or a
                            // polymorphic struct type template
    NT_type_parameter,      // Denotes a type parameter of a polymorphic struct
                            // type template
    NT_instantiated_struct, // Denotes an instantiated polymorphic struct type
    NT_member,              // Denotes a member in structure, exception
    NT_enum,                // Denotes an enumeration
    NT_enum_val,            // Denotes an enum. value
    NT_sequence,            // Denotes an IDL sequence
    NT_typedef,             // Denotes a typedef
    NT_predefined,          // Denotes a predefined type
    NT_singleton            // Denotes a singleton
};

class AstDeclaration
{
public:
    // Constructors
    AstDeclaration(NodeType type, const OString& name, AstScope* pScope);
    virtual ~AstDeclaration();

    // Data access
    void setName(const OString& name);
    const OString& getLocalName() const
        { return m_localName; }
    const OString&   getScopedName() const
        { return m_scopedName; }
    const OString&   getFullName()
        { return m_fullName; }
    virtual const sal_Char* getRelativName() const
        { return m_fullName.getStr()+1; }
    AstScope* getScope()
        { return m_pScope; }
    void setScope(AstScope* pSc)
        { m_pScope = pSc; }
    NodeType getNodeType() const
        { return m_nodeType; }
    bool isInMainfile() const
        { return m_bInMainFile; }
    void setInMainfile(bool bInMainfile)
        { m_bInMainFile = bInMainfile; }
    bool isImported() const
        { return m_bImported; }
    void setImported(bool bImported)
        { m_bImported = bImported; }
    sal_Int32 getLineNumber() const
        { return m_lineNumber; }
    void setLineNumber(sal_Int32 lineNumber)
        { m_lineNumber = lineNumber; }
    const OString& getFileName() const
        { return m_fileName; }
    void setFileName(const OString& rFileName)
        { m_fileName = rFileName; }
    const OUString& getDocumentation() const
        { return m_documentation; }
    void setDocumentation(const OUString& rDocumentation)
        { m_documentation = rDocumentation; }
    bool isAdded()
        { return m_bIsAdded; }
    void markAsAdded()
        { m_bIsAdded = true; }

    virtual bool isType() const;

    bool hasAncestor(AstDeclaration* pDecl);

    void setPublished() { m_bPublished = true; }
    bool isPublished() const { return m_bPublished; }

    virtual bool dump(RegistryKey& rKey);

    bool isPredefined() { return m_bPredefined; }
    void setPredefined(bool bPredefined);

protected:
    OString      m_localName;
    OString      m_scopedName;       // full qualified name
    OString      m_fullName;         // full qualified name with '/' as separator
    AstScope*           m_pScope;
    NodeType            m_nodeType;
    bool            m_bImported;        // imported ?
    bool            m_bIsAdded;         // mark declaration as added in scope
    bool            m_bInMainFile;      // defined in main file
    bool                m_bPublished;
    bool                m_bPredefined;
    sal_Int32           m_lineNumber;       // line number defined in
    OString      m_fileName;         // fileName defined in
    OUString     m_documentation;    // fileName defined in
};

#endif // _IDLC_ASTDECLARATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
