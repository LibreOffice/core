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
#include <map>
#include <string>

#include "clang/AST/Attr.h"
#include "clang/Basic/Builtins.h"

#include "check.hxx"
#include "compat.hxx"
#include "functionaddress.hxx"
#include "plugin.hxx"

namespace {

enum FakeBoolKind {
    FBK_No,
    FBK_BOOL, FBK_First = FBK_BOOL,
    FBK_Boolean, FBK_FT_Bool, FBK_FcBool, FBK_GLboolean, FBK_NPBool, FBK_TW_BOOL, FBK_UBool,
    FBK_boolean, FBK_dbus_bool_t, FBK_gboolean, FBK_hb_boot_t, FBK_jboolean, FBK_my_bool,
    FBK_sal_Bool,
    FBK_End };
    // matches loplugin::TypeCheck::AnyBoolean (compilerplugins/clang/check.hxx)

StringRef getName(FakeBoolKind k) {
    static constexpr llvm::StringLiteral names[] = {
        "BOOL", "Boolean", "FT_Bool", "FcBool", "GLboolean", "NPBool", "TW_BOOL", "UBool",
        "boolean", "dbus_bool_t", "gboolean", "hb_boot_t", "jboolean", "my_bool", "sal_Bool"};
    assert(k >= FBK_First && k < FBK_End);
    return names[k - FBK_First];
}

FakeBoolKind isFakeBool(QualType type) {
    TypedefType const * t = type->getAs<TypedefType>();
    if (t != nullptr) {
        auto const name = t->getDecl()->getName();
        for (int i = FBK_First; i != FBK_End; ++i) {
            auto const k = FakeBoolKind(i);
            if (name == getName(k)) {
                return k;
            }
        }
    }
    return FBK_No;
}

FakeBoolKind isFakeBoolArray(QualType type) {
    auto t = type->getAsArrayTypeUnsafe();
    if (t == nullptr) {
        return FBK_No;
    }
    auto const k = isFakeBool(t->getElementType());
    if (k != FBK_No) {
        return k;
    }
    return isFakeBoolArray(t->getElementType());
}

// It appears that, given a function declaration, there is no way to determine
// the language linkage of the function's type, only of the function's name
// (via FunctionDecl::isExternC); however, in a case like
//
//   extern "C" { static void f(); }
//
// the function's name does not have C language linkage while the function's
// type does (as clarified in C++11 [decl.link]); cf. <http://clang-developers.
// 42468.n3.nabble.com/Language-linkage-of-function-type-tt4037248.html>
// "Language linkage of function type":
bool hasCLanguageLinkageType(FunctionDecl const * decl) {
    assert(decl != nullptr);
    if (decl->isExternC()) {
        return true;
    }
    if (decl->isInExternCContext()) {
        return true;
    }
    return false;
}

enum class OverrideKind { NO, YES, MAYBE };

OverrideKind getOverrideKind(FunctionDecl const * decl) {
    CXXMethodDecl const * m = dyn_cast<CXXMethodDecl>(decl);
    if (m == nullptr) {
        return OverrideKind::NO;
    }
    if (m->size_overridden_methods() != 0 || m->hasAttr<OverrideAttr>()) {
        return OverrideKind::YES;
    }
    if (!dyn_cast<CXXRecordDecl>(m->getDeclContext())->hasAnyDependentBases()) {
        return OverrideKind::NO;
    }
    return OverrideKind::MAYBE;
}

enum class BoolOverloadKind { No, Yes, CheckNext };

BoolOverloadKind isBoolOverloadOf(
    FunctionDecl const * f, FunctionDecl const * decl, bool mustBeDeleted)
{
    if (!mustBeDeleted || f->isDeleted()) {
        unsigned n = decl->getNumParams();
        if (f->getNumParams() == n) {
            bool hasFB = false;
            for (unsigned i = 0; i != n; ++i) {
                QualType t1 { decl->getParamDecl(i)->getType() };
                bool isFB = isFakeBool(t1) != FBK_No;
                bool isFBRef = !isFB && t1->isReferenceType()
                    && isFakeBool(t1.getNonReferenceType()) != FBK_No;
                QualType t2 { f->getParamDecl(i)->getType() };
                if (!(isFB
                      ? t2->isBooleanType()
                      : isFBRef
                      ? (t2->isReferenceType()
                         && t2.getNonReferenceType()->isBooleanType())
                      : t2.getCanonicalType() == t1.getCanonicalType()))
                {
                    return BoolOverloadKind::CheckNext;
                }
                hasFB |= isFB || isFBRef;
            }
            return hasFB ? BoolOverloadKind::Yes : BoolOverloadKind::No;
                // cheaply protect against the case where decl would have no
                // fake bool parameters at all and would match itself
        }
    }
    return BoolOverloadKind::CheckNext;
}

//TODO: current implementation is not at all general, just tests what we
// encounter in practice:
bool hasBoolOverload(FunctionDecl const * decl, bool mustBeDeleted) {
    auto ctx = decl->getDeclContext();
    if (!ctx->isLookupContext()) {
        return false;
    }
    auto res = ctx->lookup(decl->getDeclName());
    for (auto d = res.begin(); d != res.end(); ++d) {
        if (auto f = dyn_cast<FunctionDecl>(*d)) {
            switch (isBoolOverloadOf(f, decl, mustBeDeleted)) {
            case BoolOverloadKind::No:
                return false;
            case BoolOverloadKind::Yes:
                return true;
            case BoolOverloadKind::CheckNext:
                break;
            }
        } else if (auto ftd = dyn_cast<FunctionTemplateDecl>(*d)) {
            for (auto f: ftd->specializations()) {
                if (f->getTemplateSpecializationKind()
                    == TSK_ExplicitSpecialization)
                {
                    switch (isBoolOverloadOf(f, decl, mustBeDeleted)) {
                    case BoolOverloadKind::No:
                        return false;
                    case BoolOverloadKind::Yes:
                        return true;
                    case BoolOverloadKind::CheckNext:
                        break;
                    }
                }
            }
        }
    }
    return false;
}

class FakeBool:
    public loplugin::FunctionAddress<loplugin::FilteringRewritePlugin<FakeBool>>
{
public:
    explicit FakeBool(loplugin::InstantiationData const & data):
        FunctionAddress(data) {}

    virtual void run() override;

    bool VisitUnaryOperator(UnaryOperator * op);

    bool VisitCallExpr(CallExpr * expr);

    bool VisitCStyleCastExpr(CStyleCastExpr * expr);

    bool VisitCXXStaticCastExpr(CXXStaticCastExpr * expr);

    bool VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr * expr);

