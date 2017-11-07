/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <limits>
#include <string>
#include "plugin.hxx"

//
// We don't like using C-style casts in C++ code
//

namespace {

bool areSimilar(QualType type1, QualType type2) {
    auto t1 = type1.getCanonicalType().getTypePtr();
    auto t2 = type2.getCanonicalType().getTypePtr();
    for (;;) {
        if (t1->isPointerType()) {
            if (!t2->isPointerType()) {
                return false;
            }
            auto t1a = t1->getAs<clang::PointerType>();
            auto t2a = t2->getAs<clang::PointerType>();
            t1 = t1a->getPointeeType().getTypePtr();
            t2 = t2a->getPointeeType().getTypePtr();
        } else if (t1->isMemberPointerType()) {
            if (!t2->isMemberPointerType()) {
                return false;
            }
            auto t1a = t1->getAs<MemberPointerType>();
            auto t2a = t2->getAs<MemberPointerType>();
            if (t1a->getClass()->getCanonicalTypeInternal()
                != t2a->getClass()->getCanonicalTypeInternal())
            {
                return false;
            }
            t1 = t1a->getPointeeType().getTypePtr();
            t2 = t2a->getPointeeType().getTypePtr();
        } else if (t1->isConstantArrayType()) {
            if (!t2->isConstantArrayType()) {
                return false;
            }
            auto t1a = static_cast<ConstantArrayType const *>(
                t1->getAsArrayTypeUnsafe());
            auto t2a = static_cast<ConstantArrayType const *>(
                t2->getAsArrayTypeUnsafe());
            if (t1a->getSize() != t2a->getSize()) {
                return false;
            }
            t1 = t1a->getElementType().getTypePtr();
            t2 = t2a->getElementType().getTypePtr();
        } else if (t1->isIncompleteArrayType()) {
            if (!t2->isIncompleteArrayType()) {
                return false;
            }
            auto t1a = static_cast<IncompleteArrayType const *>(
                t1->getAsArrayTypeUnsafe());
            auto t2a = static_cast<IncompleteArrayType const *>(
                t2->getAsArrayTypeUnsafe());
            t1 = t1a->getElementType().getTypePtr();
            t2 = t2a->getElementType().getTypePtr();
        } else {
            return false;
        }
        if (t1 == t2) {
            return true;
        }
    }
}

QualType resolvePointers(QualType type) {
    while (type->isPointerType()) {
        type = type->getAs<clang::PointerType>()->getPointeeType();
    }
    return type;
}

class CStyleCast:
    public RecursiveASTVisitor<CStyleCast>, public loplugin::Plugin
{
public:
    explicit CStyleCast(loplugin::InstantiationData const & data): Plugin(data)
    {}

    virtual void run() override {
        if (compiler.getLangOpts().CPlusPlus) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool TraverseLinkageSpecDecl(LinkageSpecDecl * decl);

    bool VisitCStyleCastExpr(const CStyleCastExpr * expr);

private:
    bool isConstCast(QualType from, QualType to);

    bool isFromCIncludeFile(SourceLocation spellingLocation) const;

    bool isSharedCAndCppCode(SourceLocation location) const;

    unsigned int externCContexts_ = 0;
};

const char * recommendedFix(clang::CastKind ck) {
    switch(ck) {
        case CK_IntegralToPointer: return "reinterpret_cast";
        case CK_PointerToIntegral: return "reinterpret_cast";
        case CK_BaseToDerived: return "static_cast";
        default: return nullptr;
    }
}

bool CStyleCast::TraverseLinkageSpecDecl(LinkageSpecDecl * decl) {
    assert(externCContexts_ != std::numeric_limits<unsigned int>::max()); //TODO
    ++externCContexts_;
    bool ret = RecursiveASTVisitor::TraverseLinkageSpecDecl(decl);
    assert(externCContexts_ != 0);
    --externCContexts_;
    return ret;
}

bool CStyleCast::VisitCStyleCastExpr(const CStyleCastExpr * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    // casting to void is typically used when a parameter or field is only used in
    // debug mode, and we want to eliminate an "unused" warning
    if( expr->getCastKind() == CK_ToVoid ) {
        return true;
    }
    // ignore integral-type conversions for now, there is insufficient agreement about
    // the merits of C++ style casting in this case
    if( expr->getCastKind() == CK_IntegralCast ) {
        return true;
    }
    char const * perf = nullptr;
    if( expr->getCastKind() == CK_NoOp ) {
        if (!((expr->getSubExpr()->getType()->isPointerType()
               && expr->getType()->isPointerType())
              || expr->getTypeAsWritten()->isReferenceType()))
        {
            return true;
        }
        if (isConstCast(
                expr->getSubExprAsWritten()->getType(),
                expr->getTypeAsWritten()))
        {
            perf = "const_cast";
        }
    }
    if (isSharedCAndCppCode(expr->getLocStart())) {
        return true;
    }
    std::string incompFrom;
    std::string incompTo;
    if( expr->getCastKind() == CK_BitCast ) {
        if (resolvePointers(expr->getSubExprAsWritten()->getType())
            ->isIncompleteType())
        {
            incompFrom = "incomplete ";
        }
        if (resolvePointers(expr->getType())->isIncompleteType()) {
            incompTo = "incomplete ";
        }
    }
    if (perf == nullptr) {
        perf = recommendedFix(expr->getCastKind());
    }
    std::string performs;
    if (perf != nullptr) {
        performs = std::string(" (performs: ") + perf + ")";
    }
    report(
        DiagnosticsEngine::Warning, "C-style cast from %0%1 to %2%3%4 (%5)",
        expr->getSourceRange().getBegin())
      << incompFrom << expr->getSubExprAsWritten()->getType()
      << incompTo << expr->getTypeAsWritten() << performs
      << expr->getCastKindName()
      << expr->getSourceRange();
    return true;
}

bool CStyleCast::isConstCast(QualType from, QualType to) {
    if (to->isReferenceType()
        && to->getAs<ReferenceType>()->getPointeeType()->isObjectType())
    {
        if (!from->isObjectType()) {
            return false;
        }
        from = compiler.getASTContext().getPointerType(from);
        to = compiler.getASTContext().getPointerType(
            to->getAs<ReferenceType>()->getPointeeType());
    } else {
        if (from->isArrayType()) {
            from = compiler.getASTContext().getPointerType(
                from->getAsArrayTypeUnsafe()->getElementType());
        } else if (from->isFunctionType()) {
            compiler.getASTContext().getPointerType(from);
        }
    }
    return areSimilar(from, to);
}

bool CStyleCast::isFromCIncludeFile(SourceLocation spellingLocation) const {
    return !compiler.getSourceManager().isInMainFile(spellingLocation)
        && (StringRef(
                compiler.getSourceManager().getPresumedLoc(spellingLocation)
                .getFilename())
            .endswith(".h"));
}

bool CStyleCast::isSharedCAndCppCode(SourceLocation location) const {
    while (compiler.getSourceManager().isMacroArgExpansion(location)) {
        location = compiler.getSourceManager().getImmediateMacroCallerLoc(
            location);
    }
    // Assume that code is intended to be shared between C and C++ if it comes
    // from an include file ending in .h, and is either in an extern "C" context
    // or the body of a macro definition:
    return
        isFromCIncludeFile(compiler.getSourceManager().getSpellingLoc(location))
        && (externCContexts_ != 0
            || compiler.getSourceManager().isMacroBodyExpansion(location));
}

loplugin::Plugin::Registration< CStyleCast > X("cstylecast");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
