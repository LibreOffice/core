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
#include "functionaddress.hxx"

/**
   Find pointer and reference params that can be declared const.

   This is not a sophisticated analysis. It deliberately skips all of the hard cases for now.
   It is an exercise in getting the most benefit for the least effort.
*/
namespace
{

class ConstParams:
    public loplugin::FunctionAddress<ConstParams>
{
public:
    explicit ConstParams(loplugin::InstantiationData const & data): loplugin::FunctionAddress<ConstParams>(data) {}

    virtual void run() override {
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sal/")
            || fn == SRCDIR "/jurt/source/pipe/staticsalhack.cxx"
            || loplugin::hasPathnamePrefix(fn, SRCDIR "/bridges/")
            || loplugin::hasPathnamePrefix(fn, SRCDIR "/binaryurp/")
            || loplugin::hasPathnamePrefix(fn, SRCDIR "/stoc/")
            || loplugin::hasPathnamePrefix(fn, WORKDIR "/YaccTarget/unoidl/source/sourceprovider-parser.cxx")
            // some weird calling through a function pointer
            || loplugin::hasPathnamePrefix(fn, SRCDIR "/svtools/source/table/defaultinputhandler.cxx")
            || loplugin::hasPathnamePrefix(fn, SRCDIR "/sdext/source/pdfimport/test/pdfunzip.cxx")
            // windows only
            || loplugin::hasPathnamePrefix(fn, SRCDIR "/basic/source/sbx/sbxdec.cxx")
            || loplugin::hasPathnamePrefix(fn, SRCDIR "/sfx2/source/doc/syspath.cxx")
            // ignore this for now
            || loplugin::hasPathnamePrefix(fn, SRCDIR "/libreofficekit")
            // I end up with a
            //    CXXMemberCallExpr
            // to a
            //    BuiltinType '<bound member function type>'
            // and the AST gives me no further useful information.
            || loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/core/doc/docfly.cxx")
            || loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/core/doc/DocumentContentOperationsManager.cxx")
            || loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/core/fields/cellfml.cxx")
            || loplugin::hasPathnamePrefix(fn, SRCDIR "/sw/source/filter/ww8/ww8par6.cxx")
            )
            return;

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        for (const ParmVarDecl *pParmVarDecl : interestingParamSet) {
            auto functionDecl = parmToFunction[pParmVarDecl];
            auto canonicalDecl = functionDecl->getCanonicalDecl();
            if (getFunctionsWithAddressTaken().find(canonicalDecl)
                != getFunctionsWithAddressTaken().end())
            {
                continue;
            }
            std::string fname = functionDecl->getQualifiedNameAsString();
            report(
                DiagnosticsEngine::Warning,
                "this parameter can be const %0",
                compat::getBeginLoc(pParmVarDecl))
                << fname << pParmVarDecl->getSourceRange();
            if (canonicalDecl->getLocation() != functionDecl->getLocation()) {
                unsigned idx = pParmVarDecl->getFunctionScopeIndex();
                const ParmVarDecl* pOther = canonicalDecl->getParamDecl(idx);
                report(
                    DiagnosticsEngine::Note,
                    "canonical parameter declaration here",
                    compat::getBeginLoc(pOther))
                    << pOther->getSourceRange();
            }
            //functionDecl->dump();
        }
    }

    bool TraverseFunctionDecl(FunctionDecl *);
    bool TraverseCXXMethodDecl(CXXMethodDecl * f);
    bool TraverseCXXConstructorDecl(CXXConstructorDecl * f);
    bool VisitDeclRefExpr(const DeclRefExpr *);

private:
    bool CheckTraverseFunctionDecl(FunctionDecl *);
    bool checkIfCanBeConst(const Stmt*, const ParmVarDecl*);
    // integral or enumeration or const * or const &
    bool isOkForParameter(const QualType& qt);
    bool isPointerOrReferenceToNonConst(const QualType& qt);

    std::unordered_set<const ParmVarDecl*> interestingParamSet;
    std::unordered_map<const ParmVarDecl*, const FunctionDecl*> parmToFunction;
    FunctionDecl* currentFunctionDecl = nullptr;
};

