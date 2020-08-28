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
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stack>
#include <string>
#include <vector>
#include <iostream>

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

// Define a "string constant" to be a constant expression either of type "array
// of N char" where each array element is a non-NULL ASCII character---except
// that the last array element may be NULL, or, in some situations, of type char
// with an ASCII value (including NULL).  Note that the former includes
// expressions denoting narrow string literals like "foo", and, with toolchains
// that support constexpr, constexpr variables declared like
//
//   constexpr char str[] = "bar";
//
// This plugin flags uses of OUString functions with string constant arguments
// that can be rewritten more directly, like
//
//   OUString::createFromAscii("foo")  ->  "foo"
//
// and
//
//   s.equals(OUString("bar"))  ->  s == "bar"

namespace {

SourceLocation getMemberLocation(Expr const * expr) {
    CallExpr const * e1 = dyn_cast<CallExpr>(expr);
    MemberExpr const * e2 = e1 == nullptr
        ? nullptr : dyn_cast<MemberExpr>(e1->getCallee());
    return e2 == nullptr ? expr->getExprLoc()/*TODO*/ : e2->getMemberLoc();
}

bool isLhsOfAssignment(FunctionDecl const * decl, unsigned parameter) {
    if (parameter != 0) {
        return false;
    }
    auto oo = decl->getOverloadedOperator();
    return oo == OO_Equal
        || (oo >= OO_PlusEqual && oo <= OO_GreaterGreaterEqual);
}

bool hasOverloads(FunctionDecl const * decl, unsigned arguments) {
    int n = 0;
    auto ctx = decl->getDeclContext();
    if (ctx->getDeclKind() == Decl::LinkageSpec) {
        ctx = ctx->getParent();
    }
    auto res = ctx->lookup(decl->getDeclName());
    for (auto d = res.begin(); d != res.end(); ++d) {
        FunctionDecl const * f = dyn_cast<FunctionDecl>(*d);
        if (f != nullptr && f->getMinRequiredArguments() <= arguments
            && f->getNumParams() >= arguments)
        {
            auto consDecl = dyn_cast<CXXConstructorDecl>(f);
            if (consDecl && consDecl->isCopyOrMoveConstructor()) {
                continue;
            }
            ++n;
            if (n == 2) {
                return true;
            }
        }
    }
    return false;
}

CXXConstructExpr const * lookForCXXConstructExpr(Expr const * expr) {
    if (auto e = dyn_cast<MaterializeTemporaryExpr>(expr)) {
        expr = compat::getSubExpr(e);
    }
    if (auto e = dyn_cast<CXXFunctionalCastExpr>(expr)) {
        expr = e->getSubExpr();
    }
    if (auto e = dyn_cast<CXXBindTemporaryExpr>(expr)) {
        expr = e->getSubExpr();
    }
    return dyn_cast<CXXConstructExpr>(expr);
}

char const * adviseNonArray(bool nonArray) {
    return nonArray
        ? ", and turn the non-array string constant into an array" : "";
}

class StringConstant:
    public loplugin::FilteringRewritePlugin<StringConstant>
{
public:
    explicit StringConstant(loplugin::InstantiationData const & data):
        FilteringRewritePlugin(data) {}

    void run() override;

    bool TraverseFunctionDecl(FunctionDecl * decl) {
        returnTypes_.push(compat::getDeclaredReturnType(decl));
        auto const ret = RecursiveASTVisitor::TraverseFunctionDecl(decl);
        assert(!returnTypes_.empty());
        assert(returnTypes_.top() == compat::getDeclaredReturnType(decl));
        returnTypes_.pop();
        return ret;
    }

    bool TraverseCXXDeductionGuideDecl(CXXDeductionGuideDecl * decl) {
        returnTypes_.push(compat::getDeclaredReturnType(decl));
        auto const ret = RecursiveASTVisitor::TraverseCXXDeductionGuideDecl(
            decl);
        assert(!returnTypes_.empty());
        assert(returnTypes_.top() == compat::getDeclaredReturnType(decl));
        returnTypes_.pop();
        return ret;
    }

    bool TraverseCXXMethodDecl(CXXMethodDecl * decl) {
        returnTypes_.push(compat::getDeclaredReturnType(decl));
        auto const ret = RecursiveASTVisitor::TraverseCXXMethodDecl(decl);
        assert(!returnTypes_.empty());
        assert(returnTypes_.top() == compat::getDeclaredReturnType(decl));
        returnTypes_.pop();
        return ret;
    }

    bool TraverseCXXConstructorDecl(CXXConstructorDecl * decl) {
        returnTypes_.push(compat::getDeclaredReturnType(decl));
        auto const ret = RecursiveASTVisitor::TraverseCXXConstructorDecl(decl);
        assert(!returnTypes_.empty());
        assert(returnTypes_.top() == compat::getDeclaredReturnType(decl));
        returnTypes_.pop();
        return ret;
    }

    bool TraverseCXXDestructorDecl(CXXDestructorDecl * decl) {
        returnTypes_.push(compat::getDeclaredReturnType(decl));
        auto const ret = RecursiveASTVisitor::TraverseCXXDestructorDecl(decl);
        assert(!returnTypes_.empty());
        assert(returnTypes_.top() == compat::getDeclaredReturnType(decl));
        returnTypes_.pop();
        return ret;
    }

    bool TraverseCXXConversionDecl(CXXConversionDecl * decl) {
        returnTypes_.push(compat::getDeclaredReturnType(decl));
        auto const ret = RecursiveASTVisitor::TraverseCXXConversionDecl(decl);
        assert(!returnTypes_.empty());
        assert(returnTypes_.top() == compat::getDeclaredReturnType(decl));
        returnTypes_.pop();
        return ret;
    }

    bool TraverseObjCMethodDecl(ObjCMethodDecl * decl) {
        returnTypes_.push(decl->getReturnType());
        auto const ret = RecursiveASTVisitor::TraverseObjCMethodDecl(decl);
        assert(!returnTypes_.empty());
        assert(returnTypes_.top() == decl->getReturnType());
        returnTypes_.pop();
        return ret;
    }

    bool TraverseCallExpr(CallExpr * expr);

    bool TraverseCXXMemberCallExpr(CXXMemberCallExpr * expr);

    bool TraverseCXXOperatorCallExpr(CXXOperatorCallExpr * expr);

    bool TraverseCXXConstructExpr(CXXConstructExpr * expr);

    bool VisitCallExpr(CallExpr const * expr);

    bool VisitCXXConstructExpr(CXXConstructExpr const * expr);

    bool VisitReturnStmt(ReturnStmt const * stmt);

private:
    enum class ContentKind { Ascii, Utf8, Arbitrary };

    enum class TreatEmpty { DefaultCtor, CheckEmpty, Error };

    enum class ChangeKind { Char, CharLen, SingleChar, OUStringChar };

    enum class PassThrough { No, EmptyConstantString, NonEmptyConstantString };

    std::string describeChangeKind(ChangeKind kind);

    bool isStringConstant(
        Expr const * expr, unsigned * size, bool * nonArray,
        ContentKind * content, bool * embeddedNuls, bool * terminatingNul,
        std::vector<char32_t> * utf8Content = nullptr);

    bool isZero(Expr const * expr);

    void reportChange(
        Expr const * expr, ChangeKind kind, std::string const & original,
        std::string const & replacement, PassThrough pass, bool nonArray,
        char const * rewriteFrom, char const * rewriteTo);

    void checkEmpty(
        CallExpr const * expr, FunctionDecl const * callee,
        TreatEmpty treatEmpty, unsigned size, std::string * replacement);

    void handleChar(
        CallExpr const * expr, unsigned arg, FunctionDecl const * callee,
        std::string const & replacement, TreatEmpty treatEmpty, bool literal,
    char const * rewriteFrom = nullptr, char const * rewriteTo = nullptr);

    void handleCharLen(
        CallExpr const * expr, unsigned arg1, unsigned arg2,
        FunctionDecl const * callee, std::string const & replacement,
        TreatEmpty treatEmpty);

    void handleOUStringCtor(
        CallExpr const * expr, unsigned arg, FunctionDecl const * callee,
        bool explicitFunctionalCastNotation);

    void handleOStringCtor(
        CallExpr const * expr, unsigned arg, FunctionDecl const * callee,
        bool explicitFunctionalCastNotation);

    void handleOUStringCtor(
        Expr const * expr, Expr const * argExpr, FunctionDecl const * callee,
        bool explicitFunctionalCastNotation);

    void handleOStringCtor(
        Expr const * expr, Expr const * argExpr, FunctionDecl const * callee,
        bool explicitFunctionalCastNotation);

    enum class StringKind { Unicode, Char };
    void handleStringCtor(
        Expr const * expr, Expr const * argExpr, FunctionDecl const * callee,
        bool explicitFunctionalCastNotation, StringKind stringKind);

    void handleFunArgOstring(
        CallExpr const * expr, unsigned arg, FunctionDecl const * callee);

    std::stack<QualType> returnTypes_;
    std::stack<Expr const *> calls_;
};

void StringConstant::run() {
    if (compiler.getLangOpts().CPlusPlus
        && compiler.getPreprocessor().getIdentifierInfo(
            "LIBO_INTERNAL_ONLY")->hasMacroDefinition())
            //TODO: some parts of it are useful for external code, too
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }
}

