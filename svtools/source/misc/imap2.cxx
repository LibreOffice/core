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
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <vcl/wrkwin.hxx>
#include <sot/formats.hxx>

#include <svl/urihelper.hxx>
#include <svtools/imap.hxx>
#include <svtools/imapobj.hxx>
#include <svtools/imaprect.hxx>
#include <svtools/imapcirc.hxx>
#include <svtools/imappoly.hxx>

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

void ImageMap::Write( SvStream& rOStm, sal_uLong nFormat ) const
{
    switch( nFormat )
    {
        case IMAP_FORMAT_BIN : Write( rOStm ); break;
        case IMAP_FORMAT_CERN : ImpWriteCERN( rOStm ); break;
        case IMAP_FORMAT_NCSA : ImpWriteNCSA( rOStm ); break;

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
            case IMAP_OBJ_RECTANGLE:
                static_cast<IMapRectangleObject*>( pObj )->WriteCERN( rOStm );
            break;

            case IMAP_OBJ_CIRCLE:
                static_cast<IMapCircleObject*>( pObj )->WriteCERN( rOStm );
            break;

            case IMAP_OBJ_POLYGON:
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
            case IMAP_OBJ_RECTANGLE:
                static_cast<IMapRectangleObject*>( pObj )->WriteNCSA( rOStm );
            break;

            case IMAP_OBJ_CIRCLE:
                static_cast<IMapCircleObject*>( pObj )->WriteNCSA( rOStm );
            break;

            case IMAP_OBJ_POLYGON:
                static_cast<IMapPolygonObject*>( pObj )->WriteNCSA( rOStm );
            break;

            default:
            break;
        }
    }
}

sal_uLong ImageMap::Read( SvStream& rIStm, sal_uLong nFormat  )
{
    sal_uLong nRet = IMAP_ERR_FORMAT;

    if ( nFormat == IMAP_FORMAT_DETECT )
        nFormat = ImpDetectFormat( rIStm );

    switch ( nFormat )
    {
        case IMAP_FORMAT_BIN    : Read( rIStm ); break;
        case IMAP_FORMAT_CERN   : nRet = ImpReadCERN( rIStm ); break;
        case IMAP_FORMAT_NCSA   : nRet = ImpReadNCSA( rIStm ); break;

        default:
        break;
    }

    if ( !rIStm.GetError() )
        nRet = IMAP_ERR_OK;

    return nRet;
}

sal_uLong ImageMap::ImpReadCERN( SvStream& rIStm )
{
    // delete old content
    ClearImageMap();

    OString aStr;
    while ( rIStm.ReadLine( aStr ) )
        ImpReadCERNLine( aStr );

    return IMAP_ERR_OK;
}