    bool VisitImplicitCastExpr(ImplicitCastExpr * expr);

    bool VisitReturnStmt(ReturnStmt const * stmt);

    bool WalkUpFromParmVarDecl(ParmVarDecl const * decl);
    bool VisitParmVarDecl(ParmVarDecl const * decl);

    bool WalkUpFromVarDecl(VarDecl const * decl);
    bool VisitVarDecl(VarDecl const * decl);

    bool WalkUpFromFieldDecl(FieldDecl const * decl);
    bool VisitFieldDecl(FieldDecl const * decl);

    bool WalkUpFromFunctionDecl(FunctionDecl const * decl);
    bool VisitFunctionDecl(FunctionDecl const * decl);

    bool VisitValueDecl(ValueDecl const * decl);

    bool TraverseStaticAssertDecl(StaticAssertDecl * decl);

    bool TraverseLinkageSpecDecl(LinkageSpecDecl * decl);

private:
    bool isFromCIncludeFile(SourceLocation spellingLocation) const;

    bool isSharedCAndCppCode(SourceLocation location) const;

    bool isInSpecialMainFile(SourceLocation spellingLocation) const;

    bool rewrite(SourceLocation location, FakeBoolKind kind);

    std::map<VarDecl const *, FakeBoolKind> varDecls_;
    std::map<FieldDecl const *, FakeBoolKind> fieldDecls_;
    std::map<ParmVarDecl const *, FakeBoolKind> parmVarDecls_;
    std::map<FunctionDecl const *, FakeBoolKind> functionDecls_;
    unsigned int externCContexts_ = 0;
};

