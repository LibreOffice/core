/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#include <config_global.h>
#include "plugin.hxx"
#include "compat.hxx"
#include "check.hxx"
#include <unordered_set>
#include <unordered_map>

namespace loplugin
{
/*
  This is a compile check. The results of this plugin need to be checked by hand, since it is a collection of heuristics.

  Check for unused variable where
  (*) we never call methods that return information from the variable.
  (*) we never pass the variable to anything else

  Classes which are safe to be warned about need to be marked using
  SAL_WARN_UNUSED (see e.g. OUString). For external classes such as std::vector
  that cannot be edited there is a manual list.

  This is an expensive plugin, since it walks up the parent tree,
  so it is off by default.
*/

class UnusedVariableMore : public loplugin::FilteringPlugin<UnusedVariableMore>
{
public:
    explicit UnusedVariableMore(const InstantiationData& data);
    virtual void run() override;
    bool VisitVarDecl(VarDecl const*);
    bool VisitDeclRefExpr(DeclRefExpr const*);
    bool VisitFunctionDecl(FunctionDecl const*);

private:
    bool checkifUnused(Stmt const*, VarDecl const*);
    bool isOkForParameter(const QualType&);

    std::unordered_set<VarDecl const*> interestingSet;
    // used to dump the last place that said the usage was unused, for debug purposes
    std::unordered_map<VarDecl const*, Stmt const*> interestingDebugMap;
};

UnusedVariableMore::UnusedVariableMore(const InstantiationData& data)
    : FilteringPlugin(data)
{
}

void UnusedVariableMore::run()
{
    std::string fn(handler.getMainFileName());
    loplugin::normalizeDotDotInFilePath(fn);

    // ignore QA folders
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sal/qa/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/i18npool/qa/"))
        return;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sc/qa/"))
        return;

    // vector of shared_ptr used to delay destruction
    if (fn == SRCDIR "/cppuhelper/source/servicemanager.cxx")
        return;
    if (fn == SRCDIR "/i18nlangtag/source/languagetag/languagetag.cxx")
        return;
    if (fn == SRCDIR "/vcl/workben/outdevgrind.cxx")
        return;
    // unordered_set of Reference to delay destruction
    if (fn == SRCDIR "/stoc/source/servicemanager/servicemanager.cxx")
        return;
    // TODO "operator >>" fooling me here
    if (fn == SRCDIR "/editeng/source/accessibility/AccessibleEditableTextPara.cxx")
        return;
    // some weird stuff
    if (fn == SRCDIR "/sfx2/source/dialog/dinfdlg.cxx")
        return;
    // std::vector< Reference< XInterface > > keep alive
    if (fn == SRCDIR "/dbaccess/source/core/dataaccess/databasedocument.cxx")
        return;
    // template magic
    if (fn == SRCDIR "/sc/source/core/tool/scmatrix.cxx")
        return;
    // storing local copy of Link<>
    if (fn == SRCDIR "/sc/source/ui/miscdlgs/simpref.cxx")
        return;
    // Using an SwPaM to do stuff
    if (fn == SRCDIR "/sw/source/core/crsr/bookmrk.cxx")
        return;
    // index variable in for loop?
    if (fn == SRCDIR "/sw/source/uibase/docvw/edtwin.cxx")
        return;
    // TODO "operator >>" fooling me here
    if (fn == SRCDIR "/sw/source/filter/ww8/ww8par.cxx")
        return;
    // TODO "operator >>" fooling me here
    if (fn == SRCDIR "/sc/source/filter/excel/xistream.cxx")
        return;

    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

    for (VarDecl const* varDecl : interestingSet)
    {
        report(DiagnosticsEngine::Warning, "unused variable %0", varDecl->getLocation())
            << varDecl->getDeclName();
        //auto it = interestingDebugMap.find(varDecl);
        //if (it != interestingDebugMap.end())
        //    it->second->dump();
    }
}

bool isWarnUnusedType(QualType type)
{
    if (auto const t = type->getAs<TypedefType>())
    {
        if (t->getDecl()->hasAttr<WarnUnusedAttr>())
        {
            return true;
        }
    }
    if (auto const t = type->getAs<RecordType>())
    {
        if (t->getDecl()->hasAttr<WarnUnusedAttr>())
        {
            return true;
        }
    }
    return loplugin::isExtraWarnUnusedType(type);
}

bool UnusedVariableMore::VisitVarDecl(VarDecl const* var)
{
    if (ignoreLocation(var))
        return true;
    if (var->isDefinedOutsideFunctionOrMethod())
        return true;
    if (isa<ParmVarDecl>(var))
        return true;
    if (!isWarnUnusedType(var->getType()))
        return true;

    // some false +
    auto dc = loplugin::TypeCheck(var->getType());
    if (dc.Class("ZCodec").GlobalNamespace())
        return true;
    if (dc.Class("ScopedVclPtrInstance").GlobalNamespace())
        return true;
    if (dc.Class("VclPtrInstance").GlobalNamespace())
        return true;
    if (dc.Class("Config").GlobalNamespace())
        return true;
    // I think these classes modify global state somehow
    if (dc.Class("SvtHistoryOptions").GlobalNamespace())
        return true;
    if (dc.Class("SvtSecurityOptions").GlobalNamespace())
        return true;
    if (dc.Class("SvtViewOptions").GlobalNamespace())
        return true;
    if (dc.Class("SvtUserOptions").GlobalNamespace())
        return true;
    if (dc.Class("SvtFontOptions").GlobalNamespace())
        return true;
    if (dc.Class("SvtMenuOptions").GlobalNamespace())
        return true;
    if (dc.Class("SvtPathOptions").GlobalNamespace())
        return true;
    if (dc.Class("SvtPrintWarningOptions").GlobalNamespace())
        return true;
    if (dc.Class("SvtSysLocaleOptions").GlobalNamespace())
        return true;

    interestingSet.insert(var);
    return true;
}

bool UnusedVariableMore::VisitDeclRefExpr(DeclRefExpr const* declRefExpr)
{
    if (ignoreLocation(declRefExpr))
        return true;
    auto varDecl = dyn_cast<VarDecl>(declRefExpr->getDecl());
    if (!varDecl)
        return true;
    if (interestingSet.find(varDecl) == interestingSet.end())
        return true;
    if (!checkifUnused(declRefExpr, varDecl))
        interestingSet.erase(varDecl);
    return true;
}

// Walk up from a statement that contains a DeclRefExpr, checking if the usage means that the
// related VarDecl is unused.
bool UnusedVariableMore::checkifUnused(Stmt const* stmt, VarDecl const* varDecl)
{
    const Stmt* parent = getParentStmt(stmt);
    if (!parent)
    {
        // check if we're inside a CXXCtorInitializer
        auto parentsRange = compiler.getASTContext().getParents(*stmt);
        if (parentsRange.begin() != parentsRange.end())
        {
            auto parentDecl = parentsRange.begin()->get<Decl>();
            if (parentDecl && (isa<CXXConstructorDecl>(parentDecl) || isa<VarDecl>(parentDecl)))
                return false;
        }
        interestingDebugMap[varDecl] = stmt;
        return true;
    }

    if (isa<ReturnStmt>(parent))
        return false;
    if (isa<IfStmt>(parent))
        return false;
    if (isa<SwitchStmt>(parent))
        return false;
    if (isa<InitListExpr>(parent))
        return false;
    if (isa<CXXConstructExpr>(parent))
        return false;
    if (isa<BinaryOperator>(parent))
        return false;
    if (isa<UnaryOperator>(parent))
        return false;
    if (isa<ConditionalOperator>(parent))
        return false;
    if (isa<ArraySubscriptExpr>(parent))
        return false;
    if (isa<CXXBindTemporaryExpr>(parent))
        return checkifUnused(parent, varDecl);
    if (isa<MaterializeTemporaryExpr>(parent))
        return checkifUnused(parent, varDecl);

    if (isa<CompoundStmt>(parent))
    {
        interestingDebugMap[varDecl] = parent;
        return true;
    }

    // check for cast to void
    if (auto explicitCastExpr = dyn_cast<ExplicitCastExpr>(parent))
    {
        if (loplugin::TypeCheck(explicitCastExpr->getTypeAsWritten()).Void())
            return false;
    }

    if (isa<MemberExpr>(parent))
        return checkifUnused(parent, varDecl);
    if (isa<ExprWithCleanups>(parent))
        return checkifUnused(parent, varDecl);
    if (isa<CastExpr>(parent))
        return checkifUnused(parent, varDecl);
    if (isa<ParenExpr>(parent))
        return checkifUnused(parent, varDecl);

    if (auto operatorCallExpr = dyn_cast<CXXOperatorCallExpr>(parent))
    {
        const CXXMethodDecl* calleeMethodDecl
            = dyn_cast_or_null<CXXMethodDecl>(operatorCallExpr->getDirectCallee());
        if (calleeMethodDecl)
        {
            if (calleeMethodDecl->getNumParams() == 0)
                return checkifUnused(parent, varDecl);
            if (operatorCallExpr->getArg(0) == stmt)
                return checkifUnused(parent, varDecl);
        }
    }
    if (auto memberCallExpr = dyn_cast<CXXMemberCallExpr>(parent))
    {
        const MemberExpr* memberExpr = dyn_cast<MemberExpr>(stmt);
        if (memberExpr && memberCallExpr->getImplicitObjectArgument() == memberExpr->getBase())
        {
            // if we are calling a method on the varDecl, walk up
            if (!checkifUnused(parent, varDecl))
                return false;
            // check if we are passing something to the var by non-const ref, in which case it is updating something else for us
            const FunctionDecl* calleeFunctionDecl = memberCallExpr->getDirectCallee();
            if (calleeFunctionDecl)
            {
                if (calleeFunctionDecl->isVariadic())
                    return false;
                for (unsigned i = 0; i < memberCallExpr->getNumArgs(); ++i)
                {
                    if (i >= calleeFunctionDecl->getNumParams()) // can happen in template code
                    {
                        interestingDebugMap[varDecl] = parent;
                        return true;
                    }
                    if (!isOkForParameter(calleeFunctionDecl->getParamDecl(i)->getType()))
                        return false;
                }
            }
            interestingDebugMap[varDecl] = parent;
            return true;
        }
    }
    if (isa<CallExpr>(parent))
        return false;

    interestingDebugMap[varDecl] = parent;
    return true;
}

bool UnusedVariableMore::isOkForParameter(const QualType& qt)
{
    if (qt->isIntegralOrEnumerationType())
        return true;
    auto const type = loplugin::TypeCheck(qt);
    if (type.Pointer())
    {
        return bool(type.Pointer().Const());
    }
    else if (type.LvalueReference())
    {
        return bool(type.LvalueReference().Const());
    }
    return false;
}

bool UnusedVariableMore::VisitFunctionDecl(FunctionDecl const* functionDecl)
{
    if (ignoreLocation(functionDecl))
        return true;
    //functionDecl->dump();
    return true;
}

static Plugin::Registration<UnusedVariableMore> X("unusedvariablemore", false);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
