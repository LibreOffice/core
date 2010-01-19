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

#ifndef optionhelper_hxx
#define optionhelper_hxx

#include <string>

#include <sal/types.h>

#include "testshl/nocopy.hxx"
#include "testshl/getopt.hxx"
#include "testshl/joblist.hxx"

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


