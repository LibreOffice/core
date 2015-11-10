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

#include "sal/log.hxx"
#include "CFStringUtilities.hxx"

rtl::OUString CFStringToOUString(const CFStringRef sOrig) {
    //DBG_PRINT_ENTRY("CFStringUtilities", __func__, "sOrig", sOrig);

    if (nullptr == sOrig) {
        return rtl::OUString();
    }

    CFRetain(sOrig);
    CFIndex nFileNameLength = CFStringGetLength(sOrig);
    //SAL_INFO("fpicker.aqua","FH: string length: " << (int)(nFileNameLength));
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

rtl::OUString FSRefToOUString(FSRef const & fsRef, InfoType info)
{
    //DBG_PRINT_ENTRY("CFStringUtilities", __func__);

    SAL_WNODEPRECATED_DECLARATIONS_PUSH //TODO: 10.9 CFURLCreateFromFSRef
    CFURLRef aUrlRef = CFURLCreateFromFSRef(nullptr, &fsRef);
    SAL_WNODEPRECATED_DECLARATIONS_POP

    rtl::OUString sResult = CFURLRefToOUString(aUrlRef, info);

    //we no longer need the CFURLRef
    CFRelease(aUrlRef);

    //DBG_PRINT_EXIT("CFStringUtilities", __func__, OUStringToOString(sResult, RTL_TEXTENCODING_UTF8).getStr());

    return sResult;
}

rtl::OUString CFURLRefToOUString(CFURLRef aUrlRef, InfoType info)
{
    //DBG_PRINT_ENTRY("CFStringUtilities", __func__);

    CFStringRef sURLString = nullptr;

    switch(info) {
        case FULLPATH:
            SAL_INFO("fpicker.aqua","Extracting the full path of an item");
            sURLString = CFURLGetString(aUrlRef);
            CFRetain(sURLString);
            break;
        case FILENAME:
            {
                SAL_INFO("fpicker.aqua","Extracting the file name of an item");
                CFStringRef fullString = CFURLGetString(aUrlRef);
                CFURLRef dirRef = CFURLCreateCopyDeletingLastPathComponent(nullptr,aUrlRef);
                CFIndex dirLength = CFStringGetLength(CFURLGetString(dirRef));
                CFRelease(dirRef);
                CFIndex fullLength = CFStringGetLength(fullString);
                CFRange substringRange = CFRangeMake(dirLength, fullLength - dirLength);
                sURLString = CFStringCreateWithSubstring(nullptr, fullString, substringRange);
            }
            break;
        case PATHWITHOUTLASTCOMPONENT:
            {
                SAL_INFO("fpicker.aqua","Extracting the last but one component of an item's path");
                CFURLRef directoryRef = CFURLCreateCopyDeletingLastPathComponent(nullptr,aUrlRef);
                sURLString = CFURLGetString(directoryRef);
                CFRetain(sURLString);
                CFRelease(directoryRef);
            }
            break;
        default:
            break;
    }

    rtl::OUString sResult = CFStringToOUString(sURLString);

    CFRelease(sURLString);

    //DBG_PRINT_EXIT("CFStringUtilities", __func__, OUStringToOString(sResult, RTL_TEXTENCODING_UTF8).getStr());

    return sResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
