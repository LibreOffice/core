/*************************************************************************
 *
 *  $RCSfile: imap.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <string.h>
#include <math.h>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/window.hxx>

#include "urihelper.hxx"
#include "imap.hxx"
#include "imapobj.hxx"
#include "imapcirc.hxx"
#include "imaprect.hxx"
#include "imappoly.hxx"


DBG_NAME( ImageMap );


#define SCALEPOINT(aPT,aFracX,aFracY) (aPT).X()=((aPT).X()*(aFracX).GetNumerator())/(aFracX).GetDenominator();  \
                                      (aPT).Y()=((aPT).Y()*(aFracY).GetNumerator())/(aFracY).GetDenominator();


/******************************************************************************/

UINT16 IMapObject::nActualTextEncoding = (UINT16) RTL_TEXTENCODING_DONTKNOW;

/******************************************************************************/


#ifdef WIN
#pragma optimize ( "", off )
#endif


/******************************************************************************
|*
|* Ctor
|*
\******************************************************************************/

IMapObject::IMapObject( const String& rURL, const String& rDescription, BOOL bURLActive ) :
            aURL            ( rURL ),
            aDescription    ( rDescription ),
            bActive         ( bURLActive )
{
}


/******************************************************************************
|*
|* Ctor
|*
\******************************************************************************/

IMapObject::IMapObject( const String& rURL, const String& rDescription,
                        const String& rTarget, BOOL bURLActive ) :
            aURL            ( rURL ),
            aDescription    ( rDescription ),
            aTarget         ( rTarget ),
            bActive         ( bURLActive )
{
}


/******************************************************************************
|*
|* Ctor
|*
\******************************************************************************/

IMapObject::IMapObject( const String& rURL, const String& rDescription,
                        const String& rTarget, const String& rName, BOOL bURLActive ) :
            aURL            ( rURL ),
            aDescription    ( rDescription ),
            aTarget         ( rTarget ),
            aName           ( rName ),
            bActive         ( bURLActive )
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

SvStream& operator<<( SvStream& rOStm, const IMapObject& rObj )
{
    IMapCompat*             pCompat;
    const rtl_TextEncoding  eEncoding = gsl_getSystemTextEncoding();

    rOStm << rObj.GetType();
    rOStm << rObj.GetVersion();
    rOStm << ( (UINT16) eEncoding );

    const ByteString aRelURL( INetURLObject::AbsToRel( rObj.aURL, INetURLObject::WAS_ENCODED,
                                                       INetURLObject::DECODE_UNAMBIGUOUS ), eEncoding );
    rOStm.WriteByteString( aRelURL );
    rOStm.WriteByteString( ByteString( rObj.aDescription, eEncoding ) );
    rOStm << rObj.bActive;
    rOStm.WriteByteString( ByteString( rObj.aTarget, eEncoding ) );

    pCompat = new IMapCompat( rOStm, STREAM_WRITE );

    rObj.WriteIMapObject( rOStm );
    rObj.aEventList.Write( rOStm );                                 // V4
    rOStm.WriteByteString( ByteString( rObj.aName, eEncoding ) );   // V5

    delete pCompat;

    return rOStm;
}


/******************************************************************************
|*
|*  Binaer-Import
|*
\******************************************************************************/

