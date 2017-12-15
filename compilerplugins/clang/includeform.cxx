/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>

#include "compat.hxx"
#include "plugin.hxx"

namespace {

class IncludeForm final: public PPCallbacks, public loplugin::RewritePlugin {
public:
    explicit IncludeForm(loplugin::InstantiationData const & data):
        RewritePlugin(data)
    { compiler.getPreprocessor().addPPCallbacks(std::unique_ptr<PPCallbacks>(this)); }

private:
    void run() override {}

    void InclusionDirective(
        SourceLocation HashLoc, Token const & IncludeTok, StringRef,
        bool IsAngled, CharSourceRange FilenameRange, FileEntry const * File,
        StringRef SearchPath, StringRef, clang::Module const *) override
    {
        if (ignoreLocation(HashLoc)) {
            return;
        }
        if (File == nullptr) { // in case of "fatal error: '...' file not found"
            return;
        }
        if (IncludeTok.getIdentifierInfo()->getPPKeywordID() != tok::pp_include)
        {
            return;
        }
        auto const uno = isInUnoIncludeFile(HashLoc)
            && !compiler.getSourceManager().isInMainFile(HashLoc);
                // exclude the various compat.cxx that are included in
                // isInUnoIncludeFile
            //TODO: 'uno' should be false if HashLoc is inside an
            // '#ifdef LIBO_INTERNAL_ONLY' block
        bool shouldUseAngles;
        if (uno) {
            shouldUseAngles
                = (!(loplugin::hasPathnamePrefix(SearchPath, SRCDIR)
                     || loplugin::hasPathnamePrefix(SearchPath, BUILDDIR))
                   || loplugin::hasPathnamePrefix(
                       SearchPath, WORKDIR "/UnpackedTarball"));
        } else {
            auto const file = StringRef(
                compiler.getSourceManager().getPresumedLoc(HashLoc)
                .getFilename());
            auto pos = file.rfind('/');
#if defined _WIN32
            auto const pos2 = file.rfind('\\');
            if (pos2 != StringRef::npos
                && (pos == StringRef::npos || pos2 > pos))
            {
                pos = pos2;
            }
#endif
            auto const dir = compat::take_front(file, pos);
            shouldUseAngles = !loplugin::isSamePathname(SearchPath, dir);
        }
        if (shouldUseAngles == IsAngled) {
            return;
        }
        if (rewriter != nullptr) {
            auto last = FilenameRange.getEnd().getLocWithOffset(-1);
            if ((compiler.getSourceManager().getCharacterData(
                     FilenameRange.getBegin())[0]
                 == (IsAngled ? '<' : '"'))
                && (compiler.getSourceManager().getCharacterData(last)[0]
                    == (IsAngled ? '>' : '"'))
                && replaceText(
                    FilenameRange.getBegin(), 1, shouldUseAngles ? "<" : "\"")
                && replaceText(last, 1, shouldUseAngles ? ">" : "\""))
            {
                //TODO: atomically only replace both or neither
                return;
            }
        }
        report(
            DiagnosticsEngine::Warning,
            ("%select{|in UNO API include file, }0replace"
             " %select{\"...\"|<...>}1 include form with"
             " %select{\"...\"|<...>}2 for inclusion of %select{%select{a"
             " source file next to the current source file|a source file not"
             " next to the current source file, or a header}2|%select{a source"
             " file|a header}2}0, %3"),
            FilenameRange.getBegin())
            << uno << IsAngled << shouldUseAngles << File->getName()
            << FilenameRange;
    }
};

static loplugin::Plugin::Registration<IncludeForm> reg("includeform", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
