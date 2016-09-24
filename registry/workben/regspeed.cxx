/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/types.h>

#include "registry/registry.h"
#include <rtl/ustring.hxx>
#include <rtl/alloc.h>

using namespace std;

/*****************************************************************
    MyTimer
*****************************************************************/
#ifndef UNX
class MyTimer
{
public:
    void start() { _ftime( &m_start ); };
    void stop()
    {
        _ftime( &m_stop );
        m_diff = difftime(m_stop.time, m_start.time);
        if (m_stop.millitm > m_start.millitm)
        {
            m_diff += ((double)(m_stop.millitm - m_start.millitm)) / 1000;
        }
        else
        {
            m_diff -= 1;
            m_diff += ((double)(1000 - (m_start.millitm - m_stop.millitm))) / 1000;
        }
        printf(" %.4f Sekunden\n", m_diff);
    };

protected:
    struct _timeb m_start, m_stop;
    double m_diff;
};
#else
extern "C" int ftime(struct timeb *pt);

class MyTimer
{
public:
    void start() { ftime( &m_start ); };
    void stop()
    {
        ftime( &m_stop );
        m_diff = difftime(m_stop.time, m_start.time);
        if (m_stop.millitm > m_start.millitm)
        {
            m_diff += ((double)(m_stop.millitm - m_start.millitm)) / 1000;
        }
        else
        {
            m_diff -= 1;
            m_diff += ((double)(1000 - (m_start.millitm - m_stop.millitm))) / 1000;
        }
        printf(" %.4f Sekunden\n", m_diff);
    };

protected:
    struct timeb m_start, m_stop;
    double m_diff;
};
#endif