bool StringConstant::TraverseCallExpr(CallExpr * expr) {
    if (!WalkUpFromCallExpr(expr)) {
        return false;
    }
    calls_.push(expr);
    bool bRes = true;
    for (auto * e: expr->children()) {
        if (!TraverseStmt(e)) {
            bRes = false;
            break;
        }
    }
    calls_.pop();
    return bRes;
}

bool StringConstant::TraverseCXXMemberCallExpr(CXXMemberCallExpr * expr) {
    if (!WalkUpFromCXXMemberCallExpr(expr)) {
        return false;
    }
    calls_.push(expr);
    bool bRes = true;
    for (auto * e: expr->children()) {
        if (!TraverseStmt(e)) {
            bRes = false;
            break;
        }
    }
    calls_.pop();
    return bRes;
}

bool StringConstant::TraverseCXXOperatorCallExpr(CXXOperatorCallExpr * expr)
{
    if (!WalkUpFromCXXOperatorCallExpr(expr)) {
        return false;
    }
    calls_.push(expr);
    bool bRes = true;
    for (auto * e: expr->children()) {
        if (!TraverseStmt(e)) {
            bRes = false;
            break;
        }
    }
    calls_.pop();
    return bRes;
}

bool StringConstant::TraverseCXXConstructExpr(CXXConstructExpr * expr) {
    if (!WalkUpFromCXXConstructExpr(expr)) {
        return false;
    }
    calls_.push(expr);
    bool bRes = true;
    for (auto * e: expr->children()) {
        if (!TraverseStmt(e)) {
            bRes = false;
            break;
        }
    }
    calls_.pop();
    return bRes;
}

