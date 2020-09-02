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
#include <iterator>
#include <list>
#include <set>

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

bool isInjectedFunction(FunctionDecl const* decl)
{
    for (auto d = decl->redecls_begin(); d != decl->redecls_end(); ++d)
    {
        auto const c = d->getLexicalDeclContext();
        if (!(c->isFunctionOrMethod() || c->isRecord()))
        {
            return false;
        }
    }
    return true;
}

// Whether type1 mentions type2 (in a way relevant for argument-dependent name lookup):
bool mentions(QualType type1, QualType type2)
{
    auto t1 = type1;
    for (;;)
    {
        if (auto const t2 = t1->getAs<ReferenceType>())
        {
            t1 = t2->getPointeeType();
        }
        else if (auto const t3 = t1->getAs<clang::PointerType>())
        {
            t1 = t3->getPointeeType();
        }
        else if (auto const t4 = t1->getAsArrayTypeUnsafe())
        {
            t1 = t4->getElementType();
        }
        else
        {
            break;
        }
    }
    if (t1.getCanonicalType().getTypePtr() == type2.getTypePtr())
    {
        return true;
    }
    if (auto const t2 = t1->getAs<TemplateSpecializationType>())
    {
        for (auto a = t2->begin(); a != t2->end(); ++a)
        {
            if (a->getKind() != TemplateArgument::Type)
            {
                continue;
            }
            if (mentions(a->getAsType(), type2))
            {
                return true;
            }
        }
        auto const t3 = t2->desugar();
        if (t3.getTypePtr() == t2)
        {
            return false;
        }
        return mentions(t3, type2);
    }
    if (auto const t2 = t1->getAs<FunctionProtoType>())
    {
        if (mentions(t2->getReturnType(), type2))
        {
            return true;
        }
        for (auto t3 = t2->param_type_begin(); t3 != t2->param_type_end(); ++t3)
        {
            if (mentions(*t3, type2))
            {
                return true;
            }
        }
        return false;
    }
    if (auto const t2 = t1->getAs<MemberPointerType>())
    {
        if (t2->getClass()->getUnqualifiedDesugaredType() == type2.getTypePtr())
        {
            return true;
        }
        return mentions(t2->getPointeeType(), type2);
    }
    return false;
}

bool hasSalDllpublicExportAttr(Decl const* decl)
{
    if (auto const attr = decl->getAttr<VisibilityAttr>())
    {
        return attr->getVisibility() == VisibilityAttr::Default;
    }
    return decl->hasAttr<DLLExportAttr>();
}

