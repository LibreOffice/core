/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#include "plugin.hxx"
#include "check.hxx"
#include <regex>

/**
 * Changes calls to tools::Rectangle methods that return a ref to instead call the setter methods.
 *
 * run as:
 *   make COMPILER_PLUGIN_TOOL=changerectanglegetref UPDATE_FILES=all FORCE_COMPILE_ALL=1
 * or
 *   make <module> COMPILER_PLUGIN_TOOL=changerectanglegetref FORCE_COMPILE_ALL=1
 */

namespace
{
class ChangeRectangleGetRef : public RecursiveASTVisitor<ChangeRectangleGetRef>,
                              public loplugin::RewritePlugin
{
public:
    explicit ChangeRectangleGetRef(loplugin::InstantiationData const& data)
        : RewritePlugin(data)
    {
    }
    virtual void run() override;
    bool VisitCXXMemberCallExpr(CXXMemberCallExpr const* call);

private:
    bool ChangeAssignment(CXXMemberCallExpr const* call, std::string const& methodName);
    bool ChangeAssignmentOp(CXXMemberCallExpr const* call, std::string const& methodName);
    bool ChangeAssignmentOpOp(CXXMemberCallExpr const* call, std::string const& methodName);
    std::string extractCode(SourceLocation startLoc, SourceLocation endLoc);
};

void ChangeRectangleGetRef::run()
{
    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
}

void replace(std::string& s, std::string const& from, std::string const& to)
{
    auto i = s.find(from);
    if (i == std::string::npos)
        return;
    s.replace(i, from.length(), to);
}

bool ChangeRectangleGetRef::VisitCXXMemberCallExpr(CXXMemberCallExpr const* call)
{
    if (ignoreLocation(call))
        return true;
    const FunctionDecl* func = call->getDirectCallee();
    if (!func)
        return true;
    auto dc = loplugin::DeclCheck(func);
    std::string methodName;
    if (dc.Function("Top").Class("Rectangle").Namespace("tools").GlobalNamespace())
        methodName = "Top";
    else if (dc.Function("Bottom").Class("Rectangle").Namespace("tools").GlobalNamespace())
        methodName = "Bottom";
    else if (dc.Function("Left").Class("Rectangle").Namespace("tools").GlobalNamespace())
        methodName = "Left";
    else if (dc.Function("Right").Class("Rectangle").Namespace("tools").GlobalNamespace())
        methodName = "Right";
    else
        return true;
    if (!loplugin::TypeCheck(func->getReturnType()).LvalueReference())
        return true;
    if (ChangeAssignment(call, methodName))
        return true;
    if (ChangeAssignmentOp(call, methodName))
        return true;
    if (ChangeAssignmentOpOp(call, methodName))
        return true;
    return true;
}

bool ChangeRectangleGetRef::ChangeAssignment(CXXMemberCallExpr const* call,
                                             std::string const& methodName)
{
    // Look for expressions like
    //    aRect.Left() = ...;
    // and replace with
    //    aRect.SetLeft( ... );
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = call->getLocStart();
    SourceLocation endLoc = call->getLocEnd();
    const char* p1 = SM.getCharacterData(startLoc);
    const char* p2 = SM.getCharacterData(endLoc);
    unsigned n = Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
    p2 += n;
    while (*p2 == ' ')
        ++p2;
    if (*p2 != '=')
        return true;
    while (*p2 != ';')
        ++p2;
    ++p2;
    std::string callText(p1, p2 - p1);
    auto originalLength = callText.size();

    auto newText = std::regex_replace(callText, std::regex(methodName + "\\(\\) *="),
                                      "Set" + methodName + "(");
    if (newText == callText)
        return false;
    replace(newText, ";", " );");

    replaceText(startLoc, originalLength, newText);
    report(DiagnosticsEngine::Warning, "found", call->getLocStart());
    return true;
}

bool ChangeRectangleGetRef::ChangeAssignmentOp(CXXMemberCallExpr const* call,
                                               std::string const& methodName)
{
    // Look for expressions like
    //    aRect.Left() += ...;
    // and replace with
    //    aRect.SetLeft( aRect.GetLeft() + ... );
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = call->getLocStart();
    SourceLocation endLoc = call->getLocEnd();
    const char* p1 = SM.getCharacterData(startLoc);
    const char* p2 = SM.getCharacterData(endLoc);
    unsigned n = Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
    p2 += n;
    while (*p2 == ' ')
        ++p2;
    std::string op;
    if (*p2 == '+')
        op = "+";
    else if (*p2 == '-')
        op = "-";
    else if (*p2 == '*')
        op = "*";
    else if (*p2 == '/')
        op = "/";
    else if (*p2 == '%')
        op = "%";
    else
        return true;
    ++p2;
    if (*p2 != '=')
        return true;
    ++p2;
    while (*p2 != ';')
        ++p2;
    ++p2;
    std::string callText(p1, p2 - p1);
    auto originalLength = callText.size();

    auto implicitObjectText = extractCode(call->getImplicitObjectArgument()->getExprLoc(),
                                          call->getImplicitObjectArgument()->getExprLoc());
    auto newText = std::regex_replace(callText, std::regex(methodName + "\\(\\) *\\" + op + "="),
                                      "Set" + methodName + "( " + implicitObjectText + ".Get"
                                          + methodName + "() " + op + " ");
    if (newText == callText)
        return false;
    // sometimes we end up with duplicate spaces after the op
    newText = std::regex_replace(newText, std::regex("Get" + methodName + "\\(\\) \\" + op + "  "),
                                 "Get" + methodName + "() " + op + " ");
    replace(newText, ";", " );");

    replaceText(startLoc, originalLength, newText);
    report(DiagnosticsEngine::Warning, "found", call->getLocStart());
    return true;
}

bool ChangeRectangleGetRef::ChangeAssignmentOpOp(CXXMemberCallExpr const* call,
                                                 std::string const& methodName)
{
    // Look for expressions like
    //    aRect.Left()++;
    // and replace with
    //    aRect.SetLeft( ++aRect.GetLeft() );
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = call->getLocStart();
    SourceLocation endLoc = call->getLocEnd();
    const char* p1 = SM.getCharacterData(startLoc);
    const char* p2 = SM.getCharacterData(endLoc);
    unsigned n = Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
    p2 += n;
    while (*p2 == ' ')
        ++p2;
    std::string op;
    if (*p2 == '+' && *(p2 + 1) == '+')
        op = "+";
    else if (*p2 == '-' && *(p2 + 1) == '-')
        op = "-";
    else
        return true;
    p2 += 2;
    while (*p2 == ' ')
        ++p2;
    if (*p2 != ';')
        return true;
    ++p2;
    std::string callText(p1, p2 - p1);
    auto originalLength = callText.size();

    auto implicitObjectText = extractCode(call->getImplicitObjectArgument()->getExprLoc(),
                                          call->getImplicitObjectArgument()->getExprLoc());
    auto newText = std::regex_replace(
        callText, std::regex(methodName + "\\(\\) *\\" + op + "\\" + op),
        "Set" + methodName + "( " + op + op + implicitObjectText + ".Get" + methodName + "() ;");
    if (newText == callText)
        return false;

    replaceText(startLoc, originalLength, newText);
    report(DiagnosticsEngine::Warning, "found", call->getLocStart());
    return true;
}

std::string ChangeRectangleGetRef::extractCode(SourceLocation startLoc, SourceLocation endLoc)
{
    SourceManager& SM = compiler.getSourceManager();
    const char* p1 = SM.getCharacterData(startLoc);
    const char* p2 = SM.getCharacterData(endLoc);
    unsigned n = Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
    return std::string(p1, p2 - p1 + n);
}

static loplugin::Plugin::Registration<ChangeRectangleGetRef> X("changerectanglegetref", false);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