void FakeBool::run() {
    if (compiler.getLangOpts().CPlusPlus) {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        for (auto const & dcl: varDecls_) {
            auto const decl = dcl.first; auto const fbk = dcl.second;
            SourceLocation loc { compat::getBeginLoc(decl) };
            TypeSourceInfo * tsi = decl->getTypeSourceInfo();
            if (tsi != nullptr) {
                SourceLocation l {
                    compiler.getSourceManager().getExpansionLoc(
                        tsi->getTypeLoc().getBeginLoc()) };
                SourceLocation end {
                    compiler.getSourceManager().getExpansionLoc(
                        tsi->getTypeLoc().getEndLoc()) };
                assert(l.isFileID() && end.isFileID());
                if (l == end
                    || compiler.getSourceManager().isBeforeInTranslationUnit(
                        l, end))
                {
                    for (;;) {
                        unsigned n = Lexer::MeasureTokenLength(
                            l, compiler.getSourceManager(),
                            compiler.getLangOpts());
                        std::string s {
                            compiler.getSourceManager().getCharacterData(l),
                            n };
                        if (s == getName(fbk)) {
                            loc = l;
                            break;
                        }
                        if (l == end) {
                            break;
                        }
                        l = l.getLocWithOffset(std::max<unsigned>(n, 1));
                    }
                }
            }
            if (!rewrite(loc, fbk)) {
                report(
                    DiagnosticsEngine::Warning,
                    "VarDecl, use \"bool\" instead of %0", loc)
                    << decl->getType().getLocalUnqualifiedType()
                    << decl->getSourceRange();
            }
        }
        for (auto const & dcl: fieldDecls_) {
            auto const decl = dcl.first; auto const fbk = dcl.second;
            SourceLocation loc { compat::getBeginLoc(decl) };
            TypeSourceInfo * tsi = decl->getTypeSourceInfo();
            if (tsi != nullptr) {
                SourceLocation l {
                    compiler.getSourceManager().getExpansionLoc(
                        tsi->getTypeLoc().getBeginLoc()) };
                SourceLocation end {
                    compiler.getSourceManager().getExpansionLoc(
                        tsi->getTypeLoc().getEndLoc()) };
                assert(l.isFileID() && end.isFileID());
                if (l == end
                    || compiler.getSourceManager().isBeforeInTranslationUnit(
                        l, end))
                {
                    for (;;) {
                        unsigned n = Lexer::MeasureTokenLength(
                            l, compiler.getSourceManager(),
                            compiler.getLangOpts());
                        std::string s {
                            compiler.getSourceManager().getCharacterData(l),
                            n };
                        if (s == getName(fbk)) {
                            loc = l;
                            break;
                        }
                        if (l == end) {
                            break;
                        }
                        l = l.getLocWithOffset(std::max<unsigned>(n, 1));
                    }
                }
            }
            if (!rewrite(loc, fbk)) {
                report(
                    DiagnosticsEngine::Warning,
                    "FieldDecl, use \"bool\" instead of %0", loc)
                    << decl->getType().getLocalUnqualifiedType() << decl->getSourceRange();
            }
        }
        auto const ignoredFns = getFunctionsWithAddressTaken();
        for (auto const & dcl: parmVarDecls_) {
            auto const decl = dcl.first; auto const fbk = dcl.second;
            FunctionDecl const * f = cast<FunctionDecl>(decl->getDeclContext())->getCanonicalDecl();
            if (ignoredFns.find(f) != ignoredFns.end()) {
                continue;
            }
            SourceLocation loc { compat::getBeginLoc(decl) };
            TypeSourceInfo * tsi = decl->getTypeSourceInfo();
            if (tsi != nullptr) {
                SourceLocation l {
                    compiler.getSourceManager().getExpansionLoc(
                        tsi->getTypeLoc().getBeginLoc()) };
                SourceLocation end {
                    compiler.getSourceManager().getExpansionLoc(
                        tsi->getTypeLoc().getEndLoc()) };
                assert(l.isFileID() && end.isFileID());
                if (l == end
                    || (compiler.getSourceManager()
                        .isBeforeInTranslationUnit(l, end)))
                {
                    for (;;) {
                        unsigned n = Lexer::MeasureTokenLength(
                            l, compiler.getSourceManager(),
                            compiler.getLangOpts());
                        std::string s {
                            compiler.getSourceManager().getCharacterData(l),
                            n };
                        if (s == getName(fbk)) {
                            loc = l;
                            break;
                        }
                        if (l == end) {
                            break;
                        }
                        l = l.getLocWithOffset(std::max<unsigned>(n, 1));
                    }
                }
            }
            // Only rewrite declarations in include files if a
            // definition is also seen, to avoid compilation of a
            // definition (in a main file only processed later) to fail
            // with a "mismatch" error before the rewriter had a chance
            // to act upon the definition (but use the heuristic of
            // assuming pure virtual functions do not have definitions);
            // also, do not automatically rewrite functions that could
            // implicitly override depend base functions (and thus stop
            // doing so after the rewrite; note that this is less
            // dangerous for return types than for parameter types,
            // where the function would still implicitly override and
            // cause a compilation error due to the incompatible return
            // type):
            OverrideKind k = getOverrideKind(f);
            if (!((compiler.getSourceManager().isInMainFile(
                       compiler.getSourceManager().getSpellingLoc(
                           dyn_cast<FunctionDecl>(
                               decl->getDeclContext())
                           ->getNameInfo().getLoc()))
                   || f->isDefined() || f->isPure())
                  && k != OverrideKind::MAYBE && rewrite(loc, fbk)))
            {
                report(
                    DiagnosticsEngine::Warning,
                    ("ParmVarDecl, use \"bool\" instead of"
                     " %0%1"),
                    loc)
                    << decl->getType().getNonReferenceType().getLocalUnqualifiedType()
                    << (k == OverrideKind::MAYBE
                        ? (" (unless this member function overrides a"
                           " dependent base member function, even"
                           " though it is not marked 'override')")
                        : "")
                    << decl->getSourceRange();
            }
        }
        for (auto const & dcl: functionDecls_) {
            auto const decl = dcl.first; auto const fbk = dcl.second;
            FunctionDecl const * f = decl->getCanonicalDecl();
            if (ignoredFns.find(f) != ignoredFns.end()) {
                continue;
            }
            SourceLocation loc { compat::getBeginLoc(decl) };
            SourceLocation l { compiler.getSourceManager().getExpansionLoc(
                loc) };
            SourceLocation end { compiler.getSourceManager().getExpansionLoc(
                decl->getNameInfo().getLoc()) };
            assert(l.isFileID() && end.isFileID());
            if (compiler.getSourceManager().isBeforeInTranslationUnit(l, end)) {
                while (l != end) {
                    unsigned n = Lexer::MeasureTokenLength(
                        l, compiler.getSourceManager(), compiler.getLangOpts());
                    std::string s {
                        compiler.getSourceManager().getCharacterData(l), n };
                    if (s == getName(fbk)) {
                        loc = l;
                        break;
                    }
                    l = l.getLocWithOffset(std::max<unsigned>(n, 1));
                }
            }
            // Only rewrite declarations in include files if a definition is
            // also seen, to avoid compilation of a definition (in a main file
            // only processed later) to fail with a "mismatch" error before the
            // rewriter had a chance to act upon the definition (but use the
            // heuristic of assuming pure virtual functions do not have
            // definitions):
            if (!((compiler.getSourceManager().isInMainFile(
                       compiler.getSourceManager().getSpellingLoc(
                           decl->getNameInfo().getLoc()))
                   || f->isDefined() || f->isPure())
                  && rewrite(loc, fbk)))
            {
                report(
                    DiagnosticsEngine::Warning,
                    "use \"bool\" instead of %0 as return type%1",
                    loc)
                    << decl->getReturnType().getNonReferenceType().getLocalUnqualifiedType()
                    << (getOverrideKind(f) == OverrideKind::MAYBE
                        ? (" (unless this member function overrides a dependent"
                           " base member function, even though it is not marked"
                           " 'override')")
                        : "")
                    << decl->getSourceRange();
            }
        }
    }
}

