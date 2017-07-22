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
#include <iostream>
#include <fstream>
#include <set>
#include <algorithm>
#include "plugin.hxx"
#include "compat.hxx"
#include "check.hxx"

/**
This performs two analyses:
 (1) look for unused fields
 (2) look for fields that are write-only

We dmp a list of calls to methods, and a list of field definitions.
Then we will post-process the 2 lists and find the set of unused methods.

Be warned that it produces around 5G of log file.

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='unusedfields' check
  $ ./compilerplugins/clang/unusedfields.py

and then
  $ for dir in *; do make FORCE_COMPILE_ALL=1 UPDATE_FILES=$dir COMPILER_PLUGIN_TOOL='unusedfieldsremove' $dir; done
to auto-remove the method declarations

Note that the actual process may involve a fair amount of undoing, hand editing, and general messing around
to get it to work :-)

*/

namespace {

struct MyFieldInfo
{
    const RecordDecl* parentRecord;
    std::string parentClass;
    std::string fieldName;
    std::string fieldType;
    std::string sourceLocation;
    std::string access;
};
bool operator < (const MyFieldInfo &lhs, const MyFieldInfo &rhs)
{
    return std::tie(lhs.parentClass, lhs.fieldName)
         < std::tie(rhs.parentClass, rhs.fieldName);
}


// try to limit the voluminous output a little
static std::set<MyFieldInfo> touchedFromInsideSet;
static std::set<MyFieldInfo> touchedFromOutsideSet;
static std::set<MyFieldInfo> readFromSet;
static std::set<MyFieldInfo> writeToSet;
static std::set<MyFieldInfo> definitionSet;


class UnusedFields:
    public RecursiveASTVisitor<UnusedFields>, public loplugin::Plugin
{
public:
    explicit UnusedFields(InstantiationData const & data): Plugin(data) {}

    virtual void run() override;

    bool shouldVisitTemplateInstantiations () const { return true; }
    bool shouldVisitImplicitCode() const { return true; }

    bool VisitFieldDecl( const FieldDecl* );
    bool VisitMemberExpr( const MemberExpr* );
    bool VisitDeclRefExpr( const DeclRefExpr* );
    bool VisitCXXConstructorDecl( const CXXConstructorDecl* );
    bool VisitVarDecl( const VarDecl* );
    bool TraverseCXXConstructorDecl( CXXConstructorDecl* );
    bool TraverseCXXMethodDecl( CXXMethodDecl* );
    bool TraverseFunctionDecl( FunctionDecl* );

private:
    MyFieldInfo niceName(const FieldDecl*);
    void checkTouchedFromOutside(const FieldDecl* fieldDecl, const Expr* memberExpr);
    void checkWriteOnly(const FieldDecl* fieldDecl, const Expr* memberExpr);
    void checkReadOnly(const FieldDecl* fieldDecl, const Expr* memberExpr);
    bool isSomeKindOfZero(const Expr* arg);
    bool IsPassedByNonConstRef(const Stmt * child, const CallExpr * callExpr, const FunctionDecl * calleeFunctionDecl, bool bParamsAndArgsOffset);

    RecordDecl *   insideMoveOrCopyDeclParent;
    RecordDecl *   insideStreamOutputOperator;
    // For reasons I do not understand, parentFunctionDecl() is not reliable, so
    // we store the parent function on the way down the AST.
    FunctionDecl * insideFunctionDecl;
};

void UnusedFields::run()
{
    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

    if (!isUnitTestMode())
    {
        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile
        std::string output;
        for (const MyFieldInfo & s : touchedFromInsideSet)
            output += "inside:\t" + s.parentClass + "\t" + s.fieldName + "\n";
        for (const MyFieldInfo & s : touchedFromOutsideSet)
            output += "outside:\t" + s.parentClass + "\t" + s.fieldName + "\n";
        for (const MyFieldInfo & s : readFromSet)
            output += "read:\t" + s.parentClass + "\t" + s.fieldName + "\n";
        for (const MyFieldInfo & s : writeToSet)
            output += "write:\t" + s.parentClass + "\t" + s.fieldName + "\n";
        for (const MyFieldInfo & s : definitionSet)
            output += "definition:\t" + s.access + "\t" + s.parentClass + "\t" + s.fieldName + "\t" + s.fieldType + "\t" + s.sourceLocation + "\n";
        std::ofstream myfile;
        myfile.open( SRCDIR "/loplugin.unusedfields.log", std::ios::app | std::ios::out);
        myfile << output;
        myfile.close();
    }
    else
    {
        for (const MyFieldInfo & s : readFromSet)
            report(
                DiagnosticsEngine::Warning,
                "read %0",
                s.parentRecord->getLocStart())
                << s.fieldName;
        for (const MyFieldInfo & s : writeToSet)
            report(
                DiagnosticsEngine::Warning,
                "write %0",
                s.parentRecord->getLocStart())
                << s.fieldName;
    }
}


MyFieldInfo UnusedFields::niceName(const FieldDecl* fieldDecl)
{
    MyFieldInfo aInfo;

    const RecordDecl* recordDecl = fieldDecl->getParent();

    if (const CXXRecordDecl* cxxRecordDecl = dyn_cast<CXXRecordDecl>(recordDecl))
    {
        if (cxxRecordDecl->getTemplateInstantiationPattern())
            cxxRecordDecl = cxxRecordDecl->getTemplateInstantiationPattern();
        aInfo.parentRecord = cxxRecordDecl;
        aInfo.parentClass = cxxRecordDecl->getQualifiedNameAsString();
    }
    else
    {
        aInfo.parentRecord = recordDecl;
        aInfo.parentClass = recordDecl->getQualifiedNameAsString();
    }

    aInfo.fieldName = fieldDecl->getNameAsString();
    // sometimes the name (if it's an anonymous thing) contains the full path of the build folder, which we don't need
    size_t idx = aInfo.fieldName.find(SRCDIR);
    if (idx != std::string::npos) {
        aInfo.fieldName = aInfo.fieldName.replace(idx, strlen(SRCDIR), "");
    }
    aInfo.fieldType = fieldDecl->getType().getAsString();

    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( fieldDecl->getLocation() );
    StringRef name = compiler.getSourceManager().getFilename(expansionLoc);
    aInfo.sourceLocation = std::string(name.substr(strlen(SRCDIR)+1)) + ":" + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
    normalizeDotDotInFilePath(aInfo.sourceLocation);

    switch (fieldDecl->getAccess())
    {
    case AS_public: aInfo.access = "public"; break;
    case AS_private: aInfo.access = "private"; break;
    case AS_protected: aInfo.access = "protected"; break;
    default: aInfo.access = "unknown"; break;
    }

    return aInfo;
}

bool UnusedFields::VisitFieldDecl( const FieldDecl* fieldDecl )
{
    fieldDecl = fieldDecl->getCanonicalDecl();
    if (ignoreLocation( fieldDecl )) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(fieldDecl->getLocation()))) {
        return true;
    }

    if (fieldDecl->getInClassInitializer() && !isSomeKindOfZero(fieldDecl->getInClassInitializer())) {
        writeToSet.insert(niceName(fieldDecl));
    }

    definitionSet.insert(niceName(fieldDecl));
    return true;
}

