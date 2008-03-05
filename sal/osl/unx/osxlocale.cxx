/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: osxlocale.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:44:52 $
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

