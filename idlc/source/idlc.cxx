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

#include <idlc/idlc.hxx>
#include <idlc/errorhandler.hxx>
#include <idlc/astscope.hxx>
#include <idlc/astmodule.hxx>
#include <idlc/astservice.hxx>
#include <idlc/astconstants.hxx>
#include <idlc/astexception.hxx>
#include <idlc/astenum.hxx>
#include <idlc/astinterface.hxx>
#include <idlc/astoperation.hxx>
#include <idlc/astbasetype.hxx>
#include "idlc/astdeclaration.hxx"
#include "idlc/astparameter.hxx"
#include "idlc/astsequence.hxx"
#include "idlc/asttype.hxx"
#include "idlc/asttypedef.hxx"

#include <osl/diagnose.h>
#include <osl/file.hxx>
#include <osl/thread.h>

#include <algorithm>

AstDeclaration* SAL_CALL scopeAsDecl(AstScope* pScope)
{
    if (pScope == nullptr) return nullptr;

    switch( pScope->getScopeNodeType() )
    {
        case NT_service:
        case NT_singleton:
            return static_cast<AstService*>(pScope);
        case NT_module:
        case NT_root:
            return static_cast<AstModule*>(pScope);
        case NT_constants:
            return static_cast<AstConstants*>(pScope);
        case NT_interface:
            return static_cast<AstInterface*>(pScope);
        case NT_operation:
            return static_cast<AstOperation*>(pScope);
        case NT_exception:
            return static_cast<AstException*>(pScope);
        case NT_struct:
            return static_cast<AstStruct*>(pScope);
        case NT_enum:
            return static_cast<AstEnum*>(pScope);
        default:
            return nullptr;
    }
}

AstScope* SAL_CALL declAsScope(AstDeclaration* pDecl)
{
    if (pDecl == nullptr) return nullptr;

    switch(pDecl->getNodeType())
    {
        case NT_interface:
            return static_cast<AstInterface*>(pDecl);
        case NT_service:
        case NT_singleton:
            return static_cast<AstService*>(pDecl);
        case NT_module:
        case NT_root:
            return static_cast<AstModule*>(pDecl);
        case NT_constants:
            return static_cast<AstConstants*>(pDecl);
        case NT_exception:
            return static_cast<AstException*>(pDecl);
        case NT_struct:
            return static_cast<AstStruct*>(pDecl);
        case NT_enum:
            return static_cast<AstEnum*>(pDecl);
        case NT_operation:
            return static_cast<AstOperation*>(pDecl);
        default:
            return nullptr;
   }
}

static void SAL_CALL predefineXInterface(AstModule* pRoot)
{
    // define the modules  com::sun::star::uno
    AstModule* pParentScope = pRoot;
    AstModule* pModule = new AstModule(OString("com"), pParentScope);
    pModule->setPredefined(true);
    pParentScope->addDeclaration(pModule);
    pParentScope = pModule;
    pModule = new AstModule(OString("sun"), pParentScope);
    pModule->setPredefined(true);
    pParentScope->addDeclaration(pModule);
    pParentScope = pModule;
    pModule = new AstModule(OString("star"), pParentScope);
    pModule->setPredefined(true);
    pParentScope->addDeclaration(pModule);
    pParentScope = pModule;
    pModule = new AstModule(OString("uno"), pParentScope);
    pModule->setPredefined(true);
    pParentScope->addDeclaration(pModule);
    pParentScope = pModule;

    // define XInterface
    AstInterface* pInterface = new AstInterface(OString("XInterface"), nullptr, pParentScope);
    pInterface->setDefined();
    pInterface->setPredefined(true);
    pInterface->setPublished();
    pParentScope->addDeclaration(pInterface);

    // define XInterface::queryInterface
    AstOperation* pOp = new AstOperation(static_cast<AstType*>(pRoot->lookupPrimitiveType(ET_any)),
                                         OString("queryInterface"), pInterface);
    AstParameter* pParam = new AstParameter(DIR_IN, false,
                                            static_cast<AstType*>(pRoot->lookupPrimitiveType(ET_type)),
                                            OString("aType"), pOp);
    pOp->addDeclaration(pParam);
    pInterface->addMember(pOp);

    // define XInterface::acquire
    pOp = new AstOperation(static_cast<AstType*>(pRoot->lookupPrimitiveType(ET_void)),
                           OString("acquire"), pInterface);
    pInterface->addMember(pOp);

    // define XInterface::release
    pOp = new AstOperation(static_cast<AstType*>(pRoot->lookupPrimitiveType(ET_void)),
                           OString("release"), pInterface);
    pInterface->addMember(pOp);
}

