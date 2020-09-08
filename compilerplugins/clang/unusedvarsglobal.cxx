/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#if !defined _WIN32 //TODO, #include <sys/file.h>

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <sys/file.h>
#include <unistd.h>

#include "config_clang.h"

#include "plugin.hxx"
#include "compat.hxx"
#include "check.hxx"

#if CLANG_VERSION >= 110000
#include "clang/AST/ParentMapContext.h"
#endif

/**
This performs two analyses:
 (1) look for unused global vars
 (2) look for global vars that are write-only
*/

namespace
{
struct MyVarInfo
{
    const VarDecl* varDecl;
    std::string fieldName;
    std::string fieldType;
    std::string sourceLocation;
};
bool operator<(const MyVarInfo& lhs, const MyVarInfo& rhs)
{
    return std::tie(lhs.sourceLocation, lhs.fieldName)
           < std::tie(rhs.sourceLocation, rhs.fieldName);
}

// try to limit the voluminous output a little
static std::set<MyVarInfo> readFromSet;
static std::set<MyVarInfo> writeToSet;
static std::set<MyVarInfo> definitionSet;

/**
 * Wrap the different kinds of callable and callee objects in the clang AST so I can define methods that handle everything.
 */
class CallerWrapper
{
    const CallExpr* m_callExpr;
    const CXXConstructExpr* m_cxxConstructExpr;

public:
    CallerWrapper(const CallExpr* callExpr)
        : m_callExpr(callExpr)
        , m_cxxConstructExpr(nullptr)
    {
    }
    CallerWrapper(const CXXConstructExpr* cxxConstructExpr)
        : m_callExpr(nullptr)
        , m_cxxConstructExpr(cxxConstructExpr)
    {
    }
    unsigned getNumArgs() const
    {
        return m_callExpr ? m_callExpr->getNumArgs() : m_cxxConstructExpr->getNumArgs();
    }
    const Expr* getArg(unsigned i) const
    {
        return m_callExpr ? m_callExpr->getArg(i) : m_cxxConstructExpr->getArg(i);
    }
};
class CalleeWrapper
{
    const FunctionDecl* m_calleeFunctionDecl = nullptr;
    const CXXConstructorDecl* m_cxxConstructorDecl = nullptr;
    const FunctionProtoType* m_functionPrototype = nullptr;

public:
    explicit CalleeWrapper(const FunctionDecl* calleeFunctionDecl)
        : m_calleeFunctionDecl(calleeFunctionDecl)
    {
    }
    explicit CalleeWrapper(const CXXConstructExpr* cxxConstructExpr)
        : m_cxxConstructorDecl(cxxConstructExpr->getConstructor())
    {
    }
    explicit CalleeWrapper(const FunctionProtoType* functionPrototype)
        : m_functionPrototype(functionPrototype)
    {
    }
    unsigned getNumParams() const
    {
        if (m_calleeFunctionDecl)
            return m_calleeFunctionDecl->getNumParams();
        else if (m_cxxConstructorDecl)
            return m_cxxConstructorDecl->getNumParams();
        else if (m_functionPrototype->param_type_begin() == m_functionPrototype->param_type_end())
            // FunctionProtoType will assert if we call getParamTypes() and it has no params
            return 0;
        else
            return m_functionPrototype->getParamTypes().size();
    }
    const QualType getParamType(unsigned i) const
    {
        if (m_calleeFunctionDecl)
            return m_calleeFunctionDecl->getParamDecl(i)->getType();
        else if (m_cxxConstructorDecl)
            return m_cxxConstructorDecl->getParamDecl(i)->getType();
        else
            return m_functionPrototype->getParamTypes()[i];
    }
    std::string getNameAsString() const
    {
        if (m_calleeFunctionDecl)
            return m_calleeFunctionDecl->getNameAsString();
        else if (m_cxxConstructorDecl)
            return m_cxxConstructorDecl->getNameAsString();
        else
            return "";
    }
    CXXMethodDecl const* getAsCXXMethodDecl() const
    {
        if (m_calleeFunctionDecl)
            return dyn_cast<CXXMethodDecl>(m_calleeFunctionDecl);
        return nullptr;
    }
};

class UnusedVarsGlobal : public loplugin::FilteringPlugin<UnusedVarsGlobal>
{
public:
    explicit UnusedVarsGlobal(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override;

    bool shouldVisitTemplateInstantiations() const { return true; }
    bool shouldVisitImplicitCode() const { return true; }

    bool VisitVarDecl(const VarDecl*);
    bool VisitDeclRefExpr(const DeclRefExpr*);
    bool TraverseCXXMethodDecl(CXXMethodDecl*);
    bool TraverseFunctionDecl(FunctionDecl*);
    bool TraverseIfStmt(IfStmt*);

private:
    MyVarInfo niceName(const VarDecl*);
    void checkIfReadFrom(const VarDecl* fieldDecl, const DeclRefExpr* declRefExpr);
    void checkIfWrittenTo(const VarDecl* fieldDecl, const DeclRefExpr* declRefExpr);
    bool isSomeKindOfZero(const Expr* arg);
    bool checkForWriteWhenUsingCollectionType(const CXXMethodDecl* calleeMethodDecl);
    bool IsPassedByNonConst(const VarDecl* fieldDecl, const Stmt* child, CallerWrapper callExpr,
                            CalleeWrapper calleeFunctionDecl);
    llvm::Optional<CalleeWrapper> getCallee(CallExpr const*);

    // For reasons I do not understand, parentFunctionDecl() is not reliable, so
    // we store the parent function on the way down the AST.
    FunctionDecl* insideFunctionDecl = nullptr;
    std::vector<VarDecl const*> insideConditionalCheckOfVarSet;
};

void UnusedVarsGlobal::run()
{
    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

    if (!isUnitTestMode())
    {
        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile
        std::string output;
        for (const MyVarInfo& s : readFromSet)
            output += "read:\t" + s.sourceLocation + "\t" + s.fieldName + "\n";
        for (const MyVarInfo& s : writeToSet)
            output += "write:\t" + s.sourceLocation + "\t" + s.fieldName + "\n";
        for (const MyVarInfo& s : definitionSet)
            output += "definition:\t" + s.fieldName + "\t" + s.fieldType + "\t" + s.sourceLocation
                      + "\n";
        std::ofstream myfile;
        myfile.open(WORKDIR "/loplugin.unusedvarsglobal.log", std::ios::app | std::ios::out);
        myfile << output;
        myfile.close();
    }
    else
    {
        for (const MyVarInfo& s : readFromSet)
            report(DiagnosticsEngine::Warning, "read", compat::getBeginLoc(s.varDecl));
        for (const MyVarInfo& s : writeToSet)
            report(DiagnosticsEngine::Warning, "write", compat::getBeginLoc(s.varDecl));
    }
}

MyVarInfo UnusedVarsGlobal::niceName(const VarDecl* varDecl)
{
    MyVarInfo aInfo;
    aInfo.varDecl = varDecl;

    aInfo.fieldName = varDecl->getNameAsString();
    // sometimes the name (if it's an anonymous thing) contains the full path of the build folder, which we don't need
    size_t idx = aInfo.fieldName.find(SRCDIR);
    if (idx != std::string::npos)
    {
        aInfo.fieldName = aInfo.fieldName.replace(idx, strlen(SRCDIR), "");
    }
    aInfo.fieldType = varDecl->getType().getAsString();

    SourceLocation expansionLoc
        = compiler.getSourceManager().getExpansionLoc(varDecl->getLocation());
    StringRef name = getFilenameOfLocation(expansionLoc);
    aInfo.sourceLocation
        = std::string(name.substr(strlen(SRCDIR) + 1)) + ":"
          + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
    loplugin::normalizeDotDotInFilePath(aInfo.sourceLocation);

    return aInfo;
}

bool UnusedVarsGlobal::VisitVarDecl(const VarDecl* varDecl)
{
    varDecl = varDecl->getCanonicalDecl();
    if (isa<ParmVarDecl>(varDecl))
        return true;
    if (!varDecl->hasGlobalStorage())
        return true;
    if (!varDecl->getLocation().isValid() || ignoreLocation(varDecl))
        return true;
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(varDecl->getLocation())))
        return true;

