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
#include <stack>
#include <string>

#include "compat.hxx"
#include "plugin.hxx"

// Define a "string constant" to be a constant expression either of type "array
// of N char" where each array element is a non-NUL ASCII character---except
// that the last array element may be NUL, or, in some situations, of type char
// with a ASCII value (including NUL).  Note that the former includes
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

bool isPlainChar(QualType type) {
    return type->isSpecificBuiltinType(BuiltinType::Char_S)
        || type->isSpecificBuiltinType(BuiltinType::Char_U);
}

SourceLocation getMemberLocation(Expr const * expr) {
    CallExpr const * e1 = dyn_cast<CallExpr>(expr);
    MemberExpr const * e2 = e1 == nullptr
        ? nullptr : dyn_cast<MemberExpr>(e1->getCallee());
    return e2 == nullptr ? expr->getExprLoc()/*TODO*/ : e2->getMemberLoc();
}

class StringConstant:
    public RecursiveASTVisitor<StringConstant>, public loplugin::RewritePlugin
{
public:
    explicit StringConstant(InstantiationData const & data): RewritePlugin(data)
    {}

    void run() override;

    bool TraverseCallExpr(CallExpr * expr);

    bool TraverseCXXMemberCallExpr(CXXMemberCallExpr * expr);

    bool TraverseCXXOperatorCallExpr(CXXOperatorCallExpr * expr);

    bool TraverseCXXConstructExpr(CXXConstructExpr * expr);

    bool VisitCallExpr(CallExpr const * expr);

    bool VisitCXXConstructExpr(CXXConstructExpr const * expr);

private:
    enum class TreatEmpty { DefaultCtor, CheckEmpty, Error };

    enum class ChangeKind { Char, CharLen, SingleChar };

    enum class PassThrough { No, EmptyConstantString, NonEmptyConstantString };

    std::string describeChangeKind(ChangeKind kind);

    bool isStringConstant(
        Expr const * expr, unsigned * size, bool * nonAscii,
        bool * embeddedNuls, bool * terminatingNul);

    bool isZero(Expr const * expr);

    void reportChange(
        Expr const * expr, ChangeKind kind, std::string const & original,
        std::string const & replacement, PassThrough pass,
        char const * rewriteFrom, char const * rewriteTo);

    void checkEmpty(
        CallExpr const * expr, std::string const & qname, TreatEmpty treatEmpty,
        unsigned size, std::string * replacement);

    void handleChar(
        CallExpr const * expr, unsigned arg, std::string const & qname,
        std::string const & replacement, TreatEmpty treatEmpty, bool literal,
    char const * rewriteFrom = nullptr, char const * rewriteTo = nullptr);

    void handleCharLen(
        CallExpr const * expr, unsigned arg1, unsigned arg2,
        std::string const & qname, std::string const & replacement,
        TreatEmpty treatEmpty);

    void handleOUStringCtor(
        CallExpr const * expr, unsigned arg, std::string const & qname);

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
    std::string qname(fdecl->getQualifiedNameAsString());
    //TODO: u.compareToAscii("foo") -> u.???("foo")
    //TODO: u.compareToIgnoreAsciiCaseAscii("foo") -> u.???("foo")
    if (qname == "rtl::OUString::createFromAscii" && fdecl->getNumParams() == 1)
    {
        // OUString::createFromAscii("foo") -> OUString("foo")
        handleChar(
            expr, 0, qname, "rtl::OUString constructor",
            TreatEmpty::DefaultCtor, true);
        return true;
    }
    if (qname == "rtl::OUString::endsWithAsciiL" && fdecl->getNumParams() == 2)
    {
        // u.endsWithAsciiL("foo", 3) -> u.endsWith("foo"):
        handleCharLen(
            expr, 0, 1, qname, "rtl::OUString::endsWith", TreatEmpty::Error);
        return true;
    }
    if (qname == "rtl::OUString::endsWithIgnoreAsciiCaseAsciiL"
        && fdecl->getNumParams() == 2)
    {
        // u.endsWithIgnoreAsciiCaseAsciiL("foo", 3) ->
        // u.endsWithIgnoreAsciiCase("foo"):
        handleCharLen(
            expr, 0, 1, qname, "rtl::OUString::endsWithIgnoreAsciiCase",
            TreatEmpty::Error);
        return true;
    }
    if (qname == "rtl::OUString::equalsAscii" && fdecl->getNumParams() == 1) {
        // u.equalsAscii("foo") -> u == "foo":
        handleChar(
            expr, 0, qname, "operator ==", TreatEmpty::CheckEmpty, false);
        return true;
    }
    if (qname == "rtl::OUString::equalsAsciiL" && fdecl->getNumParams() == 2) {
        // u.equalsAsciiL("foo", 3) -> u == "foo":
        handleCharLen(expr, 0, 1, qname, "operator ==", TreatEmpty::CheckEmpty);
        return true;
    }
    if (qname == "rtl::OUString::equalsIgnoreAsciiCaseAscii"
        && fdecl->getNumParams() == 1)
    {
        // u.equalsIgnoreAsciiCaseAscii("foo") ->
        // u.equalsIngoreAsciiCase("foo"):
        handleChar(
            expr, 0, qname, "rtl::OUString::equalsIgnoreAsciiCase",
            TreatEmpty::CheckEmpty, false);
        return true;
    }
    if (qname == "rtl::OUString::equalsIgnoreAsciiCaseAsciiL"
        && fdecl->getNumParams() == 2)
    {
        // u.equalsIgnoreAsciiCaseAsciiL("foo", 3) ->
        // u.equalsIngoreAsciiCase("foo"):
        handleCharLen(
            expr, 0, 1, qname, "rtl::OUString::equalsIgnoreAsciiCase",
            TreatEmpty::CheckEmpty);
        return true;
    }
    if (qname == "rtl::OUString::indexOfAsciiL" && fdecl->getNumParams() == 3) {
        assert(expr->getNumArgs() == 3);
        // u.indexOfAsciiL("foo", 3, i) -> u.indexOf("foo", i):
        handleCharLen(
            expr, 0, 1, qname, "rtl::OUString::indexOf", TreatEmpty::Error);
        return true;
    }
    if (qname == "rtl::OUString::lastIndexOfAsciiL"
        && fdecl->getNumParams() == 2)
    {
        // u.lastIndexOfAsciiL("foo", 3) -> u.lastIndexOf("foo"):
        handleCharLen(
            expr, 0, 1, qname, "rtl::OUString::lastIndexOf", TreatEmpty::Error);
        return true;
    }
    if (qname == "rtl::OUString::matchAsciiL" && fdecl->getNumParams() == 3) {
        assert(expr->getNumArgs() == 3);
        // u.matchAsciiL("foo", 3, i) -> u.match("foo", i):
        handleCharLen(
            expr, 0, 1, qname,
            (isZero(expr->getArg(2))
             ? std::string("rtl::OUString::startsWith")
             : std::string("rtl::OUString::match")),
            TreatEmpty::Error);
        return true;
    }
    if (qname == "rtl::OUString::matchIgnoreAsciiCaseAsciiL"
        && fdecl->getNumParams() == 3)
    {
        assert(expr->getNumArgs() == 3);
        // u.matchIgnoreAsciiCaseAsciiL("foo", 3, i) ->
        // u.matchIgnoreAsciiCase("foo", i):
        handleCharLen(
            expr, 0, 1, qname,
            (isZero(expr->getArg(2))
             ? std::string("rtl::OUString::startsWithIgnoreAsciiCase")
             : std::string("rtl::OUString::matchIgnoreAsciiCase")),
            TreatEmpty::Error);
        return true;
    }
    if (qname == "rtl::OUString::reverseCompareToAsciiL"
        && fdecl->getNumParams() == 2)
    {
        // u.reverseCompareToAsciiL("foo", 3) -> u.reverseCompareTo("foo"):
        handleCharLen(
            expr, 0, 1, qname, "rtl::OUString::reverseCompareTo",
            TreatEmpty::Error);
        return true;
    }
    if (qname == "rtl::OUString::reverseCompareTo"
        && fdecl->getNumParams() == 1)
    {
        handleOUStringCtor(expr, 0, qname);
        return true;
    }
    if (qname == "rtl::OUString::equalsIgnoreAsciiCase"
        && fdecl->getNumParams() == 1)
    {
        handleOUStringCtor(expr, 0, qname);
        return true;
    }
    if (qname == "rtl::OUString::match" && fdecl->getNumParams() == 2) {
        handleOUStringCtor(expr, 0, qname);
        return true;
    }
    if (qname == "rtl::OUString::matchIgnoreAsciiCase"
        && fdecl->getNumParams() == 2)
    {
        handleOUStringCtor(expr, 0, qname);
        return true;
    }
    if (qname == "rtl::OUString::startsWith" && fdecl->getNumParams() == 2) {
        handleOUStringCtor(expr, 0, qname);
        return true;
    }
    if (qname == "rtl::OUString::startsWithIgnoreAsciiCase"
        && fdecl->getNumParams() == 2)
    {
        handleOUStringCtor(expr, 0, qname);
        return true;
    }
    if (qname == "rtl::OUString::endsWith" && fdecl->getNumParams() == 2) {
        handleOUStringCtor(expr, 0, qname);
        return true;
    }
    if (qname == "rtl::OUString::endsWithIgnoreAsciiCase"
        && fdecl->getNumParams() == 2)
    {
        handleOUStringCtor(expr, 0, qname);
        return true;
    }
    if (qname == "rtl::OUString::indexOf" && fdecl->getNumParams() == 2) {
        handleOUStringCtor(expr, 0, qname);
        return true;
    }
    if (qname == "rtl::OUString::lastIndexOf" && fdecl->getNumParams() == 1) {
        handleOUStringCtor(expr, 0, qname);
        return true;
    }
    if (qname == "rtl::OUString::replaceFirst" && fdecl->getNumParams() == 3) {
        handleOUStringCtor(expr, 0, qname);
        handleOUStringCtor(expr, 1, qname);
        return true;
    }
    if (qname == "rtl::OUString::replaceAll"
        && (fdecl->getNumParams() == 2 || fdecl->getNumParams() == 3))
    {
        handleOUStringCtor(expr, 0, qname);
        handleOUStringCtor(expr, 1, qname);
        return true;
    }
    if (qname == "rtl::OUString::operator+=" && fdecl->getNumParams() == 1) {
        handleOUStringCtor(
            expr, dyn_cast<CXXOperatorCallExpr>(expr) == nullptr ? 0 : 1,
            qname);
        return true;
    }
    if (qname == "rtl::OUString::equals" && fdecl->getNumParams() == 1) {
        unsigned n;
        bool non;
        bool emb;
        bool trm;
        if (!isStringConstant(
                expr->getArg(0)->IgnoreParenImpCasts(), &n, &non, &emb, &trm))
        {
            return true;
        }
        if (non) {
            report(
                DiagnosticsEngine::Warning,
                ("call of " + qname
                 + (" with string constant argument containging non-ASCII"
                    " characters")),
                expr->getExprLoc())
                << expr->getSourceRange();
        }
        if (emb) {
            report(
                DiagnosticsEngine::Warning,
                ("call of " + qname
                 + " with string constant argument containging embedded NULs"),
                expr->getExprLoc())
                << expr->getSourceRange();
        }
        if (n == 0) {
            report(
                DiagnosticsEngine::Warning,
                ("rewrite call of " + qname
                 + (" with empty string constant argument as call of"
                    " rtl::OUString::isEmpty")),
                expr->getExprLoc())
                << expr->getSourceRange();
            return true;
        }
    }
    if (qname == "rtl::operator==" && fdecl->getNumParams() == 2) {
        for (unsigned i = 0; i != 2; ++i) {
            unsigned n;
            bool non;
            bool emb;
            bool trm;
            if (!isStringConstant(
                    expr->getArg(i)->IgnoreParenImpCasts(), &n, &non, &emb,
                    &trm))
            {
                continue;
            }
            if (non) {
                report(
                    DiagnosticsEngine::Warning,
                    ("call of " + qname
                     + (" with string constant argument containging non-ASCII"
                        " characters")),
                    expr->getExprLoc())
                    << expr->getSourceRange();
            }
            if (emb) {
                report(
                    DiagnosticsEngine::Warning,
                    ("call of " + qname
                     + (" with string constant argument containging embedded"
                        " NULs")),
                    expr->getExprLoc())
                    << expr->getSourceRange();
            }
            if (n == 0) {
                report(
                    DiagnosticsEngine::Warning,
                    ("rewrite call of " + qname
                     + (" with empty string constant argument as call of"
                        " rtl::OUString::isEmpty")),
                    expr->getExprLoc())
                    << expr->getSourceRange();
            }
        }
        return true;
    }
    if (qname == "rtl::operator!=" && fdecl->getNumParams() == 2) {
        for (unsigned i = 0; i != 2; ++i) {
            unsigned n;
            bool non;
            bool emb;
            bool trm;
            if (!isStringConstant(
                    expr->getArg(i)->IgnoreParenImpCasts(), &n, &non, &emb,
                    &trm))
            {
                continue;
            }
            if (non) {
                report(
                    DiagnosticsEngine::Warning,
                    ("call of " + qname
                     + (" with string constant argument containging non-ASCII"
                        " characters")),
                    expr->getExprLoc())
                    << expr->getSourceRange();
            }
            if (emb) {
                report(
                    DiagnosticsEngine::Warning,
                    ("call of " + qname
                     + (" with string constant argument containging embedded"
                        " NULs")),
                    expr->getExprLoc())
                    << expr->getSourceRange();
            }
            if (n == 0) {
                report(
                    DiagnosticsEngine::Warning,
                    ("rewrite call of " + qname
                     + (" with empty string constant argument as call of"
                        " !rtl::OUString::isEmpty")),
                    expr->getExprLoc())
                    << expr->getSourceRange();
            }
        }
        return true;
    }
    if (qname == "rtl::OUString::operator=" && fdecl->getNumParams() == 1) {
        unsigned n;
        bool non;
        bool emb;
        bool trm;
        if (!isStringConstant(
                expr->getArg(1)->IgnoreParenImpCasts(), &n, &non, &emb, &trm))
        {
            return true;
        }
        if (non) {
            report(
                DiagnosticsEngine::Warning,
                ("call of " + qname
                 + (" with string constant argument containging non-ASCII"
                    " characters")),
                expr->getExprLoc())
                << expr->getSourceRange();
        }
        if (emb) {
            report(
                DiagnosticsEngine::Warning,
                ("call of " + qname
                 + " with string constant argument containging embedded NULs"),
                expr->getExprLoc())
                << expr->getSourceRange();
        }
        if (n == 0) {
            report(
                DiagnosticsEngine::Warning,
                ("rewrite call of " + qname
                 + (" with empty string constant argument as call of"
                    " rtl::OUString::clear")),
                expr->getExprLoc())
                << expr->getSourceRange();
            return true;
        }
        return true;
    }
    if (qname == "rtl::OUStringBuffer::appendAscii"
        && fdecl->getNumParams() == 1)
    {
        // u.appendAscii("foo") -> u.append("foo")
        handleChar(
            expr, 0, qname, "rtl::OUStringBuffer::append", TreatEmpty::Error,
            true, "appendAscii", "append");
        return true;
    }
    if (qname == "rtl::OUStringBuffer::appendAscii"
        && fdecl->getNumParams() == 2)
    {
        // u.appendAscii("foo", 3) -> u.append("foo"):
        handleCharLen(
            expr, 0, 1, qname, "rtl::OUStringBuffer::append",
            TreatEmpty::Error);
        return true;
    }
    return true;
}

