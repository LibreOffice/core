/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <set>
#include <iostream>

#include "plugin.hxx"
#include "compat.hxx"
#include "check.hxx"

/**
   Find pointer and reference params that can be declared const.

   This is not a sophisticated analysis. It deliberately skips all of the hard cases for now.
   It is an exercise in getting the most benefit for the least effort.
*/
namespace
{

class ConstParams:
    public RecursiveASTVisitor<ConstParams>, public loplugin::Plugin
{
public:
    explicit ConstParams(InstantiationData const & data): Plugin(data) {}

    virtual void run() override {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitFunctionDecl(FunctionDecl *);
    bool VisitDeclRefExpr( const DeclRefExpr* );
private:
    bool checkIfCanBeConst(const Stmt*);
    bool isPointerOrReferenceToConst(const QualType& qt);
    StringRef getFilename(const SourceLocation& loc);

    bool mbInsideFunction;
    std::set<const ParmVarDecl*> interestingSet;
    std::set<const ParmVarDecl*> cannotBeConstSet;
};

bool ConstParams::VisitFunctionDecl(FunctionDecl * functionDecl)
{
    if (ignoreLocation(functionDecl) || !functionDecl->doesThisDeclarationHaveABody()) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(functionDecl)) {
        return true;
    }
    // TODO ignore these for now, requires some extra work
    if (isa<CXXConstructorDecl>(functionDecl)) {
        return true;
    }
    // TODO ignore template stuff for now
    if (functionDecl->getTemplatedKind() != FunctionDecl::TK_NonTemplate) {
        return true;
    }
    if (isa<CXXMethodDecl>(functionDecl)
        && dyn_cast<CXXMethodDecl>(functionDecl)->getParent()->getDescribedClassTemplate() != nullptr ) {
        return true;
    }
    // ignore virtual methods
    if (isa<CXXMethodDecl>(functionDecl)
        && dyn_cast<CXXMethodDecl>(functionDecl)->isVirtual() ) {
        return true;
    }
    // ignore C main
    if (functionDecl->isMain()) {
        return true;
    }
    // ignore macro expansions so we can ignore the IMPL_LINK macros from include/tools/link.hxx
    // TODO make this more precise
    if (functionDecl->getLocation().isMacroID())
        return true;

    if (functionDecl->getIdentifier()) {
        StringRef name = functionDecl->getName();
        if (name == "yyerror" // ignore lex/yacc callback
                // some function callbacks
            || name == "PDFSigningPKCS7PasswordCallback"
            || name == "VCLExceptionSignal_impl"
            || name == "parseXcsFile"
                // UNO component entry points
            || name.endswith("component_getFactory"))
                return true;
    }

    StringRef aFileName = getFilename(functionDecl->getLocStart());
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sal/")
        || loplugin::hasPathnamePrefix(aFileName, SRCDIR "/bridges/")
        || loplugin::hasPathnamePrefix(aFileName, SRCDIR "/binaryurp/")
        || loplugin::hasPathnamePrefix(aFileName, SRCDIR "/stoc/")
        || loplugin::hasPathnamePrefix(aFileName, WORKDIR "/YaccTarget/unoidl/source/sourceprovider-parser.cxx")
        // some weird calling through a function pointer
        || loplugin::hasPathnamePrefix(aFileName, SRCDIR "/svtools/source/table/defaultinputhandler.cxx")
        // windows only
        || loplugin::hasPathnamePrefix(aFileName, SRCDIR "/basic/source/sbx/sbxdec.cxx")
        || loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sfx2/source/doc/syspath.cxx")) {
        return true;
    }

    // calculate the ones we want to check
    interestingSet.clear();
    for (const ParmVarDecl *pParmVarDecl : compat::parameters(*functionDecl)) {
        // ignore unused params
        if (pParmVarDecl->getName().empty())
            continue;
        auto const type = loplugin::TypeCheck(pParmVarDecl->getType());
        if (!type.Pointer() && !type.LvalueReference())
            continue;
        if (type.Pointer().Const())
            continue;
        if (type.LvalueReference().Const())
            continue;
        // since we can't normally change typedefs, just ignore them
        if (isa<TypedefType>(pParmVarDecl->getType()))
            continue;
        // TODO ignore these for now, has some effects I don't understand
        if (type.Pointer().Pointer())
            continue;
        // const is meaningless when applied to function pointer types
        if (pParmVarDecl->getType()->isFunctionPointerType())
            continue;
        interestingSet.insert(pParmVarDecl);
    }
    if (interestingSet.empty()) {
        return true;
    }

