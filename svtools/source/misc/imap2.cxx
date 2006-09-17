/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imap2.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:11:20 $
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
#include "precompiled_svtools.hxx"

#ifdef WIN
#include <sysdep.hxx>
#endif
#include <string.h>
#include <math.h>

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif

#include "urihelper.hxx"
#include "imap.hxx"
#include "imap.hxx"
#include "imapobj.hxx"
#include "imaprect.hxx"
#include "imapcirc.hxx"
#include "imappoly.hxx"

#define NOTEOL(c) ((c)!='\0')


TYPEINIT0_AUTOFACTORY( ImageMap );


/******************************************************************************/
/******************************************************************************/


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void IMapObject::AppendCERNCoords( const Point& rPoint100, ByteString& rStr ) const
{
    const Point aPixPt( Application::GetDefaultDevice()->LogicToPixel( rPoint100, MapMode( MAP_100TH_MM ) ) );

    rStr += '(';
    rStr += ByteString::CreateFromInt32( aPixPt.X() );
    rStr += ',';
    rStr += ByteString::CreateFromInt32( aPixPt.Y() );
    rStr += ") ";
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void IMapObject::AppendNCSACoords( const Point& rPoint100, ByteString& rStr ) const
{
    const Point aPixPt( Application::GetDefaultDevice()->LogicToPixel( rPoint100, MapMode( MAP_100TH_MM ) ) );

    rStr += ByteString::CreateFromInt32( aPixPt.X() );
    rStr += ',';
    rStr += ByteString::CreateFromInt32( aPixPt.Y() );
    rStr += ' ';
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void IMapObject::AppendCERNURL( ByteString& rStr, const String& rBaseURL ) const
{
    rStr += ByteString( String(URIHelper::simpleNormalizedMakeRelative( rBaseURL, aURL )), gsl_getSystemTextEncoding() );
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void IMapObject::AppendNCSAURL( ByteString& rStr, const String& rBaseURL ) const
{
    rStr += ByteString( String(URIHelper::simpleNormalizedMakeRelative( rBaseURL, aURL )), gsl_getSystemTextEncoding() );
    rStr += ' ';
}


/******************************************************************************/
/******************************************************************************/


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void IMapRectangleObject::WriteCERN( SvStream& rOStm, const String& rBaseURL ) const
{
    ByteString aStr( "rectangle " );

    AppendCERNCoords( aRect.TopLeft(), aStr );
    AppendCERNCoords( aRect.BottomRight(), aStr );
    AppendCERNURL( aStr, rBaseURL );

    rOStm.WriteLine( aStr );
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void IMapRectangleObject::WriteNCSA( SvStream& rOStm, const String& rBaseURL ) const
{
    ByteString aStr( "rect " );

    AppendNCSAURL( aStr, rBaseURL );
    AppendNCSACoords( aRect.TopLeft(), aStr );
    AppendNCSACoords( aRect.BottomRight(), aStr );

    rOStm.WriteLine( aStr );
}


/******************************************************************************/
/******************************************************************************/


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void IMapCircleObject::WriteCERN( SvStream& rOStm, const String& rBaseURL ) const
{
    ByteString aStr( "circle " );

    AppendCERNCoords( aCenter, aStr );
    aStr += ByteString::CreateFromInt32(nRadius);
    aStr += ' ';
    AppendCERNURL( aStr, rBaseURL );

    rOStm.WriteLine( aStr );
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void IMapCircleObject::WriteNCSA( SvStream& rOStm, const String& rBaseURL ) const
{
    ByteString aStr( "circle " );

    AppendNCSAURL( aStr, rBaseURL );
    AppendNCSACoords( aCenter, aStr );
    AppendNCSACoords( aCenter + Point( nRadius, 0 ), aStr );

    rOStm.WriteLine( aStr );
}


/******************************************************************************/
/******************************************************************************/


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void IMapPolygonObject::WriteCERN( SvStream& rOStm, const String& rBaseURL  ) const
{
    ByteString      aStr( "polygon " );
    const USHORT    nCount = aPoly.GetSize();

    for ( USHORT i = 0; i < nCount; i++ )
        AppendCERNCoords( aPoly[ i ], aStr );

    AppendCERNURL( aStr, rBaseURL );

    rOStm.WriteLine( aStr );
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void IMapPolygonObject::WriteNCSA( SvStream& rOStm, const String& rBaseURL  ) const
{
    ByteString      aStr( "poly " );
    const USHORT    nCount = Min( aPoly.GetSize(), (USHORT) 100 );

    AppendNCSAURL( aStr, rBaseURL );

    for ( USHORT i = 0; i < nCount; i++ )
        AppendNCSACoords( aPoly[ i ], aStr );

    rOStm.WriteLine( aStr );
}


/******************************************************************************/
/******************************************************************************/


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void ImageMap::Write( SvStream& rOStm, ULONG nFormat, const String& rBaseURL ) const
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


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void ImageMap::ImpWriteCERN( SvStream& rOStm, const String& rBaseURL ) const
{
    IMapObject* pObj;
    USHORT      nCount = (USHORT) maList.Count();

    for ( USHORT i = 0; i < nCount; i++ )
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


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void ImageMap::ImpWriteNCSA( SvStream& rOStm, const String& rBaseURL  ) const
{
    IMapObject* pObj;
    USHORT      nCount = (USHORT) maList.Count();

    for ( USHORT i = 0; i < nCount; i++ )
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


/******************************************************************************
|*
|*
|*
\******************************************************************************/

ULONG ImageMap::Read( SvStream& rIStm, ULONG nFormat, const String& rBaseURL  )
{
    ULONG nRet = IMAP_ERR_FORMAT;

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


/******************************************************************************
|*
|*
|*
\******************************************************************************/

ULONG ImageMap::ImpReadCERN( SvStream& rIStm, const String& rBaseURL )
{
    ByteString aStr;

    // alten Inhalt loeschen
    ClearImageMap();

    while ( rIStm.ReadLine( aStr ) )
        ImpReadCERNLine( aStr, rBaseURL );

    return IMAP_ERR_OK;
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

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

            IMapRectangleObject* pObj = new IMapRectangleObject( aRect, aURL, String() );
            maList.Insert( pObj, LIST_APPEND );
        }
        else if ( ( aToken == "circle" ) || ( aToken == "circ" ) )
        {
            const Point     aCenter( ImpReadCERNCoords( &pStr ) );
            const long      nRadius = ImpReadCERNRadius( &pStr );
            const String    aURL( ImpReadCERNURL( &pStr, rBaseURL ) );

            IMapCircleObject* pObj = new IMapCircleObject( aCenter, nRadius, aURL, String() );
            maList.Insert( pObj, LIST_APPEND );
        }
        else if ( ( aToken == "polygon" ) || ( aToken == "poly" ) )
        {
            const USHORT    nCount = aStr.GetTokenCount( '(' ) - 1;
            Polygon         aPoly( nCount );
            String          aURL;

            for ( USHORT i = 0; i < nCount; i++ )
                aPoly[ i ] = ImpReadCERNCoords( &pStr );

            aURL = ImpReadCERNURL( &pStr, rBaseURL );

            IMapPolygonObject* pObj = new IMapPolygonObject( aPoly, aURL, String() );
            maList.Insert( pObj, LIST_APPEND );
        }
    }
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

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


/******************************************************************************
|*
|*
|*
\******************************************************************************/

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


/******************************************************************************
|*
|*
|*
\******************************************************************************/

String ImageMap::ImpReadCERNURL( const char** ppStr, const String& rBaseURL )
{
    String  aStr( String::CreateFromAscii( *ppStr ) );

    aStr.EraseLeadingChars( ' ' );
    aStr.EraseLeadingChars( '\t' );
    aStr.EraseTrailingChars( ' ' );
    aStr.EraseTrailingChars( '\t' );

    return INetURLObject::GetAbsURL( rBaseURL, aStr );
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

ULONG ImageMap::ImpReadNCSA( SvStream& rIStm, const String& rBaseURL )
{
    ByteString aStr;

    // alten Inhalt loeschen
    ClearImageMap();

    while ( rIStm.ReadLine( aStr ) )
        ImpReadNCSALine( aStr, rBaseURL );

    return IMAP_ERR_OK;
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

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

            IMapRectangleObject* pObj = new IMapRectangleObject( aRect, aURL, String() );
            maList.Insert( pObj, LIST_APPEND );
        }
        else if ( aToken == "circle" )
        {
            const String    aURL( ImpReadNCSAURL( &pStr, rBaseURL ) );
            const Point     aCenter( ImpReadNCSACoords( &pStr ) );
            const Point     aDX( aCenter - ImpReadNCSACoords( &pStr ) );
            long            nRadius = (long) sqrt( (double) aDX.X() * aDX.X() +
                                                   (double) aDX.Y() * aDX.Y() );

            IMapCircleObject* pObj = new IMapCircleObject( aCenter, nRadius, aURL, String() );
            maList.Insert( pObj, LIST_APPEND );
        }
        else if ( aToken == "poly" )
        {
            const USHORT    nCount = aStr.GetTokenCount( ',' ) - 1;
            const String    aURL( ImpReadNCSAURL( &pStr, rBaseURL ) );
            Polygon         aPoly( nCount );

            for ( USHORT i = 0; i < nCount; i++ )
                aPoly[ i ] = ImpReadNCSACoords( &pStr );

            IMapPolygonObject* pObj = new IMapPolygonObject( aPoly, aURL, String() );
            maList.Insert( pObj, LIST_APPEND );
        }
    }
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

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


/******************************************************************************
|*
|*
|*
\******************************************************************************/

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


/******************************************************************************
|*
|*
|*
\******************************************************************************/

ULONG ImageMap::ImpDetectFormat( SvStream& rIStm )
{
    ULONG   nPos = rIStm.Tell();
    ULONG   nRet = IMAP_FORMAT_BIN;
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
