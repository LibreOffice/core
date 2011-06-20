/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <string.h>
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <vcl/wrkwin.hxx>
#include <sot/formats.hxx>

#include "svl/urihelper.hxx"
#include <svtools/imap.hxx>
#include <svtools/imap.hxx>
#include <svtools/imapobj.hxx>
#include <svtools/imaprect.hxx>
#include <svtools/imapcirc.hxx>
#include <svtools/imappoly.hxx>

#include <string.h>
#include <math.h>

#define NOTEOL(c) ((c)!='\0')

TYPEINIT0_AUTOFACTORY( ImageMap );

void IMapObject::AppendCERNCoords(rtl::OStringBuffer& rBuf, const Point& rPoint100) const
{
    const Point aPixPt( Application::GetDefaultDevice()->LogicToPixel( rPoint100, MapMode( MAP_100TH_MM ) ) );

    rBuf.append('(');
    rBuf.append(static_cast<sal_Int32>(aPixPt.X()));
    rBuf.append(',');
    rBuf.append(static_cast<sal_Int32>(aPixPt.Y()));
    rBuf.append(RTL_CONSTASCII_STRINGPARAM(") "));
}

void IMapObject::AppendNCSACoords(rtl::OStringBuffer& rBuf, const Point& rPoint100) const
{
    const Point aPixPt( Application::GetDefaultDevice()->LogicToPixel( rPoint100, MapMode( MAP_100TH_MM ) ) );

    rBuf.append(static_cast<sal_Int32>(aPixPt.X()));
    rBuf.append(',');
    rBuf.append(static_cast<sal_Int32>(aPixPt.Y()));
    rBuf.append(' ');
}

void IMapObject::AppendCERNURL(rtl::OStringBuffer& rBuf, const String& rBaseURL) const
{
    rBuf.append(rtl::OUStringToOString(URIHelper::simpleNormalizedMakeRelative(rBaseURL, aURL), gsl_getSystemTextEncoding()));
}

void IMapObject::AppendNCSAURL(rtl::OStringBuffer& rBuf, const String& rBaseURL) const
{
    rBuf.append(rtl::OUStringToOString(URIHelper::simpleNormalizedMakeRelative(rBaseURL, aURL), gsl_getSystemTextEncoding()));
    rBuf.append(' ');
}

void IMapRectangleObject::WriteCERN( SvStream& rOStm, const String& rBaseURL ) const
{
    rtl::OStringBuffer aStrBuf(RTL_CONSTASCII_STRINGPARAM("rectangle "));

    AppendCERNCoords(aStrBuf, aRect.TopLeft());
    AppendCERNCoords(aStrBuf, aRect.BottomRight());
    AppendCERNURL(aStrBuf, rBaseURL);

    rOStm.WriteLine(aStrBuf.makeStringAndClear());
}

void IMapRectangleObject::WriteNCSA( SvStream& rOStm, const String& rBaseURL ) const
{
    rtl::OStringBuffer aStrBuf(RTL_CONSTASCII_STRINGPARAM("rect "));

    AppendNCSAURL(aStrBuf, rBaseURL);
    AppendNCSACoords(aStrBuf, aRect.TopLeft());
    AppendNCSACoords(aStrBuf, aRect.BottomRight());

    rOStm.WriteLine(aStrBuf.makeStringAndClear());
}

void IMapCircleObject::WriteCERN( SvStream& rOStm, const String& rBaseURL ) const
{
    rtl::OStringBuffer aStrBuf(RTL_CONSTASCII_STRINGPARAM("circle "));

    AppendCERNCoords(aStrBuf, aCenter);
    aStrBuf.append(nRadius);
    aStrBuf.append(' ');
    AppendCERNURL(aStrBuf, rBaseURL);

    rOStm.WriteLine(aStrBuf.makeStringAndClear());
}

