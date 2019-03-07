/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>

#include <clang/AST/CXXInheritance.h>
#include "plugin.hxx"

/**
look for methods where all they do is call their superclass method
*/

namespace {

bool hasMultipleBaseInstances_(
    CXXRecordDecl const * derived, CXXRecordDecl const * canonicBase,
    bool & hasAsNonVirtualBase, bool & hasAsVirtualBase)
{
    for (auto i = derived->bases_begin(); i != derived->bases_end(); ++i) {
        auto const cls = i->getType()->getAsCXXRecordDecl();
        if (cls == nullptr) {
            assert(i->getType()->isDependentType());
            // Conservatively assume "yes" for dependent bases:
            return true;
        }
        if (cls->getCanonicalDecl() == canonicBase) {
            if (i->isVirtual()) {
                if (hasAsNonVirtualBase) {
                    return true;
                }
                hasAsVirtualBase = true;
            } else {
                if (hasAsNonVirtualBase || hasAsVirtualBase) {
                    return true;
                }
                hasAsNonVirtualBase = true;
            }
        } else if (hasMultipleBaseInstances_(
                       cls, canonicBase, hasAsNonVirtualBase, hasAsVirtualBase))
        {
            return true;
        }
    }
    return false;
}

bool hasMultipleBaseInstances(
    CXXRecordDecl const * derived, CXXRecordDecl const * base)
{
    bool nonVirt = false;
    bool virt = false;
    return hasMultipleBaseInstances_(
        derived, base->getCanonicalDecl(), nonVirt, virt);
}

class UnnecessaryOverride:
    public loplugin::FilteringPlugin<UnnecessaryOverride>
{
public:
    explicit UnnecessaryOverride(loplugin::InstantiationData const & data): FilteringPlugin(data) {}

    virtual bool preRun() override
    {
        // ignore some files with problematic macros
        StringRef fn(handler.getMainFileName());
        if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/framework/factories/ChildWindowPane.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/forms/source/component/Date.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/forms/source/component/Time.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/svx/source/dialog/hyperdlg.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/svx/source/dialog/rubydialog.cxx"))
            return false;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/canvas/"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/view/spelldialog.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/dlg/SpellDialogChildWindow.cxx"))
            return false;
        // HAVE_ODBC_ADMINISTRATION
        if (loplugin::isSamePathname(fn, SRCDIR "/dbaccess/source/ui/dlg/dsselect.cxx"))
            return false;
        return true;
    }

    virtual void run() override
    {
        if( preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXMethodDecl(const CXXMethodDecl *);

private:
    const CXXMethodDecl * findOverriddenOrSimilarMethodInSuperclasses(const CXXMethodDecl *);
    bool BaseCheckCallback(const CXXRecordDecl *BaseDefinition);
    CXXMemberCallExpr const * extractCallExpr(Expr const *);
};

bool UnnecessaryOverride::VisitCXXMethodDecl(const CXXMethodDecl* methodDecl)
{
    if (ignoreLocation(methodDecl->getCanonicalDecl())) {
        return true;
    }
    if (isa<CXXConstructorDecl>(methodDecl)) {
        return true;
    }

    StringRef aFileName = getFileNameOfSpellingLoc(
        compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(methodDecl)));

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

    // If overriding more than one base member function, or one base member
    // function that is available in multiple (non-virtual) base class
    // instances, then this is a disambiguating override:
    if (methodDecl->isVirtual()) {
        if (methodDecl->size_overridden_methods() != 1)
        {
            return true;
        }
        if (hasMultipleBaseInstances(
                methodDecl->getParent(),
                (*methodDecl->begin_overridden_methods())->getParent()))
        {
            return true;
        }
    }

    const CXXMethodDecl* overriddenMethodDecl = findOverriddenOrSimilarMethodInSuperclasses(methodDecl);
    if (!overriddenMethodDecl) {
        return true;
    }

    // Check for differences in default parameters:
    unsigned const numParams = methodDecl->getNumParams();
    assert(overriddenMethodDecl->getNumParams() == numParams);
    for (unsigned i = 0; i != numParams; ++i) {
        if (checkIdenticalDefaultArguments(
                methodDecl->getParamDecl(i)->getDefaultArg(),
                overriddenMethodDecl->getParamDecl(i)->getDefaultArg())
            != IdenticalDefaultArgumentsResult::Yes)
        {
            return true;
        }
    }

    if (methodDecl->getReturnType().getCanonicalType()
        != overriddenMethodDecl->getReturnType().getCanonicalType())
    {
        return true;
    }

    //TODO: check for identical exception specifications

    const CompoundStmt* compoundStmt = dyn_cast<CompoundStmt>(methodDecl->getBody());
    if (!compoundStmt || compoundStmt->size() > 2)
        return true;

    const CXXMemberCallExpr* callExpr = nullptr;
    if (compoundStmt->size() == 1)
    {
        if (methodDecl->getReturnType().getCanonicalType()->isVoidType())
        {
            if (auto const e = dyn_cast<Expr>(*compoundStmt->body_begin())) {
                callExpr = dyn_cast<CXXMemberCallExpr>(e->IgnoreImplicit()->IgnoreParens());
            }
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
            callExpr = extractCallExpr(returnExpr);
        }
    }
    else if (!methodDecl->getReturnType().getCanonicalType()->isVoidType())
    {
        /** handle constructions like
               bool foo() {
                bool ret = Base::foo();
                return ret;
            }
        */
        auto bodyIt = compoundStmt->body_begin();
        auto declStmt = dyn_cast<DeclStmt>(*bodyIt);
        if (!declStmt || !declStmt->isSingleDecl())
            return true;
        auto varDecl = dyn_cast<VarDecl>(declStmt->getSingleDecl());
        ++bodyIt;
        auto returnStmt = dyn_cast<ReturnStmt>(*bodyIt);
        if (!varDecl || !returnStmt)
            return true;
        Expr const * retValue = returnStmt->getRetValue()->IgnoreParenImpCasts();
        if (auto exprWithCleanups = dyn_cast<ExprWithCleanups>(retValue))
            retValue = exprWithCleanups->getSubExpr()->IgnoreParenImpCasts();
        if (auto constructExpr = dyn_cast<CXXConstructExpr>(retValue)) {
            if (constructExpr->getNumArgs() == 1)
                retValue = constructExpr->getArg(0)->IgnoreParenImpCasts();
        }
        if (!isa<DeclRefExpr>(retValue))
            return true;
        callExpr = extractCallExpr(varDecl->getInit());
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
        auto e = callExpr->getArg(i)->IgnoreImplicit();
        if (auto const e1 = dyn_cast<CXXConstructExpr>(e)) {
            if (e1->getConstructor()->isCopyOrMoveConstructor() && e1->getNumArgs() == 1) {
                e = e1->getArg(0)->IgnoreImpCasts();
            }
        }
        const DeclRefExpr * declRefExpr = dyn_cast<DeclRefExpr>(e);
        if (!declRefExpr || declRefExpr->getDecl() != methodDecl->getParamDecl(i))
            return true;
    }

    const CXXMethodDecl* pOther = nullptr;
    if (methodDecl->getCanonicalDecl()->getLocation() != methodDecl->getLocation())
        pOther = methodDecl->getCanonicalDecl();

    if (pOther) {
        StringRef aFileName = getFileNameOfSpellingLoc(
            compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(pOther)));
        // SFX_DECL_CHILDWINDOW_WITHID macro
        if (loplugin::isSamePathname(aFileName, SRCDIR "/include/sfx2/childwin.hxx"))
            return true;
    }

    report(
            DiagnosticsEngine::Warning, "%0%1 function just calls %2 parent",
            methodDecl->getLocation())
        << methodDecl->getAccess()
        << (methodDecl->isVirtual() ? " virtual" : "")
        << overriddenMethodDecl->getAccess()
        << methodDecl->getSourceRange();
    if (methodDecl->getCanonicalDecl()->getLocation() != methodDecl->getLocation()) {
        const CXXMethodDecl* pOther = methodDecl->getCanonicalDecl();
        report(
            DiagnosticsEngine::Note,
            "method declaration here",
            pOther->getLocation())
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

    const CXXMethodDecl* similarMethod = nullptr;
    CXXBasePath similarBasePath;

    auto BaseMatchesCallback = [&](const CXXBaseSpecifier *cxxBaseSpecifier, CXXBasePath& path)
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
            auto effectiveBaseMethodAccess = baseMethod->getAccess();
            if (effectiveBaseMethodAccess == AS_public && path.Access == AS_protected)
                effectiveBaseMethodAccess = AS_protected;
            if (effectiveBaseMethodAccess != methodDecl->getAccess())
                continue;
            if (!baseMethod->getDeclName().isIdentifier() || methodDecl->getName() != baseMethod->getName()) {
                continue;
            }
            if (methodDecl->isStatic() != baseMethod->isStatic()
                || methodDecl->isConst() != baseMethod->isConst()
                || methodDecl->isVolatile() != baseMethod->isVolatile()
                || (methodDecl->getRefQualifier()
                    != baseMethod->getRefQualifier())
                || methodDecl->isVariadic() != baseMethod->isVariadic())
            {
                continue;
            }
            if (methodDecl->getReturnType().getCanonicalType()
                != baseMethod->getReturnType().getCanonicalType())
            {
                continue;
            }
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
            {
                // if we have already found a method directly below us in the inheritance hierarchy, just ignore this one
                auto Compare = [&](CXXBasePathElement const & lhs, CXXBasePathElement const & rhs)
                {
                    return lhs.Class == rhs.Class;
                };
                if (similarMethod
                    && similarBasePath.size() < path.size()
                    && std::equal(similarBasePath.begin(), similarBasePath.end(),
                                  path.begin(), Compare))
                    break;
                if (similarMethod)
                    return true; // short circuit the process
                similarMethod = baseMethod;
                similarBasePath = path;
            }
        }
        return false;
    };

    CXXBasePaths aPaths;
    if (methodDecl->getParent()->lookupInBases(BaseMatchesCallback, aPaths))
        return nullptr;

    return similarMethod;
}

CXXMemberCallExpr const * UnnecessaryOverride::extractCallExpr(Expr const *returnExpr)
{
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

    return dyn_cast<CXXMemberCallExpr>(returnExpr->IgnoreParenImpCasts());
}

loplugin::Plugin::Registration< UnnecessaryOverride > unnecessaryoverride("unnecessaryoverride", true);

}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
