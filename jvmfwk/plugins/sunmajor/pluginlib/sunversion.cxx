/*************************************************************************
 *
 *  $RCSfile: sunversion.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 11:52:17 $
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

#include "sunversion.hxx"
#include "osl/thread.h"
#include "osl/process.h"
#include "osl/security.hxx"
#include <string.h>
#include <ctype.h>

using namespace rtl;
using namespace osl;
namespace jfw_plugin  { //stoc_javadetect

//extern OUString ::Impl::usPathDelim();
#define OUSTR( x )  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( x ))
#ifdef SUNVERSION_SELFTEST
class SelfTest
{
public:
    SelfTest();
} test;
#endif

SunVersion::SunVersion():  m_nUpdateSpecial(0),
                           m_preRelease(Rel_NONE),
                           m_bValid(false)
{
    memset(m_arVersionParts, 0, sizeof(m_arVersionParts));
}

SunVersion::SunVersion(const rtl::OUString &usVer):
    m_nUpdateSpecial(0), m_preRelease(Rel_NONE),
    usVersion(usVer)
{
    memset(m_arVersionParts, 0, sizeof(m_arVersionParts));
    rtl::OString sVersion= rtl::OUStringToOString(usVer, osl_getThreadTextEncoding());
    m_bValid = init(sVersion.getStr());
}
SunVersion::SunVersion(const char * szVer):
    m_nUpdateSpecial(0), m_preRelease(Rel_NONE)
{
    memset(m_arVersionParts, 0, sizeof(m_arVersionParts));
    m_bValid = init(szVer);
    usVersion= rtl::OUString(szVer,strlen(szVer),osl_getThreadTextEncoding());
}


/**Format major.minor.maintainance_update
 */
bool SunVersion::init(const char *szVersion)
{
    if ( ! szVersion || strlen(szVersion) == 0)
        return false;

    //first get the major,minor,maintainance
    const char * pLast = szVersion;
    const char * pCur = szVersion;
    //pEnd point to the position after the last character
    const char * pEnd = szVersion + strlen(szVersion);
    // 0 = major, 1 = minor, 2 = maintainance, 3 = update
    int nPart = 0;
    // position within part beginning with 0
    int nPartPos = 0;
    char buf[128];

    //char must me a number 0 - 999 and no leading
    char cCur = *pCur;
    while (1)
    {
        if (pCur < pEnd && isdigit(*pCur))
        {
            if (pCur < pEnd)
                pCur ++;
            nPartPos ++;
        }
        //if  correct separator then form integer
        else if (
            ! (nPartPos == 0) // prevents: ".4.1", "..1", part must start with digit
            && (
                //seperators after maintainance (1.4.1_01, 1.4.1-beta, or1.4.1
                (pCur == pEnd || *pCur == '_' || *pCur == '-')
                && (nPart == 2 )
                ||
                //separators between major-minor and minor-maintainance
                (nPart < 2 && *pCur == '.') )
            && (
                //prevent 1.4.0. 1.4.0-
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

            //check next character
            if (! ( (pCur < pEnd)
                    && ( (nPart < 3) && isdigit(*pCur)))) //(*pCur >= 48 && *pCur <=57))))
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
        int nUpdatePart = 0;
        while (1)
        {
            if (pCur < pEnd && isdigit(*pCur))
            {
                if (pCur < pEnd)
                    pCur ++;
                //   nPartPos ++;
            }
            else if (nUpdatePart == 0 && (pCur == pEnd || isalpha(*pCur)))
            {
                int len = pCur - pLast;
                if (len >= 127)
                    return false;

                strncpy(buf, pLast, len);
                buf[len] = 0;
                m_arVersionParts[nPart] = atoi(buf);
                nUpdatePart ++;

                //_01a, only on character at last position
                if (pCur < pEnd && isalpha(*pCur))
                {
                    //this must be the last char
                    if (! (pCur + 1 == pEnd))
                        return false;
                    if (isupper(*pCur))
                        m_nUpdateSpecial = *pCur + 0x20; //make lowercase
                    else
                        m_nUpdateSpecial = *pCur;

                }
                break;
            }
            else
            {
                return false;
            }
        }
    }
    else if (*(pCur - 1) == '-')
    {
        if( ! strcmp(pCur,"ea"))
            m_preRelease = Rel_EA;
        else if( ! strcmp(pCur,"ea1"))
            m_preRelease = Rel_EA1;
        else if( ! strcmp(pCur,"ea2"))
            m_preRelease = Rel_EA2;
        else if( ! strcmp(pCur,"ea3"))
            m_preRelease = Rel_EA3;
        else if ( ! strcmp(pCur,"beta"))
            m_preRelease = Rel_BETA;
        else if ( ! strcmp(pCur,"beta1"))
            m_preRelease = Rel_BETA1;
        else if ( ! strcmp(pCur,"beta2"))
            m_preRelease = Rel_BETA2;
        else if ( ! strcmp(pCur,"beta3"))
            m_preRelease = Rel_BETA3;
        else if (! strcmp(pCur, "rc"))
            m_preRelease = Rel_RC;
        else if (! strcmp(pCur, "rc1"))
            m_preRelease = Rel_RC1;
        else if (! strcmp(pCur, "rc2"))
            m_preRelease = Rel_RC2;
        else if (! strcmp(pCur, "rc3"))
            m_preRelease = Rel_RC3;
        else
            return false;
    }
    else
    {
        return false;
    }
    return true;
}

SunVersion::~SunVersion()
{
//    delete[] strVersion;
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

    //compare major.minor.maintainance
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
    //major.minor.maintainance_update are equal. test for a trailing char
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
    if (m_preRelease > ver.m_preRelease)
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


#ifdef SUNVERSION_SELFTEST
SelfTest::SelfTest()
{
    bool bRet = true;

    char * versions[] = {"1.4.0", "1.4.1", "1.0.0", "10.0.0", "10.10.0",
                         "10.2.2", "10.10.0", "10.10.10", "111.0.999",
                         "1.4.1_01", "9.90.99_09", "1.4.1_99",
                         "1.4.1_00a", "1.4.0_01z", "1.4.1_99A",
                         "1.4.1-ea", "1.4.1-beta", "1.4.1-rc1"};
    char * badVersions[] = {".4.0", "..1", "", "10.0", "10.10.0.", "10.10.0-", "10.10.0.",
                            "10.2-2", "10_10.0", "10..10","10.10", "a.0.999",
                            "1.4b.1_01", "9.90.-99_09", "1.4.1_99-",
                            "1.4.1_00a2", "1.4.0_z01z", "1.4.1__99A",
                            "1.4.1-1ea"};
    char * orderedVer[] = { "1.3.1-ea", "1.3.1-beta", "1.3.1-rc1",
                            "1.3.1", "1.3.1_00a", "1.3.1_01", "1.3.1_01a", "1.3.1_01b",
                            "1.3.2", "1.4.0", "2.0.0"};

    size_t num = sizeof (versions) / sizeof(char*);
    size_t numBad = sizeof (badVersions) / sizeof(char*);
    size_t numOrdered = sizeof (orderedVer) / sizeof(char*);
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
    for ( i = 0; i < numBad; i++)
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
    for (i = 0; i < numOrdered; i ++)
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
    OSL_ENSURE(bRet, "SunVersion self test failed");

}
#endif

}
