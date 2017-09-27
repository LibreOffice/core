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

static bool startswith(const std::string& rStr, const char* pSubStr) {
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

class ConstParams:
    public RecursiveASTVisitor<ConstParams>, public loplugin::Plugin
{
public:
    explicit ConstParams(InstantiationData const & data): Plugin(data) {}

    virtual void run() override {
        std::string fn( compiler.getSourceManager().getFileEntryForID(
                        compiler.getSourceManager().getMainFileID())->getName() );
        normalizeDotDotInFilePath(fn);
        if (startswith(fn, SRCDIR "/sal/")
            || fn == SRCDIR "/jurt/source/pipe/staticsalhack.cxx"
            || startswith(fn, SRCDIR "/bridges/")
            || startswith(fn, SRCDIR "/binaryurp/")
            || startswith(fn, SRCDIR "/stoc/")
            || startswith(fn, WORKDIR "/YaccTarget/unoidl/source/sourceprovider-parser.cxx")
            // some weird calling through a function pointer
            || startswith(fn, SRCDIR "/svtools/source/table/defaultinputhandler.cxx")
            || startswith(fn, SRCDIR "/sdext/source/pdfimport/test/pdfunzip.cxx")
            // windows only
            || startswith(fn, SRCDIR "/basic/source/sbx/sbxdec.cxx")
            || startswith(fn, SRCDIR "/sfx2/source/doc/syspath.cxx")
            // ignore this for now
            || startswith(fn, SRCDIR "/libreofficekit")
            )
            return;

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        for (const ParmVarDecl *pParmVarDecl : interestingSet) {
            if (cannotBeConstSet.find(pParmVarDecl) == cannotBeConstSet.end()) {
                report(
                    DiagnosticsEngine::Warning,
                    "this parameter can be const",
                    pParmVarDecl->getLocStart())
                    << pParmVarDecl->getSourceRange();
                auto functionDecl = parmToFunction[pParmVarDecl];
                if (functionDecl->getCanonicalDecl()->getLocation() != functionDecl->getLocation()) {
                    unsigned idx = pParmVarDecl->getFunctionScopeIndex();
                    const ParmVarDecl* pOther = functionDecl->getCanonicalDecl()->getParamDecl(idx);
                    report(
                        DiagnosticsEngine::Note,
                        "canonical parameter declaration here",
                        pOther->getLocStart())
                        << pOther->getSourceRange();
                }
            }
        }
    }

    bool VisitFunctionDecl(const FunctionDecl *);
    bool VisitDeclRefExpr(const DeclRefExpr *);

private:
    bool checkIfCanBeConst(const Stmt*, const ParmVarDecl*);
    bool isPointerOrReferenceToConst(const QualType& qt);

    std::unordered_set<const ParmVarDecl*> interestingSet;
    std::unordered_map<const ParmVarDecl*, const FunctionDecl*> parmToFunction;
    std::unordered_set<const ParmVarDecl*> cannotBeConstSet;
};

bool ConstParams::VisitFunctionDecl(const FunctionDecl * functionDecl)
{
    if (ignoreLocation(functionDecl) || !functionDecl->isThisDeclarationADefinition()) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(functionDecl)) {
        return true;
    }
    // TODO ignore template stuff for now
    if (functionDecl->getTemplatedKind() != FunctionDecl::TK_NonTemplate) {
        return true;
    }
    if (functionDecl->isDeleted())
        return true;
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

    if (functionDecl->getIdentifier())
    {
        StringRef name = functionDecl->getName();
        if (name == "yyerror" // ignore lex/yacc callback
                    // some function callbacks
                    // TODO should really use a two-pass algorithm to detect and ignore these automatically
            || name == "PDFSigningPKCS7PasswordCallback"
            || name == "VCLExceptionSignal_impl"
            || name == "parseXcsFile"
            || name == "GraphicsExposePredicate"
            || name == "ImplPredicateEvent"
            || name == "timestamp_predicate"
            || name == "signalScreenSizeChanged"
            || name == "signalMonitorsChanged"
            || name == "signalButton"
            || name == "signalFocus"
            || name == "signalDestroy"
            || name == "signalSettingsNotify"
            || name == "signalStyleSet"
            || name == "signalIMCommit"
            || name == "compressWheelEvents"
            || name == "MenuBarSignalKey"
            || name == "signalDragDropReceived"
            || name == "memory_write"
            || name == "file_write"
            || name == "SalMainPipeExchangeSignal_impl"
            || name.startswith("SbRtl_")
            || name == "my_if_errors"
            || name == "my_eval_defined"
            || name == "my_eval_variable"
            || name == "ImpGetEscDir"
            || name == "ImpGetPercent"
            || name == "ImpGetAlign"
            || name == "write_function"
            || name == "PyUNO_getattr"
            || name == "PyUNO_setattr"
            || name == "PyUNOStruct_setattr"
            || name == "PyUNOStruct_getattr"
            || name == "GoNext"
            || name == "GoPrevious"
            || name == "lcl_SetOtherLineHeight"
            || name == "BoxNmsToPtr"
            || name == "PtrToBoxNms"
            || name == "RelNmsToBoxNms"
            || name == "RelBoxNmsToPtr"
            || name == "BoxNmsToRelNm"
            || name == "MakeFormula_"
            || name == "GetFormulaBoxes"
            || name == "HasValidBoxes_"
            || name == "SplitMergeBoxNm_"
            || name.startswith("Read_F_")
            || name == "UpdateFieldInformation"
             // #ifdef win32
            || name == "convert_slashes"
                // UNO component entry points
            || name.endswith("component_getFactory")
                // in Scheduler::, wants to loop until a reference to a bool becomes true
            || name == "ProcessEventsToSignal"
                // external API
            || name == "Java_com_sun_star_sdbcx_comp_hsqldb_StorageNativeOutputStream_flush"
            || name == "egiGraphicExport"
            || name == "etiGraphicExport"
            || name == "epsGraphicExport"
            || name == "releasePool" // vcl/osx/saldata.cxx
            || name == "call_userEventFn"
                // vcl/unx/gtk/gtkdata.cxx, vcl/unx/gtk3/gtk3gtkdata.cxx
            )
                return true;
    }

    // calculate the ones we want to check
    for (const ParmVarDecl *pParmVarDecl : compat::parameters(*functionDecl)) {
        // ignore unused params
        if (pParmVarDecl->getName().empty()
            || pParmVarDecl->hasAttr<UnusedAttr>())
            continue;
        auto const type = loplugin::TypeCheck(pParmVarDecl->getType());
        if (!type.Pointer() && !type.LvalueReference())
            continue;
        if (type.Pointer().Const())
            continue;
        if (type.LvalueReference().Const())
            continue;
        // since we normally can't change typedefs, just ignore them
        if (isa<TypedefType>(pParmVarDecl->getType()))
            continue;
        // TODO ignore these for now, has some effects I don't understand
        if (type.Pointer().Pointer())
            continue;
        // const is meaningless when applied to function pointer types
        if (pParmVarDecl->getType()->isFunctionPointerType())
            continue;
        // ignore things with template params
        if (pParmVarDecl->getType()->isInstantiationDependentType())
            continue;
        interestingSet.insert(pParmVarDecl);
        parmToFunction[pParmVarDecl] = functionDecl;
    }

    return true;
}

