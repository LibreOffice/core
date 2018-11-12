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

/*
 * parser.yy - BISON grammar for IDLC 1.0
 */

%{
#include <string.h>

#include <idlc.hxx>
#include <errorhandler.hxx>
#include <fehelper.hxx>
#include <astexpression.hxx>
#include <astconstants.hxx>
#include <astconstant.hxx>
#include <astbasetype.hxx>
#include <asttypedef.hxx>
#include <astexception.hxx>
#include <astmember.hxx>
#include <astenum.hxx>
#include <astsequence.hxx>
#include <astattribute.hxx>
#include <astoperation.hxx>
#include <astparameter.hxx>
#include <astinterfacemember.hxx>
#include <astservicemember.hxx>
#include <astobserves.hxx>
#include <astneeds.hxx>

#include <aststructinstance.hxx>

#include "attributeexceptions.hxx"

#include <rtl/strbuf.hxx>
#include <osl/diagnose.h>

#include <algorithm>
#include <vector>


#define YYDEBUG 1
#define YYERROR_VERBOSE 1

extern int yylex(void);
static void yyerror(char const *);

static void checkIdentifier(OString const * id)
{
    static short check = 0;
    if (check == 0) {
        if (idlc()->getOptions()->isValid("-cid"))
            check = 1;
        else
            check = 2;
    }

    if ( id->indexOf('_') >= 0 )
        if ( (id->pData->buffer[0] >= 97 && id->pData->buffer[0] <= 122)
             || id->pData->buffer[0] == '_') {
            if (check == 1) {
                OStringBuffer msg(25 + id->getLength());
                msg.append("mismatched identifier '");
                msg.append(*id);
                msg.append("'");
                ErrorHandler::syntaxError(idlc()->getParseState(),
                                         idlc()->getLineNumber(),
                                         msg.getStr());
            }
            else
                ErrorHandler::warning0(WarningCode::WrongNamingConvention, id->getStr());
        }
}

static void reportDoubleMemberDeclarations(
    AstInterface::DoubleMemberDeclarations const & doubleMembers)
{
    for (auto const& doubleMember : doubleMembers)
    {
        ErrorHandler::error2(ErrorCode::DoubleMember, doubleMember.first, doubleMember.second);
    }
}

static void addInheritedInterface(
    AstInterface * ifc, OString const & name, bool optional,
    OUString const & documentation)
{
    AstDeclaration * decl = ifc->lookupByName(name);
    AstDeclaration const * resolved = resolveTypedefs(decl);
    if (resolved != nullptr && resolved->getNodeType() == NT_interface) {
        if (ErrorHandler::checkPublished(decl)) {
            if (!static_cast< AstInterface const * >(resolved)->isDefined()) {
                ErrorHandler::inheritanceError(
                    NT_interface, &ifc->getScopedName(), decl);
            } else {
                AstInterface::DoubleDeclarations doubleDecls(
                    ifc->checkInheritedInterfaceClashes(
                        static_cast< AstInterface const * >(resolved),
                        optional));
                if (doubleDecls.interfaces.empty()
                    && doubleDecls.members.empty())
                {
                    ifc->addInheritedInterface(
                        static_cast< AstType * >(decl), optional,
                        documentation);
                } else {
                    for (auto const& elem : doubleDecls.interfaces)
                    {
                        ErrorHandler::error1(
                            ErrorCode::DoubleInheritance, elem);
                    }
                    reportDoubleMemberDeclarations(doubleDecls.members);
                }
            }
        }
    } else {
        ErrorHandler::lookupError(
            ErrorCode::InterfaceMemberLookup, name, scopeAsDecl(ifc));
    }
}

static AstDeclaration const * createNamedType(
    OString const * scopedName, DeclList const * typeArgs)
{
    AstDeclaration * decl = idlc()->scopes()->topNonNull()->lookupByName(
        *scopedName);
    AstDeclaration const * resolved = resolveTypedefs(decl);
    if (decl == nullptr) {
        ErrorHandler::lookupError(*scopedName);
    } else if (!ErrorHandler::checkPublished(decl)) {
        decl = nullptr;
    } else if (resolved->getNodeType() == NT_struct) {
        if (static_cast< AstStruct const * >(resolved)->getTypeParameterCount()
            != (typeArgs == nullptr ? 0 : typeArgs->size()))
        {
            ErrorHandler::error0(ErrorCode::WrongNumberOfTypeArguments);
            decl = nullptr;
        } else if (typeArgs != nullptr) {
            AstScope * global = idlc()->scopes()->bottom();
            AstDeclaration * inst = new AstStructInstance(
                static_cast< AstType * >(decl), typeArgs, global);
            decl = global->addDeclaration(inst);
            if (decl != inst) {
                delete inst;
            }
        }
    } else if (decl->isType()) {
        if (typeArgs != nullptr) {
            ErrorHandler::error0(ErrorCode::WrongNumberOfTypeArguments);
            decl = nullptr;
        }
    } else {
        ErrorHandler::noTypeError(decl);
        decl = nullptr;
    }
    delete scopedName;
    delete typeArgs;
    return decl;
}

static bool includes(AstDeclaration const * type1, AstDeclaration const * type2) {
    OSL_ASSERT(type2 != nullptr);
    if (type1 != nullptr) {
        if (type1->getNodeType() == NT_instantiated_struct) {
            AstStructInstance const * inst
                = static_cast< AstStructInstance const * >(type1);
            if (inst->getTypeTemplate() == type2) {
                return true;
            }
            for (DeclList::const_iterator i(inst->getTypeArgumentsBegin());
                 i != inst->getTypeArgumentsEnd(); ++i)
            {
                if (includes(*i, type2)) {
                    return true;
                }
            }
        } else if (type1 == type2) {
            return true;
        }
    }
    return false;
}

// Suppress any warnings from generated code:
#if defined _MSC_VER
#pragma warning(disable: 4702) // unreachable code
#endif
%}
/*
 * Declare the type of values in the grammar
 */
%union {
    ExprType                etval;     /* Expression type */
    AstDeclaration*     dclval;    /* Declaration */
    AstDeclaration const * cdclval;
    DeclList * dclsval;
    AstExpression*      exval;      /* expression value */
    FeDeclarator*           fdval;      /* declarator value */
    FeDeclList*         dlval;      /* declarator list value */
    FeInheritanceHeader*    ihval;      /* inheritance header value */
    OString*     sval;       /* OString value */
    std::vector< OString > * svals;
    sal_Char*           strval; /* sal_Char* value */
    bool                bval;       /* sal_Boolean* value */
    sal_Int64               ival;       /* sal_Int64 value */
    sal_uInt64 uval; /* sal_uInt64 value */
    sal_uInt32          ulval;      /* sal_uInt32 value */
    double                  dval;       /* double value */
    float                   fval;       /* float value */
    std::list< OString >*         slval;      /* StringList value */
    AttributeExceptions::Part attexcpval;
    AttributeExceptions attexcval;
}

/*
 * Token types: These are returned by the lexer
 */

%token <sval>       IDL_IDENTIFIER
%token          IDL_ATTRIBUTE
%token              IDL_BOUND
%token          IDL_CONST
%token          IDL_CONSTANTS
%token              IDL_CONSTRAINED
%token          IDL_ENUM
%token          IDL_EXCEPTION
%token          IDL_INTERFACE
%token          IDL_MAYBEAMBIGUOUS
%token          IDL_MAYBEDEFAULT
%token          IDL_MAYBEVOID
%token          IDL_MODULE
%token          IDL_NEEDS
%token          IDL_OBSERVES
%token          IDL_OPTIONAL
%token          IDL_PROPERTY
%token          IDL_RAISES
%token          IDL_READONLY
%token          IDL_REMOVABLE
%token          IDL_SERVICE
%token          IDL_SEQUENCE
%token          IDL_SINGLETON
%token          IDL_STRUCT
%token          IDL_TYPEDEF
%token              IDL_TRANSIENT

%token          IDL_ANY
%token          IDL_CHAR
%token          IDL_BOOLEAN
%token          IDL_BYTE
%token          IDL_DOUBLE
%token          IDL_FLOAT
%token          IDL_HYPER
%token          IDL_LONG
%token          IDL_SHORT
%token          IDL_VOID
%token          IDL_STRING
%token          IDL_TYPE
%token          IDL_UNSIGNED

%token          IDL_TRUE
%token          IDL_FALSE

%token          IDL_IN
%token          IDL_OUT
%token          IDL_INOUT

%token IDL_GET
%token IDL_SET

%token IDL_PUBLISHED

%token IDL_ELLIPSIS

%token <strval> IDL_LEFTSHIFT
%token <strval> IDL_RIGHTSHIFT
%token <strval>     IDL_SCOPESEPARATOR