/**
 Does the expression being used to initialise a field value evaluate to
 the same as a default value?
 */
bool UnusedFields::isSomeKindOfZero(const Expr* arg)
{
    assert(arg);
    arg = arg->IgnoreParenCasts();
    if (isa<CXXDefaultArgExpr>(arg)) {
        arg = dyn_cast<CXXDefaultArgExpr>(arg)->getExpr();
    }
    arg = arg->IgnoreParenCasts();
    // ignore this, it seems to trigger an infinite recursion
    if (isa<UnaryExprOrTypeTraitExpr>(arg)) {
        return false;
    }
    if (auto cxxConstructExpr = dyn_cast<CXXConstructExpr>(arg)) {
        return cxxConstructExpr->getConstructor()->isDefaultConstructor();
    }
    APSInt x1;
    if (arg->EvaluateAsInt(x1, compiler.getASTContext()))
    {
        return x1 == 0;
    }
    if (isa<CXXNullPtrLiteralExpr>(arg)) {
        return true;
    }
    if (isa<MaterializeTemporaryExpr>(arg))
    {
        const CXXBindTemporaryExpr* strippedArg = dyn_cast_or_null<CXXBindTemporaryExpr>(arg->IgnoreParenCasts());
        if (strippedArg)
        {
            auto temp = dyn_cast<CXXTemporaryObjectExpr>(strippedArg->getSubExpr());
            if (temp->getNumArgs() == 0)
            {
                if (loplugin::TypeCheck(temp->getType()).Class("OUString").Namespace("rtl").GlobalNamespace()) {
                    return true;
                }
                if (loplugin::TypeCheck(temp->getType()).Class("OString").Namespace("rtl").GlobalNamespace()) {
                    return true;
                }
                return false;
            }
        }
    }

    // Get the expression contents.
    // This helps us find params which are always initialised with something like "OUString()".
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = arg->getLocStart();
    SourceLocation endLoc = arg->getLocEnd();
    const char *p1 = SM.getCharacterData( startLoc );
    const char *p2 = SM.getCharacterData( endLoc );
    if (!p1 || !p2 || (p2 - p1) < 0 || (p2 - p1) > 40) {
        return false;
    }
    unsigned n = Lexer::MeasureTokenLength( endLoc, SM, compiler.getLangOpts());
    std::string s( p1, p2 - p1 + n);
    // strip linefeed and tab characters so they don't interfere with the parsing of the log file
    std::replace( s.begin(), s.end(), '\r', ' ');
    std::replace( s.begin(), s.end(), '\n', ' ');
    std::replace( s.begin(), s.end(), '\t', ' ');

    // now normalize the value. For some params, like OUString, we can pass it as OUString() or "" and they are the same thing
    if (s == "OUString()")
        return true;
    else if (s == "OString()")
        return true;
    else if (s == "aEmptyOUStr") //sw
        return true;
    else if (s == "EMPTY_OUSTRING")//sc
        return true;
    else if (s == "GetEmptyOUString()") //sc
        return true;
    return false;
}