void IMapCircleObject::WriteNCSA( SvStream& rOStm, const String& rBaseURL ) const
{
    rtl::OStringBuffer aStrBuf(RTL_CONSTASCII_STRINGPARAM("circle "));

    AppendNCSAURL(aStrBuf, rBaseURL);
    AppendNCSACoords(aStrBuf, aCenter);
    AppendNCSACoords(aStrBuf, aCenter + Point(nRadius, 0));

    rOStm.WriteLine(aStrBuf.makeStringAndClear());
}

void IMapPolygonObject::WriteCERN( SvStream& rOStm, const String& rBaseURL  ) const
{
    rtl::OStringBuffer aStrBuf(RTL_CONSTASCII_STRINGPARAM("polygon "));
    const sal_uInt16 nCount = aPoly.GetSize();

    for (sal_uInt16 i = 0; i < nCount; ++i)
        AppendCERNCoords(aStrBuf, aPoly[i]);

    AppendCERNURL(aStrBuf, rBaseURL);

    rOStm.WriteLine(aStrBuf.makeStringAndClear());
}

void IMapPolygonObject::WriteNCSA( SvStream& rOStm, const String& rBaseURL  ) const
{
    rtl::OStringBuffer aStrBuf(RTL_CONSTASCII_STRINGPARAM("poly "));
    const sal_uInt16 nCount = Min( aPoly.GetSize(), (sal_uInt16) 100 );

    AppendNCSAURL(aStrBuf, rBaseURL);

    for (sal_uInt16 i = 0; i < nCount; ++i)
        AppendNCSACoords(aStrBuf, aPoly[i]);

    rOStm.WriteLine(aStrBuf.makeStringAndClear());
}

void ImageMap::Write( SvStream& rOStm, sal_uLong nFormat, const String& rBaseURL ) const
{
    switch( nFormat )
    {
        case( IMAP_FORMAT_BIN ) : Write( rOStm, rBaseURL );
        case( IMAP_FORMAT_CERN ) : ImpWriteCERN( rOStm, rBaseURL ); break;
        case( IMAP_FORMAT_NCSA ) : ImpWriteNCSA( rOStm, rBaseURL ); break;

        default:
        break;
    }
}

void ImageMap::ImpWriteCERN( SvStream& rOStm, const String& rBaseURL ) const
{
    IMapObject* pObj;
    sal_uInt16      nCount = (sal_uInt16) maList.Count();

    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        pObj = GetIMapObject( i );

        switch( pObj->GetType() )
        {
            case( IMAP_OBJ_RECTANGLE ):
                ( (IMapRectangleObject*) pObj )->WriteCERN( rOStm, rBaseURL );
            break;

            case( IMAP_OBJ_CIRCLE ):
                ( (IMapCircleObject*) pObj )->WriteCERN( rOStm, rBaseURL );
            break;

            case( IMAP_OBJ_POLYGON ):
                ( (IMapPolygonObject*) pObj )->WriteCERN( rOStm, rBaseURL );
            break;

            default:
            break;
        }
    }
}

void ImageMap::ImpWriteNCSA( SvStream& rOStm, const String& rBaseURL  ) const
{
    IMapObject* pObj;
    sal_uInt16      nCount = (sal_uInt16) maList.Count();

    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        pObj = GetIMapObject( i );

            switch( pObj->GetType() )
        {
            case( IMAP_OBJ_RECTANGLE ):
                ( (IMapRectangleObject*) pObj )->WriteNCSA( rOStm, rBaseURL );
            break;

            case( IMAP_OBJ_CIRCLE ):
                ( (IMapCircleObject*) pObj )->WriteNCSA( rOStm, rBaseURL );
            break;

            case( IMAP_OBJ_POLYGON ):
                ( (IMapPolygonObject*) pObj )->WriteNCSA( rOStm, rBaseURL );
            break;

            default:
            break;
        }
    }
}

