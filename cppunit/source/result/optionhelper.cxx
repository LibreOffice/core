/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optionhelper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:25:08 $
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

#include "cppunit/result/optionhelper.hxx"
// #include <sstream>
// #include <string>
#include <osl/time.h>
#include "cppunit/cmdlinebits.hxx"

// -----------------------------------------------------------------------------

namespace
{
    void split( const rtl::OString& opt,
                const rtl::OString& _sSeparator,
                OStringList& optLine )
    {
        optLine.clear();
        // const sal_Int32 cSetLen = cSet.getLength();
        sal_Int32 index = 0;
        sal_Int32 oldIndex = 0;

        // sal_Int32 i;
        // sal_Int32 j = 0;
        while ( opt.getLength() > 0)
        {
            // for ( i = 0; i < cSetLen; i++ )
            // {
            index = opt.indexOf( _sSeparator, oldIndex);
            if( index != -1 )
            {
                optLine.push_back( opt.copy( oldIndex, index - oldIndex ) );
                oldIndex = index + _sSeparator.getLength();
            }
            // }
            else // if (index == -1)
            {
                optLine.push_back( opt.copy( oldIndex ) );
                break;
            }
        }
    } ///< split

    bool match(OStringList const& _aFilter, OStringList const& _aName)
    {
        OStringList::const_iterator aFilterIter = _aFilter.begin();
        OStringList::const_iterator aValueIter = _aName.begin();

        bool bMatch = false;

        while (aFilterIter != _aFilter.end() && aValueIter != _aName.end())
        {
            rtl::OString sFilter = *aFilterIter;
            rtl::OString sName   = *aValueIter;

            if (sFilter == sName)
            {
                bMatch = true;
                ++aFilterIter;
                ++aValueIter;
            }
            else if (sFilter == "*")
            {
                bMatch = true;
                break;
            }
            else
            {
                // Filter does not match
                bMatch = false;
                break;
            }
        }
        return bMatch;
    }
}

/* static */
std::string OptionHelper::integerToAscii(sal_Int32 nValue)
{
    sal_Char cBuf[30];
    sal_Char *pBuf = cBuf;
    sprintf(pBuf, "%d", static_cast<int>(nValue));
//#     std::ostringstream sBuf;
//#     sBuf << nValue;
//    rtl::OString sStr;
//    sStr = rtl::OString::valueOf(nValue);
    return std::string(pBuf);
}

/* static */
// if a given String is less then 2 char, add a "0" in front.
std::string OptionHelper::twoDigits(std::string const& _sValue)
{
    std::string sBack;
    if (_sValue.length() == 0)
    {
        sBack = "00";
    }
    else if (_sValue.length() == 1)
    {
        sBack = "0";
        sBack += _sValue;
    }
    else
    {
        sBack = _sValue;
    }
    return sBack;
}


std::string OptionHelper::createDateTag(std::string const& _sProjectId, std::string const& _sBuildId)
{
    TimeValue tmv_system;
    TimeValue tmv_local;
    oslDateTime dt;
    osl_getSystemTime( &tmv_system );
    osl_getLocalTimeFromSystemTime(&tmv_system, &tmv_local);
    osl_getDateTimeFromTimeValue( &tmv_local, &dt );

    sal_Int32 nYear = dt.Year;
    sal_Int32 nMonth = dt.Month;
    sal_Int32 nDay = dt.Day;

    std::string sDate;
    sDate = "# Current Time: ";
    sDate += std::string(twoDigits(integerToAscii(dt.Hours)));
    sDate += ":";
    sDate += std::string(twoDigits(integerToAscii(dt.Minutes)));
    sDate += ":";
    sDate += std::string(twoDigits(integerToAscii(dt.Seconds)));
    sDate += "\n";

    sDate += "[";

    // sDate += rtl::OString::valueOf(nYear);
    sDate.append(integerToAscii(nYear));
    sDate += ".";
    if (nMonth < 10)
        sDate += "0";
    // sDate += rtl::OString::valueOf(nMonth);
    sDate.append(integerToAscii(nMonth));
    sDate += ".";
    if (nDay < 10)
        sDate += "0";
    // sDate += rtl::OString::valueOf(nDay);
    sDate += std::string(integerToAscii(nDay));
    sDate += "/";
    sDate += _sProjectId;
    sDate += "/";
    sDate += _sBuildId;
    sDate += "]";

    return sDate;
}

