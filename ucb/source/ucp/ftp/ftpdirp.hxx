/*************************************************************************
 *
 *  $RCSfile: ftpdirp.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: abi $ $Date: 2002-07-31 15:13:11 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#ifndef _FTP_FTPDIRP_HXX_
#define _FTP_FTPDIRP_HXX_

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif


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
                     : DateTime(hundredthSeconds,
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
        void SetTime(sal_uInt16 time) { Hours = Minutes = Seconds = HundredthSeconds = 0; }
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

    enum FtpDirentryMode { INETCOREFTP_FILEMODE_UNKNOWN = 0x00,
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

        static sal_Bool parseUNKNOWN (
            FTPDirentry &rEntry,
            const sal_Char  *pBuffer,
            sal_uInt32       nLength);


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