static void SAL_CALL initializePredefinedTypes(AstModule* pRoot)
{
    if ( pRoot )
    {
         AstBaseType* pPredefined = new AstBaseType(ET_long, OString("long"), pRoot);
         pRoot->addDeclaration(pPredefined);

         pPredefined = new AstBaseType(ET_ulong, OString("unsigned long"), pRoot);
         pRoot->addDeclaration(pPredefined);

         pPredefined = new AstBaseType(ET_hyper, OString("hyper"), pRoot);
         pRoot->addDeclaration(pPredefined);

         pPredefined = new AstBaseType(ET_uhyper, OString("unsigned hyper"), pRoot);
         pRoot->addDeclaration(pPredefined);

         pPredefined = new AstBaseType(ET_short, OString("short"), pRoot);
         pRoot->addDeclaration(pPredefined);

         pPredefined = new AstBaseType(ET_ushort, OString("unsigned short"), pRoot);
         pRoot->addDeclaration(pPredefined);

         pPredefined = new AstBaseType(ET_float, OString("float"), pRoot);
         pRoot->addDeclaration(pPredefined);

         pPredefined = new AstBaseType(ET_double, OString("double"), pRoot);
         pRoot->addDeclaration(pPredefined);

         pPredefined = new AstBaseType(ET_char, OString("char"), pRoot);
         pRoot->addDeclaration(pPredefined);

         pPredefined = new AstBaseType(ET_byte, OString("byte"), pRoot);
         pRoot->addDeclaration(pPredefined);

         pPredefined = new AstBaseType(ET_any, OString("any"), pRoot);
         pRoot->addDeclaration(pPredefined);

         pPredefined = new AstBaseType(ET_string, OString("string"), pRoot);
         pRoot->addDeclaration(pPredefined);

         pPredefined = new AstBaseType(ET_type, OString("type"), pRoot);
         pRoot->addDeclaration(pPredefined);

         pPredefined = new AstBaseType(ET_boolean, OString("boolean"), pRoot);
         pRoot->addDeclaration(pPredefined);

         pPredefined = new AstBaseType(ET_void, OString("void"), pRoot);
         pRoot->addDeclaration(pPredefined);
    }
}

Idlc::Idlc(Options* pOptions)
    : m_pOptions(pOptions)
    , m_bIsDocValid(false)
    , m_bIsInMainfile(true)
    , m_published(false)
    , m_errorCount(0)
    , m_warningCount(0)
    , m_lineNumber(0)
    , m_offsetStart(0)
    , m_offsetEnd(0)
    , m_parseState(PS_NoState)
{
    m_pScopes = new AstStack();
    // init root object after construction
    m_pRoot = nullptr;
    m_pErrorHandler = new ErrorHandler();
    m_bGenerateDoc = m_pOptions->isValid("-C");
}

Idlc::~Idlc()
{
    delete m_pRoot;
    delete m_pScopes;
    delete m_pErrorHandler;
}

void Idlc::init()
{
    delete m_pRoot;
    m_pRoot = new AstModule(NT_root, OString(), nullptr);

    // push the root node on the stack
    m_pScopes->push(m_pRoot);
    initializePredefinedTypes(m_pRoot);
    predefineXInterface(m_pRoot);
}

void Idlc::reset()
{
    m_bIsDocValid = false;
    m_bIsInMainfile = true;
    m_published = false;

    m_errorCount = 0;
    m_warningCount = 0;
    m_lineNumber = 0;
    m_parseState = PS_NoState;

    m_fileName.clear();
    m_mainFileName.clear();
    m_realFileName.clear();
    m_documentation.clear();

    m_pScopes->clear();
    delete m_pRoot;

    m_pRoot = new AstModule(NT_root, OString(), nullptr);

    // push the root node on the stack
    m_pScopes->push(m_pRoot);
    initializePredefinedTypes(m_pRoot);

    m_includes.clear();
}

