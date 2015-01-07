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

    bool VisitFieldDecl(const FieldDecl * decl);

    bool VisitParmVarDecl(ParmVarDecl const * decl);

    bool VisitVarDecl( const VarDecl* var );

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

bool VCLWidgets::VisitFieldDecl(const FieldDecl * fieldDecl) {
    if (ignoreLocation(fieldDecl)) {
        return true;
    }
    if (fieldDecl->isBitField()) {
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

bool VCLWidgets::VisitParmVarDecl(ParmVarDecl const * pvDecl) {
    if (ignoreLocation(pvDecl)) {
        return true;
    }
    if (!pvDecl->getType()->isPointerType())
        return true;
    QualType pointeeType = pvDecl->getType()->getPointeeType();
    const RecordType *recordType = pointeeType->getAs<RecordType>();
    if (recordType == nullptr)
        return true;
    const CXXRecordDecl *recordDecl = dyn_cast<CXXRecordDecl>(recordType->getDecl());
    if (recordDecl == nullptr)
        return true;

    // check if this parameter is derived from Window
    if (isDerivedFromWindow(recordDecl)) {
        report(
            DiagnosticsEngine::Remark,
            "vcl::Window subclass passed as a pointer parameter, should be wrapped in VclPtr.",
            pvDecl->getLocation())
          << pvDecl->getSourceRange();
    }
    return true;
}

bool VCLWidgets::VisitVarDecl( const VarDecl* varDecl )
{
    if (ignoreLocation(varDecl)) {
        return true;
    }
    if (!varDecl->isLocalVarDecl())
        return true;
    if (!varDecl->getType()->isPointerType())
        return true;
    QualType pointeeType = varDecl->getType()->getPointeeType();
    const RecordType *recordType = pointeeType->getAs<RecordType>();
    if (recordType == nullptr)
        return true;
    const CXXRecordDecl *recordDecl = dyn_cast<CXXRecordDecl>(recordType->getDecl());
    if (recordDecl == nullptr)
        return true;

    // check if this variables type is derived from Window
    if (isDerivedFromWindow(recordDecl)) {
        report(
            DiagnosticsEngine::Remark,
            "vcl::Window subclass declared as a pointer var, should be wrapped in VclPtr.",
            varDecl->getLocation())
          << varDecl->getSourceRange();
    }

    return true;
}

loplugin::Plugin::Registration< VCLWidgets > X("vclwidgets");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
