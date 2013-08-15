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


#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/window.hxx>

#include "svl/urihelper.hxx"
#include <svtools/imap.hxx>
#include <svtools/imapobj.hxx>
#include <svtools/imapcirc.hxx>
#include <svtools/imaprect.hxx>
#include <svtools/imappoly.hxx>

#include <string.h>
#include <math.h>

DBG_NAME( ImageMap )


#define SCALEPOINT(aPT,aFracX,aFracY) (aPT).X()=((aPT).X()*(aFracX).GetNumerator())/(aFracX).GetDenominator();  \
                                      (aPT).Y()=((aPT).Y()*(aFracY).GetNumerator())/(aFracY).GetDenominator();


/******************************************************************************/

sal_uInt16 IMapObject::nActualTextEncoding = (sal_uInt16) RTL_TEXTENCODING_DONTKNOW;

/******************************************************************************/


IMapObject::IMapObject()
    : bActive( false )
    , nReadVersion( 0 )
{
}

IMapObject::IMapObject( const OUString& rURL, const OUString& rAltText, const OUString& rDesc,
                        const OUString& rTarget, const OUString& rName, sal_Bool bURLActive )
: aURL( rURL )
, aAltText( rAltText )
, aDesc( rDesc )
, aTarget( rTarget )
, aName( rName )
, bActive( bURLActive )
, nReadVersion( 0 )
{
}


/******************************************************************************
|*
|* Freigabe des internen Speichers
|*
\******************************************************************************/

sal_uInt16 IMapObject::GetVersion() const
{
    return IMAP_OBJ_VERSION;
}

void IMapObject::Write( SvStream& rOStm, const OUString& rBaseURL ) const
{
    IMapCompat*             pCompat;
    const rtl_TextEncoding  eEncoding = osl_getThreadTextEncoding();

    rOStm << GetType();
    rOStm << GetVersion();
    rOStm << ( (sal_uInt16) eEncoding );

    const OString aRelURL = OUStringToOString(
        URIHelper::simpleNormalizedMakeRelative(rBaseURL, aURL), eEncoding);
    write_lenPrefixed_uInt8s_FromOString<sal_uInt16>(rOStm, aRelURL);
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOStm, aAltText, eEncoding);
    rOStm << bActive;
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOStm, aTarget, eEncoding);

    pCompat = new IMapCompat( rOStm, STREAM_WRITE );

    WriteIMapObject( rOStm );
    aEventList.Write( rOStm );                                      // V4
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOStm, aName, eEncoding); // V5

    delete pCompat;
}


/******************************************************************************
|*
|*  Binaer-Import
|*
\******************************************************************************/

void IMapObject::Read( SvStream& rIStm, const OUString& rBaseURL )
{
    IMapCompat*         pCompat;
    rtl_TextEncoding    nTextEncoding;

    // Typ und Version ueberlesen wir
    rIStm.SeekRel( 2 );
    rIStm >> nReadVersion;
    rIStm >> nTextEncoding;
    aURL = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIStm, nTextEncoding);
    aAltText = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIStm, nTextEncoding);
    rIStm >> bActive;
    aTarget = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIStm, nTextEncoding);

    // URL absolut machen
    aURL = URIHelper::SmartRel2Abs( INetURLObject(rBaseURL), aURL, URIHelper::GetMaybeFileHdl(), true, false, INetURLObject::WAS_ENCODED, INetURLObject::DECODE_UNAMBIGUOUS );
    pCompat = new IMapCompat( rIStm, STREAM_READ );

    ReadIMapObject( rIStm );

    // ab Version 4 lesen wir eine EventListe
    if ( nReadVersion >= 0x0004 )
    {
        aEventList.Read(rIStm);

        // ab Version 5 kann ein Objektname vorhanden sein
        if ( nReadVersion >= 0x0005 )
            aName = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIStm, nTextEncoding);
    }

    delete pCompat;
}

sal_Bool IMapObject::IsEqual( const IMapObject& rEqObj )
{
    return ( ( aURL == rEqObj.aURL ) &&
             ( aAltText == rEqObj.aAltText ) &&
             ( aDesc == rEqObj.aDesc ) &&
             ( aTarget == rEqObj.aTarget ) &&
             ( aName == rEqObj.aName ) &&
             ( bActive == rEqObj.bActive ) );
}