SvStream& operator>>( SvStream& rIStm, IMapObject& rObj )
{
    IMapCompat*         pCompat;
    rtl_TextEncoding    nTextEncoding;
    ByteString          aString;

    // Typ und Version ueberlesen wir
    rIStm.SeekRel( 2 );
    rIStm >> rObj.nReadVersion;
    rIStm >> nTextEncoding;
    rIStm.ReadByteString( aString ); rObj.aURL = String( aString.GetBuffer(), nTextEncoding );
    rIStm.ReadByteString( aString ); rObj.aDescription = String( aString.GetBuffer(), nTextEncoding );
    rIStm >> rObj.bActive;
    rIStm.ReadByteString( aString ); rObj.aTarget = String( aString.GetBuffer(), nTextEncoding );

    // URL absolut machen
    rObj.aURL = URIHelper::SmartRelToAbs( rObj.aURL, FALSE, INetURLObject::WAS_ENCODED, INetURLObject::DECODE_UNAMBIGUOUS );
    pCompat = new IMapCompat( rIStm, STREAM_READ );

    rObj.ReadIMapObject( rIStm );

    // ab Version 4 lesen wir eine EventListe
    if ( rObj.nReadVersion >= 0x0004 )
    {
        rObj.aEventList.Read(rIStm);

        // ab Version 5 kann ein Objektname vorhanden sein
        if ( rObj.nReadVersion >= 0x0005 )
        {
            rIStm.ReadByteString( aString ); rObj.aName = String( aString.GetBuffer(), nTextEncoding );
        }
    }

    delete pCompat;

    return rIStm;
}


/******************************************************************************
|*
|* Konvertierung der logischen Koordianten in Pixel
|*
\******************************************************************************/

Point IMapObject::GetPixelPoint( const Point& rLogPoint )
{
    return Application::GetDefaultDevice()->LogicToPixel( rLogPoint, MapMode( MAP_100TH_MM ) );
}


/******************************************************************************
|*
|* Konvertierung der logischen Koordianten in Pixel
|*
\******************************************************************************/

