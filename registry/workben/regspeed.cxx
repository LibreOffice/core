/*************************************************************************
 *
 *  $RCSfile: regspeed.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:43 $
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

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/types.h>

#include "registry/registry.h"

#ifndef _RTL_USTRING_HXX_
#include    <rtl/ustring.hxx>
#endif

#ifndef _RTL_ALLOC_H_
#include    <rtl/alloc.h>
#endif


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
#ifdef OS2
    struct timeb m_start, m_stop;
#else
    struct _timeb m_start, m_stop;
#endif
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

using namespace rtl;

#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
{
    RegHandle       hReg;
    RegKeyHandle    hRootKey, hKey, hSubKey, hSubSubKey;
    OUString        sName1(RTL_CONSTASCII_USTRINGPARAM("regkey"));
    OUString        sName2(RTL_CONSTASCII_USTRINGPARAM("regSubkey"));
    OUString        sName3(RTL_CONSTASCII_USTRINGPARAM("regSubSubkey"));
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

    OUString speedReg( RTL_CONSTASCII_USTRINGPARAM("speed.reg"));
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

            for (i=0; i < S1; i++)
            {
                keyName1 = OUString::createFromAscii("/");
                keyName1 += sName1;
                keyName1 += OUString().valueOf(i);
                if (reg_openKey(hRootKey, keyName1.pData, &hKey))
                    cout << "open key \"" << OUStringToOString(keyName1, RTL_TEXTENCODING_ASCII_US).getStr()
                         << "\" failed\n";

                for (sal_Int32 j=0; j < S2; j++)
                {
                    keyName2 = OUString::createFromAscii("/");
                    keyName2 += sName1;
                    keyName2 += OUString().valueOf(i);
                    keyName2 += OUString::createFromAscii("/");
                    keyName2 += sName2;
                    keyName2 += OUString().valueOf(j);
                    if (reg_openKey(hRootKey, keyName2.pData, &hSubKey))
                        cout << "open key \"" << OUStringToOString(keyName2, RTL_TEXTENCODING_ASCII_US).getStr()
                             << "\" failed\n";

                        for (sal_Int32 n=0; n < S3; n++)
                        {
                            keyName3 = OUString::createFromAscii("/");
                            keyName3 += sName1;
                            keyName3 += OUString().valueOf(i);
                            keyName3 += OUString::createFromAscii("/");
                            keyName3 += sName2;
                            keyName3 += OUString().valueOf(j);
                            keyName3 += OUString::createFromAscii("/");
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

    return(0);
}


