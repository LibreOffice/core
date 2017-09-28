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

#include <clang/AST/CXXInheritance.h>
#include "compat.hxx"
#include "plugin.hxx"

/**
look for methods where all they do is call their superclass method
*/

namespace {

class UnnecessaryOverride:
    public RecursiveASTVisitor<UnnecessaryOverride>, public loplugin::Plugin
{
public:
    explicit UnnecessaryOverride(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        // ignore some files with problematic macros
        StringRef fn( compiler.getSourceManager().getFileEntryForID(
                          compiler.getSourceManager().getMainFileID())->getName() );
        if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/framework/factories/ChildWindowPane.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/forms/source/component/Date.cxx"))
             return;
        if (loplugin::isSamePathname(fn, SRCDIR "/forms/source/component/Time.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/svx/source/dialog/hyperdlg.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/svx/source/dialog/rubydialog.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/canvas"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/view/spelldialog.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/dlg/SpellDialogChildWindow.cxx"))
            return;
        // HAVE_ODBC_ADMINISTRATION
        if (loplugin::isSamePathname(fn, SRCDIR "/dbaccess/source/ui/dlg/dsselect.cxx"))
            return;

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXMethodDecl(const CXXMethodDecl *);

private:
    const CXXMethodDecl * findOverriddenOrSimilarMethodInSuperclasses(const CXXMethodDecl *);
    bool BaseCheckCallback(
        const CXXRecordDecl *BaseDefinition
    #if CLANG_VERSION < 30800
        , void *
    #endif
        );
};

bool UnnecessaryOverride::VisitCXXMethodDecl(const CXXMethodDecl* methodDecl)
{
    if (ignoreLocation(methodDecl->getCanonicalDecl())) {
        return true;
    }
    if (isa<CXXConstructorDecl>(methodDecl)) {
        return true;
    }

    StringRef aFileName = compiler.getSourceManager().getFilename(compiler.getSourceManager().getSpellingLoc(methodDecl->getLocStart()));

    if (isa<CXXDestructorDecl>(methodDecl)
       && !isInUnoIncludeFile(methodDecl))
    {
        // the code is this method is only __compiled__ if OSL_DEBUG_LEVEL > 1
        if (loplugin::isSamePathname(aFileName, SRCDIR "/tools/source/stream/strmunx.cxx"))
            return true;

        // Warn about unnecessarily user-declared destructors.
        // A destructor is deemed unnecessary if:
        // * it is public;
        // * its class is only defined in the .cxx file (i.e., the virtual
        //   destructor is neither used to control the place of vtable
        //   emission, nor is its definition depending on types that may still
        //   be incomplete);
        //     or
        //   the destructor is inline, the class definition is complete,
        //     and the class has no superclasses
        // * it either does not have an explicit exception specification, or has
        //   a non-dependent explicit exception specification that is compatible
        //   with a non-dependent exception specification the destructor would
        //   have if it did not have an explicit one (TODO);
        // * it is either defined as defaulted or with an empty body.
        // Removing the user-declared destructor may cause the class to get an
        // implicitly declared move constructor and/or move assignment operator;
        // that is considered acceptable:  If any subobject cannot be moved, the
        // implicitly declared function will be defined as deleted (which is in
        // practice not much different from not having it declared), and
        // otherwise offering movability is likely even an improvement over not
        // offering it due to a "pointless" user-declared destructor.
        // Similarly, removing the user-declared destructor may cause the
        // implicit definition of a copy constructor and/or copy assignment
        // operator to change from being an obsolete feature to being a standard
        // feature.  That difference is not taken into account here.
        auto cls = methodDecl->getParent();
        if (methodDecl->getAccess() != AS_public)
        {
            return true;
        }
        if (!compiler.getSourceManager().isInMainFile(
                methodDecl->getCanonicalDecl()->getLocation())
            && !( methodDecl->isInlined()))
        {
            return true;
        }
        // if it's virtual, but it has a base-class with a non-virtual destructor
        if (methodDecl->isVirtual())
        {
            bool baseWithVirtualDtor = false;
            for (auto baseSpecifier = cls->bases_begin(); baseSpecifier != cls->bases_end(); ++baseSpecifier)
            {
                const RecordType* baseRecordType = baseSpecifier->getType()->getAs<RecordType>();
                if (baseRecordType)
                {
                    const CXXRecordDecl* baseRecordDecl = dyn_cast<CXXRecordDecl>(baseRecordType->getDecl());
                    if (baseRecordDecl && baseRecordDecl->getDestructor()
                        && baseRecordDecl->getDestructor()->isVirtual())
                    {
                        baseWithVirtualDtor = true;
                        break;
                    }
                }
            }
            if (!baseWithVirtualDtor)
            {
                return true;
            }
        }
        // corner case
        if (methodDecl->isInlined()
            && compiler.getSourceManager().isInMainFile(methodDecl->getLocation())
            && !compiler.getSourceManager().isInMainFile(methodDecl->getCanonicalDecl()->getLocation()))
        {
            return true;
        }
        if (!methodDecl->isExplicitlyDefaulted()) {
            if (!methodDecl->doesThisDeclarationHaveABody()
                || methodDecl->isLateTemplateParsed())
            {
                return true;
            }
            auto stmt = dyn_cast<CompoundStmt>(methodDecl->getBody());
            if (stmt == nullptr || stmt->size() != 0) {
                return true;
            }
        }
        //TODO: exception specification
        if (!(cls->hasUserDeclaredCopyConstructor()
              || cls->hasUserDeclaredCopyAssignment()
              || cls->hasUserDeclaredMoveConstructor()
              || cls->hasUserDeclaredMoveAssignment()))
        {
        }
        if ((cls->needsImplicitMoveConstructor()
             && !(cls->hasUserDeclaredCopyConstructor()
                  || cls->hasUserDeclaredCopyAssignment()
                  || cls->hasUserDeclaredMoveAssignment()))
            || (cls->needsImplicitMoveAssignment()
                && !(cls->hasUserDeclaredCopyConstructor()
                     || cls->hasUserDeclaredCopyAssignment()
                     || cls->hasUserDeclaredMoveConstructor())))
        {
            report(DiagnosticsEngine::Fatal, "TODO", methodDecl->getLocation());
            return true;
        }
        report(
            DiagnosticsEngine::Warning, "unnecessary user-declared destructor",
            methodDecl->getLocation())
            << methodDecl->getSourceRange();
        auto cd = methodDecl->getCanonicalDecl();
        if (cd->getLocation() != methodDecl->getLocation()) {
            report(DiagnosticsEngine::Note, "declared here", cd->getLocation())
                << cd->getSourceRange();
        }
        return true;
    }

    if (!methodDecl->doesThisDeclarationHaveABody()
        || methodDecl->isLateTemplateParsed())
    {
        return true;
    }

    // if we are overriding more than one method, then this is a disambiguating override
    if (methodDecl->isVirtual()) {
        if (methodDecl->size_overridden_methods() != 1)
        {
            return true;
        }
    }
    // sometimes the disambiguation happens in a base class
    if (loplugin::isSamePathname(aFileName, SRCDIR "/comphelper/source/property/propertycontainer.cxx"))
        return true;
    // not sure what is happening here
    if (loplugin::isSamePathname(aFileName, SRCDIR "/extensions/source/bibliography/datman.cxx"))
        return true;
    // some very creative method hiding going on here
    if (loplugin::isSamePathname(aFileName, SRCDIR "/svx/source/dialog/checklbx.cxx"))
        return true;
    // entertaining template magic
    if (loplugin::isSamePathname(aFileName, SRCDIR "/sc/source/ui/vba/vbaformatcondition.cxx"))
        return true;
    // not sure what is going on here, but removing the override causes a crash
     if (methodDecl->getQualifiedNameAsString() == "SwXTextDocument::queryAdapter")
         return true;


    const CXXMethodDecl* overriddenMethodDecl = findOverriddenOrSimilarMethodInSuperclasses(methodDecl);
    if (!overriddenMethodDecl) {
        return true;
    }

    if (compat::getReturnType(*methodDecl).getCanonicalType()
        != compat::getReturnType(*overriddenMethodDecl).getCanonicalType())
    {
        return true;
    }

    //TODO: check for identical exception specifications

    const CompoundStmt* compoundStmt = dyn_cast<CompoundStmt>(methodDecl->getBody());
    if (!compoundStmt || compoundStmt->size() != 1)
        return true;

    const CXXMemberCallExpr* callExpr;
    if (compat::getReturnType(*methodDecl).getCanonicalType()->isVoidType())
    {
        callExpr = dyn_cast<CXXMemberCallExpr>(*compoundStmt->body_begin());
    }
    else
    {
        auto returnStmt = dyn_cast<ReturnStmt>(*compoundStmt->body_begin());
        if (returnStmt == nullptr) {
            return true;
        }
        auto returnExpr = returnStmt->getRetValue();
        if (returnExpr == nullptr) {
            return true;
        }
        returnExpr = returnExpr->IgnoreImplicit();

        // In something like
        //
        //  Reference< XResultSet > SAL_CALL OPreparedStatement::executeQuery(
        //      const rtl::OUString& sql)
        //      throw(SQLException, RuntimeException, std::exception)
        //  {
        //      return OCommonStatement::executeQuery( sql );
        //  }
        //
        // look down through all the
        //
        //   ReturnStmt
        //   `-ExprWithCleanups
        //     `-CXXConstructExpr
        //      `-MaterializeTemporaryExpr
        //       `-ImplicitCastExpr
        //        `-CXXBindTemporaryExpr
        //         `-CXXMemberCallExpr
        //
        // where the fact that the overriding and overridden function have identical
        // return types makes us confident that all we need to check here is whether
        // there's an (arbitrary, one-argument) CXXConstructorExpr and
        // CXXBindTemporaryExpr in between:
        if (auto ctorExpr = dyn_cast<CXXConstructExpr>(returnExpr)) {
            if (ctorExpr->getNumArgs() == 1) {
                auto tempExpr1 = ctorExpr->getArg(0)->IgnoreImplicit();
                if (auto tempExpr2 = dyn_cast<CXXBindTemporaryExpr>(tempExpr1))
                {
                    returnExpr = tempExpr2->getSubExpr();
                }
                else if (auto tempExpr2 = dyn_cast<CXXMemberCallExpr>(tempExpr1))
                {
                    returnExpr = tempExpr2;
                }
            }
        }

        callExpr = dyn_cast<CXXMemberCallExpr>(returnExpr->IgnoreParenImpCasts());
    }

    if (!callExpr || callExpr->getMethodDecl() != overriddenMethodDecl)
        return true;
    const Expr* expr1 = callExpr->getImplicitObjectArgument()->IgnoreImpCasts();
    if (!expr1)
        return true;
    const CXXThisExpr* expr2 = dyn_cast_or_null<CXXThisExpr>(expr1);
    if (!expr2)
        return true;
    for (unsigned i = 0; i<callExpr->getNumArgs(); ++i) {
        // ignore ImplicitCastExpr
        const DeclRefExpr * declRefExpr = dyn_cast<DeclRefExpr>(callExpr->getArg(i)->IgnoreImplicit());
        if (!declRefExpr || declRefExpr->getDecl() != methodDecl->getParamDecl(i))
            return true;
    }

    report(
            DiagnosticsEngine::Warning, "%0%1 function just calls %2 parent",
            methodDecl->getSourceRange().getBegin())
        << methodDecl->getAccess()
        << (methodDecl->isVirtual() ? " virtual" : "")
        << overriddenMethodDecl->getAccess()
        << methodDecl->getSourceRange();
    if (methodDecl->getCanonicalDecl()->getLocation() != methodDecl->getLocation()) {
        const CXXMethodDecl* pOther = methodDecl->getCanonicalDecl();
        report(
            DiagnosticsEngine::Note,
            "method declaration here",
            pOther->getLocStart())
          << pOther->getSourceRange();
    }
    return true;
}

const CXXMethodDecl* UnnecessaryOverride::findOverriddenOrSimilarMethodInSuperclasses(const CXXMethodDecl* methodDecl)
{
    if (methodDecl->isVirtual()) {
        return *methodDecl->begin_overridden_methods();
    }
    if (!methodDecl->getDeclName().isIdentifier()) {
        return nullptr;
    }

#if CLANG_VERSION < 30800
        return nullptr;
#else
    std::vector<const CXXMethodDecl*> maSimilarMethods;

    auto BaseMatchesCallback = [&](const CXXBaseSpecifier *cxxBaseSpecifier, CXXBasePath& )
    {
        if (cxxBaseSpecifier->getAccessSpecifier() != AS_public && cxxBaseSpecifier->getAccessSpecifier() != AS_protected)
            return false;
        if (!cxxBaseSpecifier->getType().getTypePtr())
            return false;
        const CXXRecordDecl* baseCXXRecordDecl = cxxBaseSpecifier->getType()->getAsCXXRecordDecl();
        if (!baseCXXRecordDecl)
            return false;
        if (baseCXXRecordDecl->isInvalidDecl())
            return false;
        for (const CXXMethodDecl* baseMethod : baseCXXRecordDecl->methods())
        {
            if (!baseMethod->getDeclName().isIdentifier() || methodDecl->getName() != baseMethod->getName()) {
                continue;
            }
            if (compat::getReturnType(*methodDecl).getCanonicalType()
                != compat::getReturnType(*baseMethod).getCanonicalType())
            {
                continue;
            }
            if (methodDecl->param_size() != baseMethod->param_size())
                continue;
            if (methodDecl->getNumParams() != baseMethod->getNumParams())
                continue;
            bool bParamsMatch = true;
            for (unsigned i=0; i<methodDecl->param_size(); ++i)
            {
                if (methodDecl->parameters()[i]->getType() != baseMethod->parameters()[i]->getType())
                {
                    bParamsMatch = false;
                    break;
                }
            }
            if (bParamsMatch)
                maSimilarMethods.push_back(baseMethod);
        }
        return false;
    };

    CXXBasePaths aPaths;
    methodDecl->getParent()->lookupInBases(BaseMatchesCallback, aPaths);

    if (maSimilarMethods.size() == 1) {
        return maSimilarMethods[0];
    }
    return nullptr;
#endif
}


loplugin::Plugin::Registration< UnnecessaryOverride > X("unnecessaryoverride", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
