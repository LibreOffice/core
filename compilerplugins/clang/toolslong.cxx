/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <cassert>
#include <limits>
#include <map>
#include <string>
#include <iostream>

#include "clang/AST/Attr.h"
#include "clang/Basic/Builtins.h"

#include "config_clang.h"

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

namespace
{
bool isLong(QualType type)
{
    type = type.getNonReferenceType();
    // ignore sal_Int64
    if (type->getAs<TypedefType>())
        return false;
    // some parts of the STL have ::difference_type => long
    if (type->getAs<AutoType>() || type->getAs<DecltypeType>())
        return false;
#if CLANG_VERSION < 80000
    // Prior to <https://github.com/llvm/llvm-project/commit/
    // c50240dac133451b3eae5b89cecca4c1c4af9fd4> "[AST] Get aliased type info from an aliased
    // TemplateSpecialization" in Clang 8, if type is a TemplateSpecializationType on top of a
    // TypedefType, the above getAs<TypedefType> returned null (as it unconditionally desugared the
    // TemplateSpecializationType to the underlying canonic type, not to any aliased type), so re-
    // check with the TemplateSpecializationType's aliased type:
    if (auto const t = type->getAs<TemplateSpecializationType>())
    {
        if (t->isTypeAlias())
        {
            return isLong(t->getAliasedType());
        }
    }
#endif
    if (type->isSpecificBuiltinType(BuiltinType::Kind::Long))
        return true;
    auto arrayType = type->getAsArrayTypeUnsafe();
    if (arrayType)
        return isLong(arrayType->getElementType());
    if (type->isPointerType())
        return isLong(type->getPointeeType());
    return false;
}

enum class OverrideKind
{
    NO,
    YES,
    MAYBE
};

OverrideKind getOverrideKind(FunctionDecl const* decl)
{
    CXXMethodDecl const* m = dyn_cast<CXXMethodDecl>(decl);
    if (m == nullptr)
        return OverrideKind::NO;
    if (m->size_overridden_methods() != 0 || m->hasAttr<OverrideAttr>())
        return OverrideKind::YES;
    if (!dyn_cast<CXXRecordDecl>(m->getDeclContext())->hasAnyDependentBases())
        return OverrideKind::NO;
    return OverrideKind::MAYBE;
}

class ToolsLong : public loplugin::FilteringRewritePlugin<ToolsLong>
{
public:
    explicit ToolsLong(loplugin::InstantiationData const& data)
        : loplugin::FilteringRewritePlugin<ToolsLong>(data)
    {
    }

    virtual void run() override;

    bool VisitCStyleCastExpr(CStyleCastExpr* expr);

    bool VisitCXXStaticCastExpr(CXXStaticCastExpr* expr);

    bool VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr* expr);

    bool WalkUpFromParmVarDecl(ParmVarDecl const* decl);
    bool VisitParmVarDecl(ParmVarDecl const* decl);

    bool WalkUpFromVarDecl(VarDecl const* decl);
    bool VisitVarDecl(VarDecl const* decl);

    bool WalkUpFromFieldDecl(FieldDecl const* decl);
    bool VisitFieldDecl(FieldDecl const* decl);

    bool WalkUpFromFunctionDecl(FunctionDecl const* decl);
    bool VisitFunctionDecl(FunctionDecl const* decl);

    bool VisitCallExpr(CallExpr const* expr);

private:
    bool rewrite(SourceLocation location);
    bool isExcludedFile(SourceLocation spellingLocation) const;
    /** sort by the reverse of source order, so we can do replacing from the end of the file backwards,
        which means we reduce the chances of having overlapping changes. */
    template <class T>
    std::vector<std::pair<T, bool>> reverseSourceOrder(std::map<T, bool> const& map) const
    {
        std::vector<std::pair<T, bool>> vec(map.begin(), map.end());
        std::sort(vec.begin(), vec.end(),
                  [&](std::pair<T, bool> const& lhs, std::pair<T, bool> const& rhs) {
                      return compiler.getSourceManager().getCharacterData(
                                 compat::getBeginLoc(lhs.first))
                             > compiler.getSourceManager().getCharacterData(
                                   compat::getBeginLoc(rhs.first));
                  });
        return vec;
    }

