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

#include "clang/Sema/SemaDiagnostic.h"

#include "check.hxx"
#include "plugin.hxx"

namespace
{
bool derivesFromTestFixture(CXXRecordDecl const* decl)
{
    static auto const pred = [](CXXBaseSpecifier const& spec) {
        if (auto const t = spec.getType()->getAs<RecordType>())
        { // (may be a template parameter)
            return derivesFromTestFixture(dyn_cast<CXXRecordDecl>(t->getDecl()));
        }
        return false;
    };
    return loplugin::DeclCheck(decl).Class("TestFixture").Namespace("CppUnit").GlobalNamespace()
           || std::any_of(decl->bases_begin(), decl->bases_end(), pred)
           || std::any_of(decl->vbases_begin(), decl->vbases_end(), pred);
}

class External : public loplugin::FilteringPlugin<External>
{
public:
    explicit External(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitTagDecl(TagDecl const* decl)
    {
        /*TODO:*/
        return true; // in general, moving classes or enumerations into an unnamed namespace can break ADL
        if (isa<ClassTemplateSpecializationDecl>(decl))
        {
            return true;
        }
        if (!decl->isThisDeclarationADefinition())
        {
            return true;
        }
        if (isa<CXXRecordDecl>(decl->getDeclContext()))
        {
            return true;
        }
        if (!compiler.getLangOpts().CPlusPlus)
        {
            return true;
        }
        if (auto const d = dyn_cast<CXXRecordDecl>(decl))
        {
            if (d->getDescribedClassTemplate() != nullptr)
            {
                return true;
            }
            if (auto const attr = d->getAttr<VisibilityAttr>())
            {
                if (attr->getVisibility() == VisibilityAttr::Default)
                {
                    // If the class definition has explicit default visibility, then assume that it
                    // needs to be present (e.g., a backwards-compatibility stub like in
                    // cppuhelper/source/compat.cxx):
                    return true;
                }
            }
            if (derivesFromTestFixture(d))
            {
                // The names of CppUnit tests (that can be specified with CPPUNIT_TEST_NAME) are
                // tied to the fully-qualified names of classes derived from CppUnit::TestFixture,
                // so avoid unnamed namespaces in those classes' names:
                return true;
            }
        }
        return handleDeclaration(decl);
    }

    bool VisitFunctionDecl(FunctionDecl const* decl)
    {
        if (isa<CXXMethodDecl>(decl))
        {
            return true;
        }
        if (decl->getTemplatedKind() != FunctionDecl::TK_NonTemplate)
        {
            return true;
        }
        if (!decl->isThisDeclarationADefinition())
        {
            return true;
        }
        if (decl->isMain() || decl->isMSVCRTEntryPoint())
        {
            return true;
        }
        if (loplugin::hasCLanguageLinkageType(decl)
            && loplugin::DeclCheck(decl).Function("_DllMainCRTStartup").GlobalNamespace())
        {
            return true;
        }
        // If the function definition is explicit marked SAL_DLLPUBLIC_EXPORT or similar, then
        // assume that it needs to be present (e.g., only called via dlopen, or a backwards-
        // compatibility stub like in sal/osl/all/compat.cxx):
        if (auto const attr = decl->getAttr<VisibilityAttr>())
        {
            if (attr->getVisibility() == VisibilityAttr::Default)
            {
                return true;
            }
        }
        else if (decl->hasAttr<DLLExportAttr>())
        {
            return true;
        }
        auto const canon = decl->getCanonicalDecl();
        if (loplugin::hasCLanguageLinkageType(canon)
            && (canon->hasAttr<ConstructorAttr>() || canon->hasAttr<DestructorAttr>()))
        {
            return true;
        }
        if (compiler.getDiagnostics().getDiagnosticLevel(diag::warn_unused_function,
                                                         decl->getLocation())
            < DiagnosticsEngine::Warning)
        {
            // Don't warn about e.g.
            //
            //  G_DEFINE_TYPE (GLOAction, g_lo_action, G_TYPE_OBJECT);
            //
            // in vcl/unx/gtk/gloactiongroup.cxx (which expands to non-static g_lo_action_get_type
            // function definition), which is already wrapped in
            //
            //  #pragma GCC diagnostic ignored "-Wunused-function"
            return true;
        }
        return handleDeclaration(decl);
    }

    bool VisitVarDecl(VarDecl const* decl)
    {
        if (decl->isStaticDataMember())
        {
            return true;
        }
        if (isa<VarTemplateSpecializationDecl>(decl))
        {
            return true;
        }
        if (!decl->isThisDeclarationADefinition())
        {
            return true;
        }
        if (loplugin::DeclCheck(decl).Var("_pRawDllMain").GlobalNamespace())
        {
            return true;
        }
        return handleDeclaration(decl);
    }

    bool VisitClassTemplateDecl(ClassTemplateDecl const* decl)
    {
        /*TODO:*/
        return true; // in general, moving classes or enumerations into an unnamed namespace can break ADL
        if (!decl->isThisDeclarationADefinition())
        {
            return true;
        }
        if (isa<CXXRecordDecl>(decl->getDeclContext()))
        {
            return true;
        }
        return handleDeclaration(decl);
    }

    bool VisitFunctionTemplateDecl(FunctionTemplateDecl const* decl)
    {
        if (!decl->isThisDeclarationADefinition())
        {
            return true;
        }
        if (isa<CXXRecordDecl>(decl->getDeclContext()))
        {
            return true;
        }
        return handleDeclaration(decl);
    }

    bool VisitVarTemplateDecl(VarTemplateDecl const* decl)
    {
        if (!decl->isThisDeclarationADefinition())
        {
            return true;
        }
        return handleDeclaration(decl);
    }

private:
    template <typename T> void reportSpecializations(T specializations)
    {
        for (auto const d : specializations)
        {
            auto const k = d->getTemplateSpecializationKind();
            if (isTemplateExplicitInstantiationOrSpecialization(k))
            {
                report(DiagnosticsEngine::Note,
                       "explicit %select{instantiation|specialization}0 is here", d->getLocation())
                    << (k == TSK_ExplicitSpecialization) << d->getSourceRange();
            }
        }
    }

    bool handleDeclaration(NamedDecl const* decl)
    {
        if (ignoreLocation(decl))
        {
            return true;
        }
        if (decl->getLinkageInternal() < ModuleLinkage)
        {
            return true;
        }
        //TODO: in some cases getLinkageInternal() appears to report ExternalLinkage instead of
        // UniqueExternalLinkage:
        if (decl->isInAnonymousNamespace())
        {
            return true;
        }
        for (Decl const* d = decl; d != nullptr; d = d->getPreviousDecl())
        {
            if (!compiler.getSourceManager().isInMainFile(d->getLocation()))
            {
                return true;
            }
        }
        if (compiler.getSourceManager().isMacroBodyExpansion(decl->getLocation()))
        {
            if (Lexer::getImmediateMacroName(decl->getLocation(), compiler.getSourceManager(),
                                             compiler.getLangOpts())
                == "MDDS_MTV_DEFINE_ELEMENT_CALLBACKS")
            {
                // Even wrapping in an unnamed namespace or sneaking "static" into the macro
                // wouldn't help, as then some of the functions it defines would be flagged as
                // unused:
                return true;
            }
        }
        else if (compiler.getSourceManager().isMacroArgExpansion(decl->getLocation()))
        {
            if (Lexer::getImmediateMacroName(decl->getLocation(), compiler.getSourceManager(),
                                             compiler.getLangOpts())
                == "DEFINE_GUID")
            {
                // Windows, guiddef.h:
                return true;
            }
        }
        TypedefNameDecl const* typedefed = nullptr;
        if (auto const d = dyn_cast<TagDecl>(decl))
        {
            typedefed = d->getTypedefNameForAnonDecl();
        }
        bool canStatic;
        if (auto const d = dyn_cast<CXXRecordDecl>(decl))
        {
            canStatic = d->isUnion() && d->isAnonymousStructOrUnion();
        }
        else
        {
            canStatic = isa<FunctionDecl>(decl) || isa<VarDecl>(decl)
                        || isa<FunctionTemplateDecl>(decl) || isa<VarTemplateDecl>(decl);
        }
        auto const canUnnamed = compiler.getLangOpts().CPlusPlus
                                && !(isa<FunctionDecl>(decl) || isa<FunctionTemplateDecl>(decl));
        // in general, moving functions into an unnamed namespace can break ADL
        assert(canStatic || canUnnamed);
        report(
            DiagnosticsEngine::Warning,
            ("externally available%select{| typedef'ed}0 entity %1 is not previously declared in an"
             " included file (if it is only used in this translation unit,"
             " %select{|make it static}2%select{| or }3%select{|put it in an unnamed namespace}4;"
             " otherwise, provide a declaration of it in an included file)"),
            decl->getLocation())
            << (typedefed != nullptr) << (typedefed == nullptr ? decl : typedefed) << canStatic
            << (canStatic && canUnnamed) << canUnnamed << decl->getSourceRange();
        for (auto d = decl->getPreviousDecl(); d != nullptr; d = d->getPreviousDecl())
        {
            report(DiagnosticsEngine::Note, "previous declaration is here", d->getLocation())
                << d->getSourceRange();
        }
        //TODO: Class template specializations can be in the enclosing namespace, so no need to
        // list them here (as they won't need to be put into the unnamed namespace too, unlike for
        // specializations of function and variable templates); and explicit function template
        // specializations cannot have storage-class specifiers, so as we only suggest to make
        // function templates static (but not to move them into an unnamed namespace), no need to
        // list function template specializations here, either:
        if (auto const d = dyn_cast<VarTemplateDecl>(decl))
        {
            reportSpecializations(d->specializations());
        }
        return true;
    }
};

loplugin::Plugin::Registration<External> external("external");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
