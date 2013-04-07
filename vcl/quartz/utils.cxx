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

#include <iostream>
#include <iomanip>

#include <rtl/alloc.h>
#include <rtl/ustrbuf.hxx>

#include "quartz/utils.h"

OUString GetOUString( CFStringRef rStr )
{
    if( rStr == 0 )
        return OUString();
    CFIndex nLength = CFStringGetLength( rStr );
    if( nLength == 0 )
        return OUString();
    const UniChar* pConstStr = CFStringGetCharactersPtr( rStr );
    if( pConstStr )
        return OUString( pConstStr, nLength );
    UniChar* pStr = reinterpret_cast<UniChar*>( rtl_allocateMemory( sizeof(UniChar)*nLength ) );
    CFRange aRange = { 0, nLength };
    CFStringGetCharacters( rStr, aRange, pStr );
    OUString aRet( pStr, nLength );
    rtl_freeMemory( pStr );
    return aRet;
}

OUString GetOUString( NSString* pStr )
{
    if( ! pStr )
        return OUString();
    int nLen = [pStr length];
    if( nLen == 0 )
        return OUString();

    OUStringBuffer aBuf( nLen+1 );
    aBuf.setLength( nLen );
    [pStr getCharacters: const_cast<sal_Unicode*>(aBuf.getStr())];
    return aBuf.makeStringAndClear();
}

CFStringRef CreateCFString( const OUString& rStr )
{
    return CFStringCreateWithCharacters(kCFAllocatorDefault, rStr.getStr(), rStr.getLength() );
}

NSString* CreateNSString( const OUString& rStr )
{
    return [[NSString alloc] initWithCharacters: rStr.getStr() length: rStr.getLength()];
}

std::ostream &operator <<(std::ostream& s, CGRect &rRect)
{
#ifndef SAL_LOG_INFO
    (void) rRect;
#else
    s << rRect.size << "@" << rRect.origin;
#endif
    return s;
}

std::ostream &operator <<(std::ostream& s, CGPoint &rPoint)
{
#ifndef SAL_LOG_INFO
    (void) rPoint;
#else
    s << "(" << rPoint.x << "," << rPoint.y << ")";
#endif
    return s;
}

std::ostream &operator <<(std::ostream& s, CGSize &rSize)
{
#ifndef SAL_LOG_INFO
    (void) rSize;
#else
    s << rSize.width << "x" << rSize.height;
#endif
    return s;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
