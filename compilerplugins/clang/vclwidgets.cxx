/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <iostream>

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

    bool VisitVarDecl(const VarDecl *);

    bool VisitFieldDecl(const FieldDecl *);

    bool VisitParmVarDecl(const ParmVarDecl *);

    bool VisitFunctionDecl(const FunctionDecl *);

    bool VisitCXXDestructorDecl(const CXXDestructorDecl *);

    bool VisitCXXDeleteExpr(const CXXDeleteExpr *);

    bool VisitCallExpr(const CallExpr *);
    bool VisitDeclRefExpr(const DeclRefExpr* pDeclRefExpr);
    bool VisitCXXConstructExpr( const CXXConstructExpr* expr );
private:
    bool isDisposeCallingSuperclassDispose(const CXXMethodDecl* pMethodDecl);
    bool mbCheckingMemcpy = false;
};

static bool startsWith(const std::string& s, const char* other)
{
    return s.compare(0, strlen(other), other) == 0;
}

#define BASE_REF_COUNTED_CLASS "VclReferenceBase"

bool BaseCheckNotWindowSubclass(
    const CXXRecordDecl *BaseDefinition
#if CLANG_VERSION < 30800
    , void *
#endif
    )
{
    if (BaseDefinition && BaseDefinition->getQualifiedNameAsString() == BASE_REF_COUNTED_CLASS) {
        return false;
    }
    return true;
}

bool isDerivedFromWindow(const CXXRecordDecl *decl) {
    if (!decl)
        return false;
    if (decl->getQualifiedNameAsString() == BASE_REF_COUNTED_CLASS)
        return true;
    if (!decl->hasDefinition()) {
        return false;
    }
    if (// not sure what hasAnyDependentBases() does,
        // but it avoids classes we don't want, e.g. WeakAggComponentImplHelper1
        !decl->hasAnyDependentBases() &&
        !compat::forallBases(*decl, BaseCheckNotWindowSubclass, nullptr, true)) {
        return true;
    }
    return false;
}

bool containsWindowSubclass(const Type* pType0);

bool containsWindowSubclass(const QualType& qType) {
    auto t = qType->getAs<RecordType>();
    if (t != nullptr) {
        auto d = dyn_cast<ClassTemplateSpecializationDecl>(t->getDecl());
        if (d != nullptr) {
            std::string name(d->getQualifiedNameAsString());
            if (name == "ScopedVclPtr" || name == "ScopedVclPtrInstance"
                || name == "VclPtr" || name == "VclPtrInstance")
            {
                return false;
            }
        }
    }
    return containsWindowSubclass(qType.getTypePtr());
}

bool containsWindowSubclass(const Type* pType0) {
    if (!pType0)
        return false;
    const Type* pType = pType0->getUnqualifiedDesugaredType();
    if (!pType)
        return false;
    const CXXRecordDecl* pRecordDecl = pType->getAsCXXRecordDecl();
    if (pRecordDecl) {
        const ClassTemplateSpecializationDecl* pTemplate = dyn_cast<ClassTemplateSpecializationDecl>(pRecordDecl);
        if (pTemplate) {
            auto name = pTemplate->getQualifiedNameAsString();
            if (name == "VclStatusListener") {
                return false;
            }
            bool link = name == "Link";
            for(unsigned i=0; i<pTemplate->getTemplateArgs().size(); ++i) {
                const TemplateArgument& rArg = pTemplate->getTemplateArgs()[i];
                if (rArg.getKind() == TemplateArgument::ArgKind::Type &&
                    containsWindowSubclass(rArg.getAsType()))
                {
                    // OK for first template argument of tools/link.hxx Link
                    // to be a Window-derived pointer:
                    if (!link || i != 0) {
                        return true;
                    }
                }
            }
        }
    }
    if (pType->isPointerType()) {
        QualType pointeeType = pType->getPointeeType();
        return containsWindowSubclass(pointeeType);
    } else if (pType->isArrayType()) {
        const ArrayType* pArrayType = dyn_cast<ArrayType>(pType);
        QualType elementType = pArrayType->getElementType();
        return containsWindowSubclass(elementType);
    } else {
        return isDerivedFromWindow(pRecordDecl);
    }
}

