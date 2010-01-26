/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
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

#ifndef CPPUNIT_JOBLIST_HXX
#define CPPUNIT_JOBLIST_HXX

#include <vector>
#include <string>
#include <hash_map>
#include "sal/types.h"

// #include <testshl/nocopy.hxx>

/*
  struct eqstr
  {
  bool operator()( std::string const & s1, std::string const& s2 ) const
  {
  return ( s1 == s2 );
  }
  };
*/

/*
  struct hashstr
  {
  sal_uInt32 operator()( std::string const & str ) const
  {
  return str.hashCode();
  }
  };
*/
typedef std::hash_map< std::string, int > HashMap;

// typedef std::vector<std::string> StringList;

#define JOB_UNKNOWN   0
#define JOB_NOT_FOUND 1
#define JOB_PASSED    2
#define JOB_FAILED    3
#define JOB_ACCESS    4

#define JOB_EXCLUDE_LIST 0x1000
#define JOB_ONLY_LIST    0x1001

typedef sal_Int16 JobType;

class JobList /* : NOCOPY */
{
    HashMap m_aJobList;
    char** ppsJobList;

    // returns true if the given List contains unxlngi or unxsols or wntmsci...
    bool isInCurrentEnvironment(std::string const& _sString);
    std::string trim(std::string const& _sStringToTrim);

public:
    JobList();
    ~JobList();

    bool readfile(std::string const& _sFilename, JobType _nJobType);
    const char** getList() const {return (const char**)ppsJobList;}

    int getJobListEntry(std::string const& _sIndexName);
    void setJobListEntry(std::string const& _sIndexName, int _nValue);

    int size() {return m_aJobList.size();}
    HashMap const& getHashMap() {return m_aJobList;}
};

#endif

