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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_testshl2.hxx"
#include <iostream>
#include <string>
#include <rtl/string.hxx>

using namespace std;

const char* g_cSeparatorList = " \t,;:#";

std::string trim(std::string const& _sStringToTrim)
{
    rtl::OString sStr(_sStringToTrim.c_str());
    sStr = sStr.trim();
    return std::string(sStr.getStr());
}

bool isInCurrentEnvironment(std::string const& _sString)
{
    int nSpace = 0;
    std::string sSearchString = _sString;
    bool bEnd = false;
    // run through the hole given string,
    std::string sEnv;

    do {
        nSpace = sSearchString.find_first_of(g_cSeparatorList);
        if (nSpace < 0)
        {
            sEnv = sSearchString;
            bEnd = true;
        }
        else
        {
            sEnv = sSearchString.substr(0, nSpace);
            sSearchString = sSearchString.substr(nSpace + 1);
        }
        if (sEnv.length() > 0)
        {
            cout << sEnv << endl;
#ifdef LINUX
            int nLinux = sEnv.find("unxlng");
            if (nLinux >= 0)
            {
                // found unxlng
                return true;
            }
#endif
#ifdef SOLARIS
            int nSolaris = sEnv.find("unxsol");
            if (nSolaris >= 0)
            {
                // found unxsol
                return true;
            }
#endif
#ifdef WNT
            int nWindows = sEnv.find("wnt");
            if (nWindows >= 0)
            {
                // found wnt
                return true;
            }
#endif
        }
    } while ( !bEnd );
    return false;
}

// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int /* argc */, char* /* argv */ [] )
#else
    int _cdecl main( int /* argc */ , char* /* argv */ [] )
#endif
{
    string sLine1 = "osl_Socket.ctor.ctors_001";
    string sLine = "osl_Socket.ctor.ctors_001 unxlngi5.pro unxsols4.pro wntmsci   ";

    int nSpace = sLine.find_first_of(g_cSeparatorList);
    cout << "Space: " << nSpace << endl;
    std::string sTest;

    if (nSpace > 0)
    {
        sLine = trim(sLine);
        // check environment
        sTest = sLine.substr(0, nSpace);
        std::string sRest = sLine.substr(nSpace + 1);
        if (isInCurrentEnvironment(sRest))
        {
            cout << "new string: " << sTest << endl;
        }
    }
    else
    {
        // every environment
        cout << "new string: " << sLine << endl;
    }

    // cout << "rest string: " << sRest << endl;

    int nCount = 10;
    do {
        cout << nCount << endl;
        nCount --;
    }
    while (nCount >= 0);
    cout << nCount << endl;
}