%token <ival>       IDL_INTEGER_LITERAL
%token <uval> IDL_INTEGER_ULITERAL
%token <dval>       IDL_FLOATING_PT_LITERAL

/*
 * These are production names:
 */
%type <dclval>  type_dcl
%type <dclval>  exception_name
%type <cdclval> constructed_type_spec enum_type op_type_spec
%type <cdclval> sequence_type_spec simple_type_spec struct_type
%type <cdclval> type_spec
%type <cdclval> fundamental_type type_arg type_or_parameter
%type <dclsval> opt_raises raises exception_list
%type <attexcpval> opt_attribute_get_raises attribute_get_raises
%type <attexcpval> opt_attribute_set_raises attribute_set_raises
%type <dclsval> opt_type_args type_args

%type <sval>    identifier
%type <sval>    interface_decl
%type <sval>    scoped_name inheritance_spec
%type <slval>   scoped_names at_least_one_scoped_name

%type <etval>   const_type integer_type char_type boolean_type
%type <etval>   floating_pt_type any_type signed_int string_type
%type <etval>   unsigned_int base_type_spec byte_type type_type

%type <exval>   expression const_expr or_expr xor_expr and_expr
%type <exval>   add_expr mult_expr unary_expr primary_expr shift_expr
%type <exval>   literal

%type <fdval>   declarator
%type <dlval>   declarators at_least_one_declarator

%type <ihval>   exception_header structure_header interfaceheader

%type <ulval>   flag_header opt_attrflags opt_attrflag
%type <ulval>   direction service_interface_header service_service_header

%type <bval>    optional_inherited_interface opt_rest opt_service_body

%type <attexcval> opt_attribute_block attribute_block_rest opt_attribute_raises

%type <svals> opt_type_params type_params

%%
/*
 * Grammar start here
 */
start : definitions;

definitions :
    definition definitions
    | /* EMPTY */
    ;

definition :
    opt_published publishable_definition
    | module_dcl
    {
        idlc()->setParseState(PS_ModuleDeclSeen);
    }
    ';'
    {
        idlc()->setParseState(PS_NoState);
    }
    | error ';'
    {
        yyerror("definitions");
        yyerrok;
    }
    ;

opt_published:
    IDL_PUBLISHED { idlc()->setPublished(true); }
    | /* empty */ { idlc()->setPublished(false); }
    ;

publishable_definition:
    type_dcl
    {
        idlc()->setParseState(PS_TypeDeclSeen);
    }
    ';'
    {
        idlc()->setParseState(PS_NoState);
    }
    | exception_dcl
    {
        idlc()->setParseState(PS_ExceptionDeclSeen);
    }
    ';'
    {
        idlc()->setParseState(PS_NoState);
    }
    | interface
    {
        idlc()->setParseState(PS_InterfaceDeclSeen);
    }
    ';'
    {
        idlc()->setParseState(PS_NoState);
    }
    | service_dcl
    {
        idlc()->setParseState(PS_ServiceDeclSeen);
    }
    ';'
    {
        idlc()->setParseState(PS_NoState);
    }
    | singleton_dcl
    {
        idlc()->setParseState(PS_SingletonDeclSeen);
    }
    ';'
    {
        idlc()->setParseState(PS_NoState);
    }
    | constants_dcl
    {
        idlc()->setParseState(PS_ConstantsDeclSeen);
    }
    ';'
    {
        idlc()->setParseState(PS_NoState);
    }
    ;

module_dcl :
    IDL_MODULE
    {
        idlc()->setParseState(PS_ModuleSeen);
        idlc()->setPublished(false);
    }
    identifier
    {
        idlc()->setParseState(PS_ModuleIDSeen);
        checkIdentifier($3);

        AstScope*       pScope = idlc()->scopes()->topNonNull();
        AstModule*      pModule = nullptr;

        if ( pScope )
        {
            pModule = new AstModule(*$3, pScope);
            if( AstDeclaration* pExists = pScope->lookupForAdd(pModule) )
            {
                pExists->setInMainfile(idlc()->isInMainFile());
                pExists->setFileName(pModule->getFileName());
                if (pExists->isPredefined())
                {
                    pExists->setPredefined(false);
                    if (pExists->getDocumentation().getLength() == 0 &&
                        pModule->getDocumentation().getLength() > 0)
                    {
                        pExists->setDocumentation(pModule->getDocumentation());
                    }
                }
                delete(pModule);
                pModule = static_cast<AstModule*>(pExists);
            } else
            {
                pScope->addDeclaration(pModule);
            }
            idlc()->scopes()->push(pModule);
        }
        delete $3;
    }
    '{'
    {
        idlc()->setParseState(PS_ModuleSqSeen);
    }
    definitions
    {
        idlc()->setParseState(PS_ModuleBodySeen);
    }
    '}'
    {
        idlc()->setParseState(PS_ModuleQsSeen);
        /*
         * Finished with this module - pop it from the scope stack
         */
        idlc()->scopes()->pop();
    }
    ;

interface :
    interface_dcl
    | forward_dcl
    ;

interface_decl :
    IDL_INTERFACE
    {
        idlc()->setParseState(PS_InterfaceSeen);
    }
    identifier
    {
        idlc()->setParseState(PS_InterfaceIDSeen);
        checkIdentifier($3);
        $$ = $3;
    }
    ;

forward_dcl :
    interface_decl
    {
        idlc()->setParseState(PS_ForwardDeclSeen);

        AstScope*       pScope = idlc()->scopes()->topNonNull();
        AstInterface*   pForward = nullptr;
        AstDeclaration* pDecl = nullptr;

        /*
         * Make a new forward interface node and add it to its enclosing scope
         */
        if ( pScope && $1 )
        {
            pForward = new AstInterface(*$1, nullptr, pScope);

            pDecl = pScope->lookupByName(pForward->getScopedName());
            if ( pDecl )
            {
                if ( (pDecl != pForward) &&
                     (pDecl->getNodeType() == NT_interface) )
                {
                    delete pForward;
                } else
                {
                    ErrorHandler::error2(ErrorCode::RedefScope, scopeAsDecl(pScope), pDecl);
                }
            } else
            {
                /*
                 * Add the interface to its definition scope
                 */
                pScope->addDeclaration(pForward);
            }
        }
        delete $1;
    }
    ;

interface_dcl :
    interfaceheader
    {
        idlc()->setParseState(PS_InterfaceHeadSeen);

        AstScope*       pScope = idlc()->scopes()->topNonNull();
        AstInterface*   pInterface = nullptr;
        AstInterface*   pForward = nullptr;

        /*
         * Make a new interface node and add it to its enclosing scope
         */
        if ( pScope && $1 )
        {
            pInterface = new AstInterface(
                *$1->getName(),
                static_cast< AstInterface const * >(resolveTypedefs($1->getInherits())), pScope);
            if ( AstDeclaration* pDecl = pScope->lookupByName(pInterface->getScopedName()) )
            {
                /*
                 * See if we're defining a forward declared interface.
                 */
                if (pDecl->getNodeType() == NT_interface)
                {
                    pForward = static_cast<AstInterface*>(pDecl);
                    if ( !pForward->isDefined() )
                    {
                        /*
                         * Check if redefining in same scope
                         */
                        if ( pForward->getScope() != pScope )
                        {
                            if ( pForward->getScopedName() != pInterface->getScopedName() )
                            {
                                ErrorHandler::error3(ErrorCode::ScopeConflict,
                                         pInterface, pForward, scopeAsDecl(pScope));
                            }
                        }
                        else if ( !pInterface->isPublished()
                                  && pForward->isPublished() )
                        {
                            ErrorHandler::error0(ErrorCode::PublishedForward);
                        }
                        /*
                         * All OK, set full definition
                         */
                        else
                        {
                            pForward->forwardDefined(*pInterface);
                            delete pInterface;
                            pInterface = pForward;
                        }
                    } else {
                        // special handling for XInterface because it is predefined
                        if ( pForward->isPredefined() &&
                             pForward->getScopedName() == "com::sun::star::uno::XInterface")
                        {
                            /* replace the predefined XInterface */
                            *pForward = *pInterface;
                            delete pInterface;
                            pInterface = pForward;
                        }

                    }
                }
            } else
            {
                /*
                 * Add the interface to its definition scope
                 */
                pScope->addDeclaration(pInterface);
            }
        }
        /*
         * Push it on the scope stack
         */
        idlc()->scopes()->push(pInterface);
        delete $1;
    }
    '{'
    {
        idlc()->setParseState(PS_InterfaceSqSeen);
    }
    exports
    {
        AstInterface * ifc = static_cast< AstInterface * >(
            idlc()->scopes()->topNonNull());
        if (!ifc->hasMandatoryInheritedInterfaces()
            && ifc->getScopedName() != "com::sun::star::uno::XInterface")
        {
            addInheritedInterface(
                ifc, "::com::sun::star::uno::XInterface", false,
                OUString());
        }
        ifc->setDefined();
        idlc()->setParseState(PS_InterfaceBodySeen);
    }
    '}'
    {
        idlc()->setParseState(PS_InterfaceQsSeen);
        /*
         * Done with this interface - pop it off the scopes stack
         */
        idlc()->scopes()->pop();
    }
    | error '}'
    {
        yyerror("interface definition");
        yyerrok;
    }
    ;

