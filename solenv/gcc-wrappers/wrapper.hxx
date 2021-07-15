/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef INCLUDED_SOLENV_GCC_WRAPPERS_WRAPPER_HXX
#define INCLUDED_SOLENV_GCC_WRAPPERS_WRAPPER_HXX

#include <string>
#include <iostream>
#include <vector>

std::string getexe(std::string exename, bool maybeempty = false);

void setupccenv();

std::string processccargs(std::vector<std::string> rawargs, std::string& env_prefix, bool& verbose);

int startprocess(std::string command, std::string args, bool verbose);

#endif // INCLUDED_SOLENV_GCC_WRAPPERS_WRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
