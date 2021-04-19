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


#include <comphelper/string.hxx>
#include <string.h>
#include <rtl/strbuf.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>

#include <svl/urihelper.hxx>
#include <vcl/imap.hxx>
#include <vcl/imapobj.hxx>
#include <vcl/imaprect.hxx>
#include <vcl/imapcirc.hxx>
#include <vcl/imappoly.hxx>

#include <math.h>

#define NOTEOL(c) ((c)!='\0')

void IMapObject::AppendCERNCoords(OStringBuffer& rBuf, const Point& rPoint100)
{
    const Point aPixPt( Application::GetDefaultDevice()->LogicToPixel( rPoint100, MapMode( MapUnit::Map100thMM ) ) );

    rBuf.append('(');
    rBuf.append(static_cast<sal_Int32>(aPixPt.X()));
    rBuf.append(',');
    rBuf.append(static_cast<sal_Int32>(aPixPt.Y()));
    rBuf.append(") ");
}

void IMapObject::AppendNCSACoords(OStringBuffer& rBuf, const Point& rPoint100)
{
    const Point aPixPt( Application::GetDefaultDevice()->LogicToPixel( rPoint100, MapMode( MapUnit::Map100thMM ) ) );

    rBuf.append(static_cast<sal_Int32>(aPixPt.X()));
    rBuf.append(',');
    rBuf.append(static_cast<sal_Int32>(aPixPt.Y()));
    rBuf.append(' ');
}

void IMapObject::AppendCERNURL(OStringBuffer& rBuf) const
{
    rBuf.append(OUStringToOString(URIHelper::simpleNormalizedMakeRelative("", aURL), osl_getThreadTextEncoding()));
}

void IMapObject::AppendNCSAURL(OStringBuffer& rBuf) const
{
    rBuf.append(OUStringToOString(URIHelper::simpleNormalizedMakeRelative("", aURL), osl_getThreadTextEncoding()));
    rBuf.append(' ');
}

void IMapRectangleObject::WriteCERN( SvStream& rOStm ) const
{
    OStringBuffer aStrBuf("rectangle ");

    AppendCERNCoords(aStrBuf, aRect.TopLeft());
    AppendCERNCoords(aStrBuf, aRect.BottomRight());
    AppendCERNURL(aStrBuf);

    rOStm.WriteLine(aStrBuf.makeStringAndClear());
}

void IMapRectangleObject::WriteNCSA( SvStream& rOStm ) const
{
    OStringBuffer aStrBuf("rect ");

    AppendNCSAURL(aStrBuf);
    AppendNCSACoords(aStrBuf, aRect.TopLeft());
    AppendNCSACoords(aStrBuf, aRect.BottomRight());

    rOStm.WriteLine(aStrBuf.makeStringAndClear());
}

void IMapCircleObject::WriteCERN( SvStream& rOStm ) const
{
    OStringBuffer aStrBuf("circle ");

    AppendCERNCoords(aStrBuf, aCenter);
    aStrBuf.append(nRadius);
    aStrBuf.append(' ');
    AppendCERNURL(aStrBuf);

    rOStm.WriteLine(aStrBuf.makeStringAndClear());
}

void IMapCircleObject::WriteNCSA( SvStream& rOStm ) const
{
    OStringBuffer aStrBuf("circle ");

    AppendNCSAURL(aStrBuf);
    AppendNCSACoords(aStrBuf, aCenter);
    AppendNCSACoords(aStrBuf, aCenter + Point(nRadius, 0));

    rOStm.WriteLine(aStrBuf.makeStringAndClear());
}

void IMapPolygonObject::WriteCERN( SvStream& rOStm  ) const
{
    OStringBuffer aStrBuf("polygon ");
    const sal_uInt16 nCount = aPoly.GetSize();

    for (sal_uInt16 i = 0; i < nCount; ++i)
        AppendCERNCoords(aStrBuf, aPoly[i]);

    AppendCERNURL(aStrBuf);

    rOStm.WriteLine(aStrBuf.makeStringAndClear());
}

void IMapPolygonObject::WriteNCSA( SvStream& rOStm  ) const
{
    OStringBuffer aStrBuf("poly ");
    const sal_uInt16 nCount = std::min( aPoly.GetSize(), sal_uInt16(100) );

    AppendNCSAURL(aStrBuf);

    for (sal_uInt16 i = 0; i < nCount; ++i)
        AppendNCSACoords(aStrBuf, aPoly[i]);

    rOStm.WriteLine(aStrBuf.makeStringAndClear());
}

