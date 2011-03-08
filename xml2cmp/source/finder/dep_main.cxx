/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <iostream>

#include "dependy.hxx"


int
#ifdef WNT
_cdecl
#endif
main( int       argc,
      char *    argv[] )
{
    if (argc < 2 || *argv[1] == '?')
    {
        std::cout << "\nUse:\n"
             << "srvdepy.exe  <xml-component-descriptions-root-directory>\n"
             << std::endl;
        return 0;
    }


    DependencyFinder aDependencies;

    aDependencies.GatherData(argv[1]);
    char sInput[500] = "";
    std::vector<Simstr>  aLibs;
    std::vector<Simstr>  aServs;


    std::cout
         << "\nNow you can start to put in Service names.\n"
         << "Please use correct case, but don't use namespaces.\n"
         << "Just the Service's own name.\n\n"
         << "To stop the program, put in a hashmark \"#\" + ENTER.\n"
         << std::endl;



    do {

        sInput[0] = 0;
        std::cin >> sInput;
        Simstr sImplService(sInput);
        if (*sInput != '#')
        {
            aLibs.erase( aLibs.begin(), aLibs.end() );
            aServs.erase( aServs.begin(), aServs.end() );

            aDependencies.FindNeededServices( aLibs, aServs, sImplService );

            std::cout << "\n\n\nNeeded libraries: " << std::endl;
            for ( unsigned i = 0; i < aLibs.size(); ++i )
            {
                std::cout << "    " << aLibs[i].str() << std::endl;
            }
            std::cout << "\nNeeded services: " << std::endl;
            for ( unsigned s= 0; s < aServs.size(); ++s )
            {
                std::cout << "    " << aServs[s].str() << std::endl;
            }
            std::cout << "\n\n" << std::endl;
        }
    }   while (*sInput != '#');

    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
