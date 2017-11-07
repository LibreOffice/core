/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>

#include "compat.hxx"
#include "plugin.hxx"

namespace {

class OslEndian: public loplugin::Plugin, public PPCallbacks {
public:
    explicit OslEndian(loplugin::InstantiationData const & data): Plugin(data) {
        compat::addPPCallbacks(compiler.getPreprocessor(), this);
    }

    enum { isPPCallback = true };

private:
    void run() override {}

    void MacroDefined(Token const & MacroNameTok, MacroDirective const *)
        override
    {
        auto id = MacroNameTok.getIdentifierInfo()->getName();
        if (id == "OSL_BIGENDIAN") {
            if (definedLit_.isValid()) {
                report(
                    DiagnosticsEngine::Warning,
                    "macro %0 defined in addition to 'OSL_LITENDIAN'",
                    MacroNameTok.getLocation())
                    << MacroNameTok.getIdentifierInfo();
                report(
                    DiagnosticsEngine::Note,
                    "conflicting macro definition is here", definedLit_);
            }
            definedBig_ = MacroNameTok.getLocation();
            assert(definedBig_.isValid());
        } else if (id == "OSL_LITENDIAN") {
            if (definedBig_.isValid()) {
                report(
                    DiagnosticsEngine::Warning,
                    "macro %0 defined in addition to 'OSL_BIGENDIAN'",
                    MacroNameTok.getLocation())
                    << MacroNameTok.getIdentifierInfo();
                report(
                    DiagnosticsEngine::Note,
                    "conflicting macro definition is here", definedBig_);
            }
            definedLit_ = MacroNameTok.getLocation();
            assert(definedLit_.isValid());
        }
    }

    void MacroUndefined(
        Token const & MacroNameTok, compat::MacroDefinitionParam
#if CLANG_VERSION >= 50000
        , MacroDirective const *
#endif
        ) override
    {
        auto id = MacroNameTok.getIdentifierInfo()->getName();
        if (id == "OSL_BIGENDIAN" || id == "OSL_LITENDIAN") {
            report(
                DiagnosticsEngine::Warning, "macro %0 undefinition",
                MacroNameTok.getLocation())
                << MacroNameTok.getIdentifierInfo();
        }
    }

    void Defined(
        Token const & MacroNameTok, compat::MacroDefinitionParam, SourceRange)
        override
    {
        check(MacroNameTok);
    }

    void Ifdef(
        SourceLocation, Token const & MacroNameTok,
        compat::MacroDefinitionParam) override
    {
        check(MacroNameTok);
    }

    void Ifndef(
        SourceLocation, Token const & MacroNameTok,
        compat::MacroDefinitionParam) override
    {
        check(MacroNameTok);
    }

    void check(Token const & macro) const {
        auto id = macro.getIdentifierInfo()->getName();
        if ((id == "OSL_BIGENDIAN" || id == "OSL_LITENDIAN")
            && definedBig_.isInvalid() && definedLit_.isInvalid())
        {
            report(
                DiagnosticsEngine::Warning,
                "definition of macro %0 checked but 'osl/endian.h' is not"
                    " included",
                macro.getLocation())
                << macro.getIdentifierInfo();
        }
    }

    SourceLocation definedBig_;
    SourceLocation definedLit_;
};

loplugin::Plugin::Registration<OslEndian> X("oslendian");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
