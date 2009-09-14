/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 * $Revision$
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
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
// #include "precompiled_cppunit.hxx"

#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <rtl/string.hxx>
#include "testshl/joblist.hxx"

#define C_SEPARATOR_LIST " \t,;:#"
JobList::JobList()
        :ppsJobList(NULL)
{}

// -----------------------------------------------------------------------------
bool JobList::isInCurrentEnvironment(std::string const& _sString)
{
    int nSpace = 0;
    std::string sSearchString = _sString;
    bool bEnd = false;
    // run through the hole given string,
    std::string sEnv;

    do {
        nSpace = sSearchString.find_first_of(C_SEPARATOR_LIST);
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
            // cout << sEnv << endl;
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

std::string JobList::trim(std::string const& _sStringToTrim)
{
    rtl::OString sStr(_sStringToTrim.c_str());
    sStr = sStr.trim();
    return std::string(sStr.getStr());
}

// -----------------------------------------------------------------------------
bool JobList::readfile(std::string const& _sFilename, JobType _nJobType)
{
    // StringList aJobList;
    m_aJobList.clear();

    std::ifstream in(_sFilename.c_str(), std::ios::in);

    if (!in)
    {
        // fprintf(stderr, "warning: Can't open job file: %s\n", _sFilename.c_str());
        return false;
    }
    else
    {
        if (_nJobType == JOB_EXCLUDE_LIST)
        {
            fprintf(stderr, "warning: There exist a job exclusion list file: %s\n", _sFilename.c_str());
        }

        // std::cout << "Check for signal" << std::endl;
        std::string sLine;
        while (std::getline(in, sLine))
        {
            if (sLine.size() > 0)
            {
                char ch = sLine[0];
                if (ch != '#' &&
                    isspace(ch) == 0)
                {
                    sLine = trim(sLine);

                    // m_aJobList.push_back( sLine );
                    int nSpace = sLine.find_first_of(C_SEPARATOR_LIST);
                    if (nSpace < 0)
                    {
                        m_aJobList[sLine] = JOB_UNKNOWN;
                    }
                    else
                    {
                        // found a separator
                        std::string sTest = sLine.substr(0, nSpace);
                        std::string sRest = sLine.substr(nSpace + 1);
                        if (isInCurrentEnvironment(sRest))
                        {
                            m_aJobList[sTest] = JOB_UNKNOWN;
                        }
                    }
                }
            }
        }

        // inclusion list
/*
  int nSize = m_aJobList.size();

  ppsJobList = (char**) malloc((nSize + 1) * sizeof(char*));

  int nIdx = 0;
  StringList::const_iterator aItEnd = m_aJobList.end();
  for(StringList::const_iterator it = m_aJobList.begin();
  it != aItEnd; ++it)
  {
  std::string sValue = *it;
  char* psTestFkt = (char*) malloc(sValue.size() + 1);
  strcpy(psTestFkt, sValue.c_str());
  ppsJobList[nIdx] = psTestFkt;
  ++nIdx;
  }

  ppsJobList[nIdx] = NULL;
*/
    }
    return true;
}

// -----------------------------------------------------------------------------
int JobList::getJobListEntry(std::string const& _sIndexName)
{
    if (m_aJobList.find(_sIndexName) != m_aJobList.end())
    {
        return m_aJobList[_sIndexName];
    }
    return JOB_NOT_FOUND;
}

// -----------------------------------------------------------------------------
void JobList::setJobListEntry(std::string const& _sIndexName, int _nValue)
{
    // if (m_aJobList.find(_sIndexName) == m_aJobList.end())
    // {
    m_aJobList[_sIndexName] = _nValue;
    // }
    // else
    // {
    //  fprintf(stderr, "error: test function '%s' already exist.\n", _sIndexName);
    // }
}

// -----------------------------------------------------------------------------
JobList::~JobList()
{
    int nIdx = 0;
    if (ppsJobList)
    {
        while(ppsJobList[nIdx] != NULL)
        {
            free(ppsJobList[nIdx]);
            ++nIdx;
        }
        free(ppsJobList);
    }
}
// -----------------------------------------------------------------------------
/*
  bool JobList::checkFilter(JobList m_aJobFilter, std::string const& _sNodeName, std::string const& _sName)
  {
  std::string sFilter = m_aJobFilter.m_aJobList.begin();
  }
*/
