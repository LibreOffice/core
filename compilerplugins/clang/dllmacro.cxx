/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"
#include <iostream>
#include <fstream>

/**
  Check the usage of the various DLL visibility macros.
 */
class DllMacro final:
    public RecursiveASTVisitor<DllMacro>, public loplugin::Plugin
{
public:
    explicit DllMacro(InstantiationData const & data): Plugin(data) {}

    bool VisitNamedDecl(NamedDecl const * decl);

private:
    void run() override {
        // DISABLE_DYNLOADING makes SAL_DLLPUBLIC_{EXPORT,IMPORT,TEMPLATE} expand
        // to visibility("hidden") attributes, which would cause bogus warnings
        // here (e.g., in UBSan builds that explicitly define DISABLE_DYNLOADING
        // in jurt/source/pipe/staticsalhack.cxx); alternatively, change
        // include/sal/types.h to make those SAL_DLLPUBLIC_* expand to nothing
        // for DISABLE_DYNLOADING:
        if (!compiler.getPreprocessor().getIdentifierInfo("DISABLE_DYNLOADING")
            ->hasMacroDefinition())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }
};

bool DllMacro::VisitNamedDecl(NamedDecl const * decl) {
    if (!decl->getLocation().isInvalid() && ignoreLocation(decl)) {
        return true;
    }
    auto a = decl->getAttr<VisibilityAttr>();
    if (a == nullptr) {
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
    if (a->getVisibility() == VisibilityAttr::Hidden) {
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
    }
    else if (a->getVisibility() == VisibilityAttr::Default && !a->isInherited())
    {
        auto p = dyn_cast<CXXRecordDecl>(decl);
        if (p && p->isCompleteDefinition() && !p->getDescribedClassTemplate()) {
            if (isInUnoIncludeFile(
                    compiler.getSourceManager().getSpellingLoc(
                        decl->getLocation())))
            {
                return true;
            }
            // don't know what these macros mean, leave them alone
            auto macroLoc = a->getLocation();
            while (compiler.getSourceManager().isMacroBodyExpansion(macroLoc)) {
                auto macroName = Lexer::getImmediateMacroName(
                        macroLoc, compiler.getSourceManager(),
                        compiler.getLangOpts());
                if (macroName == "SAL_EXCEPTION_DLLPUBLIC_EXPORT")
                    return true;
                if (macroName == "VCL_PLUGIN_PUBLIC")
                    return true;
                if (macroName == "SAL_DLLPUBLIC_TEMPLATE")
                    // e.g., OAnyEnumeration_BASE in
                    // include/comphelper/enumhelper.hxx ("this is the way that
                    // works for ENABLE_LTO with MSVC 2013")
                    return true;
                macroLoc = compiler.getSourceManager().getImmediateMacroCallerLoc(macroLoc);
            }
            for (auto it = p->method_begin(); it != p->method_end(); ++it) {
                if (!(it->hasInlineBody() || it->isImplicit())) {
                    return true;
                }
            }
            report(
                DiagnosticsEngine::Warning,
                "unnecessary *DLLPUBLIC declaration for class completely defined in header file",
                a->getLocation())
                << decl->getSourceRange();
        }
    }
    return true;
}

static loplugin::Plugin::Registration<DllMacro> reg("dllmacro");

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