bool StringConstant::VisitCXXConstructExpr(CXXConstructExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    std::string qname(
        expr->getConstructor()->getParent()->getQualifiedNameAsString());
    if (qname == "rtl::OUString") {
        ChangeKind kind;
        PassThrough pass;
        switch (expr->getConstructor()->getNumParams()) {
        case 1:
            {
                APSInt v;
                if (!expr->getArg(0)->isIntegerConstantExpr(
                        v, compiler.getASTContext()))
                {
                    return true;
                }
                if (v == 0 || v.uge(0x80)) {
                    return true;
                }
                kind = ChangeKind::SingleChar;
                pass = PassThrough::NonEmptyConstantString;
                break;
            }
        case 2:
            {
                unsigned n;
                bool non;
                bool emb;
                bool trm;
                if (!isStringConstant(
                        expr->getArg(0)->IgnoreParenImpCasts(), &n, &non, &emb,
                        &trm))
                {
                    return true;
                }
                if (non) {
                    report(
                        DiagnosticsEngine::Warning,
                        ("construction of " + qname
                         + (" with string constant argument containging"
                            " non-ASCII characters")),
                        expr->getExprLoc())
                        << expr->getSourceRange();
                }
                if (emb) {
                    report(
                        DiagnosticsEngine::Warning,
                        ("construction of " + qname
                         + (" with string constant argument containging"
                            " embedded NULs")),
                        expr->getExprLoc())
                        << expr->getSourceRange();
                }
                kind = ChangeKind::Char;
                pass = n == 0
                    ? PassThrough::EmptyConstantString
                    : PassThrough::NonEmptyConstantString;
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
                    e = cast<MaterializeTemporaryExpr>(e)->GetTemporaryExpr()
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
                        std::string callQname(
                            fdecl->getQualifiedNameAsString());
                        if (pass == PassThrough::EmptyConstantString) {
                            if (callQname == "rtl::OUString::equals"
                                || callQname == "rtl::operator==")
                            {
                                report(
                                    DiagnosticsEngine::Warning,
                                    ("rewrite call of " + callQname
                                     + " with construction of " + qname
                                     + (" with empty string constant argument"
                                        " as call of rtl::OUString::isEmpty")),
                                    getMemberLocation(call))
                                    << call->getSourceRange();
                                return true;
                            }
                            if (callQname == "rtl::operator!=") {
                                report(
                                    DiagnosticsEngine::Warning,
                                    ("rewrite call of " + callQname
                                     + " with construction of " + qname
                                     + (" with empty string constant argument"
                                        " as call of !rtl::OUString::isEmpty")),
                                    getMemberLocation(call))
                                    << call->getSourceRange();
                                return true;
                            }
                            if (callQname == "rtl::operator+"
                                || callQname == "rtl::OUString::operator+=")
                            {
                                report(
                                    DiagnosticsEngine::Warning,
                                    ("call of " + callQname
                                     + " with suspicous construction of "
                                     + qname
                                     + " with empty string constant argument"),
                                    getMemberLocation(call))
                                    << call->getSourceRange();
                                return true;
                            }
                            if (callQname == "rtl::OUString::operator=") {
                                report(
                                    DiagnosticsEngine::Warning,
                                    ("rewrite call of " + callQname
                                     + " with construction of " + qname
                                     + (" with empty string constant argument"
                                        " as call of rtl::OUString::clear")),
                                    getMemberLocation(call))
                                    << call->getSourceRange();
                                return true;
                            }
                        } else {
                            assert(pass == PassThrough::NonEmptyConstantString);
                            if (callQname == "rtl::OUString::equals") {
                                report(
                                    DiagnosticsEngine::Warning,
                                    ("rewrite call of " + callQname
                                     + " with construction of " + qname
                                     + " with " + describeChangeKind(kind)
                                     + " as operator =="),
                                    getMemberLocation(call))
                                    << call->getSourceRange();
                                return true;
                            }
                            if (callQname == "rtl::operator+"
                                || callQname == "rtl::OUString::operator="
                                || callQname == "rtl::operator=="
                                || callQname == "rtl::operator!=")
                            {
                                if (callQname == "rtl::operator+") {
                                    std::string file(
                                        compiler.getSourceManager().getFilename(
                                            compiler.getSourceManager()
                                            .getSpellingLoc(
                                                expr->getLocStart())));
                                    if (file
                                        == (SRCDIR
                                            "/sal/qa/rtl/strings/test_ostring_concat.cxx")
                                        || (file
                                            == (SRCDIR
                                            "/sal/qa/rtl/strings/test_oustring_concat.cxx")))
                                    {
                                        return true;
                                    }
                                }
                                report(
                                    DiagnosticsEngine::Warning,
                                    ("elide construction of " + qname + " with "
                                     + describeChangeKind(kind) + " in call of "
                                     + callQname),
                                    getMemberLocation(expr))
                                    << expr->getSourceRange();
                                return true;
                            }
                        }
                        return true;
                    } else if (isa<CXXConstructExpr>(call)) {
                    } else {
                        assert(false);
                    }
                }
            }
        }
        return true;
    }
    return true;
}