sal_uLong ImageMap::Read( SvStream& rIStm, sal_uLong nFormat, const String& rBaseURL  )
{
    sal_uLong nRet = IMAP_ERR_FORMAT;

    if ( nFormat == IMAP_FORMAT_DETECT )
        nFormat = ImpDetectFormat( rIStm );

    switch ( nFormat )
    {
        case ( IMAP_FORMAT_BIN )    : Read( rIStm, rBaseURL ); break;
        case ( IMAP_FORMAT_CERN )   : nRet = ImpReadCERN( rIStm, rBaseURL ); break;
        case ( IMAP_FORMAT_NCSA )   : nRet = ImpReadNCSA( rIStm, rBaseURL ); break;

        default:
        break;
    }

    if ( !rIStm.GetError() )
        nRet = IMAP_ERR_OK;

    return nRet;
}

sal_uLong ImageMap::ImpReadCERN( SvStream& rIStm, const String& rBaseURL )
{
    ByteString aStr;

    // alten Inhalt loeschen
    ClearImageMap();

    while ( rIStm.ReadLine( aStr ) )
        ImpReadCERNLine( aStr, rBaseURL );

    return IMAP_ERR_OK;
}

void ImageMap::ImpReadCERNLine( const ByteString& rLine, const String& rBaseURL  )
{
    ByteString  aStr( rLine );
    ByteString  aToken;

    aStr.EraseLeadingChars( ' ' );
    aStr.EraseLeadingChars( '\t' );
    aStr.EraseAllChars( ';' );
    aStr.ToLowerAscii();

    const char* pStr = aStr.GetBuffer();
    char        cChar = *pStr++;

        // Anweisung finden
    while( ( cChar >= 'a' ) && ( cChar <= 'z' ) && NOTEOL( cChar ) )
    {
        aToken += cChar;
        cChar = *pStr++;
    }

    if ( NOTEOL( cChar ) )
    {
        if ( ( aToken == "rectangle" ) || ( aToken == "rect" ) )
        {
            const Point     aTopLeft( ImpReadCERNCoords( &pStr ) );
            const Point     aBottomRight( ImpReadCERNCoords( &pStr ) );
            const String    aURL( ImpReadCERNURL( &pStr, rBaseURL ) );
            const Rectangle aRect( aTopLeft, aBottomRight );

            IMapRectangleObject* pObj = new IMapRectangleObject( aRect, aURL, String(), String(), String(), String() );
            maList.Insert( pObj, LIST_APPEND );
        }
        else if ( ( aToken == "circle" ) || ( aToken == "circ" ) )
        {
            const Point     aCenter( ImpReadCERNCoords( &pStr ) );
            const long      nRadius = ImpReadCERNRadius( &pStr );
            const String    aURL( ImpReadCERNURL( &pStr, rBaseURL ) );

            IMapCircleObject* pObj = new IMapCircleObject( aCenter, nRadius, aURL, String(), String(), String(), String() );
            maList.Insert( pObj, LIST_APPEND );
        }
        else if ( ( aToken == "polygon" ) || ( aToken == "poly" ) )
        {
            const sal_uInt16    nCount = aStr.GetTokenCount( '(' ) - 1;
            Polygon         aPoly( nCount );
            String          aURL;

            for ( sal_uInt16 i = 0; i < nCount; i++ )
                aPoly[ i ] = ImpReadCERNCoords( &pStr );

            aURL = ImpReadCERNURL( &pStr, rBaseURL );

            IMapPolygonObject* pObj = new IMapPolygonObject( aPoly, aURL, String(), String(), String(), String() );
            maList.Insert( pObj, LIST_APPEND );
        }
    }
}