    std::map<VarDecl const*, bool> varDecls_;
    std::map<FieldDecl const*, bool> fieldDecls_;
    std::map<ParmVarDecl const*, bool> parmVarDecls_;
    std::map<FunctionDecl const*, bool> functionDecls_;
    std::map<CXXStaticCastExpr const*, bool> staticCasts_;
    std::map<CXXFunctionalCastExpr const*, bool> functionalCasts_;
};

void ToolsLong::run()
{
    if (!compiler.getLangOpts().CPlusPlus)
        return;

    StringRef fn(handler.getMainFileName());
    // sberg says this is fine
    if (loplugin::isSamePathname(fn, SRCDIR "/avmedia/source/win/framegrabber.cxx")
        || loplugin::isSamePathname(fn, SRCDIR "/avmedia/source/win/manager.cxx")
        || loplugin::isSamePathname(fn, SRCDIR "/avmedia/source/win/player.cxx")
        || loplugin::isSamePathname(fn, SRCDIR "/avmedia/source/win/window.cxx")
        || loplugin::isSamePathname(fn, SRCDIR "/connectivity/source/drivers/ado/AStatement.cxx")
        || loplugin::isSamePathname(fn, SRCDIR "/connectivity/source/drivers/ado/Awrapado.cxx")
        || loplugin::isSamePathname(fn, SRCDIR "/desktop/win32/source/loader.cxx")
        || loplugin::isSamePathname(fn, SRCDIR "/extensions/source/activex/SOActiveX.cxx")
        || loplugin::isSamePathname(fn, SRCDIR "/pyuno/source/module/pyuno.cxx")
        || loplugin::isSamePathname(fn, SRCDIR
                                    "/setup_native/source/win32/customactions/sellang/sellang.cxx")
        || loplugin::isSamePathname(fn, SRCDIR
                                    "/shell/source/win32/shlxthandler/ooofilt/stream_helper.cxx")
        || loplugin::isSamePathname(fn, SRCDIR "/shell/source/win32/zipfile/zipfile.cxx")
        || loplugin::isSamePathname(fn, SRCDIR "/ucb/source/ucp/webdav-curl/CurlSession.cxx"))
        return;
    // these are places where the external API is actually "long"
    if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/filter/jpeg/JpegReader.cxx"))
        return;
    if (loplugin::isSamePathname(fn, SRCDIR "/writerperfect/source/common/DirectoryStream.cxx"))
        return;
    if (loplugin::isSamePathname(fn, SRCDIR "/writerperfect/source/common/WPXSvInputStream.cxx"))
        return;
    if (loplugin::isSamePathname(fn,
                                 SRCDIR "/writerperfect/source/calc/MSWorksCalcImportFilter.cxx"))
        return;
    if (loplugin::isSamePathname(fn, SRCDIR "/writerperfect/qa/unit/WPXSvStreamTest.cxx"))
        return;
    if (loplugin::isSamePathname(fn, SRCDIR "/desktop/source/lib/init.cxx"))
        return;

    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

    for (auto const& dcl : reverseSourceOrder(varDecls_))
    {
        auto const decl = dcl.first;
        SourceLocation loc{ compat::getBeginLoc(decl) };
        TypeSourceInfo* tsi = decl->getTypeSourceInfo();
        if (tsi != nullptr)
        {
            SourceLocation l{ compiler.getSourceManager().getExpansionLoc(
                tsi->getTypeLoc().getBeginLoc()) };
            SourceLocation end{ compiler.getSourceManager().getExpansionLoc(
                tsi->getTypeLoc().getEndLoc()) };
            assert(l.isFileID() && end.isFileID());
            if (l == end || compiler.getSourceManager().isBeforeInTranslationUnit(l, end))
            {
                for (;;)
                {
                    unsigned n = Lexer::MeasureTokenLength(l, compiler.getSourceManager(),
                                                           compiler.getLangOpts());
                    std::string s{ compiler.getSourceManager().getCharacterData(l), n };
                    if (s == "long")
                    {
                        loc = l;
                        break;
                    }
                    if (l == end)
                    {
                        break;
                    }
                    l = l.getLocWithOffset(std::max<unsigned>(n, 1));
                }
            }
        }
        if (!rewrite(loc))
        {
            report(DiagnosticsEngine::Warning, "VarDecl, use \"tools::Long\" instead of %0", loc)
                << decl->getType().getLocalUnqualifiedType() << decl->getSourceRange();
        }
    }
    for (auto const& dcl : reverseSourceOrder(fieldDecls_))
    {
        auto const decl = dcl.first;
        SourceLocation loc{ compat::getBeginLoc(decl) };
        TypeSourceInfo* tsi = decl->getTypeSourceInfo();
        if (tsi != nullptr)
        {
            SourceLocation l{ compiler.getSourceManager().getExpansionLoc(
                tsi->getTypeLoc().getBeginLoc()) };
            SourceLocation end{ compiler.getSourceManager().getExpansionLoc(
                tsi->getTypeLoc().getEndLoc()) };
            assert(l.isFileID() && end.isFileID());
            if (l == end || compiler.getSourceManager().isBeforeInTranslationUnit(l, end))
            {
                for (;;)
                {
                    unsigned n = Lexer::MeasureTokenLength(l, compiler.getSourceManager(),
                                                           compiler.getLangOpts());
                    std::string s{ compiler.getSourceManager().getCharacterData(l), n };
                    if (s == "long")
                    {
                        loc = l;
                        break;
                    }
                    if (l == end)
                    {
                        break;
                    }
                    l = l.getLocWithOffset(std::max<unsigned>(n, 1));
                }
            }
        }
        if (!rewrite(loc))
        {
            report(DiagnosticsEngine::Warning, "FieldDecl, use \"tools::Long\" instead of %0", loc)
                << decl->getType().getLocalUnqualifiedType() << decl->getSourceRange();
        }
    }
    for (auto const& dcl : reverseSourceOrder(parmVarDecls_))
    {
        auto const decl = dcl.first;
        SourceLocation loc{ compat::getBeginLoc(decl) };
        TypeSourceInfo* tsi = decl->getTypeSourceInfo();
        if (tsi != nullptr)
        {
            SourceLocation l{ compiler.getSourceManager().getExpansionLoc(
                tsi->getTypeLoc().getBeginLoc()) };
            SourceLocation end{ compiler.getSourceManager().getExpansionLoc(
                tsi->getTypeLoc().getEndLoc()) };
            assert(l.isFileID() && end.isFileID());
            if (l == end || (compiler.getSourceManager().isBeforeInTranslationUnit(l, end)))
            {
                for (;;)
                {
                    unsigned n = Lexer::MeasureTokenLength(l, compiler.getSourceManager(),
                                                           compiler.getLangOpts());
                    std::string s{ compiler.getSourceManager().getCharacterData(l), n };
                    if (s == "long")
                    {
                        loc = l;
                        break;
                    }
                    if (l == end)
                    {
                        break;
                    }
                    l = l.getLocWithOffset(std::max<unsigned>(n, 1));
                }
            }
        }
        FunctionDecl const* f = dyn_cast_or_null<FunctionDecl>(decl->getDeclContext());
        if (f)
            f = f->getCanonicalDecl();
        OverrideKind k = f ? getOverrideKind(f) : OverrideKind::NO;
        if (k == OverrideKind::MAYBE || !rewrite(loc))
        {
            report(DiagnosticsEngine::Warning,
                   ("ParmVarDecl, use \"tools::Long\" instead of"
                    " %0%1"),
                   loc)
                << decl->getType().getNonReferenceType().getLocalUnqualifiedType()
                << (k == OverrideKind::MAYBE ? (" (unless this member function overrides a"
                                                " dependent base member function, even"
                                                " though it is not marked 'override')")
                                             : "")
                << decl->getSourceRange();
        }
    }
    for (auto const& dcl : functionDecls_)
    {
        auto const decl = dcl.first;
        SourceLocation loc{ compat::getBeginLoc(decl) };
        SourceLocation l{ compiler.getSourceManager().getExpansionLoc(loc) };
        SourceLocation end{ compiler.getSourceManager().getExpansionLoc(
            decl->getNameInfo().getLoc()) };
        assert(l.isFileID() && end.isFileID());
        if (compiler.getSourceManager().isBeforeInTranslationUnit(l, end))
        {
            while (l != end)
            {
                unsigned n = Lexer::MeasureTokenLength(l, compiler.getSourceManager(),
                                                       compiler.getLangOpts());
                std::string s{ compiler.getSourceManager().getCharacterData(l), n };
                if (s == "long")
                {
                    loc = l;
                    break;
                }
                l = l.getLocWithOffset(std::max<unsigned>(n, 1));
            }
        }
        if (rewrite(loc))
            continue;
        report(DiagnosticsEngine::Warning, "use \"tools::Long\" instead of %0 as return type%1",
               loc)
            << decl->getReturnType().getNonReferenceType().getLocalUnqualifiedType()
            << (getOverrideKind(decl) == OverrideKind::MAYBE
                    ? (" (unless this member function overrides a dependent"
                       " base member function, even though it is not marked"
                       " 'override')")
                    : "")
            << decl->getSourceRange();
    }

    for (auto const& dcl : staticCasts_)
    {
        auto const expr = dcl.first;
        SourceLocation loc{ compat::getBeginLoc(expr) };
        TypeSourceInfo* tsi = expr->getTypeInfoAsWritten();
        if (tsi != nullptr)
        {
            SourceLocation l{ compiler.getSourceManager().getExpansionLoc(
                tsi->getTypeLoc().getBeginLoc()) };
            SourceLocation end{ compiler.getSourceManager().getExpansionLoc(
                tsi->getTypeLoc().getEndLoc()) };
            assert(l.isFileID() && end.isFileID());
            if (l == end || compiler.getSourceManager().isBeforeInTranslationUnit(l, end))
            {
                for (;;)
                {
                    unsigned n = Lexer::MeasureTokenLength(l, compiler.getSourceManager(),
                                                           compiler.getLangOpts());
                    std::string s{ compiler.getSourceManager().getCharacterData(l), n };
                    if (s == "long")
                    {
                        loc = l;
                        break;
                    }
                    if (l == end)
                    {
                        break;
                    }
                    l = l.getLocWithOffset(std::max<unsigned>(n, 1));
                }
            }
        }
        if (!rewrite(loc))
        {
            report(DiagnosticsEngine::Warning, "CXXStaticCastExpr, suspicious cast from %0 to %1",
                   compat::getBeginLoc(expr))
                << expr->getSubExpr()->IgnoreParenImpCasts()->getType() << expr->getType()
                << expr->getSourceRange();
        }
    }

    for (auto const& dcl : functionalCasts_)
    {
        auto const expr = dcl.first;
        SourceLocation loc{ compat::getBeginLoc(expr) };
        TypeSourceInfo* tsi = expr->getTypeInfoAsWritten();
        if (tsi != nullptr)
        {
            SourceLocation l{ compiler.getSourceManager().getExpansionLoc(
                tsi->getTypeLoc().getBeginLoc()) };
            SourceLocation end{ compiler.getSourceManager().getExpansionLoc(
                tsi->getTypeLoc().getEndLoc()) };
            assert(l.isFileID() && end.isFileID());
            if (l == end || compiler.getSourceManager().isBeforeInTranslationUnit(l, end))
            {
                for (;;)
                {
                    unsigned n = Lexer::MeasureTokenLength(l, compiler.getSourceManager(),
                                                           compiler.getLangOpts());
                    std::string s{ compiler.getSourceManager().getCharacterData(l), n };
                    if (s == "long")
                    {
                        loc = l;
                        break;
                    }
                    if (l == end)
                    {
                        break;
                    }
                    l = l.getLocWithOffset(std::max<unsigned>(n, 1));
                }
            }
        }
        if (!rewrite(loc))
        {
            report(DiagnosticsEngine::Warning,
                   "CXXFunctionalCastExpr, suspicious cast from %0 to %1",
                   compat::getBeginLoc(expr))
                << expr->getSubExpr()->IgnoreParenImpCasts()->getType() << expr->getType()
                << expr->getSourceRange();
        }
    }
}

