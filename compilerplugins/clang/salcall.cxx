/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"
#include <cassert>
#include <string>
#include <iostream>
#include <fstream>

// The SAL_CALL function annotation is only necessary on our outward
// facing C++ ABI, anywhere else it is just cargo-cult.
//

namespace
{
static bool startswith(const std::string& rStr, const char* pSubStr)
{
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

class SalCall final : public RecursiveASTVisitor<SalCall>, public loplugin::Plugin
{
public:
    explicit SalCall(loplugin::InstantiationData const& data)
        : Plugin(data)
    {
    }

    virtual void run() override
    {
        std::string fn(compiler.getSourceManager()
                           .getFileEntryForID(compiler.getSourceManager().getMainFileID())
                           ->getName());
        loplugin::normalizeDotDotInFilePath(fn);
        if (startswith(fn, SRCDIR "/sal/") || startswith(fn, SRCDIR "/bridges/") // TODO ?
            || startswith(fn, SRCDIR "/binaryurp/") // TODO ?
            || startswith(fn, SRCDIR "/cppu/") || startswith(fn, SRCDIR "/cppuhelper/")
            || startswith(fn, SRCDIR "/salhelper/") || startswith(fn, SRCDIR "/systools/")
            || startswith(fn, SRCDIR "/typelib/"))
            return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitFunctionDecl(FunctionDecl const*);
};

bool SalCall::VisitFunctionDecl(FunctionDecl const* decl)
{
    if (decl->getLocation().isInvalid() || ignoreLocation(decl))
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

    // ignore UNO implementations
    auto canonicalDecl = decl->getCanonicalDecl();
    while (auto methodDecl = dyn_cast<CXXMethodDecl>(canonicalDecl))
    {
        if (!methodDecl->size_overridden_methods())
            break;
        canonicalDecl = *methodDecl->begin_overridden_methods();
    }
    if (isInUnoIncludeFile(
            compiler.getSourceManager().getSpellingLoc(canonicalDecl->getLocation())))
        return true;
    StringRef name{ compiler.getSourceManager().getFilename(canonicalDecl->getLocation()) };
    if (loplugin::hasPathnamePrefix(name, WORKDIR "/UnoApiHeadersTarget/"))
        return true;

    SourceRange range;
    if (!decl->hasBody())
        range = decl->getSourceRange();
    else
        range = SourceRange(decl->getSourceRange().getBegin(),
                            decl->getBody()->getSourceRange().getBegin());
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = range.getBegin();
    SourceLocation endLoc = range.getEnd();
    char const* p1 = SM.getCharacterData(startLoc);
    char const* p2 = SM.getCharacterData(endLoc);

    static const char* SAL_CALL = "SAL_CALL";

    // For reasons I cannot fathom, clang will sometimes return waaaaay more code from getCharacterData
    // than the source range actually covers, so scan for the "(".
    // It seems to have something to do with "friend" declarations ????
    char const* leftBracket = static_cast<char const*>(memchr(p1, '(', p2 - p1));
    if (leftBracket)
    {
        char const* found = std::search(p1, leftBracket, SAL_CALL, SAL_CALL + strlen(SAL_CALL));

        if (found < leftBracket)
        {
            //            std::cout << std::string(p1, leftBracket - p1) << std::endl;
            report(DiagnosticsEngine::Warning, "SAL_CALL unnecessary here", decl->getLocation())
                << decl->getSourceRange();
            if (canonicalDecl != decl)
                report(DiagnosticsEngine::Note, "parent function decl here",
                       canonicalDecl->getLocation())
                    << canonicalDecl->getSourceRange();
        }
    }
    return true;
}

static loplugin::Plugin::Registration<SalCall> reg("salcall");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
