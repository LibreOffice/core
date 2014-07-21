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

    bool mbFoundWindow;
};

bool forallBasesCallback(const CXXBaseSpecifier *Specifier, CXXBasePath &, void *UserData) {
    VCLWidgets* ro = (VCLWidgets*) UserData;
    QualType qt = Specifier->getType();
    std::string name = qt.getUnqualifiedType().getCanonicalType().getAsString();
    if (name == "class Window") {
          ro->mbFoundWindow = true;
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
    mbFoundWindow = false;
    CXXBasePaths paths;
    recordDecl->lookupInBases(forallBasesCallback, this, paths);
    if (!mbFoundWindow) {
        return true;
    }

    report(
        DiagnosticsEngine::Warning,
        "vcl::Window subclass not heap allocated",
        fieldDecl->getLocation())
      << fieldDecl->getSourceRange();
    return true;
}


loplugin::Plugin::Registration< VCLWidgets > X("vclwidgets");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
