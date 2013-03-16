/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "wrapper.hxx"

int main(int argc, char *argv[]) {
    vector<string> rawargs(argv + 1, argv + argc);

    string command=getexe("REAL_CC");

    string args=processccargs(rawargs);

    setupccenv();

    cerr << "CC= " << command << " " << args << endl;

    return startprocess(command,args);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