std::string StringConstant::describeChangeKind(ChangeKind kind) {
    switch (kind) {
    case ChangeKind::Char:
        return "string constant argument";
    case ChangeKind::CharLen:
        return "string constant and matching length arguments";
    case ChangeKind::SingleChar:
        return "ASCII sal_Unicode argument";
    }
}

bool StringConstant::isStringConstant(
    Expr const * expr, unsigned * size, bool * nonAscii, bool * embeddedNuls,
    bool * terminatingNul)
{
    assert(expr != nullptr);
    assert(size != nullptr);
    assert(nonAscii != nullptr);
    assert(embeddedNuls != nullptr);
    assert(terminatingNul != nullptr);
    QualType t = expr->getType();
    if (!(t->isConstantArrayType() && t.isConstQualified()
          && isPlainChar(t->getAsArrayTypeUnsafe()->getElementType())))
    {
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
    StringLiteral const * lit = dyn_cast<StringLiteral>(expr);
    if (lit != nullptr) {
        if (!lit->isAscii()) {
            return false;
        }
        unsigned n = lit->getLength();
        bool non = false;
        bool emb = false;
        StringRef str = lit->getString();
        for (unsigned i = 0; i != n; ++i) {
            if (str[i] == '\0') {
                emb = true;
            } else if (static_cast<unsigned char>(str[i]) >= 0x80) {
                non = true;
            }
        }
        *size = n;
        *nonAscii = non;
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
            assert(e != nullptr); //TODO???
            if (!v.getLValueOffset().isZero()) {
                return false; //TODO
            }
            Expr const * e2 = e->IgnoreParenImpCasts();
            if (e2 != e) {
                return isStringConstant(
                    e2, size, nonAscii, embeddedNuls, terminatingNul);
            }
            //TODO: string literals are represented as recursive LValues???
            llvm::APInt n
                = compiler.getASTContext().getAsConstantArrayType(t)->getSize();
            assert(n != 0);
            --n;
            assert(n.ule(std::numeric_limits<unsigned>::max()));
            *size = static_cast<unsigned>(n.getLimitedValue());
            *nonAscii = false; //TODO
            *embeddedNuls = false; //TODO
            *terminatingNul = true;
            return true;
        }
    case APValue::Array:
        {
            if (v.hasArrayFiller()) { //TODO: handle final NUL filler?
                return false;
            }
            unsigned n = v.getArraySize();
            assert(n != 0);
            bool non = false;
            bool emb = false;
            for (unsigned i = 0; i != n - 1; ++i) {
                APValue e(v.getArrayInitializedElt(i));
                if (!e.isInt()) { //TODO: assert?
                    return false;
                }
                APSInt iv = e.getInt();
                if (iv == 0) {
                    emb = true;
                } else if (iv.uge(0x80)) {
                    non = true;
                }
            }
            APValue e(v.getArrayInitializedElt(n - 1));
            if (!e.isInt()) { //TODO: assert?
                return false;
            }
            bool trm = e.getInt() == 0;
            *size = trm ? n - 1 : n;
            *nonAscii = non;
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
    return expr->isIntegerConstantExpr(res, compiler.getASTContext())
        && res == 0;
}

void StringConstant::reportChange(
    Expr const * expr, ChangeKind kind, std::string const & original,
    std::string const & replacement, PassThrough pass, char const * rewriteFrom,
    char const * rewriteTo)
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
                    std::string qname(fdecl->getQualifiedNameAsString());
                    if (pass == PassThrough::EmptyConstantString) {
                        if (qname == "rtl::OUString::equals"
                            || qname == "rtl::operator==")
                        {
                            report(
                                DiagnosticsEngine::Warning,
                                ("rewrite call of " + qname + " with call of "
                                 + original
                                 + (" with empty string constant argument as"
                                    " call of rtl::OUString::isEmpty")),
                                getMemberLocation(call))
                                << call->getSourceRange();
                            return;
                        }
                        if (qname == "rtl::operator!=") {
                            report(
                                DiagnosticsEngine::Warning,
                                ("rewrite call of " + qname + " with call of "
                                 + original
                                 + (" with empty string constant argument as"
                                    " call of !rtl::OUString::isEmpty")),
                                getMemberLocation(call))
                                << call->getSourceRange();
                            return;
                        }
                        if (qname == "rtl::operator+"
                            || qname == "rtl::OUString::operator+=")
                        {
                            report(
                                DiagnosticsEngine::Warning,
                                ("call of " + qname + " with suspicous call of "
                                 + original
                                 + " with empty string constant argument"),
                                getMemberLocation(call))
                                << call->getSourceRange();
                            return;
                        }
                        if (qname == "rtl::OUString::operator=") {
                            report(
                                DiagnosticsEngine::Warning,
                                ("rewrite call of " + qname + " with call of "
                                 + original
                                 + (" with empty string constant argument as"
                                    " call of rtl::OUString::call")),
                                getMemberLocation(call))
                                << call->getSourceRange();
                            return;
                        }
                    } else {
                        assert(pass == PassThrough::NonEmptyConstantString);
                        if (qname == "rtl::OUString::equals"
                            || qname == "rtl::OUString::operator="
                            || qname == "rtl::operator=="
                            || qname == "rtl::operator!=")
                        {
                            report(
                                DiagnosticsEngine::Warning,
                                ("elide call of " + original + " with "
                                 + describeChangeKind(kind) + " in call of "
                                 + qname),
                                getMemberLocation(expr))
                                << expr->getSourceRange();
                            return;
                        }
                        if (qname == "rtl::operator+"
                            || qname == "rtl::OUString::operator+=")
                        {
                            report(
                                DiagnosticsEngine::Warning,
                                ("rewrite call of " + original + " with "
                                 + describeChangeKind(kind) + " in call of "
                                 + qname
                                 + (" as (implicit) construction of"
                                    " rtl::OUString")),
                                getMemberLocation(expr))
                                << expr->getSourceRange();
                            return;
                        }
                    }
                    report(
                        DiagnosticsEngine::Warning,
                        "TODO call inside " + qname, getMemberLocation(expr))
                        << expr->getSourceRange();
                    return;
                } else if (isa<CXXConstructExpr>(call)) {
                    std::string qname(
                        cast<CXXConstructExpr>(call)->getConstructor()
                        ->getParent()->getQualifiedNameAsString());
                    if (qname == "rtl::OUString"
                        || qname == "rtl::OUStringBuffer")
                    {
                        //TODO: propagate further out?
                        if (pass == PassThrough::EmptyConstantString) {
                            report(
                                DiagnosticsEngine::Warning,
                                ("rewrite construction of " + qname
                                 + " with call of " + original
                                 + (" with empty string constant argument as"
                                    " default construction of ")
                                 + qname),
                                getMemberLocation(call))
                                << call->getSourceRange();
                        } else {
                            assert(pass == PassThrough::NonEmptyConstantString);
                            report(
                                DiagnosticsEngine::Warning,
                                ("elide call of " + original + " with "
                                 + describeChangeKind(kind)
                                 + " in construction of " + qname),
                                getMemberLocation(expr))
                                << expr->getSourceRange();
                        }
                        return;
                    }
                } else {
                    assert(false);
                }
            }
        }
    }
    if (rewriter != nullptr && rewriteFrom != nullptr) {
        SourceLocation loc = getMemberLocation(expr);
        while (compiler.getSourceManager().isMacroArgExpansion(loc)) {
            loc = compiler.getSourceManager().getImmediateMacroCallerLoc(loc);
        }
        if (compat::isMacroBodyExpansion(compiler, loc)) {
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
        ("rewrite call of " + original + " with " + describeChangeKind(kind)
         + " as call of " + replacement),
        getMemberLocation(expr))
        << expr->getSourceRange();
}