Point IMapObject::GetLogPoint( const Point& rPixelPoint )
{
    return Application::GetDefaultDevice()->PixelToLogic( rPixelPoint, MapMode( MAP_100TH_MM ) );
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

BOOL IMapObject::IsEqual( const IMapObject& rEqObj )
{
    return ( ( aURL == rEqObj.aURL ) &&
             ( aDescription == rEqObj.aDescription ) &&
             ( aTarget == rEqObj.aTarget ) &&
             ( aName == rEqObj.aName ) &&
             ( bActive == rEqObj.bActive ) );
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


/******************************************************************************
|*
|* Ctor
|*
\******************************************************************************/

IMapRectangleObject::IMapRectangleObject( const Rectangle& rRect,
                                          const String& rURL,
                                          const String& rDescription,
                                          BOOL bURLActive,
                                          BOOL bPixelCoords ) :
            IMapObject  ( rURL, rDescription, bURLActive )
{
    ImpConstruct( rRect, bPixelCoords );
}


/******************************************************************************
|*
|* Ctor
|*
\******************************************************************************/

IMapRectangleObject::IMapRectangleObject( const Rectangle& rRect,
                                          const String& rURL,
                                          const String& rDescription,
                                          const String& rTarget,
                                          BOOL bURLActive,
                                          BOOL bPixelCoords ) :
            IMapObject  ( rURL, rDescription, rTarget, bURLActive )
{
    ImpConstruct( rRect, bPixelCoords );
}


/******************************************************************************
|*
|* Ctor
|*
\******************************************************************************/

IMapRectangleObject::IMapRectangleObject( const Rectangle& rRect,
                                          const String& rURL,
                                          const String& rDescription,
                                          const String& rTarget,
                                          const String& rName,
                                          BOOL bURLActive,
                                          BOOL bPixelCoords ) :
            IMapObject  ( rURL, rDescription, rTarget, rName, bURLActive )
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
    Rectangle   aNewRect;

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
    Point   aTL( aRect.TopLeft() );
    Point   aBR( aRect.BottomRight() );

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


/******************************************************************************
|*
|* Ctor
|*
\******************************************************************************/

IMapCircleObject::IMapCircleObject( const Point& rCenter, ULONG nCircleRadius,
                                    const String& rURL,
                                    const String& rDescription,
                                    BOOL bURLActive,
                                    BOOL bPixelCoords ) :
            IMapObject  ( rURL, rDescription, bURLActive )
{
    ImpConstruct( rCenter, nCircleRadius, bPixelCoords );
}


/******************************************************************************
|*
|* Ctor
|*
\******************************************************************************/

IMapCircleObject::IMapCircleObject( const Point& rCenter, ULONG nCircleRadius,
                                    const String& rURL,
                                    const String& rDescription,
                                    const String& rTarget,
                                    BOOL bURLActive,
                                    BOOL bPixelCoords ) :
            IMapObject  ( rURL, rDescription, rTarget, bURLActive )
{
    ImpConstruct( rCenter, nCircleRadius, bPixelCoords );
}


/******************************************************************************
|*
|* Ctor
|*
\******************************************************************************/

IMapCircleObject::IMapCircleObject( const Point& rCenter, ULONG nCircleRadius,
                                    const String& rURL,
                                    const String& rDescription,
                                    const String& rTarget,
                                    const String& rName,
                                    BOOL bURLActive,
                                    BOOL bPixelCoords ) :
            IMapObject  ( rURL, rDescription, rTarget, rName, bURLActive )
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
    const Point aPoint( aCenter - rPoint );
    BOOL        bRet = FALSE;

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


/******************************************************************************
|*
|* Ctor
|*
\******************************************************************************/

IMapPolygonObject::IMapPolygonObject( const Polygon& rPoly,
                                      const String& rURL,
                                      const String& rDescription,
                                      BOOL bURLActive,
                                      BOOL bPixelCoords ) :
            IMapObject  ( rURL, rDescription, bURLActive ),
            bEllipse    ( FALSE )
{
    ImpConstruct( rPoly, bPixelCoords );
}


/******************************************************************************
|*
|* Ctor
|*
\******************************************************************************/

IMapPolygonObject::IMapPolygonObject( const Polygon& rPoly,
                                      const String& rURL,
                                      const String& rDescription,
                                      const String& rTarget,
                                      BOOL bURLActive,
                                      BOOL bPixelCoords ) :
            IMapObject  ( rURL, rDescription, rTarget, bURLActive ),
            bEllipse    ( FALSE )
{
    ImpConstruct( rPoly, bPixelCoords );
}


/******************************************************************************
|*
|* Ctor
|*
\******************************************************************************/

IMapPolygonObject::IMapPolygonObject( const Polygon& rPoly,
                                      const String& rURL,
                                      const String& rDescription,
                                      const String& rTarget,
                                      const String& rName,
                                      BOOL bURLActive,
                                      BOOL bPixelCoords ) :
            IMapObject  ( rURL, rDescription, rTarget, rName, bURLActive ),
            bEllipse    ( FALSE )
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
    Polygon aNewPoly;

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

void IMapPolygonObject::SetExtraEllipse( const Rectangle& rEllipse )
{
    if ( aPoly.GetSize() )
    {
        bEllipse = TRUE;
        aEllipse = rEllipse;
    }
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
        const Polygon&  rEqPoly = rEqObj.aPoly;
        const USHORT    nCount = aPoly.GetSize();
        const USHORT    nEqCount = rEqPoly.GetSize();
        BOOL            bDifferent = FALSE;

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
|* Ctor
|*
\******************************************************************************/

ImageMap::ImageMap( const String& rName ) :
            aName   ( rName )
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
    const USHORT    nCount = (USHORT) maList.Count();
    const USHORT    nEqCount = rImageMap.GetIMapObjectCount();
    BOOL            bRet = FALSE;

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
|* Hit-Test
|*
\******************************************************************************/

IMapObject* ImageMap::GetHitIMapObject( const Size& rTotalSize,
                                        const Size& rDisplaySize,
                                        const Point& rRelHitPoint,
                                        ULONG nFlags )
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
    IMapObject* pObj = (IMapObject*) maList.First();
    while ( pObj )
    {
        if ( pObj->IsHit( aRelPoint ) )
            break;

        pObj = (IMapObject*) maList.Next();
    }

    return( pObj ? ( pObj->IsActive() ? pObj : NULL ) : NULL );
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

Rectangle ImageMap::GetBoundRect() const
{
    Rectangle   aBoundRect;
    ULONG       nCount = maList.Count();

    for ( ULONG i = 0; i < nCount; i++ )
        aBoundRect.Union( ( (IMapObject*) maList.GetObject( i ) )->GetBoundRect() );

    return aBoundRect;
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

void ImageMap::ImpWriteImageMap( SvStream& rOStm ) const
{
    IMapObject* pObj;
    USHORT      nCount = (USHORT) maList.Count();

    for ( USHORT i = 0; i < nCount; i++ )
    {
        pObj = (IMapObject*) maList.GetObject( i );
        rOStm << *pObj;
    }
}


/******************************************************************************
|*
|* Objekte nacheinander lesen
|*
\******************************************************************************/

void ImageMap::ImpReadImageMap( SvStream& rIStm, USHORT nCount )
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
                rIStm >> *pObj;
                maList.Insert( pObj, LIST_APPEND );
            }
            break;

            case ( IMAP_OBJ_CIRCLE ):
            {
                IMapCircleObject* pObj = new IMapCircleObject;
                rIStm >> *pObj;
                maList.Insert( pObj, LIST_APPEND );
            }
            break;

            case ( IMAP_OBJ_POLYGON ):
            {
                IMapPolygonObject* pObj = new IMapPolygonObject;
                rIStm >> *pObj;
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

SvStream& operator<<( SvStream& rOStm, const ImageMap& rImageMap )
{
    IMapCompat*             pCompat;
    String                  aName( rImageMap.GetName() );
    String                  aDummy;
    USHORT                  nOldFormat = rOStm.GetNumberFormatInt();
    UINT16                  nCount = (UINT16) rImageMap.GetIMapObjectCount();
    const rtl_TextEncoding  eEncoding = gsl_getSystemTextEncoding();

    rOStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    // MagicCode schreiben
    rOStm << IMAPMAGIC;
    rOStm << rImageMap.GetVersion();
    rOStm.WriteByteString( ByteString( aName, eEncoding ) );
    rOStm.WriteByteString( ByteString( aDummy, eEncoding ) );
    rOStm << nCount;
    rOStm.WriteByteString( ByteString( aName, eEncoding ) );

    pCompat = new IMapCompat( rOStm, STREAM_WRITE );

    // hier kann in neueren Versionen eingefuegt werden

    delete pCompat;

    rImageMap.ImpWriteImageMap( rOStm );

    rOStm.SetNumberFormatInt( nOldFormat );

    return rOStm;
}


/******************************************************************************
|*
|* Binaer laden
|*
\******************************************************************************/

SvStream& operator>>( SvStream& rIStm, ImageMap& rImageMap )
{
    ByteString  aString;
    char        cMagic[6];
    USHORT      nOldFormat = rIStm.GetNumberFormatInt();
    UINT16      nCount;

    rIStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rIStm.Read( cMagic, sizeof( cMagic ) );

    if ( !memcmp( cMagic, IMAPMAGIC, sizeof( cMagic ) ) )
    {
        IMapCompat* pCompat;

        // alten Inhalt loeschen
        rImageMap.ClearImageMap();

        // Version ueberlesen wir
        rIStm.SeekRel( 2 );

        rIStm.ReadByteString( aString ); rImageMap.aName = String( aString, gsl_getSystemTextEncoding() );
        rIStm.ReadByteString( aString ); // Dummy
        rIStm >> nCount;
        rIStm.ReadByteString( aString ); // Dummy

        pCompat = new IMapCompat( rIStm, STREAM_READ );

        // hier kann in neueren Versionen gelesen werden

        delete pCompat;
        rImageMap.ImpReadImageMap( rIStm, nCount );

    }
    else
        rIStm.SetError( SVSTREAM_GENERALERROR );

    rIStm.SetNumberFormatInt( nOldFormat );

    return rIStm;
}


#ifdef WIN
#pragma optimize ( "", on )
#endif


