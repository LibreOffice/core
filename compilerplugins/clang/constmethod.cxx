/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <iostream>

#include "config_clang.h"

#include "plugin.hxx"
#include "compat.hxx"
#include "check.hxx"
#include "functionaddress.hxx"

#if CLANG_VERSION >= 110000
#include "clang/AST/ParentMapContext.h"
#endif

/**
   Find methods that can be declared const.

   This analysis attempts to implement "logical const" as opposed to "technical const", which means
   we ignore always-const nature of std::unique_ptr::operator->

   This is not a sophisticated analysis. It deliberately skips all of the hard cases for now.
   It is an exercise in getting the most benefit for the least effort.
*/
namespace
{

class ConstMethod:
    public loplugin::FunctionAddress<loplugin::FilteringPlugin<ConstMethod>>
{
public:
    explicit ConstMethod(loplugin::InstantiationData const & data): FunctionAddress(data) {}

    virtual void run() override {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        for (const CXXMethodDecl *pMethodDecl : interestingMethodSet) {
            if (methodCannotBeConstSet.find(pMethodDecl) != methodCannotBeConstSet.end())
                continue;
            auto canonicalDecl = pMethodDecl->getCanonicalDecl();
            if (getFunctionsWithAddressTaken().find((FunctionDecl const *)canonicalDecl)
                    != getFunctionsWithAddressTaken().end())
                continue;
            StringRef aFileName = getFilenameOfLocation(compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(canonicalDecl)));
            if (loplugin::isSamePathname(aFileName, SRCDIR "/include/LibreOfficeKit/LibreOfficeKit.hxx"))
                continue;
            report(
                DiagnosticsEngine::Warning,
                "this method can be const",
                compat::getBeginLoc(pMethodDecl))
                << pMethodDecl->getSourceRange();
            if (canonicalDecl->getLocation() != pMethodDecl->getLocation()) {
                report(
                    DiagnosticsEngine::Note,
                    "canonical method declaration here",
                    compat::getBeginLoc(canonicalDecl))
                    << canonicalDecl->getSourceRange();
            }
        }
    }

    bool TraverseCXXMethodDecl(CXXMethodDecl *);
    bool TraverseCXXConversionDecl(CXXConversionDecl *);
    bool VisitCXXMethodDecl(const CXXMethodDecl *);
    bool VisitCXXThisExpr(const CXXThisExpr *);

private:
    bool isPointerOrReferenceToConst(const QualType& qt);
    bool isPointerOrReferenceToNonConst(const QualType& qt);
    bool checkIfCanBeConst(const Stmt*, const CXXMethodDecl*);

    std::unordered_set<const CXXMethodDecl*> interestingMethodSet;
    std::unordered_set<const CXXMethodDecl*> methodCannotBeConstSet;
    CXXMethodDecl const * currCXXMethodDecl;
};

bool ConstMethod::TraverseCXXMethodDecl(CXXMethodDecl * cxxMethodDecl)
{
    currCXXMethodDecl = cxxMethodDecl;
    bool rv = RecursiveASTVisitor<ConstMethod>::TraverseCXXMethodDecl(cxxMethodDecl);
    currCXXMethodDecl = nullptr;
    return rv;
}

bool ConstMethod::TraverseCXXConversionDecl(CXXConversionDecl * cxxConversionDecl)
{
    currCXXMethodDecl = cxxConversionDecl;
    bool rv = RecursiveASTVisitor<ConstMethod>::TraverseCXXConversionDecl(cxxConversionDecl);
    currCXXMethodDecl = nullptr;
    return rv;
}