void StringConstant::checkEmpty(
    CallExpr const * expr, std::string const & qname, TreatEmpty treatEmpty,
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
                ("call of " + qname
                 + " with suspicous empty string constant argument"),
                getMemberLocation(expr))
                << expr->getSourceRange();
            break;
        }
    }
}

void StringConstant::handleChar(
    CallExpr const * expr, unsigned arg, std::string const & qname,
    std::string const & replacement, TreatEmpty treatEmpty, bool literal,
    char const * rewriteFrom, char const * rewriteTo)
{
    unsigned n;
    bool non;
    bool emb;
    bool trm;
    if (!isStringConstant(
            expr->getArg(arg)->IgnoreParenImpCasts(), &n, &non, &emb, &trm))
    {
        return;
    }
    if (non) {
        report(
            DiagnosticsEngine::Warning,
            ("call of " + qname
             + (" with string constant argument containging non-ASCII"
                " characters")),
            getMemberLocation(expr))
            << expr->getSourceRange();
        return;
    }
    if (emb) {
        report(
            DiagnosticsEngine::Warning,
            ("call of " + qname
             + " with string constant argument containging embedded NULs"),
            getMemberLocation(expr))
            << expr->getSourceRange();
        return;
    }
    if (!trm) {
        report(
            DiagnosticsEngine::Warning,
            ("call of " + qname
             + " with string constant argument lacking a terminating NUL"),
            getMemberLocation(expr))
            << expr->getSourceRange();
        return;
    }
    std::string repl(replacement);
    checkEmpty(expr, qname, treatEmpty, n, &repl);
    reportChange(
        expr, ChangeKind::Char, qname, repl,
        (literal
         ? (n == 0
            ? PassThrough::EmptyConstantString
            : PassThrough::NonEmptyConstantString)
         : PassThrough::No),
        rewriteFrom, rewriteTo);
}

