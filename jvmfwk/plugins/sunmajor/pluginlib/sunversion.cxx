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


#include "sunversion.hxx"
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/process.h>
#include <osl/security.hxx>
#include <rtl/character.hxx>
#include <string.h>
#include "diagnostics.h"
using namespace osl;

namespace jfw_plugin  { //stoc_javadetect


#if OSL_DEBUG_LEVEL >= 2
class SelfTest
{
public:
    SelfTest();
} test;
#endif

SunVersion::SunVersion(const OUString &usVer):
    m_nUpdateSpecial(0), m_preRelease(Rel_NONE)
{
    memset(m_arVersionParts, 0, sizeof(m_arVersionParts));
    OString sVersion= OUStringToOString(usVer, osl_getThreadTextEncoding());
    m_bValid = init(sVersion.getStr());
}
SunVersion::SunVersion(const char * szVer):
    m_nUpdateSpecial(0), m_preRelease(Rel_NONE)
{
    memset(m_arVersionParts, 0, sizeof(m_arVersionParts));
    m_bValid = init(szVer);
}


/**Format major.minor.maintenance_update
 */
bool SunVersion::init(const char *szVersion)
{
    if (!szVersion || szVersion[0] == '\0')
        return false;

    //first get the major,minor,maintenance
    const char * pLast = szVersion;
    const char * pCur = szVersion;
    //pEnd point to the position after the last character
    const char * pEnd = szVersion + strlen(szVersion);
    // 0 = major, 1 = minor, 2 = maintenance, 3 = update
    int nPart = 0;
    // position within part beginning with 0
    int nPartPos = 0;
    char buf[128];

    //char must me a number 0 - 999 and no leading
    while (true)
    {
        if (pCur < pEnd && rtl::isAsciiDigit(static_cast<unsigned char>(*pCur)))
        {
            pCur ++;
            nPartPos ++;
        }
        //if  correct separator then form integer
        else if (
            (nPartPos != 0) // prevents: ".4.1", "..1", part must start with digit
            && (
                //separators after maintenance (1.4.1_01, 1.4.1-beta, or 1.4.1)
                (pCur == pEnd || *pCur == '_' || *pCur == '-')
                ||
                //separators between major-minor and minor-maintenance
                (nPart < 2 && *pCur == '.') )
            && (
                //prevent 1.4.0. 1.4.0-
                pCur + 1 != pEnd
                || rtl::isAsciiDigit(static_cast<unsigned char>(*pCur))) )
        {
            bool afterMaint = pCur == pEnd || *pCur == '_' || *pCur == '-';

            int len = pCur - pLast;
            if (len >= 127)
                return false;

            strncpy(buf, pLast, len);
            buf[len] = 0;
            pCur ++;
            pLast = pCur;

            m_arVersionParts[nPart] = atoi(buf);

            if (afterMaint)
                nPart = 2;
            nPart ++;
            nPartPos = 0;
            if (nPart == 3)
                break;

            //check next character
            if (! ( (pCur < pEnd)
                    && ( (nPart < 3)
                         && rtl::isAsciiDigit(
                             static_cast<unsigned char>(*pCur)))))
                return false;
        }
        else
        {
            return false;
        }
    }
    if (pCur >= pEnd)
        return true;
    //We have now 1.4.1. This can be followed by _01, -beta, etc.
    // _01 (update) According to docu must not be followed by any other
    //characters, but on Solaris 9 we have a 1.4.1_01a!!
    if (* (pCur - 1) == '_')
    {// _01, _02
        // update is the last part _01, _01a, part 0 is the digits parts and 1 the trailing alpha
        while (true)
        {
            if (pCur <= pEnd)
            {
                if ( ! rtl::isAsciiDigit(static_cast<unsigned char>(*pCur)))
                {
                    //1.8.0_102-, 1.8.0_01a,
                    size_t len = pCur - pLast;
                    if (len > sizeof(buf) - 1)
                        return false;
                    //we've got the update: 01, 02 etc
                    strncpy(buf, pLast, len);
                    buf[len] = 0;
                    m_arVersionParts[nPart] = atoi(buf);
                    if (pCur == pEnd)
                    {
                        break;
                    }
                    if (*pCur == 'a' && (pCur + 1) == pEnd)
                    {
                        //check if it s followed by a simple "a" (not specified)
                        m_nUpdateSpecial = *pCur;
                        break;
                    }
                    else if (*pCur == '-' && pCur < pEnd)
                    {
                        //check 1.5.0_01-ea
                        PreRelease pr = getPreRelease(++pCur);
                        if (pr == Rel_NONE)
                            return false;
                        //just ignore -ea because its no official release
                        break;
                    }
                    else
                    {
                        return false;
                    }
                }
                if (pCur < pEnd)
                    pCur ++;
                else
                    break;
            }
        }
    }
    // 1.4.1-ea
    else if (*(pCur - 1) == '-')
    {
        m_preRelease = getPreRelease(pCur);
        if (m_preRelease == Rel_NONE)
            return false;
#if defined(FREEBSD)
      if (m_preRelease == Rel_FreeBSD)
      {
          pCur++; //eliminate 'p'
          if (pCur < pEnd
              && rtl::isAsciiDigit(static_cast<unsigned char>(*pCur)))
              pCur ++;
          int len = pCur - pLast -1; //eliminate 'p'
          if (len >= 127)
              return false;
          strncpy(buf, (pLast+1), len); //eliminate 'p'
          buf[len] = 0;
          m_nUpdateSpecial = atoi(buf)+100; //hack for FBSD #i56953#
          return true;
      }
#endif
    }
    else
    {
        return false;
    }
    return true;
}

SunVersion::PreRelease SunVersion::getPreRelease(const char *szRelease)
{
    if (szRelease == nullptr)
        return Rel_NONE;
    if( ! strcmp(szRelease,"internal"))
        return  Rel_INTERNAL;
    else if( ! strcmp(szRelease,"ea"))
        return  Rel_EA;
    else if( ! strcmp(szRelease,"ea1"))
        return Rel_EA1;
    else if( ! strcmp(szRelease,"ea2"))
        return Rel_EA2;
    else if( ! strcmp(szRelease,"ea3"))
        return Rel_EA3;
    else if ( ! strcmp(szRelease,"beta"))
        return Rel_BETA;
    else if ( ! strcmp(szRelease,"beta1"))
        return Rel_BETA1;
    else if ( ! strcmp(szRelease,"beta2"))
        return Rel_BETA2;
    else if ( ! strcmp(szRelease,"beta3"))
        return Rel_BETA3;
    else if (! strcmp(szRelease, "rc"))
        return Rel_RC;
    else if (! strcmp(szRelease, "rc1"))
        return Rel_RC1;
    else if (! strcmp(szRelease, "rc2"))
        return Rel_RC2;
    else if (! strcmp(szRelease, "rc3"))
        return Rel_RC3;
#if defined (FREEBSD)
    else if (! strncmp(szRelease, "p", 1))
        return Rel_FreeBSD;
#endif
    else
        return Rel_NONE;
}

SunVersion::~SunVersion()
{

}

/* Examples:
   a) 1.0 < 1.1
   b) 1.0 < 1.0.0
   c)  1.0 < 1.0_00

   returns false if both values are equal
*/
bool SunVersion::operator > (const SunVersion& ver) const
{
    if( &ver == this)
        return false;

    //compare major.minor.maintenance
    for( int i= 0; i < 4; i ++)
    {
        // 1.4 > 1.3
        if(m_arVersionParts[i] > ver.m_arVersionParts[i])
        {
            return true;
        }
        else if (m_arVersionParts[i] < ver.m_arVersionParts[i])
        {
            return false;
        }
    }
    //major.minor.maintenance_update are equal. Test for a trailing char
    if (m_nUpdateSpecial > ver.m_nUpdateSpecial)
    {
        return true;
    }

    //Until here the versions are equal
    //compare pre -release values
    if ((m_preRelease == Rel_NONE && ver.m_preRelease == Rel_NONE)
        ||
        (m_preRelease != Rel_NONE && ver.m_preRelease == Rel_NONE))
        return false;
    else if (m_preRelease == Rel_NONE && ver.m_preRelease != Rel_NONE)
        return true;
    else if (m_preRelease > ver.m_preRelease)
        return true;

    return false;
}

bool SunVersion::operator < (const SunVersion& ver) const
{
    return (! operator > (ver)) && (! operator == (ver));
}

bool SunVersion::operator == (const SunVersion& ver) const
{
    bool bRet= true;
    for(int i= 0; i < 4; i++)
    {
        if( m_arVersionParts[i] != ver.m_arVersionParts[i])
        {
            bRet= false;
            break;
        }
    }
    bRet = m_nUpdateSpecial == ver.m_nUpdateSpecial && bRet;
    bRet = m_preRelease == ver.m_preRelease && bRet;
    return bRet;
}


#if OSL_DEBUG_LEVEL >= 2
SelfTest::SelfTest()
{
    bool bRet = true;

    static char const * versions[] = {"1.4.0", "1.4.1", "1.0.0", "10.0.0", "10.10.0",
                         "10.2.2", "10.10.0", "10.10.10", "111.0.999",
                         "1.4.1_01", "9.90.99_09", "1.4.1_99",
                         "1.4.1_00a",
                         "1.4.1-ea", "1.4.1-beta", "1.4.1-rc1",
                         "1.5.0_01-ea", "1.5.0_01-rc2"};
    static char const * badVersions[] = {".4.0", "..1", "", "10.0", "10.10.0.", "10.10.0-", "10.10.0.",
                            "10.2-2", "10_10.0", "10..10","10.10", "a.0.999",
                            "1.4b.1_01", "9.90.-99_09", "1.4.1_99-",
                            "1.4.1_00a2", "1.4.0_z01z", "1.4.1__99A",
                            "1.4.1-1ea", "1.5.0_010", "1.5.0._01-", "1.5.0_01-eac"};
    static char const * orderedVer[] = { "1.3.1-ea", "1.3.1-beta", "1.3.1-rc1",
                            "1.3.1", "1.3.1_00a", "1.3.1_01", "1.3.1_01a",
                            "1.3.2", "1.4.0", "1.5.0_01-ea", "2.0.0"};

    int num = SAL_N_ELEMENTS (versions);
    int numBad = SAL_N_ELEMENTS (badVersions);
    int numOrdered = SAL_N_ELEMENTS (orderedVer);
    //parsing test (positive)
    for (int i = 0; i < num; i++)
    {
        SunVersion ver(versions[i]);
        if ( ! ver)
        {
            bRet = false;
            break;
        }
    }
    OSL_ENSURE(bRet, "SunVersion selftest failed");
    //Parsing test (negative)
    for ( int i = 0; i < numBad; i++)
    {
        SunVersion ver(badVersions[i]);
        if (ver)
        {
            bRet = false;
            break;
        }
    }
    OSL_ENSURE(bRet, "SunVersion selftest failed");

    // Ordering test
    bRet = true;
    int j = 0;
    for (int i = 0; i < numOrdered; i ++)
    {
        SunVersion curVer(orderedVer[i]);
        if ( ! curVer)
        {
            bRet = false;
            break;
        }
        for (j = 0; j < numOrdered; j++)
        {
            SunVersion compVer(orderedVer[j]);
            if (i < j)
            {
                if ( !(curVer < compVer))
                {
                    bRet = false;
                    break;
                }
            }
            else if ( i == j)
            {
                if (! (curVer == compVer
                       && ! (curVer > compVer)
                       && ! (curVer < compVer)))
                {
                    bRet = false;
                    break;
                }
            }
            else if (i > j)
            {
                if ( !(curVer > compVer))
                {
                    bRet = false;
                    break;
                }
            }
        }
        if ( ! bRet)
            break;
    }
    if (bRet)
        JFW_TRACE2("Testing class SunVersion succeeded.");
    else
        OSL_ENSURE(bRet, "[Java framework] sunjavaplugin: SunVersion self test failed.");
}
#endif

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
