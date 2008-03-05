/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CFStringUtilities.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:34:06 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _CFSTRINGUTILITIES_HXX_
#include "CFStringUtilities.hxx"
#endif

rtl::OUString CFStringToOUString(const CFStringRef sOrig) {
    //DBG_PRINT_ENTRY("CFStringUtilities", __func__, "sOrig", sOrig);

    if (NULL == sOrig) {
        return rtl::OUString();
    }

    CFRetain(sOrig);
    CFIndex nFileNameLength = CFStringGetLength(sOrig);
    //OSL_TRACE("FH: string length: %d", (int)(nFileNameLength));
    UniChar unichars[nFileNameLength+1];
    //'close' the string buffer correctly
    unichars[nFileNameLength] = '\0';

    CFStringGetCharacters (sOrig, CFRangeMake(0,nFileNameLength), unichars);

    //we no longer need the original string
    CFRelease(sOrig);

    //DBG_PRINT_EXIT("CFStringUtilities", __func__, unichars);

    return rtl::OUString(unichars);
}

CFStringRef CFStringCreateWithOUString(const rtl::OUString& aString) {
    //DBG_PRINT_ENTRY("CFStringUtilities", __func__);

    CFStringRef ref = CFStringCreateWithCharacters(kCFAllocatorDefault, aString.getStr(), aString.getLength());

    //DBG_PRINT_EXIT("CFStringUtilities", __func__, ref);

    return ref;
}

rtl::OUString FSRefToOUString(FSRef fsRef, InfoType info)
{
    //DBG_PRINT_ENTRY("CFStringUtilities", __func__);

    CFURLRef aUrlRef = CFURLCreateFromFSRef(NULL, &fsRef);

    rtl::OUString sResult = CFURLRefToOUString(aUrlRef, info);

    //we no longer need the CFURLRef
    CFRelease(aUrlRef);

    //DBG_PRINT_EXIT("CFStringUtilities", __func__, OUStringToOString(sResult, RTL_TEXTENCODING_UTF8).getStr());

    return sResult;
}

rtl::OUString CFURLRefToOUString(CFURLRef aUrlRef, InfoType info)
{
    //DBG_PRINT_ENTRY("CFStringUtilities", __func__);

    CFStringRef sURLString = NULL;

    switch(info) {
        case FULLPATH:
            OSL_TRACE("Extracting the full path of an item");
            sURLString = CFURLGetString(aUrlRef);
            CFRetain(sURLString);
            break;
        case FILENAME:
            OSL_TRACE("Extracting the file name of an item");
            CFStringRef fullString = CFURLGetString(aUrlRef);
            CFURLRef dirRef = CFURLCreateCopyDeletingLastPathComponent(NULL,aUrlRef);
            CFIndex dirLength = CFStringGetLength(CFURLGetString(dirRef));
            CFRelease(dirRef);
            CFIndex fullLength = CFStringGetLength(fullString);
            CFRange substringRange = CFRangeMake(dirLength, fullLength - dirLength);
            sURLString = CFStringCreateWithSubstring(NULL, fullString, substringRange);
            break;
        case PATHWITHOUTLASTCOMPONENT:
            OSL_TRACE("Extracting the last but one component of an item's path");
            CFURLRef directoryRef = CFURLCreateCopyDeletingLastPathComponent(NULL,aUrlRef);
            sURLString = CFURLGetString(directoryRef);
            CFRetain(sURLString);
            CFRelease(directoryRef);
            break;
        default:
            break;
    }

    rtl::OUString sResult = CFStringToOUString(sURLString);

    CFRelease(sURLString);

    //DBG_PRINT_EXIT("CFStringUtilities", __func__, OUStringToOString(sResult, RTL_TEXTENCODING_UTF8).getStr());

    return sResult;
}