    /**
     If we have
        const size_t NB_PRODUCTS = 3;
        int DefaultProductDir[NB_PRODUCTS] = { 3, 3, 3 };
     clang will inline the constant "3" and never tell us that we are reading from NB_PRODUCTS,
     so just ignore integer constants.
    */
    auto varType = varDecl->getType();
    if (varType.isConstQualified() && varType->isIntegerType())
        return true;

    auto initExpr = varDecl->getAnyInitializer();
    if (initExpr && !isSomeKindOfZero(initExpr))
        writeToSet.insert(niceName(varDecl));

    definitionSet.insert(niceName(varDecl));
    return true;
}

bool UnusedVarsGlobal::VisitDeclRefExpr(const DeclRefExpr* declRefExpr)
{
    const Decl* decl = declRefExpr->getDecl();
    const VarDecl* varDecl = dyn_cast<VarDecl>(decl);
    if (!varDecl)
        return true;
    if (isa<ParmVarDecl>(varDecl))
        return true;
    if (!varDecl->hasGlobalStorage())
        return true;
    varDecl = varDecl->getCanonicalDecl();
    if (!varDecl->getLocation().isValid() || ignoreLocation(varDecl))
        return true;
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(varDecl->getLocation())))
        return true;
    checkIfReadFrom(varDecl, declRefExpr);
    checkIfWrittenTo(varDecl, declRefExpr);
    return true;
}

