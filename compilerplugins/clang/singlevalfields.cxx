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
#include "plugin.hxx"
#include "compat.hxx"

/**
Look for fields that are only ever assigned a single constant value.

We dmp a list of values assigned to fields, and a list of field definitions.
Then we will post-process the 2 lists and find the set of interesting fields.

Be warned that it produces around 5G of log file.

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='singlevalfields' check
  $ ./compilerplugins/clang/singlevalfields.py

Note that the actual process may involve a fair amount of undoing, hand editing, and general messing around
to get it to work :-)

@TODO we don't spot fields that have been zero-initialised via calloc or rtl_allocateZeroMemory or memset
@TODO calls to lambdas (see FIXME near CXXOperatorCallExpr)

*/

namespace {

struct MyFieldInfo
{
    std::string parentClass;
    std::string fieldName;
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


class SingleValFields:
    public RecursiveASTVisitor<SingleValFields>, public loplugin::Plugin
{
public:
    explicit SingleValFields(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile
        std::string output;
        for (const MyFieldAssignmentInfo & s : assignedSet)
            output += "asgn:\t" + s.parentClass + "\t" + s.fieldName + "\t" + s.value + "\n";
        for (const MyFieldInfo & s : definitionSet)
            output += "defn:\t" + s.parentClass + "\t" + s.fieldName + "\t" + s.sourceLocation + "\n";
        ofstream myfile;
        myfile.open( SRCDIR "/loplugin.singlevalfields.log", ios::app | ios::out);
        myfile << output;
        myfile.close();
    }

    bool shouldVisitTemplateInstantiations () const { return true; }
    // to catch compiler-generated constructors
    bool shouldVisitImplicitCode() const { return true; }

    bool VisitFieldDecl( const FieldDecl* );
    bool VisitMemberExpr( const MemberExpr* );
    bool VisitCXXConstructorDecl( const CXXConstructorDecl* );
    bool VisitImplicitCastExpr( const ImplicitCastExpr* );
//    bool VisitUnaryExprOrTypeTraitExpr( const UnaryExprOrTypeTraitExpr* );
private:
    void niceName(const FieldDecl*, MyFieldInfo&);
    std::string getExprValue(const Expr*);
    bool isInterestingType(const QualType&);
    const FunctionDecl* get_top_FunctionDecl_from_Stmt(const Stmt&);
    void checkCallExpr(const Stmt* child, const CallExpr* callExpr, std::string& assignValue, bool& bPotentiallyAssignedTo);
    void markAllFields(const RecordDecl* recordDecl);
};

void SingleValFields::niceName(const FieldDecl* fieldDecl, MyFieldInfo& aInfo)
{
    aInfo.parentClass = fieldDecl->getParent()->getQualifiedNameAsString();
    aInfo.fieldName = fieldDecl->getNameAsString();

    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( fieldDecl->getLocation() );
    StringRef name = compiler.getSourceManager().getFilename(expansionLoc);
    aInfo.sourceLocation = std::string(name.substr(strlen(SRCDIR)+1)) + ":" + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
}

bool SingleValFields::VisitFieldDecl( const FieldDecl* fieldDecl )
{
    fieldDecl = fieldDecl->getCanonicalDecl();
    const FieldDecl* canonicalDecl = fieldDecl;

    if( ignoreLocation( fieldDecl ) || !isInterestingType(fieldDecl->getType()) )
        return true;

    MyFieldInfo aInfo;
    niceName(canonicalDecl, aInfo);
    definitionSet.insert(aInfo);
    return true;
}

bool SingleValFields::VisitCXXConstructorDecl( const CXXConstructorDecl* decl )
{
    if( ignoreLocation( decl ) )
        return true;

    // doesn't count as a write to fields because it's self->self
    if (decl->isCopyOrMoveConstructor())
            return true;

    for(auto it = decl->init_begin(); it != decl->init_end(); ++it)
    {
        const CXXCtorInitializer* init = *it;
        const FieldDecl* fieldDecl = init->getMember();
        if( !fieldDecl || !isInterestingType(fieldDecl->getType()) )
            continue;
        MyFieldAssignmentInfo aInfo;
        niceName(fieldDecl, aInfo);
        aInfo.value = getExprValue(init->getInit());
        assignedSet.insert(aInfo);
    }
    return true;
}

/**
 * Check for calls to methods where a pointer to something is cast to a pointer to void.
 * At which case it could have anything written to it.
 */
bool SingleValFields::VisitImplicitCastExpr( const ImplicitCastExpr* castExpr )
{
    QualType qt = castExpr->getType().getDesugaredType(compiler.getASTContext());
    if (qt.isNull()) {
        return true;
    }
    if ( qt.isConstQualified() || !qt->isPointerType()
         || !qt->getAs<clang::PointerType>()->getPointeeType()->isVoidType() ) {
        return true;
    }
    const Expr* subExpr = castExpr->getSubExpr();
    qt = subExpr->getType();
    if (!qt->isPointerType()) {
        return true;
    }
    qt = qt->getPointeeType();
    if (!qt->isRecordType()) {
        return true;
    }
    const RecordDecl* recordDecl = qt->getAs<RecordType>()->getDecl();
    markAllFields(recordDecl);
    return true;
}

void SingleValFields::markAllFields(const RecordDecl* recordDecl)
{
    for(auto fieldDecl = recordDecl->field_begin();
        fieldDecl != recordDecl->field_end(); ++fieldDecl)
    {
        if (isInterestingType(fieldDecl->getType())) {
            MyFieldAssignmentInfo aInfo;
            niceName(*fieldDecl, aInfo);
            aInfo.value = "?";
            assignedSet.insert(aInfo);
        }
        else if (fieldDecl->getType()->isRecordType()) {
            markAllFields(fieldDecl->getType()->getAs<RecordType>()->getDecl());
        }
    }
    const CXXRecordDecl* cxxRecordDecl = dyn_cast<CXXRecordDecl>(recordDecl);
    if (!cxxRecordDecl || !cxxRecordDecl->hasDefinition()) {
        return;
    }
    for (auto it = cxxRecordDecl->bases_begin(); it != cxxRecordDecl->bases_end(); ++it)
    {
        QualType qt = it->getType();
        if (qt->isRecordType())
            markAllFields(qt->getAs<RecordType>()->getDecl());
    }
}

/**
 * Check for usage of sizeof(T) where T is a record.
 * Means we can't touch the size of the class by removing fields.
 *
 * @FIXME this could be tightened up. In some contexts e.g. "memset(p,sizeof(T),0)" we could emit a "set to zero"
 */
 /*
bool SingleValFields::VisitUnaryExprOrTypeTraitExpr( const UnaryExprOrTypeTraitExpr* expr )
{
    if (expr->getKind() != UETT_SizeOf || !expr->isArgumentType()) {
        return true;
    }
    QualType qt = expr->getArgumentType();
    if (!qt->isRecordType()) {
        return true;
    }
    const RecordDecl* recordDecl = qt->getAs<RecordType>()->getDecl();
    markAllFields(recordDecl);
    return true;
}
*/
bool SingleValFields::VisitMemberExpr( const MemberExpr* memberExpr )
{
    const ValueDecl* decl = memberExpr->getMemberDecl();
    const FieldDecl* fieldDecl = dyn_cast<FieldDecl>(decl);
    if (!fieldDecl) {
        return true;
    }

    if (ignoreLocation(memberExpr) || !isInterestingType(fieldDecl->getType()))
        return true;

    const FunctionDecl* parentFunction = parentFunctionDecl(memberExpr);
    const CXXMethodDecl* methodDecl = dyn_cast_or_null<CXXMethodDecl>(parentFunction);
    if (methodDecl && (methodDecl->isCopyAssignmentOperator() || methodDecl->isMoveAssignmentOperator()))
       return true;

    // walk up the tree until we find something interesting
    const Stmt* child = memberExpr;
    const Stmt* parent = parentStmt(memberExpr);
    bool bPotentiallyAssignedTo = false;
    bool bDump = false;
    std::string assignValue;

    // check for field being returned by non-const ref eg. Foo& getFoo() { return f; }
    if (parent && isa<ReturnStmt>(parent)) {
        const Stmt* parent2 = parentStmt(parent);
        if (parent2 && isa<CompoundStmt>(parent2)) {
            QualType qt = compat::getReturnType(*parentFunction).getDesugaredType(compiler.getASTContext());
            if (!qt.isConstQualified() && qt->isReferenceType()) {
                assignValue = "?";
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
                    assignValue = "?";
                    bPotentiallyAssignedTo = true;
                    break;
                }
            }
        }

        if (!parent) {
            return true;
        }
        if (isa<CastExpr>(parent) || isa<MemberExpr>(parent) || isa<ParenExpr>(parent) || isa<ParenListExpr>(parent)
             || isa<ExprWithCleanups>(parent))
        {
            child = parent;
            parent = parentStmt(parent);
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
            child = parent;
            parent = parentStmt(parent);
        }
        else if (isa<CXXOperatorCallExpr>(parent))
        {
            // FIXME need to handle this properly
            assignValue = "?";
            bPotentiallyAssignedTo = true;
            break;
        }
        else if (isa<CallExpr>(parent))
        {
            checkCallExpr(child, dyn_cast<CallExpr>(parent), assignValue, bPotentiallyAssignedTo);
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
                        assignValue = "?";
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
            if ( binaryOp->getLHS() != child ) {
                // do nothing
            }
            else if ( binaryOp->getOpcode() == BO_Assign ) {
                assignValue = getExprValue(binaryOp->getRHS());
                bPotentiallyAssignedTo = true;
            } else {
                assignValue = "?";
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
                || isa<CXXUnresolvedConstructExpr>(parent)
                )
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
              memberExpr->getLocStart())
              << memberExpr->getSourceRange();
        parent->dump();
    }
    if (bPotentiallyAssignedTo)
    {
        MyFieldAssignmentInfo aInfo;
        niceName(fieldDecl, aInfo);
        aInfo.value = assignValue;
        assignedSet.insert(aInfo);
    }

