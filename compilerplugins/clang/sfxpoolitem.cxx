/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include <string>
#include <iostream>

#include "plugin.hxx"
#include "check.hxx"
#include "clang/AST/CXXInheritance.h"

/*
what might be more interesting is a warning about subclasses that add
members but do not override virtual operator==() - that is easily
forgotten and hard to notice.
*/
namespace {

class SfxPoolItem:
    public loplugin::FilteringPlugin<SfxPoolItem>
{
public:
    explicit SfxPoolItem(loplugin::InstantiationData const & data): FilteringPlugin(data)
    {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCXXRecordDecl( const CXXRecordDecl* );
};

bool BaseCheckNotSfxPoolItemSubclass(const CXXRecordDecl *BaseDefinition) {
    if (BaseDefinition && loplugin::TypeCheck(BaseDefinition).Class("SfxPoolItem").GlobalNamespace()) {
        return false;
    }
    return true;
}

bool isDerivedFromSfxPoolItem(const CXXRecordDecl *decl) {
    if (!decl)
        return false;
    if (loplugin::TypeCheck(decl).Class("SfxPoolItem").GlobalNamespace())
        return true;
    if (!decl->hasDefinition()) {
        return false;
    }
    if (// not sure what hasAnyDependentBases() does,
        // but it avoids classes we don't want, e.g. WeakAggComponentImplHelper1
        !decl->hasAnyDependentBases() &&
        !decl->forallBases(BaseCheckNotSfxPoolItemSubclass)) {
        return true;
    }
    return false;
}


bool BaseCheckNotSwMsgPoolItemSubclass(const CXXRecordDecl *BaseDefinition) {
    if (BaseDefinition && loplugin::TypeCheck(BaseDefinition).Class("SwMsgPoolItem")) {
        return false;
    }
    return true;
}

bool isDerivedFromSwMsgPoolItem(const CXXRecordDecl *decl) {
    if (!decl)
        return false;
    if (loplugin::TypeCheck(decl).Class("SwMsgPoolItem").GlobalNamespace())
        return true;
    if (!decl->hasDefinition()) {
        return false;
    }
    if (// not sure what hasAnyDependentBases() does,
        // but it avoids classes we don't want, e.g. WeakAggComponentImplHelper1
        !decl->hasAnyDependentBases() &&
        !decl->forallBases(BaseCheckNotSwMsgPoolItemSubclass)) {
        return true;
    }
    return false;
}

bool endsWith(const std::string& a, const std::string& b) {
    if (b.size() > a.size()) return false;
    return std::equal(a.begin() + a.size() - b.size(), a.end(), b.begin());
}

bool SfxPoolItem::VisitCXXRecordDecl(const CXXRecordDecl* decl)
{
    if (ignoreLocation(decl)) {
       return true;
    }
    if (!decl->hasDefinition()) {
       return true;
    }
    // check if this class is derived from Window
    if (!isDerivedFromSfxPoolItem(decl)) {
        return true;
    }
    // the SwMsgPoolItem are some sort of hack to transport down-castable objects to SwClient::Modify(), they're not "real" items
    if (isDerivedFromSwMsgPoolItem(decl)) {
        return true;
    }
    if (decl->field_begin() == decl->field_end()) {
        return true;
    }
    // the enum types do some weird stuff involving SfxEnumItemInterface
    auto tc = loplugin::TypeCheck(decl);
    if (tc.Class("SfxEnumItem").GlobalNamespace() || tc.Class("SfxAllEnumItem").GlobalNamespace())
        return true;

    for (auto it = decl->method_begin(); it != decl->method_end(); ++it) {
        if ( endsWith((*it)->getQualifiedNameAsString(), "::operator==") )
            return true;
    }
    report(
            DiagnosticsEngine::Warning,
            "SfxPoolItem subclass %0 declares new fields, but does not override operator==",
            compat::getBeginLoc(decl))
        << decl->getQualifiedNameAsString() << decl->getSourceRange();
    return true;
}


loplugin::Plugin::Registration< SfxPoolItem > sfxpoolitem("sfxpoolitem");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
