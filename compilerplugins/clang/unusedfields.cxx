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
static std::set<MyFieldInfo> touchedFromOutsideConstructorSet;
static std::set<MyFieldInfo> readFromSet;
static std::set<MyFieldInfo> writeToSet;
static std::set<MyFieldInfo> writeToOutsideConstructorSet;
static std::set<MyFieldInfo> definitionSet;

/**
 * Wrap the different kinds of callable and callee objects in the clang AST so I can define methods that handle everything.
 */
class CallerWrapper
{
    const CallExpr * m_callExpr;
    const CXXConstructExpr * m_cxxConstructExpr;
public:
    CallerWrapper(const CallExpr * callExpr) : m_callExpr(callExpr), m_cxxConstructExpr(nullptr) {}
    CallerWrapper(const CXXConstructExpr * cxxConstructExpr) : m_callExpr(nullptr), m_cxxConstructExpr(cxxConstructExpr) {}
    unsigned getNumArgs () const
    { return m_callExpr ? m_callExpr->getNumArgs() : m_cxxConstructExpr->getNumArgs(); }
    const Expr * getArg (unsigned i) const
    { return m_callExpr ? m_callExpr->getArg(i) : m_cxxConstructExpr->getArg(i); }
};
class CalleeWrapper
{
    const FunctionDecl *       m_calleeFunctionDecl = nullptr;
    const CXXConstructorDecl * m_cxxConstructorDecl = nullptr;
    const FunctionProtoType *  m_functionPrototype = nullptr;
public:
    explicit CalleeWrapper(const FunctionDecl * calleeFunctionDecl) : m_calleeFunctionDecl(calleeFunctionDecl) {}
    explicit CalleeWrapper(const CXXConstructExpr * cxxConstructExpr) : m_cxxConstructorDecl(cxxConstructExpr->getConstructor()) {}
    explicit CalleeWrapper(const FunctionProtoType * functionPrototype) : m_functionPrototype(functionPrototype) {}
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
    CXXMethodDecl const * getAsCXXMethodDecl() const
    {
        if (m_calleeFunctionDecl)
            return dyn_cast<CXXMethodDecl>(m_calleeFunctionDecl);
        return nullptr;
    }
};