bool ToolsLong::VisitCStyleCastExpr(CStyleCastExpr* expr)
{
    if (ignoreLocation(expr))
        return true;
    if (isExcludedFile(compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(expr))))
        return true;
    auto const k = isLong(expr->getType());
    if (!k)
        return true;
    SourceLocation loc{ compat::getBeginLoc(expr) };
    while (compiler.getSourceManager().isMacroArgExpansion(loc))
        loc = compiler.getSourceManager().getImmediateMacroCallerLoc(loc);
    if (compiler.getSourceManager().isMacroBodyExpansion(loc)
        && compiler.getSourceManager().isInSystemHeader(
               compiler.getSourceManager().getSpellingLoc(loc)))
    {
        return true;
    }
    report(DiagnosticsEngine::Warning, "CStyleCastExpr, suspicious cast from %0 to %1",
           compat::getBeginLoc(expr))
        << expr->getSubExpr()->IgnoreParenImpCasts()->getType() << expr->getType()
        << expr->getSourceRange();
    return true;
}

bool ToolsLong::VisitCXXStaticCastExpr(CXXStaticCastExpr* expr)
{
    if (ignoreLocation(expr))
        return true;
    if (isExcludedFile(compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(expr))))
        return true;
    auto const k = isLong(expr->getType());
    if (!k)
        return true;
    staticCasts_.insert({ expr, k });
    return true;
}