void StringConstant::handleCharLen(
    CallExpr const * expr, unsigned arg1, unsigned arg2,
    std::string const & qname, std::string const & replacement,
    TreatEmpty treatEmpty)
{
    // Especially for f(RTL_CONSTASCII_STRINGPARAM("foo")), where
    // RTL_CONSTASCII_STRINGPARAM expands to complicated expressions involving
    // (&(X)[0] sub-expressions (and it might or might not be better to handle
    // that at the level of non-expanded macros instead, but I have not found
    // out how to do that yet anyway):
    unsigned n;
    bool non;
    bool emb;
    bool trm;
    if (!(isStringConstant(
              expr->getArg(arg1)->IgnoreParenImpCasts(), &n, &non, &emb, &trm)
          && trm))
    {
        return;
    }
    APSInt res;
    if (expr->getArg(arg2)->isIntegerConstantExpr(
            res, compiler.getASTContext()))
    {
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
        bool non2;
        bool emb2;
        bool trm2;
        if (!(isStringConstant(
                  subs->getBase()->IgnoreParenImpCasts(), &n2, &non2, &emb2,
                  &trm2)
              && n2 == n && non2 == non && emb2 == emb && trm2 == trm
                  //TODO: same strings
              && subs->getIdx()->isIntegerConstantExpr(
                  res, compiler.getASTContext())
              && res == 0))
        {
            return;
        }
    }
    if (non) {
        report(
            DiagnosticsEngine::Warning,
            ("call of " + qname
             + (" with string constant argument containging non-ASCII"
                " characters")),
            getMemberLocation(expr))
            << expr->getSourceRange();
    }
    if (emb) {
        return;
    }
    std::string repl(replacement);
    checkEmpty(expr, qname, treatEmpty, n, &repl);
    reportChange(
        expr, ChangeKind::CharLen, qname, repl, PassThrough::No, nullptr,
        nullptr);
}

