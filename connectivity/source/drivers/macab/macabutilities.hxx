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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABUTILITIES_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABUTILITIES_HXX

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/sdbc/DataType.hpp>

#include <time.h>
#include <premac.h>
#include <Carbon/Carbon.h>
#include <AddressBook/ABAddressBookC.h>
#include <postmac.h>

namespace connectivity
{
    namespace macab
    {

        inline OUString CFStringToOUString(const CFStringRef sOrig)
        {
            /* Copied all-but directly from code by Florian Heckl in
             * cws_src680_aquafilepicker01
             * File was: fpicker/source/aqua/CFStringUtilities
             * I only removed commented debugging lines and changed variable
             * names.
             */
            if (nullptr == sOrig) {
                    return OUString();
            }

            CFRetain(sOrig);
            CFIndex nStringLength = CFStringGetLength(sOrig);

            UniChar unichars[nStringLength+1];

            //'close' the string buffer correctly
            unichars[nStringLength] = '\0';

            CFStringGetCharacters (sOrig, CFRangeMake(0,nStringLength), unichars);
            CFRelease(sOrig);

            return OUString(reinterpret_cast<sal_Unicode *>(unichars));
        }


        inline CFStringRef OUStringToCFString(const OUString& aString)
        {
            /* Copied directly from code by Florian Heckl in
             * cws_src680_aquafilepicker01
             * File was: fpicker/source/aqua/CFStringUtilities
             */

            CFStringRef ref = CFStringCreateWithCharacters(kCFAllocatorDefault, reinterpret_cast<UniChar const *>(aString.getStr()), aString.getLength());

            return ref;
        }


        inline css::util::DateTime CFDateToDateTime(const CFDateRef _cfDate)
        {
                /* Carbon can give us the time since 2001 of any CFDateRef,
                 * and it also stores the time since 1970 as a constant,
                 * basically allowing us to get the unixtime of any
                 * CFDateRef. From there, it is just a matter of choosing what
                 * we want to do with it.
                 */
            css::util::DateTime nRet;
            double timeSince2001 = CFDateGetAbsoluteTime(_cfDate);
            time_t unixtime = timeSince2001+kCFAbsoluteTimeIntervalSince1970;
            struct tm *ptm = localtime(&unixtime);
            nRet.Year = ptm->tm_year+1900;
            nRet.Month = ptm->tm_mon+1;
            nRet.Day = ptm->tm_mday;
            nRet.Hours = ptm->tm_hour;
            nRet.Minutes = ptm->tm_min;
            nRet.Seconds = ptm->tm_sec;
            nRet.NanoSeconds = 0;
            return nRet;
        }


        inline OUString fixLabel(const OUString& _originalLabel)
        {
            /* Get the length, and make sure that there is actually a string
             * here.
             */
            if(_originalLabel.startsWith("_$!<"))
            {
                return _originalLabel.copy(4,_originalLabel.getLength()-8);
            }

            return _originalLabel;
        }


        inline sal_Int32 ABTypeToDataType(const ABPropertyType _abType)
        {
            sal_Int32 dataType;
            switch(_abType)
            {
                case kABStringProperty:
                    dataType = css::sdbc::DataType::CHAR;
                    break;
                case kABDateProperty:
                    dataType = css::sdbc::DataType::TIMESTAMP;
                    break;
                case kABIntegerProperty:
                    dataType = css::sdbc::DataType::INTEGER;
                    break;
                case kABRealProperty:
                    dataType = css::sdbc::DataType::FLOAT;
                    break;
                default:
                    dataType = -1;
            }
            return dataType;
        }

        void impl_throwError(const char* pErrorId);
    }
}

#endif // _ CONNECTIVITY_MACAB_UTILITIES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
