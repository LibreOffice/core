/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <cassert>
#include <limits>
#include <set>
#include <string>

#include "compat.hxx"
#include "plugin.hxx"

//
// We don't like using C-style casts in C++ code.  Similarly, warn about function-style casts (which
// are semantically equivalent to C-style casts) that are not semantically equivalent to static_cast
// and should rather be written as const_cast or reinterpret_cast.
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
            if (compat::getClass(t1a)->getCanonicalTypeInternal()
                != compat::getClass(t2a)->getCanonicalTypeInternal())
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

bool isLiteralLike(Expr const * expr) {
    expr = expr->IgnoreParenImpCasts();
    if (isa<IntegerLiteral>(expr) || isa<CharacterLiteral>(expr) || isa<FloatingLiteral>(expr)
        || isa<ImaginaryLiteral>(expr) || isa<CXXBoolLiteralExpr>(expr)
        || isa<CXXNullPtrLiteralExpr>(expr) || isa<ObjCBoolLiteralExpr>(expr))
    {
        return true;
    }
    if (auto const e = dyn_cast<DeclRefExpr>(expr)) {
        auto const d = e->getDecl();
        if (isa<EnumConstantDecl>(d)) {
            return true;
        }
        if (auto const v = dyn_cast<VarDecl>(d)) {
            if (d->getType().isConstQualified()) {
                if (auto const init = v->getAnyInitializer()) {
                    return isLiteralLike(init);
                }
            }
        }
        return false;
    }
    if (auto const e = dyn_cast<UnaryExprOrTypeTraitExpr>(expr)) {
        auto const k = e->getKind();
        return k == UETT_SizeOf || k == UETT_AlignOf;
    }
    if (auto const e = dyn_cast<UnaryOperator>(expr)) {
        auto const k = e->getOpcode();
        if (k == UO_Plus || k == UO_Minus || k == UO_Not || k == UO_LNot) {
            return isLiteralLike(e->getSubExpr());
        }
        return false;
    }
    if (auto const e = dyn_cast<BinaryOperator>(expr)) {
        auto const k = e->getOpcode();
        if (k == BO_Mul || k == BO_Div || k == BO_Rem || k == BO_Add || k == BO_Sub || k == BO_Shl
            || k == BO_Shr || k == BO_And || k == BO_Xor || k == BO_Or)
        {
            return isLiteralLike(e->getLHS()) && isLiteralLike(e->getRHS());
        }
        return false;
    }
    if (auto const e = dyn_cast<ExplicitCastExpr>(expr)) {
        auto const t = e->getTypeAsWritten();
        return (t->isArithmeticType() || t->isEnumeralType())
            && isLiteralLike(e->getSubExprAsWritten());
    }
    return false;
}

bool canBeUsedForFunctionalCast(TypeSourceInfo const * info) {
    // Must be <simple-type-specifier> or <typename-specifier>, let's approximate that here:
    assert(info != nullptr);
    auto const type = info->getType();
    if (type.hasLocalQualifiers()) {
        return false;
    }
    if (auto const t = dyn_cast<BuiltinType>(type)) {
        if (!(t->isInteger() || t->isFloatingPoint())) {
            return false;
        }
        auto const loc = info->getTypeLoc().castAs<BuiltinTypeLoc>();
        return
            (int(loc.hasWrittenSignSpec()) + int(loc.hasWrittenWidthSpec())
             + int(loc.hasWrittenTypeSpec()))
            == 1;
    }
    if (isa<TagType>(type) || isa<TemplateTypeParmType>(type) || isa<AutoType>(type)
        || isa<DecltypeType>(type) || isa<TypedefType>(type))
    {
        return true;
    }
    if (auto const t = dyn_cast<ElaboratedType>(type)) {
        return t->getKeyword() == compat::ElaboratedTypeKeyword::None;
    }
    return false;
}