void ImageMap::Write( SvStream& rOStm, IMapFormat nFormat ) const
{
    switch( nFormat )
    {
        case IMapFormat::Binary : Write( rOStm ); break;
        case IMapFormat::CERN : ImpWriteCERN( rOStm ); break;
        case IMapFormat::NCSA : ImpWriteNCSA( rOStm ); break;

        default:
        break;
    }
}

void ImageMap::ImpWriteCERN( SvStream& rOStm ) const
{
    size_t      nCount = maList.size();

    for ( size_t i = 0; i < nCount; i++ )
    {
        IMapObject* pObj = maList[ i ].get();

        switch( pObj->GetType() )
        {
            case IMapObjectType::Rectangle:
                static_cast<IMapRectangleObject*>( pObj )->WriteCERN( rOStm );
            break;

            case IMapObjectType::Circle:
                static_cast<IMapCircleObject*>( pObj )->WriteCERN( rOStm );
            break;

            case IMapObjectType::Polygon:
                static_cast<IMapPolygonObject*>( pObj )->WriteCERN( rOStm );
            break;

            default:
            break;
        }
    }
}

void ImageMap::ImpWriteNCSA( SvStream& rOStm  ) const
{
    size_t      nCount = maList.size();

    for ( size_t i = 0; i < nCount; i++ )
    {
        IMapObject* pObj = maList[ i ].get();

        switch( pObj->GetType() )
        {
            case IMapObjectType::Rectangle:
                static_cast<IMapRectangleObject*>( pObj )->WriteNCSA( rOStm );
            break;

            case IMapObjectType::Circle:
                static_cast<IMapCircleObject*>( pObj )->WriteNCSA( rOStm );
            break;

            case IMapObjectType::Polygon:
                static_cast<IMapPolygonObject*>( pObj )->WriteNCSA( rOStm );
            break;

            default:
            break;
        }
    }
}

sal_uLong ImageMap::Read( SvStream& rIStm, IMapFormat nFormat  )
{
    sal_uLong nRet = IMAP_ERR_FORMAT;

    if ( nFormat == IMapFormat::Detect )
        nFormat = ImpDetectFormat( rIStm );

    switch ( nFormat )
    {
        case IMapFormat::Binary : Read( rIStm ); break;
        case IMapFormat::CERN   : ImpReadCERN( rIStm ); break;
        case IMapFormat::NCSA   : ImpReadNCSA( rIStm ); break;

        default:
        break;
    }

    if ( !rIStm.GetError() )
        nRet = IMAP_ERR_OK;

    return nRet;
}

void ImageMap::ImpReadCERN( SvStream& rIStm )
{
    // delete old content
    ClearImageMap();

    OString aStr;
    while ( rIStm.ReadLine( aStr ) )
        ImpReadCERNLine( aStr );
}

void ImageMap::ImpReadCERNLine( std::string_view rLine  )
{
    OString aStr = comphelper::string::stripStart(rLine, ' ');
    aStr = comphelper::string::stripStart(aStr, '\t');
    aStr = aStr.replaceAll(";", "");
    aStr = aStr.toAsciiLowerCase();

    const char* pStr = aStr.getStr();
    char        cChar = *pStr++;

    // find instruction
    OStringBuffer aBuf;
    while ((cChar >= 'a') && (cChar <= 'z'))
    {
        aBuf.append(cChar);
        cChar = *pStr++;
    }
    OString aToken = aBuf.makeStringAndClear();

    if ( !(NOTEOL( cChar )) )
        return;

    if ( ( aToken == "rectangle" ) || ( aToken == "rect" ) )
    {
        const Point     aTopLeft( ImpReadCERNCoords( &pStr ) );
        const Point     aBottomRight( ImpReadCERNCoords( &pStr ) );
        const OUString  aURL( ImpReadCERNURL( &pStr ) );
        const tools::Rectangle aRect( aTopLeft, aBottomRight );

        maList.emplace_back( new IMapRectangleObject( aRect, aURL, OUString(), OUString(), OUString(), OUString() ) );
    }
    else if ( ( aToken == "circle" ) || ( aToken == "circ" ) )
    {
        const Point     aCenter( ImpReadCERNCoords( &pStr ) );
        const tools::Long      nRadius = ImpReadCERNRadius( &pStr );
        const OUString  aURL( ImpReadCERNURL( &pStr ) );

        maList.emplace_back( new IMapCircleObject( aCenter, nRadius, aURL, OUString(), OUString(), OUString(), OUString() ) );
    }
    else if ( ( aToken == "polygon" ) || ( aToken == "poly" ) )
    {
        const sal_uInt16 nCount = comphelper::string::getTokenCount(aStr, '(') - 1;
        tools::Polygon aPoly( nCount );

        for ( sal_uInt16 i = 0; i < nCount; i++ )
            aPoly[ i ] = ImpReadCERNCoords( &pStr );

        const OUString aURL = ImpReadCERNURL( &pStr );

        maList.emplace_back( new IMapPolygonObject( aPoly, aURL, OUString(), OUString(), OUString(), OUString() ) );
    }
}

