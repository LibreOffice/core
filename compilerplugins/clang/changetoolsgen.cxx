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
 * Changes calls to tools::Rectangle/Point/Size methods that return a ref to instead call the setter methods.
 *
 * run as:
 *   make COMPILER_PLUGIN_TOOL=changetoolsgen UPDATE_FILES=all FORCE_COMPILE_ALL=1
 * or
 *   make <module> COMPILER_PLUGIN_TOOL=changetoolsgen FORCE_COMPILE_ALL=1
 */

namespace
{
class ChangeToolsGen : public RecursiveASTVisitor<ChangeToolsGen>, public loplugin::RewritePlugin
{
public:
    explicit ChangeToolsGen(loplugin::InstantiationData const& data)
        : RewritePlugin(data)
    {
    }
    virtual void run() override;
    bool VisitCXXMemberCallExpr(CXXMemberCallExpr const* call);

private:
    bool ChangeAssignment(Stmt const* parent, std::string const& methodName,
                          std::string const& setPrefix);
    bool ChangeBinaryOperator(BinaryOperator const* parent, CXXMemberCallExpr const* call,
                              std::string const& methodName, std::string const& setPrefix);
    bool ChangeUnaryOperator(UnaryOperator const* parent, CXXMemberCallExpr const* call,
                             std::string const& methodName, std::string const& setPrefix);
    std::string extractCode(SourceLocation startLoc, SourceLocation endLoc);
};

void ChangeToolsGen::run() { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

bool ChangeToolsGen::VisitCXXMemberCallExpr(CXXMemberCallExpr const* call)
{
    if (ignoreLocation(call))
        return true;
    const CXXMethodDecl* func = call->getMethodDecl();
    if (!func)
        return true;
    if (func->isConst())
        return true;
    auto dc = loplugin::DeclCheck(func);
    std::string methodName;
    std::string setPrefix;
    if (dc.Function("Top").Class("Rectangle").Namespace("tools").GlobalNamespace())
    {
        methodName = "Top";
        setPrefix = "Set";
    }
    else if (dc.Function("Bottom").Class("Rectangle").Namespace("tools").GlobalNamespace())
    {
        methodName = "Bottom";
        setPrefix = "Set";
    }
    else if (dc.Function("Left").Class("Rectangle").Namespace("tools").GlobalNamespace())
    {
        methodName = "Left";
        setPrefix = "Set";
    }
    else if (dc.Function("Right").Class("Rectangle").Namespace("tools").GlobalNamespace())
    {
        methodName = "Right";
        setPrefix = "Set";
    }
    else if (dc.Function("X").Class("Point").GlobalNamespace())
    {
        methodName = "X";
        setPrefix = "set";
    }
    else if (dc.Function("Y").Class("Point").GlobalNamespace())
    {
        methodName = "Y";
        setPrefix = "set";
    }
    else if (dc.Function("Width").Class("Size").GlobalNamespace())
    {
        methodName = "Width";
        setPrefix = "set";
    }
    else if (dc.Function("Height").Class("Size").GlobalNamespace())
    {
        methodName = "Height";
        setPrefix = "set";
    }
    else
        return true;
    if (!loplugin::TypeCheck(func->getReturnType()).LvalueReference())
        return true;

    auto parent = getParentStmt(call);
    if (!parent)
        return true;
    if (auto unaryOp = dyn_cast<UnaryOperator>(parent))
    {
        if (!ChangeUnaryOperator(unaryOp, call, methodName, setPrefix))
            report(DiagnosticsEngine::Warning, "Could not fix this one1", call->getLocStart());
        return true;
    }
    auto binaryOp = dyn_cast<BinaryOperator>(parent);
    if (!binaryOp)
    {
        //        parent->dump();
        //        report(DiagnosticsEngine::Warning, "Could not fix this one3", call->getLocStart());
        return true;
    }
    auto opcode = binaryOp->getOpcode();
    if (opcode == BO_Assign)
    {
        if (!ChangeAssignment(parent, methodName, setPrefix))
            report(DiagnosticsEngine::Warning, "Could not fix this one4", call->getLocStart());
        return true;
    }
    if (opcode == BO_RemAssign || opcode == BO_AddAssign || opcode == BO_SubAssign
        || opcode == BO_MulAssign || opcode == BO_DivAssign)
    {
        if (!ChangeBinaryOperator(binaryOp, call, methodName, setPrefix))
            report(DiagnosticsEngine::Warning, "Could not fix this one5", call->getLocStart());
        return true;
    }
    return true;
}

bool ChangeToolsGen::ChangeAssignment(Stmt const* parent, std::string const& methodName,
                                      std::string const& setPrefix)
{
    // Look for expressions like
    //    aRect.Left() = ...;
    // and replace with
    //    aRect.SetLeft( ... );
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = parent->getLocStart();
    SourceLocation endLoc = parent->getLocEnd();
    const char* p1 = SM.getCharacterData(startLoc);
    const char* p2 = SM.getCharacterData(endLoc);
    unsigned n = Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
    if (p2 < p1) // clang is misbehaving, appears to be macro constant related
        return false;
    std::string callText(p1, p2 - p1 + n);
    auto originalLength = callText.size();

    auto newText = std::regex_replace(callText, std::regex(methodName + "\\(\\) *="),
                                      setPrefix + methodName + "(");
    if (newText == callText)
        return false;
    newText += " )";

    return replaceText(startLoc, originalLength, newText);
}

bool ChangeToolsGen::ChangeBinaryOperator(BinaryOperator const* binaryOp,
                                          CXXMemberCallExpr const* call,
                                          std::string const& methodName,
                                          std::string const& setPrefix)
{
    // Look for expressions like
    //    aRect.Left() += ...;
    // and replace with
    //    aRect.SetLeft( aRect.GetLeft() + ... );
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = binaryOp->getLocStart();
    SourceLocation endLoc = binaryOp->getLocEnd();
    const char* p1 = SM.getCharacterData(startLoc);
    const char* p2 = SM.getCharacterData(endLoc);
    if (p2 < p1) // clang is misbehaving, appears to be macro constant related
        return false;
    unsigned n = Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
    std::string callText(p1, p2 - p1 + n);
    auto originalLength = callText.size();

    std::string regexOpname;
    std::string replaceOpname;
    switch (binaryOp->getOpcode())
    {
        case BO_RemAssign:
            regexOpname = "\\%=";
            replaceOpname = "%";
            break;
        case BO_AddAssign:
            regexOpname = "\\+=";
            replaceOpname = "+";
            break;
        case BO_SubAssign:
            regexOpname = "\\-=";
            replaceOpname = "-";
            break;
        case BO_MulAssign:
            regexOpname = "\\*=";
            replaceOpname = "*";
            break;
        case BO_DivAssign:
            regexOpname = "\\/=";
            replaceOpname = "/";
            break;
        default:
            assert(false);
    }

    auto implicitObjectText = extractCode(call->getImplicitObjectArgument()->getExprLoc(),
                                          call->getImplicitObjectArgument()->getExprLoc());
    auto newText = std::regex_replace(callText, std::regex(methodName + "\\(\\) *" + regexOpname),
                                      setPrefix + methodName + "( " + implicitObjectText + "."
                                          + methodName + "() " + replaceOpname + " ");
    if (newText == callText)
        return false;
    // sometimes we end up with duplicate spaces after the opname
    newText
        = std::regex_replace(newText, std::regex(methodName + "\\(\\) \\" + replaceOpname + "  "),
                             methodName + "() " + replaceOpname + " ");
    newText += " )";

    return replaceText(startLoc, originalLength, newText);
}

bool ChangeToolsGen::ChangeUnaryOperator(UnaryOperator const* unaryOp,
                                         CXXMemberCallExpr const* call,
                                         std::string const& methodName,
                                         std::string const& setPrefix)
{
    // Look for expressions like
    //    aRect.Left()++;
    // and replace with
    //    aRect.SetLeft( ++aRect.GetLeft() );
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = unaryOp->getLocStart();
    SourceLocation endLoc = unaryOp->getLocEnd();
    const char* p1 = SM.getCharacterData(startLoc);
    const char* p2 = SM.getCharacterData(endLoc);
    if (p2 < p1) // clang is misbehaving, appears to be macro constant related
        return false;
    unsigned n = Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
    std::string callText(p1, p2 - p1 + n);
    auto originalLength = callText.size();

    auto implicitObjectText = extractCode(call->getImplicitObjectArgument()->getExprLoc(),
                                          call->getImplicitObjectArgument()->getExprLoc());
    auto op = unaryOp->getOpcode();
    std::string regexOpname;
    std::string replaceOpname;
    switch (op)
    {
        case UO_PostInc:
        case UO_PreInc:
            replaceOpname = "++";
            regexOpname = "\\+\\+";
            break;
        case UO_PostDec:
        case UO_PreDec:
            replaceOpname = "--";
            regexOpname = "\\-\\-";
            break;
        default:
            assert(false);
    }
    if (op == UO_PostInc || op == UO_PostDec)
    {
        auto newText
            = std::regex_replace(callText, std::regex(methodName + "\\(\\) *" + regexOpname),
                                 setPrefix + methodName + "( " + replaceOpname + implicitObjectText
                                     + "." + methodName + "()");
        return replaceText(startLoc, originalLength, newText);
    }
    else
    {
        auto newText
            = std::regex_replace(callText, std::regex(regexOpname + " *" + methodName + "\\(\\)"),
                                 setPrefix + methodName + "( " + replaceOpname + implicitObjectText
                                     + "." + methodName + "()");
        return replaceText(startLoc, originalLength, newText);
    }
}

std::string ChangeToolsGen::extractCode(SourceLocation startLoc, SourceLocation endLoc)
{
    SourceManager& SM = compiler.getSourceManager();
    const char* p1 = SM.getCharacterData(startLoc);
    const char* p2 = SM.getCharacterData(endLoc);
    unsigned n = Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
    return std::string(p1, p2 - p1 + n);
}

static loplugin::Plugin::Registration<ChangeToolsGen> X("changetoolsgen", false);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