bool StringConstant::VisitCallExpr(CallExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    FunctionDecl const * fdecl = expr->getDirectCallee();
    if (fdecl == nullptr) {
        return true;
    }
    for (unsigned i = 0; i != fdecl->getNumParams(); ++i) {
        auto t = fdecl->getParamDecl(i)->getType();
        if (loplugin::TypeCheck(t).NotSubstTemplateTypeParmType()
            .LvalueReference().Const().NotSubstTemplateTypeParmType()
            .Class("OUString").Namespace("rtl").GlobalNamespace())
        {
            if (!(isLhsOfAssignment(fdecl, i)
                  || hasOverloads(fdecl, expr->getNumArgs())))
            {
                handleOUStringCtor(expr, i, fdecl, true);
            }
        }
        if (loplugin::TypeCheck(t).NotSubstTemplateTypeParmType()
            .LvalueReference().Const().NotSubstTemplateTypeParmType()
            .Class("OString").Namespace("rtl").GlobalNamespace())
        {
            if (!(isLhsOfAssignment(fdecl, i)
                  || hasOverloads(fdecl, expr->getNumArgs())))
            {
                handleOStringCtor(expr, i, fdecl, true);
            }
        }
    }
    loplugin::DeclCheck dc(fdecl);
    //TODO: u.compareToAscii("foo") -> u.???("foo")
    //TODO: u.compareToIgnoreAsciiCaseAscii("foo") -> u.???("foo")
    if ((dc.Function("createFromAscii").Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 1)
    {
        // OUString::createFromAscii("foo") -> OUString("foo")
        handleChar(
            expr, 0, fdecl, "rtl::OUString constructor",
            TreatEmpty::DefaultCtor, true);
        return true;
    }
    if ((dc.Function("endsWithAsciiL").Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 2)
    {
        // u.endsWithAsciiL("foo", 3) -> u.endsWith("foo"):
        handleCharLen(
            expr, 0, 1, fdecl, "rtl::OUString::endsWith", TreatEmpty::Error);
        return true;
    }
    if ((dc.Function("endsWithIgnoreAsciiCaseAsciiL").Class("OUString")
         .Namespace("rtl").GlobalNamespace())
        && fdecl->getNumParams() == 2)
    {
        // u.endsWithIgnoreAsciiCaseAsciiL("foo", 3) ->
        // u.endsWithIgnoreAsciiCase("foo"):
        handleCharLen(
            expr, 0, 1, fdecl, "rtl::OUString::endsWithIgnoreAsciiCase",
            TreatEmpty::Error);
        return true;
    }
    if ((dc.Function("equalsAscii").Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 1)
    {
        // u.equalsAscii("foo") -> u == "foo":
        handleChar(
            expr, 0, fdecl, "operator ==", TreatEmpty::CheckEmpty, false);
        return true;
    }
    if ((dc.Function("equalsAsciiL").Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 2)
    {
        // u.equalsAsciiL("foo", 3) -> u == "foo":
        handleCharLen(expr, 0, 1, fdecl, "operator ==", TreatEmpty::CheckEmpty);
        return true;
    }
    if ((dc.Function("equalsIgnoreAsciiCaseAscii").Class("OUString")
         .Namespace("rtl").GlobalNamespace())
        && fdecl->getNumParams() == 1)
    {
        // u.equalsIgnoreAsciiCaseAscii("foo") ->
        // u.equalsIngoreAsciiCase("foo"):

        auto file = getFilenameOfLocation(
            compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(expr)));
        if (loplugin::isSamePathname(
                file, SRCDIR "/sal/qa/rtl/strings/test_oustring_compare.cxx"))
        {
            return true;
        }
        handleChar(
            expr, 0, fdecl, "rtl::OUString::equalsIgnoreAsciiCase",
            TreatEmpty::CheckEmpty, false);
        return true;
    }
    if ((dc.Function("equalsIgnoreAsciiCaseAsciiL").Class("OUString")
         .Namespace("rtl").GlobalNamespace())
        && fdecl->getNumParams() == 2)
    {
        // u.equalsIgnoreAsciiCaseAsciiL("foo", 3) ->
        // u.equalsIngoreAsciiCase("foo"):
        auto file = getFilenameOfLocation(
            compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(expr)));
        if (loplugin::isSamePathname(
                file, SRCDIR "/sal/qa/rtl/strings/test_oustring_compare.cxx"))
        {
            return true;
        }
        handleCharLen(
            expr, 0, 1, fdecl, "rtl::OUString::equalsIgnoreAsciiCase",
            TreatEmpty::CheckEmpty);
        return true;
    }
    if ((dc.Function("indexOfAsciiL").Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 3)
    {
        assert(expr->getNumArgs() == 3);
        // u.indexOfAsciiL("foo", 3, i) -> u.indexOf("foo", i):
        handleCharLen(
            expr, 0, 1, fdecl, "rtl::OUString::indexOf", TreatEmpty::Error);
        return true;
    }
    if ((dc.Function("lastIndexOfAsciiL").Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 2)
    {
        // u.lastIndexOfAsciiL("foo", 3) -> u.lastIndexOf("foo"):
        handleCharLen(
            expr, 0, 1, fdecl, "rtl::OUString::lastIndexOf", TreatEmpty::Error);
        return true;
    }
    if ((dc.Function("matchAsciiL").Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 3)
    {
        assert(expr->getNumArgs() == 3);
        // u.matchAsciiL("foo", 3, i) -> u.match("foo", i):
        handleCharLen(
            expr, 0, 1, fdecl,
            (isZero(expr->getArg(2))
             ? std::string("rtl::OUString::startsWith")
             : std::string("rtl::OUString::match")),
            TreatEmpty::Error);
        return true;
    }
    if ((dc.Function("matchIgnoreAsciiCaseAsciiL").Class("OUString")
         .Namespace("rtl").GlobalNamespace())
        && fdecl->getNumParams() == 3)
    {
        assert(expr->getNumArgs() == 3);
        // u.matchIgnoreAsciiCaseAsciiL("foo", 3, i) ->
        // u.matchIgnoreAsciiCase("foo", i):
        handleCharLen(
            expr, 0, 1, fdecl,
            (isZero(expr->getArg(2))
             ? std::string("rtl::OUString::startsWithIgnoreAsciiCase")
             : std::string("rtl::OUString::matchIgnoreAsciiCase")),
            TreatEmpty::Error);
        return true;
    }
    if ((dc.Function("reverseCompareToAsciiL").Class("OUString")
         .Namespace("rtl").GlobalNamespace())
        && fdecl->getNumParams() == 2)
    {
        // u.reverseCompareToAsciiL("foo", 3) -> u.reverseCompareTo("foo"):
        handleCharLen(
            expr, 0, 1, fdecl, "rtl::OUString::reverseCompareTo",
            TreatEmpty::Error);
        return true;
    }
    if ((dc.Function("reverseCompareTo").Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 1)
    {
        handleOUStringCtor(expr, 0, fdecl, false);
        return true;
    }
    if ((dc.Function("equalsIgnoreAsciiCase").Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 1)
    {
        handleOUStringCtor(expr, 0, fdecl, false);
        return true;
    }
    if ((dc.Function("match").Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 2)
    {
        handleOUStringCtor(expr, 0, fdecl, false);
        return true;
    }
    if ((dc.Function("matchIgnoreAsciiCase").Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 2)
    {
        handleOUStringCtor(expr, 0, fdecl, false);
        return true;
    }
    if ((dc.Function("startsWith").Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 2)
    {
        handleOUStringCtor(expr, 0, fdecl, false);
        return true;
    }
    if ((dc.Function("startsWithIgnoreAsciiCase").Class("OUString")
         .Namespace("rtl").GlobalNamespace())
        && fdecl->getNumParams() == 2)
    {
        handleOUStringCtor(expr, 0, fdecl, false);
        return true;
    }
    if ((dc.Function("endsWith").Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 2)
    {
        handleOUStringCtor(expr, 0, fdecl, false);
        return true;
    }
    if ((dc.Function("endsWithIgnoreAsciiCase").Class("OUString")
         .Namespace("rtl").GlobalNamespace())
        && fdecl->getNumParams() == 2)
    {
        handleOUStringCtor(expr, 0, fdecl, false);
        return true;
    }
    if ((dc.Function("indexOf").Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 2)
    {
        handleOUStringCtor(expr, 0, fdecl, false);
        return true;
    }
    if ((dc.Function("lastIndexOf").Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 1)
    {
        handleOUStringCtor(expr, 0, fdecl, false);
        return true;
    }
    if ((dc.Function("replaceFirst").Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 3)
    {
        handleOUStringCtor(expr, 0, fdecl, false);
        handleOUStringCtor(expr, 1, fdecl, false);
        return true;
    }
    if ((dc.Function("replaceAll").Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        && (fdecl->getNumParams() == 2 || fdecl->getNumParams() == 3))
    {
        handleOUStringCtor(expr, 0, fdecl, false);
        handleOUStringCtor(expr, 1, fdecl, false);
        return true;
    }
    if ((dc.Operator(OO_PlusEqual).Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 1)
    {
        handleOUStringCtor(
            expr, dyn_cast<CXXOperatorCallExpr>(expr) == nullptr ? 0 : 1,
            fdecl, false);
        return true;
    }
    if ((dc.Function("equals").Class("OUString").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 1)
    {
        unsigned n;
        bool nonArray;
        ContentKind cont;
        bool emb;
        bool trm;
        if (!isStringConstant(
                expr->getArg(0)->IgnoreParenImpCasts(), &n, &nonArray, &cont,
                &emb, &trm))
        {
            return true;
        }
        if (cont != ContentKind::Ascii) {
            report(
                DiagnosticsEngine::Warning,
                ("call of '%0' with string constant argument containing"
                 " non-ASCII characters"),
                expr->getExprLoc())
                << fdecl->getQualifiedNameAsString() << expr->getSourceRange();
        }
        if (emb) {
            report(
                DiagnosticsEngine::Warning,
                ("call of '%0' with string constant argument containing"
                 " embedded NULLs"),
                expr->getExprLoc())
                << fdecl->getQualifiedNameAsString() << expr->getSourceRange();
        }
        if (n == 0) {
            report(
                DiagnosticsEngine::Warning,
                ("rewrite call of '%0' with empty string constant argument as"
                 " call of 'rtl::OUString::isEmpty'"),
                expr->getExprLoc())
                << fdecl->getQualifiedNameAsString() << expr->getSourceRange();
            return true;
        }
    }
    if (dc.Operator(OO_EqualEqual).Namespace("rtl").GlobalNamespace()
        && fdecl->getNumParams() == 2)
    {
        for (unsigned i = 0; i != 2; ++i) {
            unsigned n;
            bool nonArray;
            ContentKind cont;
            bool emb;
            bool trm;
            if (!isStringConstant(
                    expr->getArg(i)->IgnoreParenImpCasts(), &n, &nonArray,
                    &cont, &emb, &trm))
            {
                continue;
            }
            if (cont != ContentKind::Ascii) {
                report(
                    DiagnosticsEngine::Warning,
                    ("call of '%0' with string constant argument containing"
                     " non-ASCII characters"),
                    expr->getExprLoc())
                    << fdecl->getQualifiedNameAsString()
                    << expr->getSourceRange();
            }
            if (emb) {
                report(
                    DiagnosticsEngine::Warning,
                    ("call of '%0' with string constant argument containing"
                     " embedded NULLs"),
                    expr->getExprLoc())
                    << fdecl->getQualifiedNameAsString()
                    << expr->getSourceRange();
            }
            if (n == 0) {
                report(
                    DiagnosticsEngine::Warning,
                    ("rewrite call of '%0' with empty string constant argument"
                     " as call of 'rtl::OUString::isEmpty'"),
                    expr->getExprLoc())
                    << fdecl->getQualifiedNameAsString()
                    << expr->getSourceRange();
            }
        }
        return true;
    }
    if (dc.Operator(OO_ExclaimEqual).Namespace("rtl").GlobalNamespace()
        && fdecl->getNumParams() == 2)
    {
        for (unsigned i = 0; i != 2; ++i) {
            unsigned n;
            bool nonArray;
            ContentKind cont;
            bool emb;
            bool trm;
            if (!isStringConstant(
                    expr->getArg(i)->IgnoreParenImpCasts(), &n, &nonArray,
                    &cont, &emb, &trm))
            {
                continue;
            }
            if (cont != ContentKind::Ascii) {
                report(
                    DiagnosticsEngine::Warning,
                    ("call of '%0' with string constant argument containing"
                     " non-ASCII characters"),
                    expr->getExprLoc())
                    << fdecl->getQualifiedNameAsString()
                    << expr->getSourceRange();
            }
            if (emb) {
                report(
                    DiagnosticsEngine::Warning,
                    ("call of '%0' with string constant argument containing"
                     " embedded NULLs"),
                    expr->getExprLoc())
                    << fdecl->getQualifiedNameAsString()
                    << expr->getSourceRange();
            }
            if (n == 0) {
                report(
                    DiagnosticsEngine::Warning,
                    ("rewrite call of '%0' with empty string constant argument"
                     " as call of '!rtl::OUString::isEmpty'"),
                    expr->getExprLoc())
                    << fdecl->getQualifiedNameAsString()
                    << expr->getSourceRange();
            }
        }
        return true;
    }
    if (dc.Operator(OO_Equal).Namespace("rtl").GlobalNamespace()
        && fdecl->getNumParams() == 1)
    {
        unsigned n;
        bool nonArray;
        ContentKind cont;
        bool emb;
        bool trm;
        if (!isStringConstant(
                expr->getArg(1)->IgnoreParenImpCasts(), &n, &nonArray, &cont,
                &emb, &trm))
        {
            return true;
        }
        if (cont != ContentKind::Ascii) {
            report(
                DiagnosticsEngine::Warning,
                ("call of '%0' with string constant argument containing"
                 " non-ASCII characters"),
                expr->getExprLoc())
                << fdecl->getQualifiedNameAsString() << expr->getSourceRange();
        }
        if (emb) {
            report(
                DiagnosticsEngine::Warning,
                ("call of '%0' with string constant argument containing"
                 " embedded NULLs"),
                expr->getExprLoc())
                << fdecl->getQualifiedNameAsString() << expr->getSourceRange();
        }
        if (n == 0) {
            report(
                DiagnosticsEngine::Warning,
                ("rewrite call of '%0' with empty string constant argument as"
                 " call of 'rtl::OUString::clear'"),
                expr->getExprLoc())
                << fdecl->getQualifiedNameAsString() << expr->getSourceRange();
            return true;
        }
        return true;
    }
    if (dc.Function("append").Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
        && fdecl->getNumParams() == 1)
    {
        handleChar(expr, 0, fdecl, "", TreatEmpty::Error, false);
        return true;
    }
    if ((dc.Function("appendAscii").Class("OUStringBuffer").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 1)
    {
        // u.appendAscii("foo") -> u.append("foo")
        handleChar(
            expr, 0, fdecl, "rtl::OUStringBuffer::append", TreatEmpty::Error,
            true, "appendAscii", "append");
        return true;
    }
    if ((dc.Function("appendAscii").Class("OUStringBuffer").Namespace("rtl")
         .GlobalNamespace())
        && fdecl->getNumParams() == 2)
    {
        // u.appendAscii("foo", 3) -> u.append("foo"):
        handleCharLen(
            expr, 0, 1, fdecl, "rtl::OUStringBuffer::append",
            TreatEmpty::Error);
        return true;
    }
    if (dc.Function("append").Class("OStringBuffer").Namespace("rtl")
        .GlobalNamespace())
    {
        switch (fdecl->getNumParams()) {
        case 1:
            handleFunArgOstring(expr, 0, fdecl);
            break;
        case 2:
            {
                // b.append("foo", 3) -> b.append("foo"):
                auto file = getFilenameOfLocation(
                    compiler.getSourceManager().getSpellingLoc(
                        compat::getBeginLoc(expr)));
                if (loplugin::isSamePathname(
                        file,
                        SRCDIR "/sal/qa/OStringBuffer/rtl_OStringBuffer.cxx"))
                {
                    return true;
                }
                handleCharLen(
                    expr, 0, 1, fdecl, "rtl::OStringBuffer::append",
                    TreatEmpty::Error);
            }
            break;
        default:
            break;
        }
        return true;
    }
    if (dc.Function("insert").Class("OStringBuffer").Namespace("rtl")
        .GlobalNamespace())
    {
        switch (fdecl->getNumParams()) {
        case 2:
            handleFunArgOstring(expr, 1, fdecl);
            break;
        case 3:
            {
                // b.insert(i, "foo", 3) -> b.insert(i, "foo"):
                handleCharLen(
                    expr, 1, 2, fdecl, "rtl::OStringBuffer::insert",
                    TreatEmpty::Error);
                break;
            }
        default:
            break;
        }
        return true;
    }
    return true;
}

bool StringConstant::VisitCXXConstructExpr(CXXConstructExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    auto classdecl = expr->getConstructor()->getParent();
    if (loplugin::DeclCheck(classdecl)
        .Class("OUString").Namespace("rtl").GlobalNamespace())
    {
        ChangeKind kind;
        PassThrough pass;
        bool simplify;
        switch (expr->getConstructor()->getNumParams()) {
        case 1:
            if (!loplugin::TypeCheck(
                    expr->getConstructor()->getParamDecl(0)->getType())
                .Typedef("sal_Unicode").GlobalNamespace())
            {
                return true;
            }
            kind = ChangeKind::SingleChar;
            pass = PassThrough::NonEmptyConstantString;
            simplify = false;
            break;
        case 2:
            {
                auto arg = expr->getArg(0);
                if (loplugin::TypeCheck(arg->getType())
                    .Class("OUStringChar_").Namespace("rtl")
                    .GlobalNamespace())
                {
                    kind = ChangeKind::OUStringChar;
                    pass = PassThrough::NonEmptyConstantString;
                    simplify = false;
                } else {
                    unsigned n;
                    bool nonArray;
                    ContentKind cont;
                    bool emb;
                    bool trm;
                    if (!isStringConstant(
                            arg->IgnoreParenImpCasts(), &n, &nonArray, &cont,
                            &emb, &trm))
                    {
                        return true;
                    }
                    if (cont != ContentKind::Ascii) {
                        report(
                            DiagnosticsEngine::Warning,
                            ("construction of %0 with string constant argument"
                             " containing non-ASCII characters"),
                            expr->getExprLoc())
                            << classdecl << expr->getSourceRange();
                    }
                    if (emb) {
                        report(
                            DiagnosticsEngine::Warning,
                            ("construction of %0 with string constant argument"
                             " containing embedded NULLs"),
                            expr->getExprLoc())
                            << classdecl << expr->getSourceRange();
                    }
                    kind = ChangeKind::Char;
                    pass = n == 0
                        ? PassThrough::EmptyConstantString
                        : PassThrough::NonEmptyConstantString;
                    simplify = false;
                }
                break;
            }
        case 4:
            {
                unsigned n;
                bool nonArray;
                ContentKind cont;
                bool emb;
                bool trm;
                std::vector<char32_t> utf8Cont;
                if (!isStringConstant(
                        expr->getArg(0)->IgnoreParenImpCasts(), &n, &nonArray,
                        &cont, &emb, &trm, &utf8Cont))
                {
                    return true;
                }
                APSInt res;
                if (!compat::EvaluateAsInt(expr->getArg(1),
                        res, compiler.getASTContext()))
                {
                    return true;
                }
                if (res != n) {
                    report(
                        DiagnosticsEngine::Warning,
                        ("suspicious 'rtl::OUString' constructor with literal"
                         " of length %0 and non-matching length argument %1"),
                        expr->getExprLoc())
                        << n << res.toString(10) << expr->getSourceRange();
                    return true;
                }
                APSInt enc;
                if (!compat::EvaluateAsInt(expr->getArg(2),
                        enc, compiler.getASTContext()))
                {
                    return true;
                }
                auto const encIsAscii = enc == 11; // RTL_TEXTENCODING_ASCII_US
                auto const encIsUtf8 = enc == 76; // RTL_TEXTENCODING_UTF8
                if (!compat::EvaluateAsInt(expr->getArg(3),
                        res, compiler.getASTContext())
                    || res != 0x333) // OSTRING_TO_OUSTRING_CVTFLAGS
                {
                    return true;
                }
                if (!encIsAscii && cont == ContentKind::Ascii) {
                    report(
                        DiagnosticsEngine::Warning,
                        ("suspicious 'rtl::OUString' constructor with text"
                         " encoding %0 but plain ASCII content; use"
                         " 'RTL_TEXTENCODING_ASCII_US' instead"),
                        expr->getArg(2)->getExprLoc())
                        << enc.toString(10) << expr->getSourceRange();
                    return true;
                }
                if (encIsUtf8) {
                    if (cont == ContentKind::Arbitrary) {
                        report(
                            DiagnosticsEngine::Warning,
                            ("suspicious 'rtl::OUString' constructor with text"
                             " encoding 'RTL_TEXTENCODING_UTF8' but non-UTF-8"
                             " content"),
                            expr->getArg(0)->getExprLoc())
                            << expr->getSourceRange();
                    } else {
                        assert(cont == ContentKind::Utf8);
                        //TODO: keep original content as much as possible
                        std::ostringstream s;
                        for (auto const c: utf8Cont) {
                            if (c == '\\') {
                                s << "\\\\";
                            } else if (c == '"') {
                                s << "\\\"";
                            } else if (c == '\a') {
                                s << "\\a";
                            } else if (c == '\b') {
                                s << "\\b";
                            } else if (c == '\f') {
                                s << "\\f";
                            } else if (c == '\n') {
                                s << "\\n";
                            } else if (c == '\r') {
                                s << "\\r";
                            } else if (c == '\t') {
                                s << "\\r";
                            } else if (c == '\v') {
                                s << "\\v";
                            } else if (c <= 0x1F || c == 0x7F) {
                                s << "\\x" << std::oct << std::setw(3)
                                  << std::setfill('0')
                                  << static_cast<std::uint_least32_t>(c);
                            } else if (c < 0x7F) {
                                s << char(c);
                            } else if (c <= 0xFFFF) {
                                s << "\\u" << std::hex << std::uppercase
                                  << std::setw(4) << std::setfill('0')
                                  << static_cast<std::uint_least32_t>(c);
                            } else {
                                assert(c <= 0x10FFFF);
                                s << "\\U" << std::hex << std::uppercase
                                  << std::setw(8) << std::setfill('0')
                                  << static_cast<std::uint_least32_t>(c);
                            }
                        }
                        report(
                            DiagnosticsEngine::Warning,
                            ("simplify construction of %0 with UTF-8 content as"
                             " OUString(u\"%1\")"),
                            expr->getExprLoc())
                            << classdecl << s.str() << expr->getSourceRange();

                    }
                    return true;
                }
                if (cont != ContentKind::Ascii || emb) {
                    // cf. remaining uses of RTL_CONSTASCII_USTRINGPARAM
                    return true;
                }
                kind = ChangeKind::Char;
                pass = n == 0
                    ? PassThrough::EmptyConstantString
                    : PassThrough::NonEmptyConstantString;
                simplify = true;
                break;
            }
        default:
            return true;
        }
        if (!calls_.empty()) {
            Expr const * call = calls_.top();
            CallExpr::const_arg_iterator argsBeg;
            CallExpr::const_arg_iterator argsEnd;
            if (isa<CallExpr>(call)) {
                argsBeg = cast<CallExpr>(call)->arg_begin();
                argsEnd = cast<CallExpr>(call)->arg_end();
            } else if (isa<CXXConstructExpr>(call)) {
                argsBeg = cast<CXXConstructExpr>(call)->arg_begin();
                argsEnd = cast<CXXConstructExpr>(call)->arg_end();
            } else {
                assert(false);
            }
            for (auto i(argsBeg); i != argsEnd; ++i) {
                Expr const * e = (*i)->IgnoreParenImpCasts();
                if (isa<MaterializeTemporaryExpr>(e)) {
                    e = compat::getSubExpr(cast<MaterializeTemporaryExpr>(e))
                        ->IgnoreParenImpCasts();
                }
                if (isa<CXXFunctionalCastExpr>(e)) {
                    e = cast<CXXFunctionalCastExpr>(e)->getSubExpr()
                        ->IgnoreParenImpCasts();
                }
                if (isa<CXXBindTemporaryExpr>(e)) {
                    e = cast<CXXBindTemporaryExpr>(e)->getSubExpr()
                        ->IgnoreParenImpCasts();
                }
                if (e == expr) {
                    if (isa<CallExpr>(call)) {
                        FunctionDecl const * fdecl
                            = cast<CallExpr>(call)->getDirectCallee();
                        if (fdecl == nullptr) {
                            break;
                        }
                        loplugin::DeclCheck dc(fdecl);
                        if (pass == PassThrough::EmptyConstantString) {
                            if ((dc.Function("equals").Class("OUString")
                                 .Namespace("rtl").GlobalNamespace())
                                || (dc.Operator(OO_EqualEqual).Namespace("rtl")
                                    .GlobalNamespace()))
                            {
                                report(
                                    DiagnosticsEngine::Warning,
                                    ("rewrite call of '%0' with construction of"
                                     " %1 with empty string constant argument"
                                     " as call of 'rtl::OUString::isEmpty'"),
                                    getMemberLocation(call))
                                    << fdecl->getQualifiedNameAsString()
                                    << classdecl << call->getSourceRange();
                                return true;
                            }
                            if (dc.Operator(OO_ExclaimEqual).Namespace("rtl")
                                    .GlobalNamespace())
                            {
                                report(
                                    DiagnosticsEngine::Warning,
                                    ("rewrite call of '%0' with construction of"
                                     " %1 with empty string constant argument"
                                     " as call of '!rtl::OUString::isEmpty'"),
                                    getMemberLocation(call))
                                    << fdecl->getQualifiedNameAsString()
                                    << classdecl << call->getSourceRange();
                                return true;
                            }
                            if ((dc.Operator(OO_Plus).Namespace("rtl")
                                    .GlobalNamespace())
                                || (dc.Operator(OO_Plus).Class("OUString")
                                    .Namespace("rtl").GlobalNamespace()))
                            {
                                report(
                                    DiagnosticsEngine::Warning,
                                    ("call of '%0' with suspicious construction"
                                     " of %1 with empty string constant"
                                     " argument"),
                                    getMemberLocation(call))
                                    << fdecl->getQualifiedNameAsString()
                                    << classdecl << call->getSourceRange();
                                return true;
                            }
                            if (dc.Operator(OO_Equal).Class("OUString")
                                .Namespace("rtl").GlobalNamespace())
                            {
                                report(
                                    DiagnosticsEngine::Warning,
                                    ("rewrite call of '%0' with construction of"
                                     " %1 with empty string constant argument"
                                     " as call of 'rtl::OUString::clear'"),
                                    getMemberLocation(call))
                                    << fdecl->getQualifiedNameAsString()
                                    << classdecl << call->getSourceRange();
                                return true;
                            }
                        } else {
                            assert(pass == PassThrough::NonEmptyConstantString);
                            if (dc.Function("equals").Class("OUString")
                                .Namespace("rtl").GlobalNamespace())
                            {
                                report(
                                    DiagnosticsEngine::Warning,
                                    ("rewrite call of '%0' with construction of"
                                     " %1 with %2 as 'operator =='"),
                                    getMemberLocation(call))
                                    << fdecl->getQualifiedNameAsString()
                                    << classdecl << describeChangeKind(kind)
                                    << call->getSourceRange();
                                return true;
                            }
                            if ((dc.Operator(OO_Plus).Namespace("rtl")
                                    .GlobalNamespace())
                                || (dc.Operator(OO_Plus).Class("OUString")
                                    .Namespace("rtl").GlobalNamespace())
                                || (dc.Operator(OO_EqualEqual).Namespace("rtl")
                                    .GlobalNamespace())
                                || (dc.Operator(OO_ExclaimEqual)
                                    .Namespace("rtl").GlobalNamespace()))
                            {
                                if (dc.Operator(OO_Plus).Namespace("rtl")
                                    .GlobalNamespace())
                                {
                                    auto file = getFilenameOfLocation(
                                            compiler.getSourceManager()
                                            .getSpellingLoc(
                                                compat::getBeginLoc(expr)));
                                    if (loplugin::isSamePathname(
                                            file,
                                            (SRCDIR
                                             "/sal/qa/rtl/strings/test_ostring_concat.cxx"))
                                        || loplugin::isSamePathname(
                                            file,
                                            (SRCDIR
                                             "/sal/qa/rtl/strings/test_oustring_concat.cxx")))
                                    {
                                        return true;
                                    }
                                }
                                auto loc = compat::getBeginLoc(expr->getArg(0));
                                while (compiler.getSourceManager()
                                       .isMacroArgExpansion(loc))
                                {
                                    loc = compiler.getSourceManager()
                                        .getImmediateMacroCallerLoc(loc);
                                }
                                if (kind == ChangeKind::SingleChar) {
                                    report(
                                        DiagnosticsEngine::Warning,
                                        ("rewrite construction of %0 with %1 in"
                                         " call of '%2' as construction of"
                                         " 'OUStringChar'"),
                                        getMemberLocation(expr))
                                        << classdecl << describeChangeKind(kind)
                                        << fdecl->getQualifiedNameAsString()
                                        << expr->getSourceRange();
                                } else {
                                    report(
                                        DiagnosticsEngine::Warning,
                                        ("elide construction of %0 with %1 in"
                                         " call of '%2'"),
                                        getMemberLocation(expr))
                                        << classdecl << describeChangeKind(kind)
                                        << fdecl->getQualifiedNameAsString()
                                        << expr->getSourceRange();
                                }
                                return true;
                            }
                        }
                    } else if (isa<CXXConstructExpr>(call)) {
                    } else {
                        assert(false);
                    }
                }
            }
        }
        if (simplify) {
            report(
                DiagnosticsEngine::Warning,
                "simplify construction of %0 with %1", expr->getExprLoc())
                << classdecl << describeChangeKind(kind)
                << expr->getSourceRange();
        }
        return true;
    }

    auto consDecl = expr->getConstructor();
    for (unsigned i = 0; i != consDecl->getNumParams(); ++i) {
        auto t = consDecl->getParamDecl(i)->getType();
        if (loplugin::TypeCheck(t).NotSubstTemplateTypeParmType()
            .LvalueReference().Const().NotSubstTemplateTypeParmType()
            .Class("OUString").Namespace("rtl").GlobalNamespace())
        {
            auto argExpr = expr->getArg(i);
            if (argExpr && i <= consDecl->getNumParams())
            {
                if (!hasOverloads(consDecl, expr->getNumArgs()))
                {
                    handleOUStringCtor(expr, argExpr, consDecl, true);
                }
            }
        }
        if (loplugin::TypeCheck(t).NotSubstTemplateTypeParmType()
            .LvalueReference().Const().NotSubstTemplateTypeParmType()
            .Class("OString").Namespace("rtl").GlobalNamespace())
        {
            auto argExpr = expr->getArg(i);
            if (argExpr && i <= consDecl->getNumParams())
            {
                if (!hasOverloads(consDecl, expr->getNumArgs()))
                {
                    handleOStringCtor(expr, argExpr, consDecl, true);
                }
            }
        }
    }

    return true;
}

bool StringConstant::VisitReturnStmt(ReturnStmt const * stmt) {
    if (ignoreLocation(stmt)) {
        return true;
    }
    auto const e1 = stmt->getRetValue();
    if (e1 == nullptr) {
        return true;
    }
    auto const tc1 = loplugin::TypeCheck(e1->getType().getTypePtr());
    if (!(tc1.Class("OString").Namespace("rtl").GlobalNamespace()
          || tc1.Class("OUString").Namespace("rtl").GlobalNamespace()))
    {
        return true;
    }
    assert(!returnTypes_.empty());
    auto const tc2 = loplugin::TypeCheck(returnTypes_.top().getTypePtr());
    if (!(tc2.Class("OString").Namespace("rtl").GlobalNamespace()
          || tc2.Class("OUString").Namespace("rtl").GlobalNamespace()))
    {
        return true;
    }
    auto const e2 = dyn_cast<CXXFunctionalCastExpr>(e1->IgnoreImplicit());
    if (e2 == nullptr) {
        return true;
    }
    auto const e3 = dyn_cast<CXXBindTemporaryExpr>(e2->getSubExpr());
    if (e3 == nullptr) {
        return true;
    }
    auto const e4 = dyn_cast<CXXConstructExpr>(e3->getSubExpr());
    if (e4 == nullptr) {
        return true;
    }
    if (e4->getNumArgs() != 2) {
        return true;
    }
    auto const t = e4->getArg(0)->getType();
    if (!(t.isConstQualified() && t->isConstantArrayType())) {
        return true;
    }
    auto const e5 = e4->getArg(1);
    if (!(isa<CXXDefaultArgExpr>(e5)
          && (loplugin::TypeCheck(e5->getType()).Struct("Dummy").Namespace("libreoffice_internal")
              .Namespace("rtl").GlobalNamespace())))
    {
        return true;
    }
    report(DiagnosticsEngine::Warning, "elide constructor call", compat::getBeginLoc(e1))
        << e1->getSourceRange();
    return true;
}

std::string StringConstant::describeChangeKind(ChangeKind kind) {
    switch (kind) {
    case ChangeKind::Char:
        return "string constant argument";
    case ChangeKind::CharLen:
        return "string constant and matching length arguments";
    case ChangeKind::SingleChar:
        return "sal_Unicode argument";
    case ChangeKind::OUStringChar:
        return "OUStringChar argument";
    }
    llvm_unreachable("unknown change kind");
}

bool StringConstant::isStringConstant(
    Expr const * expr, unsigned * size, bool * nonArray, ContentKind * content,
    bool * embeddedNuls, bool * terminatingNul,
    std::vector<char32_t> * utf8Content)
{
    assert(expr != nullptr);
    assert(size != nullptr);
    assert(nonArray != nullptr);
    assert(content != nullptr);
    assert(embeddedNuls != nullptr);
    assert(terminatingNul != nullptr);
    QualType t = expr->getType();
    // Look inside RTL_CONSTASCII_STRINGPARAM:
    if (loplugin::TypeCheck(t).Pointer().Const().Char()) {
        auto e2 = dyn_cast<UnaryOperator>(expr);
        if (e2 != nullptr && e2->getOpcode() == UO_AddrOf) {
            auto e3 = dyn_cast<ArraySubscriptExpr>(
                e2->getSubExpr()->IgnoreParenImpCasts());
            if (e3 == nullptr || !isZero(e3->getIdx()->IgnoreParenImpCasts())) {
                return false;
            }
            expr = e3->getBase()->IgnoreParenImpCasts();
            t = expr->getType();
        }
    }
    if (!t.isConstQualified()) {
        return false;
    }
    DeclRefExpr const * dre = dyn_cast<DeclRefExpr>(expr);
    if (dre != nullptr) {
        VarDecl const * var = dyn_cast<VarDecl>(dre->getDecl());
        if (var != nullptr) {
            Expr const * init = var->getAnyInitializer();
            if (init != nullptr) {
                expr = init->IgnoreParenImpCasts();
            }
        }
    }
    bool isPtr;
    if (loplugin::TypeCheck(t).Pointer().Const().Char()) {
        isPtr = true;
    } else if (t->isConstantArrayType()
               && (loplugin::TypeCheck(
                       t->getAsArrayTypeUnsafe()->getElementType())
                   .Char()))
    {
        isPtr = false;
    } else {
        return false;
    }
    clang::StringLiteral const * lit = dyn_cast<clang::StringLiteral>(expr);
    if (lit != nullptr) {
        if (!(lit->isAscii() || lit->isUTF8())) {
            return false;
        }
        unsigned n = lit->getLength();
        ContentKind cont = ContentKind::Ascii;
        bool emb = false;
        char32_t val = 0;
        enum class Utf8State { Start, E0, EB, F0, F4, Trail1, Trail2, Trail3 };
        Utf8State s = Utf8State::Start;
        StringRef str = lit->getString();
        for (unsigned i = 0; i != n; ++i) {
            auto const c = static_cast<unsigned char>(str[i]);
            if (c == '\0') {
                emb = true;
            }
            switch (s) {
            case Utf8State::Start:
                if (c >= 0x80) {
                    if (c >= 0xC2 && c <= 0xDF) {
                        val = c & 0x1F;
                        s = Utf8State::Trail1;
                    } else if (c == 0xE0) {
                        val = c & 0x0F;
                        s = Utf8State::E0;
                    } else if ((c >= 0xE1 && c <= 0xEA)
                               || (c >= 0xEE && c <= 0xEF))
                    {
                        val = c & 0x0F;
                        s = Utf8State::Trail2;
                    } else if (c == 0xEB) {
                        val = c & 0x0F;
                        s = Utf8State::EB;
                    } else if (c == 0xF0) {
                        val = c & 0x03;
                        s = Utf8State::F0;
                    } else if (c >= 0xF1 && c <= 0xF3) {
                        val = c & 0x03;
                        s = Utf8State::Trail3;
                    } else if (c == 0xF4) {
                        val = c & 0x03;
                        s = Utf8State::F4;
                    } else {
                        cont = ContentKind::Arbitrary;
                    }
                } else if (utf8Content != nullptr
                           && cont != ContentKind::Arbitrary)
                {
                    utf8Content->push_back(c);
                }
                break;
            case Utf8State::E0:
                if (c >= 0xA0 && c <= 0xBF) {
                    val = (val << 6) | (c & 0x3F);
                    s = Utf8State::Trail1;
                } else {
                    cont = ContentKind::Arbitrary;
                    s = Utf8State::Start;
                }
                break;
            case Utf8State::EB:
                if (c >= 0x80 && c <= 0x9F) {
                    val = (val << 6) | (c & 0x3F);
                    s = Utf8State::Trail1;
                } else {
                    cont = ContentKind::Arbitrary;
                    s = Utf8State::Start;
                }
                break;
            case Utf8State::F0:
                if (c >= 0x90 && c <= 0xBF) {
                    val = (val << 6) | (c & 0x3F);
                    s = Utf8State::Trail2;
                } else {
                    cont = ContentKind::Arbitrary;
                    s = Utf8State::Start;
                }
                break;
            case Utf8State::F4:
                if (c >= 0x80 && c <= 0x8F) {
                    val = (val << 6) | (c & 0x3F);
                    s = Utf8State::Trail2;
                } else {
                    cont = ContentKind::Arbitrary;
                    s = Utf8State::Start;
                }
                break;
            case Utf8State::Trail1:
                if (c >= 0x80 && c <= 0xBF) {
                    cont = ContentKind::Utf8;
                    if (utf8Content != nullptr)
                    {
                        utf8Content->push_back((val << 6) | (c & 0x3F));
                        val = 0;
                    }
                } else {
                    cont = ContentKind::Arbitrary;
                }
                s = Utf8State::Start;
                break;
            case Utf8State::Trail2:
                if (c >= 0x80 && c <= 0xBF) {
                    val = (val << 6) | (c & 0x3F);
                    s = Utf8State::Trail1;
                } else {
                    cont = ContentKind::Arbitrary;
                    s = Utf8State::Start;
                }
                break;
            case Utf8State::Trail3:
                if (c >= 0x80 && c <= 0xBF) {
                    val = (val << 6) | (c & 0x3F);
                    s = Utf8State::Trail2;
                } else {
                    cont = ContentKind::Arbitrary;
                    s = Utf8State::Start;
                }
                break;
            }
        }
        *size = n;
        *nonArray = isPtr;
        *content = cont;
        *embeddedNuls = emb;
        *terminatingNul = true;
        return true;
    }
    APValue v;
    if (!expr->isCXX11ConstantExpr(compiler.getASTContext(), &v)) {
        return false;
    }
    switch (v.getKind()) {
    case APValue::LValue:
        {
            Expr const * e = v.getLValueBase().dyn_cast<Expr const *>();
            if (e == nullptr) {
                return false;
            }
            if (!v.getLValueOffset().isZero()) {
                return false; //TODO
            }
            Expr const * e2 = e->IgnoreParenImpCasts();
            if (e2 != e) {
                return isStringConstant(
                    e2, size, nonArray, content, embeddedNuls, terminatingNul);
            }
            //TODO: string literals are represented as recursive LValues???
            llvm::APInt n
                = compiler.getASTContext().getAsConstantArrayType(t)->getSize();
            assert(n != 0);
            --n;
            assert(n.ule(std::numeric_limits<unsigned>::max()));
            *size = static_cast<unsigned>(n.getLimitedValue());
            *nonArray = isPtr || *nonArray;
            *content = ContentKind::Ascii; //TODO
            *embeddedNuls = false; //TODO
            *terminatingNul = true;
            return true;
        }
    case APValue::Array:
        {
            if (v.hasArrayFiller()) { //TODO: handle final NULL filler?
                return false;
            }
            unsigned n = v.getArraySize();
            assert(n != 0);
            ContentKind cont = ContentKind::Ascii;
            bool emb = false;
            //TODO: check for ContentType::Utf8
            for (unsigned i = 0; i != n - 1; ++i) {
                APValue e(v.getArrayInitializedElt(i));
                if (!e.isInt()) { //TODO: assert?
                    return false;
                }
                APSInt iv = e.getInt();
                if (iv == 0) {
                    emb = true;
                } else if (iv.uge(0x80)) {
                    cont = ContentKind::Arbitrary;
                }
            }
            APValue e(v.getArrayInitializedElt(n - 1));
            if (!e.isInt()) { //TODO: assert?
                return false;
            }
            bool trm = e.getInt() == 0;
            *size = trm ? n - 1 : n;
            *nonArray = isPtr;
            *content = cont;
            *embeddedNuls = emb;
            *terminatingNul = trm;
            return true;
        }
    default:
        assert(false); //TODO???
        return false;
    }
}

bool StringConstant::isZero(Expr const * expr) {
    APSInt res;
    return compat::EvaluateAsInt(expr, res, compiler.getASTContext()) && res == 0;
}

void StringConstant::reportChange(
    Expr const * expr, ChangeKind kind, std::string const & original,
    std::string const & replacement, PassThrough pass, bool nonArray,
    char const * rewriteFrom, char const * rewriteTo)
{
    assert((rewriteFrom == nullptr) == (rewriteTo == nullptr));
    if (pass != PassThrough::No && !calls_.empty()) {
        Expr const * call = calls_.top();
        CallExpr::const_arg_iterator argsBeg;
        CallExpr::const_arg_iterator argsEnd;
        if (isa<CallExpr>(call)) {
            argsBeg = cast<CallExpr>(call)->arg_begin();
            argsEnd = cast<CallExpr>(call)->arg_end();
        } else if (isa<CXXConstructExpr>(call)) {
            argsBeg = cast<CXXConstructExpr>(call)->arg_begin();
            argsEnd = cast<CXXConstructExpr>(call)->arg_end();
        } else {
            assert(false);
        }
        for (auto i(argsBeg); i != argsEnd; ++i) {
            Expr const * e = (*i)->IgnoreParenImpCasts();
            if (isa<CXXBindTemporaryExpr>(e)) {
                e = cast<CXXBindTemporaryExpr>(e)->getSubExpr()
                    ->IgnoreParenImpCasts();
            }
            if (e == expr) {
                if (isa<CallExpr>(call)) {
                    FunctionDecl const * fdecl
                        = cast<CallExpr>(call)->getDirectCallee();
                    if (fdecl == nullptr) {
                        break;
                    }
                    loplugin::DeclCheck dc(fdecl);
                    if (pass == PassThrough::EmptyConstantString) {
                        if ((dc.Function("equals").Class("OUString")
                             .Namespace("rtl").GlobalNamespace())
                            || (dc.Operator(OO_EqualEqual).Namespace("rtl")
                                .GlobalNamespace()))
                        {
                            report(
                                DiagnosticsEngine::Warning,
                                ("rewrite call of '%0' with call of %1 with"
                                 " empty string constant argument as call of"
                                 " 'rtl::OUString::isEmpty'"),
                                getMemberLocation(call))
                                << fdecl->getQualifiedNameAsString() << original
                                << call->getSourceRange();
                            return;
                        }
                        if (dc.Operator(OO_ExclaimEqual).Namespace("rtl")
                            .GlobalNamespace())
                        {
                            report(
                                DiagnosticsEngine::Warning,
                                ("rewrite call of '%0' with call of %1 with"
                                 " empty string constant argument as call of"
                                 " '!rtl::OUString::isEmpty'"),
                                getMemberLocation(call))
                                << fdecl->getQualifiedNameAsString() << original
                                << call->getSourceRange();
                            return;
                        }
                        if ((dc.Operator(OO_Plus).Namespace("rtl")
                             .GlobalNamespace())
                            || (dc.Operator(OO_Plus).Class("OUString")
                                .Namespace("rtl").GlobalNamespace()))
                        {
                            report(
                                DiagnosticsEngine::Warning,
                                ("call of '%0' with suspicious call of %1 with"
                                 " empty string constant argument"),
                                getMemberLocation(call))
                                << fdecl->getQualifiedNameAsString() << original
                                << call->getSourceRange();
                            return;
                        }
                        if (dc.Operator(OO_Equal).Class("OUString")
                            .Namespace("rtl").GlobalNamespace())
                        {
                            report(
                                DiagnosticsEngine::Warning,
                                ("rewrite call of '%0' with call of %1 with"
                                 " empty string constant argument as call of"
                                 " rtl::OUString::call"),
                                getMemberLocation(call))
                                << fdecl->getQualifiedNameAsString() << original
                                << call->getSourceRange();
                            return;
                        }
                        report(
                            DiagnosticsEngine::Warning,
                            "TODO call inside %0", getMemberLocation(expr))
                            << fdecl->getQualifiedNameAsString()
                            << expr->getSourceRange();
                        return;
                    } else {
                        assert(pass == PassThrough::NonEmptyConstantString);
                        if ((dc.Function("equals").Class("OUString")
                             .Namespace("rtl").GlobalNamespace())
                            || (dc.Operator(OO_Equal).Class("OUString")
                                .Namespace("rtl").GlobalNamespace())
                            || (dc.Operator(OO_EqualEqual).Namespace("rtl")
                                .GlobalNamespace())
                            || (dc.Operator(OO_ExclaimEqual).Namespace("rtl")
                                .GlobalNamespace()))
                        {
                            report(
                                DiagnosticsEngine::Warning,
                                "elide call of %0 with %1 in call of '%2'",
                                getMemberLocation(expr))
                                << original << describeChangeKind(kind)
                                << fdecl->getQualifiedNameAsString()
                                << expr->getSourceRange();
                            return;
                        }
                        report(
                            DiagnosticsEngine::Warning,
                            ("rewrite call of %0 with %1 in call of '%2' as"
                             " (implicit) construction of 'OUString'"),
                            getMemberLocation(expr))
                            << original << describeChangeKind(kind)
                            << fdecl->getQualifiedNameAsString()
                            << expr->getSourceRange();
                        return;
                    }
                } else if (isa<CXXConstructExpr>(call)) {
                    auto classdecl = cast<CXXConstructExpr>(call)
                        ->getConstructor()->getParent();
                    loplugin::DeclCheck dc(classdecl);
                    if (dc.Class("OUString").Namespace("rtl").GlobalNamespace()
                        || (dc.Class("OUStringBuffer").Namespace("rtl")
                            .GlobalNamespace()))
                    {
                        //TODO: propagate further out?
                        if (pass == PassThrough::EmptyConstantString) {
                            report(
                                DiagnosticsEngine::Warning,
                                ("rewrite construction of %0 with call of %1"
                                 " with empty string constant argument as"
                                 " default construction of %0"),
                                getMemberLocation(call))
                                << classdecl << original
                                << call->getSourceRange();
                        } else {
                            assert(pass == PassThrough::NonEmptyConstantString);
                            report(
                                DiagnosticsEngine::Warning,
                                ("elide call of %0 with %1 in construction of"
                                 " %2"),
                                getMemberLocation(expr))
                                << original << describeChangeKind(kind)
                                << classdecl << expr->getSourceRange();
                        }
                        return;
                    }
                } else {
                    assert(false);
                }
            }
        }
    }
    if (rewriter != nullptr && !nonArray && rewriteFrom != nullptr) {
        SourceLocation loc = getMemberLocation(expr);
        while (compiler.getSourceManager().isMacroArgExpansion(loc)) {
            loc = compiler.getSourceManager().getImmediateMacroCallerLoc(loc);
        }
        if (compiler.getSourceManager().isMacroBodyExpansion(loc)) {
            loc = compiler.getSourceManager().getSpellingLoc(loc);
        }
        unsigned n = Lexer::MeasureTokenLength(
            loc, compiler.getSourceManager(), compiler.getLangOpts());
        if ((std::string(compiler.getSourceManager().getCharacterData(loc), n)
             == rewriteFrom)
            && replaceText(loc, n, rewriteTo))
        {
            return;
        }
    }
    report(
        DiagnosticsEngine::Warning,
        "rewrite call of '%0' with %1 as call of '%2'%3",
        getMemberLocation(expr))
        << original << describeChangeKind(kind) << replacement
        << adviseNonArray(nonArray) << expr->getSourceRange();
}

void StringConstant::checkEmpty(
    CallExpr const * expr, FunctionDecl const * callee, TreatEmpty treatEmpty,
    unsigned size, std::string * replacement)
{
    assert(replacement != nullptr);
    if (size == 0) {
        switch (treatEmpty) {
        case TreatEmpty::DefaultCtor:
            *replacement = "rtl::OUString default constructor";
            break;
        case TreatEmpty::CheckEmpty:
            *replacement = "rtl::OUString::isEmpty";
            break;
        case TreatEmpty::Error:
            report(
                DiagnosticsEngine::Warning,
                "call of '%0' with suspicious empty string constant argument",
                getMemberLocation(expr))
                << callee->getQualifiedNameAsString() << expr->getSourceRange();
            break;
        }
    }
}

void StringConstant::handleChar(
    CallExpr const * expr, unsigned arg, FunctionDecl const * callee,
    std::string const & replacement, TreatEmpty treatEmpty, bool literal,
    char const * rewriteFrom, char const * rewriteTo)
{
    unsigned n;
    bool nonArray;
    ContentKind cont;
    bool emb;
    bool trm;
    if (!isStringConstant(
            expr->getArg(arg)->IgnoreParenImpCasts(), &n, &nonArray, &cont,
            &emb, &trm))
    {
        return;
    }
    if (cont != ContentKind::Ascii) {
        report(
            DiagnosticsEngine::Warning,
            ("call of '%0' with string constant argument containing non-ASCII"
             " characters"),
            getMemberLocation(expr))
            << callee->getQualifiedNameAsString() << expr->getSourceRange();
        return;
    }
    if (emb) {
        report(
            DiagnosticsEngine::Warning,
            ("call of '%0' with string constant argument containing embedded"
             " NULLs"),
            getMemberLocation(expr))
            << callee->getQualifiedNameAsString() << expr->getSourceRange();
        return;
    }
    if (!trm) {
        report(
            DiagnosticsEngine::Warning,
            ("call of '%0' with string constant argument lacking a terminating"
             " NULL"),
            getMemberLocation(expr))
            << callee->getQualifiedNameAsString() << expr->getSourceRange();
        return;
    }
    std::string repl(replacement);
    checkEmpty(expr, callee, treatEmpty, n, &repl);
    if (!repl.empty()) {
        reportChange(
            expr, ChangeKind::Char, callee->getQualifiedNameAsString(), repl,
            (literal
             ? (n == 0
                ? PassThrough::EmptyConstantString
                : PassThrough::NonEmptyConstantString)
             : PassThrough::No),
            nonArray, rewriteFrom, rewriteTo);
    }
}

void StringConstant::handleCharLen(
    CallExpr const * expr, unsigned arg1, unsigned arg2,
    FunctionDecl const * callee, std::string const & replacement,
    TreatEmpty treatEmpty)
{
    // Especially for f(RTL_CONSTASCII_STRINGPARAM("foo")), where
    // RTL_CONSTASCII_STRINGPARAM expands to complicated expressions involving
    // (&(X)[0] sub-expressions (and it might or might not be better to handle
    // that at the level of non-expanded macros instead, but I have not found
    // out how to do that yet anyway):
    unsigned n;
    bool nonArray;
    ContentKind cont;
    bool emb;
    bool trm;
    if (!(isStringConstant(
              expr->getArg(arg1)->IgnoreParenImpCasts(), &n, &nonArray, &cont,
              &emb, &trm)
          && trm))
    {
        return;
    }
    APSInt res;
    if (compat::EvaluateAsInt(expr->getArg(arg2), res, compiler.getASTContext())) {
        if (res != n) {
            return;
        }
    } else {
        UnaryOperator const * op = dyn_cast<UnaryOperator>(
            expr->getArg(arg1)->IgnoreParenImpCasts());
        if (op == nullptr || op->getOpcode() != UO_AddrOf) {
            return;
        }
        ArraySubscriptExpr const * subs = dyn_cast<ArraySubscriptExpr>(
            op->getSubExpr()->IgnoreParenImpCasts());
        if (subs == nullptr) {
            return;
        }
        unsigned n2;
        bool nonArray2;
        ContentKind cont2;
        bool emb2;
        bool trm2;
        if (!(isStringConstant(
                  subs->getBase()->IgnoreParenImpCasts(), &n2, &nonArray2,
                  &cont2, &emb2, &trm2)
              && n2 == n && cont2 == cont && emb2 == emb && trm2 == trm
                  //TODO: same strings
              && compat::EvaluateAsInt(subs->getIdx(), res, compiler.getASTContext())
              && res == 0))
        {
            return;
        }
    }
    if (cont != ContentKind::Ascii) {
        report(
            DiagnosticsEngine::Warning,
            ("call of '%0' with string constant argument containing non-ASCII"
             " characters"),
            getMemberLocation(expr))
            << callee->getQualifiedNameAsString() << expr->getSourceRange();
    }
    if (emb) {
        return;
    }
    std::string repl(replacement);
    checkEmpty(expr, callee, treatEmpty, n, &repl);
    reportChange(
        expr, ChangeKind::CharLen, callee->getQualifiedNameAsString(), repl,
        PassThrough::No, nonArray, nullptr, nullptr);
}

void StringConstant::handleOUStringCtor(
    CallExpr const * expr, unsigned arg, FunctionDecl const * callee,
    bool explicitFunctionalCastNotation)
{
    handleOUStringCtor(expr, expr->getArg(arg), callee, explicitFunctionalCastNotation);
}

void StringConstant::handleOStringCtor(
    CallExpr const * expr, unsigned arg, FunctionDecl const * callee,
    bool explicitFunctionalCastNotation)
{
    handleOStringCtor(expr, expr->getArg(arg), callee, explicitFunctionalCastNotation);
}

void StringConstant::handleOUStringCtor(
    Expr const * expr, Expr const * argExpr, FunctionDecl const * callee,
    bool explicitFunctionalCastNotation)
{
    handleStringCtor(expr, argExpr, callee, explicitFunctionalCastNotation, StringKind::Unicode);
}

void StringConstant::handleOStringCtor(
    Expr const * expr, Expr const * argExpr, FunctionDecl const * callee,
    bool explicitFunctionalCastNotation)
{
    handleStringCtor(expr, argExpr, callee, explicitFunctionalCastNotation, StringKind::Char);
}

void StringConstant::handleStringCtor(
    Expr const * expr, Expr const * argExpr, FunctionDecl const * callee,
    bool explicitFunctionalCastNotation, StringKind stringKind)
{
    auto e0 = argExpr->IgnoreParenImpCasts();
    auto e1 = dyn_cast<CXXFunctionalCastExpr>(e0);
    if (e1 == nullptr) {
        if (explicitFunctionalCastNotation) {
            return;
        }
    } else {
        e0 = e1->getSubExpr()->IgnoreParenImpCasts();
    }
    auto e2 = dyn_cast<CXXBindTemporaryExpr>(e0);
    if (e2 == nullptr) {
        return;
    }
    auto e3 = dyn_cast<CXXConstructExpr>(
        e2->getSubExpr()->IgnoreParenImpCasts());
    if (e3 == nullptr) {
        return;
    }
    if (!loplugin::DeclCheck(e3->getConstructor()).MemberFunction()
        .Class(stringKind == StringKind::Unicode ? "OUString" : "OString").Namespace("rtl").GlobalNamespace())
    {
        return;
    }
    if (e3->getNumArgs() == 0) {
        report(
            DiagnosticsEngine::Warning,
            ("in call of '%0', replace default-constructed 'OUString' with an"
             " empty string literal"),
            e3->getExprLoc())
            << callee->getQualifiedNameAsString() << expr->getSourceRange();
        return;
    }
    if (e3->getNumArgs() == 1
        && e3->getConstructor()->getNumParams() == 1
        && (loplugin::TypeCheck(
                e3->getConstructor()->getParamDecl(0)->getType())
            .Typedef(stringKind == StringKind::Unicode ? "sal_Unicode" : "char").GlobalNamespace()))
    {
        // It may not be easy to rewrite OUString(c), esp. given there is no
        // OUString ctor taking an OUStringChar arg, so don't warn there:
        if (!explicitFunctionalCastNotation) {
            report(
                DiagnosticsEngine::Warning,
                ("in call of '%0', replace 'OUString' constructed from a"
                 " 'sal_Unicode' with an 'OUStringChar'"),
                e3->getExprLoc())
                << callee->getQualifiedNameAsString() << expr->getSourceRange();
        }
        return;
    }
    if (e3->getNumArgs() != 2) {
        return;
    }
    unsigned n;
    bool nonArray;
    ContentKind cont;
    bool emb;
    bool trm;
    if (!isStringConstant(
            e3->getArg(0)->IgnoreParenImpCasts(), &n, &nonArray, &cont, &emb,
            &trm))
    {
        return;
    }
    //TODO: cont, emb, trm
    if (rewriter != nullptr) {
        auto loc1 = compat::getBeginLoc(e3);
        auto range = e3->getParenOrBraceRange();
        if (loc1.isFileID() && range.getBegin().isFileID()
            && range.getEnd().isFileID())
        {
            auto loc2 = range.getBegin();
            for (bool first = true;; first = false) {
                unsigned n = Lexer::MeasureTokenLength(
                    loc2, compiler.getSourceManager(), compiler.getLangOpts());
                if (!first) {
                    StringRef s(
                        compiler.getSourceManager().getCharacterData(loc2), n);
                    while (s.startswith("\\\n")) {
                        s = s.drop_front(2);
                        while (!s.empty()
                               && (s.front() == ' ' || s.front() == '\t'
                                   || s.front() == '\n' || s.front() == '\v'
                                   || s.front() == '\f'))
                        {
                            s = s.drop_front(1);
                        }
                    }
                    if (!(s.empty() || s.startswith("/*") || s.startswith("//")
                          || s == "\\"))
                    {
                        break;
                    }
                }
                loc2 = loc2.getLocWithOffset(std::max<unsigned>(n, 1));
            }
            auto loc3 = range.getEnd();
            for (;;) {
                auto l = Lexer::GetBeginningOfToken(
                    loc3.getLocWithOffset(-1), compiler.getSourceManager(),
                    compiler.getLangOpts());
                unsigned n = Lexer::MeasureTokenLength(
                    l, compiler.getSourceManager(), compiler.getLangOpts());
                StringRef s(compiler.getSourceManager().getCharacterData(l), n);
                while (s.startswith("\\\n")) {
                    s = s.drop_front(2);
                    while (!s.empty()
                           && (s.front() == ' ' || s.front() == '\t'
                               || s.front() == '\n' || s.front() == '\v'
                               || s.front() == '\f'))
                    {
                        s = s.drop_front(1);
                    }
                }
                if (!(s.empty() || s.startswith("/*") || s.startswith("//")
                      || s == "\\"))
                {
                    break;
                }
                loc3 = l;
            }
            if (removeText(CharSourceRange(SourceRange(loc1, loc2), false))) {
                if (removeText(SourceRange(loc3, range.getEnd()))) {
                    return;
                }
                report(DiagnosticsEngine::Fatal, "Corrupt rewrite", loc3)
                    << expr->getSourceRange();
                return;
            }
        }
    }
    report(
        DiagnosticsEngine::Warning,
        ("in call of '%0', replace 'OUString' constructed from a string literal"
         " directly with the string literal"),
        e3->getExprLoc())
        << callee->getQualifiedNameAsString() << expr->getSourceRange();
}

void StringConstant::handleFunArgOstring(
    CallExpr const * expr, unsigned arg, FunctionDecl const * callee)
{
    auto argExpr = expr->getArg(arg)->IgnoreParenImpCasts();
    unsigned n;
    bool nonArray;
    ContentKind cont;
    bool emb;
    bool trm;
    if (isStringConstant(argExpr, &n, &nonArray, &cont, &emb, &trm)) {
        if (cont != ContentKind::Ascii || emb) {
            return;
        }
        if (!trm) {
            report(
                DiagnosticsEngine::Warning,
                ("call of '%0' with string constant argument lacking a"
                 " terminating NULL"),
                getMemberLocation(expr))
                << callee->getQualifiedNameAsString() << expr->getSourceRange();
            return;
        }
        std::string repl;
        checkEmpty(expr, callee, TreatEmpty::Error, n, &repl);
        if (nonArray) {
            report(
                DiagnosticsEngine::Warning,
                ("in call of '%0' with non-array string constant argument,"
                 " turn the non-array string constant into an array"),
                getMemberLocation(expr))
                << callee->getQualifiedNameAsString() << expr->getSourceRange();
        }
    } else if (auto cexpr = lookForCXXConstructExpr(argExpr)) {
        auto classdecl = cexpr->getConstructor()->getParent();
        if (loplugin::DeclCheck(classdecl).Class("OString").Namespace("rtl")
            .GlobalNamespace())
        {
            switch (cexpr->getConstructor()->getNumParams()) {
            case 0:
                report(
                    DiagnosticsEngine::Warning,
                    ("in call of '%0', replace empty %1 constructor with empty"
                     " string literal"),
                    cexpr->getLocation())
                    << callee->getQualifiedNameAsString() << classdecl
                    << expr->getSourceRange();
                break;
            case 2:
                if (isStringConstant(
                        cexpr->getArg(0)->IgnoreParenImpCasts(), &n, &nonArray,
                        &cont, &emb, &trm))
                {
                    APSInt res;
                    if (compat::EvaluateAsInt(cexpr->getArg(1),
                            res, compiler.getASTContext()))
                    {
                        if (res == n && !emb && trm) {
                            report(
                                DiagnosticsEngine::Warning,
                                ("in call of '%0', elide explicit %1"
                                 " constructor%2"),
                                cexpr->getLocation())
                                << callee->getQualifiedNameAsString()
                                << classdecl << adviseNonArray(nonArray)
                                << expr->getSourceRange();
                        }
                    } else {
                        if (emb) {
                            report(
                                DiagnosticsEngine::Warning,
                                ("call of %0 constructor with string constant"
                                 " argument containing embedded NULLs"),
                                cexpr->getLocation())
                                << classdecl << cexpr->getSourceRange();
                            return;
                        }
                        if (!trm) {
                            report(
                                DiagnosticsEngine::Warning,
                                ("call of %0 constructor with string constant"
                                 " argument lacking a terminating NULL"),
                                cexpr->getLocation())
                                << classdecl << cexpr->getSourceRange();
                            return;
                        }
                        report(
                            DiagnosticsEngine::Warning,
                            "in call of '%0', elide explicit %1 constructor%2",
                            cexpr->getLocation())
                            << callee->getQualifiedNameAsString() << classdecl
                            << adviseNonArray(nonArray)
                            << expr->getSourceRange();
                    }
                }
                break;
            default:
                break;
            }
        }
    }
}

loplugin::Plugin::Registration< StringConstant > X("stringconstant", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