Point ImageMap::ImpReadCERNCoords( const char** ppStr )
{
    OUStringBuffer  aStrX;
    OUStringBuffer  aStrY;
    Point           aPt;
    char            cChar = *(*ppStr)++;

    while( NOTEOL( cChar ) && ( ( cChar < '0' ) || ( cChar > '9' ) ) )
        cChar = *(*ppStr)++;

    if ( NOTEOL( cChar ) )
    {
        while( NOTEOL( cChar ) && ( cChar >= '0' ) && ( cChar <= '9' ) )
        {
            aStrX.append( cChar );
            cChar = *(*ppStr)++;
        }

        if ( NOTEOL( cChar ) )
        {
            while( NOTEOL( cChar ) && ( ( cChar < '0' ) || ( cChar > '9' ) ) )
                cChar = *(*ppStr)++;

            while( NOTEOL( cChar ) && ( cChar >= '0' ) && ( cChar <= '9' ) )
            {
                aStrY.append( cChar );
                cChar = *(*ppStr)++;
            }

            if ( NOTEOL( cChar ) )
                while( NOTEOL( cChar ) && ( cChar != ')' ) )
                    cChar = *(*ppStr)++;

            aPt = Point( aStrX.makeStringAndClear().toInt32(), aStrY.makeStringAndClear().toInt32() );
        }
    }

    return aPt;
}

tools::Long ImageMap::ImpReadCERNRadius( const char** ppStr )
{
    OUStringBuffer  aStr;
    char            cChar = *(*ppStr)++;

    while( NOTEOL( cChar ) && ( ( cChar < '0' ) || ( cChar > '9' ) ) )
        cChar = *(*ppStr)++;

    if ( NOTEOL( cChar ) )
    {
        while( NOTEOL( cChar ) && ( cChar >= '0' ) && ( cChar <= '9' ) )
        {
            aStr.append( cChar );
            cChar = *(*ppStr)++;
        }
    }

    return aStr.makeStringAndClear().toInt32();
}

OUString ImageMap::ImpReadCERNURL( const char** ppStr )
{
    OUString aStr(OUString::createFromAscii(*ppStr));

    aStr = comphelper::string::stripStart(aStr, ' ');
    aStr = comphelper::string::stripStart(aStr, '\t');
    aStr = comphelper::string::stripEnd(aStr, ' ');
    aStr = comphelper::string::stripEnd(aStr, '\t');

    return INetURLObject::GetAbsURL( "", aStr );
}

void ImageMap::ImpReadNCSA( SvStream& rIStm )
{
    // delete old content
    ClearImageMap();

    OString aStr;
    while ( rIStm.ReadLine( aStr ) )
        ImpReadNCSALine( aStr );
}