interfaceheader :
    interface_decl inheritance_spec
    {
        idlc()->setParseState(PS_InheritSpecSeen);

        $$ = new FeInheritanceHeader(NT_interface, $1, $2, nullptr);
        delete $2;
    }
    ;

inheritance_spec :
    ':'
    {
        idlc()->setParseState(PS_InheritColonSeen);
    }
    scoped_name
    {
        $$ = $3;
    }
    | /* EMPTY */
    {
        $$ = nullptr;
    }
    ;

exports :
    exports export
    | /* EMPTY */
    ;

export :
    attribute
    {
        idlc()->setParseState(PS_AttributeDeclSeen);
    }
    ';'
    {
        idlc()->setParseState(PS_NoState);
    }
    | operation
    {
        idlc()->setParseState(PS_OperationDeclSeen);
    }
    ';'
    {
        idlc()->setParseState(PS_NoState);
    }
    | interface_inheritance_decl
    {
        idlc()->setParseState(PS_InterfaceInheritanceDeclSeen);
    }
    ';'
    {
        idlc()->setParseState(PS_NoState);
    }
    ;

attribute :
    flag_header
    simple_type_spec
    {
        idlc()->setParseState(PS_AttrTypeSeen);
    }
    declarator
    {
        idlc()->setParseState(PS_AttrCompleted);
        if (($1 & ~(AF_BOUND | AF_READONLY)) != AF_ATTRIBUTE) {
            ErrorHandler::flagError(ErrorCode::BadAttributeFlags, $1);
        }
        AstInterface * scope = static_cast< AstInterface * >(
            idlc()->scopes()->top());
        AstAttribute * attr = new AstAttribute(
            $1, FeDeclarator::compose($2), $4->getName(), scope);
        delete $4;
        AstInterface::DoubleMemberDeclarations doubleMembers(
            scope->checkMemberClashes(attr));
        if (doubleMembers.empty()) {
            scope->addMember(attr);
        } else {
            reportDoubleMemberDeclarations(doubleMembers);
        }
        idlc()->scopes()->push(attr);
    }
    opt_attribute_block
    {
        static_cast< AstAttribute * >(idlc()->scopes()->top())->setExceptions(
            $6.get.documentation, $6.get.exceptions, $6.set.documentation,
            $6.set.exceptions);
        delete $6.get.documentation;
        delete $6.get.exceptions;
        delete $6.set.documentation;
        delete $6.set.exceptions;
        idlc()->scopes()->pop();
    }
    ;

flag_header :
    '[' opt_attrflags ']'
    {
        idlc()->setParseState(PS_FlagHeaderSeen);
        $$ = $2;
    }
    ;

opt_attrflags :
    opt_attrflags ',' opt_attrflag
    {
        if ( ($1 & $3) == $3 )
            ErrorHandler::flagError(ErrorCode::DefinedAttributeFlag, $3);

        $$ = $1 | $3;
    }
    | opt_attrflag
    {
        $$ = $1;
    }
    ;

opt_attrflag :
    IDL_ATTRIBUTE
    {
        idlc()->setParseState(PS_AttrSeen);
        $$ = AF_ATTRIBUTE;
    }
    | IDL_PROPERTY
    {
        idlc()->setParseState(PS_PropertySeen);
        $$ = AF_PROPERTY;
    }
    | IDL_READONLY
    {
        idlc()->setParseState(PS_ReadOnlySeen);
        $$ = AF_READONLY;
    }
    | IDL_OPTIONAL
    {
        idlc()->setParseState(PS_OptionalSeen);
        $$ = AF_OPTIONAL;
    }
    | IDL_MAYBEVOID
    {
        idlc()->setParseState(PS_MayBeVoidSeen);
        $$ = AF_MAYBEVOID;
    }
    | IDL_BOUND
    {
        idlc()->setParseState(PS_BoundSeen);
        $$ = AF_BOUND;
    }
    | IDL_CONSTRAINED
    {
        idlc()->setParseState(PS_ConstrainedSeen);
        $$ = AF_CONSTRAINED;
    }
    | IDL_TRANSIENT
    {
        idlc()->setParseState(PS_TransientSeen);
        $$ = AF_TRANSIENT;
    }
    | IDL_MAYBEAMBIGUOUS
    {
        idlc()->setParseState(PS_MayBeAmbigiousSeen);
        $$ = AF_MAYBEAMBIGUOUS;
    }
    | IDL_MAYBEDEFAULT
    {
        idlc()->setParseState(PS_MayBeDefaultSeen);
        $$ = AF_MAYBEDEFAULT;
    }
    | IDL_REMOVABLE
    {
        idlc()->setParseState(PS_RemoveableSeen);
        $$ = AF_REMOVABLE;
    }
    | error ']'
    {
        yyerror("unknown property|attribute flag");
        yyerrok;
    }
    ;

opt_attribute_block:
    '{' attribute_block_rest { $$ = $2; }
    | /* empty */
    {
        $$.get.documentation = nullptr;
        $$.get.exceptions = nullptr;
        $$.set.documentation = nullptr;
        $$.set.exceptions = nullptr;
    }
    ;

attribute_block_rest:
    opt_attribute_raises '}'
    | error '}'
    {
        yyerror("bad attribute raises block");
        yyerrok;
        $$.get.documentation = nullptr;
        $$.get.exceptions = nullptr;
        $$.set.documentation = nullptr;
        $$.set.exceptions = nullptr;
    }
    ;

opt_attribute_raises:
    attribute_get_raises
    opt_attribute_set_raises
    {
        $$.get = $1;
        $$.set = $2;
    }
    | attribute_set_raises
    opt_attribute_get_raises
    {
        $$.get = $2;
        $$.set = $1;
    }
    | /* empty */
    {
        $$.get.documentation = nullptr;
        $$.get.exceptions = nullptr;
        $$.set.documentation = nullptr;
        $$.set.exceptions = nullptr;
    }
    ;

opt_attribute_get_raises:
    attribute_get_raises
    | /* empty */ { $$.documentation = nullptr; $$.exceptions = nullptr; }
    ;

attribute_get_raises:
    IDL_GET raises ';'
    {
        $$.documentation = new OUString(
            OStringToOUString(
                idlc()->getDocumentation(), RTL_TEXTENCODING_UTF8));
        $$.exceptions = $2;
    }
    ;

opt_attribute_set_raises:
    attribute_set_raises
    | /* empty */ { $$.documentation = nullptr; $$.exceptions = nullptr; }
    ;

attribute_set_raises:
    IDL_SET
    {
        if (static_cast< AstAttribute * >(idlc()->scopes()->top())->
            isReadonly())
        {
            ErrorHandler::error0(ErrorCode::ReadOnlyAttributeSetExceptions);
        }
    }
    raises ';'
    {
        $$.documentation = new OUString(
            OStringToOUString(
                idlc()->getDocumentation(), RTL_TEXTENCODING_UTF8));
        $$.exceptions = $3;
    }
    ;

operation :
    op_type_spec
    {
        idlc()->setParseState(PS_OpTypeSeen);
    }
    identifier
    {
        idlc()->setParseState(PS_OpIDSeen);
        checkIdentifier($3);

        AstInterface * pScope = static_cast< AstInterface * >(
            idlc()->scopes()->top());
        AstOperation*   pOp = nullptr;

        /*
         * Create a node representing an operation on an interface
         * and add it to its enclosing scope
         */
        if ( pScope && $1 )
        {
            AstType const *pType = static_cast<AstType const *>($1);
            if ( !pType || (pType->getNodeType() == NT_exception) )
            {
                // type ERROR
            } else
            {
                pOp = new AstOperation(pType, *$3, pScope);

                AstInterface::DoubleMemberDeclarations doubleMembers(
                    pScope->checkMemberClashes(pOp));
                if (doubleMembers.empty()) {
                    pScope->addMember(pOp);
                } else {
                    reportDoubleMemberDeclarations(doubleMembers);
                }
            }
        }
        delete $3;
        /*
         * Push the operation scope onto the scopes stack
         */
        idlc()->scopes()->push(pOp);
    }
    '('
    {
        idlc()->setParseState(PS_OpSqSeen);
    }
    parameters
    {
        idlc()->setParseState(PS_OpParsCompleted);
    }
    ')'
    {
        idlc()->setParseState(PS_OpQsSeen);
    }
    opt_raises
    {
        AstScope*       pScope = idlc()->scopes()->topNonNull();
        AstOperation*   pOp = nullptr;
        /*
         * Add exceptions and context to the operation
         */
        if ( pScope && pScope->getScopeNodeType() == NT_operation)
        {
            pOp = static_cast<AstOperation*>(pScope);

            if ( pOp )
                pOp->setExceptions($11);
        }
        delete $11;
        /*
         * Done with this operation. Pop its scope from the scopes stack
         */
        idlc()->scopes()->pop();
    }
    ;

