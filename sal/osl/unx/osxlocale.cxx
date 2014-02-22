/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <sal/types.h>
#include <assert.h>

#include <premac.h>
#ifndef IOS
#include <CoreServices/CoreServices.h>
#endif
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>

namespace
{
    template <typename T>
    class CFGuard
    {
    public:
        explicit CFGuard(T& rT) : rT_(rT) {}
        ~CFGuard() { if (rT_) CFRelease(rT_); }
    private:
        T& rT_;
    };

    typedef CFGuard<CFArrayRef> CFArrayGuard;
    typedef CFGuard<CFStringRef> CFStringGuard;
    typedef CFGuard<CFPropertyListRef> CFPropertyListGuard;

    /** Get the current process locale from system
    */
    CFStringRef getProcessLocale()
    {
        CFPropertyListRef pref = CFPreferencesCopyAppValue(CFSTR("AppleLocale"), kCFPreferencesCurrentApplication);
        CFPropertyListGuard proplGuard(pref);

        if (pref == NULL) 
             return CFStringCreateWithCString(kCFAllocatorDefault, "en_US", kCFStringEncodingASCII);

        CFStringRef sref = (CFGetTypeID(pref) == CFArrayGetTypeID()) ? (CFStringRef)CFArrayGetValueAtIndex((CFArrayRef)pref, 0) : (CFStringRef)pref;

        return CFLocaleCreateCanonicalLocaleIdentifierFromString(kCFAllocatorDefault, sref);
    }
}

/** Grab current locale from system.
*/
extern "C" {
int macosx_getLocale(char *locale, sal_uInt32 bufferLen)
{
    CFStringRef sref = getProcessLocale();
    CFStringGuard sGuard(sref);

    assert(sref != NULL && "osxlocale.cxx: getProcessLocale must return a non-NULL value");

    
    
    CFArrayRef subs = CFStringCreateArrayBySeparatingStrings(NULL, sref, CFSTR("_"));
    CFArrayGuard arrGuard(subs);

    CFStringRef lang = (CFStringRef)CFArrayGetValueAtIndex(subs, 0);
    CFStringGetCString(lang, locale, bufferLen, kCFStringEncodingASCII);

    
    
    if (CFArrayGetCount(subs) > 1)
    {
        strlcat(locale, "_", bufferLen - strlen(locale));

        CFStringRef country = (CFStringRef)CFArrayGetValueAtIndex(subs, 1);
        CFStringGetCString(country, locale + strlen(locale), bufferLen - strlen(locale), kCFStringEncodingASCII);
    }
    
    
    
    strlcat(locale, ".UTF-8", bufferLen - strlen(locale));

    return noErr;
}
}



/*
 * macxp_OSXConvertCFEncodingToIANACharSetName
 *
 * Convert a CoreFoundation text encoding to an IANA charset name.
 */
extern "C" int macxp_OSXConvertCFEncodingToIANACharSetName( char *buffer, unsigned int bufferLen, CFStringEncoding cfEncoding )
{
    CFStringRef sCFEncodingName;

    sCFEncodingName = CFStringConvertEncodingToIANACharSetName( cfEncoding );
    CFStringGetCString( sCFEncodingName, buffer, bufferLen, cfEncoding );

    if ( sCFEncodingName )
        CFRelease( sCFEncodingName );

    return( noErr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