class CStyleCast:
    public loplugin::FilteringRewritePlugin<CStyleCast>
{
public:
    explicit CStyleCast(loplugin::InstantiationData const & data): FilteringRewritePlugin(data)
    {}

    virtual void run() override {
        if (compiler.getLangOpts().CPlusPlus) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool TraverseInitListExpr(InitListExpr * expr, DataRecursionQueue * queue = nullptr) {
        return WalkUpFromInitListExpr(expr)
            && TraverseSynOrSemInitListExpr(
                expr->isSemanticForm() ? expr : expr->getSemanticForm(), queue);
    }

    bool TraverseLinkageSpecDecl(LinkageSpecDecl * decl);

    bool VisitCStyleCastExpr(const CStyleCastExpr * expr);

    bool VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr const * expr);

private:
    bool isConstCast(QualType from, QualType to);

    bool isFromCIncludeFile(SourceLocation spellingLocation) const;

    bool isSharedCAndCppCode(SourceLocation location) const;

    bool isLastTokenOfImmediateMacroBodyExpansion(
        SourceLocation loc, SourceLocation * macroEnd = nullptr) const;

    bool rewriteArithmeticCast(CStyleCastExpr const * expr, char const ** replacement);

    void reportCast(ExplicitCastExpr const * expr, char const * performsHint);

    unsigned int externCContexts_ = 0;
    std::set<SourceLocation> rewritten_;
        // needed when rewriting in macros, in general to avoid "double code replacement, possible
        // plugin error" warnings, and in particular to avoid adding multiple sets of parens around
        // sub-exprs
    std::set<CStyleCastExpr const *> rewrittenSubExprs_;
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
    if (isSharedCAndCppCode(expr->getBeginLoc())) {
        return true;
    }
    char const * perf = nullptr;
    if( expr->getCastKind() == CK_IntegralCast ) {
        if (rewriteArithmeticCast(expr, &perf)) {
            return true;
        }
    } else if( expr->getCastKind() == CK_NoOp ) {
        if (!((expr->getSubExpr()->getType()->isPointerType()
               && expr->getType()->isPointerType())
              || expr->getTypeAsWritten()->isReferenceType()))
        {
            if (rewriteArithmeticCast(expr, &perf)) {
                return true;
            }
        }
        if (isConstCast(
                expr->getSubExprAsWritten()->getType(),
                expr->getTypeAsWritten()))
        {
            perf = "const_cast";
        }
    }
    reportCast(expr, perf);
    return true;
}

bool CStyleCast::VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    char const * perf = nullptr;
    switch (expr->getCastKind()) {
    case CK_ConstructorConversion:
    case CK_Dependent: //TODO: really filter out all of these?
    case CK_IntegralCast:
    case CK_IntegralToBoolean:
    case CK_ToVoid:
        return true;
    case CK_NoOp:
        if (isConstCast(
                expr->getSubExprAsWritten()->getType(),
                expr->getTypeAsWritten()))
        {
            perf = "const_cast";
            break;
        }
        return true; //TODO: really filter out all of these?
    default:
        break;
    }
    reportCast(expr, perf);
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
        && compat::ends_with(
            StringRef(compiler.getSourceManager().getPresumedLoc(spellingLocation).getFilename()),
            ".h");
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

bool CStyleCast::isLastTokenOfImmediateMacroBodyExpansion(
    SourceLocation loc, SourceLocation * macroEnd) const
{
    assert(compiler.getSourceManager().isMacroBodyExpansion(loc));
    auto const spell = compiler.getSourceManager().getSpellingLoc(loc);
    auto name = Lexer::getImmediateMacroName(
        loc, compiler.getSourceManager(), compiler.getLangOpts());
    while (compat::starts_with(name, "\\\n")) {
        name = name.drop_front(2);
        while (!name.empty()
               && (name.front() == ' ' || name.front() == '\t' || name.front() == '\n'
                   || name.front() == '\v' || name.front() == '\f'))
        {
            name = name.drop_front(1);
        }
    }
    auto const MI
        = (compiler.getPreprocessor().getMacroDefinitionAtLoc(
               &compiler.getASTContext().Idents.get(name), spell)
           .getMacroInfo());
    assert(MI != nullptr);
    if (spell == MI->getDefinitionEndLoc()) {
        if (macroEnd != nullptr) {
            *macroEnd = compat::getImmediateExpansionRange(compiler.getSourceManager(), loc).second;
        }
        return true;
    }
    return false;
}

