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

using namespace std;

string getexe(string exename, bool maybeempty = false);

void setupccenv();

string processccargs(vector<string> rawargs, string& env_prefix, bool& verbose);

int startprocess(string command, string args, bool verbose);

#endif // INCLUDED_SOLENV_GCC_WRAPPERS_WRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
