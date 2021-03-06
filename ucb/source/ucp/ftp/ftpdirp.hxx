/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#pragma once

#include <rtl/ustring.hxx>
#include <com/sun/star/util/DateTime.hpp>


namespace ftp {

    /*========================================================================
     *
     * the DateTime structure
     *
     *======================================================================*/

    struct DateTime
        : public css::util::DateTime
    {
        DateTime() : css::util::DateTime(0, 0, 0, 0, 0, 0, 0, false) { }

        void SetYear(sal_uInt16 year) { Year = year; }
        void SetMonth(sal_uInt16 month) { Month = month; }
        void SetDay(sal_uInt16 day) { Day = day; }
        // Only zero allowed and used for time-argument
        void SetTime() { Hours = 0; Minutes = 0; Seconds = 0; NanoSeconds = 0; }
        void SetHour(sal_uInt16 hours) { Hours = hours; }
        void SetMin(sal_uInt16 minutes) { Minutes = minutes; }
        void SetSec(sal_uInt16 seconds) { Seconds = seconds; }
        void SetNanoSec(sal_uInt32 nanoSec) { NanoSeconds = nanoSec; }

        sal_uInt16 GetMonth() const { return Month; }
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
        OUString                       m_aURL;
        OUString                       m_aName;
        DateTime                            m_aDate;
        sal_uInt32                          m_nMode;
        sal_uInt32                          m_nSize;

        FTPDirentry()
            : m_aDate(),
              m_nMode(INETCOREFTP_FILEMODE_UNKNOWN),
              m_nSize(sal_uInt32(-1)) { }

        void clear() {
            m_aURL.clear();
            m_aName.clear();
            m_aDate = DateTime();
            m_nMode = INETCOREFTP_FILEMODE_UNKNOWN;
            m_nSize = sal_uInt32(-1);
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
        static bool parseDOS (
            FTPDirentry &rEntry,
            const char  *pBuffer );

        static bool parseVMS (
            FTPDirentry &rEntry,
            const char  *pBuffer );

        static bool parseUNIX (
            FTPDirentry &rEntry,
            const char  *pBuffer );


    private:

        static bool parseUNIX_isSizeField (
            const char *pStart,
            const char *pEnd,
            sal_uInt32     &rSize);

        static bool parseUNIX_isMonthField (
            const char *pStart,
            const char *pEnd,
            DateTime& rDateTime);

        static bool parseUNIX_isDayField (
            const char *pStart,
            const char *pEnd,
            DateTime& rDateTime);

        static bool parseUNIX_isYearTimeField (
            const char *pStart,
            const char *pEnd,
            DateTime& rDateTime);

        static bool parseUNIX_isTime (
            const char *pStart,
            const char *pEnd,
            sal_uInt16      nHour,
            DateTime& rDateTime);

        static void setYear (
            DateTime& rDateTime,
            sal_uInt16  nYear);

        static bool setPath (
            OUString& rPath,
            const char *value,
            sal_Int32       length = -1);
    };


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