bool ConstParams::TraverseFunctionDecl(FunctionDecl * functionDecl)
{
    // We cannot short-circuit the traverse here entirely without breaking the
    // loplugin::FunctionAddress stuff.
    auto prev = currentFunctionDecl;
    if (CheckTraverseFunctionDecl(functionDecl))
        currentFunctionDecl = functionDecl;
    auto rv = loplugin::FunctionAddress<ConstParams>::TraverseFunctionDecl(functionDecl);
    currentFunctionDecl = prev;
    return rv;
}
bool ConstParams::TraverseCXXMethodDecl(CXXMethodDecl * f)
{
    auto prev = currentFunctionDecl;
    if (CheckTraverseFunctionDecl(f))
        currentFunctionDecl = f;
    auto rv = loplugin::FunctionAddress<ConstParams>::TraverseCXXMethodDecl(f);
    currentFunctionDecl = prev;
    return rv;
}
bool ConstParams::TraverseCXXConstructorDecl(CXXConstructorDecl * f)
{
    auto prev = currentFunctionDecl;
    if (CheckTraverseFunctionDecl(f))
        currentFunctionDecl = f;
    auto rv = loplugin::FunctionAddress<ConstParams>::TraverseCXXConstructorDecl(f);
    currentFunctionDecl = prev;
    return rv;
}

bool ConstParams::CheckTraverseFunctionDecl(FunctionDecl * functionDecl)
{
    if (ignoreLocation(functionDecl) || !functionDecl->isThisDeclarationADefinition()) {
        return false;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(functionDecl)) {
        return false;
    }
    if (functionDecl->isDeleted())
        return false;
    // ignore virtual methods
    if (isa<CXXMethodDecl>(functionDecl)
        && dyn_cast<CXXMethodDecl>(functionDecl)->isVirtual() ) {
        return false;
    }
    // ignore C main
    if (functionDecl->isMain()) {
        return false;
    }

    // ignore the macros from include/tools/link.hxx
    auto canonicalDecl = functionDecl->getCanonicalDecl();
    if (compiler.getSourceManager().isMacroBodyExpansion(compat::getBeginLoc(canonicalDecl))
        || compiler.getSourceManager().isMacroArgExpansion(compat::getBeginLoc(canonicalDecl))) {
        StringRef name { Lexer::getImmediateMacroName(
                compat::getBeginLoc(canonicalDecl), compiler.getSourceManager(), compiler.getLangOpts()) };
        if (name.startswith("DECL_LINK") || name.startswith("DECL_STATIC_LINK"))
            return false;
        auto loc2 = compat::getImmediateExpansionRange(compiler.getSourceManager(), compat::getBeginLoc(canonicalDecl)).first;
        if (compiler.getSourceManager().isMacroBodyExpansion(loc2))
        {
            StringRef name2 { Lexer::getImmediateMacroName(
                    loc2, compiler.getSourceManager(), compiler.getLangOpts()) };
            if (name2.startswith("DECL_DLLPRIVATE_LINK"))
                return false;
        }
    }

    if (functionDecl->getIdentifier())
    {
        StringRef name = functionDecl->getName();
        if (   name == "file_write"
            || name == "SalMainPipeExchangeSignal_impl"
            || name.startswith("SbRtl_")
            || name == "GoNext"
            || name == "GoPrevious"
            || name.startswith("Read_F_")
                // UNO component entry points
            || name.endswith("component_getFactory")
            || name == "egiGraphicExport"
            || name == "etiGraphicExport"
            || name == "epsGraphicExport"
            // callback for some external code?
            || name == "ScAddInAsyncCallBack"
            // used as function pointers
            || name == "Read_Footnote"
            || name == "Read_Field"
            || name == "Read_And"
            // passed as a LINK<> to another method
            || name == "GlobalBasicErrorHdl_Impl"
            // template
            || name == "extract_throw" || name == "readProp"
            )
            return false;

    }

    std::string fqn = functionDecl->getQualifiedNameAsString();
    if ( fqn == "connectivity::jdbc::GlobalRef::set"
      || fqn == "(anonymous namespace)::ReorderNotifier::operator()"
      || fqn == "static_txtattr_cast")
        return false;

    // calculate the ones we want to check
    bool foundInterestingParam = false;
    for (const ParmVarDecl *pParmVarDecl : functionDecl->parameters()) {
        // ignore unused params
        if (pParmVarDecl->getName().empty()
            || pParmVarDecl->hasAttr<UnusedAttr>())
            continue;
        auto const type = loplugin::TypeCheck(pParmVarDecl->getType());
        if (!( type.Pointer().NonConst()
             || type.LvalueReference().NonConst()))
            continue;
        // since we normally can't change typedefs, just ignore them
        if (isa<TypedefType>(pParmVarDecl->getType()))
            continue;
        // some typedefs turn into these
        if (isa<DecayedType>(pParmVarDecl->getType()))
            continue;
        // TODO ignore these for now, has some effects I don't understand
        if (type.Pointer().Pointer())
            continue;
        // const is meaningless when applied to function pointer types
        if (pParmVarDecl->getType()->isFunctionPointerType())
            continue;
        interestingParamSet.insert(pParmVarDecl);
        parmToFunction[pParmVarDecl] = functionDecl;
        foundInterestingParam = true;
    }
    return foundInterestingParam;
}