static char easytolower(char in)
{
    if (in<='Z' && in>='A')
        return in-('Z'-'z');
    return in;
}

bool startswith(const std::string& rStr, const char* pSubStr)
{
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

bool UnusedFields::TraverseCXXConstructorDecl(CXXConstructorDecl* cxxConstructorDecl)
{
    if (!ignoreLocation(cxxConstructorDecl) && cxxConstructorDecl->isThisDeclarationADefinition())
    {
        if (cxxConstructorDecl->isCopyOrMoveConstructor())
            insideMoveOrCopyDeclParent = cxxConstructorDecl->getParent();
    }
    bool ret = RecursiveASTVisitor::TraverseCXXConstructorDecl(cxxConstructorDecl);
    insideMoveOrCopyDeclParent = nullptr;
    return ret;
}

bool UnusedFields::TraverseCXXMethodDecl(CXXMethodDecl* cxxMethodDecl)
{
    if (!ignoreLocation(cxxMethodDecl) && cxxMethodDecl->isThisDeclarationADefinition())
    {
        if (cxxMethodDecl->isCopyAssignmentOperator() || cxxMethodDecl->isMoveAssignmentOperator())
            insideMoveOrCopyDeclParent = cxxMethodDecl->getParent();
    }
    insideFunctionDecl = cxxMethodDecl;
    bool ret = RecursiveASTVisitor::TraverseCXXMethodDecl(cxxMethodDecl);
    insideMoveOrCopyDeclParent = nullptr;
    insideFunctionDecl = nullptr;
    return ret;
}

bool UnusedFields::TraverseFunctionDecl(FunctionDecl* functionDecl)
{
    if (functionDecl->getLocation().isValid() && !ignoreLocation(functionDecl) && functionDecl->isThisDeclarationADefinition())
    {
        if (functionDecl->getOverloadedOperator() == OO_LessLess
            && functionDecl->getNumParams() == 2)
        {
            QualType qt = functionDecl->getParamDecl(1)->getType();
            insideStreamOutputOperator = qt.getNonReferenceType().getUnqualifiedType()->getAsCXXRecordDecl();
        }
    }
    insideFunctionDecl = functionDecl;
    bool ret = RecursiveASTVisitor::TraverseFunctionDecl(functionDecl);
    insideStreamOutputOperator = nullptr;
    insideFunctionDecl = nullptr;
    return ret;
}

bool UnusedFields::VisitMemberExpr( const MemberExpr* memberExpr )
{
    const ValueDecl* decl = memberExpr->getMemberDecl();
    const FieldDecl* fieldDecl = dyn_cast<FieldDecl>(decl);
    if (!fieldDecl) {
        return true;
    }
    fieldDecl = fieldDecl->getCanonicalDecl();
    if (ignoreLocation(fieldDecl)) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(fieldDecl->getLocation()))) {
        return true;
    }

    checkTouchedFromOutside(fieldDecl, memberExpr);

    checkWriteOnly(fieldDecl, memberExpr);

    checkReadOnly(fieldDecl, memberExpr);

    return true;
}

