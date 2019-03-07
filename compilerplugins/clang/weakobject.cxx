/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include "check.hxx"
#include "plugin.hxx"

/* OWeakObject::release() disposes weak references.  If that isn't done
 * because a sub-class improperly overrides release() then
 * OWeakConnectionPoint::m_pObject continues to point to the deleted object
 * and that could result in use-after-free.
 */

namespace {

class WeakObject
    : public loplugin::FilteringPlugin<WeakObject>
{

public:
    explicit WeakObject(loplugin::InstantiationData const& rData): FilteringPlugin(rData)
    {}

    virtual bool preRun() override {
        return compiler.getLangOpts().CPlusPlus; // no OWeakObject in C
    }
    void run() override {
        if( preRun()) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool isDerivedFromOWeakObject(CXXMethodDecl const*const pMethodDecl)
    {
        QualType const pClass(pMethodDecl->getParent()->getTypeForDecl(), 0);
        if (loplugin::TypeCheck(pClass).Class("OWeakObject").Namespace("cppu"))
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
        if (!pMethodDecl->isThisDeclarationADefinition()
            || pMethodDecl->isLateTemplateParsed())
        {
            return true;
        }
        if (!pMethodDecl->isInstance()) {
            return true;
        }
// this is too "simple", if a NamedDecl class has a getName() member expecting it to actually work would clearly be unreasonable    if (pMethodDecl->getName() != "release") {
        if (auto const i = pMethodDecl->getIdentifier()) {
            if (i != nullptr && !i->isStr("release")) {
                return true;
            }
        }
        if (pMethodDecl->getNumParams() != 0) {
            return true;
        }
        if (loplugin::TypeCheck(QualType(pMethodDecl->getParent()->getTypeForDecl(), 0)).Class("OWeakObject").Namespace("cppu"))
        {
            return true;
        }

        CXXMethodDecl const* pOverridden(nullptr);
        for (auto it = pMethodDecl->begin_overridden_methods();
             it != pMethodDecl->end_overridden_methods(); ++it)
        {
            if (auto const i = (*it)->getIdentifier()) {
                if (i != nullptr && i->isStr("release")) {
                    pOverridden = *it;
                    break;
                }
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
        for (auto i = pCompoundStatement->body_begin();
             i != pCompoundStatement->body_end(); ++i)
        {
            // note: this is not a CXXMemberCallExpr
            CallExpr const*const pCallExpr(dyn_cast<CallExpr>(*i));
            if (pCallExpr)
            {
                // note: this is only sometimes a CXXMethodDecl
                FunctionDecl const*const pCalled(pCallExpr->getDirectCallee());
                if (pCalled->getName() == "release")
                {
//this never works  && pCalled == pOverridden
                    if (pCalled->getParent() == pOverridden->getParent())
                    {
                        return true;
                    }
                    // Allow this convenient shortcut:
                    auto td = dyn_cast<TypeDecl>(pCalled->getParent());
                    if (td != nullptr
                        && (loplugin::TypeCheck(td).Class("OWeakObject").Namespace("cppu").GlobalNamespace()
                            || loplugin::TypeCheck(td).Class("OWeakAggObject").Namespace("cppu").GlobalNamespace()))
                    {
                        return true;
                    }
                }
            }
        }

        // whitelist
        auto tc  = loplugin::TypeCheck(pMethodDecl->getParent());
        if (   tc.Class("OWeakAggObject").Namespace("cppu").GlobalNamespace() // conditional call
            || tc.Class("WeakComponentImplHelperBase").Namespace("cppu").GlobalNamespace() // extra magic
            || tc.Class("WeakAggComponentImplHelperBase").Namespace("cppu").GlobalNamespace() // extra magic
            || tc.Class("CDOMImplementation").Namespace("DOM").GlobalNamespace() // a static oddity
            || tc.Class("SwXTextFrame").GlobalNamespace() // ambiguous, 3 parents
            || tc.Class("SwXTextDocument").GlobalNamespace() // ambiguous, ~4 parents
            || tc.Class("SdStyleSheet").GlobalNamespace() // same extra magic as WeakComponentImplHelperBase
            || tc.Class("SdXImpressDocument").GlobalNamespace() // same extra magic as WeakComponentImplHelperBase
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

loplugin::Plugin::Registration<WeakObject> weakobject("weakobject");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
