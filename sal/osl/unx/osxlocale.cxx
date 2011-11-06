/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include <sal/types.h>
#include <assert.h>

#include <premac.h>
#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>

namespace /* private */
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

        if (pref == NULL) // return fallback value 'en_US'
             return CFStringCreateWithCString(kCFAllocatorDefault, "en_US", kCFStringEncodingASCII);

        CFStringRef sref = (CFGetTypeID(pref) == CFArrayGetTypeID()) ? (CFStringRef)CFArrayGetValueAtIndex((CFArrayRef)pref, 0) : (CFStringRef)pref;

        // NOTE: this API is only available with Mac OS X >=10.3. We need to use it because
        // Apple used non-ISO values on systems <10.2 like "German" for instance but didn't
        // upgrade those values during upgrade to newer Mac OS X versions. See also #i54337#
        return CFLocaleCreateCanonicalLocaleIdentifierFromString(kCFAllocatorDefault, sref);
    }
} // namespace private

/** Grab current locale from system.
*/
extern "C" {
int macosx_getLocale(char *locale, sal_uInt32 bufferLen)
{
    CFStringRef sref = getProcessLocale();
    CFStringGuard sGuard(sref);

    assert(sref != NULL && "osxlocale.cxx: getProcessLocale must return a non-NULL value");

    // split the string into substrings; the first two (if there are two) substrings
    // are language and country
    CFArrayRef subs = CFStringCreateArrayBySeparatingStrings(NULL, sref, CFSTR("_"));
    CFArrayGuard arrGuard(subs);

    CFStringRef lang = (CFStringRef)CFArrayGetValueAtIndex(subs, 0);
    CFStringGetCString(lang, locale, bufferLen, kCFStringEncodingASCII);

    // country also available? Assumption: if the array contains more than one
    // value the second value is always the country!
    if (CFArrayGetCount(subs) > 1)
    {
        strlcat(locale, "_", bufferLen - strlen(locale));

        CFStringRef country = (CFStringRef)CFArrayGetValueAtIndex(subs, 1);
        CFStringGetCString(country, locale + strlen(locale), bufferLen - strlen(locale), kCFStringEncodingASCII);
    }
    // Append 'UTF-8' to the locale because the Mac OS X file
    // system interface is UTF-8 based and sal tries to determine
    // the file system locale from the locale information
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

