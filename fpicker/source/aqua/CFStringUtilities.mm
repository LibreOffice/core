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

#include <sal/log.hxx>
#include "CFStringUtilities.hxx"

OUString CFStringToOUString(const CFStringRef sOrig)
{
    if (nullptr == sOrig) {
        return OUString();
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

    return OUString(reinterpret_cast<sal_Unicode *>(unichars));
}

CFStringRef CFStringCreateWithOUString(const OUString& aString)
{
    CFStringRef ref = CFStringCreateWithCharacters(kCFAllocatorDefault, reinterpret_cast<UniChar const *>(aString.getStr()), aString.getLength());

    return ref;
}

OUString FSRefToOUString(FSRef const & fsRef, InfoType info)
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH //TODO: 10.9 CFURLCreateFromFSRef
    CFURLRef aUrlRef = CFURLCreateFromFSRef(nullptr, &fsRef);
    SAL_WNODEPRECATED_DECLARATIONS_POP

    OUString sResult = CFURLRefToOUString(aUrlRef, info);

    //we no longer need the CFURLRef
    CFRelease(aUrlRef);

    return sResult;
}

OUString CFURLRefToOUString(CFURLRef aUrlRef, InfoType info)
{
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

    OUString sResult = CFStringToOUString(sURLString);

    CFRelease(sURLString);

    return sResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