bool VCLWidgets::VisitCXXDestructorDecl(const CXXDestructorDecl* pCXXDestructorDecl)
{
    if (ignoreLocation(pCXXDestructorDecl)) {
        return true;
    }
    if (!pCXXDestructorDecl->isThisDeclarationADefinition()) {
        return true;
    }
    const CXXRecordDecl * pRecordDecl = pCXXDestructorDecl->getParent();
    // ignore
    if (pRecordDecl->getQualifiedNameAsString() == BASE_REF_COUNTED_CLASS) {
        return true;
    }
    // check if this class is derived from Window
    if (!isDerivedFromWindow(pRecordDecl)) {
        return true;
    }
    bool bFoundVclPtrField = false;
    for(auto fieldDecl = pRecordDecl->field_begin();
        fieldDecl != pRecordDecl->field_end(); ++fieldDecl)
    {
        const RecordType *pFieldRecordType = fieldDecl->getType()->getAs<RecordType>();
        if (pFieldRecordType) {
            const CXXRecordDecl *pFieldRecordTypeDecl = dyn_cast<CXXRecordDecl>(pFieldRecordType->getDecl());
            if (startsWith(pFieldRecordTypeDecl->getQualifiedNameAsString(), "VclPtr")) {
               bFoundVclPtrField = true;
               break;
            }
       }
    }
    bool bFoundDispose = false;
    for(auto methodDecl = pRecordDecl->method_begin();
        methodDecl != pRecordDecl->method_end(); ++methodDecl)
    {
        if (methodDecl->isInstance() && methodDecl->param_size()==0 && methodDecl->getNameAsString() == "dispose") {
           bFoundDispose = true;
           break;
        }
    }
    const CompoundStmt *pCompoundStatement = dyn_cast<CompoundStmt>(pCXXDestructorDecl->getBody());
    // having an empty body and no dispose() method is fine
    if (!bFoundVclPtrField && !bFoundDispose && pCompoundStatement && pCompoundStatement->size() == 0) {
        return true;
    }
    if (bFoundVclPtrField && pCompoundStatement && pCompoundStatement->size() == 0) {
        report(
            DiagnosticsEngine::Warning,
            BASE_REF_COUNTED_CLASS " subclass with VclPtr field must call disposeOnce() from its destructor",
            pCXXDestructorDecl->getLocStart())
          << pCXXDestructorDecl->getSourceRange();
        return true;
    }
    // check that the destructor for a BASE_REF_COUNTED_CLASS subclass does nothing except call into the disposeOnce() method
    bool bOk = false;
    if (pCompoundStatement) {
        bool bFoundDisposeOnce = false;
        int nNumExtraStatements = 0;
        for (auto i = pCompoundStatement->body_begin();
             i != pCompoundStatement->body_end(); ++i)
        {
            const CXXMemberCallExpr *pCallExpr = dyn_cast<CXXMemberCallExpr>(
                *i);
            if (pCallExpr) {
                if( const FunctionDecl* func = pCallExpr->getDirectCallee()) {
                    if( func->getNumParams() == 0 && func->getIdentifier() != NULL
                        && ( func->getName() == "disposeOnce" )) {
                        bFoundDisposeOnce = true;
                    }
                }
            }
            // checking for ParenExpr is a hacky way to ignore assert statements in older versions of clang (i.e. <= 3.2)
            if (!pCallExpr && !dyn_cast<ParenExpr>(*i))
                nNumExtraStatements++;
        }
        bOk = bFoundDisposeOnce && nNumExtraStatements == 0;
    }
    if (!bOk) {
        SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(
                              pCXXDestructorDecl->getLocStart());
        StringRef filename = compiler.getSourceManager().getFilename(spellingLocation);
        if (   !(filename.startswith(SRCDIR "/vcl/source/window/window.cxx"))
            && !(filename.startswith(SRCDIR "/vcl/source/gdi/virdev.cxx")) )
        {
            report(
                DiagnosticsEngine::Warning,
                BASE_REF_COUNTED_CLASS " subclass should have nothing in its destructor but a call to disposeOnce()",
                pCXXDestructorDecl->getLocStart())
              << pCXXDestructorDecl->getSourceRange();
        }
    }
    return true;
}