Point ImageMap::ImpReadCERNCoords( const char** ppStr )
{
    String  aStrX;
    String  aStrY;
    Point   aPt;
    char    cChar = *(*ppStr)++;

    while( NOTEOL( cChar ) && ( ( cChar < '0' ) || ( cChar > '9' ) ) )
        cChar = *(*ppStr)++;

    if ( NOTEOL( cChar ) )
    {
        while( NOTEOL( cChar ) && ( cChar >= '0' ) && ( cChar <= '9' ) )
        {
            aStrX += cChar;
            cChar = *(*ppStr)++;
        }

        if ( NOTEOL( cChar ) )
        {
            while( NOTEOL( cChar ) && ( ( cChar < '0' ) || ( cChar > '9' ) ) )
                cChar = *(*ppStr)++;

            while( NOTEOL( cChar ) && ( cChar >= '0' ) && ( cChar <= '9' ) )
            {
                aStrY += cChar;
                cChar = *(*ppStr)++;
            }

            if ( NOTEOL( cChar ) )
                while( NOTEOL( cChar ) && ( cChar != ')' ) )
                    cChar = *(*ppStr)++;

            aPt = Point( aStrX.ToInt32(), aStrY.ToInt32() );
        }
    }

    return aPt;
}

long ImageMap::ImpReadCERNRadius( const char** ppStr )
{
    String  aStr;
    char    cChar = *(*ppStr)++;

    while( NOTEOL( cChar ) && ( ( cChar < '0' ) || ( cChar > '9' ) ) )
        cChar = *(*ppStr)++;

    if ( NOTEOL( cChar ) )
    {
        while( NOTEOL( cChar ) && ( cChar >= '0' ) && ( cChar <= '9' ) )
        {
            aStr += cChar;
            cChar = *(*ppStr)++;
        }
    }

    return aStr.ToInt32();
}

String ImageMap::ImpReadCERNURL( const char** ppStr, const String& rBaseURL )
{
    String  aStr( String::CreateFromAscii( *ppStr ) );

    aStr.EraseLeadingChars( ' ' );
    aStr.EraseLeadingChars( '\t' );
    aStr.EraseTrailingChars( ' ' );
    aStr.EraseTrailingChars( '\t' );

    return INetURLObject::GetAbsURL( rBaseURL, aStr );
}

sal_uLong ImageMap::ImpReadNCSA( SvStream& rIStm, const String& rBaseURL )
{
    ByteString aStr;

    // alten Inhalt loeschen
    ClearImageMap();

    while ( rIStm.ReadLine( aStr ) )
        ImpReadNCSALine( aStr, rBaseURL );

    return IMAP_ERR_OK;
}

void ImageMap::ImpReadNCSALine( const ByteString& rLine, const String& rBaseURL )
{
    ByteString  aStr( rLine );
    ByteString  aToken;

    aStr.EraseLeadingChars( ' ' );
    aStr.EraseLeadingChars( '\t' );
    aStr.EraseAllChars( ';' );
    aStr.ToLowerAscii();

    const char* pStr = aStr.GetBuffer();
    char        cChar = *pStr++;

        // Anweisung finden
    while( ( cChar >= 'a' ) && ( cChar <= 'z' ) && NOTEOL( cChar ) )
    {
        aToken += cChar;
        cChar = *pStr++;
    }

    if ( NOTEOL( cChar ) )
    {
        if ( aToken == "rect" )
        {
            const String    aURL( ImpReadNCSAURL( &pStr, rBaseURL ) );
            const Point     aTopLeft( ImpReadNCSACoords( &pStr ) );
            const Point     aBottomRight( ImpReadNCSACoords( &pStr ) );
            const Rectangle aRect( aTopLeft, aBottomRight );

            IMapRectangleObject* pObj = new IMapRectangleObject( aRect, aURL, String(), String(), String(), String() );
            maList.Insert( pObj, LIST_APPEND );
        }
        else if ( aToken == "circle" )
        {
            const String    aURL( ImpReadNCSAURL( &pStr, rBaseURL ) );
            const Point     aCenter( ImpReadNCSACoords( &pStr ) );
            const Point     aDX( aCenter - ImpReadNCSACoords( &pStr ) );
            long            nRadius = (long) sqrt( (double) aDX.X() * aDX.X() +
                                                   (double) aDX.Y() * aDX.Y() );

            IMapCircleObject* pObj = new IMapCircleObject( aCenter, nRadius, aURL, String(), String(), String(), String() );
            maList.Insert( pObj, LIST_APPEND );
        }
        else if ( aToken == "poly" )
        {
            const sal_uInt16    nCount = aStr.GetTokenCount( ',' ) - 1;
            const String    aURL( ImpReadNCSAURL( &pStr, rBaseURL ) );
            Polygon         aPoly( nCount );

            for ( sal_uInt16 i = 0; i < nCount; i++ )
                aPoly[ i ] = ImpReadNCSACoords( &pStr );

            IMapPolygonObject* pObj = new IMapPolygonObject( aPoly, aURL, String(), String(), String(), String() );
            maList.Insert( pObj, LIST_APPEND );
        }
    }
}

