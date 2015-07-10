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

/**

If you have:

    class Foo : public css::foo::XBaa {
    };

Then XBaa has acquire and release methods inherited from XInterface.
These are hard lifecycle controls.

If you see another class:

    class Baz {
        Foo aFooMember;
    };

this is a bug =) since aFooMember assumes heap allocated lifecycle and
not delete on last 'release'.

TODO check that things that extend SvRefBase are managed by SvRef
TODO fix the SvXMLImportContext class (mentioned below)
*/

namespace {

class RefCounting:
    public RecursiveASTVisitor<RefCounting>, public loplugin::Plugin
{
public:
    explicit RefCounting(InstantiationData const & data): Plugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitFieldDecl(const FieldDecl *);
};

bool BaseCheckNotSubclass(const CXXRecordDecl *BaseDefinition, void *p) {
    if (!BaseDefinition)
        return true;
    const char *pString = static_cast<const char *>(p);
    if (BaseDefinition->getQualifiedNameAsString() == pString) {
        return false;
    }
    return true;
}

bool isDerivedFrom(const CXXRecordDecl *decl, const char *pString) {
    if (!decl)
        return false;
    if (decl->getQualifiedNameAsString() == pString)
        return true;
    if (!decl->hasDefinition()) {
        return false;
    }
    if (// not sure what hasAnyDependentBases() does,
        // but it avoids classes we don't want, e.g. WeakAggComponentImplHelper1
        !decl->hasAnyDependentBases() &&
        !decl->forallBases(BaseCheckNotSubclass, static_cast<void*>(const_cast<char*>(pString)), true)) {
        return true;
    }
    return false;
}


bool containsXInterfaceSubclass(const Type* pType0);

bool containsXInterfaceSubclass(const QualType& qType) {
    return containsXInterfaceSubclass(qType.getTypePtr());
}

bool containsXInterfaceSubclass(const Type* pType0) {
    if (!pType0)
        return false;
    const Type* pType = pType0->getUnqualifiedDesugaredType();
    if (!pType)
        return false;
    const CXXRecordDecl* pRecordDecl = pType->getAsCXXRecordDecl();
    if (pRecordDecl) {
        pRecordDecl = pRecordDecl->getCanonicalDecl();
    }
    if (pRecordDecl) {
        const ClassTemplateSpecializationDecl* pTemplate = dyn_cast<ClassTemplateSpecializationDecl>(pRecordDecl);
        if (pTemplate) {
            if (pTemplate->getQualifiedNameAsString() == "com::sun::star::uno::Reference" )
                return false;
            if (pTemplate->getQualifiedNameAsString() == "com::sun::star::uno::WeakReference" )
                return false;
            if (pTemplate->getQualifiedNameAsString() == "rtl::Reference" )
                return false;
            for(unsigned i=0; i<pTemplate->getTemplateArgs().size(); ++i) {
                const TemplateArgument& rArg = pTemplate->getTemplateArgs()[i];
                if (rArg.getKind() == TemplateArgument::ArgKind::Type &&
                    containsXInterfaceSubclass(rArg.getAsType()))
                {
                    return true;
                }
            }
        }
    }
    if (pType->isPointerType()) {
        QualType pointeeType = pType->getPointeeType();
        return containsXInterfaceSubclass(pointeeType);
    } else if (pType->isArrayType()) {
        const ArrayType* pArrayType = dyn_cast<ArrayType>(pType);
        QualType elementType = pArrayType->getElementType();
        return containsXInterfaceSubclass(elementType);
    } else {
        return isDerivedFrom(pRecordDecl, "com::sun::star::uno::XInterface");
    }
}



bool RefCounting::VisitFieldDecl(const FieldDecl * fieldDecl) {
    if (ignoreLocation(fieldDecl)) {
        return true;
    }
    if (fieldDecl->isBitField()) {
        return true;
    }
    std::string aParentName = fieldDecl->getParent()->getQualifiedNameAsString();
    if ( aParentName == "com::sun::star::uno::BaseReference"
         || aParentName == "cppu::detail::element_alias"
         // playing some kind of game to avoid circular references
         || aParentName == "ucbhelper::ResultSetDataSupplier")
    {
        return true;
    }

    if (containsXInterfaceSubclass(fieldDecl->getType())) {
        report(
            DiagnosticsEngine::Warning,
            "XInterface subclass being directly heap/stack managed, should be managed via uno::Reference, "
            + fieldDecl->getType().getAsString()
            + ", parent is " + aParentName,
            fieldDecl->getLocation())
          << fieldDecl->getSourceRange();
        return true;
    }
    /*
    const RecordType *recordType = fieldDecl->getType()->getAs<RecordType>();
    if (recordType == nullptr) {
        return true;
    }
    const CXXRecordDecl *recordDecl = dyn_cast<CXXRecordDecl>(recordType->getDecl());
    if (recordDecl == nullptr) {
        return true;
    }

    // this classes override acquire/release and forwards to it's parent
    if (isDerivedFrom(recordDecl, "ListenerMultiplexerBase")) { // module UnoTools
        return true;
    }
    if (isDerivedFrom(recordDecl, "OWeakSubObject")) { // module svx
        return true;
    }
    if (isDerivedFrom(recordDecl, "dbaccess::OBookmarkContainer")) { // module dbaccess
        return true;
    }
    if (isDerivedFrom(recordDecl, "dbaui::OSbaWeakSubObject")) { // module dbaccess
        return true;
    }
    // I don't know how or why or if this is working, but it asserts in acquire/release
    if (isDerivedFrom(recordDecl, "SwXMetaText")) { // module sw
        return true;
    }
    // FIXME this class extends 2 different ref-counting bases, SvRefBase and XInterface (via.  ::cppu::WeakImplHelper)
    // I have no idea how to fix it
    if (isDerivedFrom(recordDecl, "SvXMLImportContext")) { // module xmloff
        return true;
    }

    // check if this field is derived from Window
    if (isDerivedFrom(recordDecl, "com::sun::star::uno::XInterface")) {
        report(
            DiagnosticsEngine::Warning,
            "XInterface subclass allocated as a class member, should be managed via uno::Reference. " + recordDecl->getQualifiedNameAsString(),
            fieldDecl->getLocation())
          << fieldDecl->getSourceRange();
    }
*/
    return true;
}

loplugin::Plugin::Registration< RefCounting > X("refcounting");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