bool FakeBool::VisitUnaryOperator(UnaryOperator * op) {
    if (op->getOpcode() != UO_AddrOf) {
        return FunctionAddress::VisitUnaryOperator(op);
    }
    FunctionAddress::VisitUnaryOperator(op);
    Expr const * e1 = op->getSubExpr()->IgnoreParenCasts();
    if (isFakeBool(e1->getType()) != FBK_No) {
        if (DeclRefExpr const * e2 = dyn_cast<DeclRefExpr>(e1)) {
            VarDecl const * d = dyn_cast<VarDecl>(e2->getDecl());
            if (d != nullptr) {
                varDecls_.erase(d);
            }
        } else if (auto const e3 = dyn_cast<MemberExpr>(e1)) {
            if (auto const d = dyn_cast<FieldDecl>(e3->getMemberDecl())) {
                fieldDecls_.erase(d);
            }
        }
    }
    return true;
}

bool FakeBool::VisitCallExpr(CallExpr * expr) {
    Decl const * d = expr->getCalleeDecl();
    FunctionProtoType const * ft = nullptr;
    if (d != nullptr) {
        FunctionDecl const * fd = dyn_cast<FunctionDecl>(d);
        if (fd != nullptr) {
            if (!hasBoolOverload(fd, false)) {
                clang::PointerType const * pt = fd->getType()
                    ->getAs<clang::PointerType>();
                QualType t2(
                    pt == nullptr ? fd->getType() : pt->getPointeeType());
                ft = t2->getAs<FunctionProtoType>();
                assert(
                    ft != nullptr || !compiler.getLangOpts().CPlusPlus
                    || (fd->getBuiltinID() != Builtin::NotBuiltin
                        && isa<FunctionNoProtoType>(t2)));
                    // __builtin_*s have no proto type?
            }
        } else {
            VarDecl const * vd = dyn_cast<VarDecl>(d);
            if (vd != nullptr) {
                clang::PointerType const * pt = vd->getType()
                    ->getAs<clang::PointerType>();
                ft = (pt == nullptr ? vd->getType() : pt->getPointeeType())
                    ->getAs<FunctionProtoType>();
            }
        }
    }
    if (ft != nullptr) {
        for (unsigned i = 0; i != ft->getNumParams(); ++i) {
            QualType t(ft->getParamType(i));
            bool b = false;
            if (t->isLValueReferenceType()) {
                t = t.getNonReferenceType();
                b = !t.isConstQualified() && isFakeBool(t) != FBK_No;
            } else if (t->isPointerType()) {
                for (;;) {
                    auto t2 = t->getAs<clang::PointerType>();
                    if (t2 == nullptr) {
                        break;
                    }
                    t = t2->getPointeeType();
                }
                b = isFakeBool(t) != FBK_No;
            }
            if (b && i < expr->getNumArgs()) {
                auto const e1 = expr->getArg(i)->IgnoreParenImpCasts();
                if (DeclRefExpr * ref = dyn_cast<DeclRefExpr>(e1)) {
                    VarDecl const * d = dyn_cast<VarDecl>(ref->getDecl());
                    if (d != nullptr) {
                        varDecls_.erase(d);
                    }
                } else if (auto const e2 = dyn_cast<MemberExpr>(e1)) {
                    if (auto const d = dyn_cast<FieldDecl>(e2->getMemberDecl())) {
                        fieldDecls_.erase(d);
                    }
                }
            }
        }
    }
    return true;
}