bool VCLWidgets::VisitVarDecl(const VarDecl * pVarDecl) {
    if (ignoreLocation(pVarDecl)) {
        return true;
    }
    if (  isa<ParmVarDecl>(pVarDecl) || pVarDecl->isLocalVarDecl() ) {
        return true;
    }

    if (containsWindowSubclass(pVarDecl->getType())) {
        report(
            DiagnosticsEngine::Warning,
            BASE_REF_COUNTED_CLASS " subclass %0 should be wrapped in VclPtr",
            pVarDecl->getLocation())
            << pVarDecl->getType() << pVarDecl->getSourceRange();
        return true;
    }

    const RecordType *recordType = pVarDecl->getType()->getAs<RecordType>();
    if (recordType == nullptr) {
        return true;
    }
    const CXXRecordDecl *recordDecl = dyn_cast<CXXRecordDecl>(recordType->getDecl());
    if (recordDecl == nullptr) {
        return true;
    }
    // check if this field is derived from Window
    if (isDerivedFromWindow(recordDecl)) {
        report(
            DiagnosticsEngine::Warning,
            BASE_REF_COUNTED_CLASS " subclass allocated on stack, should be allocated via VclPtr or via *",
            pVarDecl->getLocation())
          << pVarDecl->getSourceRange();
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
    const CXXRecordDecl *pParentRecordDecl = isa<RecordDecl>(fieldDecl->getDeclContext()) ? dyn_cast<CXXRecordDecl>(fieldDecl->getParent()) : nullptr;
    if (containsWindowSubclass(fieldDecl->getType())) {
        // have to ignore this for now, nasty reverse dependency from tools->vcl
        if (!(pParentRecordDecl != nullptr &&
                (pParentRecordDecl->getQualifiedNameAsString() == "ErrorContextImpl" ||
                 pParentRecordDecl->getQualifiedNameAsString() == "ScHFEditPage"))) {
            report(
                DiagnosticsEngine::Warning,
                BASE_REF_COUNTED_CLASS " subclass %0 declared as a pointer member, should be wrapped in VclPtr",
                fieldDecl->getLocation())
                << fieldDecl->getType() << fieldDecl->getSourceRange();
            return true;
       }
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
    if (isDerivedFromWindow(recordDecl)) {
        report(
            DiagnosticsEngine::Warning,
            BASE_REF_COUNTED_CLASS " subclass allocated as a class member, should be allocated via VclPtr",
            fieldDecl->getLocation())
          << fieldDecl->getSourceRange();
    }

    // If this field is a VclPtr field, then the class MUST have a dispose method
    if (pParentRecordDecl && isDerivedFromWindow(pParentRecordDecl)
        && startsWith(recordDecl->getQualifiedNameAsString(), "VclPtr"))
    {
        bool bFoundDispose = false;
        for(auto methodDecl = pParentRecordDecl->method_begin();
            methodDecl != pParentRecordDecl->method_end(); ++methodDecl)
        {
            if (methodDecl->isInstance() && methodDecl->param_size()==0 && methodDecl->getNameAsString() == "dispose") {
               bFoundDispose = true;
               break;
            }
        }
        if (!bFoundDispose) {
            report(
                DiagnosticsEngine::Warning,
                BASE_REF_COUNTED_CLASS " subclass with a VclPtr field MUST override dispose() (and call its superclass dispose() as the last thing it does)",
                fieldDecl->getLocation())
              << fieldDecl->getSourceRange();
        }
        if (!pParentRecordDecl->hasUserDeclaredDestructor()) {
            report(
                DiagnosticsEngine::Warning,
                BASE_REF_COUNTED_CLASS " subclass with a VclPtr field MUST have a user-provided destructor (that calls disposeOnce())",
                fieldDecl->getLocation())
              << fieldDecl->getSourceRange();
        }
    }

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
    // we exclude this method in VclBuilder because it's so useful to have it like this
    if (pMethodDecl
        && pMethodDecl->getNameAsString() == "get"
        && (pMethodDecl->getParent()->getQualifiedNameAsString() == "VclBuilder"
            || pMethodDecl->getParent()->getQualifiedNameAsString() == "VclBuilderContainer"))
    {
        return true;
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
        && pMethodDecl->getParent()->getQualifiedNameAsString() == "VclPtr") {
        return true;
    }
    // ignore the BASE_REF_COUNTED_CLASS::dispose() method
    if (pMethodDecl
        && pMethodDecl->getParent()->getQualifiedNameAsString() == BASE_REF_COUNTED_CLASS) {
        return true;
    }
    if (functionDecl->hasBody() && pMethodDecl && isDerivedFromWindow(pMethodDecl->getParent())) {
        // check the last thing that the dispose() method does, is to call into the superclass dispose method
        if (pMethodDecl->getNameAsString() == "dispose") {
            if (!isDisposeCallingSuperclassDispose(pMethodDecl)) {
                report(
                    DiagnosticsEngine::Warning,
                    BASE_REF_COUNTED_CLASS " subclass dispose() function MUST call dispose() of its superclass as the last thing it does",
                    functionDecl->getLocStart())
                  << functionDecl->getSourceRange();
           }
        }
    }

    return true;
}

bool VCLWidgets::VisitCXXDeleteExpr(const CXXDeleteExpr *pCXXDeleteExpr)
{
    if (ignoreLocation(pCXXDeleteExpr)) {
        return true;
    }
    const CXXRecordDecl *pPointee = pCXXDeleteExpr->getArgument()->getType()->getPointeeCXXRecordDecl();
    if (pPointee && isDerivedFromWindow(pPointee)) {
        SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(
                              pCXXDeleteExpr->getLocStart());
        StringRef filename = compiler.getSourceManager().getFilename(spellingLocation);
        if ( !(filename.startswith(SRCDIR "/include/vcl/vclreferencebase.hxx")))
        {
            report(
                DiagnosticsEngine::Warning,
                "calling delete on instance of " BASE_REF_COUNTED_CLASS " subclass, must rather call disposeAndClear()",
                pCXXDeleteExpr->getLocStart())
              << pCXXDeleteExpr->getSourceRange();
        }
    }
    const ImplicitCastExpr* pImplicitCastExpr = dyn_cast<ImplicitCastExpr>(pCXXDeleteExpr->getArgument());
    if (!pImplicitCastExpr) {
        return true;
    }
    if (pImplicitCastExpr->getCastKind() != CK_UserDefinedConversion) {
        return true;
    }
    report(
        DiagnosticsEngine::Warning,
        "calling delete on instance of VclPtr, must rather call disposeAndClear()",
        pCXXDeleteExpr->getLocStart())
     << pCXXDeleteExpr->getSourceRange();
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
    if (pCompoundStatement->size() == 0) return false;
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

bool containsVclPtr(const Type* pType0);

bool containsVclPtr(const QualType& qType) {
    auto t = qType->getAs<RecordType>();
    if (t != nullptr) {
        auto d = dyn_cast<ClassTemplateSpecializationDecl>(t->getDecl());
        if (d != nullptr) {
            std::string name(d->getQualifiedNameAsString());
            if (name == "ScopedVclPtr" || name == "ScopedVclPtrInstance"
                || name == "VclPtr" || name == "VclPtrInstance")
            {
                return true;
            }
        }
    }
    return containsVclPtr(qType.getTypePtr());
}

bool containsVclPtr(const Type* pType0) {
    if (!pType0)
        return false;
    const Type* pType = pType0->getUnqualifiedDesugaredType();
    if (!pType)
        return false;
    if (pType->isPointerType()) {
        return false;
    } else if (pType->isArrayType()) {
        const ArrayType* pArrayType = dyn_cast<ArrayType>(pType);
        QualType elementType = pArrayType->getElementType();
        return containsVclPtr(elementType);
    } else {
        const CXXRecordDecl* pRecordDecl = pType->getAsCXXRecordDecl();
        if (pRecordDecl)
        {
            std::string name(pRecordDecl->getQualifiedNameAsString());
            if (name == "ScopedVclPtr" || name == "ScopedVclPtrInstance"
                || name == "VclPtr" || name == "VclPtrInstance")
            {
                return true;
            }
            for(auto fieldDecl = pRecordDecl->field_begin();
                fieldDecl != pRecordDecl->field_end(); ++fieldDecl)
            {
                const RecordType *pFieldRecordType = fieldDecl->getType()->getAs<RecordType>();
                if (pFieldRecordType && containsVclPtr(pFieldRecordType)) {
                    return true;
                }
            }
            for(auto baseSpecifier = pRecordDecl->bases_begin();
                baseSpecifier != pRecordDecl->bases_end(); ++baseSpecifier)
            {
                const RecordType *pFieldRecordType = baseSpecifier->getType()->getAs<RecordType>();
                if (pFieldRecordType && containsVclPtr(pFieldRecordType)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool VCLWidgets::VisitCallExpr(const CallExpr* pCallExpr)
{
    if (ignoreLocation(pCallExpr)) {
        return true;
    }
    FunctionDecl const * fdecl = pCallExpr->getDirectCallee();
    if (fdecl == nullptr) {
        return true;
    }
    std::string qname { fdecl->getQualifiedNameAsString() };
    if (qname.find("memcpy") == std::string::npos
         && qname.find("bcopy") == std::string::npos
         && qname.find("memmove") == std::string::npos
         && qname.find("rtl_copy") == std::string::npos) {
        return true;
    }
    mbCheckingMemcpy = true;
    Stmt * pStmt = const_cast<Stmt*>(static_cast<const Stmt*>(pCallExpr->getArg(0)));
    TraverseStmt(pStmt);
    mbCheckingMemcpy = false;
    return true;
}

bool VCLWidgets::VisitDeclRefExpr(const DeclRefExpr* pDeclRefExpr)
{
    if (!mbCheckingMemcpy) {
        return true;
    }
    if (ignoreLocation(pDeclRefExpr)) {
        return true;
    }
    QualType pType = pDeclRefExpr->getDecl()->getType();
    if (pType->isPointerType()) {
        pType = pType->getPointeeType();
    }
    if (!containsVclPtr(pType)) {
        return true;
    }
    report(
        DiagnosticsEngine::Warning,
        "Calling memcpy on a type which contains a VclPtr",
        pDeclRefExpr->getExprLoc());
    return true;
}

bool VCLWidgets::VisitCXXConstructExpr( const CXXConstructExpr* constructExpr )
{
    if (ignoreLocation(constructExpr)) {
        return true;
    }
    if (constructExpr->getConstructionKind() != CXXConstructExpr::CK_Complete) {
        return true;
    }
    const CXXConstructorDecl* pConstructorDecl = constructExpr->getConstructor();
    const CXXRecordDecl* recordDecl = pConstructorDecl->getParent();
    if (isDerivedFromWindow(recordDecl)) {
        report(
            DiagnosticsEngine::Warning,
            "Calling constructor of a Window-derived type directly; all such creation should go via VclPtr<>::Create",
            constructExpr->getExprLoc());
    }
    return true;
}

loplugin::Plugin::Registration< VCLWidgets > X("vclwidgets");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
