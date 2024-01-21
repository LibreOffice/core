/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Find classes that derive from css::uno::XAggregation, but which implement queryInterface in
// violation of the protocol laid out in the documentation at
// udkapi/com/sun/star/uno/XAggregation.idl (which implies that such a class either doesn't actually
// make use of the deprecated XAggregation mechanism, which should thus be removed from that class
// hierarchy, or that its implementation of queryInterface needs to be fixed).

#ifndef LO_CLANG_SHARED_PLUGINS

#include <cassert>

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

namespace
{
bool isQueryInterface(CXXMethodDecl const* decl)
{
    auto const id = decl->getIdentifier();
    if (id == nullptr || id->getName() != "queryInterface")
    {
        return false;
    }
    if (decl->getNumParams() != 1)
    {
        return false;
    }
    if (!loplugin::TypeCheck(decl->getParamDecl(0)->getType())
             .LvalueReference()
             .ConstNonVolatile()
             .Class("Type")
             .Namespace("uno")
             .Namespace("star")
             .Namespace("sun")
             .Namespace("com")
             .GlobalNamespace())
    {
        return false;
    }
    return true;
}

bool derivesFromXAggregation(CXXRecordDecl const* decl, bool checkSelf)
{
    return loplugin::isDerivedFrom(decl,
                                   [](Decl const* decl) -> bool {
                                       return bool(loplugin::DeclCheck(decl)
                                                       .Class("XAggregation")
                                                       .Namespace("uno")
                                                       .Namespace("star")
                                                       .Namespace("sun")
                                                       .Namespace("com")
                                                       .GlobalNamespace());
                                   },
                                   checkSelf);
}

// Return true if decl is an implementation of css::uno::XInterface::queryInterface in a class
// derived from css::uno::XAggregation:
bool isXAggregationQueryInterface(CXXMethodDecl const* decl)
{
    return isQueryInterface(decl) && derivesFromXAggregation(decl->getParent(), false);
}

bool basesHaveOnlyPureQueryInterface(CXXRecordDecl const* decl)
{
    for (auto const& b : decl->bases())
    {
        auto const d1 = b.getType()->getAsCXXRecordDecl();
        if (!derivesFromXAggregation(d1, true))
        {
            continue;
        }
        for (auto const d2 : d1->methods())
        {
            if (!isQueryInterface(d2))
            {
                continue;
            }
            if (!compat::isPureVirtual(d2))
            {
                return false;
            }
        }
        if (!basesHaveOnlyPureQueryInterface(d1))
        {
            return false;
        }
    }
    return true;
}

class UnoAggregation final : public loplugin::FilteringPlugin<UnoAggregation>
{
public:
    explicit UnoAggregation(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool shouldVisitTemplateInstantiations() const { return true; }

    bool preRun() override { return compiler.getLangOpts().CPlusPlus; }

    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitCXXMethodDecl(CXXMethodDecl const* decl)
    {
        if (ignoreLocation(decl))
        {
            return true;
        }
        if (!decl->isThisDeclarationADefinition())
        {
            return true;
        }
        auto const parent = decl->getParent();
        if (parent->getDescribedClassTemplate() != nullptr)
        {
            // For class templates with dependent base classes, loplugin::isDerivedFrom as used in
            // isXAggregationQueryInterface would always return true; work around that by not
            // looking at any templates at all, which is OK due to
            // shouldVisitTemplateInstantiations:
            return true;
        }
        if (!isXAggregationQueryInterface(decl))
        {
            return true;
        }
        if (decl->isDeleted())
        {
            // Whether or not a deleted queryInterface makes sense, just leave those alone:
            return true;
        }
        auto const body = decl->getBody();
        assert(body != nullptr);
        // Check whether the implementation forwards to one of the base classes that derive from
        // XAggregation:
        if (auto const s1 = dyn_cast<CompoundStmt>(body))
        {
            if (s1->size() == 1)
            {
                if (auto const s2 = dyn_cast<ReturnStmt>(s1->body_front()))
                {
                    if (auto const e1 = s2->getRetValue())
                    {
                        if (auto const e2
                            = dyn_cast<CXXMemberCallExpr>(e1->IgnoreImplicit()->IgnoreParens()))
                        {
                            return true;
                            if (e2->getImplicitObjectArgument() == nullptr)
                            {
                                if (isXAggregationQueryInterface(e2->getMethodDecl()))
                                {
                                    // e2 will thus necessarily be a call of a base class's
                                    // queryInterface (or a recursive call of the given decl itself,
                                    // but which would cause the code to have undefined behavior
                                    // anyway, so don't bother to rule that out):
                                    return true;
                                }
                            }
                        }
                    }
                    else if (isDebugMode())
                    {
                        report(DiagnosticsEngine::Warning,
                               "suspicious implementation of queryInterface containing a return "
                               "statement with no operand",
                               decl->getLocation())
                            << decl->getSourceRange();
                    }
                }
            }
        }
        // As a crude approximation (but which appears to work OK), if all of the base classes that
        // derive from XAggregation only ever declare queryInterface as pure, assume that this is
        // the base implementation of queryInterface (which will necessarily not match the above
        // check for a forwarding implementation):
        if (basesHaveOnlyPureQueryInterface(parent))
        {
            return true;
        }
        if (suppressWarningAt(decl->getBeginLoc()))
        {
            return true;
        }
        report(DiagnosticsEngine::Warning,
               "%0 derives from XAggregation, but its implementation of queryInterface does not "
               "delegate to an appropriate base class queryInterface",
               decl->getLocation())
            << parent << decl->getSourceRange();
        return true;
    }
};

loplugin::Plugin::Registration<UnoAggregation> unoaggregation("unoaggregation");
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