bool FakeBool::VisitCStyleCastExpr(CStyleCastExpr * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    auto const k = isFakeBool(expr->getType());
    if (k != FBK_No) {
        SourceLocation loc { compat::getBeginLoc(expr) };
        while (compiler.getSourceManager().isMacroArgExpansion(loc)) {
            loc = compiler.getSourceManager().getImmediateMacroCallerLoc(loc);
        }
        if (compiler.getSourceManager().isMacroBodyExpansion(loc)) {
            StringRef name { Lexer::getImmediateMacroName(
                loc, compiler.getSourceManager(), compiler.getLangOpts()) };
            if (k == FBK_sal_Bool && (name == "sal_False" || name == "sal_True")) {
                auto callLoc = compiler.getSourceManager()
                    .getImmediateMacroCallerLoc(loc);
                if (!isSharedCAndCppCode(callLoc)) {
                    SourceLocation argLoc;
                    if (compiler.getSourceManager().isMacroArgExpansion(
                            compat::getBeginLoc(expr), &argLoc)
                        //TODO: check it's the complete (first) arg to the macro
                        && (Lexer::getImmediateMacroName(
                                argLoc, compiler.getSourceManager(),
                                compiler.getLangOpts())
                            == "CPPUNIT_ASSERT_EQUAL"))
                    {
                        // Ignore sal_False/True that are directly used as
                        // arguments to CPPUNIT_ASSERT_EQUAL:
                        return true;
                    }
                    bool b = k == FBK_sal_Bool && name == "sal_True";
                    if (rewriter != nullptr) {
                        auto callSpellLoc = compiler.getSourceManager()
                            .getSpellingLoc(callLoc);
                        unsigned n = Lexer::MeasureTokenLength(
                            callSpellLoc, compiler.getSourceManager(),
                            compiler.getLangOpts());
                        if (StringRef(
                                compiler.getSourceManager().getCharacterData(
                                    callSpellLoc),
                                n)
                            == name)
                        {
                            return replaceText(
                                callSpellLoc, n, b ? "true" : "false");
                        }
                    }
                    report(
                        DiagnosticsEngine::Warning,
                        "use '%select{false|true}0' instead of '%1'", callLoc)
                        << b << name << expr->getSourceRange();
                }
                return true;
            }
            if (isSharedCAndCppCode(loc)) {
                return true;
            }
        }
        report(
            DiagnosticsEngine::Warning,
            "CStyleCastExpr, suspicious cast from %0 to %1",
            compat::getBeginLoc(expr))
            << expr->getSubExpr()->IgnoreParenImpCasts()->getType()
            << expr->getType() << expr->getSourceRange();
    }
    return true;
}

