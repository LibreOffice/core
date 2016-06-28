#include<iostream>
/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stack>

#include "plugin.hxx"

namespace {

class AnonEnum: public RecursiveASTVisitor<AnonEnum>, public loplugin::Plugin {
public:
    explicit AnonEnum(InstantiationData const & data): Plugin(data) {}

    void run() override;

    bool TraverseTypedefDecl(TypedefDecl * decl);

    bool VisitEnumDecl(EnumDecl const * decl);

private:
    std::stack<QualType> typedefs_;
};

void AnonEnum::run() {
    if (compiler.getLangOpts().CPlusPlus) {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }
}

bool AnonEnum::TraverseTypedefDecl(TypedefDecl * decl) {
//std::cerr<<"Enter\n";
bool res = RecursiveASTVisitor::TraverseTypedefDecl(decl);
//std::cerr<<"Leave\n";
#if 0
    if (!WalkUpFromTypedefDecl(decl)) {
        return false;
    }
    auto info = decl->getTypeSourceInfo();
    typedefs_.push(info->getType());
    bool res = TraverseTypeLoc(info->getTypeLoc());
    typedefs_.pop();
#endif
    return res;
}

bool AnonEnum::VisitEnumDecl(EnumDecl const * decl) {
    if (ignoreLocation(decl)) {
        return true;
    }
    if (!typedefs_.empty()
        && compiler.getASTContext().getTagDeclType(decl) == typedefs_.top())
    {
        return true;
    }
if (!typedefs_.empty()){typedefs_.top().dump();compiler.getASTContext().getTagDeclType(decl).dump();}
    report(DiagnosticsEngine::Warning, "TODO", decl->getLocation())
        << decl->getSourceRange();
std::abort();
    return true;
}

loplugin::Plugin::Registration<AnonEnum> X("anonenum",false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
