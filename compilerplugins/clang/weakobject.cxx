/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"
#include "typecheck.hxx"

/* OWeakObject::release() disposes weak references.  If that isn't done
 * because a sub-class improperly overrides release() then
 * OWeakConnectionPoint::m_pObject continues to point to the deleted object
 * and that could result in use-after-free.
 */

namespace {

class WeakObject
    : public clang::RecursiveASTVisitor<WeakObject>
    , public loplugin::Plugin
{

public:
    explicit WeakObject(InstantiationData const& rData) : Plugin(rData) {}

    void run() override {
        if (compiler.getLangOpts().CPlusPlus) { // no OWeakObject in C
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool isDerivedFromOWeakObject(CXXMethodDecl const*const pMethodDecl)
    {
        CXXRecordDecl const*const pClass(pMethodDecl->getParent());
        if (pClass->getQualifiedNameAsString() == "cppu::OWeakObject")
        {
            return true;
        }
        // hopefully it's faster to recurse overridden methods than the
        // thicket of WeakImplHelper32 but that is purely speculation
        for (auto it = pMethodDecl->begin_overridden_methods();
             it != pMethodDecl->end_overridden_methods(); ++it)
        {
            if (isDerivedFromOWeakObject(*it))
            {
                return true;
            }
        }
        return false;
    }

    bool VisitCXXMethodDecl(CXXMethodDecl const*const pMethodDecl)
    {
        if (ignoreLocation(pMethodDecl)) {
            return true;
        }
        if (!pMethodDecl->isThisDeclarationADefinition()) {
            return true;
        }
        if (!pMethodDecl->isInstance()) {
            return true;
        }
// this is too "simple", if a NamedDecl class has a getName() member expecting it to actually work would clearly be unreasonable    if (pMethodDecl->getName() != "release") {
        if (pMethodDecl->getNameAsString() != "release") {
            return true;
        }
        if (pMethodDecl->getNumParams() != 0) {
            return true;
        }
        if (pMethodDecl->getParent()->getQualifiedNameAsString() == "cppu::OWeakObject")
        {
            return true;
        }

        CXXMethodDecl const* pOverridden(nullptr);
        for (auto it = pMethodDecl->begin_overridden_methods();
             it != pMethodDecl->end_overridden_methods(); ++it)
        {
            if ((*it)->getName() == "release")
            {
                pOverridden = *it;
                break;
            }
        }
        if (pOverridden == nullptr)
        {
            return true;
        }
        if (!isDerivedFromOWeakObject(pOverridden))
        {
            return true;
        }
        CompoundStmt const*const pCompoundStatement(
                dyn_cast<CompoundStmt>(pMethodDecl->getBody()));
        for (auto const pStmt : pCompoundStatement->body())
        {
            // note: this is not a CXXMemberCallExpr
            CallExpr const*const pCallExpr(dyn_cast<CallExpr>(pStmt));
            fprintf(stderr, "XXX pCallExpr %p\n", pCallExpr);
            if (pCallExpr)
            {
                // note: this is only sometimes a CXXMethodDecl
                FunctionDecl const*const pCalled(pCallExpr->getDirectCallee());
                if (pCalled->getName() == "release"
//this never works  && pCalled == pOverridden
                    && (pCalled->getParent() == pOverridden->getParent()
                        // allow this convenient shortcut
                        || pCalled->getQualifiedNameAsString() == "cppu::OWeakObject::release"
                        || pCalled->getQualifiedNameAsString() == "cppu::OWeakAggObject::release"))
                {
                    return true;
                }
                if (pCalled->getName() == "relase_ChildImpl") // FIXME remove this lunacy
                {
                    return true;
                }
            }
        }

        // whitelist
        auto const name(pMethodDecl->getParent()->getQualifiedNameAsString());
        if (   name == "cppu::OWeakAggObject" // conditional call
            || name == "cppu::WeakComponentImplHelperBase" // extra magic
            || name == "cppu::WeakAggComponentImplHelperBase" // extra magic
            || name == "DOM::CDOMImplementation" // a static oddity
            || name == "ucbhelper::ContentImplHelper" // FIXME what is that
            || name == "SwXTextFrame" // ambiguous, 2 parents
            || name == "SwXTextGraphicObject" // ambiguous, 2 parents
            || name == "SwXTextEmbeddedObject" // ambiguous, 2 parents
            || name == "SwXTextDocument" // ambiguous, ~4 parents
            || name == "SdStyleSheet" // same extra magic as WeakComponentImplHelperBase
            || name == "SdXImpressDocument" // same extra magic as WeakComponentImplHelperBase
           )
        {
            return true;
        }

        report(DiagnosticsEngine::Warning,
                "override of OWeakObject::release() does not call superclass release()",
                pMethodDecl->getLocation())
            << pMethodDecl->getSourceRange();

        return true;
    }

};

loplugin::Plugin::Registration<WeakObject> X("weakobject");

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
