/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optionhelper.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:13:38 $
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

#ifndef optionhelper_hxx
#define optionhelper_hxx

#include <string>

#include <sal/types.h>

#include "cppunit/nocopy.hxx"
#include "testshl/getopt.hxx"
#include "cppunit/joblist.hxx"

// -----------------------------------------------------------------------------

typedef std::vector<rtl::OString> OStringList;

//!? Better: OptionHelper
class OptionHelper : NOCOPY
{
    GetOpt & m_aOption;
    JobList     m_aJobOnlyList;
    JobList     m_aJobExcludeList;
    OStringList m_aJobFilter;

    std::string m_sProjectId;
    std::string m_sBuildId;

    std::string getProjectId() const;
    std::string getBuildId() const;
    std::string createDateTag(std::string const& _sProjectId, std::string const& _sBuildId);

    void handleJobs();
public:
    OptionHelper(GetOpt & _aOption)
            :m_aOption(_aOption)
        {
            if (m_aOption.hasOpt("-projectid"))
                m_sProjectId = m_aOption.getOpt("-projectid");

            if (m_aOption.hasOpt("-buildid"))
                m_sBuildId = m_aOption.getOpt("-buildid");

            handleJobs();
        }

    static std::string integerToAscii(sal_Int32 nValue);
    static std::string twoDigits(std::string const& _sValue);

    std::string createDateTag();
    bool showErrors();
    bool showTests();

    JobList getJobOnlyList() {return m_aJobOnlyList;}
    JobList getJobExcludeList() {return m_aJobExcludeList;}

    bool isAllowedToExecute(std::string const& _sNode, std::string const& _sName);

    bool isOnlyShowJobs() {return m_aOption.hasOpt("-onlyshowjobs") == sal_True ? true : false;}
    GetOpt& getOptions() {return m_aOption;}
    bool isVerbose() {return m_aOption.hasOpt("-verbose") == sal_True ? true : false;}
    bool isOptionWhereAmI() {return m_aOption.hasOpt("-whereami") == sal_True ? true : false;}
};

// -----------------------------------------------------------------------------


#endif


