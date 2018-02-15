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
    bool ChangeBinaryOperatorPlusMinus(BinaryOperator const* parent, CXXMemberCallExpr const* call,
                                       std::string const& methodName);
    bool ChangeBinaryOperatorOther(BinaryOperator const* parent, CXXMemberCallExpr const* call,
                                   std::string const& methodName, std::string const& setPrefix);
    bool ChangeUnaryOperator(UnaryOperator const* parent, CXXMemberCallExpr const* call,
                             std::string const& methodName);
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
        if (!ChangeUnaryOperator(unaryOp, call, methodName))
            report(DiagnosticsEngine::Warning, "Could not fix, unary", call->getLocStart());
        return true;
    }
    auto binaryOp = dyn_cast<BinaryOperator>(parent);
    if (!binaryOp)
    {
        // normal getter
        return true;
    }
    auto opcode = binaryOp->getOpcode();
    if (opcode == BO_Assign)
    {
        // Check for assignments embedded inside other expressions
        auto parent2 = getParentStmt(parent);
        if (dyn_cast_or_null<ExprWithCleanups>(parent2))
            parent2 = getParentStmt(parent2);
        if (parent2 && isa<Expr>(parent2))
        {
            report(DiagnosticsEngine::Warning, "Could not fix, embedded assign",
                   call->getLocStart());
            return true;
        }
        // Check for
        //   X.Width() = X.Height() = 1;
        if (auto rhs = dyn_cast<BinaryOperator>(binaryOp->getRHS()->IgnoreParenImpCasts()))
            if (rhs->getOpcode() == BO_Assign)
            {
                report(DiagnosticsEngine::Warning, "Could not fix, double assign",
                       call->getLocStart());
                return true;
            }
        if (!ChangeAssignment(parent, methodName, setPrefix))
            report(DiagnosticsEngine::Warning, "Could not fix, assign", call->getLocStart());
        return true;
    }
    if (opcode == BO_AddAssign || opcode == BO_SubAssign)
    {
        if (!ChangeBinaryOperatorPlusMinus(binaryOp, call, methodName))
            report(DiagnosticsEngine::Warning, "Could not fix, assign-and-change",
                   call->getLocStart());
        return true;
    }
    else if (opcode == BO_RemAssign || opcode == BO_MulAssign || opcode == BO_DivAssign)
    {
        if (!ChangeBinaryOperatorOther(binaryOp, call, methodName, setPrefix))
            report(DiagnosticsEngine::Warning, "Could not fix, assign-and-change",
                   call->getLocStart());
        return true;
    }
    else
        assert(false);
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
    SourceLocation startLoc = SM.getExpansionLoc(parent->getLocStart());
    SourceLocation endLoc = SM.getExpansionLoc(parent->getLocEnd());
    const char* p1 = SM.getCharacterData(startLoc);
    const char* p2 = SM.getCharacterData(endLoc);
    unsigned n = Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
    if (p2 < p1) // clang is misbehaving, appears to be macro constant related
        return false;
    std::string callText(p1, p2 - p1 + n);
    auto originalLength = callText.size();

    auto newText = std::regex_replace(callText, std::regex(methodName + " *\\( *\\) *="),
                                      setPrefix + methodName + "(");
    if (newText == callText)
        return false;
    newText += " )";

    return replaceText(startLoc, originalLength, newText);
}

bool ChangeToolsGen::ChangeBinaryOperatorPlusMinus(BinaryOperator const* binaryOp,
                                                   CXXMemberCallExpr const* call,
                                                   std::string const& methodName)
{
    // Look for expressions like
    //    aRect.Left() += ...;
    // and replace with
    //    aRect.MoveLeft( ... );
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = SM.getExpansionLoc(binaryOp->getLocStart());
    SourceLocation endLoc = SM.getExpansionLoc(binaryOp->getLocEnd());
    const char* p1 = SM.getCharacterData(startLoc);
    const char* p2 = SM.getCharacterData(endLoc);
    if (p2 < p1) // clang is misbehaving, appears to be macro constant related
        return false;
    unsigned n = Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
    std::string callText(p1, p2 - p1 + n);
    auto originalLength = callText.size();

    std::string newText;
    if (binaryOp->getOpcode() == BO_AddAssign)
    {
        newText = std::regex_replace(callText, std::regex(methodName + " *\\( *\\) +\\+= *"),
                                     "Adjust" + methodName + "(");
        newText += " )";
    }
    else
    {
        newText = std::regex_replace(callText, std::regex(methodName + " *\\( *\\) +\\-= *"),
                                     "Adjust" + methodName + "( -(");
        newText += ") )";
    }

    if (newText == callText)
    {
        report(DiagnosticsEngine::Warning, "binaryop-plusminus regex match failed",
               call->getLocStart());
        return false;
    }

    return replaceText(startLoc, originalLength, newText);
}