IMapRectangleObject::IMapRectangleObject( const Rectangle& rRect,
                                          const OUString& rURL,
                                          const OUString& rAltText,
                                          const OUString& rDesc,
                                          const OUString& rTarget,
                                          const OUString& rName,
                                          sal_Bool bURLActive,
                                          sal_Bool bPixelCoords ) :
            IMapObject  ( rURL, rAltText, rDesc, rTarget, rName, bURLActive )
{
    ImpConstruct( rRect, bPixelCoords );
}

void IMapRectangleObject::ImpConstruct( const Rectangle& rRect, sal_Bool bPixel )
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

sal_uInt16 IMapRectangleObject::GetType() const
{
    return IMAP_OBJ_RECTANGLE;
}


/******************************************************************************
|*
|* Hit-Test
|*
\******************************************************************************/

sal_Bool IMapRectangleObject::IsHit( const Point& rPoint ) const
{
    return aRect.IsInside( rPoint );
}

Rectangle IMapRectangleObject::GetRectangle( sal_Bool bPixelCoords ) const
{
    Rectangle   aNewRect;

    if ( bPixelCoords )
        aNewRect = Application::GetDefaultDevice()->LogicToPixel( aRect, MapMode( MAP_100TH_MM ) );
    else
        aNewRect = aRect;

    return aNewRect;
}

void IMapRectangleObject::Scale( const Fraction& rFracX, const Fraction& rFracY )
{
    Point   aTL( aRect.TopLeft() );
    Point   aBR( aRect.BottomRight() );

    if ( rFracX.GetDenominator() && rFracY.GetDenominator() )
    {
        SCALEPOINT( aTL, rFracX, rFracY );
        SCALEPOINT( aBR, rFracX, rFracY );
    }

    aRect = Rectangle( aTL, aBR );
}

sal_Bool IMapRectangleObject::IsEqual( const IMapRectangleObject& rEqObj )
{
    return ( IMapObject::IsEqual( rEqObj ) && ( aRect == rEqObj.aRect ) );
}

IMapCircleObject::IMapCircleObject( const Point& rCenter, sal_uLong nCircleRadius,
                                    const OUString& rURL,
                                    const OUString& rAltText,
                                    const OUString& rDesc,
                                    const OUString& rTarget,
                                    const OUString& rName,
                                    sal_Bool bURLActive,
                                    sal_Bool bPixelCoords ) :
            IMapObject  ( rURL, rAltText, rDesc, rTarget, rName, bURLActive )
{
    ImpConstruct( rCenter, nCircleRadius, bPixelCoords );
}

