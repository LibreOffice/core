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
#include <tools/fract.hxx>
#include <tools/GenericTypeSerializer.hxx>
#include <utility>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mapmod.hxx>
#include <o3tl/numeric.hxx>
#include <svl/urihelper.hxx>
#include <vcl/imap.hxx>
#include <vcl/imapobj.hxx>
#include <vcl/imapcirc.hxx>
#include <vcl/imaprect.hxx>
#include <vcl/imappoly.hxx>

#include <string.h>
#include <math.h>
#include <memory>
#include <sal/log.hxx>


#define SCALEPOINT(aPT,aFracX,aFracY) (aPT).setX(tools::Long((aPT).X()*aFracX));  \
                                      (aPT).setY(tools::Long((aPT).Y()*aFracY));


/******************************************************************************/


IMapObject::IMapObject()
    : bActive( false )
    , nReadVersion( 0 )
{
}

IMapObject::IMapObject( OUString _aURL, OUString _aAltText, OUString _aDesc,
                        OUString _aTarget, OUString _aName, bool bURLActive )
: aURL(std::move( _aURL ))
, aAltText(std::move( _aAltText ))
, aDesc(std::move( _aDesc ))
, aTarget(std::move( _aTarget ))
, aName(std::move( _aName ))
, bActive( bURLActive )
, nReadVersion( 0 )
{
}


void IMapObject::Write( SvStream& rOStm ) const
{
    const rtl_TextEncoding  eEncoding = osl_getThreadTextEncoding();

    rOStm.WriteUInt16( static_cast<sal_uInt16>(GetType()) );
    rOStm.WriteUInt16( IMAP_OBJ_VERSION );
    rOStm.WriteUInt16( eEncoding  );

    const OString aRelURL = OUStringToOString(
        URIHelper::simpleNormalizedMakeRelative(u""_ustr, aURL), eEncoding);
    write_uInt16_lenPrefixed_uInt8s_FromOString(rOStm, aRelURL);
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOStm, aAltText, eEncoding);
    rOStm.WriteBool( bActive );
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOStm, aTarget, eEncoding);

    IMapCompat aCompat( rOStm, StreamMode::WRITE );

    WriteIMapObject( rOStm );
    aEventList.Write( rOStm );                                      // V4
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOStm, aName, eEncoding); // V5
}


/******************************************************************************
|*
|*  Binary import
|*
\******************************************************************************/

void IMapObject::Read( SvStream& rIStm )
{
    rtl_TextEncoding    nTextEncoding;

    // read on type and version
    rIStm.SeekRel( 2 );
    rIStm.ReadUInt16( nReadVersion );
    rIStm.ReadUInt16( nTextEncoding );
    aURL = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIStm, nTextEncoding);
    aAltText = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIStm, nTextEncoding);
    rIStm.ReadCharAsBool( bActive );
    aTarget = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIStm, nTextEncoding);

    // make URL absolute
    aURL = URIHelper::SmartRel2Abs( INetURLObject(u""), aURL, URIHelper::GetMaybeFileHdl(), true, false, INetURLObject::EncodeMechanism::WasEncoded, INetURLObject::DecodeMechanism::Unambiguous );
    IMapCompat aCompat( rIStm, StreamMode::READ );

    ReadIMapObject( rIStm );

    // from version 4 onwards we read an eventlist
    if ( nReadVersion >= 0x0004 )
    {
        aEventList.Read(rIStm);

        // from version 5 onwards an objectname could be available
        if ( nReadVersion >= 0x0005 )
            aName = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIStm, nTextEncoding);
    }
}

bool IMapObject::IsEqual( const IMapObject& rEqObj ) const
{
    return ( ( aURL == rEqObj.aURL ) &&
             ( aAltText == rEqObj.aAltText ) &&
             ( aDesc == rEqObj.aDesc ) &&
             ( aTarget == rEqObj.aTarget ) &&
             ( aName == rEqObj.aName ) &&
             ( bActive == rEqObj.bActive ) );
}

