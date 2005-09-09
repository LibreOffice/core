/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dep_main.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 11:52:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


