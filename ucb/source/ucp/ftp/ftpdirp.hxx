/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#ifndef _FTP_FTPDIRP_HXX_
#define _FTP_FTPDIRP_HXX_

#include <osl/time.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/util/DateTime.hpp>


namespace ftp {

    /*========================================================================
     *
     * the DateTime structure
     *
     *======================================================================*/

    struct DateTime
        : public com::sun::star::util::DateTime
    {
        DateTime(const sal_uInt16& hundredthSeconds,
                 const sal_uInt16& seconds,
                 const sal_uInt16& minutes,
                 const sal_uInt16& hours,
                 const sal_uInt16& day,
                 const sal_uInt16& month,
                 const sal_uInt16& year) SAL_THROW( () )
                     : com::sun::star::util::DateTime(hundredthSeconds,
                                                      seconds,
                                                      minutes,
                                                      hours,
                                                      day,
                                                      month,
                                                      year) { }

        void SetYear(sal_uInt16 year) { Year = year; }
        void SetMonth(sal_uInt16 month) { Month = month; }
        void SetDay(sal_uInt16 day) { Day = day; }
        // Only zero allowed and used for time-argument
        void SetTime(sal_uInt16) { Hours = Minutes = Seconds = HundredthSeconds = 0; }
        void SetHour(sal_uInt16 hours) { Hours = hours; }
        void SetMin(sal_uInt16 minutes) { Minutes = minutes; }
        void SetSec(sal_uInt16 seconds) { Seconds = seconds; }
        void Set100Sec(sal_uInt16 hundredthSec) { HundredthSeconds = hundredthSec; }

        sal_uInt16 GetMonth(void) { return Month; }
    };



/*========================================================================
 *
 * the directory information structure
 *
 *======================================================================*/

    enum FTPDirentryMode { INETCOREFTP_FILEMODE_UNKNOWN = 0x00,
                           INETCOREFTP_FILEMODE_READ = 0x01,
                           INETCOREFTP_FILEMODE_WRITE = 0x02,
                           INETCOREFTP_FILEMODE_ISDIR = 0x04,
                           INETCOREFTP_FILEMODE_ISLINK = 0x08 };

    struct FTPDirentry
    {
        rtl::OUString                       m_aURL;
        rtl::OUString                       m_aName;
        DateTime                            m_aDate;
        sal_uInt32                          m_nMode;
        sal_uInt32                          m_nSize;

        FTPDirentry(void)
            : m_aDate(0,0,0,0,0,0,0),
              m_nMode(INETCOREFTP_FILEMODE_UNKNOWN),
              m_nSize((sal_uInt32)(-1)) { }

        void clear() {
            m_aURL = m_aName = rtl::OUString();
            m_aDate = DateTime(0,0,0,0,0,0,0);
            m_nMode = INETCOREFTP_FILEMODE_UNKNOWN;
            m_nSize = sal_uInt32(-1);
        }

        bool isDir() const {
            return bool(m_nMode && INETCOREFTP_FILEMODE_ISDIR);
        }

        bool isFile() const {
            return ! bool(m_nMode && INETCOREFTP_FILEMODE_ISDIR);
        }
    };


/*========================================================================
 *
 * the directory parser
 *
 *======================================================================*/


    class FTPDirectoryParser
    {
    public:
        static sal_Bool parseDOS (
            FTPDirentry &rEntry,
            const sal_Char  *pBuffer );

        static sal_Bool parseVMS (
            FTPDirentry &rEntry,
            const sal_Char  *pBuffer );

        static sal_Bool parseUNIX (
            FTPDirentry &rEntry,
            const sal_Char  *pBuffer );


    private:

        static sal_Bool parseUNIX_isSizeField (
            const sal_Char *pStart,
            const sal_Char *pEnd,
            sal_uInt32     &rSize);

        static sal_Bool parseUNIX_isMonthField (
            const sal_Char *pStart,
            const sal_Char *pEnd,
            DateTime& rDateTime);

        static sal_Bool parseUNIX_isDayField (
            const sal_Char *pStart,
            const sal_Char *pEnd,
            DateTime& rDateTime);

        static sal_Bool parseUNIX_isYearTimeField (
            const sal_Char *pStart,
            const sal_Char *pEnd,
            DateTime& rDateTime);

        static sal_Bool parseUNIX_isTime (
            const sal_Char *pStart,
            const sal_Char *pEnd,
            sal_uInt16      nHour,
            DateTime& rDateTime);

        static sal_Bool setYear (
            DateTime& rDateTime,
            sal_uInt16  nYear);

        static sal_Bool setPath (
            rtl::OUString& rPath,
            const sal_Char *value,
            sal_Int32       length = -1);
    };


}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
