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

#include "config_clang.h"

#include "compat.hxx"
#include "plugin.hxx"

#include "clang/AST/ParentMapContext.h"

/**
Look for fields that are only ever assigned a single constant value.

We dmp a list of values assigned to fields, and a list of field definitions.
Then we will post-process the 2 lists and find the set of interesting fields.

Be warned that it produces around 5G of log file.

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE=all COMPILER_PLUGIN_TOOL='singlevalfields' check
  $ ./compilerplugins/clang/singlevalfields.py

Note that the actual process may involve a fair amount of undoing, hand editing, and general messing around
to get it to work :-)

@TODO we don't spot fields that have been zero-initialised via calloc or rtl_allocateZeroMemory or memset
@TODO calls to lambdas where a reference to the field is taken

*/

namespace {

struct MyFieldInfo
{
    FieldDecl const * fieldDecl;
    std::string parentClass;
    std::string fieldName;
    std::string fieldType;
    std::string sourceLocation;
};
bool operator < (const MyFieldInfo &lhs, const MyFieldInfo &rhs)
{
    return std::tie(lhs.parentClass, lhs.fieldName)
         < std::tie(rhs.parentClass, rhs.fieldName);
}

struct MyFieldAssignmentInfo : public MyFieldInfo
{
    std::string value;
};

bool operator < (const MyFieldAssignmentInfo &lhs, const MyFieldAssignmentInfo &rhs)
{
    return std::tie(lhs.parentClass, lhs.fieldName, lhs.value)
         < std::tie(rhs.parentClass, rhs.fieldName, rhs.value);
}


// try to limit the voluminous output a little
static std::set<MyFieldAssignmentInfo> assignedSet;
static std::set<MyFieldInfo> definitionSet;

/** escape the value string to make it easier to parse the output file in python */
std::string escape(std::string s)
{
    std::string out;
    for (size_t i=0; i<s.length(); ++i)
        if (int(s[i]) >= 32)
            out += s[i];
        else
            out += "\\" + std::to_string((int)s[i]);
    return out;
}

class SingleValFields:
    public RecursiveASTVisitor<SingleValFields>, public loplugin::Plugin
{
public:
    explicit SingleValFields(loplugin::InstantiationData const & data):
        Plugin(data) {}

    virtual void run() override
    {
        handler.enableTreeWideAnalysisMode();

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        if (!isUnitTestMode())
        {
            // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
            // writing to the same logfile
            std::string output;
            for (const MyFieldAssignmentInfo & s : assignedSet)
                output += "asgn:\t" + s.parentClass + "\t" + s.fieldName + "\t" + escape(s.value) + "\n";
            for (const MyFieldInfo & s : definitionSet)
                output += "defn:\t" + s.parentClass + "\t" + s.fieldName + "\t" + s.fieldType + "\t" + s.sourceLocation + "\n";
            std::ofstream myfile;
            myfile.open( WORKDIR "/loplugin.singlevalfields.log", std::ios::app | std::ios::out);
            myfile << output;
            myfile.close();
        }
        else
        {
            for (const MyFieldAssignmentInfo & s : assignedSet)
                if (s.fieldDecl && compiler.getSourceManager().isInMainFile(s.fieldDecl->getBeginLoc()))
                    report(
                        DiagnosticsEngine::Warning,
                        "assign %0",
                        s.fieldDecl->getBeginLoc())
                        << s.value;
        }
    }

    bool shouldVisitTemplateInstantiations () const { return true; }
    // to catch compiler-generated constructors
    bool shouldVisitImplicitCode() const { return true; }

    bool VisitFieldDecl( const FieldDecl* );
    bool VisitVarDecl( const VarDecl* );
    bool VisitMemberExpr( const MemberExpr* );
    bool VisitDeclRefExpr( const DeclRefExpr* );
    bool VisitCXXConstructorDecl( const CXXConstructorDecl* );
//    bool VisitUnaryExprOrTypeTraitExpr( const UnaryExprOrTypeTraitExpr* );
private:
    void niceName(const DeclaratorDecl*, MyFieldInfo&);
    void walkPotentialAssign( const DeclaratorDecl* fieldOrVarDecl, const Stmt* stmt );
    std::string getExprValue(const Expr*);
    const FunctionDecl* get_top_FunctionDecl_from_Stmt(const Stmt&);
    void checkCallExpr(const Stmt* child, const CallExpr* callExpr, std::string& assignValue, bool& bPotentiallyAssignedTo);
};

void SingleValFields::niceName(const DeclaratorDecl* fieldOrVarDecl, MyFieldInfo& aInfo)
{
    const VarDecl* varDecl = dyn_cast<VarDecl>(fieldOrVarDecl);
    const FieldDecl* fieldDecl = dyn_cast<FieldDecl>(fieldOrVarDecl);
    aInfo.fieldDecl = fieldDecl;
    if (fieldDecl)
        aInfo.parentClass = fieldDecl->getParent()->getQualifiedNameAsString();
    else
    {
        if (auto parentRecordDecl = dyn_cast<CXXRecordDecl>(varDecl->getDeclContext()))
            aInfo.parentClass = parentRecordDecl->getQualifiedNameAsString();
        else if (auto parentMethodDecl = dyn_cast<CXXMethodDecl>(varDecl->getDeclContext()))
            aInfo.parentClass = parentMethodDecl->getQualifiedNameAsString();
        else if (auto parentFunctionDecl = dyn_cast<FunctionDecl>(varDecl->getDeclContext()))
            aInfo.parentClass = parentFunctionDecl->getQualifiedNameAsString();
        else if (isa<TranslationUnitDecl>(varDecl->getDeclContext()))
            aInfo.parentClass = handler.getMainFileName().str();
        else if (auto parentNamespaceDecl = dyn_cast<NamespaceDecl>(varDecl->getDeclContext()))
            aInfo.parentClass = parentNamespaceDecl->getQualifiedNameAsString();
        else if (isa<LinkageSpecDecl>(varDecl->getDeclContext()))
            aInfo.parentClass = "extern"; // what to do here?
        else
        {
            std::cout << "what is this? " << varDecl->getDeclContext()->getDeclKindName() << std::endl;
            exit(1);
        }
    }
    aInfo.fieldName = fieldOrVarDecl->getNameAsString();
    aInfo.fieldType = fieldOrVarDecl->getType().getAsString();

    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( fieldOrVarDecl->getLocation() );
    StringRef name = getFilenameOfLocation(expansionLoc);
    aInfo.sourceLocation = std::string(name.substr(strlen(SRCDIR)+1)) + ":" + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
    loplugin::normalizeDotDotInFilePath(aInfo.sourceLocation);
}

bool SingleValFields::VisitFieldDecl( const FieldDecl* fieldDecl )
{
    auto canonicalDecl = fieldDecl->getCanonicalDecl();

    if( isInUnoIncludeFile( compiler.getSourceManager().getSpellingLoc(canonicalDecl->getLocation())) )
        return true;

    MyFieldInfo aInfo;
    niceName(canonicalDecl, aInfo);
    definitionSet.insert(aInfo);

    if (fieldDecl->getInClassInitializer())
    {
        MyFieldAssignmentInfo aInfo;
        niceName(canonicalDecl, aInfo);
        aInfo.value = getExprValue(fieldDecl->getInClassInitializer());
        assignedSet.insert(aInfo);
    }

    return true;
}

bool SingleValFields::VisitVarDecl( const VarDecl* varDecl )
{
    if (isa<ParmVarDecl>(varDecl))
        return true;
    if (varDecl->getType().isConstQualified())
        return true;
    if (!(varDecl->isStaticLocal() || varDecl->isStaticDataMember() || varDecl->hasGlobalStorage()))
        return true;

    auto canonicalDecl = varDecl->getCanonicalDecl();
    if (!canonicalDecl->getLocation().isValid())
        return true;

    if( isInUnoIncludeFile( compiler.getSourceManager().getSpellingLoc(canonicalDecl->getLocation())) )
        return true;

    MyFieldInfo aInfo;
    niceName(canonicalDecl, aInfo);
    definitionSet.insert(aInfo);

    if (varDecl->getInit())
    {
        MyFieldAssignmentInfo aInfo;
        niceName(canonicalDecl, aInfo);
        aInfo.value = getExprValue(varDecl->getInit());
        assignedSet.insert(aInfo);
    }

    return true;
}

bool SingleValFields::VisitCXXConstructorDecl( const CXXConstructorDecl* decl )
{
    // doesn't count as a write to fields because it's self->self
    if (decl->isCopyOrMoveConstructor())
        return true;

    for(auto it = decl->init_begin(); it != decl->init_end(); ++it)
    {
        const CXXCtorInitializer* init = *it;
        const FieldDecl* fieldDecl = init->getMember();
        if( !fieldDecl )
            continue;
        MyFieldAssignmentInfo aInfo;
        niceName(fieldDecl, aInfo);
        const Expr * expr = init->getInit();
        // unwrap any single-arg constructors, this helps to find smart pointers
        // that are only assigned nullptr
        if (auto cxxConstructExpr = dyn_cast<CXXConstructExpr>(expr))
            if (cxxConstructExpr->getNumArgs() == 1)
                expr = cxxConstructExpr->getArg(0);
        aInfo.value = getExprValue(expr);
        assignedSet.insert(aInfo);
    }
    return true;
}

bool SingleValFields::VisitMemberExpr( const MemberExpr* memberExpr )
{
    const ValueDecl* decl = memberExpr->getMemberDecl();
    const FieldDecl* fieldDecl = dyn_cast<FieldDecl>(decl);
    if (!fieldDecl)
        return true;
    walkPotentialAssign(fieldDecl, memberExpr);
    return true;
}

bool SingleValFields::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    const VarDecl* varDecl = dyn_cast_or_null<VarDecl>(declRefExpr->getDecl());
    if (!varDecl)
        return true;
    if (isa<ParmVarDecl>(varDecl))
        return true;
    if (varDecl->getType().isConstQualified())
        return true;
    if (!(varDecl->isStaticLocal() || varDecl->isStaticDataMember() || varDecl->hasGlobalStorage()))
        return true;
    walkPotentialAssign(varDecl, declRefExpr);
    return true;
}