bool FakeBool::VisitCXXStaticCastExpr(CXXStaticCastExpr * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    auto const k = isFakeBool(expr->getType());
    if (k == FBK_No) {
        return true;
    }
    if (k == FBK_sal_Bool
        && isInSpecialMainFile(
            compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(expr))))
    {
        return true;
    }
    report(
        DiagnosticsEngine::Warning,
        "CXXStaticCastExpr, suspicious cast from %0 to %1",
        compat::getBeginLoc(expr))
        << expr->getSubExpr()->IgnoreParenImpCasts()->getType()
        << expr->getType() << expr->getSourceRange();
    return true;
}

bool FakeBool::VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    if (isFakeBool(expr->getType()) != FBK_No) {
        report(
            DiagnosticsEngine::Warning,
            "CXXFunctionalCastExpr, suspicious cast from %0 to %1",
            compat::getBeginLoc(expr))
            << expr->getSubExpr()->IgnoreParenImpCasts()->getType()
            << expr->getType() << expr->getSourceRange();
    }
    return true;
}

bool FakeBool::VisitImplicitCastExpr(ImplicitCastExpr * expr) {
    FunctionAddress::VisitImplicitCastExpr(expr);
    if (ignoreLocation(expr)) {
        return true;
    }
    if (isFakeBool(expr->getType()) == FBK_No) {
        return true;
    }
    auto l = compat::getBeginLoc(expr);
    while (compiler.getSourceManager().isMacroArgExpansion(l)) {
        l = compiler.getSourceManager().getImmediateMacroCallerLoc(l);
    }
    if (compiler.getSourceManager().isMacroBodyExpansion(l) && isSharedCAndCppCode(l)) {
        return true;
    }
    auto e1 = expr->getSubExprAsWritten();
    auto t = e1->getType();
    if (!t->isFundamentalType() || loplugin::TypeCheck(t).AnyBoolean()) {
        return true;
    }
    auto e2 = dyn_cast<ConditionalOperator>(e1);
    if (e2 != nullptr) {
        auto ic1 = dyn_cast<ImplicitCastExpr>(
            e2->getTrueExpr()->IgnoreParens());
        auto ic2 = dyn_cast<ImplicitCastExpr>(
            e2->getFalseExpr()->IgnoreParens());
        if (ic1 != nullptr && ic2 != nullptr
            && ic1->getType()->isSpecificBuiltinType(BuiltinType::Int)
            && (loplugin::TypeCheck(ic1->getSubExprAsWritten()->getType())
                .AnyBoolean())
            && ic2->getType()->isSpecificBuiltinType(BuiltinType::Int)
            && (loplugin::TypeCheck(ic2->getSubExprAsWritten()->getType())
                .AnyBoolean()))
        {
            return true;
        }
    }
    report(
        DiagnosticsEngine::Warning, "conversion from %0 to %1",
        compat::getBeginLoc(expr))
        << t << expr->getType() << expr->getSourceRange();
    return true;
}