bool ChangeToolsGen::ChangeBinaryOperatorOther(BinaryOperator const* binaryOp,
                                               CXXMemberCallExpr const* call,
                                               std::string const& methodName,
                                               std::string const& setPrefix)
{
    // Look for expressions like
    //    aRect.Left() += ...;
    // and replace with
    //    aRect.SetLeft( aRect.GetLeft() + ... );
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = SM.getExpansionLoc(binaryOp->getLocStart());
    SourceLocation endLoc = SM.getExpansionLoc(binaryOp->getLocEnd());
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
    std::string reString(methodName + " *\\( *\\) *" + regexOpname);
    auto newText = std::regex_replace(callText, std::regex(reString),
                                      setPrefix + methodName + "( " + implicitObjectText + "."
                                          + methodName + "() " + replaceOpname + " (");
    if (newText == callText)
    {
        report(DiagnosticsEngine::Warning, "binaryop-other regex match failed %0",
               call->getLocStart())
            << reString;
        return false;
    }
    // sometimes we end up with duplicate spaces after the opname
    newText
        = std::regex_replace(newText, std::regex(methodName + "\\(\\) \\" + replaceOpname + "  "),
                             methodName + "() " + replaceOpname + " ");
    newText += ") )";

    return replaceText(startLoc, originalLength, newText);
}

bool ChangeToolsGen::ChangeUnaryOperator(UnaryOperator const* unaryOp,
                                         CXXMemberCallExpr const* call,
                                         std::string const& methodName)
{
    // Look for expressions like
    //    aRect.Left()++;
    //    ++aRect.Left();
    // and replace with
    //    aRect.MoveLeft( 1 );

    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = SM.getExpansionLoc(unaryOp->getLocStart());
    SourceLocation endLoc = SM.getExpansionLoc(unaryOp->getLocEnd());
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
    std::string replaceOp;
    switch (op)
    {
        case UO_PostInc:
        case UO_PreInc:
            replaceOp = "1";
            regexOpname = "\\+\\+";
            break;
        case UO_PostDec:
        case UO_PreDec:
            replaceOp = "-1";
            regexOpname = "\\-\\-";
            break;
        default:
            assert(false);
    }
    std::string newText;
    std::string reString;
    if (op == UO_PostInc || op == UO_PostDec)
    {
        reString = methodName + " *\\( *\\) *" + regexOpname;
        newText = std::regex_replace(callText, std::regex(reString),
                                     "Adjust" + methodName + "( " + replaceOp);
    }
    else
    {
        newText = implicitObjectText + "." + "Adjust" + methodName + "( " + replaceOp;
    }
    if (newText == callText)
    {
        report(DiagnosticsEngine::Warning, "unaryop regex match failed %0", call->getLocStart())
            << reString;
        return false;
    }
    newText += " )";
    return replaceText(startLoc, originalLength, newText);
}

std::string ChangeToolsGen::extractCode(SourceLocation startLoc, SourceLocation endLoc)
{
    SourceManager& SM = compiler.getSourceManager();
    const char* p1 = SM.getCharacterData(SM.getExpansionLoc(startLoc));
    const char* p2 = SM.getCharacterData(SM.getExpansionLoc(endLoc));
    unsigned n = Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
    return std::string(p1, p2 - p1 + n);
}

static loplugin::Plugin::Registration<ChangeToolsGen> X("changetoolsgen", false);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
