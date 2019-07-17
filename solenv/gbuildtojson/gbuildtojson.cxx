/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <stdio.h>

using namespace std;

int main(int argc, char** argv)
{
    const string optsintro("--");
    map<string, string> vartofile;
    for(int i=1; i < argc; ++i)
    {
        const string arg(argv[i]);
        if(arg.substr(0,2) != optsintro)
        {
            cerr << "Only option args starting with -- allowed." << endl;
            return 1;
        }
        const size_t eqpos = arg.find("=", 2);
        if(eqpos == string::npos)
        {
            cerr << "Only option args assigning with = allowed." << endl;
            return 2;
        }
        const string argname(arg.substr(2, eqpos-2));
        vartofile[argname] = arg.substr(eqpos+1, string::npos);
    }
    cout << "{";
    bool first(true);
    for(auto& varandfile : vartofile)
    {
        if(first)
            first =false;
        else
            cout << "," << endl;
        string varupper(varandfile.first);
        for(auto& c : varupper)
            if(c != '_')
                c = c-32;
        ifstream filestream(varandfile.second.c_str());
        stringstream contents;
        contents << filestream.rdbuf();
        filestream.close();
        (void)remove(varandfile.second.c_str());
        string escapedcontents;
        for(auto& c : contents.str())
        {
            if(c=='\\')
                escapedcontents += "\\\\";
            else if(c=='"')
                escapedcontents += "\\\"";
            else if(c=='\n')
                continue;
            else
                escapedcontents += c;
        }
        cout << "\"" << varupper << "\": \"" << escapedcontents << "\"";
    }
    cout << "}" << endl;
    return 0;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