void UnusedFields::checkWriteOnly(const FieldDecl* fieldDecl, const Expr* memberExpr)
{
    if (insideMoveOrCopyDeclParent || insideStreamOutputOperator)
    {
        RecordDecl const * cxxRecordDecl1 = fieldDecl->getParent();
        // we don't care about reads from a field when inside the copy/move constructor/operator= for that field
        if (cxxRecordDecl1 && (cxxRecordDecl1 == insideMoveOrCopyDeclParent))
            return;
        // we don't care about reads when the field is being used in an output operator, this is normally
        // debug stuff
        if (cxxRecordDecl1 && (cxxRecordDecl1 == insideStreamOutputOperator))
            return;
    }

    auto parentsRange = compiler.getASTContext().getParents(*memberExpr);
    const Stmt* child = memberExpr;
    const Stmt* parent = parentsRange.begin() == parentsRange.end() ? nullptr : parentsRange.begin()->get<Stmt>();
    // walk up the tree until we find something interesting
    bool bPotentiallyReadFrom = false;
    bool bDump = false;
    auto walkupUp = [&]() {
       child = parent;
       auto parentsRange = compiler.getASTContext().getParents(*parent);
       parent = parentsRange.begin() == parentsRange.end() ? nullptr : parentsRange.begin()->get<Stmt>();
    };
    do
    {
        if (!parent)
        {
            // check if we're inside a CXXCtorInitializer or a VarDecl
            auto parentsRange = compiler.getASTContext().getParents(*child);
            if ( parentsRange.begin() != parentsRange.end())
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
        else if (isa<CastExpr>(parent) || isa<MemberExpr>(parent) || isa<ParenExpr>(parent) || isa<ParenListExpr>(parent)
#if CLANG_VERSION >= 40000
             || isa<ArrayInitLoopExpr>(parent)
#endif
             || isa<ExprWithCleanups>(parent))
        {
            walkupUp();
        }
        else if (auto unaryOperator = dyn_cast<UnaryOperator>(parent))
        {
            UnaryOperator::Opcode op = unaryOperator->getOpcode();
            if (memberExpr->getType()->isArrayType() && op == UO_Deref)
            {
                // ignore, deref'ing an array does not count as a read
            }
            else if (op == UO_AddrOf || op == UO_Deref
                || op == UO_Plus || op == UO_Minus
                || op == UO_Not || op == UO_LNot
                || op == UO_PreInc || op == UO_PostInc
                || op == UO_PreDec || op == UO_PostDec)
            {
                bPotentiallyReadFrom = true;
                break;
            }
            walkupUp();
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
            walkupUp();
        }
        else if (auto callExpr = dyn_cast<CallExpr>(parent))
        {
            // check for calls to ReadXXX() type methods and the operator>>= methods on Any.
            const FunctionDecl * calleeFunctionDecl = callExpr->getDirectCallee();
            if (calleeFunctionDecl && calleeFunctionDecl->getIdentifier())
            {
                std::string name = calleeFunctionDecl->getNameAsString();
                std::transform(name.begin(), name.end(), name.begin(), easytolower);
                if (startswith(name, "read") || name.find(">>=") != std::string::npos)
                    // this is a write-only call
                    ;
                else if (name == "clear" || name == "dispose" || name == "disposeAndClear" || name == "swap")
                    // we're abusing the write-only analysis here to look for fields which don't have anything useful
                    // being done to them, so we're ignoring things like std::vector::clear, std::vector::swap,
                    // and VclPtr::disposeAndClear
                    ;
                else
                    bPotentiallyReadFrom = true;
            }
            else
                bPotentiallyReadFrom = true;
            break;
        }
        else if (auto binaryOp = dyn_cast<BinaryOperator>(parent))
        {
            BinaryOperator::Opcode op = binaryOp->getOpcode();
            // If the child is on the LHS and it is an assignment op, we are obviously not reading from it
            const bool assignmentOp = op == BO_Assign || op == BO_MulAssign
                || op == BO_DivAssign || op == BO_RemAssign || op == BO_AddAssign
                || op == BO_SubAssign || op == BO_ShlAssign || op == BO_ShrAssign
                || op == BO_AndAssign || op == BO_XorAssign || op == BO_OrAssign;
            if (!(binaryOp->getLHS() == child && assignmentOp)) {
                bPotentiallyReadFrom = true;
            }
            break;
        }
        else if (isa<ReturnStmt>(parent)
                 || isa<CXXConstructExpr>(parent)
                 || isa<ConditionalOperator>(parent)
                 || isa<SwitchStmt>(parent)
                 || isa<DeclStmt>(parent)
                 || isa<WhileStmt>(parent)
                 || isa<CXXNewExpr>(parent)
                 || isa<ForStmt>(parent)
                 || isa<InitListExpr>(parent)
                 || isa<CXXDependentScopeMemberExpr>(parent)
                 || isa<UnresolvedMemberExpr>(parent)
                 || isa<MaterializeTemporaryExpr>(parent))
        {
            bPotentiallyReadFrom = true;
            break;
        }
        else if (isa<CXXDeleteExpr>(parent)
                 || isa<UnaryExprOrTypeTraitExpr>(parent)
                 || isa<CXXUnresolvedConstructExpr>(parent)
                 || isa<CompoundStmt>(parent)
                 || isa<LabelStmt>(parent)
                 || isa<CXXForRangeStmt>(parent)
                 || isa<CXXTypeidExpr>(parent)
                 || isa<DefaultStmt>(parent))
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
        report(
             DiagnosticsEngine::Warning,
             "oh dear, what can the matter be?",
              memberExpr->getLocStart())
              << memberExpr->getSourceRange();
        report(
             DiagnosticsEngine::Note,
             "parent over here",
              parent->getLocStart())
              << parent->getSourceRange();
        parent->dump();
        memberExpr->dump();
    }

    MyFieldInfo fieldInfo = niceName(fieldDecl);
    if (bPotentiallyReadFrom)
        readFromSet.insert(fieldInfo);
}

void UnusedFields::checkReadOnly(const FieldDecl* fieldDecl, const Expr* memberExpr)
{
    if (insideMoveOrCopyDeclParent)
    {
        RecordDecl const * cxxRecordDecl1 = fieldDecl->getParent();
        // we don't care about writes to a field when inside the copy/move constructor/operator= for that field
        if (cxxRecordDecl1 && (cxxRecordDecl1 == insideMoveOrCopyDeclParent))
            return;
    }

    auto parentsRange = compiler.getASTContext().getParents(*memberExpr);
    const Stmt* child = memberExpr;
    const Stmt* parent = parentsRange.begin() == parentsRange.end() ? nullptr : parentsRange.begin()->get<Stmt>();
    // walk up the tree until we find something interesting
    bool bPotentiallyWrittenTo = false;
    bool bDump = false;
    auto walkupUp = [&]() {
       child = parent;
       auto parentsRange = compiler.getASTContext().getParents(*parent);
       parent = parentsRange.begin() == parentsRange.end() ? nullptr : parentsRange.begin()->get<Stmt>();
    };
    do
    {
        if (!parent)
        {
            break;
        }
        if (isa<CXXReinterpretCastExpr>(parent))
        {
            // once we see one of these, there is not much useful we can know
            bPotentiallyWrittenTo = true;
            break;
        }
        else if (isa<CastExpr>(parent) || isa<MemberExpr>(parent) || isa<ParenExpr>(parent) || isa<ParenListExpr>(parent)
#if CLANG_VERSION >= 40000
             || isa<ArrayInitLoopExpr>(parent)
#endif
             || isa<ExprWithCleanups>(parent))
        {
            walkupUp();
        }
        else if (auto unaryOperator = dyn_cast<UnaryOperator>(parent))
        {
            UnaryOperator::Opcode op = unaryOperator->getOpcode();
            if (op == UO_AddrOf || op == UO_PostInc || op == UO_PostDec || op == UO_PreInc || op == UO_PreDec)
            {
                bPotentiallyWrittenTo = true;
                break;
            }
            walkupUp();
        }
        else if (auto arraySubscriptExpr = dyn_cast<ArraySubscriptExpr>(parent))
        {
            if (arraySubscriptExpr->getIdx() == child)
                break;
            walkupUp();
        }
        else if (auto operatorCallExpr = dyn_cast<CXXOperatorCallExpr>(parent))
        {
            const FunctionDecl * calleeFunctionDecl = operatorCallExpr->getDirectCallee();
            if (calleeFunctionDecl)
            {
                // if calling a non-const operator on the field
                auto calleeMethodDecl = dyn_cast<CXXMethodDecl>(calleeFunctionDecl);
                if (calleeMethodDecl
                    && operatorCallExpr->getArg(0) == child && !calleeMethodDecl->isConst())
                {
                    bPotentiallyWrittenTo = true;
                    break;
                }
                bool bParamsAndArgsOffset = calleeMethodDecl != nullptr;
                if (IsPassedByNonConstRef(child, operatorCallExpr, calleeFunctionDecl, bParamsAndArgsOffset))
                    bPotentiallyWrittenTo = true;
            }
            else
                bPotentiallyWrittenTo = true; // conservative, could improve
            break;
        }
        else if (auto cxxMemberCallExpr = dyn_cast<CXXMemberCallExpr>(parent))
        {
            const CXXMethodDecl * calleeMethodDecl = cxxMemberCallExpr->getMethodDecl();
            if (calleeMethodDecl)
            {
                // if calling a non-const method on the field
                const Expr* tmp = dyn_cast<Expr>(child);
                if (tmp->isBoundMemberFunction(compiler.getASTContext())) {
                    tmp = dyn_cast<MemberExpr>(tmp)->getBase();
                }
                if (cxxMemberCallExpr->getImplicitObjectArgument() == tmp
                     && !calleeMethodDecl->isConst())
                {
                    bPotentiallyWrittenTo = true;
                    break;
                }
                // check for being passed as parameter by non-const-reference
                if (IsPassedByNonConstRef(child, cxxMemberCallExpr, calleeMethodDecl, false/*bParamsAndArgsOffset*/))
                    bPotentiallyWrittenTo = true;
            }
            else
                bPotentiallyWrittenTo = true; // can happen in templates
            break;
        }
        else if (auto cxxConstructExpr = dyn_cast<CXXConstructExpr>(parent))
        {
            const CXXConstructorDecl * cxxConstructorDecl = cxxConstructExpr->getConstructor();
            // check for being passed as parameter by non-const-reference
            unsigned len = std::min(cxxConstructExpr->getNumArgs(),
                                    cxxConstructorDecl->getNumParams());
            for (unsigned i = 0; i < len; ++i)
                if (cxxConstructExpr->getArg(i) == child)
                    if (loplugin::TypeCheck(cxxConstructorDecl->getParamDecl(i)->getType()).NonConst().LvalueReference())
                    {
                        bPotentiallyWrittenTo = true;
                        break;
                    }
            break;
        }
        else if (auto callExpr = dyn_cast<CallExpr>(parent))
        {
            const FunctionDecl * calleeFunctionDecl = callExpr->getDirectCallee();
            if (calleeFunctionDecl) {
                if (IsPassedByNonConstRef(child, callExpr, calleeFunctionDecl, false/*bParamsAndArgsOffset*/))
                    bPotentiallyWrittenTo = true;
            } else
                bPotentiallyWrittenTo = true; // conservative, could improve
            break;
        }
        else if (auto binaryOp = dyn_cast<BinaryOperator>(parent))
        {
            BinaryOperator::Opcode op = binaryOp->getOpcode();
            const bool assignmentOp = op == BO_Assign || op == BO_MulAssign
                || op == BO_DivAssign || op == BO_RemAssign || op == BO_AddAssign
                || op == BO_SubAssign || op == BO_ShlAssign || op == BO_ShrAssign
                || op == BO_AndAssign || op == BO_XorAssign || op == BO_OrAssign;
            if (binaryOp->getLHS() == child && assignmentOp) {
                bPotentiallyWrittenTo = true;
            }
            break;
        }
        else if (isa<ReturnStmt>(parent))
        {
            if (insideFunctionDecl && loplugin::TypeCheck(insideFunctionDecl->getReturnType()).NonConst().LvalueReference()) {
                bPotentiallyWrittenTo = true;
            }
            break;
        }
        else if (isa<ConditionalOperator>(parent)
                 || isa<SwitchStmt>(parent)
                 || isa<DeclStmt>(parent)
                 || isa<WhileStmt>(parent)
                 || isa<CXXNewExpr>(parent)
                 || isa<ForStmt>(parent)
                 || isa<InitListExpr>(parent)
                 || isa<CXXDependentScopeMemberExpr>(parent)
                 || isa<UnresolvedMemberExpr>(parent)
                 || isa<MaterializeTemporaryExpr>(parent)
                 || isa<IfStmt>(parent)
                 || isa<DoStmt>(parent)
                 || isa<CXXDeleteExpr>(parent)
                 || isa<UnaryExprOrTypeTraitExpr>(parent)
                 || isa<CXXUnresolvedConstructExpr>(parent)
                 || isa<CompoundStmt>(parent)
                 || isa<LabelStmt>(parent)
                 || isa<CXXForRangeStmt>(parent)
                 || isa<CXXTypeidExpr>(parent)
                 || isa<DefaultStmt>(parent))
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
        report(
             DiagnosticsEngine::Warning,
             "oh dear, what can the matter be? writtenTo=%0",
              memberExpr->getLocStart())
              << bPotentiallyWrittenTo
              << memberExpr->getSourceRange();
        if (parent)
        {
            report(
                 DiagnosticsEngine::Note,
                 "parent over here",
                  parent->getLocStart())
                  << parent->getSourceRange();
            parent->dump();
        }
        memberExpr->dump();
    }

    MyFieldInfo fieldInfo = niceName(fieldDecl);
    if (bPotentiallyWrittenTo)
        writeToSet.insert(fieldInfo);
}

bool UnusedFields::IsPassedByNonConstRef(const Stmt * child, const CallExpr * callExpr,
                                         const FunctionDecl * calleeFunctionDecl,
                                         bool bParamsAndArgsOffset)
{
    unsigned len = std::min(callExpr->getNumArgs() + (bParamsAndArgsOffset ? 1 : 0),
                            calleeFunctionDecl->getNumParams());
    for (unsigned i = 0; i < len; ++i)
        if (callExpr->getArg(i + (bParamsAndArgsOffset ? 1 : 0)) == child)
            if (loplugin::TypeCheck(calleeFunctionDecl->getParamDecl(i)->getType()).NonConst().LvalueReference())
                return true;
    return false;
}

// fields that are assigned via member initialisers do not get visited in VisitDeclRef, so
// have to do it here
bool UnusedFields::VisitCXXConstructorDecl( const CXXConstructorDecl* cxxConstructorDecl )
{
    if (ignoreLocation( cxxConstructorDecl )) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(cxxConstructorDecl->getLocation()))) {
        return true;
    }

    // templates make EvaluateAsInt crash inside clang
    if (cxxConstructorDecl->isDependentContext())
       return true;

    // we don't care about writes to a field when inside the copy/move constructor/operator= for that field
    if (insideMoveOrCopyDeclParent && cxxConstructorDecl->getParent() == insideMoveOrCopyDeclParent)
        return true;

    for(auto it = cxxConstructorDecl->init_begin(); it != cxxConstructorDecl->init_end(); ++it)
    {
        const CXXCtorInitializer* init = *it;
        const FieldDecl* fieldDecl = init->getMember();
        if (fieldDecl && init->getInit() && !isSomeKindOfZero(init->getInit()))
        {
            MyFieldInfo fieldInfo = niceName(fieldDecl);
            writeToSet.insert(fieldInfo);
        }
    }
    return true;
}

