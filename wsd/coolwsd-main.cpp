/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Process entry point for the COOLWSD daemon.
 * Functions: main()
 */

#include <config.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <sysexits.h>

#include <Poco/Exception.h>

#include <common/SigUtil.hpp>
#include <common/Util.hpp>
#include <wsd/COOLWSD.hpp>

// Split a string into whitespace-separated tokens, matching the unquoted shell
// word-splitting that the old start script relied on for 'extra_params'.
static std::vector<std::string> tokenizeOnWhitespace(const std::string& s)
{
    std::vector<std::string> tokens;
    std::istringstream iss(s);
    for (std::string tok; iss >> tok;)
        tokens.push_back(tok);
    return tokens;
}

int main(int argc, char** argv)
{
    SigUtil::setUserSignals();
    SigUtil::setFatalSignals("wsd " + Util::getCoolVersion() + ' ' + Util::getCoolVersionHash());

    // The container start script used to append the 'extra_params' environment
    // variable to the coolwsd command line (exec ... ${extra_params}). With the
    // shell-less, exec-form ENTRYPOINT there is nothing left to expand it, so
    // honour it here when --use-env-vars is in effect: tokenize it and append
    // the tokens to argv so they are parsed exactly like command-line options
    // (--o:ssl.enable=false, --disable-ssl, ...). This keeps the container's
    // configuration interface unchanged.
    std::vector<std::string> argStore;
    std::vector<char*> argPtrs;
    const char* extraParams = std::getenv("extra_params");
    const bool useEnvVars =
        std::any_of(argv, argv + argc,
                    [](const char* a) { return std::strcmp(a, "--use-env-vars") == 0; });
    if (useEnvVars && extraParams && *extraParams)
    {
        for (int i = 0; i < argc; ++i)
            argStore.emplace_back(argv[i]);
        for (auto& tok : tokenizeOnWhitespace(extraParams))
            argStore.push_back(std::move(tok));
        ::unsetenv("extra_params");

        argPtrs.reserve(argStore.size() + 1);
        for (auto& a : argStore)
            argPtrs.push_back(a.data());
        argPtrs.push_back(nullptr);

        argc = static_cast<int>(argStore.size());
        argv = argPtrs.data();
    }

    try
    {
        COOLWSD app;
        return app.run(argc, argv);
    }
    catch (Poco::Exception& exc)
    {
        std::cerr << exc.displayText() << std::endl;
        return EX_SOFTWARE;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