void StringConstant::handleOUStringCtor(
    CallExpr const * expr, unsigned arg, std::string const & qname)
{
    auto e0 = expr->getArg(arg)->IgnoreParenImpCasts();
    auto e1 = dyn_cast<CXXFunctionalCastExpr>(e0);
    if (e1 != nullptr) {
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
    if (e3->getConstructor()->getQualifiedNameAsString()
         != "rtl::OUString::OUString")
    {
        return;
    }
    if (e3->getNumArgs() == 0) {
        report(
            DiagnosticsEngine::Warning,
            ("in call of %0, replace default-constructed OUString with an empty"
             " string literal"),
            e3->getExprLoc())
            << qname << expr->getSourceRange();
        return;
    }
    APSInt res;
    if (e3->getNumArgs() == 1
        && e3->getArg(0)->IgnoreParenImpCasts()->isIntegerConstantExpr(
            res, compiler.getASTContext()))
    {
        if (res.getZExtValue() <= 127) {
            report(
                DiagnosticsEngine::Warning,
                ("in call of %0, replace OUString constructed from an ASCII"
                 " char constant with a string literal"),
                e3->getExprLoc())
                << qname << expr->getSourceRange();
        }
        return;
    }
    if (e3->getNumArgs() != 2) {
        return;
    }
    unsigned n;
    bool non;
    bool emb;
    bool trm;
    if (!isStringConstant(
            e3->getArg(0)->IgnoreParenImpCasts(), &n, &non, &emb, &trm))
    {
        return;
    }
    //TODO: non, emb, trm
    report(
        DiagnosticsEngine::Warning,
        ("in call of %0, replace OUString constructed from a string literal"
         " directly with the string literal"),
        e3->getExprLoc())
        << qname << expr->getSourceRange();
}

loplugin::Plugin::Registration< StringConstant > X("stringconstant", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
