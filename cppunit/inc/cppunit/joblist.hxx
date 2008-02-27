/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: joblist.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:12:01 $
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

#ifndef CPPUNIT_JOBLIST_HXX
#define CPPUNIT_JOBLIST_HXX

#include <vector>
#include <string>
#include <hash_map>

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

// #include <cppunit/nocopy.hxx>

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