class External : public loplugin::FilteringPlugin<External>
{
public:
    explicit External(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitTagDecl(TagDecl* decl)
    {
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
            if (hasSalDllpublicExportAttr(d))
            {
                // If the class definition has explicit default visibility, then assume that it
                // needs to be present (e.g., a backwards-compatibility stub like in
                // cppuhelper/source/compat.cxx):
                return true;
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

    bool VisitFunctionDecl(FunctionDecl* decl)
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
        if (hasSalDllpublicExportAttr(decl))
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
        if (isInjectedFunction(decl))
        {
            return true;
        }
        return handleDeclaration(decl);
    }

    bool VisitVarDecl(VarDecl* decl)
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

    bool VisitClassTemplateDecl(ClassTemplateDecl* decl)
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

    bool VisitFunctionTemplateDecl(FunctionTemplateDecl* decl)
    {
        if (!decl->isThisDeclarationADefinition())
        {
            return true;
        }
        if (isa<CXXRecordDecl>(decl->getDeclContext()))
        {
            return true;
        }
        if (isInjectedFunction(decl->getTemplatedDecl()))
        {
            return true;
        }
        return handleDeclaration(decl);
    }

    bool VisitVarTemplateDecl(VarTemplateDecl* decl)
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

    void computeAffectedTypes(Decl const* decl, std::vector<QualType>* affected)
    {
        assert(affected != nullptr);
        if (auto const d = dyn_cast<EnumDecl>(decl))
        {
            affected->push_back(compiler.getASTContext().getEnumType(d));
        }
        else
        {
            //TODO: Derived types are also affected!
            CXXRecordDecl const* rec;
            if (auto const d = dyn_cast<ClassTemplateDecl>(decl))
            {
                rec = d->getTemplatedDecl();
            }
            else
            {
                rec = cast<CXXRecordDecl>(decl);
            }
            affected->push_back(compiler.getASTContext().getRecordType(rec));
            for (auto d = rec->decls_begin(); d != rec->decls_end(); ++d)
            {
                if (*d != (*d)->getCanonicalDecl())
                {
                    continue;
                }
                if (isa<TagDecl>(*d) || isa<ClassTemplateDecl>(*d))
                {
                    if (auto const d1 = dyn_cast<RecordDecl>(*d))
                    {
                        if (d1->isInjectedClassName())
                        {
                            continue;
                        }
                    }
                    computeAffectedTypes(*d, affected);
                }
            }
        }
    }

    void reportAssociatingFunctions(std::vector<QualType> const& affected, Decl* decl)
    {
        auto c = decl->getDeclContext();
        while (isa<LinkageSpecDecl>(c) || c->isInlineNamespace())
        {
            c = c->getParent();
        }
        assert(c->isTranslationUnit() || c->isNamespace());
        SmallVector<DeclContext*, 2> parts;
        c->collectAllContexts(parts);
        std::list<DeclContext const*> ctxs;
        std::copy(parts.begin(), parts.end(),
                  std::back_insert_iterator<std::list<DeclContext const*>>(ctxs));
        if (auto const d = dyn_cast<CXXRecordDecl>(decl))
        {
            // To find friend functions declared in the class:
            ctxs.push_back(d);
        }
        std::set<FunctionDecl const*> fdecls; // to report every function just once
        for (auto ctx = ctxs.begin(); ctx != ctxs.end(); ++ctx)
        {
            for (auto i = (*ctx)->decls_begin(); i != (*ctx)->decls_end(); ++i)
            {
                auto d = *i;
                if (auto const d1 = dyn_cast<LinkageSpecDecl>(d))
                {
                    ctxs.push_back(d1);
                    continue;
                }
                if (auto const d1 = dyn_cast<NamespaceDecl>(d))
                {
                    if (d1->isInline())
                    {
                        ctxs.push_back(d1);
                    }
                    continue;
                }
                if (auto const d1 = dyn_cast<FriendDecl>(d))
                {
                    d = d1->getFriendDecl();
                    if (d == nullptr) // happens for 'friend struct S;'
                    {
                        continue;
                    }
                }
                FunctionDecl const* f;
                if (auto const d1 = dyn_cast<FunctionTemplateDecl>(d))
                {
                    f = d1->getTemplatedDecl();
                }
                else
                {
                    f = dyn_cast<FunctionDecl>(d);
                    if (f == nullptr)
                    {
                        continue;
                    }
                }
                if (!fdecls.insert(f->getCanonicalDecl()).second)
                {
                    continue;
                }
                if (isa<CXXMethodDecl>(f))
                {
                    continue;
                }
                for (auto const& t : affected)
                {
                    auto const tc = t.getCanonicalType();
                    for (auto p = f->param_begin(); p != f->param_end(); ++p)
                    {
                        if (mentions((*p)->getType(), tc))
                        {
                            report(DiagnosticsEngine::Note,
                                   "a %select{function|function template|function template "
                                   "specialization}0 associating %1 is declared here",
                                   f->getLocation())
                                << (f->isFunctionTemplateSpecialization()
                                        ? 2
                                        : f->getDescribedFunctionTemplate() != nullptr ? 1 : 0)
                                << t << f->getSourceRange();
                            for (auto f1 = f->redecls_begin(); f1 != f->redecls_end(); ++f1)
                            {
                                if (*f1 == f)
                                {
                                    continue;
                                }
                                report(DiagnosticsEngine::Note, "another declaration is here",
                                       f1->getLocation())
                                    << f1->getSourceRange();
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    void reportAssociatingFunctions(Decl* decl)
    {
        std::vector<QualType> affected; // enum/class/class template + recursively affected members
        computeAffectedTypes(decl, &affected);
        reportAssociatingFunctions(affected, decl);
    }

    bool handleDeclaration(NamedDecl* decl)
    {
        if (ignoreLocation(decl))
        {
            return true;
        }
        if (decl->getLinkageInternal() < ModuleLinkage)
        {
            return true;
        }
        // In some cases getLinkageInternal() arguably wrongly reports ExternalLinkage, see the
        // commit message of <https://github.com/llvm/llvm-project/commit/
        // df963a38a9e27fc43b485dfdf52bc1b090087e06> "DR1113: anonymous namespaces formally give
        // their contents internal linkage":
        //
        //  "We still deviate from the standard in one regard here: extern "C" declarations
        //   in anonymous namespaces are still granted external linkage. Changing those does
        //   not appear to have been an intentional consequence of the standard change in
        //   DR1113."
        //
        // Do not warn about such "wrongly external" declarations here:
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
        // In general, moving functions into an unnamed namespace can: break ADL like in
        //
        //   struct S1 { int f() { return 1; } };
        //   int f(S1 s) { return s.f(); }
        //   namespace N {
        //     struct S2: S1 { int f() { return 0; } };
        //     int f(S2 s) { return s.f(); } // [*]
        //   }
        //   int main() { return f(N::S2()); }
        //
        // changing from returning 0 to returning 1 when [*] is moved into an unnamed namespace; can
        // conflict with function declarations in the moved function like in
        //
        //   int f(int) { return 0; }
        //   namespace { int f(int) { return 1; } }
        //   int g() { // [*]
        //     int f(int);
        //     return f(0);
        //   }
        //   int main() { return g(); }
        //
        // changing from returning 0 to returning 1 when [*] is moved into an unnamed namespace; and
        // can conflict with overload resolution in general like in
        //
        //   int f(int) { return 0; }
        //   namespace { int f(...) { return 1; } }
        //   int g() { return f(0); } // [*]
        //   int main() { return g(); }
        //
        // changing from returning 0 to returning 1 when [*] is moved into an unnamed namespace:
        auto const canUnnamed = compiler.getLangOpts().CPlusPlus
                                && !(isa<FunctionDecl>(decl) || isa<FunctionTemplateDecl>(decl));
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
        for (auto d = decl->redecls_begin(); d != decl->redecls_end(); ++d)
        {
            if (*d == decl)
            {
                continue;
            }
            report(DiagnosticsEngine::Note, "another declaration is here", d->getLocation())
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
        if (isa<TagDecl>(decl) || isa<ClassTemplateDecl>(decl))
        {
            reportAssociatingFunctions(decl);
        }
        return true;
    }
};

loplugin::Plugin::Registration<External> external("external");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
