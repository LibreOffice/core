/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: joblist.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:22:52 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppunit.hxx"

#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <rtl/string.hxx>
#include "cppunit/joblist.hxx"

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