    mbInsideFunction = true;
    cannotBeConstSet.clear();
    bool ret = RecursiveASTVisitor::TraverseStmt(functionDecl->getBody());
    mbInsideFunction = false;

    for (const ParmVarDecl *pParmVarDecl : interestingSet) {
        if (cannotBeConstSet.find(pParmVarDecl) == cannotBeConstSet.end()) {
            report(
                DiagnosticsEngine::Warning,
                "this parameter can be const",
                pParmVarDecl->getLocStart())
                << pParmVarDecl->getSourceRange();
            if (functionDecl->getCanonicalDecl()->getLocation() != functionDecl->getLocation()) {
                unsigned idx = pParmVarDecl->getFunctionScopeIndex();
                ParmVarDecl* pOther = functionDecl->getCanonicalDecl()->getParamDecl(idx);
                report(
                    DiagnosticsEngine::Note,
                    "canonical parameter declaration here",
                    pOther->getLocStart())
                    << pOther->getSourceRange();
            }
            //functionDecl->dump();
        }
    }
    return ret;
}

bool ConstParams::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    if (!mbInsideFunction) {
        return true;
    }
    const ParmVarDecl* parmVarDecl = dyn_cast_or_null<ParmVarDecl>(declRefExpr->getDecl());
    if (!parmVarDecl) {
        return true;
    }
    if (interestingSet.find(parmVarDecl) == interestingSet.end()) {
        return true;
    }
    if (!checkIfCanBeConst(declRefExpr))
        cannotBeConstSet.insert(parmVarDecl);

    return true;
}