void IMapCircleObject::ImpConstruct( const Point& rCenter, sal_uLong nRad, sal_Bool bPixel )
{
    if ( bPixel )
    {
        MapMode aMap100( MAP_100TH_MM );

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
    sal_uInt32 nTmp = nRadius;

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
    sal_uInt32 nTmp;

    rIStm >> aCenter;
    rIStm >> nTmp;

    nRadius = nTmp;
}


/******************************************************************************
|*
|* Typ-Rueckgabe
|*
\******************************************************************************/

sal_uInt16 IMapCircleObject::GetType() const
{
    return IMAP_OBJ_CIRCLE;
}


/******************************************************************************
|*
|* Hit-Test
|*
\******************************************************************************/

sal_Bool IMapCircleObject::IsHit( const Point& rPoint ) const
{
    const Point aPoint( aCenter - rPoint );
    sal_Bool        bRet = sal_False;

    if ( (sal_Int32) sqrt( (double) aPoint.X() * aPoint.X() +
                       aPoint.Y() * aPoint.Y() ) <= nRadius )
    {
        bRet = sal_True;
    }

    return bRet;
}

Point IMapCircleObject::GetCenter( sal_Bool bPixelCoords ) const
{
    Point aNewPoint;

    if ( bPixelCoords )
        aNewPoint = Application::GetDefaultDevice()->LogicToPixel( aCenter, MapMode( MAP_100TH_MM ) );
    else
        aNewPoint = aCenter;

    return aNewPoint;
}

sal_uLong IMapCircleObject::GetRadius( sal_Bool bPixelCoords ) const
{
    sal_uLong nNewRadius;

    if ( bPixelCoords )
        nNewRadius = Application::GetDefaultDevice()->LogicToPixel( Size( nRadius, 0 ), MapMode( MAP_100TH_MM ) ).Width();
    else
        nNewRadius = nRadius;

    return nNewRadius;
}

Rectangle IMapCircleObject::GetBoundRect() const
{
    long nWidth = nRadius << 1;

    return Rectangle( Point(  aCenter.X() - nRadius, aCenter.Y() - nRadius ),
                      Size( nWidth, nWidth ) );
}

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

sal_Bool IMapCircleObject::IsEqual( const IMapCircleObject& rEqObj )
{
    return ( IMapObject::IsEqual( rEqObj ) &&
             ( aCenter == rEqObj.aCenter ) &&
             ( nRadius == rEqObj.nRadius ) );
}

IMapPolygonObject::IMapPolygonObject( const Polygon& rPoly,
                                      const OUString& rURL,
                                      const OUString& rAltText,
                                      const OUString& rDesc,
                                      const OUString& rTarget,
                                      const OUString& rName,
                                      sal_Bool bURLActive,
                                      sal_Bool bPixelCoords ) :
            IMapObject  ( rURL, rAltText, rDesc, rTarget, rName, bURLActive ),
            bEllipse    ( sal_False )
{
    ImpConstruct( rPoly, bPixelCoords );
}

void IMapPolygonObject::ImpConstruct( const Polygon& rPoly, sal_Bool bPixel )
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
    rOStm << bEllipse;  // >= Version 2
    rOStm << aEllipse;  // >= Version 2
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

sal_uInt16 IMapPolygonObject::GetType() const
{
    return IMAP_OBJ_POLYGON;
}


/******************************************************************************
|*
|* Hit-Test
|*
\******************************************************************************/

sal_Bool IMapPolygonObject::IsHit( const Point& rPoint ) const
{
    return aPoly.IsInside( rPoint );
}

Polygon IMapPolygonObject::GetPolygon( sal_Bool bPixelCoords ) const
{
    Polygon aNewPoly;

    if ( bPixelCoords )
        aNewPoly = Application::GetDefaultDevice()->LogicToPixel( aPoly, MapMode( MAP_100TH_MM ) );
    else
        aNewPoly = aPoly;

    return aNewPoly;
}

void IMapPolygonObject::SetExtraEllipse( const Rectangle& rEllipse )
{
    if ( aPoly.GetSize() )
    {
        bEllipse = sal_True;
        aEllipse = rEllipse;
    }
}

void IMapPolygonObject::Scale( const Fraction& rFracX, const Fraction& rFracY )
{
    sal_uInt16 nCount = aPoly.GetSize();

    for ( sal_uInt16 i = 0; i < nCount; i++ )
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
        Point   aTL( aEllipse.TopLeft() );
        Point   aBR( aEllipse.BottomRight() );

        if ( rFracX.GetDenominator() && rFracY.GetDenominator() )
        {
            SCALEPOINT( aTL, rFracX, rFracY );
            SCALEPOINT( aBR, rFracX, rFracY );
        }

        aEllipse = Rectangle( aTL, aBR );
    }
}

sal_Bool IMapPolygonObject::IsEqual( const IMapPolygonObject& rEqObj )
{
    sal_Bool bRet = sal_False;

    if ( IMapObject::IsEqual( rEqObj ) )
    {
        const Polygon&  rEqPoly = rEqObj.aPoly;
        const sal_uInt16    nCount = aPoly.GetSize();
        const sal_uInt16    nEqCount = rEqPoly.GetSize();
        sal_Bool            bDifferent = sal_False;

        if ( nCount == nEqCount )
        {
            for ( sal_uInt16 i = 0; i < nCount; i++ )
            {
                if ( aPoly[ i ] != rEqPoly[ i ] )
                {
                    bDifferent = sal_True;
                    break;
                }
            }

            if ( !bDifferent )
                bRet = sal_True;
        }
    }

    return bRet;
}

/******************************************************************************
|*
|* Ctor
|*
\******************************************************************************/

ImageMap::ImageMap( const OUString& rName )
:   aName( rName )
{
}


/******************************************************************************
|*
|* Copy-Ctor
|*
\******************************************************************************/