op_type_spec :
    simple_type_spec
    | IDL_VOID
    {
        $$ = idlc()->scopes()->bottom()->lookupPrimitiveType(ET_void);
    }
    ;

parameters :
    parameter
    | parameters
    ','
    {
        idlc()->setParseState(PS_OpParCommaSeen);
    }
    parameter
    | /* EMPTY */
    | error ','
    {
        yyerror("parameter definition");
        yyerrok;
    }
    ;

parameter :
    '['
    direction
    ']'
    {
        idlc()->setParseState(PS_OpParDirSeen);
    }
    simple_type_spec
    {
        idlc()->setParseState(PS_OpParTypeSeen);
    }
    opt_rest
    declarator
    {
        idlc()->setParseState(PS_OpParDeclSeen);

        AstOperation * pScope = static_cast< AstOperation * >(
            idlc()->scopes()->top());
        AstParameter*   pParam = nullptr;

        /*
         * Create a node representing an argument to an operation
         * Add it to the enclosing scope (the operation scope)
         */
        if ( pScope && $5 && $8 )
        {
            AstType const * pType = FeDeclarator::compose($5);
            if ( pType )
            {
                if (pScope->isConstructor() && $2 != DIR_IN) {
                    ErrorHandler::error0(ErrorCode::ConstructorParameterNotIn);
                }
                if (pScope->isVariadic()) {
                    ErrorHandler::error0(ErrorCode::RestParameterNotLast);
                }
                if ($7) {
                    AstDeclaration const * type = resolveTypedefs(pType);
                    if (type->getNodeType() != NT_predefined
                        || (static_cast< AstBaseType const * >(type)->
                            getExprType() != ET_any))
                    {
                        ErrorHandler::error0(ErrorCode::RestParameterNotAny);
                    }
                    if (pScope->isConstructor()) {
                        if (pScope->getIteratorBegin()
                            != pScope->getIteratorEnd())
                        {
                            ErrorHandler::error0(
                                ErrorCode::ConstructorRestParameterNotFirst);
                        }
                    } else {
                        ErrorHandler::error0(ErrorCode::MethodHasRestParameter);
                    }
                }

                pParam = new AstParameter(
                    static_cast< Direction >($2), $7, pType, $8->getName(),
                    pScope);

                if ( !$8->checkType($5) )
                {
                    // WARNING
                }

                pScope->addDeclaration(pParam);
            }
        }
    }
    | error
    simple_type_spec
    {
        idlc()->setParseState(PS_NoState);
        yyerrok;
    }
    ;

direction :
    IDL_IN
    {
        $$ = DIR_IN;
    }
    | IDL_OUT
    {
        $$ = DIR_OUT;
    }
    | IDL_INOUT
    {
        $$ = DIR_INOUT;
    }
    ;

opt_rest:
    IDL_ELLIPSIS
    {
        $$ = true;
    }
    | /* empty */
    {
        $$ = false;
    }
    ;

opt_raises:
    raises
    | /* empty */
    {
        $$ = nullptr;
    }
    ;

raises:
    IDL_RAISES
    {
        idlc()->setParseState(PS_RaiseSeen);
    }
    '('
    {
        idlc()->setParseState(PS_RaiseSqSeen);
    }
    exception_list
    ')'
    {
        idlc()->setParseState(PS_RaiseQsSeen);
        $$ = $5;
    }
    ;

exception_list:
    exception_name
    {
        $$ = new DeclList;
        $$->push_back($1);
    }
    | exception_list ',' exception_name
    {
        $1->push_back($3);
        $$ = $1;
    }
    ;

exception_name:
    scoped_name
    {
        // The topmost scope is either an AstOperation (for interface methods
        // and service constructors) or an AstAttribute (for interface
        // attributes), so look up exception names in the next-to-topmost scope:
        AstDeclaration * decl = idlc()->scopes()->nextToTop()->lookupByName(
            *$1);
        if (decl == nullptr) {
            ErrorHandler::lookupError(*$1);
        } else if (!ErrorHandler::checkPublished(decl)) {
            decl = nullptr;
        } else if (decl->getNodeType() != NT_exception) {
            ErrorHandler::error1(ErrorCode::IllegalRaises, decl);
            decl = nullptr;
        }
        delete $1;
        $$ = decl;
    }
    ;

interface_inheritance_decl:
    optional_inherited_interface
    IDL_INTERFACE
    {
        idlc()->setParseState(PS_ServiceIFHeadSeen);
    }
    scoped_name
    {
        AstInterface * ifc = static_cast< AstInterface * >(
            idlc()->scopes()->top());
        if (ifc->usesSingleInheritance()) {
            ErrorHandler::error0(ErrorCode::MixedInheritance);
        } else {
            addInheritedInterface(
                ifc, *$4, $1,
                OStringToOUString(
                    idlc()->getDocumentation(), RTL_TEXTENCODING_UTF8));
        }
        delete $4;
    }
    ;

optional_inherited_interface:
    '[' IDL_OPTIONAL ']' { $$ = true; }
    | /* EMPTY */ { $$ = false; }
    ;

constants_exports :
    constants_export constants_exports
    | /* EMPTY */
    ;

constants_export :
    IDL_CONST
    {
        idlc()->setParseState(PS_ConstSeen);
    }
    const_type
    {
        idlc()->setParseState(PS_ConstTypeSeen);
    }
    identifier
    {
        idlc()->setParseState(PS_ConstIDSeen);
        checkIdentifier($5);
    }
    '='
    {
        idlc()->setParseState(PS_ConstAssignSeen);
    }
    expression
    {
        idlc()->setParseState(PS_ConstExprSeen);

        AstScope*       pScope = idlc()->scopes()->topNonNull();
        AstConstant*    pConstant = nullptr;

        if ( $9 && pScope )
        {
            if ( !$9->coerce($3) )
            {
                ErrorHandler::coercionError($9, $3);
            } else
            {
                pConstant = new AstConstant($3, $9, *$5, pScope);
                pScope->addDeclaration(pConstant);
            }
        }
        delete $5;

        idlc()->setParseState(PS_ConstantDeclSeen);
    }
    ';' {};
    ;

constants_dcl :
    IDL_CONSTANTS
    {
        idlc()->setParseState(PS_ConstantsSeen);
    }
    identifier
    {
        idlc()->setParseState(PS_ConstantsIDSeen);
        checkIdentifier($3);
    }
    '{'
    {
        idlc()->setParseState(PS_ConstantsSqSeen);

        AstScope*       pScope = idlc()->scopes()->topNonNull();
        AstConstants*   pConstants = nullptr;

        if ( pScope )
        {
            pConstants = new AstConstants(*$3, pScope);
            if( AstDeclaration* pExists = pScope->lookupForAdd(pConstants) )
            {
                pExists->setInMainfile(idlc()->isInMainFile());
                delete(pConstants);
                pConstants = static_cast<AstConstants*>(pExists);
            } else
            {
                pScope->addDeclaration(pConstants);
            }
            idlc()->scopes()->push(pConstants);
        }
        delete $3;
    }
    constants_exports
    {
        idlc()->setParseState(PS_ConstantsBodySeen);
    }
    '}'
    {
        idlc()->setParseState(PS_ConstantsQsSeen);
        /*
         * Finished with this constants - pop it from the scope stack
         */
        idlc()->scopes()->pop();
    }
    ;

expression : const_expr ;

const_expr : or_expr ;

or_expr :
    xor_expr
    | or_expr '|' xor_expr
    {
        $$ = new AstExpression(ExprComb::Or, $1, $3);
    }
    ;

xor_expr :
    and_expr
    | xor_expr '^' and_expr
    {
        $$ = new AstExpression(ExprComb::Xor, $1, $3);
    }
    ;

and_expr :
    shift_expr
    | and_expr '&' shift_expr
    {
        $$ = new AstExpression(ExprComb::And, $1, $3);
    }
    ;

