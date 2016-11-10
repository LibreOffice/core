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
#include "check.hxx"
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

    bool shouldVisitTemplateInstantiations () const { return true; }

    bool VisitVarDecl(const VarDecl *);
    bool VisitFieldDecl(const FieldDecl *);
    bool VisitParmVarDecl(const ParmVarDecl *);
    bool VisitFunctionDecl(const FunctionDecl *);
    bool VisitCXXDestructorDecl(const CXXDestructorDecl *);
    bool VisitCXXDeleteExpr(const CXXDeleteExpr *);
    bool VisitCallExpr(const CallExpr *);
    bool VisitDeclRefExpr(const DeclRefExpr *);
    bool VisitCXXConstructExpr(const CXXConstructExpr *);
    bool VisitBinaryOperator(const BinaryOperator *);
private:
    void checkAssignmentForVclPtrToRawConversion(const Type* lhsType, const Expr* rhs);
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

bool isDerivedFromVclReferenceBase(const CXXRecordDecl *decl) {
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

bool containsVclReferenceBaseSubclass(const Type* pType0);

bool containsVclReferenceBaseSubclass(const QualType& qType) {
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
    return containsVclReferenceBaseSubclass(qType.getTypePtr());
}

bool containsVclReferenceBaseSubclass(const Type* pType0) {
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
                    containsVclReferenceBaseSubclass(rArg.getAsType()))
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
        return containsVclReferenceBaseSubclass(pointeeType);
    } else if (pType->isArrayType()) {
        const ArrayType* pArrayType = dyn_cast<ArrayType>(pType);
        QualType elementType = pArrayType->getElementType();
        return containsVclReferenceBaseSubclass(elementType);
    } else {
        return isDerivedFromVclReferenceBase(pRecordDecl);
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
    // check if this class is derived from VclReferenceBase
    if (!isDerivedFromVclReferenceBase(pRecordDecl)) {
        return true;
    }
    // check if we have any VclPtr<> fields
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
    // check if there is a dispose() method
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
            && !(filename.startswith(SRCDIR "/vcl/source/gdi/virdev.cxx"))
            && !(filename.startswith(SRCDIR "/vcl/qa/cppunit/lifecycle.cxx")) )
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

bool VCLWidgets::VisitBinaryOperator(const BinaryOperator * binaryOperator)
{
    if (ignoreLocation(binaryOperator)) {
        return true;
    }
    if ( !binaryOperator->isAssignmentOp() ) {
        return true;
    }
    checkAssignmentForVclPtrToRawConversion(binaryOperator->getLHS()->getType().getTypePtr(), binaryOperator->getRHS());
    return true;
}

// Look for places where we are accidentally assigning a returned-by-value VclPtr<T> to a T*, which generally
// ends up in a use-after-free.
void VCLWidgets::checkAssignmentForVclPtrToRawConversion(const Type* lhsType, const Expr* rhs)
{
    if (!lhsType || !isa<PointerType>(lhsType)) {
        return;
    }
    if (!rhs) {
        return;
    }
    // lots of null checking for something weird going in SW that tends to crash clang with:
    // const clang::ExtQualsTypeCommonBase *clang::QualType::getCommonPtr() const: Assertion `!isNull() && "Cannot retrieve a NULL type pointer"'
    if (rhs->getType().getTypePtrOrNull()) {
        if (const PointerType* pt = dyn_cast<PointerType>(rhs->getType())) {
             const Type* pointeeType = pt->getPointeeType().getTypePtrOrNull();
             if (pointeeType && !isa<SubstTemplateTypeParmType>(pointeeType)) {
                return;
             }
        }
    }
    const CXXRecordDecl* pointeeClass = lhsType->getPointeeType()->getAsCXXRecordDecl();
    if (!isDerivedFromVclReferenceBase(pointeeClass)) {
        return;
    }
    const ExprWithCleanups* exprWithCleanups = dyn_cast<ExprWithCleanups>(rhs);
    if (!exprWithCleanups) {
        return;
    }
    const ImplicitCastExpr* implicitCast = dyn_cast<ImplicitCastExpr>(exprWithCleanups->getSubExpr());
    if (!implicitCast) {
        return;
    }
    //rhs->getType().dump();
    report(
        DiagnosticsEngine::Warning,
        "assigning a returned-by-value VclPtr<T> to a T* variable is dodgy, should be assigned to a VclPtr",
         rhs->getSourceRange().getBegin())
        << rhs->getSourceRange();
}

