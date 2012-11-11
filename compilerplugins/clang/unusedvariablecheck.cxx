/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#include "unusedvariablecheck.hxx"

#include <clang/Basic/SourceManager.h>

namespace loplugin
{

/*
This is a compile check.

Check for unused classes where the compiler cannot decide (e.g. because of
non-trivial or extern ctors) if a variable is unused if only its ctor/dtor
are called and nothing else. For example std::vector is a class where
the ctor may call further functions, but an unused std::string variable
does nothing. On the other hand, std::auto_ptr instances are used
for their dtors and so are not unused even if not otherwise accessed.

Classes which are safe to be warned about need to be marked using
SAL_WARN_UNUSED (see e.g. OUString). For external classes such as std::vector
that cannot be edited there is a manual list below.
*/

UnusedVariableCheck::UnusedVariableCheck( ASTContext& context )
    : Plugin( context )
    {
    }

void UnusedVariableCheck::run()
    {
    TraverseDecl( context.getTranslationUnitDecl());
    }

bool UnusedVariableCheck::VisitVarDecl( VarDecl* var )
    {
    if( ignoreLocation( var ))
        return true;
    if( var->isReferenced() || var->isUsed())
        return true;
    if( var->isDefinedOutsideFunctionOrMethod())
        return true;
    if( CXXRecordDecl* type = var->getType()->getAsCXXRecordDecl())
        {
        bool warn_unused = false;
        if( type->hasAttrs())
            {
            // Clang currently has no support for custom attributes, but
            // the annotate attribute comes close, so check for __attribute__((annotate("lo_warn_unused")))
            for( specific_attr_iterator<AnnotateAttr> i = type->specific_attr_begin<AnnotateAttr>(),
                    e = type->specific_attr_end<AnnotateAttr>();
                 i != e;
                 ++i )
                {
                if( (*i)->getAnnotation() == "lo_warn_unused" )
                    {
                    warn_unused = true;
                    break;
                    }
                }
            }
        if( !warn_unused )
            {
            string n = type->getQualifiedNameAsString();
            // Check some common non-LO types.
            if( n == "std::string" || n == "std::basic_string"
                || n == "std::list" || n == "std::__debug::list"
                || n == "std::vector" || n == "std::__debug::vector" )
                warn_unused = true;
            }
        if( warn_unused )
            {
            if( const ParmVarDecl* param = dyn_cast< ParmVarDecl >( var ))
                {
                if( !param->getDeclName())
                    return true; // unnamed parameter -> unused
                // If this declaration does not have a body, then the parameter is indeed not used,
                // so ignore.
                if( const FunctionDecl* func = dyn_cast_or_null< FunctionDecl >( param->getParentFunctionOrMethod()))
                    if( !func->doesThisDeclarationHaveABody())
                        return true;
                report( DiagnosticsEngine::Warning, "unused parameter %0 [loplugin]",
                    var->getLocation()) << var->getDeclName();
                }
            else
                report( DiagnosticsEngine::Warning, "unused variable %0 [loplugin]",
                    var->getLocation()) << var->getDeclName();
            }
        }
    return true;
    }

} // namespace