ImageMap::ImageMap( const ImageMap& rImageMap )
{
    DBG_CTOR( ImageMap, NULL );

    size_t nCount = rImageMap.GetIMapObjectCount();

    for ( size_t i = 0; i < nCount; i++ )
    {
        IMapObject* pCopyObj = rImageMap.GetIMapObject( i );

        switch( pCopyObj->GetType() )
        {
            case( IMAP_OBJ_RECTANGLE ):
                maList.push_back( new IMapRectangleObject( *(IMapRectangleObject*) pCopyObj ) );
            break;

            case( IMAP_OBJ_CIRCLE ):
                maList.push_back( new IMapCircleObject( *(IMapCircleObject*) pCopyObj ) );
            break;

            case( IMAP_OBJ_POLYGON ):
                maList.push_back( new IMapPolygonObject( *(IMapPolygonObject*) pCopyObj ) );
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
    for( size_t i = 0, n = maList.size(); i < n; ++i )
        delete maList[ i ];
    maList.clear();

    aName = String();
}


/******************************************************************************
|*
|* Zuweisungsoperator
|*
\******************************************************************************/

ImageMap& ImageMap::operator=( const ImageMap& rImageMap )
{
    size_t nCount = rImageMap.GetIMapObjectCount();

    ClearImageMap();

    for ( size_t i = 0; i < nCount; i++ )
    {
        IMapObject* pCopyObj = rImageMap.GetIMapObject( i );

        switch( pCopyObj->GetType() )
        {
            case( IMAP_OBJ_RECTANGLE ):
                maList.push_back( new IMapRectangleObject( *(IMapRectangleObject*) pCopyObj ) );
            break;

            case( IMAP_OBJ_CIRCLE ):
                maList.push_back( new IMapCircleObject( *(IMapCircleObject*) pCopyObj ) );
            break;

            case( IMAP_OBJ_POLYGON ):
                maList.push_back( new IMapPolygonObject( *(IMapPolygonObject*) pCopyObj ) );
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

sal_Bool ImageMap::operator==( const ImageMap& rImageMap )
{
    const size_t    nCount = maList.size();
    const size_t    nEqCount = rImageMap.GetIMapObjectCount();
    sal_Bool        bRet = sal_False;

    if ( nCount == nEqCount )
    {
        sal_Bool bDifferent = ( aName != rImageMap.aName );

        for ( size_t i = 0; ( i < nCount ) && !bDifferent; i++ )
        {
            IMapObject* pObj = maList[ i ];
            IMapObject* pEqObj = rImageMap.GetIMapObject( i );

            if ( pObj->GetType() == pEqObj->GetType() )
            {
                switch( pObj->GetType() )
                {
                    case( IMAP_OBJ_RECTANGLE ):
                    {
                        if ( !( (IMapRectangleObject*) pObj )->IsEqual( *(IMapRectangleObject*) pEqObj ) )
                            bDifferent = sal_True;
                    }
                    break;

                    case( IMAP_OBJ_CIRCLE ):
                    {
                        if ( !( (IMapCircleObject*) pObj )->IsEqual( *(IMapCircleObject*) pEqObj ) )
                            bDifferent = sal_True;
                    }
                    break;

                    case( IMAP_OBJ_POLYGON ):
                    {
                        if ( !( (IMapPolygonObject*) pObj )->IsEqual( *(IMapPolygonObject*) pEqObj ) )
                            bDifferent = sal_True;
                    }
                    break;

                    default:
                    break;
                }
            }
            else
                bDifferent = sal_True;
        }

        if ( !bDifferent )
            bRet = sal_True;
    }

    return bRet;
}


/******************************************************************************
|*
|* Vergleichsoperator II
|*
\******************************************************************************/

sal_Bool ImageMap::operator!=( const ImageMap& rImageMap )
{
    return !( *this == rImageMap );
}


/******************************************************************************
|*
|* Freigabe des internen Speichers
|*
\******************************************************************************/

sal_uInt16 ImageMap::GetVersion() const
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
            maList.push_back( new IMapRectangleObject( (IMapRectangleObject&) rIMapObject ) );
        break;

        case( IMAP_OBJ_CIRCLE ):
            maList.push_back( new IMapCircleObject( (IMapCircleObject&) rIMapObject ) );
        break;

        case( IMAP_OBJ_POLYGON ):
            maList.push_back( new IMapPolygonObject( (IMapPolygonObject&) rIMapObject ) );
        break;

        default:
        break;
    }
}


/******************************************************************************
|*
|* Hit-Test
|*
\******************************************************************************/

IMapObject* ImageMap::GetHitIMapObject( const Size& rTotalSize,
                                        const Size& rDisplaySize,
                                        const Point& rRelHitPoint,
                                        sal_uLong nFlags )
{
    Point aRelPoint( rTotalSize.Width() * rRelHitPoint.X() / rDisplaySize.Width(),
                     rTotalSize.Height() * rRelHitPoint.Y() / rDisplaySize.Height() );

    // Falls Flags zur Spiegelung etc. angegeben sind, wird
    // der zu pruefende Punkt vor der Pruefung entspr. transformiert
    if ( nFlags )
    {
        if ( nFlags & IMAP_MIRROR_HORZ )
            aRelPoint.X() = rTotalSize.Width() - aRelPoint.X();

        if ( nFlags & IMAP_MIRROR_VERT )
            aRelPoint.Y() = rTotalSize.Height() - aRelPoint.Y();
    }

    // Alle Objekte durchlaufen und HitTest ausfuehren
    IMapObject* pObj = NULL;
    for( size_t i = 0, n = maList.size(); i < n; ++i ) {
        if ( maList[ i ]->IsHit( aRelPoint ) ) {
            pObj = maList[ i ];
            break;
        }
    }

    return( pObj ? ( pObj->IsActive() ? pObj : NULL ) : NULL );
}

void ImageMap::Scale( const Fraction& rFracX, const Fraction& rFracY )
{
    size_t nCount = maList.size();

    for ( size_t i = 0; i < nCount; i++ )
    {
        IMapObject* pObj = maList[ i ];

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

void ImageMap::ImpWriteImageMap( SvStream& rOStm, const OUString& rBaseURL ) const
{
    IMapObject* pObj;
    size_t      nCount = maList.size();

    for ( size_t i = 0; i < nCount; i++ )
    {
        pObj = maList[ i ];
        pObj->Write( rOStm, rBaseURL );
    }
}


/******************************************************************************
|*
|* Objekte nacheinander lesen
|*
\******************************************************************************/

void ImageMap::ImpReadImageMap( SvStream& rIStm, size_t nCount, const OUString& rBaseURL )
{
    // neue Objekte einlesen
    for ( size_t i = 0; i < nCount; i++ )
    {
        sal_uInt16 nType;

        rIStm >> nType;
        rIStm.SeekRel( -2 );

        switch( nType )
        {
            case ( IMAP_OBJ_RECTANGLE ):
            {
                IMapRectangleObject* pObj = new IMapRectangleObject;
                pObj->Read( rIStm, rBaseURL );
                maList.push_back( pObj );
            }
            break;

            case ( IMAP_OBJ_CIRCLE ):
            {
                IMapCircleObject* pObj = new IMapCircleObject;
                pObj->Read( rIStm, rBaseURL );
                maList.push_back( pObj );
            }
            break;

            case ( IMAP_OBJ_POLYGON ):
            {
                IMapPolygonObject* pObj = new IMapPolygonObject;
                pObj->Read( rIStm, rBaseURL );
                maList.push_back( pObj );
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

void ImageMap::Write( SvStream& rOStm, const OUString& rBaseURL ) const
{
    IMapCompat*             pCompat;
    String                  aImageName( GetName() );
    sal_uInt16                  nOldFormat = rOStm.GetNumberFormatInt();
    sal_uInt16                  nCount = (sal_uInt16) GetIMapObjectCount();
    const rtl_TextEncoding  eEncoding = osl_getThreadTextEncoding(); //vomit!

    rOStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    // MagicCode schreiben
    rOStm << IMAPMAGIC;
    rOStm << GetVersion();
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOStm, aImageName, eEncoding);
    write_lenPrefixed_uInt8s_FromOString<sal_uInt16>(rOStm, OString()); //dummy
    rOStm << nCount;
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOStm, aImageName, eEncoding);

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

void ImageMap::Read( SvStream& rIStm, const OUString& rBaseURL )
{
    char        cMagic[6];
    sal_uInt16      nOldFormat = rIStm.GetNumberFormatInt();
    sal_uInt16      nCount;

    rIStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rIStm.Read( cMagic, sizeof( cMagic ) );

    if ( !memcmp( cMagic, IMAPMAGIC, sizeof( cMagic ) ) )
    {
        IMapCompat* pCompat;

        // alten Inhalt loeschen
        ClearImageMap();

        // Version ueberlesen wir
        rIStm.SeekRel( 2 );

        aName = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIStm, osl_getThreadTextEncoding());
        read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rIStm); // Dummy
        rIStm >> nCount;
        read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rIStm); // Dummy

        pCompat = new IMapCompat( rIStm, STREAM_READ );

        // hier kann in neueren Versionen gelesen werden

        delete pCompat;
        ImpReadImageMap( rIStm, nCount, rBaseURL );

    }
    else
        rIStm.SetError( SVSTREAM_GENERALERROR );

    rIStm.SetNumberFormatInt( nOldFormat );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
