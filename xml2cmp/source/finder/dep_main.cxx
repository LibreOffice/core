/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