void SingleValFields::walkPotentialAssign( const DeclaratorDecl* fieldOrVarDecl, const Stmt* memberExpr )
{
    const FunctionDecl* parentFunction = getParentFunctionDecl(memberExpr);
    if (parentFunction)
    {
        auto methodDecl = dyn_cast<CXXMethodDecl>(parentFunction);
        if (methodDecl && (methodDecl->isCopyAssignmentOperator() || methodDecl->isMoveAssignmentOperator()))
           return;
        if (methodDecl && methodDecl->getIdentifier()
            && (compat::starts_with(methodDecl->getName(), "Clone") || compat::starts_with(methodDecl->getName(), "clone")))
           return;
        auto cxxConstructorDecl = dyn_cast<CXXConstructorDecl>(parentFunction);
        if (cxxConstructorDecl && cxxConstructorDecl->isCopyOrMoveConstructor())
           return;
    }

    // walk up the tree until we find something interesting
    const Stmt* child = memberExpr;
    const Stmt* parent = getParentStmt(memberExpr);
    bool bPotentiallyAssignedTo = false;
    bool bDump = false;
    std::string assignValue = "?";

    // check for field being returned by non-const ref eg. Foo& getFoo() { return f; }
    if (parentFunction && parent && isa<ReturnStmt>(parent)) {
        const Stmt* parent2 = getParentStmt(parent);
        if (parent2 && isa<CompoundStmt>(parent2)) {
            QualType qt = parentFunction->getReturnType().getDesugaredType(compiler.getASTContext());
            if (!qt.isConstQualified() && qt->isReferenceType()) {
                bPotentiallyAssignedTo = true;
            }
        }
    }

    while (!bPotentiallyAssignedTo) {
        // check for field being accessed by a reference variable e.g. Foo& f = m.foo;
        auto parentsList = compiler.getASTContext().getParents(*child);
        auto it = parentsList.begin();
        if (it != parentsList.end()) {
            const VarDecl *varDecl = it->get<VarDecl>();
            if (varDecl) {
                QualType qt = varDecl->getType().getDesugaredType(compiler.getASTContext());
                if (!qt.isConstQualified() && qt->isReferenceType()) {
                    bPotentiallyAssignedTo = true;
                    break;
                }
            }
        }

        if (!parent) {
            return;
        }
        if (isa<CastExpr>(parent) || isa<MemberExpr>(parent) || isa<ParenExpr>(parent) || isa<ParenListExpr>(parent)
             || isa<ExprWithCleanups>(parent))
        {
            child = parent;
            parent = getParentStmt(parent);
        }
        else if (isa<UnaryOperator>(parent))
        {
            const UnaryOperator* unaryOperator = dyn_cast<UnaryOperator>(parent);
            int x = unaryOperator->getOpcode();
            if (x == UO_AddrOf || x == UO_PostInc || x == UO_PostDec || x == UO_PreInc || x == UO_PreDec) {
                assignValue = "?";
                bPotentiallyAssignedTo = true;
                break;
            }
            // cannot be assigned to anymore
            break;
        }
        else if (auto callExpr = dyn_cast<CallExpr>(parent))
        {
            checkCallExpr(child, callExpr, assignValue, bPotentiallyAssignedTo);
            break;
        }
        else if (isa<CXXConstructExpr>(parent))
        {
            const CXXConstructExpr* consExpr = dyn_cast<CXXConstructExpr>(parent);
            const CXXConstructorDecl* consDecl = consExpr->getConstructor();
            for (unsigned i = 0; i < consExpr->getNumArgs(); ++i) {
                if (i >= consDecl->getNumParams()) // can happen in template code
                    break;
                if (consExpr->getArg(i) == child) {
                    const ParmVarDecl* parmVarDecl = consDecl->getParamDecl(i);
                    QualType qt = parmVarDecl->getType().getDesugaredType(compiler.getASTContext());
                    if (!qt.isConstQualified() && qt->isReferenceType()) {
                        bPotentiallyAssignedTo = true;
                    }
                    break;
                }
            }
            break;
        }
        else if (isa<BinaryOperator>(parent))
        {
            const BinaryOperator* binaryOp = dyn_cast<BinaryOperator>(parent);
            auto op = binaryOp->getOpcode();
            if ( binaryOp->getLHS() != child ) {
                // if the expr is on the RHS, do nothing
            }
            else if ( op == BO_Assign ) {
                assignValue = getExprValue(binaryOp->getRHS());
                bPotentiallyAssignedTo = true;
            } else if ( op == BO_MulAssign || op == BO_DivAssign
                        || op == BO_RemAssign || op == BO_AddAssign
                        || op == BO_SubAssign || op == BO_ShlAssign
                        || op == BO_ShrAssign || op == BO_AndAssign
                        || op == BO_XorAssign || op == BO_OrAssign )
            {
                bPotentiallyAssignedTo = true;
            }
            break;
        }
        else if ( isa<CompoundStmt>(parent)
                || isa<SwitchStmt>(parent) || isa<CaseStmt>(parent) || isa<DefaultStmt>(parent)
                || isa<DoStmt>(parent) || isa<WhileStmt>(parent)
                || isa<IfStmt>(parent)
                || isa<ForStmt>(parent)
                || isa<ReturnStmt>(parent)
                || isa<CXXNewExpr>(parent)
                || isa<CXXDeleteExpr>(parent)
                || isa<ConditionalOperator>(parent)
                || isa<CXXTypeidExpr>(parent)
                || isa<ArraySubscriptExpr>(parent)
                || isa<CXXDependentScopeMemberExpr>(parent)
                || isa<DeclStmt>(parent)
                || isa<UnaryExprOrTypeTraitExpr>(parent)
                || isa<UnresolvedMemberExpr>(parent)
                || isa<MaterializeTemporaryExpr>(parent)  //???
                || isa<InitListExpr>(parent)
                || isa<DesignatedInitExpr>(parent)
                || isa<CXXUnresolvedConstructExpr>(parent)
                || isa<LambdaExpr>(parent)
                || isa<PackExpansionExpr>(parent)
                || isa<CXXPseudoDestructorExpr>(parent)
                )
        {
            break;
        }
        else if ( isa<ArrayInitLoopExpr>(parent) || isa<AtomicExpr>(parent) || isa<GCCAsmStmt>(parent) || isa<VAArgExpr>(parent))
        {
            bPotentiallyAssignedTo = true;
            break;
        }
        else if (isa<DeclRefExpr>(parent)) // things like o3tl::convertNarrowing pass members as template params
        {
            break;
        }
        else {
            bPotentiallyAssignedTo = true;
            bDump = true;
            break;
        }
    }
    if (bDump)
    {
        report(
             DiagnosticsEngine::Warning,
             "oh dear, what can the matter be?",
              memberExpr->getBeginLoc())
              << memberExpr->getSourceRange();
        parent->dump();
    }
    if (bPotentiallyAssignedTo)
    {
        MyFieldAssignmentInfo aInfo;
        niceName(fieldOrVarDecl, aInfo);
        aInfo.value = assignValue;
        assignedSet.insert(aInfo);
    }
}

