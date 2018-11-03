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
#include <unordered_map>
#include <unordered_set>

#include "plugin.hxx"
#include "check.hxx"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/StmtVisitor.h"

/**
  TODO deal with C++ operator overload assign
*/

namespace
{
//static bool startswith(const std::string& rStr, const char* pSubStr) {
//    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
//}
class BuriedAssign : public loplugin::FilteringPlugin<BuriedAssign>
{
public:
    explicit BuriedAssign(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override
    {
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);
        // getParentStmt appears not to be working very well here
        if (fn == SRCDIR "/stoc/source/inspect/introspection.cxx"
            || fn == SRCDIR "/stoc/source/corereflection/criface.cxx")
            return;
        // calling an acquire via function pointer
        if (fn == SRCDIR "/cppu/source/uno/lbenv.cxx"
            || fn == SRCDIR "cppu/source/typelib/static_types.cxx")
            return;
        // false+, not sure why
        if (fn == SRCDIR "/vcl/source/window/menu.cxx")
            return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitBinaryOperator(BinaryOperator const*);
    bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr const*);

private:
    void checkExpr(Expr const*);
};

static bool isAssignmentOp(clang::BinaryOperatorKind op)
{
    // We ignore BO_ShrAssign i.e. >>= because we use that everywhere for
    // extracting data from css::uno::Any
    return op == BO_Assign || op == BO_MulAssign || op == BO_DivAssign || op == BO_RemAssign
           || op == BO_AddAssign || op == BO_SubAssign || op == BO_ShlAssign || op == BO_AndAssign
           || op == BO_XorAssign || op == BO_OrAssign;
}

static bool isAssignmentOp(clang::OverloadedOperatorKind Opc)
{
    // Same logic as CXXOperatorCallExpr::isAssignmentOp(), which our supported clang
    // doesn't have yet.
    // Except that we ignore OO_GreaterGreaterEqual i.e. >>= because we use that everywhere for
    // extracting data from css::uno::Any
    return Opc == OO_Equal || Opc == OO_StarEqual || Opc == OO_SlashEqual || Opc == OO_PercentEqual
           || Opc == OO_PlusEqual || Opc == OO_MinusEqual || Opc == OO_LessLessEqual
           || Opc == OO_AmpEqual || Opc == OO_CaretEqual || Opc == OO_PipeEqual;
}

bool BuriedAssign::VisitBinaryOperator(BinaryOperator const* binaryOp)
{
    if (ignoreLocation(binaryOp))
        return true;

    if (!isAssignmentOp(binaryOp->getOpcode()))
        return true;

    checkExpr(binaryOp);
    return true;
}

bool BuriedAssign::VisitCXXOperatorCallExpr(CXXOperatorCallExpr const* cxxOper)
{
    if (ignoreLocation(cxxOper))
        return true;
    if (!isAssignmentOp(cxxOper->getOperator()))
        return true;
    checkExpr(cxxOper);
    return true;
}

void BuriedAssign::checkExpr(Expr const* binaryOp)
{
    if (compiler.getSourceManager().isMacroBodyExpansion(compat::getBeginLoc(binaryOp)))
        return;
    if (compiler.getSourceManager().isMacroArgExpansion(compat::getBeginLoc(binaryOp)))
        return;

    /**
    Note: I tried writing this plugin without getParentStmt, but in order to make that work, I had to
    hack things like TraverseWhileStmt to call TraverseStmt on the child nodes myself, so I could know whether I was inside the body or the condition.
    But I could not get that to work, so switched to this approach.
    */

    // look up past the temporary nodes
    Stmt const* child = binaryOp;
    Stmt const* parent = getParentStmt(binaryOp);
    while (true)
    {
        // This part is not ideal, but getParentStmt() appears to fail us in some cases, notably when the assignment
        // is inside a decl like:
        //     int x = a = 1;
        if (!parent)
            return;
        if (!(isa<MaterializeTemporaryExpr>(parent) || isa<CXXBindTemporaryExpr>(parent)
              || isa<ImplicitCastExpr>(parent) || isa<CXXConstructExpr>(parent)
              || isa<ParenExpr>(parent) || isa<ExprWithCleanups>(parent)))
            break;
        child = parent;
        parent = getParentStmt(parent);
    }

    if (isa<CompoundStmt>(parent))
        return;
    // ignore chained assignment like "a = b = 1;"
    if (auto cxxOper = dyn_cast<CXXOperatorCallExpr>(parent))
    {
        if (cxxOper->getOperator() == OO_Equal)
            return;
    }
    // ignore chained assignment like "a = b = 1;"
    if (auto parentBinOp = dyn_cast<BinaryOperator>(parent))
    {
        if (parentBinOp->getOpcode() == BO_Assign)
            return;
    }
    // ignore chained assignment like "int a = b = 1;"
    if (isa<DeclStmt>(parent))
        return;

    if (isa<CaseStmt>(parent) || isa<DefaultStmt>(parent) || isa<LabelStmt>(parent)
        || isa<ForStmt>(parent) || isa<CXXForRangeStmt>(parent) || isa<IfStmt>(parent)
        || isa<DoStmt>(parent) || isa<WhileStmt>(parent) || isa<ReturnStmt>(parent))
        return;

    // now check for the statements where we don't care at all if we see a buried assignment
    while (true)
    {
        if (isa<CompoundStmt>(parent))
            break;
        if (isa<CaseStmt>(parent) || isa<DefaultStmt>(parent) || isa<LabelStmt>(parent))
            return;
        // Ignore assign in these statements, just seems to be part of the "natural" idiom of C/C++
        // TODO: perhaps include ReturnStmt?
        if (auto forStmt = dyn_cast<ForStmt>(parent))
        {
            if (child == forStmt->getBody())
                break;
            return;
        }
        if (auto forRangeStmt = dyn_cast<CXXForRangeStmt>(parent))
        {
            if (child == forRangeStmt->getBody())
                break;
            return;
        }
        if (auto ifStmt = dyn_cast<IfStmt>(parent))
        {
            if (child == ifStmt->getCond())
                return;
        }
        if (auto doStmt = dyn_cast<DoStmt>(parent))
        {
            if (child == doStmt->getCond())
                return;
        }
        if (auto whileStmt = dyn_cast<WhileStmt>(parent))
        {
            if (child == whileStmt->getBody())
                break;
            return;
        }
        if (isa<ReturnStmt>(parent))
            return;
        // This appears to be a valid way of making it obvious that we need to call acquire when assigning such ref-counted
        // stuff e.g.
        //     rtl_uString_acquire( a = b );
        if (auto callExpr = dyn_cast<CallExpr>(parent))
        {
            if (callExpr->getDirectCallee() && callExpr->getDirectCallee()->getIdentifier())
            {
                auto name = callExpr->getDirectCallee()->getName();
                if (name == "rtl_uString_acquire" || name == "_acquire"
                    || name == "typelib_typedescriptionreference_acquire")
                    return;
            }
        }
        child = parent;
        parent = getParentStmt(parent);
        if (!parent)
            break;
    }

    report(DiagnosticsEngine::Warning, "buried assignment, very hard to read",
           compat::getBeginLoc(binaryOp))
        << binaryOp->getSourceRange();
}

// off by default because it uses getParentStmt so it's more expensive to run
loplugin::Plugin::Registration<BuriedAssign> X("buriedassign", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
