/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include "plugin.hxx"

namespace {

class DllPrivate final:
    public loplugin::FilteringPlugin<DllPrivate>
{
public:
    explicit DllPrivate(loplugin::InstantiationData const & data): FilteringPlugin(data)
    {}

    bool VisitNamedDecl(NamedDecl const * decl) {
        if (!decl->getLocation().isInvalid()&&ignoreLocation(decl)) {
            return true;
        }
        auto a = decl->getAttr<VisibilityAttr>();
        if (a == nullptr || a->getVisibility() != VisibilityAttr::Hidden) {
            return true;
        }
        if (compiler.getSourceManager().isMacroBodyExpansion(
                decl->getLocation())
            && (Lexer::getImmediateMacroName(
                    decl->getLocation(), compiler.getSourceManager(),
                    compiler.getLangOpts())
                == "Q_OBJECT")) // from /usr/include/QtCore/qobjectdefs.h
        {
            return true;
        }
        auto p = dyn_cast<RecordDecl>(decl->getDeclContext());
        if (p == nullptr) {
            report(
                DiagnosticsEngine::Warning,
                "top-level declaration redundantly marked as DLLPRIVATE",
                a->getLocation())
                << decl->getSourceRange();
        } else if (p->getVisibility() == HiddenVisibility) {
            report(
                DiagnosticsEngine::Warning,
                ("declaration nested in DLLPRIVATE declaration redundantly"
                 " marked as DLLPRIVATE"),
                a->getLocation())
                << decl->getSourceRange();
            report(
                DiagnosticsEngine::Note, "parent declaration is here",
                p->getLocation())
                << p->getSourceRange();
        }
        return true;
    }

    bool preRun() override {
        // DISABLE_DYNLOADING makes SAL_DLLPUBLIC_{EXPORT,IMPORT,TEMPLATE} expand
        // to visibility("hidden") attributes, which would cause bogus warnings
        // here (e.g., in UBSan builds that explicitly define DISABLE_DYNLOADING
        // in jurt/source/pipe/staticsalhack.cxx); alternatively, change
        // include/sal/types.h to make those SAL_DLLPUBLIC_* expand to nothing
        // for DISABLE_DYNLOADING:
        return !compiler.getPreprocessor().getIdentifierInfo("DISABLE_DYNLOADING")
            ->hasMacroDefinition();
    }

    void run() override {
        if (preRun()) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }
};

static loplugin::Plugin::Registration<DllPrivate> dllprivate("dllprivate");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS


/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
