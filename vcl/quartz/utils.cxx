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

#include <sal/config.h>

#include <iostream>
#include <iomanip>

#include <rtl/alloc.h>
#include <rtl/ustrbuf.hxx>

#include <quartz/utils.h>

OUString GetOUString( CFStringRef rStr )
{
    if( rStr == nullptr )
    {
        return OUString();
    }

    CFIndex nLength = CFStringGetLength( rStr );
    if( nLength == 0 )
    {
        return OUString();
    }

    const UniChar* pConstStr = CFStringGetCharactersPtr( rStr );
    if( pConstStr )
    {
        return OUString( reinterpret_cast<sal_Unicode const *>(pConstStr), nLength );
    }

    std::unique_ptr<UniChar[]> pStr(new UniChar[nLength]);
    CFRange aRange = { 0, nLength };
    CFStringGetCharacters( rStr, aRange, pStr.get() );

    OUString aRet( reinterpret_cast<sal_Unicode *>(pStr.get()), nLength );
    return aRet;
}

OUString GetOUString( const NSString* pStr )
{
    if( ! pStr )
    {
        return OUString();
    }

    int nLen = [pStr length];
    if( nLen == 0 )
    {
        return OUString();
    }

    OUStringBuffer aBuf( nLen+1 );
    aBuf.setLength( nLen );
    [pStr getCharacters:
     reinterpret_cast<unichar *>(const_cast<sal_Unicode*>(aBuf.getStr()))];

    return aBuf.makeStringAndClear();
}

CFStringRef CreateCFString( const OUString& rStr )
{
    return CFStringCreateWithCharacters(kCFAllocatorDefault, reinterpret_cast<UniChar const *>(rStr.getStr()), rStr.getLength() );
}

NSString* CreateNSString( const OUString& rStr )
{
    return [[NSString alloc] initWithCharacters: reinterpret_cast<unichar const *>(rStr.getStr()) length: rStr.getLength()];
}

OUString NSStringArrayToOUString(NSArray* array)
{
    OUString result = "[";
    OUString sep;
    for (NSUInteger i = 0; i < [array count]; i++)
    {
        result = result + sep + OUString::fromUtf8([[array objectAtIndex:i] UTF8String]);
        sep = ",";
    }
    result = result + "]";
    return result;
}

OUString NSDictionaryKeysToOUString(NSDictionary* dict)
{
    OUString result = "{";
    OUString sep;
    for (NSString *key in dict)
    {
        result = result + sep + OUString::fromUtf8([key UTF8String]);
        sep = ",";
    }
    result = result + "}";
    return result;
}

std::ostream &operator <<(std::ostream& s, const CGRect &rRect)
{
#ifndef SAL_LOG_INFO
    (void) rRect;
#else
    if (CGRectIsNull(rRect))
    {
        s << "NULL";
    }
    else
    {
        s << rRect.size << "@" << rRect.origin;
    }
#endif
    return s;
}

std::ostream &operator <<(std::ostream& s, const CGPoint &rPoint)
{
#ifndef SAL_LOG_INFO
    (void) rPoint;
#else
    s << "(" << rPoint.x << "," << rPoint.y << ")";
#endif
    return s;
}

std::ostream &operator <<(std::ostream& s, const CGSize &rSize)
{
#ifndef SAL_LOG_INFO
    (void) rSize;
#else
    s << rSize.width << "x" << rSize.height;
#endif
    return s;
}

std::ostream &operator <<(std::ostream& s, CGColorRef pColor)
{
#ifndef SAL_LOG_INFO
    (void) pColor;
#else
    CFStringRef colorString = CFCopyDescription(pColor);
    if (colorString)
    {
        s << GetOUString(colorString);
        CFRelease(colorString);
    }
    else
    {
        s << "NULL";
    }
#endif
    return s;
}

std::ostream &operator <<(std::ostream& s, const CGAffineTransform &aXform)
{
#ifndef SAL_LOG_INFO
    (void) aXform;
#else
    if (CGAffineTransformIsIdentity(aXform))
    {
        s << "IDENT";
    }
    else
    {
        s << "[" << aXform.a << "," << aXform.b << "," << aXform.c << "," << aXform.d << "," << aXform.tx << "," << aXform.ty << "]";
    }
#endif
    return s;
}

std::ostream &operator <<(std::ostream& s, CGColorSpaceRef cs)
{
#ifndef SAL_LOG_INFO
    (void) cs;
#else
    if (cs == nullptr)
    {
        s << "null";
        return s;
    }

    CGColorSpaceModel model = CGColorSpaceGetModel(cs);
    switch (model)
    {
    case kCGColorSpaceModelUnknown:
        s << "Unknown";
        break;
    case kCGColorSpaceModelMonochrome:
        s << "Monochrome";
        break;
    case kCGColorSpaceModelRGB:
        s << "RGB";
        if (CGColorSpaceIsWideGamutRGB(cs))
            s << " (wide gamut)";
        break;
    case kCGColorSpaceModelCMYK:
        s << "CMYK";
        break;
    case kCGColorSpaceModelLab:
        s << "Lab";
        break;
    case kCGColorSpaceModelDeviceN:
        s << "DeviceN";
        break;
    case kCGColorSpaceModelIndexed:
        s << "Indexed (" << CGColorSpaceGetColorTableCount(cs) << ")";
        break;
    case kCGColorSpaceModelPattern:
        s << "Pattern";
        break;
    case kCGColorSpaceModelXYZ:
        s << "XYZ";
        break;
    default:
        s << "?(" << model << ")";
        break;
    }

    CFStringRef name = CGColorSpaceCopyName(cs);
    if (name != nullptr)
        s << " (" << [static_cast<NSString *>(name) UTF8String] << ")";
#endif
    return s;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
