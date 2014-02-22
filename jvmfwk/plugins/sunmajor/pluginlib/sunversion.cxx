/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "sunversion.hxx"
#include "osl/thread.h"
#include "osl/process.h"
#include "osl/security.hxx"
#include <string.h>
#include <ctype.h>
#include "diagnostics.h"
using namespace osl;

namespace jfw_plugin  { 


#if OSL_DEBUG_LEVEL >= 2
class SelfTest
{
public:
    SelfTest();
} test;
#endif

SunVersion::SunVersion(const OUString &usVer):
    m_nUpdateSpecial(0), m_preRelease(Rel_NONE),
    usVersion(usVer)
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
    usVersion= OUString(szVer,strlen(szVer),osl_getThreadTextEncoding());
}


/**Format major.minor.maintainance_update
 */
bool SunVersion::init(const char *szVersion)
{
    if ( ! szVersion || strlen(szVersion) == 0)
        return false;

    
    const char * pLast = szVersion;
    const char * pCur = szVersion;
    
    const char * pEnd = szVersion + strlen(szVersion);
    
    int nPart = 0;
    
    int nPartPos = 0;
    char buf[128];

    
    while (true)
    {
        if (pCur < pEnd && isdigit(*pCur))
        {
            if (pCur < pEnd)
                pCur ++;
            nPartPos ++;
        }
        
        else if (
            ! (nPartPos == 0) 
            && (
                
                ((pCur == pEnd || *pCur == '_' || *pCur == '-') && (nPart == 2 ))
                ||
                
                (nPart < 2 && *pCur == '.') )
            && (
                
                pCur + 1 == pEnd ? isdigit(*(pCur)) : 1) )
        {
            int len = pCur - pLast;
            if (len >= 127)
                return false;

            strncpy(buf, pLast, len);
            buf[len] = 0;
            pCur ++;
            pLast = pCur;

            m_arVersionParts[nPart] = atoi(buf);
            nPart ++;
            nPartPos = 0;
            if (nPart == 3)
                break;

            
            if (! ( (pCur < pEnd)
                    && ( (nPart < 3) && isdigit(*pCur))))
                return false;
        }
        else
        {
            return false;
        }
    }
    if (pCur >= pEnd)
        return true;
    
    
    
    if (* (pCur - 1) == '_')
    {
        
        while (true)
        {
            if (pCur <= pEnd)
            {
                if ( ! isdigit(*pCur))
                {
                    
                    int len = pCur - pLast;
                    if (len > 2)
                        return false;
                    
                    strncpy(buf, pLast, len);
                    buf[len] = 0;
                    m_arVersionParts[nPart] = atoi(buf);
                    if (pCur == pEnd)
                    {
                        break;
                    }
                    if (*pCur == 'a' && (pCur + 1) == pEnd)
                    {
                        
                        m_nUpdateSpecial = *pCur;
                        break;
                    }
                    else if (*pCur == '-' && pCur < pEnd)
                    {
                        
                        PreRelease pr = getPreRelease(++pCur);
                        if (pr == Rel_NONE)
                            return false;
                        
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
    
    else if (*(pCur - 1) == '-')
    {
        m_preRelease = getPreRelease(pCur);
        if (m_preRelease == Rel_NONE)
            return false;
#if defined(FREEBSD)
      if (m_preRelease == Rel_FreeBSD)
      {
          pCur++; 
          if (pCur < pEnd && isdigit(*pCur))
              pCur ++;
          int len = pCur - pLast -1; 
          if (len >= 127)
              return false;
          strncpy(buf, (pLast+1), len); 
          buf[len] = 0;
          m_nUpdateSpecial = atoi(buf)+100; 
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
    if (szRelease == NULL)
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

    
    for( int i= 0; i < 4; i ++)
    {
        
        if(m_arVersionParts[i] > ver.m_arVersionParts[i])
        {
            return true;
        }
        else if (m_arVersionParts[i] < ver.m_arVersionParts[i])
        {
            return false;
        }
    }
    
    if (m_nUpdateSpecial > ver.m_nUpdateSpecial)
    {
        return true;
    }

    
    
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

SunVersion::operator bool()
{
    return m_bValid;
}

#if OSL_DEBUG_LEVEL >= 2
SelfTest::SelfTest()
{
    bool bRet = true;

    char const * versions[] = {"1.4.0", "1.4.1", "1.0.0", "10.0.0", "10.10.0",
                         "10.2.2", "10.10.0", "10.10.10", "111.0.999",
                         "1.4.1_01", "9.90.99_09", "1.4.1_99",
                         "1.4.1_00a",
                         "1.4.1-ea", "1.4.1-beta", "1.4.1-rc1",
                         "1.5.0_01-ea", "1.5.0_01-rc2"};
    char const * badVersions[] = {".4.0", "..1", "", "10.0", "10.10.0.", "10.10.0-", "10.10.0.",
                            "10.2-2", "10_10.0", "10..10","10.10", "a.0.999",
                            "1.4b.1_01", "9.90.-99_09", "1.4.1_99-",
                            "1.4.1_00a2", "1.4.0_z01z", "1.4.1__99A",
                            "1.4.1-1ea", "1.5.0_010", "1.5.0._01-", "1.5.0_01-eac"};
    char const * orderedVer[] = { "1.3.1-ea", "1.3.1-beta", "1.3.1-rc1",
                            "1.3.1", "1.3.1_00a", "1.3.1_01", "1.3.1_01a",
                            "1.3.2", "1.4.0", "1.5.0_01-ea", "2.0.0"};

    int num = sizeof (versions) / sizeof(char*);
    int numBad = sizeof (badVersions) / sizeof(char*);
    int numOrdered = sizeof (orderedVer) / sizeof(char*);
    
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
        JFW_TRACE2("[Java framework] sunjavaplugin: Testing class SunVersion succeeded.\n");
    else
        OSL_ENSURE(bRet, "[Java framework] sunjavaplugin: SunVersion self test failed.\n");
}
#endif

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