/**
 Does the expression being used to initialise a field value evaluate to
 the same as a default value?
 */
bool UnusedVarsGlobal::isSomeKindOfZero(const Expr* arg)
{
    assert(arg);
    if (arg->isValueDependent())
        return false;
    if (arg->getType().isNull())
        return false;
    if (isa<CXXDefaultArgExpr>(arg))
        arg = dyn_cast<CXXDefaultArgExpr>(arg)->getExpr();
    arg = arg->IgnoreParenCasts();
    // ignore this, it seems to trigger an infinite recursion
    if (isa<UnaryExprOrTypeTraitExpr>(arg))
        return false;
    if (auto cxxConstructExpr = dyn_cast<CXXConstructExpr>(arg))
        return cxxConstructExpr->getConstructor()->isDefaultConstructor();
    APSInt x1;
    if (compat::EvaluateAsInt(arg, x1, compiler.getASTContext()))
        return x1 == 0;
    if (isa<CXXNullPtrLiteralExpr>(arg))
        return true;
    if (isa<MaterializeTemporaryExpr>(arg))
    {
        const CXXBindTemporaryExpr* strippedArg
            = dyn_cast_or_null<CXXBindTemporaryExpr>(arg->IgnoreParenCasts());
        if (strippedArg)
        {
            auto temp = dyn_cast<CXXTemporaryObjectExpr>(strippedArg->getSubExpr());
            if (temp->getNumArgs() == 0)
            {
                if (loplugin::TypeCheck(temp->getType())
                        .Class("OUString")
                        .Namespace("rtl")
                        .GlobalNamespace())
                    return true;
                if (loplugin::TypeCheck(temp->getType())
                        .Class("OString")
                        .Namespace("rtl")
                        .GlobalNamespace())
                    return true;
                return false;
            }
        }
    }

    // Get the expression contents.
    // This helps us find params which are always initialised with something like "OUString()".
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = compat::getBeginLoc(arg);
    SourceLocation endLoc = compat::getEndLoc(arg);
    const char* p1 = SM.getCharacterData(startLoc);
    const char* p2 = SM.getCharacterData(endLoc);
    if (!p1 || !p2 || (p2 - p1) < 0 || (p2 - p1) > 40)
        return false;
    unsigned n = Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
    std::string s(p1, p2 - p1 + n);
    // strip linefeed and tab characters so they don't interfere with the parsing of the log file
    std::replace(s.begin(), s.end(), '\r', ' ');
    std::replace(s.begin(), s.end(), '\n', ' ');
    std::replace(s.begin(), s.end(), '\t', ' ');

    // now normalize the value. For some params, like OUString, we can pass it as OUString() or "" and they are the same thing
    if (s == "OUString()")
        return true;
    else if (s == "OString()")
        return true;
    else if (s == "aEmptyOUStr") //sw
        return true;
    else if (s == "EMPTY_OUSTRING") //sc
        return true;
    else if (s == "GetEmptyOUString()") //sc
        return true;
    return false;
}

static char easytolower(char in)
{
    if (in <= 'Z' && in >= 'A')
        return in - ('Z' - 'z');
    return in;
}

