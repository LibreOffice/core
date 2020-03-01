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

#include "plugin.hxx"
#include "check.hxx"
#include "compat.hxx"
#include <fstream>
#include <unordered_set>

namespace loplugin
{
/*
Check that we are using exceptionToString when printing exceptions inside SAL_WARN, so that we
get nicely formatted exception details in our logs.
*/

class LogExceptionNicely : public loplugin::FilteringPlugin<LogExceptionNicely>
{
    std::unordered_set<SourceLocation> m_visited;

public:
    LogExceptionNicely(const InstantiationData& data)
        : FilteringPlugin(data)
    {
    }

    bool preRun()
    {
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);
        // these are below tools in the module hierarchy, so we can't use the pretty printing
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/cppuhelper/"))
            return false;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/ucbhelper/"))
            return false;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/binaryurp/"))
            return false;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/comphelper/"))
            return false;
        // can't do that here, don't have an Any
        if (loplugin::hasPathnamePrefix(fn, SRCDIR
                                        "/connectivity/source/drivers/hsqldb/HStorageMap.cxx"))
            return false;
        return true;
    }

    void run()
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    static bool BaseCheckNotExceptionSubclass(const CXXRecordDecl* BaseDefinition)
    {
        if (!BaseDefinition)
            return true;
        auto tc = loplugin::TypeCheck(BaseDefinition);
        if (tc.Class("Exception")
                .Namespace("uno")
                .Namespace("star")
                .Namespace("sun")
                .Namespace("com")
                .GlobalNamespace())
            return false;
        return true;
    }

    bool isDerivedFromException(const CXXRecordDecl* decl)
    {
        if (!decl || !decl->hasDefinition())
            return false;
        auto tc = loplugin::TypeCheck(decl);
        if (tc.Class("Exception")
                .Namespace("uno")
                .Namespace("star")
                .Namespace("sun")
                .Namespace("com")
                .GlobalNamespace())
            return true;
        if ( // not sure what hasAnyDependentBases() does,
            // but it avoids classes we don't want, e.g. WeakAggComponentImplHelper1
            !decl->hasAnyDependentBases() && !decl->forallBases(BaseCheckNotExceptionSubclass))
        {
            return true;
        }
        return false;
    }

    bool VisitCXXOperatorCallExpr(const CXXOperatorCallExpr* operatorCallExpr)
    {
        if (ignoreLocation(operatorCallExpr))
            return true;

        StringRef fn = getFilenameOfLocation(
            compiler.getSourceManager().getExpansionLoc(compat::getBeginLoc(operatorCallExpr)));
        // these are below tools in the module hierarchy, so we can't use the pretty printing
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/include/comphelper/"))
            return true;

        if (operatorCallExpr->getOperator() != OO_LessLess)
            return true;
        auto expr = operatorCallExpr->getArg(1)->IgnoreImplicit();
        if (auto declRefExpr = dyn_cast<DeclRefExpr>(expr))
            if (auto varDecl = dyn_cast<VarDecl>(declRefExpr->getDecl()))
            {
                const clang::Type* type = varDecl->getType()->getUnqualifiedDesugaredType();
                const CXXRecordDecl* cxxRecordDecl = type->getAsCXXRecordDecl();
                if (!cxxRecordDecl)
                    cxxRecordDecl = type->getPointeeCXXRecordDecl();
                if (!cxxRecordDecl)
                    return true;
                if (!isDerivedFromException(cxxRecordDecl))
                    return true;
                auto loc = compat::getBeginLoc(operatorCallExpr);
                // for some reason, I'm warning multiple times? so just check if I've warned already
                if (!m_visited.insert(compiler.getSourceManager().getExpansionLoc(loc)).second)
                    return true;
                report(DiagnosticsEngine::Warning,
                       "use TOOLS_WARN_EXCEPTION/TOOLS_INFO_EXCEPTION/exceptionToString to print "
                       "exception nicely",
                       loc)
                    << operatorCallExpr->getSourceRange();
                return true;
            }
        return true;
    }
};

static Plugin::Registration<LogExceptionNicely> logexceptionnicely("logexceptionnicely");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
