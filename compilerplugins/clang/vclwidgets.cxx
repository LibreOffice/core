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

private:
    bool isDisposeCallingSuperclassDispose(const CXXMethodDecl* pMethodDecl);
};

static bool startsWith(const std::string& s, const char* other)
{
    return s.compare(0, strlen(other), other) == 0;
}

bool BaseCheckNotWindowSubclass(const CXXRecordDecl *BaseDefinition, void *) {
    if (BaseDefinition && BaseDefinition->getQualifiedNameAsString().compare("vcl::Window") == 0) {
        return false;
    }
    return true;
}

bool isDerivedFromWindow(const CXXRecordDecl *decl) {
    if (!decl)
        return false;
    if (decl->getQualifiedNameAsString() == "vcl::Window")
        return true;
    if (!decl->hasDefinition()) {
        return false;
    }
    if (// not sure what hasAnyDependentBases() does,
        // but it avoids classes we don't want, e.g. WeakAggComponentImplHelper1
        !decl->hasAnyDependentBases() &&
        !decl->forallBases(BaseCheckNotWindowSubclass, nullptr, true)) {
        return true;
    }
    return false;
}

bool containsWindowSubclass(const Type* pType0);

bool containsWindowSubclass(const QualType& qType) {
    if (startsWith(qType.getAsString(), "VclPtr"))
        return false;
    if (startsWith(qType.getAsString(), "class VclPtr"))
        return false;
    if (startsWith(qType.getAsString(), "const class VclPtr"))
        return false;
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
            for(unsigned i=0; i<pTemplate->getTemplateArgs().size(); ++i) {
                const TemplateArgument& rArg = pTemplate->getTemplateArgs()[i];
                if (rArg.getKind() == TemplateArgument::ArgKind::Type &&
                    containsWindowSubclass(rArg.getAsType()))
                {
                    return true;
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
    // ignore vcl::Window class
    if (pRecordDecl->getQualifiedNameAsString().compare("vcl::Window") == 0) {
        return true;
    }
    // check if this class is derived from Window
    if (!isDerivedFromWindow(pRecordDecl)) {
        return true;
    }
    bool foundVclPtrField = false;
    for(auto fieldDecl : pRecordDecl->fields()) {
        const RecordType *pFieldRecordType = fieldDecl->getType()->getAs<RecordType>();
        if (pFieldRecordType) {
            const CXXRecordDecl *pFieldRecordTypeDecl = dyn_cast<CXXRecordDecl>(pFieldRecordType->getDecl());
            if (startsWith(pFieldRecordTypeDecl->getQualifiedNameAsString(), "VclPtr")) {
               foundVclPtrField = true;
               break;
            }
       }
    }
    bool foundDispose = false;
    for(auto methodDecl : pRecordDecl->methods()) {
        if (methodDecl->isInstance() && methodDecl->param_size()==0 && methodDecl->getNameAsString() == "dispose") {
           foundDispose = true;
           break;
        }
    }
    const CompoundStmt *pCompoundStatement = dyn_cast<CompoundStmt>(pCXXDestructorDecl->getBody());
    // having an empty body and no dispose() method is fine
    if (!foundVclPtrField && !foundDispose && pCompoundStatement && pCompoundStatement->size() == 0) {
        return true;
    }
    if (foundVclPtrField && pCompoundStatement && pCompoundStatement->size() == 0) {
        report(
            DiagnosticsEngine::Warning,
            "vcl::Window subclass with VclPtr field must call dispose() from it's destructor.",
            pCXXDestructorDecl->getLocStart())
          << pCXXDestructorDecl->getSourceRange();
        return true;
    }
    // check that the destructor for a vcl::Window subclass does nothing except call into the dispose() method
    bool ok = false;
    if (pCompoundStatement && pCompoundStatement->size() == 1) {
        const CXXMemberCallExpr *pCallExpr = dyn_cast<CXXMemberCallExpr>(*pCompoundStatement->body_begin());
        if (pCallExpr) {
            if( const FunctionDecl* func = pCallExpr->getDirectCallee()) {
                if( func->getNumParams() == 0 && func->getIdentifier() != NULL
                    && ( func->getName() == "disposeOnce" )) {
                    ok = true;
                }
            }
        }
    }
    if (!ok) {
        report(
            DiagnosticsEngine::Warning,
            "vcl::Window subclass should have nothing in it's destructor but a call to disposeOnce().",
            pCXXDestructorDecl->getLocStart())
          << pCXXDestructorDecl->getSourceRange();
        return true;
    }
    return true;
}


bool VCLWidgets::VisitVarDecl(const VarDecl * pVarDecl) {
    if (ignoreLocation(pVarDecl)) {
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
            "vcl::Window subclass allocated on stack, should be allocated via VclPtr or via *.",
            pVarDecl->getLocation())
          << pVarDecl->getSourceRange();
    }
    if (   !startsWith(pVarDecl->getType().getAsString(), "std::vector<vcl::Window *>")
        && !startsWith(pVarDecl->getType().getAsString(), "std::map<vcl::Window *, Size>")
        && !startsWith(pVarDecl->getType().getAsString(), "std::map<vcl::Window *, class Size>")
        && !startsWith(pVarDecl->getType().getAsString(), "::std::vector<class Button *>")
        && !startsWith(pVarDecl->getType().getAsString(), "::std::vector<Button *>")
        && !startsWith(pVarDecl->getType().getAsString(), "::std::mem_fun1_t<")
        && !startsWith(pVarDecl->getType().getAsString(), "::comphelper::mem_fun1_t<")
        && !startsWith(pVarDecl->getType().getAsString(), "::std::pair<formula::RefButton *, formula::RefEdit *>")
        && !startsWith(pVarDecl->getType().getAsString(), "::std::pair<RefButton *, RefEdit *>")
        && !startsWith(pVarDecl->getType().getAsString(), "std::list<SwSidebarWin *>")
        && !startsWith(pVarDecl->getType().getAsString(), "::std::map<OTableWindow *, sal_Int32>")
        && !startsWith(pVarDecl->getType().getAsString(), "::std::map<class OTableWindow *, sal_Int32>")
        && !startsWith(pVarDecl->getType().getAsString(), "::std::multimap<sal_Int32, OTableWindow *>")
        && !startsWith(pVarDecl->getType().getAsString(), "::std::multimap<sal_Int32, class OTableWindow *>")
        && !startsWith(pVarDecl->getType().getAsString(), "::dbp::OMultiInstanceAutoRegistration< ::dbp::OUnoAutoPilot<")
        && !startsWith(pVarDecl->getType().getAsString(), "SwSidebarWin_iterator")
        && containsWindowSubclass(pVarDecl->getType()))
    {
        report(
            DiagnosticsEngine::Warning,
            "vcl::Window subclass should be wrapped in VclPtr. " + pVarDecl->getType().getAsString(),
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
    if (containsWindowSubclass(fieldDecl->getType())) {
        report(
            DiagnosticsEngine::Warning,
            "vcl::Window subclass declared as a pointer field, should be wrapped in VclPtr." + fieldDecl->getType().getAsString(),
            fieldDecl->getLocation())
          << fieldDecl->getSourceRange();
        return true;
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
            "vcl::Window subclass allocated as a class member, should be allocated via VclPtr.",
            fieldDecl->getLocation())
          << fieldDecl->getSourceRange();
    }

    // If this field is a VclPtr field, then the class MUST have a dispose method
    const CXXRecordDecl *pParentRecordDecl = dyn_cast<CXXRecordDecl>(fieldDecl->getParent());
    if (pParentRecordDecl && isDerivedFromWindow(pParentRecordDecl)
        && startsWith(recordDecl->getQualifiedNameAsString(), "VclPtr"))
    {
        bool foundDispose = false;
        for(auto methodDecl : pParentRecordDecl->methods()) {
            if (methodDecl->isInstance() && methodDecl->param_size()==0 && methodDecl->getNameAsString() == "dispose") {
               foundDispose = true;
               break;
            }
        }
        if (!foundDispose) {
            report(
                DiagnosticsEngine::Warning,
                "vcl::Window subclass with a VclPtr field MUST have a dispose() method.",
                fieldDecl->getLocation())
              << fieldDecl->getSourceRange();
        }
        if (!pParentRecordDecl->hasUserDeclaredDestructor()) {
            report(
                DiagnosticsEngine::Warning,
                "vcl::Window subclass with a VclPtr field MUST have an explicit destructor.",
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

static void findDisposeAndClearStatements2(std::vector<std::string>& aVclPtrFields, const Stmt *pStmt);

static void findDisposeAndClearStatements(std::vector<std::string>& aVclPtrFields, const CompoundStmt *pCompoundStatement)
{
    for(const Stmt* pStmt : pCompoundStatement->body()) {
        findDisposeAndClearStatements2(aVclPtrFields, pStmt);
    }
}

static void findDisposeAndClearStatements2(std::vector<std::string>& aVclPtrFields, const Stmt *pStmt)
{
    if (isa<CompoundStmt>(pStmt)) {
        findDisposeAndClearStatements(aVclPtrFields, dyn_cast<CompoundStmt>(pStmt));
        return;
    }
    if (isa<ForStmt>(pStmt)) {
        const CompoundStmt *pBody = dyn_cast<CompoundStmt>(dyn_cast<ForStmt>(pStmt)->getBody());
        if (pBody)
            findDisposeAndClearStatements(aVclPtrFields, pBody);
        return;
    }
    if (!isa<CallExpr>(pStmt)) return;
    const CallExpr *pCallExpr = dyn_cast<CallExpr>(pStmt);

    if (!pCallExpr->getDirectCallee()) return;
    if (!isa<CXXMethodDecl>(pCallExpr->getDirectCallee())) return;
    const CXXMethodDecl *pCalleeMethodDecl = dyn_cast<CXXMethodDecl>(pCallExpr->getDirectCallee());
    if (pCalleeMethodDecl->getNameAsString() != "disposeAndClear") return;

    if (!pCallExpr->getCallee()) return;

    if (!isa<MemberExpr>(pCallExpr->getCallee())) return;
    const MemberExpr *pCalleeMemberExpr = dyn_cast<MemberExpr>(pCallExpr->getCallee());

    if (!pCalleeMemberExpr->getBase()) return;
    if (!isa<MemberExpr>(pCalleeMemberExpr->getBase())) return;
    const MemberExpr *pCalleeMemberExprBase = dyn_cast<MemberExpr>(pCalleeMemberExpr->getBase());

    std::string xxx = pCalleeMemberExprBase->getMemberDecl()->getNameAsString();
    aVclPtrFields.erase(std::remove(aVclPtrFields.begin(), aVclPtrFields.end(), xxx), aVclPtrFields.end());
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
    // ignore the vcl::Window::dispose() method
    if (pMethodDecl
        && pMethodDecl->getParent()->getQualifiedNameAsString() == "vcl::Window") {
        return true;
    }
    if (functionDecl->hasBody() && pMethodDecl && isDerivedFromWindow(pMethodDecl->getParent())) {
        // check the last thing that the dispose() method does, is to call into the superclass dispose method
        if (pMethodDecl->getNameAsString() == "dispose") {
            if (!isDisposeCallingSuperclassDispose(pMethodDecl)) {
                report(
                    DiagnosticsEngine::Warning,
                    "vcl::Window subclass dispose() method MUST call it's superclass dispose() as the last thing it does",
                    functionDecl->getLocStart())
                  << functionDecl->getSourceRange();
           }
        }
    }
    // check dispose method to make sure we are actually disposing all of the VclPtr fields
    /*
       Now that we are in the debugging phase this is no longer useful, since we have to break this rule on
       occassion to make the destruction process work cleanly.
    if (pMethodDecl && pMethodDecl->isInstance() && pMethodDecl->getBody()
        && pMethodDecl->param_size()==0
        && pMethodDecl->getNameAsString() == "dispose"
        && isDerivedFromWindow(pMethodDecl->getParent()) )
    {
        // exclude a couple of methods with hard-to-parse code
        if (pMethodDecl->getQualifiedNameAsString() == "SvxRubyDialog::dispose")
            return true;
        if (pMethodDecl->getQualifiedNameAsString() == "SvxPersonalizationTabPage::dispose")
            return true;
        if (pMethodDecl->getQualifiedNameAsString() == "SelectPersonaDialog::dispose")
            return true;
        if (pMethodDecl->getQualifiedNameAsString() == "MappingDialog_Impl::dispose")
            return true;
        if (pMethodDecl->getQualifiedNameAsString() == "BibGeneralPage::dispose")
            return true;
        if (pMethodDecl->getQualifiedNameAsString() == "SwCreateAuthEntryDlg_Impl::dispose")
            return true;
        if (pMethodDecl->getQualifiedNameAsString() == "SwTableColumnPage::dispose")
            return true;
        if (pMethodDecl->getQualifiedNameAsString() == "SwAssignFieldsControl::dispose")
            return true;
        if (pMethodDecl->getQualifiedNameAsString() == "ScOptSolverDlg::dispose")
            return true;
        if (pMethodDecl->getQualifiedNameAsString() == "ScPivotFilterDlg::dispose")
            return true;
        if (pMethodDecl->getQualifiedNameAsString() == "SmToolBoxWindow::dispose")
            return true;
        if (pMethodDecl->getQualifiedNameAsString() == "dbaui::DlgOrderCrit::dispose")
            return true;
        if (pMethodDecl->getQualifiedNameAsString() == "SvxStyleBox_Impl::dispose")
            return true;
        if (pMethodDecl->getQualifiedNameAsString() == "dbaui::OAppDetailPageHelper::dispose")
            return true;
        if (pMethodDecl->getQualifiedNameAsString() == "sd::CustomAnimationCreateDialog::dispose")
            return true;

        std::vector<std::string> aVclPtrFields;
        for(auto fieldDecl : pMethodDecl->getParent()->fields()) {
            if (startsWith(fieldDecl->getType().getAsString(), "VclPtr")) {
                aVclPtrFields.push_back(fieldDecl->getNameAsString());
            }
        }
        if (!aVclPtrFields.empty()) {
            if (pMethodDecl->getBody() && isa<CompoundStmt>(pMethodDecl->getBody()))
                findDisposeAndClearStatements( aVclPtrFields, dyn_cast<CompoundStmt>(pMethodDecl->getBody()) );
            if (!aVclPtrFields.empty()) {
                //pMethodDecl->dump();
                std::string aMessage = "vcl::Window subclass dispose() method does not call disposeAndClear() on the following field(s) ";
                for(auto s : aVclPtrFields)
                    aMessage += "\n    " + s + ".clear();";
                report(
                    DiagnosticsEngine::Warning,
                    aMessage,
                    functionDecl->getLocStart())
                  << functionDecl->getSourceRange();
           }
       }
    }
    */
    return true;
}

bool VCLWidgets::VisitCXXDeleteExpr(const CXXDeleteExpr *pCXXDeleteExpr)
{
    if (ignoreLocation(pCXXDeleteExpr)) {
        return true;
    }
    const CXXRecordDecl *pPointee = pCXXDeleteExpr->getArgument()->getType()->getPointeeCXXRecordDecl();
    if (pPointee && isDerivedFromWindow(pPointee)) {
        report(
            DiagnosticsEngine::Warning,
            "calling delete on instance of vcl::Window subclass, must rather call disposeAndClear()",
            pCXXDeleteExpr->getLocStart())
          << pCXXDeleteExpr->getSourceRange();
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



loplugin::Plugin::Registration< VCLWidgets > X("vclwidgets");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