bool ConstParams::checkIfCanBeConst(const Stmt* stmt)
{
    const Stmt* parent = parentStmt( stmt );
    if (isa<UnaryOperator>(parent)) {
        const UnaryOperator* unaryOperator = dyn_cast<UnaryOperator>(parent);
        UnaryOperator::Opcode op = unaryOperator->getOpcode();
        if (op == UO_AddrOf || op == UO_PreInc || op == UO_PostInc
            || op == UO_PreDec || op == UO_PostDec) {
            return false;
        }
        if (op == UO_Deref) {
            return checkIfCanBeConst(parent);
        }
        return true;
    } else if (isa<BinaryOperator>(parent)) {
        const BinaryOperator* binaryOp = dyn_cast<BinaryOperator>(parent);
        BinaryOperator::Opcode op = binaryOp->getOpcode();
        // TODO could do better, but would require tracking the LHS
        if (binaryOp->getRHS() == stmt && op == BO_Assign) {
            return false;
        }
        if (binaryOp->getRHS() == stmt) {
            return true;
        }
        if (op == BO_Assign || op == BO_PtrMemD || op == BO_PtrMemI || op == BO_MulAssign
            || op == BO_DivAssign || op == BO_RemAssign || op == BO_AddAssign
            || op == BO_SubAssign || op == BO_ShlAssign || op == BO_ShrAssign
            || op == BO_AndAssign || op == BO_XorAssign || op == BO_OrAssign) {
            return false;
        }
        // for pointer arithmetic need to check parent
        if (binaryOp->getType()->isPointerType()) {
            return checkIfCanBeConst(parent);
        }
        return true;
    } else if (isa<CXXConstructExpr>(parent)) {
        const CXXConstructExpr* constructExpr = dyn_cast<CXXConstructExpr>(parent);
        const CXXConstructorDecl * constructorDecl = constructExpr->getConstructor();
        for (unsigned i = 0; i < constructExpr->getNumArgs(); ++i) {
            if (constructExpr->getArg(i) == stmt) {
                return isPointerOrReferenceToConst(constructorDecl->getParamDecl(i)->getType());
            }
        }
        std::cout << "dump0" << std::endl;
        parent->dump();
    } else if (isa<CXXOperatorCallExpr>(parent)) {
        const CXXOperatorCallExpr* operatorCallExpr = dyn_cast<CXXOperatorCallExpr>(parent);
        const CXXMethodDecl* calleeMethodDecl = dyn_cast<CXXMethodDecl>(operatorCallExpr->getDirectCallee());
        if (calleeMethodDecl) {
            // unary operator
            if (calleeMethodDecl->getNumParams() == 0) {
                return calleeMethodDecl->isConst();
            }
            // binary operator
            if (operatorCallExpr->getArg(0) == stmt) {
                return calleeMethodDecl->isConst();
            }
            if (operatorCallExpr->getArg(1) == stmt) {
                return isPointerOrReferenceToConst(calleeMethodDecl->getParamDecl(0)->getType());
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
        std::cout << "dump1" << std::endl;
        parent->dump();
        stmt->dump();
    } else if (isa<CallExpr>(parent)) {
        const CallExpr* callExpr = dyn_cast<CallExpr>(parent);
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
        if (isa<CXXMemberCallExpr>(parent)) {
            const CXXMemberCallExpr* memberCallExpr = dyn_cast<CXXMemberCallExpr>(parent);
            const MemberExpr* memberExpr = dyn_cast<MemberExpr>(stmt);
            if (memberExpr && memberCallExpr->getImplicitObjectArgument() == memberExpr->getBase())
            {
                const CXXMethodDecl* calleeMethodDecl = dyn_cast<CXXMethodDecl>(calleeFunctionDecl);
                return calleeMethodDecl->isConst();
            }
        }
        if (!calleeFunctionDecl) {
            std::cout << "---------------dump2" << std::endl;
            std::cout << "parent" << std::endl;
            parent->dump();
            std::cout << "child" << std::endl;
            stmt->dump();
            std::cout << "callee-type" << std::endl;
            callExpr->getCallee()->getType()->dump();
            for (unsigned i = 0; i < callExpr->getNumArgs(); ++i) {
                std::cout << "    param " << i << std::endl;
                callExpr->getArg(i)->dump();
            }
            return true;
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
                break;
            if (callExpr->getArg(i) == stmt) {
                return isPointerOrReferenceToConst(calleeFunctionDecl->getParamDecl(i)->getType());
            }
        }
        std::cout << "------------dump3" << std::endl;
        std::cout << "parent" << std::endl;
        parent->dump();
        std::cout << "child" << std::endl;
        stmt->dump();
        calleeFunctionDecl->dump();
        for (unsigned i = 0; i < callExpr->getNumArgs(); ++i) {
            std::cout << "    param " << i << std::endl;
            callExpr->getArg(i)->dump();
        }
        if (isa<CXXMemberCallExpr>(parent)) {
            std::cout << "implicit" << std::endl;
            dyn_cast<CXXMemberCallExpr>(parent)->getImplicitObjectArgument()->dump();
        }
    } else if (isa<CXXReinterpretCastExpr>(parent)) {
        return false;
    } else if (isa<CXXConstCastExpr>(parent)) {
        return false;
    } else if (isa<CastExpr>(parent)) { // all other cast expression subtypes
        return checkIfCanBeConst(parent);
    } else if (isa<MemberExpr>(parent)) {
        return checkIfCanBeConst(parent);
    } else if (isa<ArraySubscriptExpr>(parent)) {
        return checkIfCanBeConst(parent);
    } else if (isa<ParenExpr>(parent)) {
        return checkIfCanBeConst(parent);
    } else if (isa<DeclStmt>(parent)) {
        // TODO could do better here, but would require tracking the target(s)
        return false;
    } else if (isa<ReturnStmt>(parent)) {
        return isPointerOrReferenceToConst(dyn_cast<Expr>(stmt)->getType());
    } else if (isa<InitListExpr>(parent)) {
        return false;
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
    } else if (isa<CXXDeleteExpr>(parent)) {
        return false;
    } else if (isa<VAArgExpr>(parent)) {
        return false;
    } else if (isa<MaterializeTemporaryExpr>(parent)) {
        return true;
    } else if (const ConditionalOperator* conditionalExpr = dyn_cast<ConditionalOperator>(parent)) {
        if (conditionalExpr->getCond() == stmt)
            return true;
        return checkIfCanBeConst(parent);
    } else {
        std::cout << "dump5" << std::endl;
        parent->dump();
    }
    return true;
}

bool ConstParams::isPointerOrReferenceToConst(const QualType& qt) {
    auto const type = loplugin::TypeCheck(qt);
    if (type.Pointer()) {
        return bool(type.Pointer().Const());
    } else if (type.LvalueReference().Const().Pointer()) {
        // If we have a method that takes (T* t) and it calls std::vector<T*>::push_back
        // then the type of push_back is T * const &
        // There is probably a more elegant way to check this, but it will probably require
        // recalculating types while walking up the AST.
        return false;
    } else if (type.LvalueReference()) {
        return bool(type.LvalueReference().Const());
    }
    return false;
}

StringRef ConstParams::getFilename(const SourceLocation& loc)
{
    SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(loc);
    StringRef name { compiler.getSourceManager().getFilename(spellingLocation) };
    return name;
}

loplugin::Plugin::Registration< ConstParams > X("constparams", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