bool startswith(const std::string& rStr, const char* pSubStr)
{
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

bool UnusedVarsGlobal::TraverseCXXMethodDecl(CXXMethodDecl* cxxMethodDecl)
{
    auto copy2 = insideFunctionDecl;
    insideFunctionDecl = cxxMethodDecl;
    bool ret = RecursiveASTVisitor::TraverseCXXMethodDecl(cxxMethodDecl);
    insideFunctionDecl = copy2;
    return ret;
}

bool UnusedVarsGlobal::TraverseFunctionDecl(FunctionDecl* functionDecl)
{
    auto copy2 = insideFunctionDecl;
    insideFunctionDecl = functionDecl;
    bool ret = RecursiveASTVisitor::TraverseFunctionDecl(functionDecl);
    insideFunctionDecl = copy2;
    return ret;
}

bool UnusedVarsGlobal::TraverseIfStmt(IfStmt* ifStmt)
{
    VarDecl const* varDecl = nullptr;
    Expr const* cond = ifStmt->getCond()->IgnoreParenImpCasts();

    if (auto memberCallExpr = dyn_cast<CXXMemberCallExpr>(cond))
    {
        if (auto cxxConvert = dyn_cast_or_null<CXXConversionDecl>(memberCallExpr->getMethodDecl()))
        {
            if (cxxConvert->getConversionType()->isBooleanType())
                if (auto declRefExpr = dyn_cast<DeclRefExpr>(
                        memberCallExpr->getImplicitObjectArgument()->IgnoreParenImpCasts()))
                    if ((varDecl = dyn_cast<VarDecl>(declRefExpr->getDecl())))
                        insideConditionalCheckOfVarSet.push_back(varDecl);
        }
    }
    else if (auto declRefExpr = dyn_cast<DeclRefExpr>(cond))
    {
        if ((varDecl = dyn_cast<VarDecl>(declRefExpr->getDecl())))
            insideConditionalCheckOfVarSet.push_back(varDecl);
    }

    bool ret = RecursiveASTVisitor::TraverseIfStmt(ifStmt);
    if (varDecl)
        insideConditionalCheckOfVarSet.pop_back();
    return ret;
}

void UnusedVarsGlobal::checkIfReadFrom(const VarDecl* varDecl, const DeclRefExpr* declRefExpr)
{
    auto parentsRange = compiler.getASTContext().getParents(*declRefExpr);
    const Stmt* child = declRefExpr;
    const Stmt* parent
        = parentsRange.begin() == parentsRange.end() ? nullptr : parentsRange.begin()->get<Stmt>();
    // walk up the tree until we find something interesting
    bool bPotentiallyReadFrom = false;
    bool bDump = false;
    auto walkUp = [&]() {
        child = parent;
        auto parentsRange = compiler.getASTContext().getParents(*parent);
        parent = parentsRange.begin() == parentsRange.end() ? nullptr
                                                            : parentsRange.begin()->get<Stmt>();
    };
    do
    {
        if (!parent)
        {
            // check if we're inside a CXXCtorInitializer or a VarDecl
            auto parentsRange = compiler.getASTContext().getParents(*child);
            if (parentsRange.begin() != parentsRange.end())
            {
                const Decl* decl = parentsRange.begin()->get<Decl>();
                if (decl && (isa<CXXConstructorDecl>(decl) || isa<VarDecl>(decl)))
                    bPotentiallyReadFrom = true;
            }
            if (!bPotentiallyReadFrom)
                return;
            break;
        }
        if (isa<CXXReinterpretCastExpr>(parent))
        {
            // once we see one of these, there is not much useful we can know
            bPotentiallyReadFrom = true;
            break;
        }
        else if (isa<CastExpr>(parent) || isa<MemberExpr>(parent) || isa<ParenExpr>(parent)
                 || isa<ParenListExpr>(parent) || isa<ArrayInitLoopExpr>(parent)
                 || isa<ExprWithCleanups>(parent))
        {
            walkUp();
        }
        else if (auto unaryOperator = dyn_cast<UnaryOperator>(parent))
        {
            UnaryOperator::Opcode op = unaryOperator->getOpcode();
            if (declRefExpr->getType()->isArrayType() && op == UO_Deref)
            {
                // ignore, deref'ing an array does not count as a read
            }
            else if (op == UO_AddrOf || op == UO_Deref || op == UO_Plus || op == UO_Minus
                     || op == UO_Not || op == UO_LNot)
            {
                bPotentiallyReadFrom = true;
                break;
            }
            /* The following are technically reads, but from a code-sense they're more of a write/modify, so
                ignore them to find interesting fields that only modified, not usefully read:
                UO_PreInc / UO_PostInc / UO_PreDec / UO_PostDec
                But we still walk up in case the result of the expression is used in a read sense.
            */
            walkUp();
        }
        else if (auto caseStmt = dyn_cast<CaseStmt>(parent))
        {
            bPotentiallyReadFrom = caseStmt->getLHS() == child || caseStmt->getRHS() == child;
            break;
        }
        else if (auto ifStmt = dyn_cast<IfStmt>(parent))
        {
            bPotentiallyReadFrom = ifStmt->getCond() == child;
            break;
        }
        else if (auto doStmt = dyn_cast<DoStmt>(parent))
        {
            bPotentiallyReadFrom = doStmt->getCond() == child;
            break;
        }
        else if (auto arraySubscriptExpr = dyn_cast<ArraySubscriptExpr>(parent))
        {
            if (arraySubscriptExpr->getIdx() == child)
            {
                bPotentiallyReadFrom = true;
                break;
            }
            walkUp();
        }
        else if (auto binaryOp = dyn_cast<BinaryOperator>(parent))
        {
            BinaryOperator::Opcode op = binaryOp->getOpcode();
            const bool assignmentOp = op == BO_Assign || op == BO_MulAssign || op == BO_DivAssign
                                      || op == BO_RemAssign || op == BO_AddAssign
                                      || op == BO_SubAssign || op == BO_ShlAssign
                                      || op == BO_ShrAssign || op == BO_AndAssign
                                      || op == BO_XorAssign || op == BO_OrAssign;
            if (binaryOp->getLHS() == child && assignmentOp)
                break;
            else
            {
                bPotentiallyReadFrom = true;
                break;
            }
        }
        else if (auto operatorCallExpr = dyn_cast<CXXOperatorCallExpr>(parent))
        {
            auto op = operatorCallExpr->getOperator();
            const bool assignmentOp = op == OO_Equal || op == OO_StarEqual || op == OO_SlashEqual
                                      || op == OO_PercentEqual || op == OO_PlusEqual
                                      || op == OO_MinusEqual || op == OO_LessLessEqual
                                      || op == OO_AmpEqual || op == OO_CaretEqual
                                      || op == OO_PipeEqual;
            if (operatorCallExpr->getArg(0) == child && assignmentOp)
                break;
            else if (op == OO_GreaterGreaterEqual && operatorCallExpr->getArg(1) == child)
                break; // this is a write-only call
            else
            {
                bPotentiallyReadFrom = true;
                break;
            }
        }
        else if (auto cxxMemberCallExpr = dyn_cast<CXXMemberCallExpr>(parent))
        {
            bool bWriteOnlyCall = false;
            const CXXMethodDecl* callee = cxxMemberCallExpr->getMethodDecl();
            if (callee)
            {
                const Expr* tmp = dyn_cast<Expr>(child);
                if (tmp->isBoundMemberFunction(compiler.getASTContext()))
                {
                    tmp = dyn_cast<MemberExpr>(tmp)->getBase();
                }
                if (cxxMemberCallExpr->getImplicitObjectArgument() == tmp)
                {
                    // FIXME perhaps a better solution here would be some kind of SAL_PARAM_WRITEONLY attribute
                    // which we could scatter around.
                    std::string name = callee->getNameAsString();
                    std::transform(name.begin(), name.end(), name.begin(), easytolower);
                    if (startswith(name, "emplace") || name == "insert" || name == "erase"
                        || name == "remove" || name == "remove_if" || name == "sort"
                        || name == "push_back" || name == "pop_back" || name == "push_front"
                        || name == "pop_front" || name == "reserve" || name == "resize"
                        || name == "reset" || name == "clear" || name == "fill")
                        // write-only modifications to collections
                        bWriteOnlyCall = true;
                    else if (name == "dispose" || name == "disposeAndClear" || name == "swap")
                        // we're abusing the write-only analysis here to look for fields which don't have anything useful
                        // being done to them, so we're ignoring things like std::vector::clear, std::vector::swap,
                        // and VclPtr::disposeAndClear
                        bWriteOnlyCall = true;
                }
            }
            if (!bWriteOnlyCall)
                bPotentiallyReadFrom = true;
            break;
        }
        else if (auto callExpr = dyn_cast<CallExpr>(parent))
        {
            bool bWriteOnlyCall = false;
            // check for calls to ReadXXX(foo) type methods, where foo is write-only
            auto callee = getCallee(callExpr);
            if (callee)
            {
                // FIXME perhaps a better solution here would be some kind of SAL_PARAM_WRITEONLY attribute
                // which we could scatter around.
                std::string name = callee->getNameAsString();
                std::transform(name.begin(), name.end(), name.begin(), easytolower);
                if (startswith(name, "read"))
                    // this is a write-only call
                    bWriteOnlyCall = true;
            }
            if (!bWriteOnlyCall)
                bPotentiallyReadFrom = true;
            break;
        }
        else if (isa<ReturnStmt>(parent) || isa<CXXConstructExpr>(parent)
                 || isa<ConditionalOperator>(parent) || isa<SwitchStmt>(parent)
                 || isa<DeclStmt>(parent) || isa<WhileStmt>(parent) || isa<CXXNewExpr>(parent)
                 || isa<ForStmt>(parent) || isa<InitListExpr>(parent)
                 || isa<CXXDependentScopeMemberExpr>(parent) || isa<UnresolvedMemberExpr>(parent)
                 || isa<MaterializeTemporaryExpr>(parent))
        {
            bPotentiallyReadFrom = true;
            break;
        }
        else if (isa<CXXDeleteExpr>(parent) || isa<UnaryExprOrTypeTraitExpr>(parent)
                 || isa<CXXUnresolvedConstructExpr>(parent) || isa<CompoundStmt>(parent)
                 || isa<LabelStmt>(parent) || isa<CXXForRangeStmt>(parent)
                 || isa<CXXTypeidExpr>(parent) || isa<DefaultStmt>(parent)
                 || isa<GCCAsmStmt>(parent) || isa<LambdaExpr>(parent) // TODO
                 || isa<CXXDefaultArgExpr>(parent) || isa<AtomicExpr>(parent)
                 || isa<VAArgExpr>(parent) || isa<DeclRefExpr>(parent)
#if CLANG_VERSION >= 80000
                 || isa<ConstantExpr>(parent)
#endif
                 || isa<SubstNonTypeTemplateParmExpr>(parent))
        {
            break;
        }
        else
        {
            bPotentiallyReadFrom = true;
            bDump = true;
            break;
        }
    } while (true);

    if (bDump)
    {
        report(DiagnosticsEngine::Warning, "oh dear, what can the matter be?",
               compat::getBeginLoc(declRefExpr))
            << declRefExpr->getSourceRange();
        report(DiagnosticsEngine::Note, "parent over here", compat::getBeginLoc(parent))
            << parent->getSourceRange();
        parent->dump();
        declRefExpr->dump();
    }

    if (bPotentiallyReadFrom)
        readFromSet.insert(niceName(varDecl));
}

void UnusedVarsGlobal::checkIfWrittenTo(const VarDecl* varDecl, const DeclRefExpr* declRefExpr)
{
    // if we're inside a block that looks like
    //   if (varDecl)
    //       ...
    // then writes to this field don't matter, because unless we find another write to this field, this field is dead
    if (std::find(insideConditionalCheckOfVarSet.begin(), insideConditionalCheckOfVarSet.end(),
                  varDecl)
        != insideConditionalCheckOfVarSet.end())
        return;

    auto parentsRange = compiler.getASTContext().getParents(*declRefExpr);
    const Stmt* child = declRefExpr;
    const Stmt* parent
        = parentsRange.begin() == parentsRange.end() ? nullptr : parentsRange.begin()->get<Stmt>();
    // walk up the tree until we find something interesting
    bool bPotentiallyWrittenTo = false;
    bool bDump = false;
    auto walkUp = [&]() {
        child = parent;
        auto parentsRange = compiler.getASTContext().getParents(*parent);
        parent = parentsRange.begin() == parentsRange.end() ? nullptr
                                                            : parentsRange.begin()->get<Stmt>();
    };
    do
    {
        if (!parent)
        {
            // check if we have an expression like
            //    int& r = m_field;
            auto parentsRange = compiler.getASTContext().getParents(*child);
            if (parentsRange.begin() != parentsRange.end())
            {
                auto varDecl = dyn_cast_or_null<VarDecl>(parentsRange.begin()->get<Decl>());
                // The isImplicit() call is to avoid triggering when we see the vardecl which is part of a for-range statement,
                // which is of type 'T&&' and also an l-value-ref ?
                if (varDecl && !varDecl->isImplicit()
                    && loplugin::TypeCheck(varDecl->getType()).LvalueReference().NonConst())
                {
                    bPotentiallyWrittenTo = true;
                }
            }
            break;
        }
        if (isa<CXXReinterpretCastExpr>(parent))
        {
            // once we see one of these, there is not much useful we can know
            bPotentiallyWrittenTo = true;
            break;
        }
        else if (isa<CastExpr>(parent) || isa<MemberExpr>(parent) || isa<ParenExpr>(parent)
                 || isa<ParenListExpr>(parent) || isa<ArrayInitLoopExpr>(parent)
                 || isa<ExprWithCleanups>(parent))
        {
            walkUp();
        }
        else if (auto unaryOperator = dyn_cast<UnaryOperator>(parent))
        {
            UnaryOperator::Opcode op = unaryOperator->getOpcode();
            if (op == UO_AddrOf || op == UO_PostInc || op == UO_PostDec || op == UO_PreInc
                || op == UO_PreDec)
            {
                bPotentiallyWrittenTo = true;
            }
            break;
        }
        else if (auto arraySubscriptExpr = dyn_cast<ArraySubscriptExpr>(parent))
        {
            if (arraySubscriptExpr->getIdx() == child)
                break;
            walkUp();
        }
        else if (auto operatorCallExpr = dyn_cast<CXXOperatorCallExpr>(parent))
        {
            auto callee = getCallee(operatorCallExpr);
            if (callee)
            {
                // if calling a non-const operator on the field
                auto calleeMethodDecl = callee->getAsCXXMethodDecl();
                if (calleeMethodDecl && operatorCallExpr->getArg(0) == child)
                {
                    if (!calleeMethodDecl->isConst())
                        bPotentiallyWrittenTo
                            = checkForWriteWhenUsingCollectionType(calleeMethodDecl);
                }
                else if (IsPassedByNonConst(varDecl, child, operatorCallExpr, *callee))
                {
                    bPotentiallyWrittenTo = true;
                }
            }
            else
                bPotentiallyWrittenTo = true; // conservative, could improve
            break;
        }
        else if (auto cxxMemberCallExpr = dyn_cast<CXXMemberCallExpr>(parent))
        {
            const CXXMethodDecl* calleeMethodDecl = cxxMemberCallExpr->getMethodDecl();
            if (calleeMethodDecl)
            {
                // if calling a non-const method on the field
                const Expr* tmp = dyn_cast<Expr>(child);
                if (tmp->isBoundMemberFunction(compiler.getASTContext()))
                {
                    tmp = dyn_cast<MemberExpr>(tmp)->getBase();
                }
                if (cxxMemberCallExpr->getImplicitObjectArgument() == tmp)
                {
                    if (!calleeMethodDecl->isConst())
                        bPotentiallyWrittenTo
                            = checkForWriteWhenUsingCollectionType(calleeMethodDecl);
                    break;
                }
                else if (IsPassedByNonConst(varDecl, child, cxxMemberCallExpr,
                                            CalleeWrapper(calleeMethodDecl)))
                    bPotentiallyWrittenTo = true;
            }
            else
                bPotentiallyWrittenTo = true; // can happen in templates
            break;
        }
        else if (auto cxxConstructExpr = dyn_cast<CXXConstructExpr>(parent))
        {
            if (IsPassedByNonConst(varDecl, child, cxxConstructExpr,
                                   CalleeWrapper(cxxConstructExpr)))
                bPotentiallyWrittenTo = true;
            break;
        }
        else if (auto callExpr = dyn_cast<CallExpr>(parent))
        {
            auto callee = getCallee(callExpr);
            if (callee)
            {
                if (IsPassedByNonConst(varDecl, child, callExpr, *callee))
                    bPotentiallyWrittenTo = true;
            }
            else
                bPotentiallyWrittenTo = true; // conservative, could improve
            break;
        }
        else if (auto binaryOp = dyn_cast<BinaryOperator>(parent))
        {
            BinaryOperator::Opcode op = binaryOp->getOpcode();
            const bool assignmentOp = op == BO_Assign || op == BO_MulAssign || op == BO_DivAssign
                                      || op == BO_RemAssign || op == BO_AddAssign
                                      || op == BO_SubAssign || op == BO_ShlAssign
                                      || op == BO_ShrAssign || op == BO_AndAssign
                                      || op == BO_XorAssign || op == BO_OrAssign;
            if (assignmentOp)
            {
                if (binaryOp->getLHS() == child)
                    bPotentiallyWrittenTo = true;
                else if (loplugin::TypeCheck(binaryOp->getLHS()->getType())
                             .LvalueReference()
                             .NonConst())
                    // if the LHS is a non-const reference, we could write to the field later on
                    bPotentiallyWrittenTo = true;
            }
            break;
        }
        else if (isa<ReturnStmt>(parent))
        {
            if (insideFunctionDecl)
            {
                auto tc = loplugin::TypeCheck(insideFunctionDecl->getReturnType());
                if (tc.LvalueReference().NonConst())
                    bPotentiallyWrittenTo = true;
            }
            break;
        }
        else if (isa<ConditionalOperator>(parent) || isa<SwitchStmt>(parent)
                 || isa<DeclStmt>(parent) || isa<WhileStmt>(parent) || isa<CXXNewExpr>(parent)
                 || isa<ForStmt>(parent) || isa<InitListExpr>(parent)
                 || isa<CXXDependentScopeMemberExpr>(parent) || isa<UnresolvedMemberExpr>(parent)
                 || isa<MaterializeTemporaryExpr>(parent) || isa<IfStmt>(parent)
                 || isa<DoStmt>(parent) || isa<CXXDeleteExpr>(parent)
                 || isa<UnaryExprOrTypeTraitExpr>(parent) || isa<CXXUnresolvedConstructExpr>(parent)
                 || isa<CompoundStmt>(parent) || isa<LabelStmt>(parent)
                 || isa<CXXForRangeStmt>(parent) || isa<CXXTypeidExpr>(parent)
                 || isa<DefaultStmt>(parent) || isa<GCCAsmStmt>(parent)
#if CLANG_VERSION >= 80000
                 || isa<ConstantExpr>(parent)
#endif
                 || isa<AtomicExpr>(parent) || isa<CXXDefaultArgExpr>(parent)
                 || isa<VAArgExpr>(parent) || isa<DeclRefExpr>(parent)
                 || isa<SubstNonTypeTemplateParmExpr>(parent) || isa<LambdaExpr>(parent)) // TODO
        {
            break;
        }
        else
        {
            bPotentiallyWrittenTo = true;
            bDump = true;
            break;
        }
    } while (true);

    if (bDump)
    {
        report(DiagnosticsEngine::Warning, "oh dear, what can the matter be? writtenTo=%0",
               compat::getBeginLoc(declRefExpr))
            << bPotentiallyWrittenTo << declRefExpr->getSourceRange();
        if (parent)
        {
            report(DiagnosticsEngine::Note, "parent over here", compat::getBeginLoc(parent))
                << parent->getSourceRange();
            parent->dump();
        }
        declRefExpr->dump();
        varDecl->getType()->dump();
    }

    if (bPotentiallyWrittenTo)
        writeToSet.insert(niceName(varDecl));
}

// return true if this not a collection type, or if it is a collection type, and we might be writing to it
bool UnusedVarsGlobal::checkForWriteWhenUsingCollectionType(const CXXMethodDecl* calleeMethodDecl)
{
    auto const tc = loplugin::TypeCheck(calleeMethodDecl->getParent());
    bool listLike = false, setLike = false, mapLike = false, cssSequence = false;
    if (tc.Class("deque").StdNamespace() || tc.Class("list").StdNamespace()
        || tc.Class("queue").StdNamespace() || tc.Class("vector").StdNamespace())
    {
        listLike = true;
    }
    else if (tc.Class("set").StdNamespace() || tc.Class("unordered_set").StdNamespace())
    {
        setLike = true;
    }
    else if (tc.Class("map").StdNamespace() || tc.Class("unordered_map").StdNamespace())
    {
        mapLike = true;
    }
    else if (tc.Class("Sequence")
                 .Namespace("uno")
                 .Namespace("star")
                 .Namespace("sun")
                 .Namespace("com")
                 .GlobalNamespace())
    {
        cssSequence = true;
    }
    else
        return true;

    if (calleeMethodDecl->isOverloadedOperator())
    {
        auto oo = calleeMethodDecl->getOverloadedOperator();
        if (oo == OO_Equal)
            return true;
        // This is operator[]. We only care about things that add elements to the collection.
        // if nothing modifies the size of the collection, then nothing useful
        // is stored in it.
        if (listLike)
            return false;
        return true;
    }

    auto name = calleeMethodDecl->getName();
    if (listLike || setLike || mapLike)
    {
        if (name == "reserve" || name == "shrink_to_fit" || name == "clear" || name == "erase"
            || name == "pop_back" || name == "pop_front" || name == "front" || name == "back"
            || name == "data" || name == "remove" || name == "remove_if" || name == "unique"
            || name == "sort" || name == "begin" || name == "end" || name == "rbegin"
            || name == "rend" || name == "at" || name == "find" || name == "equal_range"
            || name == "lower_bound" || name == "upper_bound")
            return false;
    }
    if (cssSequence)
    {
        if (name == "getArray" || name == "begin" || name == "end")
            return false;
    }

    return true;
}

bool UnusedVarsGlobal::IsPassedByNonConst(const VarDecl* varDecl, const Stmt* child,
                                          CallerWrapper callExpr, CalleeWrapper calleeFunctionDecl)
{
    unsigned len = std::min(callExpr.getNumArgs(), calleeFunctionDecl.getNumParams());
    // if it's an array, passing it by value to a method typically means the
    // callee takes a pointer and can modify the array
    if (varDecl->getType()->isConstantArrayType())
    {
        for (unsigned i = 0; i < len; ++i)
            if (callExpr.getArg(i) == child)
                if (loplugin::TypeCheck(calleeFunctionDecl.getParamType(i)).Pointer().NonConst())
                    return true;
    }
    else
    {
        for (unsigned i = 0; i < len; ++i)
            if (callExpr.getArg(i) == child)
                if (loplugin::TypeCheck(calleeFunctionDecl.getParamType(i))
                        .LvalueReference()
                        .NonConst())
                    return true;
    }
    return false;
}

llvm::Optional<CalleeWrapper> UnusedVarsGlobal::getCallee(CallExpr const* callExpr)
{
    FunctionDecl const* functionDecl = callExpr->getDirectCallee();
    if (functionDecl)
        return CalleeWrapper(functionDecl);

    // Extract the functionprototype from a type
    clang::Type const* calleeType = callExpr->getCallee()->getType().getTypePtr();
    if (auto pointerType = calleeType->getUnqualifiedDesugaredType()->getAs<clang::PointerType>())
    {
        if (auto prototype = pointerType->getPointeeType()
                                 ->getUnqualifiedDesugaredType()
                                 ->getAs<FunctionProtoType>())
        {
            return CalleeWrapper(prototype);
        }
    }

    return llvm::Optional<CalleeWrapper>();
}

loplugin::Plugin::Registration<UnusedVarsGlobal> X("unusedvarsglobal", false);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