class UnusedFields:
    public RecursiveASTVisitor<UnusedFields>, public loplugin::Plugin
{
public:
    explicit UnusedFields(loplugin::InstantiationData const & data):
        Plugin(data) {}

    virtual void run() override;

    bool shouldVisitTemplateInstantiations () const { return true; }
    bool shouldVisitImplicitCode() const { return true; }

    bool VisitFieldDecl( const FieldDecl* );
    bool VisitMemberExpr( const MemberExpr* );
    bool VisitDeclRefExpr( const DeclRefExpr* );
    bool VisitCXXConstructorDecl( const CXXConstructorDecl* );
    bool VisitInitListExpr( const InitListExpr* );
    bool TraverseCXXConstructorDecl( CXXConstructorDecl* );
    bool TraverseCXXMethodDecl( CXXMethodDecl* );
    bool TraverseFunctionDecl( FunctionDecl* );
    bool TraverseIfStmt( IfStmt* );

private:
    MyFieldInfo niceName(const FieldDecl*);
    void checkTouchedFromOutside(const FieldDecl* fieldDecl, const Expr* memberExpr);
    void checkWriteFromOutsideConstructor(const FieldDecl* fieldDecl, const Expr* memberExpr);
    void checkWriteOnly(const FieldDecl* fieldDecl, const Expr* memberExpr);
    void checkReadOnly(const FieldDecl* fieldDecl, const Expr* memberExpr);
    bool isSomeKindOfZero(const Expr* arg);
    bool IsPassedByNonConst(const FieldDecl* fieldDecl, const Stmt * child, CallerWrapper callExpr,
                                        CalleeWrapper calleeFunctionDecl);
    llvm::Optional<CalleeWrapper> getCallee(CallExpr const *);

    RecordDecl *   insideMoveOrCopyOrCloneDeclParent = nullptr;
    RecordDecl *   insideStreamOutputOperator = nullptr;
    // For reasons I do not understand, parentFunctionDecl() is not reliable, so
    // we store the parent function on the way down the AST.
    FunctionDecl * insideFunctionDecl = nullptr;
    std::vector<FieldDecl const *> insideConditionalCheckOfMemberSet;
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
        for (const MyFieldInfo & s : touchedFromOutsideConstructorSet)
            output += "outside-constructor:\t" + s.parentClass + "\t" + s.fieldName + "\n";
        for (const MyFieldInfo & s : readFromSet)
            output += "read:\t" + s.parentClass + "\t" + s.fieldName + "\n";
        for (const MyFieldInfo & s : writeToSet)
            output += "write:\t" + s.parentClass + "\t" + s.fieldName + "\n";
        for (const MyFieldInfo & s : writeToOutsideConstructorSet)
            output += "write-outside-constructor:\t" + s.parentClass + "\t" + s.fieldName + "\n";
        for (const MyFieldInfo & s : definitionSet)
            output += "definition:\t" + s.access + "\t" + s.parentClass + "\t" + s.fieldName + "\t" + s.fieldType + "\t" + s.sourceLocation + "\n";
        std::ofstream myfile;
        myfile.open( WORKDIR "/loplugin.unusedfields.log", std::ios::app | std::ios::out);
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
    loplugin::normalizeDotDotInFilePath(aInfo.sourceLocation);

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
    auto copy = insideMoveOrCopyOrCloneDeclParent;
    if (!ignoreLocation(cxxConstructorDecl) && cxxConstructorDecl->isThisDeclarationADefinition())
    {
        if (cxxConstructorDecl->isCopyOrMoveConstructor())
            insideMoveOrCopyOrCloneDeclParent = cxxConstructorDecl->getParent();
    }
    bool ret = RecursiveASTVisitor::TraverseCXXConstructorDecl(cxxConstructorDecl);
    insideMoveOrCopyOrCloneDeclParent = copy;
    return ret;
}

bool UnusedFields::TraverseCXXMethodDecl(CXXMethodDecl* cxxMethodDecl)
{
    auto copy1 = insideMoveOrCopyOrCloneDeclParent;
    auto copy2 = insideFunctionDecl;
    if (!ignoreLocation(cxxMethodDecl) && cxxMethodDecl->isThisDeclarationADefinition())
    {
        if (cxxMethodDecl->isCopyAssignmentOperator()
            || cxxMethodDecl->isMoveAssignmentOperator()
            || (cxxMethodDecl->getIdentifier() && cxxMethodDecl->getName() == "Clone"))
            insideMoveOrCopyOrCloneDeclParent = cxxMethodDecl->getParent();
    }
    insideFunctionDecl = cxxMethodDecl;
    bool ret = RecursiveASTVisitor::TraverseCXXMethodDecl(cxxMethodDecl);
    insideMoveOrCopyOrCloneDeclParent = copy1;
    insideFunctionDecl = copy2;
    return ret;
}

bool UnusedFields::TraverseFunctionDecl(FunctionDecl* functionDecl)
{
    auto copy1 = insideStreamOutputOperator;
    auto copy2 = insideFunctionDecl;
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
    insideStreamOutputOperator = copy1;
    insideFunctionDecl = copy2;
    return ret;
}

