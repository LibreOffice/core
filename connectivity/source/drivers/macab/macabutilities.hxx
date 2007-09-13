/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macabutilities.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-13 17:57:07 $
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

#ifndef _CONNECTIVITY_MACAB_UTILITIES_HXX_
#define _CONNECTIVITY_MACAB_UTILITIES_HXX_

#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif

#include <time.h>
#include <premac.h>
#include <Carbon/Carbon.h>
#include <AddressBook/ABAddressBookC.h>
#include <postmac.h>

namespace connectivity
{
    namespace macab
    {
        // -------------------------------------------------------------------------
        inline ::rtl::OUString CFStringToOUString(const CFStringRef sOrig)
        {
            /* Copied all-but directly from code by Florian Heckl in
             * cws_src680_aquafilepicker01
             * File was: fpicker/source/aqua/CFStringUtilities
             * I only removed commented debugging lines and changed variable
             * names.
             */
            if (NULL == sOrig) {
                    return rtl::OUString();
            }

            CFRetain(sOrig);
            CFIndex nStringLength = CFStringGetLength(sOrig);

            UniChar unichars[nStringLength+1];

            //'close' the string buffer correctly
            unichars[nStringLength] = '\0';

            CFStringGetCharacters (sOrig, CFRangeMake(0,nStringLength), unichars);
            CFRelease(sOrig);

            return rtl::OUString(unichars);
        }

        // -------------------------------------------------------------------------
        inline CFStringRef OUStringToCFString(const ::rtl::OUString& aString)
        {
            /* Copied directly from code by Florian Heckl in
             * cws_src680_aquafilepicker01
             * File was: fpicker/source/aqua/CFStringUtilities
             */

            CFStringRef ref = CFStringCreateWithCharacters(kCFAllocatorDefault, aString.getStr(), aString.getLength());

            return ref;
        }

        // -------------------------------------------------------------------------
        inline com::sun::star::util::DateTime CFDateToDateTime(const CFDateRef _cfDate)
        {
                /* Carbon can give us the time since 2001 of any CFDateRef,
                 * and it also stores the time since 1970 as a constant,
                 * basically allowing us to get the unixtime of any
                 * CFDateRef. From there, it is just a matter of choosing what
                 * we want to do with it.
                 */
            com::sun::star::util::DateTime nRet;
            double timeSince2001 = CFDateGetAbsoluteTime(_cfDate);
            time_t unixtime = timeSince2001+kCFAbsoluteTimeIntervalSince1970;
            struct tm *ptm = localtime(&unixtime);
            nRet.Year = ptm->tm_year+1900;
            nRet.Month = ptm->tm_mon+1;
            nRet.Day = ptm->tm_mday;
            nRet.Hours = ptm->tm_hour;
            nRet.Minutes = ptm->tm_min;
            nRet.Seconds = ptm->tm_sec;
            nRet.HundredthSeconds = 0;
            return nRet;
        }

        // -------------------------------------------------------------------------
        inline ::rtl::OUString fixLabel(const ::rtl::OUString _originalLabel)
        {
            /* Get the length, and make sure that there is actually a string
             * here.
             */
            if(_originalLabel.indexOf(::rtl::OUString::createFromAscii("_$!<")) == 0)
            {
                return _originalLabel.copy(4,_originalLabel.getLength()-8);
            }

            return _originalLabel;
        }

        // -------------------------------------------------------------------------
        inline sal_Int32 ABTypeToDataType(const ABPropertyType _abType)
        {
            sal_Int32 dataType;
            switch(_abType)
            {
                case kABStringProperty:
                    dataType = ::com::sun::star::sdbc::DataType::CHAR;
                    break;
                case kABDateProperty:
                    dataType = ::com::sun::star::sdbc::DataType::TIMESTAMP;
                    break;
                case kABIntegerProperty:
                    dataType = ::com::sun::star::sdbc::DataType::INTEGER;
                    break;
                case kABRealProperty:
                    dataType = ::com::sun::star::sdbc::DataType::FLOAT;
                    break;
                default:
                    dataType = -1;
            }
            return dataType;
        }

    }
}

#endif // _ CONNECTIVITY_MACAB_UTILITIES_HXX_
