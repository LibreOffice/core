/*************************************************************************
 *
 *  $RCSfile: dep_main.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2001-03-23 13:32:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <iostream>

#include "dependy.hxx"


using namespace std;



int
#ifdef WNT
_cdecl
#endif
main( int       argc,
      char *    argv[] )
{
    if (argc < 2 || *argv[1] == '?')
    {
        cout << "\nUse:" << endl
             << "srvdepy.exe  <xml-component-descriptions-root-directory>"
             << endl << endl;
        return 0;
    }


    DependencyFinder aDependencies;

    aDependencies.GatherData(argv[1]);
    char sInput[500] = "";
    std::vector<Simstr>  aLibs;
    std::vector<Simstr>  aServs;


    cout << "\nNow you can start to put in Service names." << endl
         << "Please use correct case, but don't use namespaces." << endl
         << "Just the Service's own name." << endl << endl
         << "To stop the program, put in a hashmark \"#\" + ENTER." << endl << endl;



    do {

        sInput[0] = 0;
        cin >> sInput;
        Simstr sImplService(sInput);
        if (*sInput != '#')
        {
            aLibs.erase( aLibs.begin(), aLibs.end() );
            aServs.erase( aServs.begin(), aServs.end() );

            aDependencies.FindNeededServices( aLibs, aServs, sImplService );

            cout << "\n\n\nNeeded libraries: " << endl;
            for ( unsigned i = 0; i < aLibs.size(); ++i )
            {
                cout << "    " << aLibs[i].str() << endl;
            }
            cout << "\nNeeded services: " << endl;
            for ( unsigned s= 0; s < aServs.size(); ++s )
            {
                cout << "    " << aServs[s].str() << endl;
            }
            cout << endl << endl << endl;
        }
    }   while (*sInput != '#');

    return 0;
}


