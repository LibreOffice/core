/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Find uses of dynamic_cast that cast between unrelated classes, which is suspicious and might
// indicate a bug.  The heuristic used to consider two classes unrelated is that neither derives
// from the other (directly or indirectly) and they do not both virtually derive (directly or
// indirectly) from a common third class.  Additionally, class definitions can be attributed with
// SAL_LOPLUGIN_ANNOTATE("crosscast") (from sal/types.h) to suppress false warnings about known-good
// cases casting from or to such a class.

#ifndef LO_CLANG_SHARED_PLUGINS

#include <cassert>
#include <set>

#include "compat.hxx"
#include "plugin.hxx"

namespace
{
void computeVirtualBases(CXXRecordDecl const* decl, std::set<CXXRecordDecl const*>* vbases)
{
    assert(vbases != nullptr);
    for (auto const& i : decl->bases())
    {
        auto const d = i.getType()->getAsCXXRecordDecl();
        if (d == nullptr)
        {
            continue;
        }
        if (i.isVirtual())
        {
            if (!vbases->insert(d->getCanonicalDecl()).second)
            {
                // As we track the already computed virtual bases in vbases anyway, we can cheaply
                // optimize the case where we see a virtual base again, even though we don't bother
                // to optimize the case where we see a non-virtual base multiple times:
                continue;
            }
        }
        computeVirtualBases(d, vbases);
    }
}

bool compareVirtualBases(CXXRecordDecl const* decl, std::set<CXXRecordDecl const*>& vbases)
{
    for (auto const& i : decl->bases())
    {
        auto const d = i.getType()->getAsCXXRecordDecl();
        if (d == nullptr)
        {
            continue;
        }
        if (i.isVirtual() && vbases.count(d->getCanonicalDecl()) == 1)
        {
            return true;
        }
        if (compareVirtualBases(d, vbases))
        {
            return true;
        }
    }
    return false;
}

bool haveCommonVirtualBase(CXXRecordDecl const* decl1, CXXRecordDecl const* decl2)
{
    std::set<CXXRecordDecl const*> vbases;
    computeVirtualBases(decl1, &vbases);
    return compareVirtualBases(decl2, vbases);
}

bool isAllowedInCrossCasts(CXXRecordDecl const* decl)
{
    auto const def = decl->getDefinition();
    if (def == nullptr)
    {
        return false;
    }
    for (auto const attr : def->specific_attrs<AnnotateAttr>())
    {
        if (attr->getAnnotation() == "loplugin:crosscast")
        {
            return true;
        }
    }
    return false;
}

class CrossCast final : public loplugin::FilteringPlugin<CrossCast>
{
public:
    explicit CrossCast(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool preRun() override { return compiler.getLangOpts().CPlusPlus; }

    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitCXXDynamicCastExpr(CXXDynamicCastExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        auto const t2 = expr->getTypeAsWritten();
        if (t2->isVoidPointerType())
        {
            return true;
        }
        auto const d2 = t2->getPointeeCXXRecordDecl();
        if (d2 == nullptr)
        {
            return true;
        }
        auto const t1 = compat::getSubExprAsWritten(expr)->getType();
        auto t1a = t1;
        if (auto const t = t1a->getAs<clang::PointerType>())
        {
            t1a = t->getPointeeType();
        }
        auto const d1 = t1a->getAsCXXRecordDecl();
        if (d1 == nullptr)
        {
            return true;
        }
        if (d1->getCanonicalDecl() == d2->getCanonicalDecl() || d1->isDerivedFrom(d2)
            || d2->isDerivedFrom(d1) || haveCommonVirtualBase(d1, d2))
        {
            return true;
        }
        if (isAllowedInCrossCasts(d1) || isAllowedInCrossCasts(d2))
        {
            return true;
        }
        if (suppressWarningAt(expr->getBeginLoc()))
        {
            return true;
        }
        report(DiagnosticsEngine::Warning, "suspicious dynamic cross cast from %0 to %1",
               expr->getExprLoc())
            << t1 << t2 << expr->getSourceRange();
        return true;
    }
};

loplugin::Plugin::Registration<CrossCast> crosscast("crosscast");
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
