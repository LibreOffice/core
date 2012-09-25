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

#ifndef _IDLC_IDLC_HXX_
#include <idlc/idlc.hxx>
#endif
#ifndef _IDLC_ERRORHANDLER_HXX_
#include <idlc/errorhandler.hxx>
#endif
#ifndef _IDLC_FEHELPER_HXX_
#include <idlc/fehelper.hxx>
#endif
#ifndef _IDLC_EXPRESSION_HXX_
#include <idlc/astexpression.hxx>
#endif
#ifndef _IDLC_ASTCONSTANTS_HXX_
#include <idlc/astconstants.hxx>
#endif
#ifndef _IDLC_ASTCONSTANT_HXX_
#include <idlc/astconstant.hxx>
#endif
#ifndef _IDLC_ASTARRAY_HXX_
#include <idlc/astarray.hxx>
#endif
#ifndef _IDLC_ASTBASETYPE_HXX_
#include <idlc/astbasetype.hxx>
#endif
#ifndef _IDLC_ASTTYPEDEF_HXX_
#include <idlc/asttypedef.hxx>
#endif
#ifndef _IDLC_ASTEXCEPTION_HXX_
#include <idlc/astexception.hxx>
#endif
#ifndef _IDLC_ASTMEMBER_HXX_
#include <idlc/astmember.hxx>
#endif
#ifndef _IDLC_ASTENUM_HXX_
#include <idlc/astenum.hxx>
#endif
#ifndef _IDLC_ASTSEQUENCE_HXX_
#include <idlc/astsequence.hxx>
#endif
#ifndef _IDLC_ASTATTRIBUTE_HXX_
#include <idlc/astattribute.hxx>
#endif
#ifndef _IDLC_ASTOPERATION_HXX_
#include <idlc/astoperation.hxx>
#endif
#ifndef _IDLC_ASTPARAMETER_HXX_
#include <idlc/astparameter.hxx>
#endif
#ifndef _IDLC_ASTINTERFACEMEMBER_HXX_
#include <idlc/astinterfacemember.hxx>
#endif
#ifndef _IDLC_ASTSERVICEMEMBER_HXX_
#include <idlc/astservicemember.hxx>
#endif
#ifndef _IDLC_ASTOBSERVES_HXX_
#include <idlc/astobserves.hxx>
#endif
#ifndef _IDLC_ASTNEEDS_HXX_
#include <idlc/astneeds.hxx>
#endif
#ifndef _IDLC_ASTUNION_HXX_
#include <idlc/astunion.hxx>
#endif
#include "idlc/aststructinstance.hxx"

#include "attributeexceptions.hxx"

#include "rtl/strbuf.hxx"

#include <algorithm>
#include <vector>


#define YYDEBUG 1
#if !(defined MACOSX && defined PPC)
#define YYERROR_VERBOSE 1
#endif

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OStringToOUString;
using ::rtl::OStringBuffer;

extern int yylex(void);
void yyerror(char const *);

void checkIdentifier(::rtl::OString* id)
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
                ::rtl::OStringBuffer msg(25 + id->getLength());
                msg.append("mismatched identifier '");
                msg.append(*id);
                msg.append("'");
                idlc()->error()->syntaxError(idlc()->getParseState(),
                                         idlc()->getLineNumber(),
                                         msg.getStr());
            }
            else
                idlc()->error()->warning0(WIDL_WRONG_NAMING_CONV, id->getStr());
        }
}

void reportDoubleMemberDeclarations(
    AstInterface::DoubleMemberDeclarations const & doubleMembers)
{
    for (AstInterface::DoubleMemberDeclarations::const_iterator i(
             doubleMembers.begin());
         i != doubleMembers.end(); ++i)
    {
        idlc()->error()->error2(EIDL_DOUBLE_MEMBER, i->first, i->second);
    }
}

void addInheritedInterface(
    AstInterface * ifc, rtl::OString const & name, bool optional,
    rtl::OUString const & documentation)
{
    AstDeclaration * decl = ifc->lookupByName(name);
    AstDeclaration const * resolved = resolveTypedefs(decl);
    if (resolved != 0 && resolved->getNodeType() == NT_interface) {
        if (idlc()->error()->checkPublished(decl)) {
            if (!static_cast< AstInterface const * >(resolved)->isDefined()) {
                idlc()->error()->inheritanceError(
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
                    for (AstInterface::DoubleInterfaceDeclarations::iterator i(
                             doubleDecls.interfaces.begin());
                         i != doubleDecls.interfaces.end(); ++i)
                    {
                        idlc()->error()->error1(
                            EIDL_DOUBLE_INHERITANCE, *i);
                    }
                    reportDoubleMemberDeclarations(doubleDecls.members);
                }
            }
        }
    } else {
        idlc()->error()->lookupError(
            EIDL_INTERFACEMEMBER_LOOKUP, name, scopeAsDecl(ifc));
    }
}

AstDeclaration const * createNamedType(
    rtl::OString const * scopedName, DeclList const * typeArgs)
{
    AstDeclaration * decl = idlc()->scopes()->topNonNull()->lookupByName(
        *scopedName);
    AstDeclaration const * resolved = resolveTypedefs(decl);
    if (decl == 0) {
        idlc()->error()->lookupError(*scopedName);
    } else if (!idlc()->error()->checkPublished(decl)) {
        decl = 0;
    } else if (resolved->getNodeType() == NT_struct) {
        if (static_cast< AstStruct const * >(resolved)->getTypeParameterCount()
            != (typeArgs == 0 ? 0 : typeArgs->size()))
        {
            idlc()->error()->error0(EIDL_WRONG_NUMBER_OF_TYPE_ARGUMENTS);
            decl = 0;
        } else if (typeArgs != 0) {
            AstScope * global = idlc()->scopes()->bottom();
            AstDeclaration * inst = new AstStructInstance(
                static_cast< AstType * >(decl), typeArgs, global);
            decl = global->addDeclaration(inst);
            if (decl != inst) {
                delete inst;
            }
        }
    } else if (decl->isType()) {
        if (typeArgs != 0) {
            idlc()->error()->error0(EIDL_WRONG_NUMBER_OF_TYPE_ARGUMENTS);
            decl = 0;
        }
    } else {
        idlc()->error()->noTypeError(decl);
        decl = 0;
    }
    delete scopedName;
    delete typeArgs;
    return decl;
}