bool ConstParams::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    if (!currentFunctionDecl)
        return true;
    const ParmVarDecl* parmVarDecl = dyn_cast_or_null<ParmVarDecl>(declRefExpr->getDecl());
    if (!parmVarDecl)
        return true;
    if (interestingParamSet.find(parmVarDecl) == interestingParamSet.end())
        return true;
    if (!checkIfCanBeConst(declRefExpr, parmVarDecl))
        interestingParamSet.erase(parmVarDecl);
    return true;
}

// Walk up from a statement that contains a DeclRefExpr, checking if the usage means that the
// related ParamVarDecl can be const.
bool ConstParams::checkIfCanBeConst(const Stmt* stmt, const ParmVarDecl* parmVarDecl)
{
    const Stmt* parent = getParentStmt( stmt );
    if (!parent)
    {
        // check if we're inside a CXXCtorInitializer
        auto parentsRange = compiler.getASTContext().getParents(*stmt);
        if ( parentsRange.begin() != parentsRange.end())
        {
            if (auto cxxConstructorDecl = dyn_cast_or_null<CXXConstructorDecl>(parentsRange.begin()->get<Decl>()))
            {
                for ( auto cxxCtorInitializer : cxxConstructorDecl->inits())
                {
                    if ( cxxCtorInitializer->getInit() == stmt)
                    {
                        if (cxxCtorInitializer->isAnyMemberInitializer())
                        {
                            // if the member is not pointer-to-const or ref-to-const or value, we cannot make the param const
                            auto fieldDecl = cxxCtorInitializer->getAnyMember();
                            auto tc = loplugin::TypeCheck(fieldDecl->getType());
                            if (tc.Pointer() || tc.LvalueReference())
                                return tc.Pointer().Const() || tc.LvalueReference().Const();
                            else
                                return true;
                        }
                        else
                        {
                            // probably base initialiser, but no simple way to look up the relevant constructor decl
                            return false;
                        }
                    }
                }
            }
            if (auto varDecl = dyn_cast_or_null<VarDecl>(parentsRange.begin()->get<Decl>()))
            {
                return isOkForParameter(varDecl->getType());
            }
        }
        parmVarDecl->dump();
        stmt->dump();
        report(
             DiagnosticsEngine::Warning,
             "no parent?",
              compat::getBeginLoc(stmt))
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
        if (binaryOp->getRHS() == stmt && op == BO_Assign) {
            return isOkForParameter(binaryOp->getLHS()->getType());
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
                return isOkForParameter(constructorDecl->getParamDecl(i)->getType());
            }
        }
    } else if (auto operatorCallExpr = dyn_cast<CXXOperatorCallExpr>(parent)) {
        const CXXMethodDecl* calleeMethodDecl = dyn_cast_or_null<CXXMethodDecl>(operatorCallExpr->getDirectCallee());
        if (calleeMethodDecl) {
            // unary operator
            if (calleeMethodDecl->getNumParams() == 0)
                return calleeMethodDecl->isConst();
            // Same logic as CXXOperatorCallExpr::isAssignmentOp(), which our supported clang
            // doesn't have yet.
            auto Opc = operatorCallExpr->getOperator();
            if (Opc == OO_Equal || Opc == OO_StarEqual ||
                Opc == OO_SlashEqual || Opc == OO_PercentEqual ||
                Opc == OO_PlusEqual || Opc == OO_MinusEqual ||
                Opc == OO_LessLessEqual || Opc == OO_GreaterGreaterEqual ||
                Opc == OO_AmpEqual || Opc == OO_CaretEqual ||
                Opc == OO_PipeEqual)
            {
                if (operatorCallExpr->getArg(0) == stmt) // assigning to the param
                    return false;
                // not all operator= take a const&
                return isOkForParameter(calleeMethodDecl->getParamDecl(0)->getType());
            }
            if (operatorCallExpr->getOperator() == OO_Subscript && operatorCallExpr->getArg(1) == stmt)
                return true;
            if (operatorCallExpr->getOperator() == OO_EqualEqual || operatorCallExpr->getOperator() == OO_ExclaimEqual)
                return true;
            // binary operator
            if (operatorCallExpr->getArg(0) == stmt)
                return calleeMethodDecl->isConst();
            unsigned const n = std::min(
                operatorCallExpr->getNumArgs(),
                calleeMethodDecl->getNumParams() + 1);
            for (unsigned i = 1; i < n; ++i)
                if (operatorCallExpr->getArg(i) == stmt) {
                    auto qt = calleeMethodDecl->getParamDecl(i - 1)->getType();
                    return isOkForParameter(qt);
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
                        return isOkForParameter(calleeFunctionDecl->getParamDecl(i)->getType());
                    }
                }
            }
        }
        return false;
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
                    return isOkForParameter(prototype->getParamType(i));
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
                    return isOkForParameter(calleeFunctionDecl->getParamDecl(i)->getType());
                }
            }
        }
        return false;
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
                    return isOkForParameter(
                        method->param_begin()[i]->getType());
                }
            }
        }
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
                    if (sub->getDecl() == parmVarDecl)
                        return false;
                }
            }
        }
        return checkIfCanBeConst(parent, parmVarDecl);
    } else if (isa<MemberExpr>(parent)) {
        return checkIfCanBeConst(parent, parmVarDecl);
    } else if (auto arraySubscriptExpr = dyn_cast<ArraySubscriptExpr>(parent)) {
        if (arraySubscriptExpr->getIdx() == stmt)
            return true;
        return checkIfCanBeConst(parent, parmVarDecl);
    } else if (isa<ParenExpr>(parent)) {
        return checkIfCanBeConst(parent, parmVarDecl);
    } else if (isa<DeclStmt>(parent)) {
        // TODO could do better here, but would require tracking the target(s)
        //return false;
    } else if (isa<ReturnStmt>(parent)) {
        return !isPointerOrReferenceToNonConst(currentFunctionDecl->getReturnType());
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
        for (unsigned i = 0; i < cxxNewExpr->getNumPlacementArgs(); ++i)
            if (cxxNewExpr->getPlacementArg(i) == stmt)
                return false;
        return true; // ???
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
        return false; // could be improved, seen in constructors when calling base class constructor
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
          compat::getBeginLoc(parent))
          << parent->getSourceRange();
    return true;
}

bool ConstParams::isOkForParameter(const QualType& qt) {
    if (qt->isIntegralOrEnumerationType())
        return true;
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

bool ConstParams::isPointerOrReferenceToNonConst(const QualType& qt) {
    auto const type = loplugin::TypeCheck(qt);
    if (type.Pointer()) {
        return !bool(type.Pointer().Const());
    } else if (type.LvalueReference()) {
        return !bool(type.LvalueReference().Const());
    }
    return false;
}

loplugin::Plugin::Registration< ConstParams > X("constparams", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