bool ToolsLong::VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr* expr)
{
    if (ignoreLocation(expr))
        return true;
    if (isExcludedFile(compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(expr))))
        return true;
    auto const k = isLong(expr->getType());
    if (!k)
        return true;
    functionalCasts_.insert({ expr, k });
    return true;
}

bool ToolsLong::WalkUpFromParmVarDecl(ParmVarDecl const* decl) { return VisitParmVarDecl(decl); }

bool ToolsLong::VisitParmVarDecl(ParmVarDecl const* decl)
{
    if (ignoreLocation(decl))
        return true;
    if (isExcludedFile(compiler.getSourceManager().getSpellingLoc(decl->getLocation())))
        return true;
    auto const fbk = isLong(decl->getType());
    if (!fbk)
        return true;
    FunctionDecl const* f = dyn_cast<FunctionDecl>(decl->getDeclContext());
    if (f) // e.g.: typedef sal_Bool (* FuncPtr )( sal_Bool );
    {
        // ignore the function in include/test/cppunitasserthelper.hxx
        if (f->getIdentifier() && f->getName() == "assertEquals")
            return true;
        auto canonicalF = f->getCanonicalDecl();
        if (canonicalF->isDeletedAsWritten() && isa<CXXConversionDecl>(canonicalF))
            return true;
        if (auto const d = dyn_cast<CXXMethodDecl>(canonicalF))
        {
            if (d->isVirtual())
            {
                return true;
            }
        }
        // Only rewrite declarations in include files if a definition is
        // also seen, to avoid compilation of a definition (in a main file
        // only processed later) to fail with a "mismatch" error before the
        // rewriter had a chance to act upon the definition:
        bool ok = canonicalF->isDefined()
                  || compiler.getSourceManager().isInMainFile(
                         compiler.getSourceManager().getSpellingLoc(f->getNameInfo().getLoc()));
        if (!ok)
            return true;
    }
    parmVarDecls_.insert({ decl, fbk });
    return true;
}