bool VCLWidgets::VisitVarDecl(const VarDecl * pVarDecl) {
    if (ignoreLocation(pVarDecl)) {
        return true;
    }
    if (isa<ParmVarDecl>(pVarDecl)) {
        return true;
    }
    if (pVarDecl->getInit()) {
        checkAssignmentForVclPtrToRawConversion(pVarDecl->getType().getTypePtr(), pVarDecl->getInit());
    }
    StringRef aFileName = compiler.getSourceManager().getFilename(compiler.getSourceManager().getSpellingLoc(pVarDecl->getLocStart()));
    if (aFileName == SRCDIR "/include/vcl/vclptr.hxx")
        return true;
    if (aFileName == SRCDIR "/vcl/source/window/layout.cxx")
        return true;
    // whitelist the valid things that can contain pointers.
    // It is containing stuff like std::unique_ptr we get worried
    if (pVarDecl->getType()->isArrayType()) {
        return true;
    }
    auto tc = loplugin::TypeCheck(pVarDecl->getType());
    if (tc.Pointer()
        || tc.Class("map").StdNamespace()
        || tc.Class("multimap").StdNamespace()
        || tc.Class("vector").StdNamespace()
        || tc.Class("list").StdNamespace()
        || tc.Class("mem_fun1_t").StdNamespace()
          // registration template thing, doesn't actually allocate anything we need to care about
        || tc.Class("OMultiInstanceAutoRegistration").Namespace("dbp").GlobalNamespace())
    {
        return true;
    }
    // Apparently I should be doing some kind of lookup for a partial specialisations of std::iterator_traits<T> to see if an
    // object is an iterator, but that sounds like too much work
    auto t = pVarDecl->getType().getDesugaredType(compiler.getASTContext());
    std::string s = t.getAsString();
    if (s.find("iterator") != std::string::npos
        || loplugin::TypeCheck(t).Class("__wrap_iter").StdNamespace())
    {
        return true;
    }
    // std::pair seems to show up in whacky ways in clang's AST. Sometimes it's a class, sometimes it's a typedef, and sometimes
    // its an ElaboratedType (whatever that is)
    if (s.find("pair") != std::string::npos) {
        return true;
    }

    if (containsVclReferenceBaseSubclass(pVarDecl->getType())) {
        report(
            DiagnosticsEngine::Warning,
            BASE_REF_COUNTED_CLASS " subclass %0 should be wrapped in VclPtr",
            pVarDecl->getLocation())
            << pVarDecl->getType() << pVarDecl->getSourceRange();
        return true;
    }
    return true;
}