IMapRectangleObject::IMapRectangleObject( const tools::Rectangle& rRect,
                                          const OUString& rURL,
                                          const OUString& rAltText,
                                          const OUString& rDesc,
                                          const OUString& rTarget,
                                          const OUString& rName,
                                          bool bURLActive,
                                          bool bPixelCoords ) :
            IMapObject  ( rURL, rAltText, rDesc, rTarget, rName, bURLActive )
{
    ImpConstruct( rRect, bPixelCoords );
}

void IMapRectangleObject::ImpConstruct( const tools::Rectangle& rRect, bool bPixel )
{
    if ( bPixel )
        aRect = Application::GetDefaultDevice()->PixelToLogic( rRect, MapMode( MapUnit::Map100thMM ) );
    else
        aRect = rRect;
}


/******************************************************************************
|*
|* Binary export
|*
\******************************************************************************/

void IMapRectangleObject::WriteIMapObject( SvStream& rOStm ) const
{
    tools::GenericTypeSerializer aSerializer(rOStm);
    aSerializer.writeRectangle(aRect);
}


/******************************************************************************
|*
|* Binary import
|*
\******************************************************************************/

void IMapRectangleObject::ReadIMapObject( SvStream& rIStm )
{
    tools::GenericTypeSerializer aSerializer(rIStm);
    aSerializer.readRectangle(aRect);
}


/******************************************************************************
|*
|* return type
|*
\******************************************************************************/

IMapObjectType IMapRectangleObject::GetType() const
{
    return IMapObjectType::Rectangle;
}


/******************************************************************************
|*
|* Hit test
|*
\******************************************************************************/

bool IMapRectangleObject::IsHit( const Point& rPoint ) const
{
    return aRect.Contains( rPoint );
}

tools::Rectangle IMapRectangleObject::GetRectangle( bool bPixelCoords ) const
{
    tools::Rectangle   aNewRect;

    if ( bPixelCoords )
        aNewRect = Application::GetDefaultDevice()->LogicToPixel( aRect, MapMode( MapUnit::Map100thMM ) );
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

    aRect = tools::Rectangle( aTL, aBR );
}

bool IMapRectangleObject::IsEqual( const IMapRectangleObject& rEqObj ) const
{
    return ( IMapObject::IsEqual( rEqObj ) && ( aRect == rEqObj.aRect ) );
}

IMapCircleObject::IMapCircleObject( const Point& rCenter, sal_Int32 nCircleRadius,
                                    const OUString& rURL,
                                    const OUString& rAltText,
                                    const OUString& rDesc,
                                    const OUString& rTarget,
                                    const OUString& rName,
                                    bool bURLActive,
                                    bool bPixelCoords ) :
            IMapObject  ( rURL, rAltText, rDesc, rTarget, rName, bURLActive )
{
    ImpConstruct( rCenter, nCircleRadius, bPixelCoords );
}

