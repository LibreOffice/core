/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>
#include <config_options.h>
#include <vcl/svgparser.hxx>
#include <sal/log.hxx>
#include <osl/module.hxx>
#include <tools/svlibrary.h>

namespace vcl
{
AbstractSvgParser::~AbstractSvgParser() {}

#ifndef DISABLE_DYNLOADING
extern "C" {
static void thisModule() {}
}
#endif

#ifndef DISABLE_DYNLOADING
typedef AbstractSvgParser* (*svgio_create_parser)();

std::unique_ptr<AbstractSvgParser> loadSvgParser()
{
    static const svgio_create_parser pSymbol = [&]() {
        osl::Module aMod;

        // load basctl module
        if (!aMod.loadRelative(&thisModule,
#if ENABLE_MERGELIBS
                               SVLIBRARY("merged")
#else
                               SVLIBRARY("svgio")
#endif
                                   ))
        {
            SAL_WARN("drawinglayer.tools", "cannot load basctl");
            return static_cast<svgio_create_parser>(nullptr);
        }

        // get symbol
        svgio_create_parser pSymbol
            = reinterpret_cast<svgio_create_parser>(aMod.getFunctionSymbol("svgio_create_parser"));
        aMod.release();
        return pSymbol;
    }();

    return std::unique_ptr<AbstractSvgParser>(pSymbol());
}

#else
extern "C" AbstractSvgParser* svgio_create_parser();

std::unique_ptr<AbstractSvgParser> loadSvgParser()
{
    return std::unique_ptr<AbstractSvgParser>(svgio_create_parser);
}
#endif
};
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