void ImageMap::ImpReadNCSALine( std::string_view rLine )
{
    OString aStr = comphelper::string::stripStart(rLine, ' ');
    aStr = comphelper::string::stripStart(aStr, '\t');
    aStr = aStr.replaceAll(";", "");
    aStr = aStr.toAsciiLowerCase();

    const char* pStr = aStr.getStr();
    char        cChar = *pStr++;

    // find instruction
    OStringBuffer aBuf;
    while ((cChar >= 'a') && (cChar <= 'z'))
    {
        aBuf.append(cChar);
        cChar = *pStr++;
    }
    OString aToken = aBuf.makeStringAndClear();

    if ( !(NOTEOL( cChar )) )
        return;

    if ( aToken == "rect" )
    {
        const OUString  aURL( ImpReadNCSAURL( &pStr ) );
        const Point     aTopLeft( ImpReadNCSACoords( &pStr ) );
        const Point     aBottomRight( ImpReadNCSACoords( &pStr ) );
        const tools::Rectangle aRect( aTopLeft, aBottomRight );

        maList.emplace_back( new IMapRectangleObject( aRect, aURL, OUString(), OUString(), OUString(), OUString() ) );
    }
    else if ( aToken == "circle" )
    {
        const OUString  aURL( ImpReadNCSAURL( &pStr ) );
        const Point     aCenter( ImpReadNCSACoords( &pStr ) );
        const Point     aDX( aCenter - ImpReadNCSACoords( &pStr ) );
        tools::Long            nRadius = static_cast<tools::Long>(sqrt( static_cast<double>(aDX.X()) * aDX.X() +
                                               static_cast<double>(aDX.Y()) * aDX.Y() ));

        maList.emplace_back( new IMapCircleObject( aCenter, nRadius, aURL, OUString(), OUString(), OUString(), OUString() ) );
    }
    else if ( aToken == "poly" )
    {
        const sal_uInt16 nCount = comphelper::string::getTokenCount(aStr, ',') - 1;
        const OUString aURL( ImpReadNCSAURL( &pStr ) );
        tools::Polygon aPoly( nCount );

        for ( sal_uInt16 i = 0; i < nCount; i++ )
            aPoly[ i ] = ImpReadNCSACoords( &pStr );

        maList.emplace_back( new IMapPolygonObject( aPoly, aURL, OUString(), OUString(), OUString(), OUString() ) );
    }
}

OUString ImageMap::ImpReadNCSAURL( const char** ppStr )
{
    OUStringBuffer  aStr;
    char            cChar = *(*ppStr)++;

    while( NOTEOL( cChar ) && ( ( cChar == ' ' ) || ( cChar == '\t' ) ) )
        cChar = *(*ppStr)++;

    if ( NOTEOL( cChar ) )
    {
        while( NOTEOL( cChar ) && ( cChar != ' ' ) && ( cChar != '\t' ) )
        {
            aStr.append( cChar );
            cChar = *(*ppStr)++;
        }
    }

    return INetURLObject::GetAbsURL( "", aStr.makeStringAndClear() );
}

Point ImageMap::ImpReadNCSACoords( const char** ppStr )
{
    OUStringBuffer  aStrX;
    OUStringBuffer  aStrY;
    Point           aPt;
    char            cChar = *(*ppStr)++;

    while( NOTEOL( cChar ) && ( ( cChar < '0' ) || ( cChar > '9' ) ) )
        cChar = *(*ppStr)++;

    if ( NOTEOL( cChar ) )
    {
        while( NOTEOL( cChar ) && ( cChar >= '0' ) && ( cChar <= '9' ) )
        {
            aStrX.append( cChar );
            cChar = *(*ppStr)++;
        }

        if ( NOTEOL( cChar ) )
        {
            while( NOTEOL( cChar ) && ( ( cChar < '0' ) || ( cChar > '9' ) ) )
                cChar = *(*ppStr)++;

            while( NOTEOL( cChar ) && ( cChar >= '0' ) && ( cChar <= '9' ) )
            {
                aStrY.append( cChar );
                cChar = *(*ppStr)++;
            }

            aPt = Point( aStrX.makeStringAndClear().toInt32(), aStrY.makeStringAndClear().toInt32() );
        }
    }

    return aPt;
}

IMapFormat ImageMap::ImpDetectFormat( SvStream& rIStm )
{
    sal_uInt64  nPos = rIStm.Tell();
    IMapFormat  nRet = IMapFormat::Binary;
    char    cMagic[6];

    rIStm.ReadBytes(cMagic, sizeof(cMagic));

    // if we do not have an internal formats
    // we check the format
    if ( memcmp( cMagic, IMAPMAGIC, sizeof( cMagic ) ) )
    {
        tools::Long        nCount = 128;

        rIStm.Seek( nPos );
        OString aStr;
        while ( rIStm.ReadLine( aStr ) && nCount-- )
        {
            aStr = aStr.toAsciiLowerCase();

            if ( (aStr.indexOf("rect") != -1) ||
                 (aStr.indexOf("circ") != -1) ||
                 (aStr.indexOf("poly") != -1) )
            {
                if ( ( aStr.indexOf('(') != -1 ) &&
                     ( aStr.indexOf(')') != -1 ) )
                {
                    nRet = IMapFormat::CERN;
                }
                else
                    nRet = IMapFormat::NCSA;

                break;
            }
        }
    }

    rIStm.Seek( nPos );

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