void SingleValFields::checkCallExpr(const Stmt* child, const CallExpr* callExpr, std::string& assignValue, bool& bPotentiallyAssignedTo)
{
    if (callExpr->getCallee() == child) {
        return;
    }
    const FunctionDecl* functionDecl;
    if (auto memberCallExpr = dyn_cast<CXXMemberCallExpr>(callExpr)) {
        functionDecl = memberCallExpr->getMethodDecl();
    } else {
        functionDecl = callExpr->getDirectCallee();
    }
    if (functionDecl) {
        if (auto operatorCallExpr = dyn_cast<CXXOperatorCallExpr>(callExpr)) {
            if (operatorCallExpr->getArg(0) == child) {
                const CXXMethodDecl* calleeMethodDecl = dyn_cast_or_null<CXXMethodDecl>(operatorCallExpr->getDirectCallee());
                if (calleeMethodDecl) {
                    if (operatorCallExpr->getOperator() == OO_Equal) {
                        assignValue = getExprValue(operatorCallExpr->getArg(1));
                        bPotentiallyAssignedTo = true;
                        return;
                    }
                }
            }
        }
        for (unsigned i = 0; i < callExpr->getNumArgs(); ++i) {
            if (i >= functionDecl->getNumParams()) // can happen in template code
                break;
            if (callExpr->getArg(i) == child) {
                const ParmVarDecl* parmVarDecl = functionDecl->getParamDecl(i);
                QualType qt = parmVarDecl->getType().getDesugaredType(compiler.getASTContext());
                if (!qt.isConstQualified() && qt->isReferenceType()) {
                    assignValue = "?";
                    bPotentiallyAssignedTo = true;
                }
                break;
            }
        }
        return;
    }
    // check for function pointers
    const FieldDecl* calleeFieldDecl = dyn_cast_or_null<FieldDecl>(callExpr->getCalleeDecl());
    if (!calleeFieldDecl) {
        return;
    }
    QualType qt = calleeFieldDecl->getType().getDesugaredType(compiler.getASTContext());
    if (!qt->isPointerType()) {
        return;
    }
    qt = qt->getPointeeType().getDesugaredType(compiler.getASTContext());
    const FunctionProtoType* proto = qt->getAs<FunctionProtoType>();
    if (!proto) {
        return;
    }
    for (unsigned i = 0; i < callExpr->getNumArgs(); ++i) {
        if (i >= proto->getNumParams()) // can happen in template code
            break;
        if (callExpr->getArg(i) == child) {
            QualType qt = proto->getParamType(i).getDesugaredType(compiler.getASTContext());
            if (!qt.isConstQualified() && qt->isReferenceType()) {
                assignValue = "?";
                bPotentiallyAssignedTo = true;
            }
            break;
        }
    }
}


