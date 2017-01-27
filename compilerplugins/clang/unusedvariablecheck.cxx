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

#include <config_global.h>

// If there is support for warn_unused attribute even in STL classes, then there's
// no point in having this check enabled, otherwise keep it at least for STL
// (LO classes won't get duplicated warnings, as the attribute is different).
#if !HAVE_GCC_ATTRIBUTE_WARN_UNUSED_STL

#include "compat.hxx"
#include "check.hxx"
#include "unusedvariablecheck.hxx"

#include <clang/AST/Attr.h>

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
    : Plugin( data )
    {
    }

void UnusedVariableCheck::run()
    {
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }

bool BaseCheckNotSomethingInterestingSubclass(
    const CXXRecordDecl *BaseDefinition
#if CLANG_VERSION < 30800
    , void *
#endif
    )
{
    if (BaseDefinition) {
        auto tc = loplugin::TypeCheck(BaseDefinition);
        if (tc.Class("Dialog").GlobalNamespace() || tc.Class("SfxPoolItem").GlobalNamespace()) {
            return false;
        }
    }
    return true;
}

bool isDerivedFromSomethingInteresting(const CXXRecordDecl *decl) {
    if (!decl)
        return false;
    auto tc = loplugin::TypeCheck(decl);
    if (tc.Class("Dialog"))
        return true;
    if (tc.Class("SfxPoolItem"))
        return true;
    if (!decl->hasDefinition()) {
        return false;
    }
    if (// not sure what hasAnyDependentBases() does,
        // but it avoids classes we don't want, e.g. WeakAggComponentImplHelper1
        !decl->hasAnyDependentBases() &&
        !compat::forallBases(*decl, BaseCheckNotSomethingInterestingSubclass, nullptr, true)) {
        return true;
    }
    return false;
}

bool UnusedVariableCheck::VisitVarDecl( const VarDecl* var )
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
            auto tc = loplugin::TypeCheck(type);
            // Check some common non-LO types.
            if( tc.Class("string").Namespace("std").GlobalNamespace()
                || tc.Class("basic_string").Namespace("std").GlobalNamespace()
                || tc.Class("list").Namespace("std").GlobalNamespace()
                || tc.Class("list").Namespace("__debug").Namespace("std").GlobalNamespace()
                || tc.Class("vector").Namespace("std").GlobalNamespace()
                || tc.Class("vector" ).Namespace("__debug").Namespace("std").GlobalNamespace())
                warn_unused = true;
            if (!warn_unused && isDerivedFromSomethingInteresting(type))
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
                report( DiagnosticsEngine::Warning, "unused parameter %0",
                    var->getLocation()) << var->getDeclName();
                }
            else
                report( DiagnosticsEngine::Warning, "unused variable %0",
                    var->getLocation()) << var->getDeclName();
            }
        }
    return true;
    }

static Plugin::Registration< UnusedVariableCheck > X( "unusedvariablecheck" );

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
