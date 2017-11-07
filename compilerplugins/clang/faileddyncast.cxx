/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>

#include "clang/AST/Attr.h"
#include "clang/AST/CXXInheritance.h"

#include "plugin.hxx"

namespace {

// cf. Clang's clang::AST::CXXDynamicCastExpr::isAlwaysNull
// (lib/AST/ExprCXX.cpp):
bool isAlwaysNull(CXXDynamicCastExpr const * expr) {
  QualType SrcType = expr->getSubExpr()->getType();
  QualType DestType = expr->getType();

  if (const clang::PointerType *SrcPTy = SrcType->getAs<clang::PointerType>()) {
    SrcType = SrcPTy->getPointeeType();
#if 0
    DestType = DestType->castAs<PointerType>()->getPointeeType();
#else
    auto DstPTy = DestType->getAs<clang::PointerType>();
    if (!DstPTy)
      return false;
    DestType = DstPTy->getPointeeType();
#endif
  }

  if (DestType->isVoidType())
    return false;

#if 0
  const CXXRecordDecl *SrcRD =
    cast<CXXRecordDecl>(SrcType->castAs<RecordType>()->getDecl());
#else
  auto SrcRT = SrcType->getAs<RecordType>();
  if (!SrcRT)
    return false;
  const CXXRecordDecl *SrcRD = cast<CXXRecordDecl>(SrcRT->getDecl());
#endif

#if 0
  if (!SrcRD->hasAttr<FinalAttr>())
    return false;
#endif

#if 0
  const CXXRecordDecl *DestRD =
    cast<CXXRecordDecl>(DestType->castAs<RecordType>()->getDecl());
#else
  auto DestRT = DestType->getAs<RecordType>();
  if (!DestRT)
    return false;
  const CXXRecordDecl *DestRD = cast<CXXRecordDecl>(DestRT->getDecl());
#endif

#if 1
  if (!(SrcRD && DestRD))
    return false;

  if (DestRD->hasAttr<FinalAttr>()) {
    CXXBasePaths Paths(/*FindAmbiguities=*/false, /*RecordPaths=*/true,
                       /*DetectVirtual=*/false);
    if (DestRD->isDerivedFrom(SrcRD, Paths) &&
        std::all_of(Paths.begin(), Paths.end(),
                    [](CXXBasePath const & Path) {
                        return Path.Access != AS_public; }))
      return true;
  }

  if (!SrcRD->hasAttr<FinalAttr>())
    return false;
#endif

#if 0
  return !DestRD->isDerivedFrom(SrcRD);
#else
  return !(DestRD->isDerivedFrom(SrcRD)
           || SrcRD->isDerivedFrom(DestRD)
           || SrcRD == DestRD);
#endif
}

class FailedDynCast:
    public RecursiveASTVisitor<FailedDynCast>, public loplugin::Plugin
{
public:
    explicit FailedDynCast(loplugin::InstantiationData const & data):
        Plugin(data) {}

    bool shouldVisitTemplateInstantiations() const { return true; }

    void run() override;

    bool VisitCXXDynamicCastExpr(CXXDynamicCastExpr const * expr);
};

void FailedDynCast::run() {
    if (compiler.getLangOpts().CPlusPlus) {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }
}

bool FailedDynCast::VisitCXXDynamicCastExpr(CXXDynamicCastExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    if (isAlwaysNull(expr)) {
        report(
            DiagnosticsEngine::Warning,
            "dynamic_cast from %0 to %1 always fails", expr->getLocStart())
            << expr->getSubExpr()->getType() << expr->getType()
            << expr->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration<FailedDynCast> X("faileddyncast");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