bool ConstMethod::VisitCXXMethodDecl(const CXXMethodDecl * cxxMethodDecl)
{
    if (ignoreLocation(cxxMethodDecl) || !cxxMethodDecl->isThisDeclarationADefinition()) {
        return true;
    }
    if (cxxMethodDecl->isConst())
        return true;
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(cxxMethodDecl)) {
        return true;
    }
    // TODO ignore template stuff for now
    if (cxxMethodDecl->getTemplatedKind() != FunctionDecl::TK_NonTemplate) {
        return true;
    }
    if (cxxMethodDecl->isDeleted())
        return true;
    if (cxxMethodDecl->isStatic())
        return true;
    if (cxxMethodDecl->isOverloadedOperator())
        return true;
    if (isa<CXXConstructorDecl>(cxxMethodDecl))
        return true;
    if (isa<CXXDestructorDecl>(cxxMethodDecl))
        return true;
    if (cxxMethodDecl->getParent()->getDescribedClassTemplate() != nullptr ) {
        return true;
    }
    // ignore virtual methods
    if (cxxMethodDecl->isVirtual() ) {
        return true;
    }
    // ignore macro expansions so we can ignore the IMPL_LINK macros from include/tools/link.hxx
    // TODO make this more precise
    if (cxxMethodDecl->getLocation().isMacroID())
        return true;

    if (!cxxMethodDecl->getIdentifier())
        return true;
    if (cxxMethodDecl->getNumParams() > 0)
        return true;
    // returning pointers or refs to non-const stuff, and then having the whole method
    // be const doesn't seem like a good idea
    auto tc = loplugin::TypeCheck(cxxMethodDecl->getReturnType());
    if (tc.Pointer().NonConst())
        return true;
    if (tc.LvalueReference().NonConst())
        return true;
    // a Get method that returns void is probably doing something that has side-effects
    if (tc.Void())
        return true;

    StringRef name = cxxMethodDecl->getName();
    if (!name.startswith("get") && !name.startswith("Get")
        && !name.startswith("is") && !name.startswith("Is")
        && !name.startswith("has") && !name.startswith("Has"))
        return true;

    // something lacking in my analysis here
    if (loplugin::DeclCheck(cxxMethodDecl).Function("GetDescr").Class("SwRangeRedline").GlobalNamespace())
        return true;

    interestingMethodSet.insert(cxxMethodDecl);

    return true;
}

bool ConstMethod::VisitCXXThisExpr( const CXXThisExpr* cxxThisExpr )
{
    if (!currCXXMethodDecl)
        return true;
    if (ignoreLocation(cxxThisExpr))
        return true;
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compat::getBeginLoc(cxxThisExpr)))
        return true;
    if (interestingMethodSet.find(currCXXMethodDecl) == interestingMethodSet.end())
        return true;
    // no need to check again if we have already eliminated this one
    if (methodCannotBeConstSet.find(currCXXMethodDecl) != methodCannotBeConstSet.end())
        return true;
    if (!checkIfCanBeConst(cxxThisExpr, currCXXMethodDecl))
        methodCannotBeConstSet.insert(currCXXMethodDecl);

    return true;
}