bool ConstParams::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    if (ignoreLocation(declRefExpr)) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(declRefExpr->getLocStart())) {
        return true;
    }
    const ParmVarDecl* parmVarDecl = dyn_cast_or_null<ParmVarDecl>(declRefExpr->getDecl());
    if (!parmVarDecl) {
        return true;
    }
    // no need to check again if we have already eliminated this one
    if (cannotBeConstSet.find(parmVarDecl) != cannotBeConstSet.end())
        return true;
    if (!checkIfCanBeConst(declRefExpr, parmVarDecl))
        cannotBeConstSet.insert(parmVarDecl);

    return true;
}

// Walk up from a statement that contains a DeclRefExpr, checking if the usage means that the
// related ParamVarDecl can be const.
bool ConstParams::checkIfCanBeConst(const Stmt* stmt, const ParmVarDecl* parmVarDecl)
{
    const Stmt* parent = parentStmt( stmt );
    if (!parent)
    {
        // check if we're inside a CXXCtorInitializer
        auto parentsRange = compiler.getASTContext().getParents(*stmt);
        if ( parentsRange.begin() == parentsRange.end())
            return true;
        auto cxxConstructorDecl = dyn_cast_or_null<CXXConstructorDecl>(parentsRange.begin()->get<Decl>());
        if (!cxxConstructorDecl)
            return true;
        for ( auto cxxCtorInitializer : cxxConstructorDecl->inits())
        {
            if (cxxCtorInitializer->isAnyMemberInitializer() && cxxCtorInitializer->getInit() == stmt)
            {
                // if the member is not pointer or ref to-const, we cannot make the param const
                auto fieldDecl = cxxCtorInitializer->getAnyMember();
                auto tc = loplugin::TypeCheck(fieldDecl->getType());
                return tc.Pointer().Const() || tc.LvalueReference().Const();
            }
        }
        parmVarDecl->dump();
        stmt->dump();
        cxxConstructorDecl->dump();
        report(
             DiagnosticsEngine::Warning,
             "couldn't find the CXXCtorInitializer?",
              stmt->getLocStart())
              << stmt->getSourceRange();
        return false;
    }
    if (auto unaryOperator = dyn_cast<UnaryOperator>(parent)) {
        UnaryOperator::Opcode op = unaryOperator->getOpcode();
        if (op == UO_AddrOf || op == UO_PreInc || op == UO_PostInc
            || op == UO_PreDec || op == UO_PostDec) {
            return false;
        }
        if (op == UO_Deref) {
            return checkIfCanBeConst(parent, parmVarDecl);
        }
        return true;
    } else if (auto binaryOp = dyn_cast<BinaryOperator>(parent)) {
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
            return checkIfCanBeConst(parent, parmVarDecl);
        }
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
                return calleeMethodDecl->isConst();
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
    } else if (auto callExpr = dyn_cast<ObjCMessageExpr>(parent)) {
        if (callExpr->getInstanceReceiver() == stmt) {
            return true;
        }
        if (auto const method = callExpr->getMethodDecl()) {
            // TODO could do better
            if (method->isVariadic()) {
                return false;
            }
            assert(method->param_size() == callExpr->getNumArgs());
            for (unsigned i = 0; i < callExpr->getNumArgs(); ++i) {
                if (callExpr->getArg(i) == stmt) {
                    return isPointerOrReferenceToConst(
                        method->param_begin()[i]->getType());
                }
            }
        }
        return false; // TODO ????
    } else if (isa<CXXReinterpretCastExpr>(parent)) {
        return false;
    } else if (isa<CXXConstCastExpr>(parent)) {
        return false;
    } else if (isa<CastExpr>(parent)) { // all other cast expression subtypes
        if (auto e = dyn_cast<ExplicitCastExpr>(parent)) {
            if (loplugin::TypeCheck(e->getTypeAsWritten()).Void()) {
                if (auto const sub = dyn_cast<DeclRefExpr>(
                        e->getSubExpr()->IgnoreParenImpCasts()))
                {
                    if (sub->getDecl() == parmVarDecl) {
                        return false;
                    }
                }
            }
        }
        return checkIfCanBeConst(parent, parmVarDecl);
    } else if (isa<MemberExpr>(parent)) {
        return checkIfCanBeConst(parent, parmVarDecl);
    } else if (isa<ArraySubscriptExpr>(parent)) {
        return checkIfCanBeConst(parent, parmVarDecl);
    } else if (isa<ParenExpr>(parent)) {
        return checkIfCanBeConst(parent, parmVarDecl);
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
    } else if (isa<DoStmt>(parent)) {
        return true;
    } else if (isa<CXXDeleteExpr>(parent)) {
        return false;
    } else if (isa<VAArgExpr>(parent)) {
        return false;
    } else if (isa<CXXDependentScopeMemberExpr>(parent)) {
        return false;
    } else if (isa<MaterializeTemporaryExpr>(parent)) {
        return checkIfCanBeConst(parent, parmVarDecl);
    } else if (auto conditionalExpr = dyn_cast<ConditionalOperator>(parent)) {
        if (conditionalExpr->getCond() == stmt)
            return true;
        return checkIfCanBeConst(parent, parmVarDecl);
    } else if (isa<UnaryExprOrTypeTraitExpr>(parent)) {
        return false; // ???
    } else if (auto cxxNewExpr = dyn_cast<CXXNewExpr>(parent)) {
        for (auto pa : cxxNewExpr->placement_arguments())
            if (pa == stmt)
                return false;
        return true; // because the ParamVarDecl must be a parameter to the expression, probably an array length
    } else if (auto lambdaExpr = dyn_cast<LambdaExpr>(parent)) {
        for (auto it = lambdaExpr->capture_begin(); it != lambdaExpr->capture_end(); ++it)
        {
            if (it->capturesVariable() && it->getCapturedVar() == parmVarDecl)
                return it->getCaptureKind() != LCK_ByRef;
        }
        return false;
    } else if (isa<CXXTypeidExpr>(parent)) {
        return true;
    } else if (isa<ParenListExpr>(parent)) {
        return true;
    } else if (isa<CXXUnresolvedConstructExpr>(parent)) {
        return false;
    } else if (isa<UnresolvedMemberExpr>(parent)) {
        return false;
    } else if (isa<PackExpansionExpr>(parent)) {
        return false;
    } else if (isa<ExprWithCleanups>(parent)) {
        return checkIfCanBeConst(parent, parmVarDecl);
    } else if (isa<CaseStmt>(parent)) {
        return true;
    } else if (isa<CXXPseudoDestructorExpr>(parent)) {
        return false;
    } else if (isa<CXXDependentScopeMemberExpr>(parent)) {
        return false;
    } else if (isa<ObjCIvarRefExpr>(parent)) {
        return checkIfCanBeConst(parent, parmVarDecl);
    }
    parent->dump();
    parmVarDecl->dump();
    report(
         DiagnosticsEngine::Warning,
         "oh dear, what can the matter be?",
          parent->getLocStart())
          << parent->getSourceRange();
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

loplugin::Plugin::Registration< ConstParams > X("constparams", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
