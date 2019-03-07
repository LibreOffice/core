/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <set>
#include <string>

#include "plugin.hxx"

namespace {

using Bases = std::set<CXXRecordDecl const *>;

Visibility getTypeVisibility(CXXRecordDecl const * decl) {
    assert(decl->isThisDeclarationADefinition());
    if (auto const opt = decl->getExplicitVisibility(
            NamedDecl::VisibilityForType))
    {
        return *opt;
    }
    if (auto const opt = decl->getExplicitVisibility(
            NamedDecl::VisibilityForValue))
    {
        return *opt;
    }
    auto const vis = decl->getVisibility();
    return vis == DefaultVisibility && decl->isInAnonymousNamespace()
        ? HiddenVisibility : vis;
}

// Check whether 'decl' is derived from 'base', gathering any 'bases' between
// 'decl' and 'base', and whether any of those 'bases' or 'base' are 'hidden'
// (i.e., have non-default visibility):
bool isDerivedFrom(
    CXXRecordDecl const * decl, CXXRecordDecl const * base, Bases * bases,
    bool * hidden)
{
    bool derived = false;
    for (auto const i: decl->bases()) {
        auto const bd
            = (cast<CXXRecordDecl>(i.getType()->getAs<RecordType>()->getDecl())
               ->getDefinition());
        assert(bd != nullptr);
        if (bd == base) {
            *hidden |= getTypeVisibility(base) != DefaultVisibility;
            derived = true;
        }
        else if (bd->isDerivedFrom(base)) {
            if (bases->insert(bd).second) {
                auto const d = isDerivedFrom(bd, base, bases, hidden);
                assert(d);
                *hidden |= getTypeVisibility(bd) != DefaultVisibility;
            }
            derived = true;
        }
    }
    return derived;
}

StringRef vis(Visibility v) {
    switch (v) {
    case HiddenVisibility:
        return "hidden";
    case ProtectedVisibility:
        return "protected";
    case DefaultVisibility:
        return "default";
    default:
        llvm_unreachable("unknown visibility");
    }
}

class DynCastVisibility final:
    public loplugin::FilteringPlugin<DynCastVisibility>
{
public:
    explicit DynCastVisibility(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}

    bool shouldVisitTemplateInstantiations() const { return true; }

    bool VisitCXXDynamicCastExpr(CXXDynamicCastExpr const * expr) {
        if (ignoreLocation(expr)) {
            return true;
        }
        auto td = expr->getTypeAsWritten();
        if (auto const t = td->getAs<ReferenceType>()) {
            td = t->getPointeeType();
        }
        while (auto const t = td->getAs<clang::PointerType>()) {
            td = t->getPointeeType();
        }
        auto const rtd = td->getAs<RecordType>();
        if (rtd == nullptr) {
            return true;
        }
        auto const rdd = cast<CXXRecordDecl>(rtd->getDecl())->getDefinition();
        assert(rdd != nullptr);
        if (getTypeVisibility(rdd) != DefaultVisibility) {
            // Heuristic to find problematic dynamic_cast<T> with hidden type T is: T is defined in
            // include/M1/ while the compilation unit is in module M2/ with M1 != M2.  There are
            // legitimate cases where T is a hidden type in dynamic_cast<T>, e.g., when both the
            // type and the cast are in the same library.  This heuristic appears to be conservative
            // enough to produce only a few false positives (which have been addressed with
            // preceding commits, marking the relevant types in global include files as
            // SAL_DLLPUBLIC_RTTI after all, to be on the safe side) and aggressive enough to find
            // at least some interesting cases (though it would still not be aggressive enough to
            // have found ff570b4b58dbf274d3094d21d974f18b613e9b4b "DocumentSettingsSerializer must
            // be SAL_DLLPUBLIC_RTTI for dynamic_cast"):
            auto const file = getFileNameOfSpellingLoc(
                compiler.getSourceManager().getSpellingLoc(rdd->getLocation()));
            if (loplugin::hasPathnamePrefix(file, SRCDIR "/include/")) {
                std::size_t const n1 = std::strlen(SRCDIR "/include/");
                std::size_t n2 = file.find('/', n1);
#if defined _WIN32
                n2 = std::min(n2, file.find('\\', n1));
#endif
                auto const seg = n2 >= file.size() ? file.substr(n1) : file.substr(n1, n2 - n1);
                auto prefix = std::string(SRCDIR "/");
                prefix += seg;
                if (!loplugin::hasPathnamePrefix(
                            handler.getMainFileName(), prefix))
                {
                    report(
                        DiagnosticsEngine::Warning,
                        "Suspicious dynamic_cast to %0 with %1 type visibility", expr->getExprLoc())
                        << td << vis(getTypeVisibility(rdd)) << expr->getSourceRange();
                    report(DiagnosticsEngine::Note, "class %0 defined here", rdd->getLocation())
                        << td << rdd->getSourceRange();
                }
            }
            return true;
        }
        auto ts = expr->getSubExpr()->getType();
        while (auto const t = ts->getAs<clang::PointerType>()) {
            ts = t->getPointeeType();
        }
        auto const rts = ts->getAs<RecordType>();
        if (rts == nullptr) { // in case it's a dependent type
            return true;
        }
        auto const rds = cast<CXXRecordDecl>(rts->getDecl())->getDefinition();
        assert(rds != nullptr);
        Bases bs;
        bool hidden = false;
        if (!(isDerivedFrom(rdd, rds, &bs, &hidden) && hidden)) {
            return true;
        }
        report(
            DiagnosticsEngine::Warning,
            ("dynamic_cast from %0 with %1 type visibility to %2 with %3 type"
             " visibility"),
            expr->getExprLoc())
            << ts << vis(getTypeVisibility(rds)) << td
            << vis(getTypeVisibility(rdd)) << expr->getSourceRange();
        report(
            DiagnosticsEngine::Note,
            "base class %0 with %1 type visibility defined here",
            rds->getLocation())
            << ts << vis(getTypeVisibility(rds)) << rds->getSourceRange();
        for (auto const i: bs) {
            if (getTypeVisibility(i) != DefaultVisibility) {
                report(
                    DiagnosticsEngine::Note,
                    ("intermediary class %0 with %1 type visibility defined"
                     " here"),
                    i->getLocation())
                    << i << vis(getTypeVisibility(i)) << i->getSourceRange();
            }
        }
        report(
            DiagnosticsEngine::Note,
            "derived class %0 with %1 type visibility defined here",
            rdd->getLocation())
            << td << vis(getTypeVisibility(rdd)) << rdd->getSourceRange();
        return true;
    }

    virtual bool preRun() override {
        return compiler.getLangOpts().CPlusPlus;
    }
private:
    void run() override {
        if (preRun()) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }
};

static loplugin::Plugin::Registration<DynCastVisibility> dyncastvisibility(
    "dyncastvisibility");

}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