bool FakeBool::VisitReturnStmt(ReturnStmt const * stmt) {
    // Just enough to avoid warnings in rtl_getUriCharClass (sal/rtl/uri.cxx),
    // which has
    //
    //  static sal_Bool const aCharClass[][nCharClassSize] = ...;
    //
    // and
    //
    //  return aCharClass[eCharClass];
    //
    if (ignoreLocation(stmt)) {
        return true;
    }
    auto e = stmt->getRetValue();
    if (e == nullptr) {
        return true;
    }
    auto t = e->getType();
    if (!t->isPointerType()) {
        return true;
    }
    for (;;) {
        auto t2 = t->getAs<clang::PointerType>();
        if (t2 == nullptr) {
            break;
        }
        t = t2->getPointeeType();
    }
    if (isFakeBool(t) != FBK_sal_Bool) {
        return true;
    }
    auto e2 = dyn_cast<ArraySubscriptExpr>(e->IgnoreParenImpCasts());
    if (e2 == nullptr) {
        return true;
    }
    auto e3 = dyn_cast<DeclRefExpr>(e2->getBase()->IgnoreParenImpCasts());
    if (e3 == nullptr) {
        return true;
    }
    auto d = dyn_cast<VarDecl>(e3->getDecl());
    if (d == nullptr) {
        return true;
    }
    varDecls_.erase(d);
    return true;
}

bool FakeBool::WalkUpFromParmVarDecl(ParmVarDecl const * decl) {
    return VisitParmVarDecl(decl);
}

bool FakeBool::VisitParmVarDecl(ParmVarDecl const * decl) {
    if (ignoreLocation(decl)) {
        return true;
    }
    auto const fbk = isFakeBool(decl->getType().getNonReferenceType());
    if (fbk != FBK_No) {
        FunctionDecl const * f = dyn_cast<FunctionDecl>(decl->getDeclContext());
        if (f != nullptr) { // e.g.: typedef sal_Bool (* FuncPtr )( sal_Bool );
            f = f->getCanonicalDecl();
            if (handler.isAllRelevantCodeDefined(f)
                && !(hasCLanguageLinkageType(f)
                     || (fbk == FBK_sal_Bool && isInUnoIncludeFile(f)
                         && (!f->isInlined() || f->hasAttr<DeprecatedAttr>()
                             || decl->getType()->isReferenceType()
                             || hasBoolOverload(f, false)))
                     || f->isDeleted() || hasBoolOverload(f, true)))
            {
                OverrideKind k = getOverrideKind(f);
                if (k != OverrideKind::YES) {
                    parmVarDecls_.insert({decl, fbk});
                }
            }
        }
    }
    return true;
}

bool FakeBool::WalkUpFromVarDecl(VarDecl const * decl) {
    return VisitVarDecl(decl);
}

bool FakeBool::VisitVarDecl(VarDecl const * decl) {
    if (ignoreLocation(decl)) {
        return true;
    }
    if (decl->isExternC()) {
        return true;
    }
    auto k = isFakeBool(decl->getType());
    if (k == FBK_No) {
        k = isFakeBoolArray(decl->getType());
    }
    if (k == FBK_No) {
        return true;
    }
    auto const loc = compat::getBeginLoc(decl);
    if (k == FBK_sal_Bool
        && isInSpecialMainFile(
            compiler.getSourceManager().getSpellingLoc(loc)))
    {
        return true;
    }
    auto l = loc;
    while (compiler.getSourceManager().isMacroArgExpansion(l)) {
        l = compiler.getSourceManager().getImmediateMacroCallerLoc(l);
    }
    if (compiler.getSourceManager().isMacroBodyExpansion(l)
        && isSharedCAndCppCode(l))
    {
        return true;
    }
    varDecls_.insert({decl, k});
    return true;
}

bool FakeBool::WalkUpFromFieldDecl(FieldDecl const * decl) {
    return VisitFieldDecl(decl);
}

bool FakeBool::VisitFieldDecl(FieldDecl const * decl) {
    if (ignoreLocation(decl)) {
        return true;
    }
    auto k = isFakeBool(decl->getType());
    if (k == FBK_No) {
        k = isFakeBoolArray(decl->getType());
    }
    if (k == FBK_No) {
        return true;
    }
    if (!handler.isAllRelevantCodeDefined(decl)) {
        return true;
    }
    if (k == FBK_sal_Bool
        && isInSpecialMainFile(
            compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(decl))))
    {
        return true;
    }
    TagDecl const * td = dyn_cast<TagDecl>(decl->getDeclContext());
    if (td == nullptr) {
        //TODO: ObjCInterface
        return true;
    }
    if (!(((td->isStruct() || td->isUnion()) && td->isExternCContext())
          || isInUnoIncludeFile(
              compiler.getSourceManager().getSpellingLoc(
                  decl->getLocation()))))
    {
        fieldDecls_.insert({decl, k});
    }
    return true;
}

