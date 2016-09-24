/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"

namespace {

class DerivedClass:
    public RecursiveASTVisitor<DerivedClass>,
    public loplugin::Plugin
{
public:
    explicit DerivedClass(InstantiationData const & data):
        Plugin(data) {}

    virtual void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCXXRecordDecl(CXXRecordDecl const * decl);
};

bool BaseCheck(const CXXRecordDecl *BaseDefinition, void *BaseClassName) {
    // print warning about deriving from this classes
    // the name has to contain namespace, e.g. foo::bar::ClassName
    const char *BaseClasses[] = {
        "Dialog",
        "ProgressBar",
        "SfxToolBoxControl",
        "StatusBar",
        0,
    };
    for (int i = 0; BaseClasses[i]; i++)
        if (BaseDefinition->getQualifiedNameAsString().compare(BaseClasses[i]) == 0) {
            *(const char **)BaseClassName = BaseClasses[i];
            return false;
        }
    return true;
}

bool DerivedClass::VisitCXXRecordDecl(CXXRecordDecl const * decl) {
    const char *BaseClassName = 0;
    // checking for decl->hasDefinition() avoids crash in decl->forallBases
    if (decl->hasDefinition() &&
            // not sure what hasAnyDependentBases() does,
            // but it avoids classes we don't want, e.g. WeakAggComponentImplHelper1
            !decl->hasAnyDependentBases() &&
            !decl->forallBases(BaseCheck, &BaseClassName)) {
        string warning_msg("class %0 derives from ");
        // no idea how BaseClassName can be 0 sometimes..
        if (BaseClassName)
            warning_msg += BaseClassName;
        report(
            DiagnosticsEngine::Warning,
            warning_msg,
            decl->getLocStart())
        << decl->getQualifiedNameAsString() << decl->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration<DerivedClass> X("derivedclass");

}

/* vim:set shiftwidth=4 softtabstop=4 tabstop=4 expandtab: */
