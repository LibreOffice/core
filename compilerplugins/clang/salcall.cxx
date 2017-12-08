/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"
#include "check.hxx"

#include <algorithm>
#include <set>
#include <utility>
#include <vector>

// The SAL_CALL function annotation is only necessary on our outward
// facing C++ ABI, anywhere else it is just cargo-cult.
//

//TODO: To find inconsistencies like
//
//  template<typename> struct S { void f(); }; // #1
//  template<typename T> void S<T>::f() {} // #2
//  template void SAL_CALL S<void>::f();
//
// VisitFunctionDecl would need to also visit explicit instantiations, by letting
// shouldVisitTemplateInstantiations return true and returning from VisitFunctionDecl early iff
// decl->getTemplateSpecializationKind() == TSK_ImplicitInstantiation.  However, an instantiatied
// FunctionDecl is created in TemplateDeclInstantiator::VisitCXXMethodDecl by copying information
// (including source locations) from the declaration at #1, and later modified in
// Sema::InstantiateFunctionDefinition with some source location information from the definition at
// #2.  That means that the source scanning in isSalCallFunction below would be thoroughly confused
// and break.  (This happens for both explicit and implicit template instantiations, which is the
// reason why calls to isSalCallFunction make sure to not call it with any FunctionDecls
// representing such template instantiations.)

namespace
{
//static bool startswith(const std::string& rStr, const char* pSubStr)
//{
//    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
//}

CXXMethodDecl const* getTemplateInstantiationPattern(CXXMethodDecl const* decl)
{
    auto const p = decl->getTemplateInstantiationPattern();
    return p == nullptr ? decl : cast<CXXMethodDecl>(p);
}

class SalCall final : public RecursiveASTVisitor<SalCall>, public loplugin::RewritePlugin
{
public:
    explicit SalCall(loplugin::InstantiationData const& data)
        : RewritePlugin(data)
    {
    }

