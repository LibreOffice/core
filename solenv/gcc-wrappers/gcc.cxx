/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "wrapper.hxx"

int main(int argc, char* argv[])
{
    std::vector<std::string> rawargs(argv + 1, argv + argc);

    std::string env_prefix; // defaults to REAL_
    bool verbose = false;
    std::string args = processccargs(rawargs, env_prefix, verbose);

    std::string command = getexe(env_prefix + "CC");
    std::string flags = getexe(env_prefix + "CC_FLAGS", true);
    args.insert(0, flags.empty() ? std::string() : flags + " ");

    setupccenv();

    return startprocess(command, args, verbose);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