#if (defined UNX)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
{
    RegHandle       hReg;
    RegKeyHandle    hRootKey, hKey, hSubKey, hSubSubKey;
    OUString        sName1("regkey");
    OUString        sName2("regSubkey");
    OUString        sName3("regSubSubkey");
    OUString        keyName1;
    OUString        keyName2;
    OUString        keyName3;
    int             S1 = 10;
    int             S2 = 10;
    int             S3 = 10;
    MyTimer         aTimer;

    if (argc < 4)
    {
        cerr << "using regspeed count1 count2 count3\n";
        exit(1);
    }

    S1 = atoi(argv[1]);
    S2 = atoi(argv[2]);
    S3 = atoi(argv[3]);

    OUString speedReg( "speed.reg");
    if (reg_createRegistry(speedReg.pData, &hReg))
    {
        cout << "creating registry \"test.reg\" failed\n";
    } else
    {
        if (reg_openRootKey(hReg, &hRootKey))
        {
            cout << "open root key \"test.reg\" failed\n";
        } else
        {
            printf("\n %d keys anlegen, oeffnen und schliessen dauert ... ", (S1 * S2 * S3));
            aTimer.start();

            for (sal_Int32 i=0; i < S1; i++)
            {
                keyName1 = sName1;
                keyName1 += OUString().valueOf(i);
                if (reg_createKey(hRootKey, keyName1.pData, &hKey))
                    cout << "creating key \"" << OUStringToOString(keyName1, RTL_TEXTENCODING_ASCII_US).getStr()
                         << "\" failed\n";

                for (sal_Int32 j=0; j < S2; j++)
                {
                    keyName2 = sName2;
                    keyName2 += OUString().valueOf(j);
                    if (reg_createKey(hKey, keyName2.pData, &hSubKey))
                        cout << "creating key \"" << OUStringToOString(keyName2, RTL_TEXTENCODING_ASCII_US).getStr()
                             << "\" failed\n";

                        for (sal_Int32 n=0; n < S3; n++)
                        {
                            keyName3 = sName3;
                            keyName3 += OUString().valueOf(n);
                            if (reg_createKey(hSubKey, keyName3.pData, &hSubSubKey))
                                cout << "creating key \"" << OUStringToOString(keyName3, RTL_TEXTENCODING_ASCII_US).getStr()
                                     << "\" failed\n";

                            if (reg_closeKey(hSubSubKey))
                                cout << "closing key \"" << OUStringToOString(keyName3, RTL_TEXTENCODING_ASCII_US).getStr()
                                     << "\" failed\n";
                        }

                    if (reg_closeKey(hSubKey))
                        cout << "closing key \"" << OUStringToOString(keyName2, RTL_TEXTENCODING_ASCII_US).getStr()
                             << "\" failed\n";
                }

                if (reg_closeKey(hKey))
                    cout << "closing key \"" << OUStringToOString(keyName1, RTL_TEXTENCODING_ASCII_US).getStr()
                         << "\" failed\n";
            }

            aTimer.stop();

            printf("\n %d keys oeffnen und schliessen dauert ... ", (S1 * S2 * S3));
            aTimer.start();

            for (sal_Int32 i=0; i < S1; i++)
            {
                keyName1 = "/";
                keyName1 += sName1;
                keyName1 += OUString().valueOf(i);
                if (reg_openKey(hRootKey, keyName1.pData, &hKey))
                    cout << "open key \"" << OUStringToOString(keyName1, RTL_TEXTENCODING_ASCII_US).getStr()
                         << "\" failed\n";

                for (sal_Int32 j=0; j < S2; j++)
                {
                    keyName2 = "/";
                    keyName2 += sName1;
                    keyName2 += OUString().valueOf(i);
                    keyName2 += "/";
                    keyName2 += sName2;
                    keyName2 += OUString().valueOf(j);
                    if (reg_openKey(hRootKey, keyName2.pData, &hSubKey))
                        cout << "open key \"" << OUStringToOString(keyName2, RTL_TEXTENCODING_ASCII_US).getStr()
                             << "\" failed\n";

                        for (sal_Int32 n=0; n < S3; n++)
                        {
                            keyName3 = "/";
                            keyName3 += sName1;
                            keyName3 += OUString().valueOf(i);
                            keyName3 += "/";
                            keyName3 += sName2;
                            keyName3 += OUString().valueOf(j);
                            keyName3 += "/";
                            keyName3 += sName3;
                            keyName3 += OUString().valueOf(n);
                            if (reg_openKey(hRootKey, keyName3.pData, &hSubSubKey))
                                cout << "open key \"" << OUStringToOString(keyName3, RTL_TEXTENCODING_ASCII_US).getStr()
                                     << "\" failed\n";

                            if (reg_closeKey(hSubSubKey))
                                cout << "open key \"" << OUStringToOString(keyName3, RTL_TEXTENCODING_ASCII_US).getStr()
                                     << "\" failed\n";
                        }

                    if (reg_closeKey(hSubKey))
                        cout << "closing key \"" << OUStringToOString(keyName2, RTL_TEXTENCODING_ASCII_US).getStr()
                             << "\" failed\n";
                }

                if (reg_closeKey(hKey))
                    cout << "closing key \"" << OUStringToOString(keyName1, RTL_TEXTENCODING_ASCII_US).getStr()
                         << "\" failed\n";
            }

            aTimer.stop();

            printf("\n 1 key oeffnen und schliessen dauert ... ");
            aTimer.start();

            if (reg_openKey(hRootKey, keyName3.pData, &hSubSubKey))
                cout << "open key \"" << OUStringToOString(keyName3, RTL_TEXTENCODING_ASCII_US).getStr()
                     << "\" failed\n";

            if (reg_closeKey(hSubSubKey))
                cout << "open key \"" << OUStringToOString(keyName3, RTL_TEXTENCODING_ASCII_US).getStr()
                     << "\" failed\n";

            aTimer.stop();

        }

        if (reg_closeKey(hRootKey))
            cout << "closing root key failed\n";
        if (reg_closeRegistry(hReg))
            cout << "\t41. closing registry \"test.reg\" failed\n";
    }

    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