void IMapCircleObject::ImpConstruct( const Point& rCenter, sal_Int32 nRad, bool bPixel )
{
    if ( bPixel )
    {
        MapMode aMap100( MapUnit::Map100thMM );

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
|* Binary export
|*
\******************************************************************************/

void IMapCircleObject::WriteIMapObject( SvStream& rOStm ) const
{
    sal_uInt32 nTmp = nRadius;
    tools::GenericTypeSerializer aSerializer(rOStm);
    aSerializer.writePoint(aCenter);
    rOStm.WriteUInt32( nTmp );
}


/******************************************************************************
|*
|* Binary import
|*
\******************************************************************************/

void IMapCircleObject::ReadIMapObject( SvStream& rIStm )
{
    sal_uInt32 nTmp;

    tools::GenericTypeSerializer aSerializer(rIStm);
    aSerializer.readPoint(aCenter);
    rIStm.ReadUInt32( nTmp );

    nRadius = nTmp;
}


/******************************************************************************
|*
|* return type
|*
\******************************************************************************/

IMapObjectType IMapCircleObject::GetType() const
{
    return IMapObjectType::Circle;
}


/******************************************************************************
|*
|* Hit-Test
|*
\******************************************************************************/

bool IMapCircleObject::IsHit( const Point& rPoint ) const
{
    const Point aPoint( aCenter - rPoint );

    return static_cast<sal_Int32>( std::hypot( aPoint.X(), aPoint.Y() ) ) <= nRadius;
}

Point IMapCircleObject::GetCenter( bool bPixelCoords ) const
{
    Point aNewPoint;

    if ( bPixelCoords )
        aNewPoint = Application::GetDefaultDevice()->LogicToPixel( aCenter, MapMode( MapUnit::Map100thMM ) );
    else
        aNewPoint = aCenter;

    return aNewPoint;
}

sal_Int32 IMapCircleObject::GetRadius( bool bPixelCoords ) const
{
    sal_Int32 nNewRadius;

    if ( bPixelCoords )
        nNewRadius = Application::GetDefaultDevice()->LogicToPixel( Size( nRadius, 0 ), MapMode( MapUnit::Map100thMM ) ).Width();
    else
        nNewRadius = nRadius;

    return nNewRadius;
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

    if (!aAverage.GetDenominator())
        throw o3tl::divide_by_zero();

    nRadius = double(nRadius * aAverage);
}

bool IMapCircleObject::IsEqual( const IMapCircleObject& rEqObj ) const
{
    return ( IMapObject::IsEqual( rEqObj ) &&
             ( aCenter == rEqObj.aCenter ) &&
             ( nRadius == rEqObj.nRadius ) );
}

IMapPolygonObject::IMapPolygonObject( const tools::Polygon& rPoly,
                                      const OUString& rURL,
                                      const OUString& rAltText,
                                      const OUString& rDesc,
                                      const OUString& rTarget,
                                      const OUString& rName,
                                      bool bURLActive,
                                      bool bPixelCoords ) :
            IMapObject  ( rURL, rAltText, rDesc, rTarget, rName, bURLActive ),
            bEllipse    ( false )
{
    ImpConstruct( rPoly, bPixelCoords );
}

void IMapPolygonObject::ImpConstruct( const tools::Polygon& rPoly, bool bPixel )
{
    if ( bPixel )
        aPoly = Application::GetDefaultDevice()->PixelToLogic( rPoly, MapMode( MapUnit::Map100thMM ) );
    else
        aPoly = rPoly;
}


/******************************************************************************
|*
|* Binary export
|*
\******************************************************************************/

void IMapPolygonObject::WriteIMapObject( SvStream& rOStm ) const
{
    tools::GenericTypeSerializer aSerializer(rOStm);
    WritePolygon( rOStm, aPoly );
    // Version 2
    rOStm.WriteBool( bEllipse );
    aSerializer.writeRectangle(aEllipse);
}


/******************************************************************************
|*
|* Binary import
|*
\******************************************************************************/

void IMapPolygonObject::ReadIMapObject( SvStream& rIStm )
{
    ReadPolygon( rIStm, aPoly );

    // Version >= 2 has additional ellipses information
    if ( nReadVersion >= 2 )
    {
        rIStm.ReadCharAsBool( bEllipse );
        tools::GenericTypeSerializer aSerializer(rIStm);
        aSerializer.readRectangle(aEllipse);
    }
}


/******************************************************************************
|*
|* return type
|*
\******************************************************************************/

IMapObjectType IMapPolygonObject::GetType() const
{
    return IMapObjectType::Polygon;
}


/******************************************************************************
|*
|* hit test
|*
\******************************************************************************/

bool IMapPolygonObject::IsHit( const Point& rPoint ) const
{
    return aPoly.Contains( rPoint );
}

tools::Polygon IMapPolygonObject::GetPolygon( bool bPixelCoords ) const
{
    tools::Polygon aNewPoly;

    if ( bPixelCoords )
        aNewPoly = Application::GetDefaultDevice()->LogicToPixel( aPoly, MapMode( MapUnit::Map100thMM ) );
    else
        aNewPoly = aPoly;

    return aNewPoly;
}

void IMapPolygonObject::SetExtraEllipse( const tools::Rectangle& rEllipse )
{
    if ( aPoly.GetSize() )
    {
        bEllipse = true;
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

    if ( !bEllipse )
        return;

    Point   aTL( aEllipse.TopLeft() );
    Point   aBR( aEllipse.BottomRight() );

    if ( rFracX.GetDenominator() && rFracY.GetDenominator() )
    {
        SCALEPOINT( aTL, rFracX, rFracY );
        SCALEPOINT( aBR, rFracX, rFracY );
    }

    aEllipse = tools::Rectangle( aTL, aBR );
}

bool IMapPolygonObject::IsEqual( const IMapPolygonObject& rEqObj )
{
    bool bRet = false;

    if ( IMapObject::IsEqual( rEqObj ) )
    {
        const tools::Polygon&   rEqPoly = rEqObj.aPoly;
        const sal_uInt16 nCount = aPoly.GetSize();
        const sal_uInt16 nEqCount = rEqPoly.GetSize();

        if ( nCount == nEqCount )
        {
            bool bDifferent = false;

            for ( sal_uInt16 i = 0; i < nCount; i++ )
            {
                if ( aPoly[ i ] != rEqPoly[ i ] )
                {
                    bDifferent = true;
                    break;
                }
            }

            if ( !bDifferent )
                bRet = true;
        }
    }

    return bRet;
}

/******************************************************************************
|*
|* Ctor
|*
\******************************************************************************/

ImageMap::ImageMap( OUString _aName )
:   aName(std::move( _aName ))
{
}


/******************************************************************************
|*
|* Copy-Ctor
|*
\******************************************************************************/

ImageMap::ImageMap( const ImageMap& rImageMap )
{

    size_t nCount = rImageMap.GetIMapObjectCount();

    for ( size_t i = 0; i < nCount; i++ )
    {
        IMapObject* pCopyObj = rImageMap.GetIMapObject( i );

        switch( pCopyObj->GetType() )
        {
            case IMapObjectType::Rectangle:
                maList.emplace_back( new IMapRectangleObject( *static_cast<IMapRectangleObject*>( pCopyObj ) ) );
            break;

            case IMapObjectType::Circle:
                maList.emplace_back( new IMapCircleObject( *static_cast<IMapCircleObject*>( pCopyObj ) ) );
            break;

            case IMapObjectType::Polygon:
                maList.emplace_back( new IMapPolygonObject( *static_cast<IMapPolygonObject*>( pCopyObj ) ) );
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
}


/******************************************************************************
|*
|* release internal memory
|*
\******************************************************************************/

void ImageMap::ClearImageMap()
{
    maList.clear();

    aName.clear();
}


/******************************************************************************
|*
|* assignment operator
|*
\******************************************************************************/

ImageMap& ImageMap::operator=( const ImageMap& rImageMap )
{
    if (this != &rImageMap)
    {
        size_t nCount = rImageMap.GetIMapObjectCount();

        ClearImageMap();

        for ( size_t i = 0; i < nCount; i++ )
        {
            IMapObject* pCopyObj = rImageMap.GetIMapObject( i );

            switch( pCopyObj->GetType() )
            {
                case IMapObjectType::Rectangle:
                    maList.emplace_back( new IMapRectangleObject( *static_cast<IMapRectangleObject*>(pCopyObj) ) );
                break;

                case IMapObjectType::Circle:
                    maList.emplace_back( new IMapCircleObject( *static_cast<IMapCircleObject*>(pCopyObj) ) );
                break;

                case IMapObjectType::Polygon:
                    maList.emplace_back( new IMapPolygonObject( *static_cast<IMapPolygonObject*>(pCopyObj) ) );
                break;

                default:
                break;
            }
        }

        aName = rImageMap.aName;
    }
    return *this;
}


/******************************************************************************
|*
|* compare operator I
|*
\******************************************************************************/

bool ImageMap::operator==( const ImageMap& rImageMap )
{
    const size_t    nCount = maList.size();
    const size_t    nEqCount = rImageMap.GetIMapObjectCount();
    bool            bRet = false;

    if ( nCount == nEqCount )
    {
        bool bDifferent = ( aName != rImageMap.aName );

        for ( size_t i = 0; ( i < nCount ) && !bDifferent; i++ )
        {
            IMapObject* pObj = maList[ i ].get();
            IMapObject* pEqObj = rImageMap.GetIMapObject( i );

            if ( pObj->GetType() == pEqObj->GetType() )
            {
                switch( pObj->GetType() )
                {
                    case IMapObjectType::Rectangle:
                    {
                        if ( ! static_cast<IMapRectangleObject*>(pObj)->IsEqual( *static_cast<IMapRectangleObject*>(pEqObj) ) )
                            bDifferent = true;
                    }
                    break;

                    case IMapObjectType::Circle:
                    {
                        if ( ! static_cast<IMapCircleObject*>(pObj)->IsEqual( *static_cast<IMapCircleObject*>(pEqObj) ) )
                            bDifferent = true;
                    }
                    break;

                    case IMapObjectType::Polygon:
                    {
                        if ( ! static_cast<IMapPolygonObject*>(pObj)->IsEqual( *static_cast<IMapPolygonObject*>(pEqObj) ) )
                            bDifferent = true;
                    }
                    break;

                    default:
                    break;
                }
            }
            else
                bDifferent = true;
        }

        if ( !bDifferent )
            bRet = true;
    }

    return bRet;
}


/******************************************************************************
|*
|* compare operator II
|*
\******************************************************************************/

bool ImageMap::operator!=( const ImageMap& rImageMap )
{
    return !( *this == rImageMap );
}


/******************************************************************************
|*
|* insert new object
|*
\******************************************************************************/

void ImageMap::InsertIMapObject( const IMapObject& rIMapObject )
{
    switch( rIMapObject.GetType() )
    {
        case IMapObjectType::Rectangle:
            maList.emplace_back( new IMapRectangleObject( static_cast<const IMapRectangleObject&>( rIMapObject ) ) );
        break;

        case IMapObjectType::Circle:
            maList.emplace_back( new IMapCircleObject( static_cast<const IMapCircleObject&>( rIMapObject ) ) );
        break;

        case IMapObjectType::Polygon:
            maList.emplace_back( new IMapPolygonObject( static_cast<const IMapPolygonObject&>( rIMapObject ) ) );
        break;

        default:
        break;
    }
}

void ImageMap::InsertIMapObject( std::unique_ptr<IMapObject> pNewObject )
{
    maList.emplace_back( std::move(pNewObject) );
}

/******************************************************************************
|*
|* hit test
|*
\******************************************************************************/

IMapObject* ImageMap::GetHitIMapObject( const Size& rTotalSize,
                                        const Size& rDisplaySize,
                                        const Point& rRelHitPoint,
                                        sal_uLong nFlags ) const
{
    Point aRelPoint( rTotalSize.Width() * rRelHitPoint.X() / rDisplaySize.Width(),
                     rTotalSize.Height() * rRelHitPoint.Y() / rDisplaySize.Height() );

    // transform point to check before checking if flags to mirror etc. are set,
    if ( nFlags )
    {
        if ( nFlags & IMAP_MIRROR_HORZ )
            aRelPoint.setX( rTotalSize.Width() - aRelPoint.X() );

        if ( nFlags & IMAP_MIRROR_VERT )
            aRelPoint.setY( rTotalSize.Height() - aRelPoint.Y() );
    }

    // walk over all objects and execute HitTest
    IMapObject* pObj = nullptr;
    for(const auto& i : maList) {
        if ( i->IsHit( aRelPoint ) ) {
            pObj = i.get();
            break;
        }
    }

    return( pObj ? ( pObj->IsActive() ? pObj : nullptr ) : nullptr );
}

void ImageMap::Scale( const Fraction& rFracX, const Fraction& rFracY )
{
    size_t nCount = maList.size();

    for ( size_t i = 0; i < nCount; i++ )
    {
        IMapObject* pObj = maList[ i ].get();

        switch( pObj->GetType() )
        {
            case IMapObjectType::Rectangle:
                static_cast<IMapRectangleObject*>( pObj )->Scale( rFracX, rFracY );
            break;

            case IMapObjectType::Circle:
                static_cast<IMapCircleObject*>( pObj )->Scale( rFracX, rFracY );
            break;

            case IMapObjectType::Polygon:
                static_cast<IMapPolygonObject*>( pObj )->Scale( rFracX, rFracY );
            break;

            default:
            break;
        }
    }
}


/******************************************************************************
|*
|* sequentially write objects
|*
\******************************************************************************/

void ImageMap::ImpWriteImageMap( SvStream& rOStm ) const
{
    size_t      nCount = maList.size();

    for ( size_t i = 0; i < nCount; i++ )
    {
        auto& pObj = maList[ i ];
        pObj->Write( rOStm );
    }
}


/******************************************************************************
|*
|* sequentially read objects
|*
\******************************************************************************/

void ImageMap::ImpReadImageMap( SvStream& rIStm, size_t nCount )
{
    const size_t nMinRecordSize = 12; //circle, three 32bit numbers
    const size_t nMaxRecords = rIStm.remainingSize() / nMinRecordSize;

    if (nCount > nMaxRecords)
    {
        SAL_WARN("svtools.misc", "Parsing error: " << nMaxRecords << " max possible entries, but " <<
                 nCount << " claimed, truncating");
        nCount = nMaxRecords;
    }

    // read new objects
    for (size_t i = 0; i < nCount; ++i)
    {
        sal_uInt16 nType;

        rIStm.ReadUInt16( nType );
        rIStm.SeekRel( -2 );

        switch( static_cast<IMapObjectType>(nType) )
        {
            case IMapObjectType::Rectangle:
            {
                IMapRectangleObject* pObj = new IMapRectangleObject;
                pObj->Read( rIStm );
                maList.emplace_back( pObj );
            }
            break;

            case IMapObjectType::Circle:
            {
                IMapCircleObject* pObj = new IMapCircleObject;
                pObj->Read( rIStm );
                maList.emplace_back( pObj );
            }
            break;

            case IMapObjectType::Polygon:
            {
                IMapPolygonObject* pObj = new IMapPolygonObject;
                pObj->Read( rIStm );
                maList.emplace_back( pObj );
            }
            break;

            default:
            break;
        }
    }
}


/******************************************************************************
|*
|* store binary
|*
\******************************************************************************/

void ImageMap::Write( SvStream& rOStm ) const
{
    IMapCompat*             pCompat;
    OUString                aImageName( GetName() );
    SvStreamEndian          nOldFormat = rOStm.GetEndian();
    sal_uInt16              nCount = static_cast<sal_uInt16>(GetIMapObjectCount());
    const rtl_TextEncoding  eEncoding = osl_getThreadTextEncoding(); //vomit!

    rOStm.SetEndian( SvStreamEndian::LITTLE );

    // write MagicCode
    rOStm.WriteOString( IMAPMAGIC );
    rOStm.WriteUInt16( IMAGE_MAP_VERSION );
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOStm, aImageName, eEncoding);
    write_uInt16_lenPrefixed_uInt8s_FromOString(rOStm, ""); //dummy
    rOStm.WriteUInt16( nCount );
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOStm, aImageName, eEncoding);

    pCompat = new IMapCompat( rOStm, StreamMode::WRITE );

    // here one can insert in newer versions

    delete pCompat;

    ImpWriteImageMap( rOStm );

    rOStm.SetEndian( nOldFormat );
}


/******************************************************************************
|*
|* load binary
|*
\******************************************************************************/

void ImageMap::Read( SvStream& rIStm )
{
    char            cMagic[6];
    SvStreamEndian  nOldFormat = rIStm.GetEndian();

    rIStm.SetEndian( SvStreamEndian::LITTLE );
    rIStm.ReadBytes(cMagic, sizeof(cMagic));

    if ( !memcmp( cMagic, IMAPMAGIC, sizeof( cMagic ) ) )
    {
        IMapCompat* pCompat;
        sal_uInt16  nCount;

        // delete old content
        ClearImageMap();

        // read on version
        rIStm.SeekRel( 2 );

        aName = read_uInt16_lenPrefixed_uInt8s_ToOUString(rIStm, osl_getThreadTextEncoding());
        read_uInt16_lenPrefixed_uInt8s_ToOString(rIStm); // Dummy
        rIStm.ReadUInt16( nCount );
        read_uInt16_lenPrefixed_uInt8s_ToOString(rIStm); // Dummy

        pCompat = new IMapCompat( rIStm, StreamMode::READ );

        // here one can read in newer versions

        delete pCompat;
        ImpReadImageMap( rIStm, nCount );

    }
    else
        rIStm.SetError( SVSTREAM_GENERALERROR );

    rIStm.SetEndian( nOldFormat );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