// Fields that are assigned via init-list-expr do not get visited in VisitDeclRef, so
// have to do it here.
// TODO could be more precise here about which fields are actually being written to
bool UnusedFields::VisitVarDecl( const VarDecl* varDecl)
{
    if (!varDecl->getLocation().isValid() || ignoreLocation( varDecl ))
        return true;
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(varDecl->getLocation())))
        return true;

    if (!varDecl->hasInit())
        return true;
    auto initListExpr = dyn_cast<InitListExpr>(varDecl->getInit()->IgnoreImplicit());
    if (!initListExpr)
        return true;

    // If this is an array, navigate down until we hit a record.
    // It appears to be somewhat painful to navigate down an array type structure reliably.
    QualType varType = varDecl->getType().getDesugaredType(compiler.getASTContext());
    while (varType->isArrayType() || varType->isConstantArrayType()
            || varType->isIncompleteArrayType() || varType->isVariableArrayType()
            || varType->isDependentSizedArrayType())
        varType = varType->getAsArrayTypeUnsafe()->getElementType().getDesugaredType(compiler.getASTContext());
    auto recordType = varType->getAs<RecordType>();
    if (!recordType)
        return true;

    auto recordDecl = recordType->getDecl();
    for (auto it = recordDecl->field_begin(); it != recordDecl->field_end(); ++it)
    {
        MyFieldInfo fieldInfo = niceName(*it);
        writeToSet.insert(fieldInfo);
    }

    return true;
}

