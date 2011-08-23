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

#include <tools/table.hxx>
#include <string.h>
#include <math.h>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/window.hxx>

#include "urihelper.hxx"
#include <bf_svtools/imap.hxx>
#include <bf_svtools/imapobj.hxx>
#include <bf_svtools/imapcirc.hxx>
#include <bf_svtools/imaprect.hxx>
#include <bf_svtools/imappoly.hxx>

namespace binfilter
{

DBG_NAME( ImageMap )


#define SCALEPOINT(aPT,aFracX,aFracY) (aPT).X()=((aPT).X()*(aFracX).GetNumerator())/(aFracX).GetDenominator();	\
                                      (aPT).Y()=((aPT).Y()*(aFracY).GetNumerator())/(aFracY).GetDenominator();


/******************************************************************************/

UINT16 IMapObject::nActualTextEncoding = (UINT16) RTL_TEXTENCODING_DONTKNOW;

/******************************************************************************/


#ifdef WIN
#pragma optimize ( "", off )
#endif

IMapObject::IMapObject( const String& rURL,	const String& rAltText, const String& rDesc,
                        const String& rTarget, const String& rName, BOOL bURLActive )
: aURL( rURL )
, aAltText( rAltText )
, aDesc( rDesc )
, aTarget( rTarget )
, aName( rName )
, bActive( bURLActive )
{
}


/******************************************************************************
|*
|* Freigabe des internen Speichers
|*
\******************************************************************************/

UINT16 IMapObject::GetVersion() const
{
    return IMAP_OBJ_VERSION;
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void IMapObject::Write( SvStream& rOStm, const String& rBaseURL ) const
{
    IMapCompat*				pCompat;
    const rtl_TextEncoding	eEncoding = gsl_getSystemTextEncoding();

    rOStm << GetType();
    rOStm << GetVersion();
    rOStm << ( (UINT16) eEncoding );

    const ByteString aRelURL = ByteString( String(::binfilter::simpleNormalizedMakeRelative( rBaseURL, aURL )), eEncoding );
    rOStm.WriteByteString( aRelURL );
    rOStm.WriteByteString( ByteString( aAltText, eEncoding ) );
    rOStm << bActive;
    rOStm.WriteByteString( ByteString( aTarget, eEncoding ) );

    pCompat = new IMapCompat( rOStm, STREAM_WRITE );

    WriteIMapObject( rOStm );
    aEventList.Write( rOStm );                                 // V4
    rOStm.WriteByteString( ByteString( aName, eEncoding ) );   // V5

    delete pCompat;
}


/******************************************************************************
|*
|* 	Binaer-Import
|*
\******************************************************************************/

void IMapObject::Read( SvStream& rIStm, const String& rBaseURL )
{
    IMapCompat*			pCompat;
    rtl_TextEncoding	nTextEncoding;
    ByteString			aString;

    // Typ und Version ueberlesen wir
    rIStm.SeekRel( 2 );
    rIStm >> nReadVersion;
    rIStm >> nTextEncoding;
    rIStm.ReadByteString( aString ); aURL = String( aString.GetBuffer(), nTextEncoding );
    rIStm.ReadByteString( aString ); aAltText = String( aString.GetBuffer(), nTextEncoding );
    rIStm >> bActive;
    rIStm.ReadByteString( aString ); aTarget = String( aString.GetBuffer(), nTextEncoding );

    // URL absolut machen
    aURL = ::binfilter::SmartRel2Abs( INetURLObject(rBaseURL), aURL, ::binfilter::GetMaybeFileHdl(), true, false, INetURLObject::WAS_ENCODED, INetURLObject::DECODE_UNAMBIGUOUS );
    pCompat = new IMapCompat( rIStm, STREAM_READ );

    ReadIMapObject( rIStm );

    // ab Version 4 lesen wir eine EventListe
    if ( nReadVersion >= 0x0004 )
    {
        aEventList.Read(rIStm);

        // ab Version 5 kann ein Objektname vorhanden sein
        if ( nReadVersion >= 0x0005 )
        {
            rIStm.ReadByteString( aString ); aName = String( aString.GetBuffer(), nTextEncoding );
        }
    }

    delete pCompat;
}

/******************************************************************************
|*
|*
|*
\******************************************************************************/

BOOL IMapObject::IsEqual( const IMapObject& rEqObj )
{
    return ( ( aURL == rEqObj.aURL ) &&
             ( aAltText == rEqObj.aAltText ) &&
             ( aDesc == rEqObj.aDesc ) &&
             ( aTarget == rEqObj.aTarget ) &&
             ( aName == rEqObj.aName ) &&
             ( bActive == rEqObj.bActive ) );
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

IMapRectangleObject::IMapRectangleObject( const Rectangle& rRect,
                                          const String& rURL,
                                          const String& rAltText,
                                          const String& rDesc,
                                          const String&	rTarget,
                                          const String& rName,
                                          BOOL bURLActive,
                                          BOOL bPixelCoords ) :
            IMapObject	( rURL, rAltText, rDesc, rTarget, rName, bURLActive )
{
    ImpConstruct( rRect, bPixelCoords );
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void IMapRectangleObject::ImpConstruct( const Rectangle& rRect, BOOL bPixel )
{
    if ( bPixel )
        aRect = Application::GetDefaultDevice()->PixelToLogic( rRect, MapMode( MAP_100TH_MM ) );
    else
        aRect = rRect;
}


/******************************************************************************
|*
|* Binaer-Export
|*
\******************************************************************************/

void IMapRectangleObject::WriteIMapObject( SvStream& rOStm ) const
{
    rOStm << aRect;
}


/******************************************************************************
|*
|* Binaer-Import
|*
\******************************************************************************/

void IMapRectangleObject::ReadIMapObject( SvStream& rIStm )
{
    rIStm >> aRect;
}


/******************************************************************************
|*
|* Typ-Rueckgabe
|*
\******************************************************************************/

UINT16 IMapRectangleObject::GetType() const
{
    return IMAP_OBJ_RECTANGLE;
}


/******************************************************************************
|*
|* Hit-Test
|*
\******************************************************************************/

BOOL IMapRectangleObject::IsHit( const Point& rPoint ) const
{
    return aRect.IsInside( rPoint );
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

Rectangle IMapRectangleObject::GetRectangle( BOOL bPixelCoords ) const
{
    Rectangle	aNewRect;

    if ( bPixelCoords )
        aNewRect = Application::GetDefaultDevice()->LogicToPixel( aRect, MapMode( MAP_100TH_MM ) );
    else
        aNewRect = aRect;

    return aNewRect;
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void IMapRectangleObject::Scale( const Fraction& rFracX, const Fraction& rFracY )
{
    Point	aTL( aRect.TopLeft() );
    Point	aBR( aRect.BottomRight() );

    if ( rFracX.GetDenominator() && rFracY.GetDenominator() )
    {
        SCALEPOINT( aTL, rFracX, rFracY );
        SCALEPOINT( aBR, rFracX, rFracY );
    }

    aRect = Rectangle( aTL, aBR );
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

BOOL IMapRectangleObject::IsEqual( const IMapRectangleObject& rEqObj )
{
    return ( IMapObject::IsEqual( rEqObj ) && ( aRect == rEqObj.aRect ) );
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

IMapCircleObject::IMapCircleObject( const Point& rCenter, ULONG nCircleRadius,
                                    const String& rURL,
                                    const String& rAltText,
                                    const String& rDesc,
                                    const String& rTarget,
                                    const String& rName,
                                    BOOL bURLActive,
                                    BOOL bPixelCoords ) :
            IMapObject	( rURL, rAltText, rDesc, rTarget, rName, bURLActive )
{
    ImpConstruct( rCenter, nCircleRadius, bPixelCoords );
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void IMapCircleObject::ImpConstruct( const Point& rCenter, ULONG nRad, BOOL bPixel )
{
    if ( bPixel )
    {
        MapMode	aMap100( MAP_100TH_MM );

        aCenter = Application::GetDefaultDevice()->PixelToLogic( rCenter, aMap100 );
        nRadius = Application::GetDefaultDevice()->PixelToLogic( Size( nRad, 0 ), aMap100 ).Width();
    }
    else
    {
        aCenter = rCenter;
        nRadius = nRad;
    }
}


/******************************************************************************
|*
|* Binaer-Export
|*
\******************************************************************************/

void IMapCircleObject::WriteIMapObject( SvStream& rOStm ) const
{
    UINT32 nTmp = nRadius;

    rOStm << aCenter;
    rOStm << nTmp;
}


/******************************************************************************
|*
|* Binaer-Import
|*
\******************************************************************************/

void IMapCircleObject::ReadIMapObject( SvStream& rIStm )
{
    UINT32 nTmp;

    rIStm >> aCenter;
    rIStm >> nTmp;

    nRadius = nTmp;
}


/******************************************************************************
|*
|* Typ-Rueckgabe
|*
\******************************************************************************/

UINT16 IMapCircleObject::GetType() const
{
    return IMAP_OBJ_CIRCLE;
}


/******************************************************************************
|*
|* Hit-Test
|*
\******************************************************************************/

BOOL IMapCircleObject::IsHit( const Point& rPoint ) const
{
    const Point	aPoint( aCenter - rPoint );
    BOOL		bRet = FALSE;

    if ( (ULONG) sqrt( (double) aPoint.X() * aPoint.X() +
                       aPoint.Y() * aPoint.Y() ) <= nRadius )
    {
        bRet = TRUE;
    }

    return bRet;
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

Point IMapCircleObject::GetCenter( BOOL bPixelCoords ) const
{
    Point aNewPoint;

    if ( bPixelCoords )
        aNewPoint = Application::GetDefaultDevice()->LogicToPixel( aCenter, MapMode( MAP_100TH_MM ) );
    else
        aNewPoint = aCenter;

    return aNewPoint;
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

ULONG IMapCircleObject::GetRadius( BOOL bPixelCoords ) const
{
    ULONG nNewRadius;

    if ( bPixelCoords )
        nNewRadius = Application::GetDefaultDevice()->LogicToPixel( Size( nRadius, 0 ), MapMode( MAP_100TH_MM ) ).Width();
    else
        nNewRadius = nRadius;

    return nNewRadius;
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

Rectangle IMapCircleObject::GetBoundRect() const
{
    long nWidth = nRadius << 1;

    return Rectangle( Point(  aCenter.X() - nRadius, aCenter.Y() - nRadius ),
                      Size( nWidth, nWidth ) );
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void IMapCircleObject::Scale( const Fraction& rFracX, const Fraction& rFracY )
{
    Fraction aAverage( rFracX );

    aAverage += rFracY;
    aAverage *= Fraction( 1, 2 );

    if ( rFracX.GetDenominator() && rFracY.GetDenominator() )
    {
        SCALEPOINT( aCenter, rFracX, rFracY );
    }

    nRadius = ( nRadius * aAverage.GetNumerator() ) / aAverage.GetDenominator();
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

BOOL IMapCircleObject::IsEqual( const IMapCircleObject& rEqObj )
{
    return ( IMapObject::IsEqual( rEqObj ) &&
             ( aCenter == rEqObj.aCenter ) &&
             ( nRadius == rEqObj.nRadius ) );
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
IMapPolygonObject::IMapPolygonObject( const Polygon& rPoly,
                                      const String& rURL,
                                      const String& rAltText,
                                      const String& rDesc,
                                      const String& rTarget,
                                      const String& rName,
                                      BOOL bURLActive,
                                      BOOL bPixelCoords ) :
            IMapObject	( rURL, rAltText, rDesc, rTarget, rName, bURLActive ),
            bEllipse	( FALSE )
{
    ImpConstruct( rPoly, bPixelCoords );
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void IMapPolygonObject::ImpConstruct( const Polygon& rPoly, BOOL bPixel )
{
    if ( bPixel )
        aPoly = Application::GetDefaultDevice()->PixelToLogic( rPoly, MapMode( MAP_100TH_MM ) );
    else
        aPoly = rPoly;
}


/******************************************************************************
|*
|* Binaer-Export
|*
\******************************************************************************/

void IMapPolygonObject::WriteIMapObject( SvStream& rOStm ) const
{
    rOStm << aPoly;
    rOStm << bEllipse;	// >= Version 2
    rOStm << aEllipse;	// >= Version 2
}


/******************************************************************************
|*
|* Binaer-Import
|*
\******************************************************************************/

void IMapPolygonObject::ReadIMapObject( SvStream& rIStm )
{
    rIStm >> aPoly;

    // Version >= 2 hat zusaetzlich Ellipsen-Information
    if ( nReadVersion >= 2 )
    {
        rIStm >> bEllipse;
        rIStm >> aEllipse;
    }
}


/******************************************************************************
|*
|* Typ-Rueckgabe
|*
\******************************************************************************/

UINT16 IMapPolygonObject::GetType() const
{
    return IMAP_OBJ_POLYGON;
}


/******************************************************************************
|*
|* Hit-Test
|*
\******************************************************************************/

BOOL IMapPolygonObject::IsHit( const Point& rPoint ) const
{
    return aPoly.IsInside( rPoint );
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

Polygon IMapPolygonObject::GetPolygon( BOOL bPixelCoords ) const
{
    Polygon	aNewPoly;

    if ( bPixelCoords )
        aNewPoly = Application::GetDefaultDevice()->LogicToPixel( aPoly, MapMode( MAP_100TH_MM ) );
    else
        aNewPoly = aPoly;

    return aNewPoly;
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void IMapPolygonObject::Scale( const Fraction& rFracX, const Fraction& rFracY )
{
    USHORT nCount = aPoly.GetSize();

    for ( USHORT i = 0; i < nCount; i++ )
    {
        Point aScaledPt( aPoly[ i ] );

        if ( rFracX.GetDenominator() && rFracY.GetDenominator() )
        {
            SCALEPOINT( aScaledPt, rFracX, rFracY );
        }

        aPoly[ i ] = aScaledPt;
    }

    if ( bEllipse )
    {
        Point	aTL( aEllipse.TopLeft() );
        Point	aBR( aEllipse.BottomRight() );

        if ( rFracX.GetDenominator() && rFracY.GetDenominator() )
        {
            SCALEPOINT( aTL, rFracX, rFracY );
            SCALEPOINT( aBR, rFracX, rFracY );
        }

        aEllipse = Rectangle( aTL, aBR );
    }
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

BOOL IMapPolygonObject::IsEqual( const IMapPolygonObject& rEqObj )
{
    BOOL bRet = FALSE;

    if ( IMapObject::IsEqual( rEqObj ) )
    {
        const Polygon&	rEqPoly = rEqObj.aPoly;
        const USHORT	nCount = aPoly.GetSize();
        const USHORT	nEqCount = rEqPoly.GetSize();
        BOOL			bDifferent = FALSE;

        if ( nCount == nEqCount )
        {
            for ( USHORT i = 0; i < nCount; i++ )
            {
                if ( aPoly[ i ] != rEqPoly[ i ] )
                {
                    bDifferent = TRUE;
                    break;
                }
            }

            if ( !bDifferent )
                bRet = TRUE;
        }
    }

    return bRet;
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************
|*
|* Copy-Ctor
|*
\******************************************************************************/

ImageMap::ImageMap( const ImageMap& rImageMap )
{
    DBG_CTOR( ImageMap, NULL );

    USHORT nCount = rImageMap.GetIMapObjectCount();

    for ( USHORT i = 0; i < nCount; i++ )
    {
        IMapObject* pCopyObj = rImageMap.GetIMapObject( i );

        switch( pCopyObj->GetType() )
        {
            case( IMAP_OBJ_RECTANGLE ):
                maList.Insert( new IMapRectangleObject( *(IMapRectangleObject*) pCopyObj ), LIST_APPEND );
            break;

            case( IMAP_OBJ_CIRCLE ):
                maList.Insert( new IMapCircleObject( *(IMapCircleObject*) pCopyObj ), LIST_APPEND );
            break;

            case( IMAP_OBJ_POLYGON ):
                maList.Insert( new IMapPolygonObject( *(IMapPolygonObject*) pCopyObj ), LIST_APPEND );
            break;

            default:
            break;
        }
    }

    aName = rImageMap.aName;
}


/******************************************************************************
|*
|* Dtor
|*
\******************************************************************************/

ImageMap::~ImageMap()
{
    DBG_DTOR( ImageMap, NULL );

    ClearImageMap();
}


/******************************************************************************
|*
|* Freigabe des internen Speichers
|*
\******************************************************************************/

void ImageMap::ClearImageMap()
{
    IMapObject* pObj = (IMapObject*) maList.First();

    while ( pObj )
    {
        delete pObj;
        pObj = (IMapObject*) maList.Next();
    }

    maList.Clear();

    aName = String();
}


/******************************************************************************
|*
|* Zuweisungsoperator
|*
\******************************************************************************/

ImageMap& ImageMap::operator=( const ImageMap& rImageMap )
{
    USHORT nCount = rImageMap.GetIMapObjectCount();

    ClearImageMap();

    for ( USHORT i = 0; i < nCount; i++ )
    {
        IMapObject* pCopyObj = rImageMap.GetIMapObject( i );

        switch( pCopyObj->GetType() )
        {
            case( IMAP_OBJ_RECTANGLE ):
                maList.Insert( new IMapRectangleObject( *(IMapRectangleObject*) pCopyObj ), LIST_APPEND );
            break;

            case( IMAP_OBJ_CIRCLE ):
                maList.Insert( new IMapCircleObject( *(IMapCircleObject*) pCopyObj ), LIST_APPEND );
            break;

            case( IMAP_OBJ_POLYGON ):
                maList.Insert( new IMapPolygonObject( *(IMapPolygonObject*) pCopyObj ), LIST_APPEND );
            break;

            default:
            break;
        }
    }

    aName = rImageMap.aName;

    return *this;
}


/******************************************************************************
|*
|* Vergleichsoperator I
|*
\******************************************************************************/

BOOL ImageMap::operator==( const ImageMap& rImageMap )
{
    const USHORT	nCount = (USHORT) maList.Count();
    const USHORT	nEqCount = rImageMap.GetIMapObjectCount();
    BOOL			bRet = FALSE;

    if ( nCount == nEqCount )
    {
        BOOL bDifferent = ( aName != rImageMap.aName );

        for ( USHORT i = 0; ( i < nCount ) && !bDifferent; i++ )
        {
            IMapObject* pObj = (IMapObject*) maList.GetObject( i );
            IMapObject* pEqObj = rImageMap.GetIMapObject( i );

            if ( pObj->GetType() == pEqObj->GetType() )
            {
                switch( pObj->GetType() )
                {
                    case( IMAP_OBJ_RECTANGLE ):
                    {
                        if ( !( (IMapRectangleObject*) pObj )->IsEqual( *(IMapRectangleObject*) pEqObj ) )
                            bDifferent = TRUE;
                    }
                    break;

                    case( IMAP_OBJ_CIRCLE ):
                    {
                        if ( !( (IMapCircleObject*) pObj )->IsEqual( *(IMapCircleObject*) pEqObj ) )
                            bDifferent = TRUE;
                    }
                    break;

                    case( IMAP_OBJ_POLYGON ):
                    {
                        if ( !( (IMapPolygonObject*) pObj )->IsEqual( *(IMapPolygonObject*) pEqObj ) )
                            bDifferent = TRUE;
                    }
                    break;

                    default:
                    break;
                }
            }
            else
                bDifferent = TRUE;
        }

        if ( !bDifferent )
            bRet = TRUE;
    }

    return bRet;
}


/******************************************************************************
|*
|* Vergleichsoperator II
|*
\******************************************************************************/

BOOL ImageMap::operator!=( const ImageMap& rImageMap )
{
    return !( *this == rImageMap );
}


/******************************************************************************
|*
|* Freigabe des internen Speichers
|*
\******************************************************************************/

UINT16 ImageMap::GetVersion() const
{
    return IMAGE_MAP_VERSION;
}


/******************************************************************************
|*
|* Einfuegen eines neuen Objekts
|*
\******************************************************************************/

void ImageMap::InsertIMapObject( const IMapObject& rIMapObject )
{
    switch( rIMapObject.GetType() )
    {
        case( IMAP_OBJ_RECTANGLE ):
            maList.Insert( new IMapRectangleObject( (IMapRectangleObject&) rIMapObject ), LIST_APPEND );
        break;

        case( IMAP_OBJ_CIRCLE ):
            maList.Insert( new IMapCircleObject( (IMapCircleObject&) rIMapObject ), LIST_APPEND );
        break;

        case( IMAP_OBJ_POLYGON ):
            maList.Insert( new IMapPolygonObject( (IMapPolygonObject&) rIMapObject ), LIST_APPEND );
        break;

        default:
        break;
    }
}

/******************************************************************************
|*
|*
|*
\******************************************************************************/

void ImageMap::Scale( const Fraction& rFracX, const Fraction& rFracY )
{
    USHORT nCount = (USHORT) maList.Count();

    for ( USHORT i = 0; i < nCount; i++ )
    {
        IMapObject* pObj = GetIMapObject( i );

        switch( pObj->GetType() )
        {
            case( IMAP_OBJ_RECTANGLE ):
                ( (IMapRectangleObject*) pObj )->Scale( rFracX, rFracY );
            break;

            case( IMAP_OBJ_CIRCLE ):
                ( (IMapCircleObject*) pObj )->Scale( rFracX, rFracY );
            break;

            case( IMAP_OBJ_POLYGON ):
                ( (IMapPolygonObject*) pObj )->Scale( rFracX, rFracY );
            break;

            default:
            break;
        }
    }
}


/******************************************************************************
|*
|* Objekte nacheinander wegschreiben
|*
\******************************************************************************/

void ImageMap::ImpWriteImageMap( SvStream& rOStm, const String& rBaseURL ) const
{
    IMapObject* pObj;
    USHORT		nCount = (USHORT) maList.Count();

    for ( USHORT i = 0; i < nCount; i++ )
    {
        pObj = (IMapObject*) maList.GetObject( i );
        pObj->Write( rOStm, rBaseURL );
    }
}


/******************************************************************************
|*
|* Objekte nacheinander lesen
|*
\******************************************************************************/

void ImageMap::ImpReadImageMap( SvStream& rIStm, USHORT nCount, const String& rBaseURL )
{
    // neue Objekte einlesen
    for ( USHORT i = 0; i < nCount; i++ )
    {
        UINT16 nType;

        rIStm >> nType;
        rIStm.SeekRel( -2 );

        switch( nType )
        {
            case ( IMAP_OBJ_RECTANGLE ):
            {
                IMapRectangleObject* pObj = new IMapRectangleObject;
                pObj->Read( rIStm, rBaseURL );
                maList.Insert( pObj, LIST_APPEND );
            }
            break;

            case ( IMAP_OBJ_CIRCLE ):
            {
                IMapCircleObject* pObj = new IMapCircleObject;
                pObj->Read( rIStm, rBaseURL );
                maList.Insert( pObj, LIST_APPEND );
            }
            break;

            case ( IMAP_OBJ_POLYGON ):
            {
                IMapPolygonObject* pObj = new IMapPolygonObject;
                pObj->Read( rIStm, rBaseURL );
                maList.Insert( pObj, LIST_APPEND );
            }
            break;

            default:
            break;
        }
    }
}


/******************************************************************************
|*
|* Binaer speichern
|*
\******************************************************************************/

void ImageMap::Write( SvStream& rOStm, const String& rBaseURL ) const
{
    IMapCompat*				pCompat;
    String                  aImageName( GetName() );
    String					aDummy;
    USHORT					nOldFormat = rOStm.GetNumberFormatInt();
    UINT16                  nCount = (UINT16) GetIMapObjectCount();
    const rtl_TextEncoding	eEncoding = gsl_getSystemTextEncoding();

    rOStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    // MagicCode schreiben
    rOStm << IMAPMAGIC;
    rOStm << GetVersion();
    rOStm.WriteByteString( ByteString( aImageName, eEncoding ) );
    rOStm.WriteByteString( ByteString( aDummy, eEncoding ) );
    rOStm << nCount;
    rOStm.WriteByteString( ByteString( aImageName, eEncoding ) );

    pCompat = new IMapCompat( rOStm, STREAM_WRITE );

    // hier kann in neueren Versionen eingefuegt werden

    delete pCompat;

    ImpWriteImageMap( rOStm, rBaseURL );

    rOStm.SetNumberFormatInt( nOldFormat );
}


/******************************************************************************
|*
|* Binaer laden
|*
\******************************************************************************/

void ImageMap::Read( SvStream& rIStm, const String& rBaseURL )
{
    ByteString	aString;
    char		cMagic[6];
    USHORT		nOldFormat = rIStm.GetNumberFormatInt();
    UINT16		nCount;

    rIStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rIStm.Read( cMagic, sizeof( cMagic ) );

    if ( !memcmp( cMagic, IMAPMAGIC, sizeof( cMagic ) ) )
    {
        IMapCompat* pCompat;

        // alten Inhalt loeschen
        ClearImageMap();

        // Version ueberlesen wir
        rIStm.SeekRel( 2 );

        rIStm.ReadByteString( aString ); aName = String( aString, gsl_getSystemTextEncoding() );
        rIStm.ReadByteString( aString ); // Dummy
        rIStm >> nCount;
        rIStm.ReadByteString( aString ); // Dummy

        pCompat = new IMapCompat( rIStm, STREAM_READ );

        // hier kann in neueren Versionen gelesen werden

        delete pCompat;
        ImpReadImageMap( rIStm, nCount, rBaseURL );

    }
    else
        rIStm.SetError( SVSTREAM_GENERALERROR );

    rIStm.SetNumberFormatInt( nOldFormat );
}


#ifdef WIN
#pragma optimize ( "", on )
#endif


}