void ImageMap::ImpReadCERNLine( const OString& rLine  )
{
    OString aStr = comphelper::string::stripStart(rLine, ' ');
    aStr = comphelper::string::stripStart(aStr, '\t');
    aStr = aStr.replaceAll(";", "");
    aStr = aStr.toAsciiLowerCase();

    const char* pStr = aStr.getStr();
    char        cChar = *pStr++;

    // find instruction
    OStringBuffer aBuf;
    while( ( cChar >= 'a' ) && ( cChar <= 'z' ) && NOTEOL( cChar ) )
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
        const OUString  aURL( ImpReadCERNURL( &pStr, "" ) );
        const tools::Rectangle aRect( aTopLeft, aBottomRight );

        maList.emplace_back( new IMapRectangleObject( aRect, aURL, OUString(), OUString(), OUString(), OUString() ) );
    }
    else if ( ( aToken == "circle" ) || ( aToken == "circ" ) )
    {
        const Point     aCenter( ImpReadCERNCoords( &pStr ) );
        const long      nRadius = ImpReadCERNRadius( &pStr );
        const OUString  aURL( ImpReadCERNURL( &pStr, "" ) );

        maList.emplace_back( new IMapCircleObject( aCenter, nRadius, aURL, OUString(), OUString(), OUString(), OUString() ) );
    }
    else if ( ( aToken == "polygon" ) || ( aToken == "poly" ) )
    {
        const sal_uInt16 nCount = comphelper::string::getTokenCount(aStr, '(') - 1;
        tools::Polygon aPoly( nCount );
        OUString aURL;

        for ( sal_uInt16 i = 0; i < nCount; i++ )
            aPoly[ i ] = ImpReadCERNCoords( &pStr );

        aURL = ImpReadCERNURL( &pStr, "" );

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

long ImageMap::ImpReadCERNRadius( const char** ppStr )
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

OUString ImageMap::ImpReadCERNURL( const char** ppStr, const OUString& rBaseURL )
{
    OUString aStr(OUString::createFromAscii(*ppStr));

    aStr = comphelper::string::stripStart(aStr, ' ');
    aStr = comphelper::string::stripStart(aStr, '\t');
    aStr = comphelper::string::stripEnd(aStr, ' ');
    aStr = comphelper::string::stripEnd(aStr, '\t');

    return INetURLObject::GetAbsURL( rBaseURL, aStr );
}

sal_uLong ImageMap::ImpReadNCSA( SvStream& rIStm )
{
    // delete old content
    ClearImageMap();

    OString aStr;
    while ( rIStm.ReadLine( aStr ) )
        ImpReadNCSALine( aStr );

    return IMAP_ERR_OK;
}

void ImageMap::ImpReadNCSALine( const OString& rLine )
{
    OString aStr = comphelper::string::stripStart(rLine, ' ');
    aStr = comphelper::string::stripStart(aStr, '\t');
    aStr = aStr.replaceAll(";", "");
    aStr = aStr.toAsciiLowerCase();

    const char* pStr = aStr.getStr();
    char        cChar = *pStr++;

    // find instruction
    OStringBuffer aBuf;
    while( ( cChar >= 'a' ) && ( cChar <= 'z' ) && NOTEOL( cChar ) )
    {
        aBuf.append(cChar);
        cChar = *pStr++;
    }
    OString aToken = aBuf.makeStringAndClear();

    if ( !(NOTEOL( cChar )) )
        return;

    if ( aToken == "rect" )
    {
        const OUString  aURL( ImpReadNCSAURL( &pStr, "" ) );
        const Point     aTopLeft( ImpReadNCSACoords( &pStr ) );
        const Point     aBottomRight( ImpReadNCSACoords( &pStr ) );
        const tools::Rectangle aRect( aTopLeft, aBottomRight );

        maList.emplace_back( new IMapRectangleObject( aRect, aURL, OUString(), OUString(), OUString(), OUString() ) );
    }
    else if ( aToken == "circle" )
    {
        const OUString  aURL( ImpReadNCSAURL( &pStr, "" ) );
        const Point     aCenter( ImpReadNCSACoords( &pStr ) );
        const Point     aDX( aCenter - ImpReadNCSACoords( &pStr ) );
        long            nRadius = static_cast<long>(sqrt( static_cast<double>(aDX.X()) * aDX.X() +
                                               static_cast<double>(aDX.Y()) * aDX.Y() ));

        maList.emplace_back( new IMapCircleObject( aCenter, nRadius, aURL, OUString(), OUString(), OUString(), OUString() ) );
    }
    else if ( aToken == "poly" )
    {
        const sal_uInt16 nCount = comphelper::string::getTokenCount(aStr,
            ',') - 1;
        const OUString aURL( ImpReadNCSAURL( &pStr, "" ) );
        tools::Polygon aPoly( nCount );

        for ( sal_uInt16 i = 0; i < nCount; i++ )
            aPoly[ i ] = ImpReadNCSACoords( &pStr );

        maList.emplace_back( new IMapPolygonObject( aPoly, aURL, OUString(), OUString(), OUString(), OUString() ) );
    }
}

OUString ImageMap::ImpReadNCSAURL( const char** ppStr, const OUString& rBaseURL )
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

    return INetURLObject::GetAbsURL( rBaseURL, aStr.makeStringAndClear() );
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

sal_uLong ImageMap::ImpDetectFormat( SvStream& rIStm )
{
    sal_uInt64  nPos = rIStm.Tell();
    sal_uLong   nRet = IMAP_FORMAT_BIN;
    char    cMagic[6];

    rIStm.ReadBytes(cMagic, sizeof(cMagic));

    // if we do not have an internal formats
    // we check the format
    if ( memcmp( cMagic, IMAPMAGIC, sizeof( cMagic ) ) )
    {
        long        nCount = 128;

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
                    nRet = IMAP_FORMAT_CERN;
                }
                else
                    nRet = IMAP_FORMAT_NCSA;

                break;
            }
        }
    }

    rIStm.Seek( nPos );

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