bool UnusedFields::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    const Decl* decl = declRefExpr->getDecl();
    const FieldDecl* fieldDecl = dyn_cast<FieldDecl>(decl);
    if (!fieldDecl) {
        return true;
    }
    fieldDecl = fieldDecl->getCanonicalDecl();
    if (ignoreLocation(fieldDecl)) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(fieldDecl->getLocation()))) {
        return true;
    }
    checkTouchedFromOutside(fieldDecl, declRefExpr);
    return true;
}

void UnusedFields::checkTouchedFromOutside(const FieldDecl* fieldDecl, const Expr* memberExpr) {
    const FunctionDecl* memberExprParentFunction = parentFunctionDecl(memberExpr);
    const CXXMethodDecl* methodDecl = dyn_cast_or_null<CXXMethodDecl>(memberExprParentFunction);

    MyFieldInfo fieldInfo = niceName(fieldDecl);

    // it's touched from somewhere outside a class
    if (!methodDecl) {
        touchedFromOutsideSet.insert(fieldInfo);
        return;
    }

    auto constructorDecl = dyn_cast<CXXConstructorDecl>(methodDecl);
    if (methodDecl->isCopyAssignmentOperator() || methodDecl->isMoveAssignmentOperator()) {
        // ignore move/copy operator, it's self->self
    } else if (constructorDecl && (constructorDecl->isCopyConstructor() || constructorDecl->isMoveConstructor())) {
        // ignore move/copy constructor, it's self->self
    } else {
        if (memberExprParentFunction->getParent() == fieldDecl->getParent()) {
            touchedFromInsideSet.insert(fieldInfo);
        } else {
            touchedFromOutsideSet.insert(fieldInfo);
        }
    }
}

loplugin::Plugin::Registration< UnusedFields > X("unusedfields", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