bool FakeBool::WalkUpFromFunctionDecl(FunctionDecl const * decl) {
    return VisitFunctionDecl(decl);
}

bool FakeBool::VisitFunctionDecl(FunctionDecl const * decl) {
    if (ignoreLocation(decl)) {
        return true;
    }
    auto const fbk = isFakeBool(decl->getReturnType().getNonReferenceType());
    if (fbk != FBK_No
        && !(decl->isDeletedAsWritten() && isa<CXXConversionDecl>(decl))
        && handler.isAllRelevantCodeDefined(decl))
    {
        FunctionDecl const * f = decl->getCanonicalDecl();
        OverrideKind k = getOverrideKind(f);
        if (k != OverrideKind::YES
            && !(hasCLanguageLinkageType(f)
                 || (isInUnoIncludeFile(f)
                     && (!f->isInlined() || f->hasAttr<DeprecatedAttr>()))))
        {
            functionDecls_.insert({decl, fbk});
        }
    }
    return true;
}

bool FakeBool::VisitValueDecl(ValueDecl const * decl) {
    if (ignoreLocation(decl)) {
        return true;
    }
    auto const k = isFakeBool(decl->getType());
    if (k != FBK_No && !rewrite(compat::getBeginLoc(decl), k)) {
        report(
            DiagnosticsEngine::Warning,
            "ValueDecl, use \"bool\" instead of %0",
            compat::getBeginLoc(decl))
            << decl->getType() << decl->getSourceRange();
    }
    return true;
}

bool FakeBool::TraverseStaticAssertDecl(StaticAssertDecl * decl) {
    // Ignore special code like
    //
    //   static_cast<sal_Bool>(true) == sal_True
    //
    // inside static_assert in cppu/source/uno/check.cxx:
    return
        loplugin::isSamePathname(
            getFilenameOfLocation(decl->getLocation()),
            SRCDIR "/cppu/source/uno/check.cxx")
        || RecursiveASTVisitor::TraverseStaticAssertDecl(decl);
}

bool FakeBool::TraverseLinkageSpecDecl(LinkageSpecDecl * decl) {
    assert(externCContexts_ != std::numeric_limits<unsigned int>::max()); //TODO
    ++externCContexts_;
    bool ret = RecursiveASTVisitor::TraverseLinkageSpecDecl(decl);
    assert(externCContexts_ != 0);
    --externCContexts_;
    return ret;
}

bool FakeBool::isFromCIncludeFile(SourceLocation spellingLocation) const {
    return !compiler.getSourceManager().isInMainFile(spellingLocation)
        && (StringRef(
                compiler.getSourceManager().getPresumedLoc(spellingLocation)
                .getFilename())
            .endswith(".h"));
}

bool FakeBool::isSharedCAndCppCode(SourceLocation location) const {
    // Assume that code is intended to be shared between C and C++ if it comes
    // from an include file ending in .h, and is either in an extern "C" context
    // or the body of a macro definition:
    return
        isFromCIncludeFile(compiler.getSourceManager().getSpellingLoc(location))
        && (externCContexts_ != 0
            || compiler.getSourceManager().isMacroBodyExpansion(location));
}

bool FakeBool::isInSpecialMainFile(SourceLocation spellingLocation) const {
    if (!compiler.getSourceManager().isInMainFile(spellingLocation)) {
        return false;
    }
    auto f = getFilenameOfLocation(spellingLocation);
    return loplugin::isSamePathname(f, SRCDIR "/cppu/qa/test_any.cxx")
        || loplugin::isSamePathname(f, SRCDIR "/cppu/source/uno/check.cxx");
        // TODO: the offset checks
}

bool FakeBool::rewrite(SourceLocation location, FakeBoolKind kind) {
    if (rewriter != nullptr) {
        //TODO: "::sal_Bool" -> "bool", not "::bool"
        SourceLocation loc { compiler.getSourceManager().getExpansionLoc(
                location) };
        unsigned n = Lexer::MeasureTokenLength(
            loc, compiler.getSourceManager(), compiler.getLangOpts());
        if (std::string(compiler.getSourceManager().getCharacterData(loc), n)
            == getName(kind))
        {
            return replaceText(loc, n, "bool");
        }
    }
    return false;
}

loplugin::Plugin::Registration<FakeBool> X("fakebool", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