bool CStyleCast::rewriteArithmeticCast(CStyleCastExpr const * expr, char const ** replacement) {
    assert(replacement != nullptr);
    auto const sub = expr->getSubExprAsWritten();
    auto const functional = isLiteralLike(sub)
        && canBeUsedForFunctionalCast(expr->getTypeInfoAsWritten());
    *replacement = functional ? "functional cast" : "static_cast";
    if (rewriter == nullptr) {
        return false;
    }
    // Doing modifications for a chain of C-style casts as in
    //
    //  (foo)(bar)(baz)x
    //
    // leads to unpredictable results, so only rewrite them one at a time, starting with the
    // outermost:
    if (auto const e = dyn_cast<CStyleCastExpr>(sub)) {
        rewrittenSubExprs_.insert(e);
    }
    if (rewrittenSubExprs_.find(expr) != rewrittenSubExprs_.end()) {
        return false;
    }
    // Two or four ranges to replace:
    // First is the CStyleCast's LParen, plus following whitespace, replaced with either "" or
    // "static_cast<".  (TODO: insert space before "static_cast<" when converting "else(int)...".)
    // Second is the CStyleCast's RParen, plus preceding and following whitespace, replaced with
    // either "" or ">".
    // If the sub expr is not a ParenExpr, third is the sub expr's begin, inserting "(", and fourth
    // is the sub expr's end, inserting ")".
    // (The reason the second and third are not combined is in case there's a comment between them.)
    auto firstBegin = expr->getLParenLoc();
    auto secondBegin = expr->getRParenLoc();
    while (compiler.getSourceManager().isMacroArgExpansion(firstBegin)
           && compiler.getSourceManager().isMacroArgExpansion(secondBegin)
           && (compat::getImmediateExpansionRange(compiler.getSourceManager(), firstBegin)
               == compat::getImmediateExpansionRange(compiler.getSourceManager(), secondBegin)))
    {
        firstBegin = compiler.getSourceManager().getImmediateSpellingLoc(firstBegin);
        secondBegin = compiler.getSourceManager().getImmediateSpellingLoc(secondBegin);
    }
    if (compiler.getSourceManager().isMacroBodyExpansion(firstBegin)
        && compiler.getSourceManager().isMacroBodyExpansion(secondBegin)
        && (compiler.getSourceManager().getImmediateMacroCallerLoc(firstBegin)
            == compiler.getSourceManager().getImmediateMacroCallerLoc(secondBegin)))
    {
        firstBegin = compiler.getSourceManager().getSpellingLoc(firstBegin);
        secondBegin = compiler.getSourceManager().getSpellingLoc(secondBegin);
    }
    auto third = sub->getBeginLoc();
    auto fourth = sub->getEndLoc();
    bool macro = false;
    // Ensure that
    //
    //  #define FOO(x) (int)x
    //  FOO(y)
    //
    // is changed to
    //
    //  #define FOO(x) static_cast<int>(x)
    //  FOO(y)
    //
    // instead of
    //
    //  #define FOO(x) static_cast<int>x
    //  FOO((y))
    while (compiler.getSourceManager().isMacroArgExpansion(third)
           && compiler.getSourceManager().isMacroArgExpansion(fourth)
           && (compat::getImmediateExpansionRange(compiler.getSourceManager(), third)
               == compat::getImmediateExpansionRange(compiler.getSourceManager(), fourth))
           && compiler.getSourceManager().isAtStartOfImmediateMacroExpansion(third))
            //TODO: check fourth is at end of immediate macro expansion, but
            // SourceManager::isAtEndOfImmediateMacroExpansion requires a location pointing at the
            // character end of the last token
    {
        auto const range = compat::getImmediateExpansionRange(compiler.getSourceManager(), third);
        third = range.first;
        fourth = range.second;
        macro = true;
        assert(third.isValid());
    }
    while (compiler.getSourceManager().isMacroArgExpansion(third)
           && compiler.getSourceManager().isMacroArgExpansion(fourth)
           && (compat::getImmediateExpansionRange(compiler.getSourceManager(), third)
               == compat::getImmediateExpansionRange(compiler.getSourceManager(), fourth)))
    {
        third = compiler.getSourceManager().getImmediateSpellingLoc(third);
        fourth = compiler.getSourceManager().getImmediateSpellingLoc(fourth);
    }
    if (isa<ParenExpr>(sub)) {
        // Ensure that with
        //
        //  #define FOO (x)
        //
        // a cast like
        //
        //  (int) FOO
        //
        // is changed to
        //
        //  static_cast<int>(FOO)
        //
        // instead of
        //
        //  static_cast<int>FOO
        for (;; macro = true) {
            if (!(compiler.getSourceManager().isMacroBodyExpansion(third)
                  && compiler.getSourceManager().isMacroBodyExpansion(fourth)
                  && (compiler.getSourceManager().getImmediateMacroCallerLoc(third)
                      == compiler.getSourceManager().getImmediateMacroCallerLoc(fourth))
                  && compiler.getSourceManager().isAtStartOfImmediateMacroExpansion(third)
                  && isLastTokenOfImmediateMacroBodyExpansion(fourth)))
            {
                if (!macro) {
                    third = fourth = SourceLocation();
                }
                break;
            }
            auto const range = compat::getImmediateExpansionRange(
                compiler.getSourceManager(), third);
            third = range.first;
            fourth = range.second;
            assert(third.isValid());
        }
        if (third.isValid() && compiler.getSourceManager().isMacroBodyExpansion(third)
            && compiler.getSourceManager().isMacroBodyExpansion(fourth)
            && (compiler.getSourceManager().getImmediateMacroCallerLoc(third)
                == compiler.getSourceManager().getImmediateMacroCallerLoc(fourth)))
        {
            third = compiler.getSourceManager().getSpellingLoc(third);
            fourth = compiler.getSourceManager().getSpellingLoc(fourth);
            assert(third.isValid());
        }
    } else {
        // Ensure that a cast like
        //
        //  (int)LONG_MAX
        //
        // (where LONG_MAX expands to __LONG_MAX__, which in turn is a built-in expanding to a value
        // like 9223372036854775807L) is changed to
        //
        //  int(LONG_MAX)
        //
        // instead of trying to add the parentheses to the built-in __LONG_MAX__ definition:
        for (;;) {
            if (!(compiler.getSourceManager().isMacroBodyExpansion(third)
                  && compiler.getSourceManager().isMacroBodyExpansion(fourth)
                  && (compiler.getSourceManager().getImmediateMacroCallerLoc(third)
                      == compiler.getSourceManager().getImmediateMacroCallerLoc(fourth))
                  && compiler.getSourceManager().isAtStartOfImmediateMacroExpansion(third)))
                // TODO: check that fourth is at end of immediate macro expansion (but
                // SourceManager::isAtEndOfImmediateMacroExpansion wants a location pointing at the
                // character end)
            {
                break;
            }
            auto const range = compat::getImmediateExpansionRange(
                compiler.getSourceManager(), third);
            third = range.first;
            fourth = range.second;
        }
        // ...and additionally asymmetrically unwind macros only at the start or end, for code like
        //
        //  (long)ubidi_getVisualIndex(...)
        //
        // (in editeng/source/editeng/impedit2.cxx) where ubidi_getVisualIndex is an object-like
        // macro, or
        //
        //  #define YY_SC_TO_UI(c) ((unsigned int) (unsigned char) c)
        //
        // (in hwpfilter/source/lexer.cxx):
        if (!fourth.isMacroID()) {
            while (compiler.getSourceManager().isMacroBodyExpansion(third)
                   && compiler.getSourceManager().isAtStartOfImmediateMacroExpansion(third, &third))
            {}
        } else if (compiler.getSourceManager().isMacroBodyExpansion(fourth)) {
            while (compiler.getSourceManager().isMacroArgExpansion(third)
                   && compiler.getSourceManager().isAtStartOfImmediateMacroExpansion(third, &third)) {}
        }
        if (!third.isMacroID()) {
            while (compiler.getSourceManager().isMacroBodyExpansion(fourth)
                   && isLastTokenOfImmediateMacroBodyExpansion(fourth, &fourth))
            {}
        } else if (compiler.getSourceManager().isMacroBodyExpansion(third)) {
            while (compiler.getSourceManager().isMacroArgExpansion(fourth, &fourth)) {}
        }
        if (compiler.getSourceManager().isMacroBodyExpansion(third)
            && compiler.getSourceManager().isMacroBodyExpansion(fourth)
            && (compiler.getSourceManager().getImmediateMacroCallerLoc(third)
                == compiler.getSourceManager().getImmediateMacroCallerLoc(fourth)))
        {
            third = compiler.getSourceManager().getSpellingLoc(third);
            fourth = compiler.getSourceManager().getSpellingLoc(fourth);
        }
        assert(third.isValid());
    }
    if (firstBegin.isMacroID() || secondBegin.isMacroID() || (third.isValid() && third.isMacroID())
        || (fourth.isValid() && fourth.isMacroID()))
    {
        if (isDebugMode()) {
            report(
                DiagnosticsEngine::Fatal,
                "TODO: cannot rewrite C-style cast in macro, needs investigation",
                expr->getExprLoc())
                << expr->getSourceRange();
        }
        return false;
    }
    unsigned firstLen = Lexer::MeasureTokenLength(
        firstBegin, compiler.getSourceManager(), compiler.getLangOpts());
    for (auto l = firstBegin.getLocWithOffset(std::max<unsigned>(firstLen, 1));;
         l = l.getLocWithOffset(1))
    {
        unsigned n = Lexer::MeasureTokenLength(
            l, compiler.getSourceManager(), compiler.getLangOpts());
        if (n != 0) {
            break;
        }
        ++firstLen;
    }
    unsigned secondLen = Lexer::MeasureTokenLength(
        secondBegin, compiler.getSourceManager(), compiler.getLangOpts());
    for (auto l = secondBegin.getLocWithOffset(std::max<unsigned>(secondLen, 1));;
         l = l.getLocWithOffset(1))
    {
        unsigned n = Lexer::MeasureTokenLength(
            l, compiler.getSourceManager(), compiler.getLangOpts());
        if (n != 0) {
            break;
        }
        ++secondLen;
    }
    for (;;) {
        auto l = secondBegin.getLocWithOffset(-1);
        auto const c = compiler.getSourceManager().getCharacterData(l)[0];
        if (c == '\n') {
            if (compiler.getSourceManager().getCharacterData(l.getLocWithOffset(-1))[0] == '\\') {
                break;
            }
        } else if (!(c == ' ' || c == '\t' || c == '\v' || c == '\f')) {
            break;
        }
        secondBegin = l;
        ++secondLen;
    }
    if (rewritten_.insert(firstBegin).second) {
        if (!replaceText(firstBegin, firstLen, functional ? "" : "static_cast<")) {
            if (isDebugMode()) {
                report(
                    DiagnosticsEngine::Fatal, "TODO: cannot rewrite #1, needs investigation",
                    firstBegin);
                report(
                    DiagnosticsEngine::Note, "when rewriting this C-style cast", expr->getExprLoc())
                    << expr->getSourceRange();
            }
            return false;
        }
        if (!replaceText(secondBegin, secondLen, functional ? "" : ">")) {
            //TODO: roll back
            if (isDebugMode()) {
                report(
                    DiagnosticsEngine::Fatal, "TODO: cannot rewrite #2, needs investigation",
                    secondBegin);
                report(
                    DiagnosticsEngine::Note, "when rewriting this C-style cast", expr->getExprLoc())
                    << expr->getSourceRange();
            }
            return false;
        }
    }
    if (third.isValid()) {
        if (rewritten_.insert(third).second) {
            if (!insertTextBefore(third, "(")) {
                //TODO: roll back
                if (isDebugMode()) {
                    report(
                        DiagnosticsEngine::Fatal, "TODO: cannot rewrite #3, needs investigation",
                        third);
                    report(
                        DiagnosticsEngine::Note, "when rewriting this C-style cast",
                        expr->getExprLoc())
                        << expr->getSourceRange();
                }
                return false;
            }
            if (!insertTextAfterToken(fourth, ")")) {
                //TODO: roll back
                if (isDebugMode()) {
                    report(
                        DiagnosticsEngine::Fatal, "TODO: cannot rewrite #4, needs investigation",
                        third);
                    report(
                        DiagnosticsEngine::Note, "when rewriting this C-style cast",
                        expr->getExprLoc())
                        << expr->getSourceRange();
                }
                return false;
            }
        }
    }
    return true;
}

void CStyleCast::reportCast(ExplicitCastExpr const * expr, char const * performsHint) {
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
    if (performsHint == nullptr) {
        performsHint = recommendedFix(expr->getCastKind());
    }
    std::string performs;
    if (performsHint != nullptr) {
        performs = std::string(" (performs: ") + performsHint + ")";
    }
    report(
        DiagnosticsEngine::Warning, "%select{C|Function}0-style cast from %1%2 to %3%4%5 (%6)",
        expr->getSourceRange().getBegin())
      << isa<CXXFunctionalCastExpr>(expr)
      << incompFrom << expr->getSubExprAsWritten()->getType()
      << incompTo << expr->getTypeAsWritten() << performs
      << expr->getCastKindName()
      << expr->getSourceRange();
}

loplugin::Plugin::Registration< CStyleCast > X("cstylecast", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
