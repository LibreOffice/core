/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>

#include "check.hxx"
#include "plugin.hxx"
#include "config_clang.h"

/**
 * Two checks to prevent GDI handle leaks on Windows:
 *
 * 1. Local VclPtr<VirtualDevice> created via VclPtr::Create() but never
 *    disposed — should use ScopedVclPtr<VirtualDevice> instead.
 *    Suppressed when the variable is:
 *      - returned from the function (factory pattern)
 *      - explicitly disposeAndClear()'d / clear()'d
 *      - not initialized via VclPtr::Create() (borrowing a shared reference)
 *
 * 2. Functions returning VclPtr<VirtualDevice> — should return
 *    ScopedVclPtr<VirtualDevice> so callers get automatic cleanup.
 *
 * VclPtr does NOT call dispose() in its destructor.  ScopedVclPtr does.
 */

namespace
{
// ---------------------------------------------------------------------------
// helpers
// ---------------------------------------------------------------------------

/// Strip implicit conversions, copy/move constructors, and temporary
/// materializations to find the underlying DeclRefExpr (if any).
static const DeclRefExpr* getUnderlyingDeclRef(const Expr* pExpr)
{
    if (!pExpr)
        return nullptr;
    pExpr = pExpr->IgnoreImplicit();
    if (auto* pDeclRef = dyn_cast<DeclRefExpr>(pExpr))
        return pDeclRef;
    if (auto* pConstruct = dyn_cast<CXXConstructExpr>(pExpr))
    {
        if (pConstruct->getNumArgs() == 1)
            return getUnderlyingDeclRef(pConstruct->getArg(0));
    }
    if (auto* pMaterialize = dyn_cast<MaterializeTemporaryExpr>(pExpr))
        return getUnderlyingDeclRef(pMaterialize->getSubExpr());
    if (auto* pBind = dyn_cast<CXXBindTemporaryExpr>(pExpr))
        return getUnderlyingDeclRef(pBind->getSubExpr());
    return nullptr;
}

/// Is the VarDecl directly returned from any ReturnStmt in the function body?
/// Only matches when the variable itself is the return value (possibly through
/// implicit conversions), NOT when it is merely referenced inside the return
/// expression (e.g. pVar->GetSomething()).
static bool isReturnedFromStmt(const Stmt* pStmt, const VarDecl* pVarDecl)
{
    if (!pStmt)
        return false;
    if (auto* pReturn = dyn_cast<ReturnStmt>(pStmt))
    {
        auto* pDeclRef = getUnderlyingDeclRef(pReturn->getRetValue());
        if (pDeclRef && pDeclRef->getDecl() == pVarDecl)
            return true;
    }
    for (auto* child : pStmt->children())
    {
        if (isReturnedFromStmt(child, pVarDecl))
            return true;
    }
    return false;
}

/// Is disposeAndClear() or clear() called on the VarDecl somewhere in the body?
static bool isDisposedInStmt(const Stmt* pStmt, const VarDecl* pVarDecl)
{
    if (!pStmt)
        return false;
    if (auto* pCallExpr = dyn_cast<CXXMemberCallExpr>(pStmt))
    {
        if (auto* pCallee = pCallExpr->getDirectCallee())
        {
            auto check = loplugin::DeclCheck(pCallee);
            if (check.Function("disposeAndClear") || check.Function("clear"))
            {
                // Check that the object being called is our VarDecl
                if (auto* pMemberExpr = dyn_cast<MemberExpr>(pCallExpr->getCallee()))
                {
                    auto* pBase = pMemberExpr->getBase()->IgnoreImplicit();
                    if (auto* pDeclRef = dyn_cast<DeclRefExpr>(pBase))
                    {
                        if (pDeclRef->getDecl() == pVarDecl)
                            return true;
                    }
                }
            }
        }
    }
    for (auto* child : pStmt->children())
    {
        if (isDisposedInStmt(child, pVarDecl))
            return true;
    }
    return false;
}

/// Does the expression tree contain a call to VclPtr<...>::Create()?
static bool containsVclPtrCreate(const Stmt* pStmt)
{
    if (!pStmt)
        return false;
    if (auto* pCallExpr = dyn_cast<CallExpr>(pStmt))
    {
        if (auto* pCallee = pCallExpr->getDirectCallee())
        {
            if (pCallee->getNameAsString() == "Create")
            {
                if (auto* pMethodDecl = dyn_cast<CXXMethodDecl>(pCallee))
                {
                    if (loplugin::DeclCheck(pMethodDecl)
                            .MemberFunction()
                            .Class("VclPtr")
                            .GlobalNamespace())
                        return true;
                }
            }
        }
    }
    for (auto* child : pStmt->children())
    {
        if (containsVclPtrCreate(child))
            return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// plugin
// ---------------------------------------------------------------------------

class ScopedVclPtrCheck : public loplugin::FilteringPlugin<ScopedVclPtrCheck>
{
public:
    explicit ScopedVclPtrCheck(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual bool preRun() override
    {
        StringRef fn(handler.getMainFileName());
        if (loplugin::isSamePathname(fn, SRCDIR "/include/vcl/vclptr.hxx"))
            return false;
        return true;
    }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitVarDecl(const VarDecl*);
    bool VisitFunctionDecl(const FunctionDecl*);

private:
    bool isVclPtrToVirtualDevice(QualType qType);
};

bool ScopedVclPtrCheck::isVclPtrToVirtualDevice(QualType qType)
{
    auto check = loplugin::TypeCheck(qType);
    if (!check.Class("VclPtr").GlobalNamespace())
        return false;

    const clang::Type* pType = qType.getTypePtr();
    if (!pType)
        return false;

    const CXXRecordDecl* pRecordDecl = pType->getAsCXXRecordDecl();
    if (!pRecordDecl)
        return false;

    const auto* pTemplate = dyn_cast<ClassTemplateSpecializationDecl>(pRecordDecl);
    if (!pTemplate)
        return false;

    if (pTemplate->getTemplateArgs().size() < 1)
        return false;

    const TemplateArgument& rArg = pTemplate->getTemplateArgs()[0];
    if (rArg.getKind() != TemplateArgument::ArgKind::Type)
        return false;

    return bool(loplugin::TypeCheck(rArg.getAsType()).Class("VirtualDevice").GlobalNamespace());
}

bool ScopedVclPtrCheck::VisitVarDecl(const VarDecl* pVarDecl)
{
    if (ignoreLocation(pVarDecl))
        return true;

    if (isa<ParmVarDecl>(pVarDecl))
        return true;
    if (isa<FieldDecl>(pVarDecl))
        return true;
    if (pVarDecl->hasGlobalStorage())
        return true;

    if (!isVclPtrToVirtualDevice(pVarDecl->getType()))
        return true;

    // Only warn when the variable is initialized via VclPtr::Create().
    // Variables initialized from other function calls are typically borrowing
    // a shared/cached/member-owned device, not creating a new one.
    auto* pInit = pVarDecl->getInit();
    if (!pInit || !containsVclPtrCreate(pInit))
        return true;

    auto* pFuncDecl = dyn_cast<FunctionDecl>(pVarDecl->getDeclContext());
    if (pFuncDecl && pFuncDecl->hasBody())
    {
        const Stmt* pBody = pFuncDecl->getBody();

        // Factory pattern: the local is returned to the caller.
        if (isReturnedFromStmt(pBody, pVarDecl))
            return true;

        // Manual lifecycle: disposeAndClear() or clear() is called explicitly.
        if (isDisposedInStmt(pBody, pVarDecl))
            return true;
    }

    if (suppressWarningAt(pVarDecl->getLocation()))
        return true;

    report(DiagnosticsEngine::Warning,
           "use ScopedVclPtr<VirtualDevice> instead of VclPtr<VirtualDevice>"
           " for local variables to prevent GDI handle leaks"
           " [loplugin:scopedvclptr]",
           pVarDecl->getLocation())
        << pVarDecl->getSourceRange();

    return true;
}

bool ScopedVclPtrCheck::VisitFunctionDecl(const FunctionDecl* pFuncDecl)
{
    if (ignoreLocation(pFuncDecl))
        return true;

    if (!pFuncDecl->isThisDeclarationADefinition())
        return true;

    if (!isVclPtrToVirtualDevice(pFuncDecl->getReturnType()))
        return true;

    if (suppressWarningAt(pFuncDecl->getLocation()))
        return true;

    report(DiagnosticsEngine::Warning,
           "use ScopedVclPtr<VirtualDevice> as return type instead of"
           " VclPtr<VirtualDevice> to prevent GDI handle leaks"
           " [loplugin:scopedvclptr]",
           pFuncDecl->getLocation())
        << pFuncDecl->getSourceRange();

    return true;
}

loplugin::Plugin::Registration<ScopedVclPtrCheck> scopedvclptr("scopedvclptr");

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
