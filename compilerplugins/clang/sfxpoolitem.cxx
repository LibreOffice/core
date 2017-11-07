/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <iostream>

#include "plugin.hxx"
#include "compat.hxx"
#include "check.hxx"
#include "clang/AST/CXXInheritance.h"

/*
what might be more interesting is a warning about subclasses that add
members but do not override virtual operator==() - that is easily
forgotten and hard to notice.
*/
namespace {

class SfxPoolItem:
    public RecursiveASTVisitor<SfxPoolItem>, public loplugin::Plugin
{
public:
    explicit SfxPoolItem(loplugin::InstantiationData const & data): Plugin(data)
    {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCXXRecordDecl( const CXXRecordDecl* );
};

bool BaseCheckNotSfxPoolItemSubclass(
    const CXXRecordDecl *BaseDefinition
#if CLANG_VERSION < 30800
    , void *
#endif
    )
{
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
        !compat::forallBases(*decl, BaseCheckNotSfxPoolItemSubclass, nullptr, true)) {
        return true;
    }
    return false;
}


bool BaseCheckNotSwMsgPoolItemSubclass(
    const CXXRecordDecl *BaseDefinition
#if CLANG_VERSION < 30800
    , void *
#endif
    )
{
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
        !compat::forallBases(*decl, BaseCheckNotSwMsgPoolItemSubclass, nullptr, true)) {
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

    // the new field is only used for reading and writing to storage
    if (tc.Class("SvxCharSetColorItem").GlobalNamespace())
        return true;

    for (auto it = decl->method_begin(); it != decl->method_end(); ++it) {
        if ( endsWith((*it)->getQualifiedNameAsString(), "::operator==") )
            return true;
    }
    report(
            DiagnosticsEngine::Warning,
            "SfxPoolItem subclass %0 declares new fields, but does not override operator==",
            decl->getLocStart())
        << decl->getQualifiedNameAsString() << decl->getSourceRange();
    return true;
}


loplugin::Plugin::Registration< SfxPoolItem > X("sfxpoolitem");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
