/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: errorhandler.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:03:42 $
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
#ifndef _IDLC_ERRORHANDLER_HXX_
#define _IDLC_ERRORHANDLER_HXX_

#ifndef _IDLC_ASTDECLARATION_HXX_
#include <idlc/astdeclaration.hxx>
#endif
#ifndef _IDLC_ASTEXPRESSION_HXX_
#include <idlc/astexpression.hxx>
#endif
#ifndef _IDLC_ASTUNION_HXX_
#include <idlc/astunion.hxx>
#endif
#ifndef _IDLC_ASTENUM_HXX_
#include <idlc/astenum.hxx>
#endif

enum ErrorCode
{
    EIDL_NONE,              // No error
    EIDL_SYNTAX_ERROR,      // Syntax error in IDL input
                            // More details will be gleaned from examining
                            // the parse state
    EIDL_REDEF,             // Redefinition
    EIDL_REDEF_SCOPE,       // Redefinition inside defining scope
    EIDL_DEF_USE,           // Definition after use
    EIDL_MULTIPLE_BRANCH,   // More than one union branch with this label
    EIDL_COERCION_FAILURE,  // Coercion failure
    EIDL_SCOPE_CONFLICT,    // Between fwd declare and full declare
    EIDL_ONEWAY_CONFLICT,   // Between op decl and argument direction
    EIDL_DISC_TYPE,         // Illegal discriminator type in union
    EIDL_LABEL_TYPE,        // Mismatch with discriminator type in union
    EIDL_ILLEGAL_ADD,       // Illegal add action
    EIDL_ILLEGAL_USE,       // Illegal type used in expression
    EIDL_ILLEGAL_RAISES,    // Error in "raises" clause
    EIDL_CANT_INHERIT,      // Cannot inherit from non-interface
    EIDL_LOOKUP_ERROR,      // Identifier not found
    EIDL_INHERIT_FWD_ERROR, // Cannot inherit from fwd decl interface
    EIDL_CONSTANT_EXPECTED, // We got something else..
    EIDL_NAME_CASE_ERROR,   // Spelling differences found
    EIDL_ENUM_VAL_EXPECTED, // Expected an enumerator
    EIDL_ENUM_VAL_NOT_FOUND,    // Didnt find an enumerator with that name
    EIDL_EVAL_ERROR,        // Error in evaluating expression
    EIDL_AMBIGUOUS,         // Ambiguous name definition
    EIDL_DECL_NOT_DEFINED,  // Forward declared but never defined
    EIDL_FWD_DECL_LOOKUP,   // Tried to lookup in fwd declared intf
    EIDL_RECURSIVE_TYPE,    // Illegal recursive use of type
    EIDL_NONVOID_ONEWAY,    // Non-void return type in oneway operation
    EIDL_NOT_A_TYPE,        // Not a type
    EIDL_TYPE_NOT_VALID,    // Type is not valid in this context
    EIDL_INTERFACEMEMBER_LOOKUP,    // interface is not defined or a fwd declaration not exists
    EIDL_SERVICEMEMBER_LOOKUP,
    EIDL_TYPE_IDENT_CONFLICT,   // type and identifier has equal names
    EIDL_ONEWAY_RAISE_CONFLICT, // oneway function raised excpetion conflict
    EIDL_WRONGATTRIBUTEFLAG,
    EIDL_DEFINED_ATTRIBUTEFLAG,
    EIDL_WRONGATTRIBUTEKEYWORD,
    EIDL_MISSINGATTRIBUTEKEYWORD,
    EIDL_BAD_ATTRIBUTE_FLAGS,
    EIDL_OPTIONALEXPECTED,
    EIDL_MIXED_INHERITANCE,
    EIDL_DOUBLE_INHERITANCE,
    EIDL_DOUBLE_MEMBER,
    EIDL_CONSTRUCTOR_PARAMETER_NOT_IN,
    EIDL_CONSTRUCTOR_REST_PARAMETER_NOT_FIRST,
    EIDL_REST_PARAMETER_NOT_LAST,
    EIDL_REST_PARAMETER_NOT_ANY,
    EIDL_METHOD_HAS_REST_PARAMETER,
    EIDL_READONLY_ATTRIBUTE_SET_EXCEPTIONS,
    EIDL_UNSIGNED_TYPE_ARGUMENT,
    EIDL_WRONG_NUMBER_OF_TYPE_ARGUMENTS,
    EIDL_INSTANTIATED_STRUCT_TYPE_TYPEDEF,
    EIDL_IDENTICAL_TYPE_PARAMETERS,
    EIDL_STRUCT_TYPE_TEMPLATE_WITH_BASE,
    EIDL_PUBLISHED_FORWARD,
    EIDL_PUBLISHED_USES_UNPUBLISHED
};

enum WarningCode
{
    WIDL_EXPID_CONFLICT,        // exception id conflict
    WIDL_REQID_CONFLICT,        // request id conflict
    WIDL_INHERIT_IDCONFLICT,    // request id conflict inheritance tree
    WIDL_TYPE_IDENT_CONFLICT,   // type and identifier has equal names
    WIDL_WRONG_NAMING_CONV      // type or identifier doesn't fulfill the UNO naming convention
};

class ErrorHandler
{
public:
    // Report errors with varying numbers of arguments
    void    error0(ErrorCode e);
    void    error1(ErrorCode e, AstDeclaration const * d);
    void    error2(
        ErrorCode e, AstDeclaration const * d1, AstDeclaration const * d2);
    void    error3(ErrorCode e, AstDeclaration* d1, AstDeclaration* d2, AstDeclaration* d3);

    // Warning
    void    warning0(WarningCode e, const sal_Char* warningmsg);
    void    warning1(WarningCode e, AstDeclaration* d);
    void    warning2(WarningCode e, AstDeclaration* d1, AstDeclaration* d2);

    // Report a syntax error in IDL input
    void    syntaxError(ParseState state, sal_Int32 lineNumber, const sal_Char* errmsg);

    // Report a name being used with different spellings
    void    nameCaseError(sal_Char *n, sal_Char *t);

    // Report an unsuccesful coercion attempt
    void    coercionError(AstExpression *pExpr, ExprType et);

    // Report a failed name lookup attempt
    void    lookupError(const ::rtl::OString& n);
    // Report a failed name lookup attempt
    void    lookupError(ErrorCode e, const ::rtl::OString& n, AstDeclaration* pScope);


    // Report a type error
    void    noTypeError(AstDeclaration const * pDecl);

    void    inheritanceError(NodeType nodeType, const ::rtl::OString* name, AstDeclaration* pDecl);

    void    flagError(ErrorCode e, sal_uInt32 flag);

    void    forwardLookupError(AstDeclaration* pForward, const ::rtl::OString& name);

    void    constantExpected(AstDeclaration* pDecl, const ::rtl::OString& name);

    void    evalError(AstExpression* pExpr);

    // Report a situation where an enumerator was expected but we got
    // something else instead. This occurs when a union with an enum
    // discriminator is being parsed and one of the branch labels is
    // not an enumerator in that enum
    void    enumValExpected(AstUnion* pUnion, AstUnionLabel *pLabel);

    // Report a failed enumerator lookup in an enum
    void    enumValLookupFailure(AstUnion* pUnion, AstEnum* pEnum, const ::rtl::OString& name);

    bool checkPublished(AstDeclaration const * decl);
};

#endif // _IDLC_ERRORHANDLER_HXX_