bool ToolsLong::WalkUpFromVarDecl(VarDecl const* decl) { return VisitVarDecl(decl); }

bool ToolsLong::VisitVarDecl(VarDecl const* decl)
{
    if (ignoreLocation(decl))
        return true;
    if (isExcludedFile(compiler.getSourceManager().getSpellingLoc(decl->getLocation())))
        return true;
    auto k = isLong(decl->getType());
    if (!k)
        return true;
    varDecls_.insert({ decl, k });
    return true;
}

bool ToolsLong::WalkUpFromFieldDecl(FieldDecl const* decl) { return VisitFieldDecl(decl); }

bool ToolsLong::VisitFieldDecl(FieldDecl const* decl)
{
    if (ignoreLocation(decl))
        return true;
    if (isExcludedFile(compiler.getSourceManager().getSpellingLoc(decl->getLocation())))
        return true;
    auto k = isLong(decl->getType());
    if (!k)
        return true;
    TagDecl const* td = dyn_cast<TagDecl>(decl->getDeclContext());
    if (td == nullptr)
    {
        //TODO: ObjCInterface
        return true;
    }
    fieldDecls_.insert({ decl, k });
    return true;
}

bool ToolsLong::WalkUpFromFunctionDecl(FunctionDecl const* decl) { return VisitFunctionDecl(decl); }

