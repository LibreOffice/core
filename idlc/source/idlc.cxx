/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: idlc.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:11:08 $
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
#ifndef _IDLC_IDLC_HXX_
#include <idlc/idlc.hxx>
#endif

#ifndef _IDLC_ERRORHANDLER_HXX_
#include <idlc/errorhandler.hxx>
#endif
#ifndef _IDLC_ASTSCOPE_HXX_
#include <idlc/astscope.hxx>
#endif
#ifndef _IDLC_ASTMODULE_HXX_
#include <idlc/astmodule.hxx>
#endif
#ifndef _IDLC_ASTSERVICE_HXX_
#include <idlc/astservice.hxx>
#endif
#ifndef _IDLC_ASTCONSTANTS_HXX_
#include <idlc/astconstants.hxx>
#endif
#ifndef _IDLC_ASTEXCEPTION_HXX_
#include <idlc/astexception.hxx>
#endif
#ifndef _IDLC_ASTUNION_HXX_
#include <idlc/astunion.hxx>
#endif
#ifndef _IDLC_ASTENUM_HXX_
#include <idlc/astenum.hxx>
#endif
#ifndef _IDLC_ASTINTERFACE_HXX_
#include <idlc/astinterface.hxx>
#endif
#ifndef _IDLC_ASTOPERATION_HXX_
#include <idlc/astoperation.hxx>
#endif
#ifndef _IDLC_ASTBASETYPE_HXX_
#include <idlc/astbasetype.hxx>
#endif
#include "idlc/astdeclaration.hxx"
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
    , m_bIsInMainfile(sal_True)
    , m_errorCount(0)
    , m_warningCount(0)
    , m_lineNumber(0)
    , m_parseState(PS_NoState)
    , m_bIsDocValid(sal_False)
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
}

void Idlc::reset()
{
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

AstInterface const * resolveInterfaceTypedefs(AstType const * type) {
    AstDeclaration const * decl = resolveTypedefs(type);
    OSL_ASSERT(decl->getNodeType() == NT_interface);
    return static_cast< AstInterface const * >(decl);
}
