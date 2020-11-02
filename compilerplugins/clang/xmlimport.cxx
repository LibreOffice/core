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

#include "compat.hxx"
#include "plugin.hxx"
#include "check.hxx"
#include <iostream>
#include <unordered_map>
#include "clang/AST/CXXInheritance.h"

/*
 * This is a compile-time checker.
 *
 * Check that when we override SvXmlImportContext, and we override createFastChildContext,
 * we have also overridden startFastElement, or the fast-parser stuff will not work
 * correctly.
*/

namespace
{
class XmlImport : public loplugin::FilteringPlugin<XmlImport>
{
public:
    explicit XmlImport(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool preRun() override
    {
        StringRef fn(handler.getMainFileName());
        if (loplugin::isSamePathname(fn, SRCDIR "/xmloff/source/core/xmlictxt.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/xmloff/source/core/xmlimp.cxx"))
            return false;
        // These are mostly classes delegating calls to other classes
        if (loplugin::isSamePathname(fn, SRCDIR "/xmloff/source/text/XMLTextFrameContext.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/xmloff/source/draw/ximpshap.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/xmloff/source/table/XMLTableImport.cxx"))
            return false;
        if (loplugin::isSamePathname(fn,
                                     SRCDIR "/sc/source/filter/xml/XMLTrackedChangesContext.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/filter/xml/xmlannoi.cxx"))
            return false;
        // this class specifically wants to prevent some endFastElement processing happening in its superclass
        if (loplugin::isSamePathname(fn, SRCDIR
                                     "/xmloff/source/text/XMLIndexBibliographySourceContext.cxx"))
            return false;
        return true;
    }

    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitCXXMethodDecl(const CXXMethodDecl*);
    bool VisitCXXMemberCallExpr(const CXXMemberCallExpr*);

private:
    std::unordered_map<const CXXRecordDecl*, const CXXMethodDecl*> startFastElementSet;
    std::unordered_map<const CXXRecordDecl*, const CXXMethodDecl*> StartElementSet;
    std::unordered_map<const CXXRecordDecl*, const CXXMethodDecl*> endFastElementSet;
    std::unordered_map<const CXXRecordDecl*, const CXXMethodDecl*> EndElementSet;
    std::unordered_map<const CXXRecordDecl*, const CXXMethodDecl*> charactersSet;
    std::unordered_map<const CXXRecordDecl*, const CXXMethodDecl*> CharactersSet;
};

bool XmlImport::VisitCXXMethodDecl(const CXXMethodDecl* methodDecl)
{
    auto beginLoc = compat::getBeginLoc(methodDecl);
    if (!beginLoc.isValid() || ignoreLocation(beginLoc))
        return true;

    if (!methodDecl->getIdentifier())
        return true;

    auto cxxRecordDecl = methodDecl->getParent();
    if (!cxxRecordDecl || !cxxRecordDecl->getIdentifier())
        return true;

    if (loplugin::DeclCheck(cxxRecordDecl).Class("SvXMLImportContext"))
        return true;

    if (!loplugin::isDerivedFrom(cxxRecordDecl, [](Decl const* decl) -> bool {
            auto const dc = loplugin::DeclCheck(decl);
            return bool(dc.ClassOrStruct("SvXMLImportContext").GlobalNamespace());
        }))
        return true;

    auto name = methodDecl->getName();
    if (name == "startFastElement")
        startFastElementSet.insert({ cxxRecordDecl, methodDecl });
    else if (name == "StartElement")
        StartElementSet.insert({ cxxRecordDecl, methodDecl });
    else if (name == "endFastElement")
        endFastElementSet.insert({ cxxRecordDecl, methodDecl });
    else if (name == "EndElement")
        EndElementSet.insert({ cxxRecordDecl, methodDecl });
    else if (name == "characters")
    {
        if (methodDecl->getNumParams() == 1)
            charactersSet.insert({ cxxRecordDecl, methodDecl });
    }
    else if (name == "Characters")
    {
        if (methodDecl->getNumParams() == 1)
            CharactersSet.insert({ cxxRecordDecl, methodDecl });
    }

    {
        auto it1 = endFastElementSet.find(cxxRecordDecl);
        auto it2 = EndElementSet.find(cxxRecordDecl);
        if (it1 != endFastElementSet.end() && it2 != EndElementSet.end())
        {
            auto methodDecl1 = it1->second;
            report(DiagnosticsEngine::Warning, "cannot override both endFastElement and EndElement",
                   compat::getBeginLoc(methodDecl1))
                << methodDecl1->getSourceRange();
            auto methodDecl2 = it2->second;
            report(DiagnosticsEngine::Warning, "cannot override both endFastElement and EndElement",
                   compat::getBeginLoc(methodDecl2))
                << methodDecl2->getSourceRange();
        }
    }

    {
        auto it1 = startFastElementSet.find(cxxRecordDecl);
        auto it2 = StartElementSet.find(cxxRecordDecl);
        if (it1 != startFastElementSet.end() && it2 != StartElementSet.end())
        {
            auto methodDecl1 = it1->second;
            report(DiagnosticsEngine::Warning,
                   "cannot override both startFastElement and StartElement",
                   compat::getBeginLoc(methodDecl1))
                << methodDecl1->getSourceRange();
            auto methodDecl2 = it2->second;
            report(DiagnosticsEngine::Warning,
                   "cannot override both startFastElement and StartElement",
                   compat::getBeginLoc(methodDecl2))
                << methodDecl2->getSourceRange();
        }
    }
    {
        auto it1 = charactersSet.find(cxxRecordDecl);
        auto it2 = CharactersSet.find(cxxRecordDecl);
        if (it1 != charactersSet.end() && it2 != CharactersSet.end())
        {
            auto methodDecl1 = it1->second;
            report(DiagnosticsEngine::Warning, "cannot override both characters and Characters",
                   compat::getBeginLoc(methodDecl1))
                << methodDecl1->getSourceRange();
            auto methodDecl2 = it2->second;
            report(DiagnosticsEngine::Warning, "cannot override both characters and Characters",
                   compat::getBeginLoc(methodDecl2))
                << methodDecl2->getSourceRange();
        }
    }

    auto checkEmpty = [&]() {
        if (!methodDecl->isThisDeclarationADefinition())
            return;
        auto compoundStmt = dyn_cast_or_null<CompoundStmt>(methodDecl->getBody());
        if (compoundStmt == nullptr || compoundStmt->size() > 0)
            return;
        report(DiagnosticsEngine::Warning, "empty, should be removed",
               compat::getBeginLoc(methodDecl))
            << methodDecl->getSourceRange();
        auto canonicalDecl = methodDecl->getCanonicalDecl();
        if (canonicalDecl != methodDecl)
            report(DiagnosticsEngine::Note, "definition here", compat::getBeginLoc(canonicalDecl))
                << canonicalDecl->getSourceRange();
    };
    auto checkOnlyReturn = [&]() {
        if (!methodDecl->isThisDeclarationADefinition())
            return;
        auto compoundStmt = dyn_cast_or_null<CompoundStmt>(methodDecl->getBody());
        if (compoundStmt == nullptr || compoundStmt->size() > 1)
            return;
        auto returnStmt = dyn_cast_or_null<ReturnStmt>(*compoundStmt->body_begin());
        if (!returnStmt)
            return;
        auto cxxConstructExpr
            = dyn_cast_or_null<CXXConstructExpr>(returnStmt->getRetValue()->IgnoreImplicit());
        if (!cxxConstructExpr)
            return;
        if (cxxConstructExpr->getNumArgs() != 1)
            return;
        if (!isa<CXXNullPtrLiteralExpr>(cxxConstructExpr->getArg(0)->IgnoreImplicit()))
            return;
        report(DiagnosticsEngine::Warning, "empty, should be removed",
               compat::getBeginLoc(methodDecl))
            << methodDecl->getSourceRange();
        auto canonicalDecl = methodDecl->getCanonicalDecl();
        if (canonicalDecl != methodDecl)
            report(DiagnosticsEngine::Note, "definition here", compat::getBeginLoc(canonicalDecl))
                << canonicalDecl->getSourceRange();
    };

    if (name == "startFastElement")
        checkEmpty();
    else if (name == "endFastElement")
        checkEmpty();
    else if (name == "characters")
        checkEmpty();
    else if (name == "createFastChildContext")
        checkOnlyReturn();
    else if (name == "createUnknownChildContext")
        checkOnlyReturn();

    return true;
}

bool XmlImport::VisitCXXMemberCallExpr(const CXXMemberCallExpr* callExpr)
{
    auto beginLoc = compat::getBeginLoc(callExpr);
    if (!beginLoc.isValid() || ignoreLocation(callExpr))
        return true;

    CXXMethodDecl* methodDecl = callExpr->getMethodDecl();
    if (!methodDecl || !methodDecl->getIdentifier())
        return true;

    auto cxxRecordDecl = methodDecl->getParent();
    if (!cxxRecordDecl || !cxxRecordDecl->getIdentifier())
        return true;

    if (!loplugin::DeclCheck(cxxRecordDecl).Class("SvXMLImportContext"))
        return true;

    auto name = methodDecl->getName();
    if (name == "startFastElement" || name == "characters" || name == "endFastElement"
        || name == "createFastChildContext" || name == "createUnknownChildContext"
        || name == "StartElement" || name == "EndElement" || name == "Characters"
        || name == "CreateChildContext")
    {
        /**
         * Calling this superclass method from a subclass method will mess with the fallback logic in the superclass.
         */
        report(DiagnosticsEngine::Warning, "don't call this superclass method",
               compat::getBeginLoc(callExpr))
            << callExpr->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration<XmlImport> xmlimport("xmlimport");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