//# rtl::OString createDateTag()
//# {
//#     TimeValue tmv;
//#     oslDateTime dt;
//#     osl_getSystemTime( &tmv );
//#     osl_getDateTimeFromTimeValue( &tmv, &dt );
//#
//#     sal_Int32 nYear = dt.Year;
//#     sal_Int32 nMonth = dt.Month;
//#     sal_Int32 nDay = dt.Day;
//#
//#     rtl::OString sDate("[");
//#     sDate += rtl::OString::valueOf(nYear);
//#     sDate += ".";
//#     if (nMonth < 10)
//#             sDate += "0";
//#     sDate += rtl::OString::valueOf(nMonth);
//#     sDate += ".";
//#     if (nDay < 10)
//#             sDate += "0";
//#     sDate += rtl::OString::valueOf(nDay);
//#     sDate += "//]";
//#
//#     return sDate;
//# }

std::string OptionHelper::createDateTag()
{
    return createDateTag(m_sProjectId, m_sBuildId);
}

bool OptionHelper::showErrors()
{
    bool bRetValue = true; // default, show all
    if (m_aOption.hasOpt("-noerrors"))
    {
        bRetValue = false;
    }
    return bRetValue;
}

bool OptionHelper::showTests()
{
    bool bRetValue = true; // default, show all
    if (m_aOption.hasOpt("-onlyerrors"))
    {
        bRetValue = false;
    }
    return bRetValue;
}

// Check which parameter is given for handle the jobs.
// If no parameter is given, all jobs (tests) will run through
void OptionHelper::handleJobs()
{
    // load job file, the file contains the functions which we only want to test.
    if (m_aOption.hasOpt("-jobexclude"))
    {
        rtl::OString sJobFile = m_aOption.getOpt("-jobexclude");
        m_aJobExcludeList.readfile(sJobFile.getStr(), JOB_EXCLUDE_LIST);
    }

    if (m_aOption.hasOpt("-jobonly"))
    {
        rtl::OString sJobFile = m_aOption.getOpt("-jobonly");
        m_aJobOnlyList.readfile(sJobFile.getStr(), JOB_ONLY_LIST);
    }

    //
    if (m_aOption.hasOpt("-jobfilter"))
    {
        rtl::OString sJobFilter = m_aOption.getOpt("-jobfilter");
        split(sJobFilter, ".", m_aJobFilter);
    }
}

// -----------------------------------------------------------------------------
// Here will be decide, if a job/test will execute or not.
// therefore exist some lists, in which stay all jobs (positive list) or a list (negative list) which jobs should not execute
// or if only specific jobs should execute by a given filter

bool OptionHelper::isAllowedToExecute(std::string const& _sNodeName, std::string const& _sName)
{
    std::string sJobName = _sNodeName + "." + _sName;
    // t_print(T_VERBOSE, "Jobname %s\n", sJobName.c_str());
    if (isOnlyShowJobs())                        // true, if parameter -onlyshowjob
    {
        m_aJobOnlyList.setJobListEntry(sJobName, JOB_ACCESS);
        return false;
    }

    if (m_aJobOnlyList.size())                   // >0 if parameter -jobonly
    {
        if (m_aJobOnlyList.getJobListEntry(sJobName) != JOB_NOT_FOUND)
        {
            // job entry found, mark as accessed
            m_aJobOnlyList.setJobListEntry(sJobName, JOB_ACCESS);
            return true;
        }
        return false;
    }
    else if (m_aJobFilter.size() > 0)            // >0 if parameter -jobfilter
    {
        OStringList aSplitName;
        split(sJobName.c_str(), ".", aSplitName);
        if (! match(m_aJobFilter, aSplitName))
        {
            t_print(T_VERBOSE, "job: '%s' filtered by [-jobfilter]\n", sJobName.c_str());
            return false;
        }
    }
    else if (m_aJobExcludeList.size() > 0)       // >0 if parameter -jobexclude
    {
        if (m_aJobExcludeList.getJobListEntry(sJobName) != JOB_NOT_FOUND)
        {
            // job entry found, this job should not executed, so return false
            t_print(T_VERBOSE, "job: '%s' filtered by [-jobexclude]\n", sJobName.c_str());
            return false;
        }
        // m_aJobOnlyList.setJobListEntry(sJobName, JOB_ACCESS);
        return true;
    }
    else
    {
        //! not handled yet
    }

    return true;
}