OUString Idlc::processDocumentation()
{
    OUString doc;
    if (m_bIsDocValid) {
        OString raw(getDocumentation());
        if (m_bGenerateDoc) {
            doc = OStringToOUString(raw, RTL_TEXTENCODING_UTF8);
        } else if (raw.indexOf("@deprecated") != -1) {
            //TODO: this check is somewhat crude
            doc = "@deprecated";
        }
    }
    return doc;
}

static void lcl_writeString(::osl::File & rFile, ::osl::FileBase::RC & o_rRC,
        OString const& rString)
{
    sal_uInt64 nWritten(0);
    if (::osl::FileBase::E_None == o_rRC) {
        o_rRC = rFile.write(rString.getStr(), rString.getLength(), nWritten);
        if (static_cast<sal_uInt64>(rString.getLength()) != nWritten) {
            o_rRC = ::osl::FileBase::E_INVAL; //?
        }
    }
}

struct WriteDep
{
    ::osl::File& m_rFile;
    ::osl::FileBase::RC & m_rRC;
    explicit WriteDep(::osl::File & rFile, ::osl::FileBase::RC & rRC)
        : m_rFile(rFile), m_rRC(rRC) { }
    void operator() (OString const& rEntry)
    {
        lcl_writeString(m_rFile, m_rRC, " \\\n ");
        lcl_writeString(m_rFile, m_rRC, rEntry);
    }
};

// write a dummy target for one included file, so the incremental build does
// not break with "No rule to make target" if the included file is removed
struct WriteDummy
{
    ::osl::File& m_rFile;
    ::osl::FileBase::RC & m_rRC;
    explicit WriteDummy(::osl::File & rFile, ::osl::FileBase::RC & rRC)
        : m_rFile(rFile), m_rRC(rRC) { }
    void operator() (OString const& rEntry)
    {
        lcl_writeString(m_rFile, m_rRC, rEntry);
        lcl_writeString(m_rFile, m_rRC, ":\n\n");
    }
};

bool
Idlc::dumpDeps(OString const& rDepFile, OString const& rTarget)
{
    ::osl::File depFile(
            OStringToOUString(rDepFile, osl_getThreadTextEncoding()));
    ::osl::FileBase::RC rc =
        depFile.open(osl_File_OpenFlag_Write | osl_File_OpenFlag_Create);
    if (::osl::FileBase::E_None != rc) {
        return false;
    }
    lcl_writeString(depFile, rc, rTarget);
    if (::osl::FileBase::E_None != rc) {
        return false;
    }
    lcl_writeString(depFile, rc, " :");
    if (::osl::FileBase::E_None != rc) {
        return false;
    }
    m_includes.erase(getRealFileName()); // eeek, that is a temp file...
    ::std::for_each(m_includes.begin(), m_includes.end(),
            WriteDep(depFile, rc));
    lcl_writeString(depFile, rc, "\n\n");
    ::std::for_each(m_includes.begin(), m_includes.end(),
            WriteDummy(depFile, rc));
    if (::osl::FileBase::E_None != rc) {
        return false;
    }
    rc = depFile.close();
    return ::osl::FileBase::E_None == rc;
}

static Idlc* pStaticIdlc = nullptr;

Idlc* SAL_CALL idlc()
{
    return pStaticIdlc;
}

Idlc* SAL_CALL setIdlc(Options* pOptions)
{
    delete pStaticIdlc;
    pStaticIdlc = new Idlc(pOptions);
    pStaticIdlc->init();
    return pStaticIdlc;
}

AstDeclaration const * resolveTypedefs(AstDeclaration const * type) {
    if (type != nullptr) {
        while (type->getNodeType() == NT_typedef) {
            type = static_cast< AstTypeDef const * >(type)->getBaseType();
        }
    }
    return type;
}

AstDeclaration const * deconstructAndResolveTypedefs(
    AstDeclaration const * type, sal_Int32 * rank)
{
    *rank = 0;
    for (;;) {
        if (type == nullptr) {
            return nullptr;
        }
        switch (type->getNodeType()) {
        case NT_typedef:
            type = static_cast< AstTypeDef const * >(type)->getBaseType();
            break;
        case NT_sequence:
            ++(*rank);
            type = static_cast< AstSequence const * >(type)->getMemberType();
            break;
        default:
            return type;
        }
    }
}

AstInterface const * resolveInterfaceTypedefs(AstType const * type) {
    AstDeclaration const * decl = resolveTypedefs(type);
    OSL_ASSERT(decl->getNodeType() == NT_interface);
    return static_cast< AstInterface const * >(decl);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
