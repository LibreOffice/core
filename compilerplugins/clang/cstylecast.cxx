/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <string>
#include "plugin.hxx"
#include "compat.hxx"

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
            auto t1a = t1->getAs<PointerType>();
            auto t2a = t2->getAs<PointerType>();
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

bool hasCLanguageLinkageType(FunctionDecl const * decl) {
    return decl->isExternC() || compat::isInExternCContext(*decl);
}

QualType resolvePointers(QualType type) {
    while (type->isPointerType()) {
        type = type->getAs<PointerType>()->getPointeeType();
    }
    return type;
}

class CStyleCast:
    public RecursiveASTVisitor<CStyleCast>, public loplugin::Plugin
{
public:
    explicit CStyleCast(InstantiationData const & data):
        Plugin(data), externCFunction(false)
    {}

    virtual void run() override {
        if (compiler.getLangOpts().CPlusPlus) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool TraverseFunctionDecl(FunctionDecl * decl);

    bool VisitCStyleCastExpr(const CStyleCastExpr * expr);

private:
    bool isConstCast(QualType from, QualType to);

    bool externCFunction;
};

static const char * recommendedFix(clang::CastKind ck) {
    switch(ck) {
        case CK_IntegralToPointer: return "reinterpret_cast";
        case CK_PointerToIntegral: return "reinterpret_cast";
        case CK_BaseToDerived: return "static_cast";
        default: return nullptr;
    }
}

bool CStyleCast::TraverseFunctionDecl(FunctionDecl * decl) {
    bool ext = hasCLanguageLinkageType(decl)
        && decl->isThisDeclarationADefinition();
    if (ext) {
        assert(!externCFunction);
        externCFunction = true;
    }
    bool ret = RecursiveASTVisitor::TraverseFunctionDecl(decl);
    if (ext) {
        externCFunction = false;
    }
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
        QualType t1 = expr->getSubExpr()->getType();
        QualType t2 = expr->getType();
        if (t1->isPointerType() && t2->isPointerType()) {
            t1 = t1->getAs<PointerType>()->getPointeeType();
            t2 = t2->getAs<PointerType>()->getPointeeType();
        } else if (t1->isLValueReferenceType() && t2->isLValueReferenceType()) {
            t1 = t1->getAs<LValueReferenceType>()->getPointeeType();
            t2 = t2->getAs<LValueReferenceType>()->getPointeeType();
        } else {
            return true;
        }
        if (isConstCast(
                expr->getSubExprAsWritten()->getType(),
                expr->getTypeAsWritten()))
        {
            perf = "const_cast";
        }
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
    if (externCFunction || expr->getLocStart().isMacroID()) {
        SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(
            expr->getLocStart());
        StringRef filename = compiler.getSourceManager().getFilename(spellingLocation);
        // ignore C code
        if ( filename.endswith(".h") ) {
            return true;
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
      << incompTo << expr->getType() << performs
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

loplugin::Plugin::Registration< CStyleCast > X("cstylecast");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