std::string SingleValFields::getExprValue(const Expr* arg)
{
    if (!arg)
        return "?";
    arg = arg->IgnoreParenCasts();
    arg = arg->IgnoreImplicit();
    // ignore this, it seems to trigger an infinite recursion
    if (isa<UnaryExprOrTypeTraitExpr>(arg))
        return "?";
    if (arg->isValueDependent())
        return "?";
    // for stuff like: OUString foo = "xxx";
    if (auto stringLiteral = dyn_cast<clang::StringLiteral>(arg))
    {
        if (stringLiteral->getCharByteWidth() == 1)
            return stringLiteral->getString().str();
        return "?";
    }
    // ParenListExpr containing a CXXNullPtrLiteralExpr and has a NULL type pointer
    if (auto parenListExpr = dyn_cast<ParenListExpr>(arg))
    {
        if (parenListExpr->getNumExprs() == 1)
            return getExprValue(parenListExpr->getExpr(0));
        return "?";
    }
    if (auto constructExpr = dyn_cast<CXXConstructExpr>(arg))
    {
        if (constructExpr->getNumArgs() >= 1
            && isa<clang::StringLiteral>(constructExpr->getArg(0)))
        {
            auto stringLiteral = dyn_cast<clang::StringLiteral>(constructExpr->getArg(0));
            if (stringLiteral->getCharByteWidth() == 1)
                return stringLiteral->getString().str();
            return "?";
        }
    }
    if (arg->getType()->isFloatingType())
    {
        APFloat x1(0.0f);
        if (arg->EvaluateAsFloat(x1, compiler.getASTContext()))
        {
            std::string s;
            llvm::raw_string_ostream os(s);
            x1.print(os);
            return os.str();
        }
    }
    APSInt x1;
    if (compat::EvaluateAsInt(arg, x1, compiler.getASTContext()))
        return compat::toString(x1, 10);
    if (isa<CXXNullPtrLiteralExpr>(arg))
        return "0";
    return "?";
}

loplugin::Plugin::Registration< SingleValFields > X("singlevalfields", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