shift_expr :
    add_expr
    | shift_expr IDL_LEFTSHIFT add_expr
    {
        $$ = new AstExpression(ExprComb::Left, $1, $3);
    }
    | shift_expr IDL_RIGHTSHIFT add_expr
    {
        $$ = new AstExpression(ExprComb::Right, $1, $3);
    }
    ;

add_expr :
    mult_expr
    | add_expr '+' mult_expr
    {
        $$ = new AstExpression(ExprComb::Add, $1, $3);
    }
    | add_expr '-' mult_expr
    {
        $$ = new AstExpression(ExprComb::Minus, $1, $3);
    }
    ;

mult_expr :
    unary_expr
    | mult_expr '*' unary_expr
    {
        $$ = new AstExpression(ExprComb::Mul, $1, $3);
    }
    | mult_expr '/' unary_expr
    {
        $$ = new AstExpression(ExprComb::Div, $1, $3);
    }
    | mult_expr '%' unary_expr
    {
        $$ = new AstExpression(ExprComb::Mod, $1, $3);
    }
    ;

unary_expr :
    primary_expr
    | '+' primary_expr
    {
        $$ = new AstExpression(ExprComb::UPlus, $2, nullptr);
    }
    | '-' primary_expr
    {
        $$ = new AstExpression(ExprComb::UMinus, $2, nullptr);
    }
    | '~' primary_expr
    {
    }
    ;

primary_expr :
    scoped_name
    {
        /*
         * An expression which is a scoped name is not resolved now,
         * but only when it is evaluated (such as when it is assigned
         * as a constant value)
         */
        $$ = new AstExpression($1);
    }
    | literal
    | '(' const_expr ')'
    {
        $$ = $2;
    }
    ;

literal :
    IDL_INTEGER_LITERAL
    {
        $$ = new AstExpression($1);
    }
    | IDL_INTEGER_ULITERAL
    {
        $$ = new AstExpression($1);
    }
    | IDL_FLOATING_PT_LITERAL
    {
        $$ = new AstExpression($1);
    }
    | IDL_TRUE
    {
        $$ = new AstExpression(sal_Int32(1), ET_boolean);
    }
    | IDL_FALSE
    {
        $$ = new AstExpression(sal_Int32(0), ET_boolean);
    }
    ;

const_type :
    integer_type
    | byte_type
    | boolean_type
    | floating_pt_type
    | scoped_name
    {
        AstScope*       pScope = idlc()->scopes()->topNonNull();
        AstDeclaration const * type = nullptr;

        /*
         * If the constant's type is a scoped name, it must resolve
         * to a scalar constant type
         */
        if ( pScope && (type = pScope->lookupByName(*$1)) ) {
            if (!ErrorHandler::checkPublished(type))
            {
                type = nullptr;
                $$ = ET_none;
            }
            else
            {
                type = resolveTypedefs(type);
                if (type->getNodeType() == NT_predefined)
                {
                    $$ = static_cast< AstBaseType const * >(type)->
                        getExprType();
                } else
                    $$ = ET_any;
            }
        } else
            $$ = ET_any;
    }
    ;

exception_header :
    IDL_EXCEPTION
    {
        idlc()->setParseState(PS_ExceptSeen);
    }
    identifier
    {
        idlc()->setParseState(PS_ExceptIDSeen);
        checkIdentifier($3);
    }
    inheritance_spec
    {
        idlc()->setParseState(PS_InheritSpecSeen);

        $$ = new FeInheritanceHeader(NT_exception, $3, $5, nullptr);
        delete $5;
    }
    ;

exception_dcl :
    exception_header
    {
        idlc()->setParseState(PS_ExceptHeaderSeen);

        AstScope*       pScope = idlc()->scopes()->topNonNull();
        AstException*   pExcept = nullptr;

        if ( pScope )
        {
            AstException* pBase = static_cast< AstException* >(
                $1->getInherits());
            pExcept = new AstException(*$1->getName(), pBase, pScope);
            pScope->addDeclaration(pExcept);
        }
        /*
         * Push the scope of the exception on the scopes stack
         */
        idlc()->scopes()->push(pExcept);
        delete $1;
    }
    '{'
    {
        idlc()->setParseState(PS_ExceptSqSeen);
    }
    members
    {
        idlc()->setParseState(PS_ExceptBodySeen);
    }
    '}'
    {
        idlc()->setParseState(PS_ExceptQsSeen);
        /* this exception is finished, pop its scope from the stack */
        idlc()->scopes()->pop();
    }
    ;

property :
    flag_header
    simple_type_spec
    {
        idlc()->setParseState(PS_PropertyTypeSeen);
    }
    at_least_one_declarator
    {
        idlc()->setParseState(PS_PropertyCompleted);

        AstScope*       pScope = idlc()->scopes()->topNonNull();
        AstAttribute*   pAttr = nullptr;
        FeDeclList*     pList = $4;
        FeDeclarator*   pDecl = nullptr;
        AstType const * pType = nullptr;

        if ( pScope->getScopeNodeType() == NT_singleton )
        {
            ErrorHandler::error0(ErrorCode::IllegalAdd);
        } else
        {
            if ( ($1 & AF_ATTRIBUTE) == AF_ATTRIBUTE )
                ErrorHandler::flagError(ErrorCode::WrongAttributeKeyword, AF_ATTRIBUTE);

            if ( ($1 & AF_PROPERTY) != AF_PROPERTY )
                ErrorHandler::flagError(ErrorCode::MissingAttributeKeyword, AF_PROPERTY);

            /*
             * Create nodes representing attributes and add them to the
             * enclosing scope
             */
            if ( pScope && $2 && pList )
            {
                FeDeclList::iterator iter = pList->begin();
                FeDeclList::iterator end = pList->end();

                while (iter != end)
                {
                    pDecl = (*iter);
                    if ( !pDecl )
                    {
                        ++iter;
                        continue;
                    }

                    pType = FeDeclarator::compose($2);

                    if ( !pType )
                    {
                        ++iter;
                        continue;
                    }

                    pAttr = new AstAttribute(NT_property, $1, pType, pDecl->getName(), pScope);

                    pScope->addDeclaration(pAttr);
                    ++iter;
                    delete pDecl;
                }
            }
        }

        if ( pList )
            delete pList;
    }
    | error ';'
    {
        yyerror("property");
        yyerrok;
    }
    ;

service_exports :
    service_exports service_export
    | /* EMPTY */
    ;