bool VCLWidgets::VisitFieldDecl(const FieldDecl * fieldDecl) {
    if (ignoreLocation(fieldDecl)) {
        return true;
    }
    StringRef aFileName = compiler.getSourceManager().getFilename(compiler.getSourceManager().getSpellingLoc(fieldDecl->getLocStart()));
    if (aFileName == SRCDIR "/include/vcl/vclptr.hxx")
        return true;
    if (aFileName == SRCDIR "/include/rtl/ref.hxx")
        return true;
    if (aFileName == SRCDIR "/include/o3tl/enumarray.hxx")
        return true;
    if (aFileName == SRCDIR "/vcl/source/window/layout.cxx")
        return true;
    if (fieldDecl->isBitField()) {
        return true;
    }
    const CXXRecordDecl *pParentRecordDecl = isa<RecordDecl>(fieldDecl->getDeclContext()) ? dyn_cast<CXXRecordDecl>(fieldDecl->getParent()) : nullptr;
    if (pParentRecordDecl && loplugin::DeclCheck(pParentRecordDecl).Class("VclPtr").GlobalNamespace()) {
        return true;
    }
    if (containsVclReferenceBaseSubclass(fieldDecl->getType())) {
        // have to ignore this for now, nasty reverse dependency from tools->vcl
        if (!(pParentRecordDecl != nullptr &&
                (pParentRecordDecl->getQualifiedNameAsString() == "ErrorContextImpl" ||
                 pParentRecordDecl->getQualifiedNameAsString() == "ScHFEditPage"))) {
            report(
                DiagnosticsEngine::Warning,
                BASE_REF_COUNTED_CLASS " subclass %0 declared as a pointer member, should be wrapped in VclPtr",
                fieldDecl->getLocation())
                << fieldDecl->getType() << fieldDecl->getSourceRange();
            if (auto parent = dyn_cast<ClassTemplateSpecializationDecl>(fieldDecl->getParent())) {
                report(
                    DiagnosticsEngine::Note,
                    "template field here",
                    parent->getPointOfInstantiation());
            }
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

    // check if this field is derived fromVclReferenceBase
    if (isDerivedFromVclReferenceBase(recordDecl)) {
        report(
            DiagnosticsEngine::Warning,
            BASE_REF_COUNTED_CLASS " subclass allocated as a class member, should be allocated via VclPtr",
            fieldDecl->getLocation())
          << fieldDecl->getSourceRange();
    }

    // If this field is a VclPtr field, then the class MUST have a dispose method
    if (pParentRecordDecl && isDerivedFromVclReferenceBase(pParentRecordDecl)
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


static void findDisposeAndClearStatements(std::set<const FieldDecl*>& aVclPtrFields, const Stmt *pStmt)
{
    if (!pStmt)
        return;
    if (isa<CompoundStmt>(pStmt)) {
        const CompoundStmt *pCompoundStatement = dyn_cast<CompoundStmt>(pStmt);
        for (auto i = pCompoundStatement->body_begin();
             i != pCompoundStatement->body_end(); ++i)
        {
            findDisposeAndClearStatements(aVclPtrFields, *i);
        }
        return;
    }
    if (isa<ForStmt>(pStmt)) {
        findDisposeAndClearStatements(aVclPtrFields, dyn_cast<ForStmt>(pStmt)->getBody());
        return;
    }
    if (isa<IfStmt>(pStmt)) {
        findDisposeAndClearStatements(aVclPtrFields, dyn_cast<IfStmt>(pStmt)->getThen());
        findDisposeAndClearStatements(aVclPtrFields, dyn_cast<IfStmt>(pStmt)->getElse());
        return;
    }
    if (!isa<CallExpr>(pStmt)) return;
    const CallExpr *pCallExpr = dyn_cast<CallExpr>(pStmt);

    if (!pCallExpr->getDirectCallee()) return;
    if (!isa<CXXMethodDecl>(pCallExpr->getDirectCallee())) return;
    const CXXMethodDecl *pCalleeMethodDecl = dyn_cast<CXXMethodDecl>(pCallExpr->getDirectCallee());
    if (pCalleeMethodDecl->getNameAsString() != "disposeAndClear"
        && pCalleeMethodDecl->getNameAsString() != "clear")
            return;

    if (!pCallExpr->getCallee()) return;

    if (!isa<MemberExpr>(pCallExpr->getCallee())) return;
    const MemberExpr *pCalleeMemberExpr = dyn_cast<MemberExpr>(pCallExpr->getCallee());

    if (!pCalleeMemberExpr->getBase()) return;
    if (!isa<MemberExpr>(pCalleeMemberExpr->getBase())) return;
    const MemberExpr *pCalleeMemberExprBase = dyn_cast<MemberExpr>(pCalleeMemberExpr->getBase());

    const FieldDecl* xxx = dyn_cast_or_null<FieldDecl>(pCalleeMemberExprBase->getMemberDecl());
    if (xxx)
        aVclPtrFields.erase(xxx);
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
    if (functionDecl->hasBody() && pMethodDecl && isDerivedFromVclReferenceBase(pMethodDecl->getParent())) {
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

    // check dispose method to make sure we are actually disposing all of the VclPtr fields
    // FIXME this is not exhaustive. We should enable shouldVisitTemplateInstantiations and look deeper inside type declarations
    if (pMethodDecl && pMethodDecl->isInstance() && pMethodDecl->getBody()
        && pMethodDecl->param_size()==0
        && pMethodDecl->getNameAsString() == "dispose"
        && isDerivedFromVclReferenceBase(pMethodDecl->getParent()) )
    {
        std::string methodParent = pMethodDecl->getParent()->getNameAsString();
        if (methodParent == "VirtualDevice" || methodParent == "Breadcrumb")
            return true;

        std::set<const FieldDecl*> aVclPtrFields;
        for (auto i = pMethodDecl->getParent()->field_begin();
             i != pMethodDecl->getParent()->field_end(); ++i)
        {
            auto const type = loplugin::TypeCheck((*i)->getType());
            if (type.Class("VclPtr").GlobalNamespace()) {
               aVclPtrFields.insert(*i);
            } else if (type.Class("vector").StdNamespace()
                       || type.Class("map").StdNamespace()
                       || type.Class("list").StdNamespace()
                       || type.Class("set").StdNamespace())
            {
                const RecordType* recordType = dyn_cast_or_null<RecordType>((*i)->getType()->getUnqualifiedDesugaredType());
                if (recordType) {
                    auto d = dyn_cast<ClassTemplateSpecializationDecl>(recordType->getDecl());
                    if (d && d->getTemplateArgs().size()>0) {
                        auto const type = loplugin::TypeCheck(d->getTemplateArgs()[0].getAsType());
                        if (type.Class("VclPtr").GlobalNamespace()) {
                            aVclPtrFields.insert(*i);
                        }
                    }
                }
            }
        }
        if (!aVclPtrFields.empty()) {
            findDisposeAndClearStatements( aVclPtrFields, pMethodDecl->getBody() );
            if (!aVclPtrFields.empty()) {
                //pMethodDecl->dump();
                std::string aMessage = BASE_REF_COUNTED_CLASS " subclass dispose() method does not call disposeAndClear() or clear() on the following field(s): ";
                for(auto s : aVclPtrFields)
                    aMessage += ", " + s->getNameAsString();
                report(
                    DiagnosticsEngine::Warning,
                    aMessage,
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
    if (pPointee && isDerivedFromVclReferenceBase(pPointee)) {
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
    StringRef aFileName = compiler.getSourceManager().getFilename(compiler.getSourceManager().getSpellingLoc(constructExpr->getLocStart()));
    if (aFileName == SRCDIR "/include/vcl/vclptr.hxx")
        return true;
    if (constructExpr->getConstructionKind() != CXXConstructExpr::CK_Complete) {
        return true;
    }
    const CXXConstructorDecl* pConstructorDecl = constructExpr->getConstructor();
    const CXXRecordDecl* recordDecl = pConstructorDecl->getParent();
    if (isDerivedFromVclReferenceBase(recordDecl)) {
        report(
            DiagnosticsEngine::Warning,
            "Calling constructor of a VclReferenceBase-derived type directly; all such creation should go via VclPtr<>::Create",
            constructExpr->getExprLoc());
    }
    return true;
}

loplugin::Plugin::Registration< VCLWidgets > X("vclwidgets");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