    virtual void run() override
    {
        m_phase = PluginPhase::FindAddressOf;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        m_phase = PluginPhase::Warning;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitFunctionDecl(FunctionDecl const*);
    bool VisitUnaryAddrOf(UnaryOperator const*);
    bool VisitInitListExpr(InitListExpr const*);
    bool VisitCallExpr(CallExpr const*);
    bool VisitBinAssign(BinaryOperator const*);
    bool VisitCXXConstructExpr(CXXConstructExpr const*);

private:
    void checkForFunctionDecl(Expr const*, bool bCheckOnly = false);
    bool rewrite(SourceLocation);
    bool checkOverlap(SourceRange);
    bool isSalCallFunction(FunctionDecl const* functionDecl, SourceLocation* pLoc = nullptr);

    std::set<FunctionDecl const*> m_addressOfSet;
    enum class PluginPhase
    {
        FindAddressOf,
        Warning
    };
    PluginPhase m_phase;
    std::vector<std::pair<char const*, char const*>> mvModifiedRanges;
};

bool SalCall::VisitUnaryAddrOf(UnaryOperator const* op)
{
    if (m_phase != PluginPhase::FindAddressOf)
        return true;
    checkForFunctionDecl(op->getSubExpr());
    return true;
}

bool SalCall::VisitBinAssign(BinaryOperator const* binaryOperator)
{
    if (m_phase != PluginPhase::FindAddressOf)
        return true;
    checkForFunctionDecl(binaryOperator->getRHS());
    return true;
}

bool SalCall::VisitCallExpr(CallExpr const* callExpr)
{
    if (m_phase != PluginPhase::FindAddressOf)
        return true;
    for (auto arg : callExpr->arguments())
        checkForFunctionDecl(arg);
    return true;
}

bool SalCall::VisitCXXConstructExpr(CXXConstructExpr const* constructExpr)
{
    if (m_phase != PluginPhase::FindAddressOf)
        return true;
    for (auto arg : constructExpr->arguments())
        checkForFunctionDecl(arg);
    return true;
}

bool SalCall::VisitInitListExpr(InitListExpr const* initListExpr)
{
    if (m_phase != PluginPhase::FindAddressOf)
        return true;
    for (auto subStmt : *initListExpr)
        checkForFunctionDecl(dyn_cast<Expr>(subStmt));
    return true;
}

void SalCall::checkForFunctionDecl(Expr const* expr, bool bCheckOnly)
{
    auto e1 = expr->IgnoreParenCasts();
    auto declRef = dyn_cast<DeclRefExpr>(e1);
    if (!declRef)
        return;
    auto functionDecl = dyn_cast<FunctionDecl>(declRef->getDecl());
    if (!functionDecl)
        return;
    if (bCheckOnly)
        getParentStmt(expr)->dump();
    else
        m_addressOfSet.insert(functionDecl->getCanonicalDecl());
}

bool SalCall::VisitFunctionDecl(FunctionDecl const* decl)
{
    if (m_phase != PluginPhase::Warning)
        return true;
    if (ignoreLocation(decl))
        return true;

    // ignore template stuff
    if (decl->getTemplatedKind() != clang::FunctionDecl::TK_NonTemplate)
        return true;
    auto recordDecl = dyn_cast<CXXRecordDecl>(decl->getDeclContext());
    if (recordDecl
        && (recordDecl->getTemplateSpecializationKind() != TSK_Undeclared
            || recordDecl->isDependentContext()))
    {
        return true;
    }

    auto canonicalDecl = decl->getCanonicalDecl();

    // ignore UNO implementations
    if (isInUnoIncludeFile(
            compiler.getSourceManager().getSpellingLoc(canonicalDecl->getLocation())))
        return true;

    // macros make getCharacterData() extremely unreliable
    if (compiler.getSourceManager().isMacroArgExpansion(decl->getLocation())
        || compiler.getSourceManager().isMacroBodyExpansion(decl->getLocation()))
        return true;

    SourceLocation rewriteLoc;
    SourceLocation rewriteCanonicalLoc;
    bool bDeclIsSalCall = isSalCallFunction(decl, &rewriteLoc);
    bool bCanonicalDeclIsSalCall = isSalCallFunction(canonicalDecl, &rewriteCanonicalLoc);

    // first, check for consistency, so we don't trip ourselves up on Linux, where we normally run the plugin
    if (canonicalDecl != decl)
    {
        if (bCanonicalDeclIsSalCall)
            ; // this is fine, the actual definition have or not have SAL_CALL, and MSVC is fine with it
        else if (bDeclIsSalCall)
        {
            // not fine
            report(DiagnosticsEngine::Warning, "SAL_CALL inconsistency",
                   canonicalDecl->getLocation())
                << canonicalDecl->getSourceRange();
            report(DiagnosticsEngine::Note, "SAL_CALL inconsistency", decl->getLocation())
                << decl->getSourceRange();
            return true;
        }
    }
    auto methodDecl = dyn_cast<CXXMethodDecl>(canonicalDecl);
    if (methodDecl)
    {
        for (auto iter = methodDecl->begin_overridden_methods();
             iter != methodDecl->end_overridden_methods(); ++iter)
        {
            const CXXMethodDecl* overriddenMethod
                = getTemplateInstantiationPattern(*iter)->getCanonicalDecl();
            if (bCanonicalDeclIsSalCall != isSalCallFunction(overriddenMethod))
            {
                report(DiagnosticsEngine::Warning, "SAL_CALL inconsistency",
                       methodDecl->getLocation())
                    << methodDecl->getSourceRange();
                report(DiagnosticsEngine::Note, "SAL_CALL inconsistency",
                       overriddenMethod->getLocation())
                    << overriddenMethod->getSourceRange();
                return true;
            }
        }
    }

    if (!bCanonicalDeclIsSalCall)
        return true;

    // @TODO For now, I am ignore free functions, since those are most likely to have their address taken.
    // I'll do them later. They are harder to verify since MSVC does not verify when assigning to function pointers
    // that the calling convention of the function matches the calling convention of the function pointer!
    if (!methodDecl)
        return true;

    // can only check when we have a definition since this is the most likely time
    // when the address of the method will be taken
    if (!(methodDecl && methodDecl->isPure()) && !decl->isThisDeclarationADefinition())
        return true;
    if (m_addressOfSet.find(decl->getCanonicalDecl()) != m_addressOfSet.end())
        return true;

    // ignore extern "C" UNO factory constructor functions
    if (decl->isExternC())
    {
        if (loplugin::TypeCheck(decl->getReturnType())
                .Pointer()
                .Class("XInterface")
                .Namespace("uno")
                .Namespace("star")
                .Namespace("sun")
                .Namespace("com")
                .GlobalNamespace())
            return true;
        if (loplugin::TypeCheck(decl->getReturnType()).Pointer().Void())
            return true;
    }

    // some base classes are overridden by sub-classes which override both the base-class and an UNO class
    if (recordDecl)
    {
        auto dc = loplugin::DeclCheck(recordDecl);
        if (dc.Class("OProxyAggregation").Namespace("comphelper").GlobalNamespace()
            || dc.Class("OComponentProxyAggregationHelper")
                   .Namespace("comphelper")
                   .GlobalNamespace()
            || dc.Class("SvxShapeMaster").GlobalNamespace()
            || dc.Class("ListBoxAccessibleBase").Namespace("accessibility").GlobalNamespace()
            || dc.Class("AsyncEventNotifierBase").Namespace("comphelper").GlobalNamespace()
            || dc.Class("ODescriptor")
                   .Namespace("sdbcx")
                   .Namespace("connectivity")
                   .GlobalNamespace()
            || dc.Class("IController").Namespace("dbaui").GlobalNamespace()
            || dc.Class("ORowSetBase").Namespace("dbaccess").GlobalNamespace()
            || dc.Class("OComponentAdapterBase").Namespace("bib").GlobalNamespace()
            || dc.Class("IEventProcessor").Namespace("comphelper").GlobalNamespace()
            || dc.Class("SvxUnoTextBase").GlobalNamespace()
            || dc.Class("OInterfaceContainer").Namespace("frm").GlobalNamespace()
            || dc.Class("AccessibleComponentBase").Namespace("accessibility").GlobalNamespace()
            || dc.Class("ContextHandler2Helper")
                   .Namespace("core")
                   .Namespace("oox")
                   .GlobalNamespace()
            || dc.Class("AccessibleStaticTextBase").Namespace("accessibility").GlobalNamespace()
            || dc.Class("OCommonPicker").Namespace("svt").GlobalNamespace()
            || dc.Class("VbaDocumentBase").GlobalNamespace()
            || dc.Class("VbaPageSetupBase").GlobalNamespace()
            || dc.Class("ScVbaControl").GlobalNamespace()

        )
            return true;
    }

    // if any of the overridden methods are SAL_CALL, we should be too
    if (methodDecl)
    {
        for (auto iter = methodDecl->begin_overridden_methods();
             iter != methodDecl->end_overridden_methods(); ++iter)
        {
            const CXXMethodDecl* overriddenMethod
                = getTemplateInstantiationPattern(*iter)->getCanonicalDecl();
            if (isSalCallFunction(overriddenMethod))
                return true;
        }
    }

    // these often have their address taken
    if (methodDecl && methodDecl->getIdentifier())
    {
        auto name = methodDecl->getName();
        if (name == "getImplementationName_static" || name == "getSupportedServiceNames_static"
            || name == "getSupportedServiceNames_Static" || name == "Create" || name == "create"
            || name == "CreateInstance" || name == "getImplementationName_Static"
            || name == "getSingletonName_static" || name == "getLdapUserProfileBeName"
            || name == "getLdapUserProfileBeServiceNames" || name == "impl_staticCreateSelfInstance"
            || name == "impl_createInstance" || name == "impl_staticGetImplementationName"
            || name == "impl_staticGetSupportedServiceNames"
            || name == "impl_getStaticSupportedServiceNames"
            || name == "impl_getStaticImplementationName" || name == "getBackendName"
            || name == "getBackendServiceNames")
            return true;
    }

    bool bOK = rewrite(rewriteLoc);
    if (bOK && canonicalDecl != decl)
    {
        bOK = rewrite(rewriteCanonicalLoc);
    }
    if (bOK)
        return true;

    report(DiagnosticsEngine::Warning, "SAL_CALL unnecessary here", rewriteLoc)
        << decl->getSourceRange();
    if (canonicalDecl != decl)
        report(DiagnosticsEngine::Warning, "SAL_CALL unnecessary here", rewriteCanonicalLoc)
            << canonicalDecl->getSourceRange();

    return true;
}

bool SalCall::isSalCallFunction(FunctionDecl const* functionDecl, SourceLocation* pLoc)
{
    assert(!functionDecl->isTemplateInstantiation());

    //TODO:  It appears that FunctionDecls representing explicit template specializations have the
    // same issue as those representing (implicit or explicit) instantiations, namely that their
    // data (including relevant source locations) is an incoherent combination of data from the
    // original template declaration and the later specialization definition.  For example, for the
    // OValueLimitedType<double>::registerProperties specialization at
    // forms/source/xforms/datatyperepository.cxx:241, the FunctionDecl (which is even considered
    // canonic) representing the base-class function overridden by ODecimalType::registerProperties
    // (forms/source/xforms/datatypes.hxx:299) is dumped as
    //
    //  CXXMethodDecl <forms/source/xforms/datatypes.hxx:217:9, col:54>
    //   forms/source/xforms/datatyperepository.cxx:242:37 registerProperties 'void (void)' virtual
    //
    // mixing the source range ("datatypes.hxx:217:9, col:54") from the original declaration with
    // the name location ("datatyperepository.cxx:242:37") from the explicit specialization.  Just
    // give up for now and assume no "SAL_CALL" is present:
    if (functionDecl->getTemplateSpecializationKind() == TSK_ExplicitSpecialization)
    {
        return false;
    }

    SourceManager& SM = compiler.getSourceManager();

    // Stop searching for "SAL_CALL" at the start of the function declaration's name (for qualified
    // names this will point after the qualifiers, but needlessly including those in the search
    // should be harmless):
    SourceLocation endLoc = functionDecl->getNameInfo().getLocStart();
    while (endLoc.isMacroID() && SM.isAtStartOfImmediateMacroExpansion(endLoc))
    {
        endLoc = SM.getImmediateMacroCallerLoc(endLoc);
    }
    endLoc = SM.getSpellingLoc(endLoc);

    SourceLocation startLoc;
    bool noReturnType = isa<CXXConstructorDecl>(functionDecl)
                        || isa<CXXDestructorDecl>(functionDecl)
                        || isa<CXXConversionDecl>(functionDecl);
    bool startAfterReturnType = !noReturnType;
    if (startAfterReturnType)
    {
        // For functions that do have a return type, start searching for "SAL_CALL" after the return
        // type (which for SAL_CALL functions on Windows will be an AttributedTypeLoc, which the
        // implementation of FunctionDecl::getReturnTypeSourceRange does not take into account, so
        // do that here explicitly):
        auto const TSI = functionDecl->getTypeSourceInfo();
        if (TSI == nullptr)
        {
            return false;
        }
        auto TL = TSI->getTypeLoc().IgnoreParens();
        if (auto ATL = TL.getAs<AttributedTypeLoc>())
        {
            TL = ATL.getModifiedLoc();
        }
        auto const FTL = TL.getAs<FunctionTypeLoc>();
        if (!FTL)
        {
            // Happens when a function declaration uses a typedef for the function type, as in
            //
            //  SAL_JNI_EXPORT javaunohelper::detail::Func_bootstrap
            //  Java_com_sun_star_comp_helper_Bootstrap_cppuhelper_1bootstrap;
            //
            // in javaunohelper/source/juhx-export-functions.hxx.
            //TODO: check the typedef for mention of "SAL_CALL" (and also check for usage of such
            // typedefs in the !startAfterReturnType case below)
            return false;
        }
        startLoc = FTL.getReturnLoc().getEndLoc();
        auto const slEnd = Lexer::getLocForEndOfToken(startLoc, 0, SM, compiler.getLangOpts());
        if (slEnd.isValid()) // i.e., startLoc either non-macro, or at end of macro
        {
            startLoc = slEnd;
        }
        else // otherwise, resolve into macro text
        {
            startLoc = Lexer::getLocForEndOfToken(SM.getSpellingLoc(startLoc), 0, SM,
                                                  compiler.getLangOpts());
        }
        while (startLoc.isMacroID() && SM.isAtEndOfImmediateMacroExpansion(startLoc, &startLoc))
        {
        }
        startLoc = SM.getSpellingLoc(startLoc);

        if (startLoc.isValid() && endLoc.isValid() && startLoc != endLoc
            && !SM.isBeforeInTranslationUnit(startLoc, endLoc))
        {
            // Happens for uses of trailing return type (in which case starting instead at the start
            // of the function declaration should be fine), but also for cases like
            //
            //  void (*f())();
            //
            // where the function name is within the function type (TODO: in which case starting at
            // the start can erroneously pick up the "SAL_CALL" from the returned pointer-to-
            // function type in cases like
            //
            //  void SAL_CALL (*f())();
            //
            // that are hopefully rare):
            startAfterReturnType = false;
        }
    }
    if (!startAfterReturnType)
    {
        // Ctors/dtors/conversion functions don't have a return type, start searching for "SAL_CALL"
        // at the start of the function declaration:
        startLoc = functionDecl->getSourceRange().getBegin();
        while (startLoc.isMacroID() && SM.isAtStartOfImmediateMacroExpansion(startLoc))
        {
            startLoc = SM.getImmediateMacroCallerLoc(startLoc);
        }
        startLoc = SM.getSpellingLoc(startLoc);
#if !defined _WIN32
        // When the SAL_CALL macro expands to nothing, it may even precede the function
        // declaration's source range, so go back one token (unless the declaration is known to
        // start with a token that must precede a possible "SAL_CALL", like "virtual" or
        // "explicit"):
        //TODO: this will produce false positives if the declaration is immediately preceded by a
        // macro definition whose replacement text ends in "SAL_CALL"
        if (noReturnType
            && !(functionDecl->isVirtualAsWritten()
                 || (isa<CXXConstructorDecl>(functionDecl)
                     && cast<CXXConstructorDecl>(functionDecl)->isExplicitSpecified())
                 || (isa<CXXConversionDecl>(functionDecl)
                     && cast<CXXConversionDecl>(functionDecl)->isExplicitSpecified())))
        {
            for (;;)
            {
                startLoc = Lexer::GetBeginningOfToken(startLoc.getLocWithOffset(-1), SM,
                                                      compiler.getLangOpts());
                auto const s
                    = StringRef(SM.getCharacterData(startLoc),
                                Lexer::MeasureTokenLength(startLoc, SM, compiler.getLangOpts()));
                // When looking backward at least through a function-like macro replacement like
                //
                // | foo\         |
                // |    barbaz##X    |
                //
                // starting at "barbaz" in the secod line, the next token reported will start at "\"
                // in the first line and include the intervening spaces and (part of? looks like an
                // error in Clang) "barbaz", so just skip any tokens starting with backslash-newline
                // when looking backwards here, without even trying to look at their content:
                if (!(s.empty() || s.startswith("/*") || s.startswith("//")
                      || s.startswith("\\\n")))
                {
                    break;
                }
            }
        }
#endif
    }

    if (startLoc.isInvalid() || endLoc.isInvalid())
        //TODO: should probably not happen
        return false;

    SourceLocation found;

    while (SM.isBeforeInTranslationUnit(startLoc, endLoc))
    {
        unsigned n = Lexer::MeasureTokenLength(startLoc, SM, compiler.getLangOpts());
        auto s = StringRef(compiler.getSourceManager().getCharacterData(startLoc), n);
        while (s.startswith("\\\n"))
        {
            s = s.drop_front(2);
            while (!s.empty()
                   && (s.front() == ' ' || s.front() == '\t' || s.front() == '\n'
                       || s.front() == '\v' || s.front() == '\f'))
            {
                s = s.drop_front(1);
            }
        }
        if (s == "SAL_CALL")
        {
            found = startLoc;
            break;
        }
        if (startLoc == endLoc)
        {
            break;
        }
        startLoc = startLoc.getLocWithOffset(std::max<unsigned>(n, 1));
    }

    if (found.isInvalid())
        return false;

    if (pLoc)
        *pLoc = found;

    return true;
}

bool SalCall::rewrite(SourceLocation locBegin)
{
    if (!rewriter)
        return false;
    if (!locBegin.isValid())
        return false;

    auto locEnd = locBegin.getLocWithOffset(8);
    if (!locEnd.isValid())
        return false;

    SourceRange range(locBegin, locEnd);

    // If we overlap with a previous area we modified, we cannot perform this change
    // without corrupting the source
    if (!checkOverlap(range))
        return false;

    if (!replaceText(locBegin, 9, ""))
        return false;

    return true;
}

// If we overlap with a previous area we modified, we cannot perform this change
// without corrupting the source
bool SalCall::checkOverlap(SourceRange range)
{
    SourceManager& SM = compiler.getSourceManager();
    char const* p1 = SM.getCharacterData(range.getBegin());
    char const* p2 = SM.getCharacterData(range.getEnd());
    for (std::pair<char const*, char const*> const& rPair : mvModifiedRanges)
    {
        if (rPair.first <= p1 && p1 <= rPair.second)
            return false;
        if (p1 <= rPair.second && rPair.first <= p2)
            return false;
    }
    mvModifiedRanges.emplace_back(p1, p2);
    return true;
}

static loplugin::Plugin::Registration<SalCall> reg("salcall", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
