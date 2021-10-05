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

int main(int argc, char** argv)
{
    const std::string optsintro("--");
    std::map<std::string, std::string> vartofile;
    for(int i=1; i < argc; ++i)
    {
        const std::string arg(argv[i]);
        if(arg.substr(0,2) != optsintro)
        {
            std::cerr << "Only option args starting with -- allowed." << std::endl;
            return 1;
        }
        const size_t eqpos = arg.find("=", 2);
        if(eqpos == std::string::npos)
        {
            std::cerr << "Only option args assigning with = allowed." << std::endl;
            return 2;
        }
        const std::string argname(arg.substr(2, eqpos-2));
        vartofile[argname] = arg.substr(eqpos+1, std::string::npos);
    }
    std::cout << "{";
    bool first(true);
    for(const auto& varandfile : vartofile)
    {
        if(first)
            first =false;
        else
            std::cout << "," << std::endl;
        std::string varupper(varandfile.first);
        for(auto& c : varupper)
            if(c != '_')
                c = c-32;
        std::ifstream filestream(varandfile.second.c_str());
        std::stringstream contents;
        contents << filestream.rdbuf();
        filestream.close();
        (void)remove(varandfile.second.c_str());
        std::string escapedcontents;
        for(const auto& c : contents.str())
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
        std::cout << "\"" << varupper << "\": \"" << escapedcontents << "\"";
    }
    std::cout << "}" << std::endl;
    return 0;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