service_export :
    service_interface_header
    at_least_one_scoped_name
    ';'
    {
        idlc()->setParseState(PS_ServiceMemberSeen);

        AstScope*           pScope = idlc()->scopes()->topNonNull();
        AstDeclaration*     pDecl = nullptr;
        AstInterfaceMember* pIMember = nullptr;

        if ( pScope->getScopeNodeType() == NT_singleton )
        {
            ErrorHandler::error0(ErrorCode::IllegalAdd);
        } else
        {
            /*
             * Create a node representing a class member.
             * Store it in the enclosing scope
             */
            if ( pScope && $2 )
            {
                for (auto const& elem : *($2))
                {
                    pDecl = pScope->lookupByName(elem);
                    if ( pDecl && (pDecl->getNodeType() == NT_interface) )
                    {
                        /* we relax the strict published check and allow to add new
                         * interfaces if they are optional
                         */
                        bool bOptional = (($1 & AF_OPTIONAL) == AF_OPTIONAL);
                        if ( ErrorHandler::checkPublished(pDecl, bOptional) )
                        {
                            pIMember = new AstInterfaceMember(
                                $1, static_cast<AstInterface*>(pDecl), elem, pScope);
                            pScope->addDeclaration(pIMember);
                        }
                    } else
                    {
                        ErrorHandler::lookupError(ErrorCode::InterfaceMemberLookup, elem, scopeAsDecl(pScope));
                    }
                }
            }
        }
        delete $2;
    }
    | service_service_header
    at_least_one_scoped_name
    ';'
    {
        idlc()->setParseState(PS_ServiceMemberSeen);

        AstScope*         pScope = idlc()->scopes()->topNonNull();
        AstDeclaration*   pDecl = nullptr;
        AstServiceMember* pSMember = nullptr;

        /*
         * Create a node representing a class member.
         * Store it in the enclosing scope
         */
        if ( pScope && $2 )
        {
            for (auto const& elem : *($2))
            {
                pDecl = pScope->lookupByName(elem);
                if ( pDecl && (pDecl->getNodeType() == NT_service) )
                {
                    if ( static_cast< AstService * >(pDecl)->isSingleInterfaceBasedService() || (pScope->getScopeNodeType() == NT_singleton && pScope->nMembers() > 0) )
                        ErrorHandler::error0(ErrorCode::IllegalAdd);
                    else if ( ErrorHandler::checkPublished(pDecl) )
                    {
                        pSMember = new AstServiceMember(
                            $1, static_cast<AstService*>(pDecl), elem, pScope);
                        pScope->addDeclaration(pSMember);
                    }
                } else
                {
                    ErrorHandler::lookupError(ErrorCode::ServiceMemberLookup, elem, scopeAsDecl(pScope));
                }
            }
        }
        delete $2;
    }
    | IDL_OBSERVES
    at_least_one_scoped_name
    ';'
    {
        idlc()->setParseState(PS_ServiceMemberSeen);

        AstScope*       pScope = idlc()->scopes()->topNonNull();
        AstDeclaration* pDecl = nullptr;
        AstObserves*    pObserves = nullptr;

        if ( pScope->getScopeNodeType() == NT_singleton )
        {
            ErrorHandler::error0(ErrorCode::IllegalAdd);
        } else
        {
            /*
             * Create a node representing a class member.
             * Store it in the enclosing scope
             */
            if ( pScope && $2 )
            {
                for (auto const& elem : *($2))
                {
                    pDecl = pScope->lookupByName(elem);
                    if ( pDecl && (pDecl->getNodeType() == NT_interface) )
                    {
                        pObserves = new AstObserves(static_cast<AstInterface*>(pDecl), elem, pScope);
                        pScope->addDeclaration(pObserves);
                    } else
                    {
                        ErrorHandler::lookupError(ErrorCode::InterfaceMemberLookup, elem, scopeAsDecl(pScope));
                    }
                }
            }
        }
        delete $2;
    }
    | IDL_NEEDS
    at_least_one_scoped_name
    ';'
    {
        idlc()->setParseState(PS_ServiceMemberSeen);

        AstScope*       pScope = idlc()->scopes()->topNonNull();
        AstDeclaration* pDecl = nullptr;
        AstNeeds*       pNeeds = nullptr;

        if ( pScope->getScopeNodeType() == NT_singleton )
        {
            ErrorHandler::error0(ErrorCode::IllegalAdd);
        } else
        {
            /*
             * Create a node representing a class member.
             * Store it in the enclosing scope
             */
            if ( pScope && $2 )
            {
                for (auto const& elem : *($2))
                {
                    pDecl = pScope->lookupByName(elem);
                    if ( pDecl && (pDecl->getNodeType() == NT_service) )
                    {
                        pNeeds = new AstNeeds(static_cast<AstService*>(pDecl), elem, pScope);
                        pScope->addDeclaration(pNeeds);
                    } else
                    {
                        ErrorHandler::lookupError(ErrorCode::ServiceMemberLookup, elem, scopeAsDecl(pScope));
                    }
                }
            }
        }
        delete $2;
    }
    | property
    ';'
    {
        idlc()->setParseState(PS_PropertyDeclSeen);
    }
    ;

service_interface_header :
    IDL_INTERFACE
    {
        idlc()->setParseState(PS_ServiceIFHeadSeen);
        $$ = AF_INVALID;
    }
    | flag_header
    IDL_INTERFACE
    {
        idlc()->setParseState(PS_ServiceIFHeadSeen);
        if ( (AF_OPTIONAL != $1) && ( AF_INVALID != $1) )
            ErrorHandler::flagError(ErrorCode::ExpectedOptional, $1);
        $$ = $1;
    }
    ;

service_service_header :
    IDL_SERVICE
    {
        idlc()->setParseState(PS_ServiceSHeadSeen);
        $$ = AF_INVALID;
    }
    | flag_header
    IDL_SERVICE
    {
        idlc()->setParseState(PS_ServiceSHeadSeen);
        if ( (AF_OPTIONAL != $1) && ( AF_INVALID != $1) )
            ErrorHandler::flagError(ErrorCode::ExpectedOptional, $1);
        $$ = $1;
    }
    ;

service_dcl :
    IDL_SERVICE
    {
        idlc()->setParseState(PS_ServiceSeen);
    }
    identifier
    {
        idlc()->setParseState(PS_ServiceIDSeen);
        checkIdentifier($3);

        AstScope*   pScope = idlc()->scopes()->topNonNull();
        AstService* pService = nullptr;

        /*
         * Make a new service and add it to the enclosing scope
         */
        if (pScope != nullptr)
        {
            pService = new AstService(*$3, pScope);
            pScope->addDeclaration(pService);
        }
        delete $3;
        /*
         * Push it on the stack
         */
        idlc()->scopes()->push(pService);
    }
    service_dfn
    {
        /* this service is finished, pop its scope from the stack */
        idlc()->scopes()->pop();
    }
    ;

service_dfn:
    service_interface_dfn
    | service_obsolete_dfn
    ;

service_interface_dfn:
    ':' scoped_name
    {
        AstScope * scope = idlc()->scopes()->nextToTop();
            // skip the scope pushed by service_dcl
        AstDeclaration * decl = scope->lookupByName(*$2);
        if (decl != nullptr
            && resolveTypedefs(decl)->getNodeType() == NT_interface)
        {
            if (ErrorHandler::checkPublished(decl)) {
                idlc()->scopes()->top()->addDeclaration(decl);
            }
        } else {
            ErrorHandler::lookupError(
                ErrorCode::InterfaceMemberLookup, *$2, scopeAsDecl(scope));
        }
        delete $2;
    }
    opt_service_body
    {
        AstService * s = static_cast< AstService * >(idlc()->scopes()->top());
        if (s != nullptr) {
            s->setSingleInterfaceBasedService();
            s->setDefaultConstructor(!$4);
        }
    }
    ;

opt_service_body:
    service_body { $$ = true; }
    | /* empty */ { $$ = false; }
    ;

service_body:
    '{'
    constructors
    '}'
    ;

constructors:
    constructors constructor
    | /* empty */
    ;

constructor:
    identifier
    {
        checkIdentifier($1);
        AstScope * scope = idlc()->scopes()->top();
        AstOperation * ctor = new AstOperation(nullptr, *$1, scope);
        delete $1;
        scope->addDeclaration(ctor);
        idlc()->scopes()->push(ctor);
    }
    '('
    parameters
    ')'
    opt_raises
    {
        static_cast< AstOperation * >(idlc()->scopes()->top())->setExceptions(
            $6);
        delete $6;
        idlc()->scopes()->pop();
        if (static_cast< AstService * >(idlc()->scopes()->top())->
            checkLastConstructor())
        {
            ErrorHandler::error0(ErrorCode::SimilarConstructors);
        }
    }
    ';'
    ;

singleton_dcl :
    IDL_SINGLETON
    {
        idlc()->setParseState(PS_SingletonSeen);
    }
    identifier
    {
        idlc()->setParseState(PS_SingletonIDSeen);
        checkIdentifier($3);

        AstScope*   pScope = idlc()->scopes()->topNonNull();
        AstService* pService = nullptr;

        /*
         * Make a new service and add it to the enclosing scope
         */
        if (pScope != nullptr)
        {
            pService = new AstService(NT_singleton, *$3, pScope);
            pScope->addDeclaration(pService);
        }
        delete $3;
        /*
         * Push it on the stack
         */
        idlc()->scopes()->push(pService);
    }
    singleton_dfn
    {
        /* this singelton is finished, pop its scope from the stack */
        idlc()->scopes()->pop();
    }
    ;

singleton_dfn:
    singleton_interface_dfn
    | service_obsolete_dfn
    ;

singleton_interface_dfn:
    ':' scoped_name
    {
        AstScope * scope = idlc()->scopes()->nextToTop();
            // skip the scope (needlessly) pushed by singleton_dcl
        AstDeclaration * decl = scope->lookupByName(*$2);
        if (decl != nullptr
            && resolveTypedefs(decl)->getNodeType() == NT_interface)
        {
            if (ErrorHandler::checkPublished(decl)) {
                idlc()->scopes()->top()->addDeclaration(decl);
            }
        } else {
            ErrorHandler::lookupError(
                ErrorCode::InterfaceMemberLookup, *$2, scopeAsDecl(scope));
        }
        delete $2;
    }
    ;

service_obsolete_dfn:
    '{'
    {
        idlc()->setParseState(
            idlc()->scopes()->top()->getScopeNodeType() == NT_service
            ? PS_ServiceSqSeen : PS_SingletonSqSeen);
    }
    service_exports
    {
        idlc()->setParseState(
            idlc()->scopes()->top()->getScopeNodeType() == NT_service
            ? PS_ServiceBodySeen : PS_SingletonBodySeen);
    }
    '}'
    {
        idlc()->setParseState(
            idlc()->scopes()->top()->getScopeNodeType() == NT_service
            ? PS_ServiceQsSeen : PS_SingletonQsSeen);
    }
    ;