// Walk up from a statement that contains a CXXThisExpr, checking if the usage means that the
// related CXXMethodDecl can be const.
bool ConstMethod::checkIfCanBeConst(const Stmt* stmt, const CXXMethodDecl* cxxMethodDecl)
{
    const Stmt* parent = getParentStmt( stmt );
    if (!parent) {
        auto parentsRange = compiler.getASTContext().getParents(*stmt);
        if ( parentsRange.begin() == parentsRange.end())
            return true;
        auto varDecl = dyn_cast_or_null<VarDecl>(parentsRange.begin()->get<Decl>());
        if (!varDecl)
        {
            report(
                 DiagnosticsEngine::Warning,
                 "no parent?",
                  compat::getBeginLoc(stmt))
                  << stmt->getSourceRange();
            return false;
        }
        return varDecl->getType()->isIntegralOrEnumerationType()
            || loplugin::TypeCheck(varDecl->getType()).Pointer().Const()
            || loplugin::TypeCheck(varDecl->getType()).LvalueReference().Const();
    }

    if (auto unaryOperator = dyn_cast<UnaryOperator>(parent)) {
        UnaryOperator::Opcode op = unaryOperator->getOpcode();
        if (op == UO_AddrOf || op == UO_PreInc || op == UO_PostInc
            || op == UO_PreDec || op == UO_PostDec) {
            return false;
        }
        if (op == UO_Deref) {
            return checkIfCanBeConst(parent, cxxMethodDecl);
        }
        return true;
    } else if (auto binaryOp = dyn_cast<BinaryOperator>(parent)) {
        BinaryOperator::Opcode op = binaryOp->getOpcode();
        if (binaryOp->getRHS() == stmt) {
            return true;
        }
        if (op == BO_Assign || op == BO_PtrMemD || op == BO_PtrMemI || op == BO_MulAssign
            || op == BO_DivAssign || op == BO_RemAssign || op == BO_AddAssign
            || op == BO_SubAssign || op == BO_ShlAssign || op == BO_ShrAssign
            || op == BO_AndAssign || op == BO_XorAssign || op == BO_OrAssign) {
            return false;
        }
//        // for pointer arithmetic need to check parent
//        if (binaryOp->getType()->isPointerType()) {
//            return checkIfCanBeConst(parent, cxxMethodDecl);
//        }
        return true;
    } else if (auto constructExpr = dyn_cast<CXXConstructExpr>(parent)) {
        const CXXConstructorDecl * constructorDecl = constructExpr->getConstructor();
        for (unsigned i = 0; i < constructExpr->getNumArgs(); ++i) {
            if (constructExpr->getArg(i) == stmt) {
                return isPointerOrReferenceToConst(constructorDecl->getParamDecl(i)->getType());
            }
        }
        return false; // TODO ??
    } else if (auto operatorCallExpr = dyn_cast<CXXOperatorCallExpr>(parent)) {
        const CXXMethodDecl* calleeMethodDecl = dyn_cast_or_null<CXXMethodDecl>(operatorCallExpr->getDirectCallee());
        if (calleeMethodDecl) {
            // unary operator
            if (calleeMethodDecl->getNumParams() == 0) {
                // some classes like std::unique_ptr do not do a very good job with their operator-> which is always const
                if (operatorCallExpr->getOperator() == OO_Arrow || operatorCallExpr->getOperator() == OO_Star) {
                    return checkIfCanBeConst(parent, cxxMethodDecl);
                }
                return calleeMethodDecl->isConst();
            }
            // some classes like std::unique_ptr do not do a very good job with their operator[] which is always const
            if (calleeMethodDecl->getNumParams() == 1 && operatorCallExpr->getArg(0) == stmt) {
                if (operatorCallExpr->getOperator() == OO_Subscript) {
                    return false;
                }
            }
            // binary operator
            if (operatorCallExpr->getArg(0) == stmt) {
                return calleeMethodDecl->isConst();
            }
            unsigned const n = std::min(
                operatorCallExpr->getNumArgs(),
                calleeMethodDecl->getNumParams());
            for (unsigned i = 1; i < n; ++i)
                if (operatorCallExpr->getArg(i) == stmt) {
                    return isPointerOrReferenceToConst(calleeMethodDecl->getParamDecl(i - 1)->getType());
                }
        } else {
            const Expr* callee = operatorCallExpr->getCallee()->IgnoreParenImpCasts();
            const DeclRefExpr* dr = dyn_cast<DeclRefExpr>(callee);
            const FunctionDecl* calleeFunctionDecl = nullptr;
            if (dr) {
                calleeFunctionDecl = dyn_cast<FunctionDecl>(dr->getDecl());
            }
            if (calleeFunctionDecl) {
                for (unsigned i = 0; i < operatorCallExpr->getNumArgs(); ++i) {
                    if (operatorCallExpr->getArg(i) == stmt) {
                        return isPointerOrReferenceToConst(calleeFunctionDecl->getParamDecl(i)->getType());
                    }
                }
            }
        }
        return false; // TODO ???
    } else if (auto callExpr = dyn_cast<CallExpr>(parent)) {
        QualType functionType = callExpr->getCallee()->getType();
        if (functionType->isFunctionPointerType()) {
            functionType = functionType->getPointeeType();
        }
        if (const FunctionProtoType* prototype = functionType->getAs<FunctionProtoType>()) {
            // TODO could do better
            if (prototype->isVariadic()) {
                return false;
            }
            if (callExpr->getCallee() == stmt) {
                return true;
            }
            for (unsigned i = 0; i < callExpr->getNumArgs(); ++i) {
                if (callExpr->getArg(i) == stmt) {
                    return isPointerOrReferenceToConst(prototype->getParamType(i));
                }
            }
        }
        const FunctionDecl* calleeFunctionDecl = callExpr->getDirectCallee();
        if (calleeFunctionDecl)
        {
            if (auto memberCallExpr = dyn_cast<CXXMemberCallExpr>(parent)) {
                const MemberExpr* memberExpr = dyn_cast<MemberExpr>(stmt);
                if (memberExpr && memberCallExpr->getImplicitObjectArgument() == memberExpr->getBase())
                {
                    const CXXMethodDecl* calleeMethodDecl = dyn_cast<CXXMethodDecl>(calleeFunctionDecl);
                    // some classes like std::unique_ptr do not do a very good job with their get() which is always const
                    if (calleeMethodDecl->getIdentifier() && calleeMethodDecl->getName() == "get") {
                        return checkIfCanBeConst(parent, cxxMethodDecl);
                    }
                    // VclPtr<T>'s implicit conversion to T*
                    if (isa<CXXConversionDecl>(calleeMethodDecl)) {
                        if (loplugin::DeclCheck(calleeMethodDecl->getParent()).Class("OWeakObject").Namespace("cppu").GlobalNamespace())
                            return false;
                        return checkIfCanBeConst(parent, cxxMethodDecl);
                    }
                    return calleeMethodDecl->isConst();
                }
            }
            // TODO could do better
            if (calleeFunctionDecl->isVariadic()) {
                return false;
            }
            if (callExpr->getCallee() == stmt) {
                return true;
            }
            for (unsigned i = 0; i < callExpr->getNumArgs(); ++i) {
                if (i >= calleeFunctionDecl->getNumParams()) // can happen in template code
                    return false;
                if (callExpr->getArg(i) == stmt) {
                    return isPointerOrReferenceToConst(calleeFunctionDecl->getParamDecl(i)->getType());
                }
            }
        }
        return false; // TODO ????
//    } else if (auto callExpr = dyn_cast<ObjCMessageExpr>(parent)) {
//        if (callExpr->getInstanceReceiver() == stmt) {
//            return true;
//        }
//        if (auto const method = callExpr->getMethodDecl()) {
//            // TODO could do better
//            if (method->isVariadic()) {
//                return false;
//            }
//            assert(method->param_size() == callExpr->getNumArgs());
//            for (unsigned i = 0; i < callExpr->getNumArgs(); ++i) {
//                if (callExpr->getArg(i) == stmt) {
//                    return isPointerOrReferenceToConst(
//                        method->param_begin()[i]->getType());
//                }
//            }
//        }
//        return false; // TODO ????
    } else if (isa<CXXReinterpretCastExpr>(parent)) {
        return false;
    } else if (isa<ImplicitCastExpr>(parent)) {
        return checkIfCanBeConst(parent, cxxMethodDecl);
    } else if (isa<CXXStaticCastExpr>(parent)) {
        return checkIfCanBeConst(parent, cxxMethodDecl);
    } else if (isa<CXXDynamicCastExpr>(parent)) {
        return checkIfCanBeConst(parent, cxxMethodDecl);
    } else if (isa<CXXFunctionalCastExpr>(parent)) {
        return checkIfCanBeConst(parent, cxxMethodDecl);
    } else if (isa<CXXConstCastExpr>(parent)) {
        return false;
    } else if (isa<CStyleCastExpr>(parent)) {
        return checkIfCanBeConst(parent, cxxMethodDecl);
//    } else if (isa<CastExpr>(parent)) { // all other cast expression subtypes
//        if (auto e = dyn_cast<ExplicitCastExpr>(parent)) {
//            if (loplugin::TypeCheck(e->getTypeAsWritten()).Void()) {
//                if (auto const sub = dyn_cast<DeclRefExpr>(
//                        e->getSubExpr()->IgnoreParenImpCasts()))
//                {
//                    if (sub->getDecl() == cxxMethodDecl) {
//                        return false;
//                    }
//                }
//            }
//        }
//        return checkIfCanBeConst(parent, cxxMethodDecl);
    } else if (isa<MemberExpr>(parent)) {
        return checkIfCanBeConst(parent, cxxMethodDecl);
    } else if (auto arraySubscriptExpr = dyn_cast<ArraySubscriptExpr>(parent)) {
        if (arraySubscriptExpr->getIdx() == stmt)
            return true;
        return checkIfCanBeConst(parent, cxxMethodDecl);
    } else if (isa<ParenExpr>(parent)) {
        return checkIfCanBeConst(parent, cxxMethodDecl);
    } else if (auto declStmt = dyn_cast<DeclStmt>(parent)) {
        for (Decl const * decl : declStmt->decls())
            if (auto varDecl = dyn_cast<VarDecl>(decl)) {
                if (varDecl->getInit() == stmt) {
                    auto tc = loplugin::TypeCheck(varDecl->getType());
                    if (tc.LvalueReference() && !tc.LvalueReference().Const())
                        return false;
                    if (tc.Pointer() && !tc.Pointer().Const())
                        return false;
                    return true;
                }
            }
        // fall through
    } else if (isa<ReturnStmt>(parent)) {
        return !isPointerOrReferenceToNonConst(cxxMethodDecl->getReturnType());
    } else if (isa<InitListExpr>(parent)) {
        return false; // TODO could be improved
    } else if (isa<IfStmt>(parent)) {
        return true;
    } else if (isa<WhileStmt>(parent)) {
        return true;
    } else if (isa<ForStmt>(parent)) {
        return true;
    } else if (isa<CompoundStmt>(parent)) {
        return true;
    } else if (isa<SwitchStmt>(parent)) {
        return true;
    } else if (isa<DoStmt>(parent)) {
        return true;
    } else if (isa<CXXDeleteExpr>(parent)) {
        return false;
//    } else if (isa<VAArgExpr>(parent)) {
//        return false;
    } else if (isa<CXXDependentScopeMemberExpr>(parent)) {
        return false;
    } else if (isa<MaterializeTemporaryExpr>(parent)) {
        return checkIfCanBeConst(parent, cxxMethodDecl);
    } else if (auto conditionalExpr = dyn_cast<ConditionalOperator>(parent)) {
        if (conditionalExpr->getCond() == stmt)
            return true;
        return checkIfCanBeConst(parent, cxxMethodDecl);
//    } else if (isa<UnaryExprOrTypeTraitExpr>(parent)) {
//        return false; // ???
    } else if (isa<CXXNewExpr>(parent)) {
//        for (auto pa : cxxNewExpr->placement_arguments())
//            if (pa == stmt)
//                return false;
        return true; // because the Stmt must be a parameter to the expression, probably an array length
//    } else if (auto lambdaExpr = dyn_cast<LambdaExpr>(parent)) {
////        for (auto it = lambdaExpr->capture_begin(); it != lambdaExpr->capture_end(); ++it)
////        {
////            if (it->capturesVariable() && it->getCapturedVar() == cxxMethodDecl)
////                return it->getCaptureKind() != LCK_ByRef;
////        }
//        return true;
//    } else if (isa<CXXTypeidExpr>(parent)) {
//        return true;
    } else if (isa<ParenListExpr>(parent)) {
        return true;
    } else if (isa<CXXUnresolvedConstructExpr>(parent)) {
        return false;
//    } else if (isa<UnresolvedMemberExpr>(parent)) {
//        return false;
//    } else if (isa<PackExpansionExpr>(parent)) {
//        return false;
    } else if (isa<ExprWithCleanups>(parent)) {
        return checkIfCanBeConst(parent, cxxMethodDecl);
//    } else if (isa<CaseStmt>(parent)) {
//        return true;
//    } else if (isa<CXXPseudoDestructorExpr>(parent)) {
//        return false;
//    } else if (isa<CXXDependentScopeMemberExpr>(parent)) {
//        return false;
//    } else if (isa<ObjCIvarRefExpr>(parent)) {
//        return checkIfCanBeConst(parent, cxxMethodDecl);
    } else if (isa<CXXTemporaryObjectExpr>(parent)) {
        return true;
    } else if (isa<CXXBindTemporaryExpr>(parent)) {
        return true;
    }
    if (parent)
        parent->dump();
//    if (cxxMethodDecl)
//        cxxMethodDecl->dump();
    report(
         DiagnosticsEngine::Warning,
         "oh dear, what can the matter be?",
          compat::getBeginLoc(parent))
          << parent->getSourceRange();
    return false;
}

bool ConstMethod::isPointerOrReferenceToConst(const QualType& qt) {
    auto const type = loplugin::TypeCheck(qt);
    if (type.Pointer()) {
        return bool(type.Pointer().Const());
    } else if (type.LvalueReference()) {
        return bool(type.LvalueReference().Const());
    }
    return false;
}

bool ConstMethod::isPointerOrReferenceToNonConst(const QualType& qt) {
    auto const type = loplugin::TypeCheck(qt);
    if (type.Pointer()) {
        return !bool(type.Pointer().Const());
    } else if (type.LvalueReference()) {
        return !bool(type.LvalueReference().Const());
    }
    return false;
}

loplugin::Plugin::Registration< ConstMethod > X("constmethod", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
