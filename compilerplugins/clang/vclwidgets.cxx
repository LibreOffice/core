/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>

#include "plugin.hxx"
#include "compat.hxx"
#include "clang/AST/CXXInheritance.h"

// Final goal: Checker for VCL widget references. Makes sure that VCL Window subclasses are properly referenced counted and dispose()'ed.
//
// But at the moment it just finds subclasses of Window which are not heap-allocated
//
// TODO do I need to check for local and static variables, too ?
// TODO when we have a dispose() method, verify that the dispose() methods releases all of the Window references
// TODO when we have a dispose() method, verify that it calls the super-class dispose() method at some point.

namespace {

class VCLWidgets:
    public RecursiveASTVisitor<VCLWidgets>, public loplugin::Plugin
{
public:
    explicit VCLWidgets(InstantiationData const & data): Plugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCXXRecordDecl(const CXXRecordDecl * decl);

    bool VisitFieldDecl(const FieldDecl * decl);

    bool VisitParmVarDecl(ParmVarDecl const * decl);

    bool VisitFunctionDecl( const FunctionDecl* var );

private:
    bool isDisposeCallingSuperclassDispose(const CXXMethodDecl* pMethodDecl);
};

bool BaseCheckNotWindowSubclass(const CXXRecordDecl *BaseDefinition, void *) {
    if (BaseDefinition->getQualifiedNameAsString().compare("vcl::Window") == 0) {
        return false;
    }
    return true;
}

bool isDerivedFromWindow(const CXXRecordDecl *decl) {
    if (!decl->hasDefinition())
        return false;
    if (decl->getQualifiedNameAsString().compare("vcl::Window") == 0)
        return true;
    if (// not sure what hasAnyDependentBases() does,
        // but it avoids classes we don't want, e.g. WeakAggComponentImplHelper1
        !decl->hasAnyDependentBases() &&
        !decl->forallBases(BaseCheckNotWindowSubclass, nullptr, true)) {
        return true;
    }
    return false;
}

bool isPointerToWindowSubclass(const QualType& pType) {
    if (!pType->isPointerType())
        return false;
    QualType pointeeType = pType->getPointeeType();
    const RecordType *recordType = pointeeType->getAs<RecordType>();
    if (recordType == nullptr) {
        return false;
    }
    const CXXRecordDecl *recordDecl = dyn_cast<CXXRecordDecl>(recordType->getDecl());
    if (recordDecl == nullptr) {
        return false;
    }
    return isDerivedFromWindow(recordDecl);
}

bool VCLWidgets::VisitCXXRecordDecl(const CXXRecordDecl * recordDecl) {
    if (ignoreLocation(recordDecl)) {
        return true;
    }
    if (!recordDecl->isCompleteDefinition())
        return true;
    // check if this class is derived from Window
    if (!isDerivedFromWindow(recordDecl)) {
        return true;
    }
    if (!recordDecl->hasUserDeclaredDestructor()) {
        return true;
    }
    bool foundDispose = false;
    for(auto methodDecl : recordDecl->methods()) {
        if (methodDecl->isInstance() && methodDecl->param_size()==0 && methodDecl->getNameAsString() == "dispose") {
           foundDispose = true;
           break;
        }
    }
    if (!foundDispose) {
        report(
            DiagnosticsEngine::Warning,
            "vcl::Window subclass with destructor should declare a dispose() method.",
            recordDecl->getLocation())
          << recordDecl->getSourceRange();
    }
    return true;
}


bool VCLWidgets::VisitFieldDecl(const FieldDecl * fieldDecl) {
    if (ignoreLocation(fieldDecl)) {
        return true;
    }
    if (fieldDecl->isBitField()) {
        return true;
    }
    if (isPointerToWindowSubclass(fieldDecl->getType())) {
/*        report(
            DiagnosticsEngine::Remark,
            "vcl::Window subclass declared as a pointer field, should be wrapped in VclPtr.",
            fieldDecl->getLocation())
          << fieldDecl->getSourceRange();
        return true;*/
    }

    const RecordType *recordType = fieldDecl->getType()->getAs<RecordType>();
    if (recordType == nullptr) {
        return true;
    }
    const CXXRecordDecl *recordDecl = dyn_cast<CXXRecordDecl>(recordType->getDecl());
    if (recordDecl == nullptr) {
        return true;
    }
    // check if this field is derived from Window
    if (!isDerivedFromWindow(recordDecl)) {
        return true;
    }

    report(
        DiagnosticsEngine::Warning,
        "vcl::Window subclass allocated as a class member, should be allocated via VclPtr.",
        fieldDecl->getLocation())
      << fieldDecl->getSourceRange();
    return true;
}

bool VCLWidgets::VisitParmVarDecl(ParmVarDecl const * pvDecl)
{
    if (ignoreLocation(pvDecl)) {
        return true;
    }
    // ignore the stuff in the VclPtr template class
    const CXXMethodDecl *pMethodDecl = dyn_cast<CXXMethodDecl>(pvDecl->getDeclContext());
    if (pMethodDecl
        && pMethodDecl->getParent()->getQualifiedNameAsString().find("VclPtr") != std::string::npos) {
        return true;
    }
    if (pvDecl->getType().getAsString().find("VclPtr") != std::string::npos) {
        report(
            DiagnosticsEngine::Warning,
            "vcl::Window subclass passed as a VclPtr parameter, should be passed as a raw pointer.",
            pvDecl->getLocation())
          << pvDecl->getSourceRange();
    }
    return true;
}

bool VCLWidgets::VisitFunctionDecl( const FunctionDecl* functionDecl )
{
    if (ignoreLocation(functionDecl)) {
        return true;
    }
    // ignore the stuff in the VclPtr template class
    const CXXMethodDecl *pMethodDecl = dyn_cast<CXXMethodDecl>(functionDecl);
    if (pMethodDecl
        && pMethodDecl->getParent()->getQualifiedNameAsString().find("VclPtr") != std::string::npos) {
        return true;
    }
    if (pMethodDecl
        && pMethodDecl->getParent()->getQualifiedNameAsString().compare("vcl::Window") == 0) {
        return true;
    }
    QualType t1 { compat::getReturnType(*functionDecl) };
    if (t1.getAsString().find("VclPtr") != std::string::npos) {
        report(
            DiagnosticsEngine::Warning,
            "VclPtr declared as a return type from a method/function, should be passed as a raw pointer.",
            functionDecl->getLocation())
          << functionDecl->getSourceRange();
    }
    if (functionDecl->hasBody() && pMethodDecl && isDerivedFromWindow(pMethodDecl->getParent())) {
        const CXXDestructorDecl *pDestructorDecl = dyn_cast<CXXDestructorDecl>(pMethodDecl);
        // check that the destructor for a vcl::Window subclass does nothing except call into the dispose() method
        if (pDestructorDecl) {
            const CompoundStmt *pCompoundStatement = dyn_cast<CompoundStmt>(functionDecl->getBody());
            bool ok = false;
            if (pCompoundStatement && pCompoundStatement->size() == 1) {
                const CXXMemberCallExpr *pCallExpr = dyn_cast<CXXMemberCallExpr>(*pCompoundStatement->body_begin());
                if (pCallExpr) {
                    ok = true;
                }
            }
            if (!ok) {
                report(
                    DiagnosticsEngine::Warning,
                    "vcl::Window subclass should have nothing in it's destructor but a call to dispose().",
                    functionDecl->getBody()->getLocStart())
                  << functionDecl->getBody()->getSourceRange();
            }
        // check the last thing that the dispose() method does, is to call into the superclass dispose method
        } else if (pMethodDecl->getNameAsString() == "dispose") {
            if (!isDisposeCallingSuperclassDispose(pMethodDecl)) {
                report(
                    DiagnosticsEngine::Warning,
                    "vcl::Window subclass dispose() method MUST call it's superclass dispose() as the last thing it does.",
                    functionDecl->getBody()->getLocStart())
                  << functionDecl->getBody()->getSourceRange();
           }
        }
    }
    return true;
}

/**
The AST looks like:
`-CXXMemberCallExpr 0xb06d8b0 'void'
  `-MemberExpr 0xb06d868 '<bound member function type>' ->dispose 0x9d34880
    `-ImplicitCastExpr 0xb06d8d8 'class SfxTabPage *' <UncheckedDerivedToBase (SfxTabPage)>
      `-CXXThisExpr 0xb06d850 'class SfxAcceleratorConfigPage *' this

*/
bool VCLWidgets::isDisposeCallingSuperclassDispose(const CXXMethodDecl* pMethodDecl)
{
    const CompoundStmt *pCompoundStatement = dyn_cast<CompoundStmt>(pMethodDecl->getBody());
    if (!pCompoundStatement) return false;
    // find the last statement
    const CXXMemberCallExpr *pCallExpr = dyn_cast<CXXMemberCallExpr>(*pCompoundStatement->body_rbegin());
    if (!pCallExpr) return false;
    const MemberExpr *pMemberExpr = dyn_cast<MemberExpr>(pCallExpr->getCallee());
    if (!pMemberExpr) return false;
    if (pMemberExpr->getMemberDecl()->getNameAsString() != "dispose") return false;
    const CXXMethodDecl *pDirectCallee = dyn_cast<CXXMethodDecl>(pCallExpr->getDirectCallee());
    if (!pDirectCallee) return false;
/* Not working yet. Partially because sometimes the superclass does not a dispose() method, so it gets passed up the chain.
   Need complex checking for that case.
    if (pDirectCallee->getParent()->getTypeForDecl() != (*pMethodDecl->getParent()->bases_begin()).getType().getTypePtr()) {
        report(
            DiagnosticsEngine::Warning,
            "dispose() method calling wrong baseclass, calling " + pDirectCallee->getParent()->getQualifiedNameAsString() +
            " should be calling " + (*pMethodDecl->getParent()->bases_begin()).getType().getAsString(),
            pCallExpr->getLocStart())
          << pCallExpr->getSourceRange();
        return false;
    }*/
    return true;
}



loplugin::Plugin::Registration< VCLWidgets > X("vclwidgets");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