bool includes(AstDeclaration const * type1, AstDeclaration const * type2) {
    OSL_ASSERT(type2 != 0);
    if (type1 != 0) {
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
#if defined __SUNPRO_CC
#pragma disable_warn
#elif defined _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable: 4273 4701 4702 4706)
#endif
%}
/*
 * Declare the type of values in the grammar
 */
%union {
	ExprType				etval;     /* Expression type */
	AstDeclaration*		dclval;    /* Declaration */
    AstDeclaration const * cdclval;
    DeclList * dclsval;
	AstExpression*		exval;		/* expression value */
	ExprList*				exlval;	/* expression list value */
	FeDeclarator*			fdval;		/* declarator value */
	FeDeclList*			dlval;		/* declarator list value */
	FeInheritanceHeader*	ihval;		/* inheritance header value */
	::rtl::OString*		sval;		/* OString value */
    std::vector< rtl::OString > * svals;
	sal_Char* 			strval;	/* sal_Char* value */
	bool				bval;		/* sal_Boolean* value */
	sal_Int64				ival;		/* sal_Int64 value */
    sal_uInt64 uval; /* sal_uInt64 value */
	sal_uInt32			ulval;		/* sal_uInt32 value */
	double					dval;		/* double value */
	float					fval;		/* float value */
	StringList*			slval;		/* StringList value	*/
	LabelList*			llval;		/* LabelList value	*/
	AstUnionLabel*		lbval;		/* union label value */
	AstMember*			mval;		/* member value */
    AttributeExceptions::Part attexcpval;
    AttributeExceptions attexcval;
}

/*
 * Token types: These are returned by the lexer
 */

%token <sval>		IDL_IDENTIFIER
%token 			IDL_ATTRIBUTE
%token				IDL_BOUND
%token 			IDL_CASE
%token 			IDL_CONST
%token 			IDL_CONSTANTS
%token				IDL_CONSTRAINED
%token 			IDL_DEFAULT
%token 			IDL_ENUM
%token 			IDL_EXCEPTION
%token 			IDL_INTERFACE
%token 			IDL_MAYBEAMBIGUOUS
%token 			IDL_MAYBEDEFAULT
%token 			IDL_MAYBEVOID
%token 			IDL_MODULE
%token 			IDL_NEEDS
%token 			IDL_OBSERVES
%token 			IDL_OPTIONAL
%token 			IDL_PROPERTY
%token 			IDL_RAISES
%token 			IDL_READONLY
%token 			IDL_REMOVEABLE
%token 			IDL_SERVICE
%token 			IDL_SEQUENCE
%token 			IDL_SINGLETON
%token 			IDL_STRUCT
%token 			IDL_SWITCH
%token 			IDL_TYPEDEF
%token				IDL_TRANSIENT
%token 			IDL_UNION

%token 			IDL_ANY
%token 			IDL_CHAR
%token 			IDL_BOOLEAN
%token 			IDL_BYTE
%token 			IDL_DOUBLE
%token 			IDL_FLOAT
%token 			IDL_HYPER
%token 			IDL_LONG
%token 			IDL_SHORT
%token 			IDL_VOID
%token 			IDL_STRING
%token 			IDL_TYPE
%token 			IDL_UNSIGNED

%token 			IDL_TRUE
%token 			IDL_FALSE

%token 			IDL_IN
%token 			IDL_OUT
%token 			IDL_INOUT
%token 			IDL_ONEWAY

%token IDL_GET
%token IDL_SET

%token IDL_PUBLISHED

%token IDL_ELLIPSIS

%token <strval>	IDL_LEFTSHIFT
%token <strval>	IDL_RIGHTSHIFT
%token <strval> 	IDL_SCOPESEPARATOR

%token <ival>		IDL_INTEGER_LITERAL
%token <uval> IDL_INTEGER_ULITERAL
%token <dval>		IDL_FLOATING_PT_LITERAL

/*
 * These are production names:
 */
%type <dclval>	type_dcl const_dcl
%type <dclval>	array_declarator
%type <dclval>  exception_name
%type <cdclval> array_type constructed_type_spec enum_type op_type_spec
%type <cdclval> sequence_type_spec simple_type_spec struct_type switch_type_spec
%type <cdclval> template_type_spec type_spec union_type
%type <cdclval> fundamental_type type_arg type_or_parameter
%type <dclsval> opt_raises raises exception_list
%type <attexcpval> opt_attribute_get_raises attribute_get_raises
%type <attexcpval> opt_attribute_set_raises attribute_set_raises
%type <dclsval> opt_type_args type_args

%type <sval>    identifier
%type <sval>	interface_decl 
%type <sval>	scoped_name inheritance_spec
%type <slval>  	scoped_names at_least_one_scoped_name

%type <etval>	const_type integer_type char_type boolean_type
%type <etval>	floating_pt_type any_type signed_int string_type
%type <etval>	unsigned_int base_type_spec byte_type type_type 

%type <exval>	expression const_expr or_expr xor_expr and_expr
%type <exval>	add_expr mult_expr unary_expr primary_expr shift_expr
%type <exval>	literal positive_int_expr array_dim

%type <exlval> 	at_least_one_array_dim array_dims

%type <fdval> 	declarator simple_declarator complex_declarator
%type <dlval> 	declarators at_least_one_declarator

%type <ihval>	exception_header structure_header interfaceheader

%type <ulval> 	flag_header opt_attrflags opt_attrflag operation_head
%type <ulval>	direction service_interface_header service_service_header

%type <llval>	case_labels at_least_one_case_label
%type <lbval>	case_label
%type <mval>	element_spec

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
	| const_dcl
	{
		idlc()->setParseState(PS_ConstantDeclSeen);
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

        AstScope* 		pScope = idlc()->scopes()->topNonNull();
        AstModule* 		pModule = NULL;
        AstDeclaration*	pExists = NULL;

        if ( pScope )
        {
        	pModule = new AstModule(*$3, pScope);
			if( (pExists = pScope->lookupForAdd(pModule)) )
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
				pModule = (AstModule*)pExists;
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

		AstScope* 		pScope = idlc()->scopes()->topNonNull();
		AstInterface*	pForward = NULL;
		AstDeclaration*	pDecl = NULL;

        /*
		 * Make a new forward interface node and add it to its enclosing scope
		 */
		if ( pScope && $1 ) 
		{
			pForward = new AstInterface(*$1, NULL, pScope);
			
			pDecl = pScope->lookupByName(pForward->getScopedName());
			if ( pDecl ) 
			{
				if ( (pDecl != pForward) && 
					 (pDecl->getNodeType() == NT_interface) )
				{
					delete pForward;
				} else
				{
					idlc()->error()->error2(EIDL_REDEF_SCOPE, scopeAsDecl(pScope), pDecl);
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

		AstScope* 		pScope = idlc()->scopes()->topNonNull();
		AstInterface*	pInterface = NULL;
		AstInterface*	pForward = NULL;
		AstDeclaration*	pDecl = NULL;

        /*
		 * Make a new interface node and add it to its enclosing scope
		 */
		if ( pScope && $1 ) 
		{
			pInterface = new AstInterface(
                *$1->getName(),
                static_cast< AstInterface * >($1->getInherits()), pScope);
			if ( pInterface &&
				(pDecl = pScope->lookupByName(pInterface->getScopedName())) ) 
			{
				/*
				 * See if we're defining a forward declared interface.
				 */
				if (pDecl->getNodeType() == NT_interface) 
				{
					pForward = (AstInterface*)pDecl;
					if ( !pForward->isDefined() ) 
					{
						/*
						 * Check if redefining in same scope
						 */
						if ( pForward->getScope() != pScope ) 
						{
							if ( pForward->getScopedName() != pInterface->getScopedName() )
							{
								idlc()->error()->error3(EIDL_SCOPE_CONFLICT,
										 pInterface, pForward, scopeAsDecl(pScope));
							}
						}
                        else if ( !pInterface->isPublished()
                                  && pForward->isPublished() )
                        {
                            idlc()->error()->error0(EIDL_PUBLISHED_FORWARD);
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
		delete($1);
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
                ifc, rtl::OString("::com::sun::star::uno::XInterface"), false,
                rtl::OUString());
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

		$$ = new FeInheritanceHeader(NT_interface, $1, $2, 0);
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
		$$ = NULL;
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
    simple_declarator
	{
		idlc()->setParseState(PS_AttrCompleted);
        if (($1 & ~(AF_BOUND | AF_READONLY)) != AF_ATTRIBUTE) {
            idlc()->error()->flagError(EIDL_BAD_ATTRIBUTE_FLAGS, $1);
        }
        AstInterface * scope = static_cast< AstInterface * >(
            idlc()->scopes()->top());
        AstAttribute * attr = new AstAttribute(
            $1, $4->compose($2), $4->getName(), scope);
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
 			idlc()->error()->flagError(EIDL_DEFINED_ATTRIBUTEFLAG, $3);
 
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
	| IDL_REMOVEABLE
	{
		idlc()->setParseState(PS_RemoveableSeen);
		$$ = AF_REMOVEABLE;
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
        $$.get.documentation = 0;
        $$.get.exceptions = 0;
        $$.set.documentation = 0;
        $$.set.exceptions = 0;
    }
    ;

attribute_block_rest:
    opt_attribute_raises '}'
    | error '}'
    {
        yyerror("bad attribute raises block");
        yyerrok;
        $$.get.documentation = 0;
        $$.get.exceptions = 0;
        $$.set.documentation = 0;
        $$.set.exceptions = 0;
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
        $$.get.documentation = 0;
        $$.get.exceptions = 0;
        $$.set.documentation = 0;
        $$.set.exceptions = 0;
    }
    ;

opt_attribute_get_raises:
    attribute_get_raises
    | /* empty */ { $$.documentation = 0; $$.exceptions = 0; }
    ;

attribute_get_raises:
    IDL_GET raises ';'
    {
        $$.documentation = new rtl::OUString(
            rtl::OStringToOUString(
                idlc()->getDocumentation(), RTL_TEXTENCODING_UTF8));
        $$.exceptions = $2;
    }
    ;

opt_attribute_set_raises:
    attribute_set_raises
    | /* empty */ { $$.documentation = 0; $$.exceptions = 0; }
    ;

attribute_set_raises:
    IDL_SET
    {
        if (static_cast< AstAttribute * >(idlc()->scopes()->top())->
            isReadonly())
        {
            idlc()->error()->error0(EIDL_READONLY_ATTRIBUTE_SET_EXCEPTIONS);
        }
    }
    raises ';'
    {
        $$.documentation = new rtl::OUString(
            rtl::OStringToOUString(
                idlc()->getDocumentation(), RTL_TEXTENCODING_UTF8));
        $$.exceptions = $3;
    }
    ;

operation :
	operation_head
	op_type_spec
	{
		idlc()->setParseState(PS_OpTypeSeen);
	}
	identifier
	{
		idlc()->setParseState(PS_OpIDSeen);
       checkIdentifier($4);

		AstInterface * pScope = static_cast< AstInterface * >(
            idlc()->scopes()->top());
		AstOperation* 	pOp = NULL;

		/*
		 * Create a node representing an operation on an interface
		 * and add it to its enclosing scope
		 */
		if ( pScope && $2 )
		{
			AstType *pType = (AstType*)$2;
			if ( !pType || (pType->getNodeType() == NT_exception) )
			{
				// type ERROR 
			} else 
			{
				pOp = new AstOperation($1, pType, *$4, pScope);

                AstInterface::DoubleMemberDeclarations doubleMembers(
                    pScope->checkMemberClashes(pOp));
                if (doubleMembers.empty()) {
                    pScope->addMember(pOp);
                } else {
                    reportDoubleMemberDeclarations(doubleMembers);
                }
			}
		}
		delete $4;
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
		AstScope*		pScope = idlc()->scopes()->topNonNull();
		AstOperation* 	pOp = NULL;
		/*
		 * Add exceptions and context to the operation
		 */
		if ( pScope && pScope->getScopeNodeType() == NT_operation)
		{
			pOp = (AstOperation*)pScope;

			if ( pOp )
				pOp->setExceptions($12);
		}
        delete $12;
		/*
		 * Done with this operation. Pop its scope from the scopes stack
		 */
		idlc()->scopes()->pop();
	}
	;

operation_head :
	'['
	IDL_ONEWAY
	{
		idlc()->setParseState(PS_OpOnewaySeen);
	}
	']'
	{
		idlc()->setParseState(PS_OpHeadSeen);
		$$ = OP_ONEWAY;
	}
	| /* EMPTY */
	{
		$$ = OP_NONE;
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
		AstParameter* 	pParam = NULL;

		/*
		 * Create a node representing an argument to an operation
		 * Add it to the enclosing scope (the operation scope)
		 */
		if ( pScope && $5 && $8 )
		{
            AstType const * pType = $8->compose($5);
			if ( pType )
			{
                if (pScope->isConstructor() && $2 != DIR_IN) {
                    idlc()->error()->error0(EIDL_CONSTRUCTOR_PARAMETER_NOT_IN);
                }
                if (pScope->isVariadic()) {
                    idlc()->error()->error0(EIDL_REST_PARAMETER_NOT_LAST);
                }
                if ($7) {
                    AstDeclaration const * type = resolveTypedefs(pType);
                    if (type->getNodeType() != NT_predefined
                        || (static_cast< AstBaseType const * >(type)->
                            getExprType() != ET_any))
                    {
                        idlc()->error()->error0(EIDL_REST_PARAMETER_NOT_ANY);
                    }
                    if (pScope->isConstructor()) {
                        if (pScope->getIteratorBegin()
                            != pScope->getIteratorEnd())
                        {
                            idlc()->error()->error0(
                                EIDL_CONSTRUCTOR_REST_PARAMETER_NOT_FIRST);
                        }
                    } else {
                        idlc()->error()->error0(EIDL_METHOD_HAS_REST_PARAMETER);
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
        $$ = 0;
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
        if (decl == 0) {
            idlc()->error()->lookupError(*$1);
        } else if (!idlc()->error()->checkPublished(decl)) {
            decl = 0;
        } else if (decl->getNodeType() != NT_exception) {
            idlc()->error()->error1(EIDL_ILLEGAL_RAISES, decl);
            decl = 0;
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
            idlc()->error()->error0(EIDL_MIXED_INHERITANCE);
        } else {
            addInheritedInterface(
                ifc, *$4, $1,
                rtl::OStringToOUString(
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
	const_dcl
	{
		idlc()->setParseState(PS_ConstantDeclSeen);
	}
	';' {};

const_dcl : 
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

		AstScope* 		pScope = idlc()->scopes()->topNonNull();
		AstConstant*	pConstant = NULL;

		if ( $9 && pScope )
		{
			if ( !$9->coerce($3) )
			{
				idlc()->error()->coercionError($9, $3);
			} else
			{
				pConstant = new AstConstant($3, $9, *$5, pScope);
				pScope->addDeclaration(pConstant);
			}
		}
		delete $5;
	}
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

		AstScope* 		pScope = idlc()->scopes()->topNonNull();
		AstConstants*	pConstants = NULL;
		AstDeclaration*	pExists = NULL;

		if ( pScope )
		{
			pConstants = new AstConstants(*$3, pScope);
			if( (pExists = pScope->lookupForAdd(pConstants)) )
			{
				pExists->setInMainfile(idlc()->isInMainFile());
				delete(pConstants);
				pConstants = (AstConstants*)pExists;
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
		$$ = new AstExpression(EC_or, $1, $3);
	}
	;

xor_expr :
	and_expr
	| xor_expr '^' and_expr
	{
		$$ = new AstExpression(EC_xor, $1, $3);
	}
	;

and_expr :
	shift_expr
	| and_expr '&' shift_expr
	{
		$$ = new AstExpression(EC_and, $1, $3);
	}
	;

shift_expr : 
	add_expr
	| shift_expr IDL_LEFTSHIFT add_expr
	{
		$$ = new AstExpression(EC_left, $1, $3);
	}
	| shift_expr IDL_RIGHTSHIFT add_expr
	{
		$$ = new AstExpression(EC_right, $1, $3);
	}
	;

add_expr : 
	mult_expr
	| add_expr '+' mult_expr
	{
		$$ = new AstExpression(EC_add, $1, $3);
	}
	| add_expr '-' mult_expr
	{
		$$ = new AstExpression(EC_minus, $1, $3);
	}
	;

mult_expr : 
	unary_expr
	| mult_expr '*' unary_expr
	{
		$$ = new AstExpression(EC_mul, $1, $3);
	}
	| mult_expr '/' unary_expr
	{
		$$ = new AstExpression(EC_div, $1, $3);
	}
	| mult_expr '%' unary_expr
	{
		$$ = new AstExpression(EC_mod, $1, $3);
	}
	;

unary_expr : 
	primary_expr
	| '+' primary_expr
	{
		$$ = new AstExpression(EC_u_plus, $2, NULL);
	}
	| '-' primary_expr
	{
		$$ = new AstExpression(EC_u_minus, $2, NULL);
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
		$$ = new AstExpression((sal_Int32)1, ET_boolean);
	}
	| IDL_FALSE
	{
		$$ = new AstExpression((sal_Int32)0, ET_boolean);
	}
	;

positive_int_expr :
	const_expr
	{
		$1->evaluate(EK_const);
		if ( !$1->coerce(ET_ulong) )
		{
			idlc()->error()->coercionError($1, ET_ulong);
			delete $1;
			$$ = NULL;
		}
	}
	;

const_type :
	integer_type
	| char_type
	| byte_type
	| boolean_type
	| floating_pt_type
	| scoped_name
	{
		AstScope* 		pScope = idlc()->scopes()->topNonNull();
        AstDeclaration const * type = 0;
		
		/*
		 * If the constant's type is a scoped name, it must resolve
		 * to a scalar constant type
		 */
		if ( pScope && (type = pScope->lookupByName(*$1)) ) {
            if (!idlc()->error()->checkPublished(type))
            {
                type = 0;
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

		$$ = new FeInheritanceHeader(NT_exception, $3, $5, 0);
		delete $5;
	}
	;

exception_dcl : 
	exception_header
	{
		idlc()->setParseState(PS_ExceptHeaderSeen);

		AstScope* 		pScope = idlc()->scopes()->topNonNull();
		AstException*	pExcept = NULL;

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

		AstScope* 		pScope = idlc()->scopes()->topNonNull();
		AstAttribute* 	pAttr = NULL;
		FeDeclList*		pList = $4;
		FeDeclarator*	pDecl = NULL;
        AstType const * pType = NULL;

		if ( pScope->getScopeNodeType() == NT_singleton )
		{
			idlc()->error()->error0(EIDL_ILLEGAL_ADD);			
		} else
		{
			if ( ($1 & AF_ATTRIBUTE) == AF_ATTRIBUTE )
				idlc()->error()->flagError(EIDL_WRONGATTRIBUTEKEYWORD, AF_ATTRIBUTE);

			if ( ($1 & AF_PROPERTY) != AF_PROPERTY )
				idlc()->error()->flagError(EIDL_MISSINGATTRIBUTEKEYWORD, AF_PROPERTY);

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
						iter++;
						continue;
					}

					pType = pDecl->compose($2);				

					if ( !pType )
					{
						iter++;
						continue;
					}

					pAttr = new AstAttribute(NT_property, $1, pType, pDecl->getName(), pScope);

					pScope->addDeclaration(pAttr);
					iter++;
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
		
		AstScope* 			pScope = idlc()->scopes()->topNonNull();
		AstDeclaration* 	pDecl = NULL;
		AstInterfaceMember* pIMember = NULL;

		if ( pScope->getScopeNodeType() == NT_singleton )
		{
			idlc()->error()->error0(EIDL_ILLEGAL_ADD);			
		} else
		{
			/*
			 * Create a node representing a class member.
			 * Store it in the enclosing scope
	         */
			if ( pScope && $2 )
			{
				StringList::iterator iter = $2->begin();
				StringList::iterator end = $2->end();						

				while ( iter != end )
				{
					pDecl = pScope->lookupByName(*iter);
					if ( pDecl && (pDecl->getNodeType() == NT_interface) )
					{
                        /* we relax the strict published check and allow to add new
                         * interfaces if they are optional
                         */
                        bool bOptional = (($1 & AF_OPTIONAL) == AF_OPTIONAL);
                        if ( idlc()->error()->checkPublished(pDecl, bOptional) )
                        {
                            pIMember = new AstInterfaceMember(
                                $1, (AstInterface*)pDecl, *iter, pScope);
                            pScope->addDeclaration(pIMember);
                        }
					} else
					{
						idlc()->error()->
							lookupError(EIDL_INTERFACEMEMBER_LOOKUP, *iter, scopeAsDecl(pScope));
					}
					iter++;
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

		AstScope* 		  pScope = idlc()->scopes()->topNonNull();
		AstDeclaration*   pDecl = NULL;
		AstServiceMember* pSMember = NULL;

		/*
		 * Create a node representing a class member.
		 * Store it in the enclosing scope
         */
		if ( pScope && $2 )
		{
			StringList::iterator iter = $2->begin();
			StringList::iterator end = $2->end();						

			while ( iter != end )
			{
				pDecl = pScope->lookupByName(*iter);
				if ( pDecl && (pDecl->getNodeType() == NT_service) )
				{
					if ( pScope->getScopeNodeType() == NT_singleton && pScope->nMembers() > 0 )
						idlc()->error()->error0(EIDL_ILLEGAL_ADD);										
                    else if ( idlc()->error()->checkPublished(pDecl) )
                    {
                        pSMember = new AstServiceMember(
                            $1, (AstService*)pDecl, *iter, pScope);
                        pScope->addDeclaration(pSMember);
                    }
				} else
				{
					idlc()->error()->
						lookupError(EIDL_SERVICEMEMBER_LOOKUP, *iter, scopeAsDecl(pScope));
				}
				iter++;
			}
		}				
		delete $2;
	}
	| IDL_OBSERVES
	at_least_one_scoped_name
	';'
	{
		idlc()->setParseState(PS_ServiceMemberSeen);

		AstScope* 		pScope = idlc()->scopes()->topNonNull();
		AstDeclaration* pDecl = NULL;
		AstObserves* 	pObserves = NULL;

		if ( pScope->getScopeNodeType() == NT_singleton )
		{
			idlc()->error()->error0(EIDL_ILLEGAL_ADD);			
		} else
		{
			/*
			 * Create a node representing a class member.
			 * Store it in the enclosing scope
	         */
			if ( pScope && $2 )
			{
				StringList::iterator iter = $2->begin();
				StringList::iterator end = $2->end();						

				while ( iter != end )
				{
					pDecl = pScope->lookupByName(*iter);
					if ( pDecl && (pDecl->getNodeType() == NT_interface) )
					{
						pObserves = new AstObserves((AstInterface*)pDecl, *iter, pScope);
						pScope->addDeclaration(pObserves);
					} else
					{
						idlc()->error()->
							lookupError(EIDL_INTERFACEMEMBER_LOOKUP, *iter, scopeAsDecl(pScope));
					}
					iter++;
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

		AstScope* 		pScope = idlc()->scopes()->topNonNull();
		AstDeclaration*	pDecl = NULL;
		AstNeeds*	   	pNeeds = NULL;

		if ( pScope->getScopeNodeType() == NT_singleton )
		{
			idlc()->error()->error0(EIDL_ILLEGAL_ADD);			
		} else
		{
			/*
			 * Create a node representing a class member.
			 * Store it in the enclosing scope
	         */
			if ( pScope && $2 )
			{
				StringList::iterator iter = $2->begin();
				StringList::iterator end = $2->end();						

				while ( iter != end )
				{
					pDecl = pScope->lookupByName(*iter);
					if ( pDecl && (pDecl->getNodeType() == NT_service) )
					{
						pNeeds = new AstNeeds((AstService*)pDecl, *iter, pScope);
						pScope->addDeclaration(pNeeds);
					} else
					{
						idlc()->error()->
							lookupError(EIDL_SERVICEMEMBER_LOOKUP, *iter, scopeAsDecl(pScope));
					}
					iter++;
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
			idlc()->error()->flagError(EIDL_OPTIONALEXPECTED, $1);
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
			idlc()->error()->flagError(EIDL_OPTIONALEXPECTED, $1);
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

        AstScope* 	pScope = idlc()->scopes()->topNonNull();
        AstService*	pService = NULL;

        /*
         * Make a new service and add it to the enclosing scope
         */
        if (pScope != NULL)
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
        if (decl != 0 && resolveTypedefs(decl)->getNodeType() == NT_interface) {
            if (idlc()->error()->checkPublished(decl)) {
                idlc()->scopes()->top()->addDeclaration(decl);
            }
        } else {
            idlc()->error()->lookupError(
                EIDL_INTERFACEMEMBER_LOOKUP, *$2, scopeAsDecl(scope));
        }
        delete $2;
    }
    opt_service_body
    {
        AstService * s = static_cast< AstService * >(idlc()->scopes()->top());
        if (s != 0) {
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
        AstOperation * ctor = new AstOperation(OP_NONE, 0, *$1, scope);
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
            idlc()->error()->error0(EIDL_SIMILAR_CONSTRUCTORS);
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

		AstScope* 	pScope = idlc()->scopes()->topNonNull();
		AstService*	pService = NULL;
		
		/*
		 * Make a new service and add it to the enclosing scope
		 */
		if (pScope != NULL) 
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
        if (decl != 0 && resolveTypedefs(decl)->getNodeType() == NT_interface) {
            if (idlc()->error()->checkPublished(decl)) {
                idlc()->scopes()->top()->addDeclaration(decl);
            }
        } else {
            idlc()->error()->lookupError(
                EIDL_INTERFACEMEMBER_LOOKUP, *$2, scopeAsDecl(scope));
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
	| union_type {}
	| enum_type {}
	;

type_declarator :
	type_spec
	{
		idlc()->setParseState(PS_TypeSpecSeen);
        if ($1 != 0 && $1->getNodeType() == NT_instantiated_struct) {
            idlc()->error()->error0(EIDL_INSTANTIATED_STRUCT_TYPE_TYPEDEF);
        }
	}
	at_least_one_declarator
	{
		idlc()->setParseState(PS_DeclaratorsSeen);

		AstScope* 		pScope = idlc()->scopes()->topNonNull();
		AstTypeDef* 	pTypeDef = NULL;
		FeDeclList*		pList = $3;
		FeDeclarator*	pDecl = NULL;
        AstType const * pType = NULL;

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
					iter++;
					continue;
				}

				pType = pDecl->compose($1);

				if ( !pType )
				{
					iter++;
					continue;
				}

				pTypeDef = new AstTypeDef(pType, pDecl->getName(), pScope);

				pScope->addDeclaration(pTypeDef);
				iter++;
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
			FeDeclList* pList = new FeDeclList();
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
			FeDeclList* pList = new FeDeclList();
			pList->push_back($4);
			$$ = pList;
		}
	}
	| /* EMPTY */
	{
		$$ = NULL;
	}
	;

declarator : 
	simple_declarator
	| complex_declarator
	;

simple_declarator :
	identifier
	{
        // For historic reasons, the struct com.sun.star.uno.Uik contains
        // members with illegal names (of the form "m_DataN"); avoid useless
        // warnings about them:
        AstScope * scope = idlc()->scopes()->top();
        if (scope == 0 || scope->getScopeNodeType() != NT_struct
            || (scopeAsDecl(scope)->getScopedName()
                != "com::sun::star::uno::Uik"))
        {
            checkIdentifier($1);
        }

        $$ = new FeDeclarator(*$1, FeDeclarator::FD_simple, NULL);
        delete $1;
	}
	;

complex_declarator :
	array_declarator
	{
		$$ = new FeDeclarator($1->getLocalName(), FeDeclarator::FD_complex, $1);
	}
	;

array_declarator :
	identifier
	{
        idlc()->setParseState(PS_ArrayIDSeen);
        checkIdentifier($1);
	}
	at_least_one_array_dim
	{
		idlc()->setParseState(PS_ArrayCompleted);
		$$ = new AstArray(*$1, NULL, *$3, idlc()->scopes()->bottom());
		delete $1;
	}
	;

at_least_one_array_dim :
	array_dim array_dims
	{
		if( $2 )
		{
			$2->push_front($1);
			$$ = $2;
		} else
		{
			ExprList* pList = new ExprList();
			pList->push_back($1);
			$$ = pList;
		}
	}
	;

array_dims : 
	array_dims array_dim
	{
		if( $1 )
		{
			$1->push_back($2);
			$$ = $1;
		} else
		{
			ExprList* pList = new ExprList();
			pList->push_back($2);
			$$ = pList;
		}
	}
	| /* EMPTY */
	{
		$$ = NULL;
	}
    ;

array_dim :
	'['
	{
		idlc()->setParseState(PS_DimSqSeen);
	}
	positive_int_expr
	{
		idlc()->setParseState(PS_DimExprSeen);
	}
	']'
	{
		idlc()->setParseState(PS_DimQsSeen);
		/*
		 * Array dimensions are expressions which must be coerced to
		 * positive integers
		 */
		if ( !$3 || !$3->coerce(ET_uhyper) ) 
		{
			idlc()->error()->coercionError($3, ET_uhyper);
			$$ = NULL;
		} else
			$$ = $3;
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
			StringList* pNames = new StringList();
			pNames->push_back(*$1);
			$$ = pNames;
		}
		delete($1);
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
			StringList* pNames = new StringList();
			pNames->push_back(*$4);
			$$ = pNames;
		}
		delete($4);
	}
	| /* EMPTY */
	{
		$$ = NULL;
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
        *$1 += ::rtl::OString("::");
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
	| template_type_spec
    ;

opt_type_args:
    '<' type_args '>' { $$ = $2; }
    | /* empty */ { $$ = 0; }
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
        if ($1 != 0 && static_cast< AstType const * >($1)->isUnsigned()) {
            idlc()->error()->error0(EIDL_UNSIGNED_TYPE_ARGUMENT);
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

template_type_spec : 
	sequence_type_spec
	| array_type
	;

constructed_type_spec : 
	struct_type
	| union_type
	| enum_type
	;

array_type :
	simple_type_spec
	{
		idlc()->setParseState(PS_ArrayTypeSeen);
	}
	at_least_one_array_dim
	{
		idlc()->setParseState(PS_ArrayCompleted);

		AstScope* pScope = idlc()->scopes()->bottom();
		AstDeclaration* pDecl = NULL;
		AstDeclaration* pArray = NULL;

		if ( $1 )
		{
			pArray = new AstArray((AstType*)$1, *$3, idlc()->scopes()->bottom());
			if ( pScope )
			{
				pDecl = pScope->addDeclaration(pArray);				
				if ( pArray != pDecl )
				{
					// if array type already defined then use it
					delete pArray;
					pArray = pDecl;					
				}
			}
		}
		$$ = pArray; 
	}
	;

sequence_type_spec :
	IDL_SEQUENCE
	{
		idlc()->setParseState(PS_SequenceSeen);
		/*
		 * Push a sequence marker on scopes stack
		 */
		idlc()->scopes()->push(NULL);
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
		if (idlc()->scopes()->top() == NULL)
			idlc()->scopes()->pop();
		/*
		 * Create a node representing a sequence
		 */
		AstScope* pScope = idlc()->scopes()->bottom();
		AstDeclaration* pDecl = NULL;
		AstDeclaration* pSeq = NULL;
		
		if ( $5 )
		{
			AstType *pType = (AstType*)$5;
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
        $$ = 0;
	}
	;

struct_type :
	structure_header
	{
		idlc()->setParseState(PS_StructHeaderSeen);

		AstScope* 	pScope = idlc()->scopes()->topNonNull();
		AstStruct*	pStruct = NULL;

		if ( pScope )
		{
			AstStruct* pBase= static_cast< AstStruct* >($1->getInherits());
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
        if ($5 != 0 && $6 != 0) {
            idlc()->error()->error0(EIDL_STRUCT_TYPE_TEMPLATE_WITH_BASE);
        }

        $$ = new FeInheritanceHeader(NT_struct, $3, $6, $5);
        delete $5;
        delete $6;
	}
	;

opt_type_params:
    '<' type_params '>' { $$ = $2; }
    | /* empty */ { $$ = 0; }
    ;

type_params:
    identifier
    {
        $$ = new std::vector< rtl::OString >;
        $$->push_back(*$1);
        delete $1;
    }
    | type_params ',' identifier
    {
        if (std::find($1->begin(), $1->end(), *$3) != $1->end()) {
            idlc()->error()->error0(EIDL_IDENTICAL_TYPE_PARAMETERS);
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

		AstScope* 		pScope = idlc()->scopes()->topNonNull();
		AstMember*		pMember = NULL;
		FeDeclList*		pList = $3;
		FeDeclarator*	pDecl = NULL;
        AstType const * pType = NULL;

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
					iter++;
					continue;
				}

				pType = pDecl->compose($1);				

				if ( !pType )
				{
					iter++;
					continue;
				}

				pMember = new AstMember(pType, pDecl->getName(), pScope);

				if ( !pDecl->checkType($1) )
				{
					// WARNING	
				}

				pScope->addDeclaration(pMember);
				iter++;
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
        AstDeclaration const * decl = 0;
        AstStruct * scope = static_cast< AstStruct * >(idlc()->scopes()->top());
        if (scope != 0 && $2 == 0) {
            decl = scope->findTypeParameter(*$1);
        }
        if (decl != 0) {
            delete $1;
            delete $2;
        } else {
            decl = createNamedType($1, $2);
            if (scope != 0 && includes(decl, scopeAsDecl(scope))) {
                idlc()->error()->error1(
                    EIDL_RECURSIVE_TYPE, scopeAsDecl(scope));
                decl = 0;
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

		AstScope* 		pScope = idlc()->scopes()->topNonNull();
		AstEnum*		pEnum = NULL;

		/*
		 * Create a node representing an enum and add it to its
		 * enclosing scope
		 */
		if (pScope != NULL) 
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
		if (idlc()->scopes()->top() == NULL)
			$$ = NULL;
		else 
		{
			$$ = (AstEnum*)idlc()->scopes()->topNonNull();
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

		AstScope* 		pScope = idlc()->scopes()->topNonNull();
		AstEnum*		pEnum = NULL;
		AstConstant* 	pEnumVal = NULL;

		if ( pScope && pScope->getScopeNodeType() == NT_enum) 
		{
			pEnum = (AstEnum*)pScope;
			if (pEnum && $1)
			{
				AstExpression* pExpr = new AstExpression(pEnum->getEnumValueCount());
				pEnumVal = new AstConstant(ET_long , NT_enum_val, 
										   pExpr, *$1, pScope);
			}
			if ( pEnum->checkValue(pEnumVal->getConstValue()) )
				idlc()->error()->error1(EIDL_EVAL_ERROR, pEnum);				

			pScope->addDeclaration(pEnumVal);
		}
		delete $1;
	}
	| identifier
	'='
	const_expr
	{
        checkIdentifier($1);

		AstScope* 		pScope = idlc()->scopes()->topNonNull();
		AstEnum*		pEnum = NULL;
		AstConstant* 	pEnumVal = NULL;

		if ( $3 && pScope && pScope->getScopeNodeType() == NT_enum) 
		{
			$3->evaluate(EK_const);
			if ( $3->coerce(ET_long) )
			{
				pEnum = (AstEnum*)pScope;
				if (pEnum)
				{
					pEnumVal = new AstConstant(ET_long , NT_enum_val, 
											   $3, *$1, pScope);
				}
				if ( pEnum->checkValue(pEnumVal->getConstValue()) )
					idlc()->error()->error1(EIDL_EVAL_ERROR, pEnum);				

				pScope->addDeclaration(pEnumVal);
			} else
			{
				idlc()->error()->coercionError($3, ET_long);
				delete $3;
			}
		}
		delete $1;
	}
	;

union_type :
	IDL_UNION
	{
		idlc()->setParseState(PS_UnionSeen);
	}
	identifier
	{
        idlc()->setParseState(PS_UnionIDSeen);
        checkIdentifier($3);
	}
	IDL_SWITCH
	{
		idlc()->setParseState(PS_SwitchSeen);
	}
	'('
	{
		idlc()->setParseState(PS_SwitchOpenParSeen);
	}
	switch_type_spec
	{
		idlc()->setParseState(PS_SwitchTypeSeen);
	}
	')'
	{
		idlc()->setParseState(PS_SwitchCloseParSeen);

		AstScope* 		pScope = idlc()->scopes()->topNonNull();
		AstUnion*		pUnion = NULL;

		/*
		 * Create a node representing a union. Add it to its enclosing
		 * scope
		 */
		if ( $9 && pScope ) 
		{
			AstType* pType = (AstType*)$9;
			if ( !pType) 
			{
				idlc()->error()->noTypeError($9);
			} else 
			{
				pUnion = new AstUnion(*$3, pType, pScope);
				pScope->addDeclaration(pUnion);
			}
		}
		delete $3;
		/*
		 * Push the scope of the union on the scopes stack
		 */
		idlc()->scopes()->push(pUnion);
	}
	'{'
	{
		idlc()->setParseState(PS_UnionSqSeen);
	}
	at_least_one_case_branch
	{
		idlc()->setParseState(PS_UnionBodySeen);
	}
	'}'
	{
		idlc()->setParseState(PS_UnionQsSeen);
		/* this union is finished, pop its scope from the stack */ 
		idlc()->scopes()->pop();
	}
	;

switch_type_spec :
	integer_type
	{	
		$$ = idlc()->scopes()->bottom()->lookupPrimitiveType($1);
	}
	| char_type
	{
		$$ = idlc()->scopes()->bottom()->lookupPrimitiveType($1);
	}
	| boolean_type
	{
		$$ = idlc()->scopes()->bottom()->lookupPrimitiveType($1);
	}
	| enum_type
	| scoped_name
	{
		AstScope* 		pScope = idlc()->scopes()->topNonNull();
		AstBaseType*	pBaseType = NULL;
        AstDeclaration const * pDecl = NULL;
		AstTypeDef*		pTypeDef = NULL;
		sal_Bool		bFound = sal_False;
		/*
		 * If the constant's type is a scoped name, it must resolve
		 * to a scalar constant type
		 */
		if ( pScope && (pDecl = pScope->lookupByName(*$1)) ) 
		{
			/*
			 * Look through typedefs
			 */
			while ( !bFound ) 
			{
				switch (pDecl->getNodeType()) 
				{
					case NT_enum:
						$$ = pDecl;
						bFound = sal_True;
						break;
					case NT_predefined:
						pBaseType = (AstBaseType*)pDecl;
						if ( pBaseType ) 
						{
							switch (pBaseType->getExprType()) 
							{
								case ET_short:
								case ET_ushort:
								case ET_long:
								case ET_ulong:
								case ET_hyper:
								case ET_uhyper:
								case ET_char:
								case ET_byte:
								case ET_boolean:
									$$ = pBaseType;
									bFound = sal_True;
									break;
								default:
									$$ = NULL;
									bFound = sal_True;
									break;
							}
						}
						break;
					case NT_typedef:
						pTypeDef = (AstTypeDef*)pDecl;
						if ( pTypeDef )
							pDecl = pTypeDef->getBaseType();
						break;
					default:
						$$ = NULL;
						bFound = sal_True;
		               break;		
				}
			}
		} else
			$$ = NULL;

		if ($$ == NULL)
			idlc()->error()->lookupError(*$1);
	}
	;

at_least_one_case_branch : case_branch case_branches ;

case_branches : 
	case_branches case_branch
	| /* EMPTY */
	;

case_branch :
	at_least_one_case_label
	{
		idlc()->setParseState(PS_UnionLabelSeen);
	}
	element_spec
	{
		idlc()->setParseState(PS_UnionElemSeen);

		AstScope* 		pScope = idlc()->scopes()->topNonNull();
		AstUnionLabel*	pLabel = NULL;
		AstUnionBranch* pBranch = NULL;
		AstMember*		pMember = $3;

		/*
		 * Create several nodes representing branches of a union.
		 * Add them to the enclosing scope (the union scope)
		 */
		if ( pScope && $1 && $3 ) 
		{
			LabelList::iterator iter = $1->begin();
			LabelList::iterator end = $1->end();
			for (;iter != end; iter++) 
			{
				pLabel = *iter;
				if ( !pLabel )
				{
					iter++;
					continue;
				}
				pBranch = new AstUnionBranch(pLabel, pMember->getType(),
                                        	 pMember->getLocalName(), pScope);
				pScope->addDeclaration(pBranch);
			}
		}
		if ( $1 ) delete($1);
	}
	;

at_least_one_case_label :
	case_label case_labels
	{
		if ( $2 )
		{
			$2->push_front($1);
		 	$$ = $2;
		} else
		{
			LabelList* pLabels = new LabelList();
			pLabels->push_back($1);
			$$ = pLabels;
		}
	}
	;

case_labels : 
	case_labels case_label
	{
		if ( $1 )
		{
			$1->push_back($2);
		 	$$ = $1;
		} else
		{
			LabelList* pLabels = new LabelList();
			pLabels->push_back($2);
			$$ = pLabels;
		}
	}
	| /* EMPTY */
	{
		$$ = NULL;
	}
	;

case_label : 
	IDL_DEFAULT
	{
		idlc()->setParseState(PS_DefaultSeen);
	}
	':'
	{
		idlc()->setParseState(PS_LabelColonSeen);
		$$ = new AstUnionLabel(UL_default, NULL);
	}
	| IDL_CASE
	{
		idlc()->setParseState(PS_CaseSeen);
	}
	const_expr
	{
		idlc()->setParseState(PS_LabelExprSeen);
	}
	':'
	{
		idlc()->setParseState(PS_LabelColonSeen);
		$$ = new AstUnionLabel(UL_label, $3);
	}
	;

element_spec :
	type_spec
	{
		idlc()->setParseState(PS_UnionElemTypeSeen);
	}
	declarator
	{
		idlc()->setParseState(PS_UnionElemDeclSeen);
	}
	';'
	{
		idlc()->setParseState(PS_UnionElemCompleted);

		AstScope* pScope = idlc()->scopes()->topNonNull();
		/*
		 * Check for illegal recursive use of type
		 */
//		if ( $1 && AST_illegal_recursive_type($1))
//			idlc()->error()->error1(EIDL_RECURSIVE_TYPE, $1);
		/*
		 * Create a field in a union branch
		 */
		if ( $1 && $3 )
		{
            AstType const * pType = $3->compose($1);
			if ( !pType )
				$$ = NULL;
			else
				$$ = new AstMember(pType, $3->getName(), pScope);
		} else
			$$ = NULL;
		
		if ( $3 ) delete $3;
	}
	| error
	';'
	{
		$$ = NULL;
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
	idlc()->error()->syntaxError(idlc()->getParseState(), idlc()->getLineNumber(), errmsg);
	idlc()->setParseState(PS_NoState);
}