type_dcl :
    IDL_TYPEDEF
    {
        idlc()->setParseState(PS_TypedefSeen);
    }
    type_declarator {}
    | struct_type {}
    | enum_type {}
    ;

type_declarator :
    type_spec
    {
        idlc()->setParseState(PS_TypeSpecSeen);
        if ($1 != nullptr && $1->getNodeType() == NT_instantiated_struct) {
            ErrorHandler::error0(ErrorCode::InstantiatedStructTypeTypedef);
        }
    }
    at_least_one_declarator
    {
        idlc()->setParseState(PS_DeclaratorsSeen);

        AstScope*       pScope = idlc()->scopes()->topNonNull();
        AstTypeDef*     pTypeDef = nullptr;
        FeDeclList*     pList = $3;
        FeDeclarator*   pDecl = nullptr;
        AstType const * pType = nullptr;

        /*
         * Create nodes representing typedefs and add them to the
         * enclosing scope
         */
        if ( pScope && $1 && pList )
        {
            FeDeclList::iterator iter = pList->begin();
            FeDeclList::iterator end = pList->end();

            while (iter != end)
            {
                pDecl = (*iter);
                if ( !pDecl )
                {
                    ++iter;
                    continue;
                }

                pType = FeDeclarator::compose($1);

                if ( !pType )
                {
                    ++iter;
                    continue;
                }

                pTypeDef = new AstTypeDef(pType, pDecl->getName(), pScope);

                pScope->addDeclaration(pTypeDef);
                ++iter;
                delete pDecl;
            }
            delete pList;
        }
    }
    ;

at_least_one_declarator :
    declarator declarators
    {
        if ( $2 )
        {
            $2->push_back($1);
            $$ = $2;
        } else
        {
            FeDeclList* pList = new FeDeclList;
            pList->push_back($1);
            $$ = pList;
        }
    }
    ;

declarators :
    declarators
    ','
    {
        idlc()->setParseState(PS_DeclsCommaSeen);
    }
    declarator
    {
        idlc()->setParseState(PS_DeclsDeclSeen);
        if ( $1 )
        {
            $1->push_back($4);
            $$ = $1;
        } else
        {
            FeDeclList* pList = new FeDeclList;
            pList->push_back($4);
            $$ = pList;
        }
    }
    | /* EMPTY */
    {
        $$ = nullptr;
    }
    ;

declarator :
    identifier
    {
        // For historic reasons, the struct com.sun.star.uno.Uik contains
        // members with illegal names (of the form "m_DataN"); avoid useless
        // warnings about them:
        AstScope * scope = idlc()->scopes()->top();
        if (scope == nullptr || scope->getScopeNodeType() != NT_struct
            || (scopeAsDecl(scope)->getScopedName()
                != "com::sun::star::uno::Uik"))
        {
            checkIdentifier($1);
        }

        $$ = new FeDeclarator(*$1);
        delete $1;
    }
    ;

at_least_one_scoped_name :
    scoped_name scoped_names
    {
        if ($2)
        {
            $2->push_front(*$1);
            $$ = $2;
        } else
        {
            std::list< OString >* pScopedNames = new std::list< OString >;
            // coverity[copy_paste_error : FALSE] - this is not a cut and paste
            pScopedNames->push_back(*$1);
            $$ = pScopedNames;
        }
        delete $1;
    }
    ;

scoped_names :
    scoped_names
    ','
    {
        idlc()->setParseState(PS_SNListCommaSeen);
    }
    scoped_name
    {
        idlc()->setParseState(PS_ScopedNameSeen);
        if ($1)
        {
            $1->push_back(*$4);
            $$ = $1;
        } else
        {
            std::list< OString >* pNames = new std::list< OString >;
            pNames->push_back(*$4);
            $$ = pNames;
        }
        delete $4;
    }
    | /* EMPTY */
    {
        $$ = nullptr;
    }
    ;

scoped_name :
    identifier
    {
        idlc()->setParseState(PS_SN_IDSeen);
        checkIdentifier($1);
        $$ = $1;
    }
    | IDL_SCOPESEPARATOR
    {
        idlc()->setParseState(PS_ScopeDelimSeen);
    }
    identifier
    {
        checkIdentifier($3);
        OString* pName = new OString("::");
        *pName += *$3;
        delete $3;
        $$ = pName;
    }
    | scoped_name
    IDL_SCOPESEPARATOR
    {
    }
    identifier
    {
        checkIdentifier($4);
        *$1 += OString("::");
        *$1 += *$4;
        delete $4;
        $$ = $1;
    }
    ;

type_spec :
    simple_type_spec
    | constructed_type_spec
    ;

simple_type_spec :
    fundamental_type
    | scoped_name opt_type_args
    {
        $$ = createNamedType($1, $2);
    }
    ;

fundamental_type:
    base_type_spec
    {
        $$ = idlc()->scopes()->bottom()->lookupPrimitiveType($1);
    }
    | sequence_type_spec
    ;

opt_type_args:
    '<' type_args '>' { $$ = $2; }
    | /* empty */ { $$ = nullptr; }
    ;

type_args:
    type_arg
    {
        $$ = new DeclList;
        $$->push_back(const_cast< AstDeclaration * >($1)); //TODO: const_cast
    }
    | type_args ',' type_arg
    {
        $1->push_back(const_cast< AstDeclaration * >($3)); //TODO: const_cast
        $$ = $1;
    }
    ;

type_arg:
    simple_type_spec
    {
        if ($1 != nullptr && static_cast< AstType const * >($1)->isUnsigned()) {
            ErrorHandler::error0(ErrorCode::UnsignedTypeArgument);
        }
        $$ = $1;
    }
    ;

base_type_spec :
    integer_type
    | floating_pt_type
    | char_type
    | boolean_type
    | byte_type
    | any_type
    | type_type
    | string_type
    ;

integer_type :
    signed_int
    | unsigned_int
    ;

signed_int :
    IDL_LONG
    {
        $$ = ET_long;
    }
    | IDL_HYPER
    {
        $$ = ET_hyper;
    }
    | IDL_SHORT
    {
        $$ = ET_short;
    }
    ;

unsigned_int :
    IDL_UNSIGNED IDL_LONG
    {
        $$ = ET_ulong;
    }
    | IDL_UNSIGNED IDL_HYPER
    {
        $$ = ET_uhyper;
    }
    | IDL_UNSIGNED IDL_SHORT
    {
        $$ = ET_ushort;
    }
    ;

floating_pt_type :
    IDL_DOUBLE
    {
        $$ = ET_double;
    }
    | IDL_FLOAT
    {
        $$ = ET_float;
    }
    ;

char_type :
    IDL_CHAR
    {
        $$ = ET_char;
    }
    ;

byte_type :
    IDL_BYTE
    {
        $$ = ET_byte;
    }
    ;

boolean_type :
    IDL_BOOLEAN
    {
        $$ = ET_boolean;
    }
    ;

any_type :
    IDL_ANY
    {
        $$ = ET_any;
    }
    ;

type_type :
    IDL_TYPE
    {
        $$ = ET_type;
    }
    ;

string_type :
    IDL_STRING
    {
        $$ = ET_string;
    }
    ;

constructed_type_spec :
    struct_type
    | enum_type
    ;

sequence_type_spec :
    IDL_SEQUENCE
    {
        idlc()->setParseState(PS_SequenceSeen);
        /*
         * Push a sequence marker on scopes stack
         */
        idlc()->scopes()->push(nullptr);
    }
    '<'
    {
        idlc()->setParseState(PS_SequenceSqSeen);
    }
    simple_type_spec
    {
        idlc()->setParseState(PS_SequenceTypeSeen);
    }
    '>'
    {
        idlc()->setParseState(PS_SequenceQsSeen);
        /*
         * Remove sequence marker from scopes stack
         */
        if (idlc()->scopes()->top() == nullptr)
            idlc()->scopes()->pop();
        /*
         * Create a node representing a sequence
         */
        AstScope* pScope = idlc()->scopes()->bottom();
        AstDeclaration* pDecl = nullptr;
        AstDeclaration* pSeq = nullptr;

        if ( $5 )
        {
            AstType const *pType = static_cast<AstType const *>($5);
            if ( pType )
            {
                pSeq = new AstSequence(pType, pScope);
                /*
                 * Add this AstSequence to the types defined in the global scope
                 */
                pDecl = pScope->addDeclaration(pSeq);
                if ( pSeq != pDecl )
                {
                    // if sequence type already defined then use it
                    delete pSeq;
                    pSeq = pDecl;
                }
            }
        }
        $$ = pSeq;
    }
    | error '>'
    {
        yyerror("sequence declaration");
        yyerrok;
        $$ = nullptr;
    }
    ;