bool ToolsLong::VisitFunctionDecl(FunctionDecl const* decl)
{
    if (ignoreLocation(decl))
        return true;
    if (isExcludedFile(compiler.getSourceManager().getSpellingLoc(decl->getLocation())))
        return true;
    auto const fbk = isLong(decl->getReturnType());
    if (!fbk)
        return true;
    if (decl->isDeletedAsWritten() && isa<CXXConversionDecl>(decl))
        return true;
    if (auto const d = dyn_cast<CXXMethodDecl>(decl))
    {
        if (d->isVirtual())
        {
            return true;
        }
    }
    if (decl->isDefined()
        || compiler.getSourceManager().isInMainFile(
               compiler.getSourceManager().getSpellingLoc(decl->getNameInfo().getLoc())))
    {
        functionDecls_.insert({ decl, fbk });
    }
    return true;
}

bool ToolsLong::VisitCallExpr(CallExpr const* expr)
{
    if (ignoreLocation(expr))
    {
        return true;
    }
    auto const d1 = expr->getDirectCallee();
    if (d1 == nullptr || !loplugin::DeclCheck(d1).Function("curl_easy_getinfo").GlobalNamespace())
    {
        return true;
    }
    if (expr->getNumArgs() != 3)
    {
        return true;
    }
    //TODO: Check expr->getArg(1) is CURLINFO_RESPONSE_CODE
    auto const e1 = dyn_cast<UnaryOperator>(expr->getArg(2)->IgnoreParenImpCasts());
    if (e1 == nullptr || e1->getOpcode() != UO_AddrOf)
    {
        return true;
    }
    auto const e2 = dyn_cast<DeclRefExpr>(e1->getSubExpr()->IgnoreParenImpCasts());
    if (e2 == nullptr)
    {
        return true;
    }
    auto const d2 = e2->getDecl();
    if (auto const d3 = dyn_cast<ParmVarDecl>(d2))
    {
        parmVarDecls_.erase(d3);
    }
    else if (auto const d4 = dyn_cast<VarDecl>(d2))
    {
        varDecls_.erase(d4);
    }
    else if (auto const d5 = dyn_cast<FieldDecl>(d2))
    {
        fieldDecls_.erase(d5);
    }
    return true;
}

bool ToolsLong::rewrite(SourceLocation location)
{
    if (rewriter != nullptr)
    {
        SourceLocation loc{ compiler.getSourceManager().getExpansionLoc(location) };
        unsigned n
            = Lexer::MeasureTokenLength(loc, compiler.getSourceManager(), compiler.getLangOpts());
        if (std::string(compiler.getSourceManager().getCharacterData(loc), n) == "long")
        {
            return replaceText(loc, n, "tools::Long");
        }
    }
    return false;
}

bool ToolsLong::isExcludedFile(SourceLocation spellingLocation) const
{
    if (isInUnoIncludeFile(spellingLocation))
        return true;
    auto f = getFilenameOfLocation(spellingLocation);
    return loplugin::hasPathnamePrefix(f, SRCDIR "/include/cppu/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/include/cppuhelper/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/include/registry/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/include/osl/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/include/rtl/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/include/sal/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/include/salhelper/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/include/typelib/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/include/LibreOfficeKit/") // TODO
           || loplugin::hasPathnamePrefix(f, SRCDIR "/bridges/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/codemaker/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/configmgr/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/cppu/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/cppuhelper/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/external/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/libreofficekit/") // TODO
           || loplugin::hasPathnamePrefix(f, SRCDIR "/registry/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/rtl/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/sal/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/salhelper/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/soltools/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/unoidl/")
           || loplugin::hasPathnamePrefix(f, SRCDIR "/workdir/");
}

loplugin::Plugin::Registration<ToolsLong> X("toolslong", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
