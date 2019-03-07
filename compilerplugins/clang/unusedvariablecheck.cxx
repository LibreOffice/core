/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include <config_global.h>

#include "compat.hxx"
#include "check.hxx"
#include "unusedvariablecheck.hxx"

namespace loplugin
{

/*
This is a compile check.

Check for unused classes where the compiler cannot decide (e.g. because of
non-trivial or extern ctors) if a variable is unused if only its ctor/dtor
are called and nothing else. For example std::vector is a class where
the ctor may call further functions, but an unused std::string variable
does nothing. On the other hand, std::lock_guard instances are used
for their dtors and so are not unused even if not otherwise accessed.

Classes which are safe to be warned about need to be marked using
SAL_WARN_UNUSED (see e.g. OUString). For external classes such as std::vector
that cannot be edited there is a manual list below.
*/

UnusedVariableCheck::UnusedVariableCheck( const InstantiationData& data )
    : FilteringPlugin( data )
    {
    }

void UnusedVariableCheck::run()
    {
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }

bool UnusedVariableCheck::VisitVarDecl( const VarDecl* var )
    {
    if( ignoreLocation( var ))
        return true;
    if( var->isReferenced() || var->isUsed())
        return true;
    if( var->isDefinedOutsideFunctionOrMethod())
        return true;
    if( loplugin::isExtraWarnUnusedType(var->getType()))
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
                report( DiagnosticsEngine::Warning, "unused parameter %0",
                    var->getLocation()) << var->getDeclName();
                }
            else
                report( DiagnosticsEngine::Warning, "unused variable %0",
                    var->getLocation()) << var->getDeclName();
        }
    return true;
    }

static Plugin::Registration< UnusedVariableCheck > unusedvariablecheck( "unusedvariablecheck" );

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