struct_type :
    structure_header
    {
        idlc()->setParseState(PS_StructHeaderSeen);

        AstScope*   pScope = idlc()->scopes()->topNonNull();
        AstStruct*  pStruct = nullptr;

        if ( pScope )
        {
            AstStruct const* pBase= static_cast< AstStruct const* >(resolveTypedefs($1->getInherits()));
            pStruct = new AstStruct(
                *$1->getName(), $1->getTypeParameters(), pBase, pScope);
            pScope->addDeclaration(pStruct);
        }
        /*
         * Push the scope of the struct on the scopes stack
         */
        idlc()->scopes()->push(pStruct);
        delete $1;
    }
    '{'
    {
        idlc()->setParseState(PS_StructSqSeen);
    }
    at_least_one_member
    {
        idlc()->setParseState(PS_StructBodySeen);
    }
    '}'
    {
        idlc()->setParseState(PS_StructQsSeen);
        /* this exception is finished, pop its scope from the stack */
        idlc()->scopes()->pop();
    }
    ;

structure_header :
    IDL_STRUCT
    {
        idlc()->setParseState(PS_StructSeen);
    }
    identifier
    {
        idlc()->setParseState(PS_StructIDSeen);
        checkIdentifier($3);
    }
    opt_type_params
    inheritance_spec
    {
        idlc()->setParseState(PS_InheritSpecSeen);

        // Polymorphic struct type templates with base types would cause various
        // problems in language bindings, so forbid them here.  For example,
        // GCC prior to version 3.4 fails with code like
        //
        //  struct Base { ... };
        //  template< typename typeparam_T > struct Derived: public Base {
        //      int member1 CPPU_GCC3_ALIGN(Base);
        //      ... };
        //
        // (Note that plain struct types with instantiated polymorphic struct
        // type bases, which might also cause problems in language bindings, are
        // already rejected on a syntactic level.)
        if ($5 != nullptr && $6 != nullptr) {
            ErrorHandler::error0(ErrorCode::StructTypeTemplateWithBase);
        }

        $$ = new FeInheritanceHeader(NT_struct, $3, $6, $5);
        delete $5;
        delete $6;
    }
    ;

opt_type_params:
    '<' type_params '>' { $$ = $2; }
    | /* empty */ { $$ = nullptr; }
    ;

type_params:
    identifier
    {
        $$ = new std::vector< OString >;
        $$->push_back(*$1);
        delete $1;
    }
    | type_params ',' identifier
    {
        if (std::find($1->begin(), $1->end(), *$3) != $1->end()) {
            ErrorHandler::error0(ErrorCode::IdenticalTypeParameters);
        }
        $1->push_back(*$3);
        delete $3;
        $$ = $1;
    }
    ;

at_least_one_member : member members ;

members :
    members member
    | /* EMPTY */
    ;

member :
    type_or_parameter
    {
        idlc()->setParseState(PS_MemberTypeSeen);
    }
    at_least_one_declarator
    {
        idlc()->setParseState(PS_MemberDeclsSeen);
    }
    ';'
    {
        idlc()->setParseState(PS_MemberDeclsCompleted);

        AstScope*       pScope = idlc()->scopes()->topNonNull();
        AstMember*      pMember = nullptr;
        FeDeclList*     pList = $3;
        FeDeclarator*   pDecl = nullptr;
        AstType const * pType = nullptr;

        // !!! check recursive type

        if ( pScope && pList && $1 )
        {
            FeDeclList::iterator iter = pList->begin();
            FeDeclList::iterator end = pList->end();
            while (iter != end)
            {
                pDecl = (*iter);
                if ( !pDecl )
                {
                    ++iter;
                    continue;
                }

                pType = FeDeclarator::compose($1);

                if ( !pType )
                {
                    ++iter;
                    continue;
                }

                pMember = new AstMember(pType, pDecl->getName(), pScope);

                if ( !pDecl->checkType($1) )
                {
                    // WARNING
                }

                pScope->addDeclaration(pMember);
                ++iter;
                delete pDecl;
            }
            delete pList;
        }
    }
    | error ';'
    {
        yyerror("member definition");
        yyerrok;
    }
    ;

type_or_parameter:
    fundamental_type
    | scoped_name opt_type_args
    {
        AstDeclaration const * decl = nullptr;
        AstStruct * scope = static_cast< AstStruct * >(idlc()->scopes()->top());
        if (scope != nullptr && $2 == nullptr) {
            decl = scope->findTypeParameter(*$1);
        }
        if (decl != nullptr) {
            delete $1;
            delete $2;
        } else {
            decl = createNamedType($1, $2);
            if (scope != nullptr && includes(decl, scopeAsDecl(scope))) {
                ErrorHandler::error1(
                    ErrorCode::RecursiveType, scopeAsDecl(scope));
                decl = nullptr;
            }
        }
        $$ = decl;
    }
    ;

enum_type :
    IDL_ENUM
    {
        idlc()->setParseState(PS_EnumSeen);
    }
    identifier
    {
        idlc()->setParseState(PS_EnumIDSeen);
        checkIdentifier($3);

        AstScope*       pScope = idlc()->scopes()->topNonNull();
        AstEnum*        pEnum = nullptr;

        /*
         * Create a node representing an enum and add it to its
         * enclosing scope
         */
        if (pScope != nullptr)
        {
            pEnum = new AstEnum(*$3, pScope);
            /*
             * Add it to its defining scope
             */
            pScope->addDeclaration(pEnum);
        }
        delete $3;
        /*
         * Push the enum scope on the scopes stack
         */
        idlc()->scopes()->push(pEnum);

    }
    '{'
    {
        idlc()->setParseState(PS_EnumSqSeen);
    }
    at_least_one_enumerator
    {
        idlc()->setParseState(PS_EnumBodySeen);
    }
    '}'
    {
        idlc()->setParseState(PS_EnumQsSeen);
        /*
         * Done with this enum. Pop its scope from the scopes stack
         */
        if (idlc()->scopes()->top() == nullptr)
            $$ = nullptr;
        else
        {
            $$ = static_cast<AstEnum*>(idlc()->scopes()->topNonNull());
            idlc()->scopes()->pop();
        }
    }
    ;

at_least_one_enumerator : enumerator enumerators ;

enumerators :
    enumerators
    ','
    {
        idlc()->setParseState(PS_EnumCommaSeen);
    }
    enumerator
    | /* EMPTY */
    | error ','
    {
        yyerror("enumerator definition");
        yyerrok;
    }
    ;

enumerator :
    identifier
    {
        checkIdentifier($1);

        AstScope*       pScope = idlc()->scopes()->topNonNull();
        AstEnum*        pEnum = nullptr;
        AstConstant*    pEnumVal = nullptr;

        if ( pScope && pScope->getScopeNodeType() == NT_enum)
        {
            pEnum = static_cast<AstEnum*>(pScope);
            if (pEnum && $1)
            {
                AstExpression* pExpr = new AstExpression(pEnum->getEnumValueCount());
                pEnumVal = new AstConstant(ET_long , NT_enum_val,
                                           pExpr, *$1, pScope);
            }
            if ( pEnum->checkValue(pEnumVal->getConstValue()) )
                ErrorHandler::error1(ErrorCode::Eval, pEnum);

            pScope->addDeclaration(pEnumVal);
        }
        delete $1;
    }
    | identifier
    '='
    const_expr
    {
        checkIdentifier($1);

        AstScope*       pScope = idlc()->scopes()->topNonNull();
        AstEnum*        pEnum = nullptr;
        AstConstant*    pEnumVal = nullptr;

        if ( $3 && pScope && pScope->getScopeNodeType() == NT_enum)
        {
            $3->evaluate();
            if ( $3->coerce(ET_long) )
            {
                pEnum = static_cast<AstEnum*>(pScope);
                if (pEnum)
                {
                    pEnumVal = new AstConstant(ET_long , NT_enum_val,
                                               $3, *$1, pScope);
                }
                if ( pEnum->checkValue(pEnumVal->getConstValue()) )
                    ErrorHandler::error1(ErrorCode::Eval, pEnum);

                pScope->addDeclaration(pEnumVal);
            } else
            {
                ErrorHandler::coercionError($3, ET_long);
                delete $3;
            }
        }
        delete $1;
    }
    ;

identifier:
    IDL_IDENTIFIER
    | IDL_GET { $$ = new OString("get"); }
    | IDL_SET { $$ = new OString("set"); }
    | IDL_PUBLISHED { $$ = new OString("published"); }
    ;

%%

/*
 * Report an error situation discovered in a production
 */
void yyerror(char const *errmsg)
{
    ErrorHandler::syntaxError(idlc()->getParseState(), idlc()->getLineNumber(), errmsg);
    idlc()->setParseState(PS_NoState);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