bool UnusedFields::TraverseIfStmt(IfStmt* ifStmt)
{
    FieldDecl const * memberFieldDecl = nullptr;
    Expr const * cond = ifStmt->getCond()->IgnoreParenImpCasts();
    if (auto memberExpr = dyn_cast<MemberExpr>(cond))
    {
        if ((memberFieldDecl = dyn_cast<FieldDecl>(memberExpr->getMemberDecl())))
            insideConditionalCheckOfMemberSet.push_back(memberFieldDecl);
    }
    bool ret = RecursiveASTVisitor::TraverseIfStmt(ifStmt);
    if (memberFieldDecl)
        insideConditionalCheckOfMemberSet.pop_back();
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
    if (insideMoveOrCopyOrCloneDeclParent || insideStreamOutputOperator)
    {
        RecordDecl const * cxxRecordDecl1 = fieldDecl->getParent();
        // we don't care about reads from a field when inside the copy/move constructor/operator= for that field
        if (cxxRecordDecl1 && (cxxRecordDecl1 == insideMoveOrCopyOrCloneDeclParent))
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
            auto callee = getCallee(callExpr);
            if (callee)
            {
                // FIXME perhaps a better solution here would be some kind of SAL_PARAM_WRITEONLY attribute
                // which we could scatter around.
                std::string name = callee->getNameAsString();
                std::transform(name.begin(), name.end(), name.begin(), easytolower);
                if (startswith(name, "read"))
                    // this is a write-only call
                    ;
                else if (name.find(">>=") != std::string::npos && callExpr->getArg(1) == child)
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
    {
        readFromSet.insert(fieldInfo);
        if (fieldInfo.fieldName == "nNextElementNumber")
        {
            parent->dump();
            memberExpr->dump();
        }
    }
}

void UnusedFields::checkReadOnly(const FieldDecl* fieldDecl, const Expr* memberExpr)
{
    if (insideMoveOrCopyOrCloneDeclParent)
    {
        RecordDecl const * cxxRecordDecl1 = fieldDecl->getParent();
        // we don't care about writes to a field when inside the copy/move constructor/operator= for that field
        if (cxxRecordDecl1 && (cxxRecordDecl1 == insideMoveOrCopyOrCloneDeclParent))
        {
            // ... but they matter to tbe can-be-const analysis
            checkWriteFromOutsideConstructor(fieldDecl, memberExpr);
            return;
        }
    }

    // if we're inside a block that looks like
    //   if (fieldDecl)
    //       ....
    // then writes to this field don't matter, because unless we find another write to this field, this field is dead
    if (std::find(insideConditionalCheckOfMemberSet.begin(), insideConditionalCheckOfMemberSet.end(), fieldDecl) != insideConditionalCheckOfMemberSet.end())
        return;

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
            // check if we have an expression like
            //    int& r = m_field;
            auto parentsRange = compiler.getASTContext().getParents(*child);
            if (parentsRange.begin() != parentsRange.end())
            {
                auto varDecl = dyn_cast_or_null<VarDecl>(parentsRange.begin()->get<Decl>());
                // The isImplicit() call is to avoid triggering when we see the vardecl which is part of a for-range statement,
                // which is of type 'T&&' and also an l-value-ref ?
                if (varDecl && !varDecl->isImplicit() && loplugin::TypeCheck(varDecl->getType()).LvalueReference().NonConst())
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
            }
            break;
        }
        else if (auto arraySubscriptExpr = dyn_cast<ArraySubscriptExpr>(parent))
        {
            if (arraySubscriptExpr->getIdx() == child)
                break;
            walkupUp();
        }
        else if (auto operatorCallExpr = dyn_cast<CXXOperatorCallExpr>(parent))
        {
            auto callee = getCallee(operatorCallExpr);
            if (callee)
            {
                // if calling a non-const operator on the field
                auto calleeMethodDecl = callee->getAsCXXMethodDecl();
                if (calleeMethodDecl
                    && operatorCallExpr->getArg(0) == child && !calleeMethodDecl->isConst())
                {
                    bPotentiallyWrittenTo = true;
                }
                else if (IsPassedByNonConst(fieldDecl, child, operatorCallExpr, *callee))
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
                if (IsPassedByNonConst(fieldDecl, child, cxxMemberCallExpr, CalleeWrapper(calleeMethodDecl)))
                    bPotentiallyWrittenTo = true;
            }
            else
                bPotentiallyWrittenTo = true; // can happen in templates
            break;
        }
        else if (auto cxxConstructExpr = dyn_cast<CXXConstructExpr>(parent))
        {
            if (IsPassedByNonConst(fieldDecl, child, cxxConstructExpr, CalleeWrapper(cxxConstructExpr)))
                bPotentiallyWrittenTo = true;
            break;
        }
        else if (auto callExpr = dyn_cast<CallExpr>(parent))
        {
            auto callee = getCallee(callExpr);
            if (callee) {
                if (IsPassedByNonConst(fieldDecl, child, callExpr, *callee))
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
            if (assignmentOp)
            {
                if (binaryOp->getLHS() == child)
                    bPotentiallyWrittenTo = true;
                else if (loplugin::TypeCheck(binaryOp->getLHS()->getType()).LvalueReference().NonConst())
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
        fieldDecl->getType()->dump();
    }

    MyFieldInfo fieldInfo = niceName(fieldDecl);
    if (bPotentiallyWrittenTo)
    {
        writeToSet.insert(fieldInfo);
        checkWriteFromOutsideConstructor(fieldDecl, memberExpr);
    }
}

bool UnusedFields::IsPassedByNonConst(const FieldDecl* fieldDecl, const Stmt * child, CallerWrapper callExpr,
                                         CalleeWrapper calleeFunctionDecl)
{
    unsigned len = std::min(callExpr.getNumArgs(),
                            calleeFunctionDecl.getNumParams());
    // if it's an array, passing it by value to a method typically means the
    // callee takes a pointer and can modify the array
    if (fieldDecl->getType()->isConstantArrayType())
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
                if (loplugin::TypeCheck(calleeFunctionDecl.getParamType(i)).LvalueReference().NonConst())
                    return true;
    }
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
    if (insideMoveOrCopyOrCloneDeclParent && cxxConstructorDecl->getParent() == insideMoveOrCopyOrCloneDeclParent)
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
bool UnusedFields::VisitInitListExpr( const InitListExpr* initListExpr)
{
    if (ignoreLocation( initListExpr ))
        return true;

    QualType varType = initListExpr->getType().getDesugaredType(compiler.getASTContext());
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
    const FunctionDecl* memberExprParentFunction = getParentFunctionDecl(memberExpr);
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
            if (!constructorDecl)
                touchedFromOutsideConstructorSet.insert(fieldInfo);
        } else {
            touchedFromOutsideSet.insert(fieldInfo);
        }
    }
}

// For the const-field analysis.
// Called when we have a write to a field, and we want to record that write only if it's writing from
// outside the constructor.
void UnusedFields::checkWriteFromOutsideConstructor(const FieldDecl* fieldDecl, const Expr* memberExpr) {
    const FunctionDecl* memberExprParentFunction = getParentFunctionDecl(memberExpr);
    bool doWrite = false;

    if (!memberExprParentFunction)
        // If we are not inside a function
        doWrite = true;
    else if (memberExprParentFunction->getParent() != fieldDecl->getParent())
        // or we are inside a method from another class (than the one the field belongs to)
        doWrite = true;
    else if (!isa<CXXConstructorDecl>(memberExprParentFunction))
        // or we are not inside constructor
        doWrite = true;

    if (doWrite)
        writeToOutsideConstructorSet.insert(niceName(fieldDecl));
}

llvm::Optional<CalleeWrapper> UnusedFields::getCallee(CallExpr const * callExpr)
{
    FunctionDecl const * functionDecl = callExpr->getDirectCallee();
    if (functionDecl)
        return CalleeWrapper(functionDecl);

    // Extract the functionprototype from a type
    clang::Type const * calleeType = callExpr->getCallee()->getType().getTypePtr();
    if (auto pointerType = calleeType->getUnqualifiedDesugaredType()->getAs<clang::PointerType>()) {
        if (auto prototype = pointerType->getPointeeType()->getUnqualifiedDesugaredType()->getAs<FunctionProtoType>()) {
            return CalleeWrapper(prototype);
        }
    }

    return llvm::Optional<CalleeWrapper>();
}

loplugin::Plugin::Registration< UnusedFields > X("unusedfields", false);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