    return true;
}

bool SingleValFields::isInterestingType(const QualType& qt) {
   return qt.isCXX11PODType(compiler.getASTContext());
}

void SingleValFields::checkCallExpr(const Stmt* child, const CallExpr* callExpr, std::string& assignValue, bool& bPotentiallyAssignedTo)
{
    if (callExpr->getCallee() == child) {
        return;
    }
    const FunctionDecl* functionDecl;
    if (isa<CXXMemberCallExpr>(callExpr)) {
        functionDecl = dyn_cast<CXXMemberCallExpr>(callExpr)->getMethodDecl();
    } else {
        functionDecl = callExpr->getDirectCallee();
    }
    if (functionDecl) {
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
        if (i >= compat::getNumParams(*proto)) // can happen in template code
            break;
        if (callExpr->getArg(i) == child) {
            QualType qt = compat::getParamType(*proto, i).getDesugaredType(compiler.getASTContext());
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
    // workaround bug in clang
    if (isa<ParenListExpr>(arg))
        return "?";
    // ignore this, it seems to trigger an infinite recursion
    if (isa<UnaryExprOrTypeTraitExpr>(arg)) {
        return "?";
    }
    APSInt x1;
    if (arg->EvaluateAsInt(x1, compiler.getASTContext()))
    {
        return x1.toString(10);
    }
    return "?";
}

loplugin::Plugin::Registration< SingleValFields > X("singlevalfields", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