String ImageMap::ImpReadNCSAURL( const char** ppStr, const String& rBaseURL )
{
    String  aStr;
    char    cChar = *(*ppStr)++;

    while( NOTEOL( cChar ) && ( ( cChar == ' ' ) || ( cChar == '\t' ) ) )
        cChar = *(*ppStr)++;

    if ( NOTEOL( cChar ) )
    {
        while( NOTEOL( cChar ) && ( cChar != ' ' ) && ( cChar != '\t' ) )
        {
            aStr += cChar;
            cChar = *(*ppStr)++;
        }
    }

    return INetURLObject::GetAbsURL( rBaseURL, aStr );
}

Point ImageMap::ImpReadNCSACoords( const char** ppStr )
{
    String  aStrX;
    String  aStrY;
    Point   aPt;
    char    cChar = *(*ppStr)++;

    while( NOTEOL( cChar ) && ( ( cChar < '0' ) || ( cChar > '9' ) ) )
        cChar = *(*ppStr)++;

    if ( NOTEOL( cChar ) )
    {
        while( NOTEOL( cChar ) && ( cChar >= '0' ) && ( cChar <= '9' ) )
        {
            aStrX += cChar;
            cChar = *(*ppStr)++;
        }

        if ( NOTEOL( cChar ) )
        {
            while( NOTEOL( cChar ) && ( ( cChar < '0' ) || ( cChar > '9' ) ) )
                cChar = *(*ppStr)++;

            while( NOTEOL( cChar ) && ( cChar >= '0' ) && ( cChar <= '9' ) )
            {
                aStrY += cChar;
                cChar = *(*ppStr)++;
            }

            aPt = Point( aStrX.ToInt32(), aStrY.ToInt32() );
        }
    }

    return aPt;
}

sal_uLong ImageMap::ImpDetectFormat( SvStream& rIStm )
{
    sal_uLong   nPos = rIStm.Tell();
    sal_uLong   nRet = IMAP_FORMAT_BIN;
    char    cMagic[6];

    rIStm.Read( cMagic, sizeof( cMagic ) );

    // Falls wir kein internes Format haben,
    // untersuchen wir das Format
    if ( memcmp( cMagic, IMAPMAGIC, sizeof( cMagic ) ) )
    {
        ByteString  aStr;
        long        nCount = 128;

        rIStm.Seek( nPos );
        while ( rIStm.ReadLine( aStr ) && nCount-- )
        {
            aStr.ToLowerAscii();

            if ( ( aStr.Search( "rect" ) != STRING_NOTFOUND ) ||
                 ( aStr.Search( "circ" ) != STRING_NOTFOUND ) ||
                 ( aStr.Search( "poly" ) != STRING_NOTFOUND ) )
            {
                if ( ( aStr.Search( '(' ) != STRING_NOTFOUND ) &&
                     ( aStr.Search( ')' ) != STRING_NOTFOUND ) )
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
