/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: idlc.cxx,v $
 * $Revision: 1.12 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_idlc.hxx"
#include <idlc/idlc.hxx>
#include <idlc/errorhandler.hxx>
#include <idlc/astscope.hxx>
#include <idlc/astmodule.hxx>
#include <idlc/astservice.hxx>
#include <idlc/astconstants.hxx>
#include <idlc/astexception.hxx>
#include <idlc/astunion.hxx>
#include <idlc/astenum.hxx>
#include <idlc/astinterface.hxx>
#include <idlc/astoperation.hxx>
#include <idlc/astbasetype.hxx>
#include "idlc/astdeclaration.hxx"
#include "idlc/astparameter.hxx"
#include "idlc/astsequence.hxx"
#include "idlc/asttype.hxx"
#include "idlc/asttypedef.hxx"

#include "osl/diagnose.h"

using namespace ::rtl;

AstDeclaration* SAL_CALL scopeAsDecl(AstScope* pScope)
{
    if (pScope == NULL) return NULL;

    switch( pScope->getScopeNodeType() )
    {
        case NT_service:
        case NT_singleton:
            return (AstService*)(pScope);
        case NT_module:
        case NT_root:
            return (AstModule*)(pScope);
        case NT_constants:
            return (AstConstants*)(pScope);
        case NT_interface:
            return (AstInterface*)(pScope);
        case NT_operation:
            return (AstOperation*)(pScope);
        case NT_exception:
            return (AstException*)(pScope);
        case NT_union:
            return (AstUnion*)(pScope);
        case NT_struct:
            return (AstStruct*)(pScope);
        case NT_enum:
            return (AstEnum*)(pScope);
        default:
            return NULL;
    }
}

AstScope* SAL_CALL declAsScope(AstDeclaration* pDecl)
{
    if (pDecl == NULL) return NULL;

    switch(pDecl->getNodeType())
    {
        case NT_interface:
            return (AstInterface*)(pDecl);
        case NT_service:
        case NT_singleton:
            return (AstService*)(pDecl);
        case NT_module:
        case NT_root:
            return (AstModule*)(pDecl);
        case NT_constants:
            return (AstConstants*)(pDecl);
        case NT_exception:
            return (AstException*)(pDecl);
        case NT_union:
            return (AstUnion*)(pDecl);
        case NT_struct:
            return (AstStruct*)(pDecl);
        case NT_enum:
            return (AstEnum*)(pDecl);
        case NT_operation:
            return (AstOperation*)(pDecl);
        default:
            return NULL;
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
    AstInterface* pInterface = new AstInterface(OString("XInterface"), NULL, pParentScope);
    pInterface->setDefined();
    pInterface->setPredefined(true);
    pInterface->setPublished();
    pParentScope->addDeclaration(pInterface);

    // define XInterface::queryInterface
    AstOperation* pOp = new AstOperation(0, (AstType*)(pRoot->lookupPrimitiveType(ET_any)),
                                         OString("queryInterface"), pInterface);
    AstParameter* pParam = new AstParameter(DIR_IN, false,
                                            (AstType*)(pRoot->lookupPrimitiveType(ET_type)),
                                            OString("aType"), pOp);
    pOp->addDeclaration(pParam);
    pInterface->addMember(pOp);

    // define XInterface::acquire
    pOp = new AstOperation(1, (AstType*)(pRoot->lookupPrimitiveType(ET_void)),
                                         OString("acquire"), pInterface);
    pInterface->addMember(pOp);

    // define XInterface::release
    pOp = new AstOperation(1, (AstType*)(pRoot->lookupPrimitiveType(ET_void)),
                                         OString("release"), pInterface);
    pInterface->addMember(pOp);
}

static void SAL_CALL initializePredefinedTypes(AstModule* pRoot)
{
    AstBaseType* pPredefined = NULL;
    if ( pRoot )
    {
         pPredefined = new AstBaseType(ET_long, OString("long"), pRoot);
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
    , m_bIsDocValid(sal_False)
    , m_bIsInMainfile(sal_True)
    , m_published(false)
    , m_errorCount(0)
    , m_warningCount(0)
    , m_lineNumber(0)
    , m_parseState(PS_NoState)
{
    m_pScopes = new AstStack();
    // init root object after construction
    m_pRoot = NULL;
    m_pErrorHandler = new ErrorHandler();
    m_bGenerateDoc = m_pOptions->isValid("-C");
}

Idlc::~Idlc()
{
    if (m_pRoot)
        delete m_pRoot;
    if (m_pScopes)
        delete m_pScopes;
    if (m_pErrorHandler)
        delete m_pErrorHandler;
}

void Idlc::init()
{
    if ( m_pRoot )
        delete m_pRoot;

    m_pRoot = new AstModule(NT_root, OString(), NULL);

    // push the root node on the stack
    m_pScopes->push(m_pRoot);
    initializePredefinedTypes(m_pRoot);
    predefineXInterface(m_pRoot);
}

void Idlc::reset()
{
    m_bIsDocValid = sal_False;
    m_bIsInMainfile = sal_True;
    m_published = false;

    m_errorCount = 0;
    m_warningCount = 0;
    m_lineNumber = 0;
    m_parseState = PS_NoState;

    m_fileName = OString();
    m_mainFileName = OString();
    m_realFileName = OString();
    m_documentation = OString();

    m_pScopes->clear();
    if ( m_pRoot)
        delete m_pRoot;

    m_pRoot = new AstModule(NT_root, OString(), NULL);

    // push the root node on the stack
    m_pScopes->push(m_pRoot);
    initializePredefinedTypes(m_pRoot);
}

sal_Bool Idlc::isDocValid()
{
    if ( m_bGenerateDoc )
        return m_bIsDocValid;
    return sal_False;;
}

static Idlc* pStaticIdlc = NULL;

Idlc* SAL_CALL idlc()
{
    return pStaticIdlc;
}

Idlc* SAL_CALL setIdlc(Options* pOptions)
{
    if ( pStaticIdlc )
    {
        delete pStaticIdlc;
    }
    pStaticIdlc = new Idlc(pOptions);
    pStaticIdlc->init();
    return pStaticIdlc;
}

AstDeclaration const * resolveTypedefs(AstDeclaration const * type) {
    if (type != 0) {
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
        if (type == 0) {
            return 0;
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
