/*************************************************************************
 *
 *  $RCSfile: svdoashp.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:47:29 $
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

#ifndef _SVDOASHP_HXX
#include "svdoashp.hxx"
#endif
#ifndef _SVX_UNOAPI_HXX_
#include "unoapi.hxx"
#endif
#ifndef _SVX_UNOSHAPE_HXX
#include "unoshape.hxx"
#endif
#ifndef _UCBHELPER_CONTENT_HXX_
#include <ucbhelper/content.hxx>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX_
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _UNOTOOLS_DATETIME_HXX_
#include <unotools/datetime.hxx>
#endif
#ifndef SVX_LIGHT
#ifndef _LNKBASE_HXX //autogen
#include <so3/lnkbase.hxx>
#endif
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_XCUSTOMSHAPEENGINE_HPP_
#include <drafts/com/sun/star/drawing/XCustomShapeEngine.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONBEZIERCOORDS_HPP_
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef __com_sun_star_awt_Rectangle_hpp_
#include <com/sun/star/awt/Rectangle.hpp>
#endif
#ifndef _SVX_UNOPOLYHELPER_HXX
#include "unopolyhelper.hxx"
#endif
#include <comphelper/processfactory.hxx>
#include <svtools/urihelper.hxx>
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _SVDOGRP_HXX
#include "svdogrp.hxx"
#endif
#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>     // FRound
#endif
#ifndef _SVDHDL_HXX
#include "svdhdl.hxx"
#endif
#ifndef _SVDDRAG_HXX
#include "svddrag.hxx"
#endif
#include "xpool.hxx"
#include "xpoly.hxx"
#include "svdxout.hxx"
#include "svdmodel.hxx"
#include "svdpage.hxx"
#include "svditer.hxx"
#include "svdobj.hxx"
#include "svdtrans.hxx"
#include "svdio.hxx"
#include "svdetc.hxx"
#include "svdattrx.hxx"  // NotPersistItems
#include "svdoedge.hxx"  // #32383# Die Verbinder nach Move nochmal anbroadcasten
#include "svdglob.hxx"   // StringCache
#include "svdstr.hrc"    // Objektname
#include "eeitem.hxx"
#include "editstat.hxx"
#include "svdoutl.hxx"
#include "outlobj.hxx"
#include "sdtfchim.hxx"


//      textitem.hxx        editdata.hxx
#define ITEMID_COLOR        EE_CHAR_COLOR
#define ITEMID_FONT         EE_CHAR_FONTINFO
#define ITEMID_FONTHEIGHT   EE_CHAR_FONTHEIGHT
#define ITEMID_FONTWIDTH    EE_CHAR_FONTWIDTH
#define ITEMID_WEIGHT       EE_CHAR_WEIGHT
#define ITEMID_UNDERLINE    EE_CHAR_UNDERLINE
#define ITEMID_CROSSEDOUT   EE_CHAR_STRIKEOUT
#define ITEMID_POSTURE      EE_CHAR_ITALIC
#define ITEMID_CONTOUR      EE_CHAR_OUTLINE
#define ITEMID_SHADOWED     EE_CHAR_SHADOW
#define ITEMID_ESCAPEMENT   EE_CHAR_ESCAPEMENT
#define ITEMID_AUTOKERN     EE_CHAR_PAIRKERNING
#define ITEMID_WORDLINEMODE EE_CHAR_WLM
//      paraitem.hxx       editdata.hxx
#define ITEMID_ADJUST      EE_PARA_JUST
#define ITEMID_FIELD       EE_FEATURE_FIELD
#include "xlnclit.hxx"



#ifndef _PERSIST_HXX //autogen
#include <so3/persist.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif

#ifndef _SDR_PROPERTIES_CUSTOMSHAPEPROPERTIES_HXX
#include <svx/sdr/properties/customshapeproperties.hxx>
#endif
#ifndef _SDR_CONTACT_VIEWCONTACTOFSDROBJCUSTOMSHAPE_HXX
#include <svx/sdr/contact/viewcontactofsdrobjcustomshape.hxx>
#endif

// #104018# replace macros above with type-safe methods
inline double ImplTwipsToMM(double fVal) { return (fVal * (127.0 / 72.0)); }
inline double ImplMMToTwips(double fVal) { return (fVal * (72.0 / 127.0)); }

static Rectangle aOld;
static Rectangle aNew;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using namespace ::drafts::com::sun::star::drawing;

Reference< XCustomShapeEngine > SdrObjCustomShape::GetCustomShapeEngine( const SdrObjCustomShape* pCustomShape ) const
{
    Reference< XCustomShapeEngine > xCustomShapeEngine;
    String aEngine(((SdrCustomShapeEngineItem&)pCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_ENGINE )).GetValue());
    if ( !aEngine.Len() )
        aEngine = String( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.drawing.EnhancedCustomShapeEngine" ) );

    Reference< XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );

    Reference< XShape > aXShape = GetXShapeForSdrObject( (SdrObjCustomShape*)pCustomShape );
    if ( !aXShape.is() )
        aXShape = new SvxCustomShape( (SdrObjCustomShape*)pCustomShape );

    if ( aEngine.Len() && xFactory.is() && aXShape.is() )
    {
        Sequence< Any > aArgument( 1 );
        Sequence< PropertyValue > aPropValues( 1 );
        aPropValues[ 0 ].Name = rtl::OUString::createFromAscii( "CustomShape" );
        aPropValues[ 0 ].Value <<= aXShape;
        aArgument[ 0 ] <<= aPropValues;
        Reference< XInterface > xInterface( xFactory->createInstanceWithArguments( aEngine, aArgument ) );
        if ( xInterface.is() )
            xCustomShapeEngine = Reference< XCustomShapeEngine >( xInterface, UNO_QUERY );
    }
    return xCustomShapeEngine;
}
const SdrObject* SdrObjCustomShape::GetSdrObjectFromCustomShape() const
{
    if ( !pRenderedCustomShape )
    {
        Reference< XCustomShapeEngine > xCustomShapeEngine( GetCustomShapeEngine( this ) );
        if ( xCustomShapeEngine.is() )
        {
            Reference< XShape > xRenderedShape = xCustomShapeEngine->render();
            if ( xRenderedShape.is() )
            {
                ((SdrObjCustomShape*)this)->pRenderedCustomShape = GetSdrObjectFromXShape( xRenderedShape );

                uno::Reference< lang::XComponent > xComponent( xRenderedShape, uno::UNO_QUERY );
                xComponent->dispose();
            }
        }
    }
    return pRenderedCustomShape;
}

const sal_Bool SdrObjCustomShape::IsTextPath() const
{
    const rtl::OUString sTextPath( RTL_CONSTASCII_USTRINGPARAM ( "TextPath" ) );
    const rtl::OUString sOn( RTL_CONSTASCII_USTRINGPARAM ( "On" ) );
    sal_Bool bTextPathOn = sal_False;
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
    Any* pAny = rGeometryItem.GetPropertyValueByName( sTextPath, sOn );
    if ( pAny )
        *pAny >>= bTextPathOn;
    return bTextPathOn;
}

const sal_Bool SdrObjCustomShape::IsMirroredX() const
{
    sal_Bool bMirroredX = sal_False;
    SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
    const rtl::OUString sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
    com::sun::star::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sMirroredX );
    if ( pAny )
        *pAny >>= bMirroredX;
    return bMirroredX;
}
const sal_Bool SdrObjCustomShape::IsMirroredY() const
{
    sal_Bool bMirroredY = sal_False;
    SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
    const rtl::OUString sMirroredY( RTL_CONSTASCII_USTRINGPARAM ( "MirroredY" ) );
    com::sun::star::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sMirroredY );
    if ( pAny )
        *pAny >>= bMirroredY;
    return bMirroredY;
}
void SdrObjCustomShape::SetMirroredX( const sal_Bool bMirrorX )
{
    SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
    const rtl::OUString sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
    com::sun::star::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sMirroredX );
    PropertyValue aPropVal;
    aPropVal.Name = sMirroredX;
    aPropVal.Value <<= bMirrorX;
    aGeometryItem.SetPropertyValue( aPropVal );
    SetMergedItem( aGeometryItem );
}
void SdrObjCustomShape::SetMirroredY( const sal_Bool bMirrorY )
{
    SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
    const rtl::OUString sMirroredY( RTL_CONSTASCII_USTRINGPARAM ( "MirroredY" ) );
    com::sun::star::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sMirroredY );
    PropertyValue aPropVal;
    aPropVal.Name = sMirroredY;
    aPropVal.Value <<= bMirrorY;
    aGeometryItem.SetPropertyValue( aPropVal );
    SetMergedItem( aGeometryItem );
}

const double SdrObjCustomShape::GetObjectRotation() const
{
    return fObjectRotation;
}

const double SdrObjCustomShape::GetExtraTextRotation() const
{
    const com::sun::star::uno::Any* pAny;
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
/*
    const rtl::OUString sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
    const rtl::OUString sMirroredY( RTL_CONSTASCII_USTRINGPARAM ( "MirroredY" ) );
    pAny = rGeometryItem.GetPropertyValueByName( sMirroredX );
    sal_Bool bFlipH;
    if ( pAny && ( *pAny >>= bFlipH ) && bFlipH )
        nDrehWink = -nDrehWink;
    pAny = rGeometryItem.GetPropertyValueByName( sMirroredY );
    sal_Bool bFlipV;
    if ( pAny && ( *pAny >>= bFlipV ) && bFlipV )
        nDrehWink = -nDrehWink;
*/
    const rtl::OUString sTextRotateAngle( RTL_CONSTASCII_USTRINGPARAM ( "TextRotateAngle" ) );
    pAny = rGeometryItem.GetPropertyValueByName( sTextRotateAngle );
    double fExtraTextRotateAngle = 0.0;
    if ( pAny )
        *pAny >>= fExtraTextRotateAngle;
    return fExtraTextRotateAngle;
}
const sal_Bool SdrObjCustomShape::GetTextBounds( Rectangle& rTextBound ) const
{
    sal_Bool bRet = sal_False;
    Reference< XCustomShapeEngine > xCustomShapeEngine( GetCustomShapeEngine( this ) ); // a candidate for being cached
    if ( xCustomShapeEngine.is() )
    {
        awt::Rectangle aR( xCustomShapeEngine->getTextBounds() );
        rTextBound = Rectangle( Point( aR.X, aR.Y ), Size( aR.Width, aR.Height ) );
        bRet = sal_True;
    }
    return bRet;
}
const sal_Bool SdrObjCustomShape::GetLineGeometry( XPolyPolygon& rLineGeometry, const SdrObjCustomShape* pCustomShape, const sal_Bool bBezierAllowed ) const
{
    sal_Bool bRet = sal_False;
    Reference< XCustomShapeEngine > xCustomShapeEngine( GetCustomShapeEngine( pCustomShape ) );
    if ( xCustomShapeEngine.is() )
    {
        com::sun::star::drawing::PolyPolygonBezierCoords aBezierCoords = xCustomShapeEngine->getLineGeometry();
        try
        {
            SvxConvertPolyPolygonBezierToXPolyPolygon( &aBezierCoords, rLineGeometry );
            if ( !bBezierAllowed )
            {
                sal_uInt16 i;
                XPolyPolygon aXPP;
                for ( i = 0; i < rLineGeometry.Count(); i++ )
                    aXPP.Insert( XOutCreatePolygon( rLineGeometry.GetObject( i ), NULL, 100 ), XPOLYPOLY_APPEND );
                rLineGeometry = aXPP;
            }
            bRet = sal_True;
        }
        catch ( const com::sun::star::lang::IllegalArgumentException )
        {

        }
    }
    return bRet;
}
Sequence< Reference< drafts::com::sun::star::drawing::XCustomShapeHandle > > SdrObjCustomShape::GetInteraction( const SdrObjCustomShape* pCustomShape ) const
{
    Sequence< Reference< drafts::com::sun::star::drawing::XCustomShapeHandle > > xInteraction;
    Reference< XCustomShapeEngine > xCustomShapeEngine( GetCustomShapeEngine( pCustomShape ) );
    if ( xCustomShapeEngine.is() )
        xInteraction = xCustomShapeEngine->getInteraction();
    return xInteraction;
}

//////////////////////////////////////////////////////////////////////////////
// BaseProperties section

sdr::properties::BaseProperties* SdrObjCustomShape::CreateObjectSpecificProperties()
{
    return new sdr::properties::CustomShapeProperties(*this);
}

TYPEINIT1(SdrObjCustomShape,SdrTextObj);
SdrObjCustomShape::SdrObjCustomShape() :
    SdrTextObj(),
    fObjectRotation( 0.0 ),
    pRenderedCustomShape( NULL )
{
    bTextFrame = TRUE;
}

SdrObjCustomShape::~SdrObjCustomShape()
{
    delete pRenderedCustomShape, pRenderedCustomShape = NULL;
}

void SdrObjCustomShape::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bResizeFreeAllowed=TRUE;
    rInfo.bResizePropAllowed=TRUE;
    rInfo.bRotateFreeAllowed=TRUE;
    rInfo.bRotate90Allowed  =TRUE;
    rInfo.bMirrorFreeAllowed=TRUE;
    rInfo.bMirror45Allowed  =TRUE;
    rInfo.bMirror90Allowed  =TRUE;
    rInfo.bTransparenceAllowed = FALSE;
    rInfo.bGradientAllowed = FALSE;
    rInfo.bShearAllowed     =FALSE;
    rInfo.bEdgeRadiusAllowed=FALSE;
    rInfo.bNoContortion     =TRUE;
}

UINT16 SdrObjCustomShape::GetObjIdentifier() const
{
    return UINT16(OBJ_CUSTOMSHAPE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrObjCustomShape::RecalcSnapRect()
{
    SdrTextObj::RecalcSnapRect();
}
void SdrObjCustomShape::RecalcBoundRect()
{
    aOutRect = GetSnapRect();

    const SdrObject* pSdrObject = GetSdrObjectFromCustomShape();
    if ( pSdrObject )
        aOutRect = pSdrObject->GetCurrentBoundRect();

    long nLineWdt = ImpGetLineWdt();
    nLineWdt++; nLineWdt/=2;
    if ( nLineWdt != 0 )
    {
        long a = nLineWdt;
        if ( ( aGeo.nDrehWink != 0 || aGeo.nShearWink != 0) && GetEckenradius() == 0 )
        {
            a*=2; // doppelt, wegen evtl. spitzen Ecken
        }
        aOutRect.Left  ()-=a;
        aOutRect.Top   ()-=a;
        aOutRect.Right ()+=a;
        aOutRect.Bottom()+=a;
    }
    SdrTextObj::ImpAddShadowToBoundRect();

    // add text to ImpAddTextToBoundrect:
    if ( pOutlinerParaObject )
    {
        SdrOutliner& rOutliner=ImpGetDrawOutliner();
        Rectangle aTextRect;
        Rectangle aAnchorRect;
        TakeTextRect( rOutliner, aTextRect, TRUE, &aAnchorRect ); // EditText ignorieren!
        rOutliner.Clear();
        if ( aGeo.nDrehWink )
        {
            Polygon aPol( aTextRect );
            RotatePoly( aPol, aTextRect.TopLeft(), aGeo.nSin, aGeo.nCos );
            aOutRect.Union( aPol.GetBoundRect() );
        }
        else
        {
            aOutRect.Union( aTextRect );
        }
    }
}

const Rectangle& SdrObjCustomShape::GetSnapRect() const
{
    return SdrTextObj::GetSnapRect();
}
const Rectangle& SdrObjCustomShape::GetCurrentBoundRect() const
{
    return SdrTextObj::GetCurrentBoundRect();
}
const Rectangle& SdrObjCustomShape::GetLogicRect() const
{
    return SdrTextObj::GetLogicRect();
}
void SdrObjCustomShape::NbcSetSnapRect( const Rectangle& rRect )
{
    aRect=rRect;
    ImpJustifyRect(aRect);
    Rectangle aTextBound( aRect );
    if ( GetTextBounds( aTextBound ) )
    {
        if ( pModel==NULL || !pModel->IsPasteResize() )
        {
            long nHDist=GetTextLeftDistance()+GetTextRightDistance();
            long nVDist=GetTextUpperDistance()+GetTextLowerDistance();
            long nTWdt=aTextBound.GetWidth ()-1-nHDist; if (nTWdt<0) nTWdt=0;
            long nTHgt=aTextBound.GetHeight()-1-nVDist; if (nTHgt<0) nTHgt=0;
            if ( IsAutoGrowWidth() )
                NbcSetMinTextFrameWidth( nTWdt );
            if ( IsAutoGrowHeight() )
                NbcSetMinTextFrameHeight( nTHgt );
            NbcAdjustTextFrameWidthAndHeight();
        }
    }
    ImpCheckShear();
    SetRectsDirty();
    SetChanged();
    delete pRenderedCustomShape, pRenderedCustomShape = NULL;
}
void SdrObjCustomShape::SetSnapRect( const Rectangle& rRect )
{
    NbcSetSnapRect( rRect );
    delete pRenderedCustomShape, pRenderedCustomShape = NULL;
}
void SdrObjCustomShape::NbcSetLogicRect( const Rectangle& rRect )
{
    aRect = rRect;
    ImpJustifyRect( aRect );
    Rectangle aTextBound( aRect );
    if ( GetTextBounds( aTextBound ) )
    {
        long nHDist=GetTextLeftDistance()+GetTextRightDistance();
        long nVDist=GetTextUpperDistance()+GetTextLowerDistance();

        long nTWdt=aTextBound.GetWidth()-1-nHDist; if (nTWdt<0) nTWdt=0;
        long nTHgt=aTextBound.GetHeight()-1-nVDist; if (nTHgt<0) nTHgt=0;
        if ( IsAutoGrowWidth() )
            NbcSetMinTextFrameWidth( nTWdt );
        if ( IsAutoGrowHeight() )
            NbcSetMinTextFrameHeight( nTHgt );
        NbcAdjustTextFrameWidthAndHeight();
    }
    SetRectsDirty();
    SetChanged();
    delete pRenderedCustomShape, pRenderedCustomShape = NULL;
}
void SdrObjCustomShape::SetLogicRect( const Rectangle& rRect )
{
    NbcSetLogicRect(rRect);
    delete pRenderedCustomShape, pRenderedCustomShape = NULL;
}

void SdrObjCustomShape::Move( const Size& rSiz )
{
    SdrTextObj::Move( rSiz );
    delete pRenderedCustomShape, pRenderedCustomShape = NULL;
}
void SdrObjCustomShape::NbcMove( const Size& rSiz )
{
    SdrTextObj::NbcMove( rSiz );
    delete pRenderedCustomShape, pRenderedCustomShape = NULL;
}

void SdrObjCustomShape::Resize( const Point& rRef, const Fraction& xFact, const Fraction& yFact )
{
    SdrTextObj::Resize( rRef, xFact, yFact );
    delete pRenderedCustomShape, pRenderedCustomShape = NULL;
}
void SdrObjCustomShape::NbcResize( const Point& rRef, const Fraction& xFact, const Fraction& yFact )
{
    SdrTextObj::NbcResize( rRef, xFact, yFact );
    delete pRenderedCustomShape, pRenderedCustomShape = NULL;
}

void SdrObjCustomShape::NbcRotate( const Point& rRef, long nWink, double sn, double cs )
{
    sal_Bool bMirroredX = IsMirroredX();
    sal_Bool bMirroredY = IsMirroredY();

    fObjectRotation = fmod( fObjectRotation, 360.0 );
    if ( fObjectRotation < 0 )
        fObjectRotation = 360 + fObjectRotation;

    // the rotation angle for ashapes is stored in fObjectRotation, this rotation
    // has to be applied to the text object (which is internally using aGeo.nWink).
    SdrTextObj::NbcRotate( aRect.TopLeft(), -aGeo.nDrehWink,        // retrieving the unrotated text object
                            sin( (-aGeo.nDrehWink) * F_PI18000 ),
                            cos( (-aGeo.nDrehWink) * F_PI18000 ) );
    aGeo.nDrehWink = 0;                                             // resetting aGeo data
    aGeo.RecalcSinCos();

    long nW = (long)( fObjectRotation * 100 );                      // applying our object rotation
    if ( bMirroredY )
        nW = 36000 - nW;
    if ( bMirroredX )
        nW = 18000 - nW;
    nW = nW % 36000;
    if ( nW < 0 )
        nW = 36000 + nW;
    SdrTextObj::NbcRotate( aRect.TopLeft(), nW,                     // applying text rotation
                            sin( nW * F_PI18000 ),
                            cos( nW * F_PI18000 ) );

    int nSwap = 0;
    if ( bMirroredX )
        nSwap ^= 1;
    if ( bMirroredY )
        nSwap ^= 1;

    double fWink = nWink;                                                   // updating to our new object rotation
    fWink /= 100.0;
    fObjectRotation = fmod( nSwap ? fObjectRotation - fWink : fObjectRotation + fWink, 360.0 );
    if ( fObjectRotation < 0 )
        fObjectRotation = 360 + fObjectRotation;

    SdrTextObj::NbcRotate( rRef, nWink, sn, cs );                           // applying text rotation
    delete pRenderedCustomShape, pRenderedCustomShape = NULL;
}

void SdrObjCustomShape::NbcMirror( const Point& rRef1, const Point& rRef2 )
{
    // storing horizontal and vertical flipping without modifying the rotate angle

    sal_Bool bHorz = sal_False;
    sal_Bool bVert = sal_False;
    if ( rRef1.X() == rRef2.X() )
        bHorz = sal_True;
    if ( rRef1.Y() == rRef2.Y() )
        bVert = sal_True;
    if ( !bHorz && !bVert )
        bHorz = bVert = sal_True;

    if ( bHorz || bVert )
    {
        SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

        /////////////////
        // "MirroredX" //
        /////////////////
        if ( bHorz )
        {
            const rtl::OUString sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
            com::sun::star::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sMirroredX );
            if ( pAny )
            {
                sal_Bool bFlip;
                if ( *pAny >>= bFlip )
                {
                    if ( bFlip )
                        bHorz = sal_False;
                }
            }
            PropertyValue aPropVal;
            aPropVal.Name = sMirroredX;
            aPropVal.Value <<= bHorz;
            aGeometryItem.SetPropertyValue( aPropVal );
        }

        /////////////////
        // "MirroredY" //
        /////////////////
        if ( bVert )
        {
            const rtl::OUString sMirroredY( RTL_CONSTASCII_USTRINGPARAM ( "MirroredY" ) );
            com::sun::star::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sMirroredY );
            if ( pAny )
            {
                sal_Bool bFlip;
                if ( *pAny >>= bFlip )
                {
                    if ( bFlip )
                        bVert = sal_False;
                }
            }
            PropertyValue aPropVal;
            aPropVal.Name = sMirroredY;
            aPropVal.Value <<= bVert;
            aGeometryItem.SetPropertyValue( aPropVal );
        }
        SetMergedItem( aGeometryItem );
    }
    SdrTextObj::NbcMirror( rRef1, rRef2 );
    delete pRenderedCustomShape, pRenderedCustomShape = NULL;
}

void SdrObjCustomShape::Shear( const Point& rRef, long nWink, double tn, FASTBOOL bVShear )
{
    SdrTextObj::Shear( rRef, nWink, tn, bVShear );
    delete pRenderedCustomShape, pRenderedCustomShape = NULL;
}
void SdrObjCustomShape::NbcShear( const Point& rRef, long nWink, double tn, FASTBOOL bVShear )
{
    SdrTextObj::NbcShear(rRef,nWink,tn,bVShear);
    delete pRenderedCustomShape, pRenderedCustomShape = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrObjCustomShape::DoPaintObject(ExtOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec) const
{
    sal_Bool bOk = sal_True;

    //if( ( rInfoRec.nPaintMode & SDRPAINTMODE_MASTERPAGE ) && bNotVisibleAsMaster )    // maybe this should
    //  return sal_True;                                                            // be removed
    // AW: Yes, it should be removed :-)

    const SdrObject* pSdrObject = GetSdrObjectFromCustomShape();
    if ( pSdrObject )
    {
        ((SdrObject*)pSdrObject)->SetLayer( GetLayer() );
        if ( pSdrObject->ISA( SdrObjGroup ) )
            ((SdrObject*)pSdrObject)->SingleObjectPainter( rXOut, rInfoRec );
        else
            pSdrObject->DoPaintObject( rXOut, rInfoRec );
    }
    if ( HasText() )
    {
        if ( IsInEditMode() || ( IsTextPath() == sal_False ) )
            SdrTextObj::DoPaintObject( rXOut, rInfoRec );
    }
    return bOk;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const SdrGluePointList* SdrObjCustomShape::GetGluePointList() const
{
    const SdrObject* pSdrObject = GetSdrObjectFromCustomShape();
    if ( pSdrObject )
    {
        const SdrGluePointList* pSource = pSdrObject->GetGluePointList();
        if ( pSource )
            ((SdrObjCustomShape*)this)->ForceGluePointList();
    }
    return pPlusData ? pPlusData->pGluePoints : NULL;
}

SdrGluePointList* SdrObjCustomShape::GetGluePointList()
{
    const SdrObject* pSdrObject = GetSdrObjectFromCustomShape();
    if ( pSdrObject )
    {
        const SdrGluePointList* pSource = pSdrObject->GetGluePointList();
        if ( pSource )
            ((SdrObjCustomShape*)this)->ForceGluePointList();
    }
    return pPlusData ? pPlusData->pGluePoints : NULL;
}

SdrGluePointList* SdrObjCustomShape::ForceGluePointList()
{
    SdrGluePointList* pRet = SdrTextObj::ForceGluePointList();
    const SdrObject* pSdrObject = GetSdrObjectFromCustomShape();
    if ( pSdrObject )
    {
        const SdrGluePointList* pSource = pSdrObject->GetGluePointList();
        if ( pSource )
        {
            pRet->Clear();
            *pRet = *pSource;
        }
    }
    return pRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

USHORT SdrObjCustomShape::GetHdlCount() const
{
    USHORT nBasicHdlCount = SdrTextObj::GetHdlCount();
    Sequence< Reference< drafts::com::sun::star::drawing::XCustomShapeHandle > >
        sCustomShapeHandles = GetInteraction( this );
    return (USHORT)( sCustomShapeHandles.getLength() + nBasicHdlCount );
}

SdrHdl* SdrObjCustomShape::GetHdl( USHORT nHdlNum ) const
{
    SdrHdl* pH = NULL;
    USHORT nBasicHdlCount = SdrTextObj::GetHdlCount();

    if ( nHdlNum < nBasicHdlCount )
        pH = SdrTextObj::GetHdl( nHdlNum );
    else
    {
        Sequence< Reference< drafts::com::sun::star::drawing::XCustomShapeHandle > >
            sCustomShapeHandles = GetInteraction( this );
        USHORT nCustomShapeHdlNum = nHdlNum - nBasicHdlCount;
        if ( nCustomShapeHdlNum < sCustomShapeHandles.getLength() )
        {
            Reference< drafts::com::sun::star::drawing::XCustomShapeHandle > xHdl
                = sCustomShapeHandles[ nCustomShapeHdlNum ];
            if ( xHdl.is() )
            {
                try
                {
                    com::sun::star::awt::Point aPosition( xHdl->getPosition() );
                    pH = new SdrHdl( Point( aPosition.X, aPosition.Y ), HDL_CUSTOMSHAPE1 );
                    pH->SetPointNum( nCustomShapeHdlNum );
                    pH->SetObj( (SdrObject*)this );
                }
                catch ( const uno::RuntimeException& )
                {
                    // it was not possible to retrieve a handle, maybe the shape type
                    // has been changed, or the shape is not inserted into a draw page
                }
            }
        }
    }
    return pH;
}

FASTBOOL SdrObjCustomShape::HasSpecialDrag() const
{
    return TRUE;
}

FASTBOOL SdrObjCustomShape::BegDrag( SdrDragStat& rDrag ) const
{
    FASTBOOL bRet = TRUE;
    const SdrHdl* pHdl = rDrag.GetHdl();
    if ( pHdl && ( pHdl->GetKind() == HDL_CUSTOMSHAPE1 ) )
    {
        rDrag.SetEndDragChangesAttributes( sal_True );
        rDrag.SetNoSnap( TRUE );
        rDrag.SetUser( Clone() );
    }
    else
        bRet = SdrTextObj::BegDrag( rDrag );
    return bRet;
}

FASTBOOL SdrObjCustomShape::MovDrag( SdrDragStat& rDrag ) const
{
    FASTBOOL bRet = FALSE;
    const SdrHdl* pHdl = rDrag.GetHdl();
    if ( pHdl && ( pHdl->GetKind() == HDL_CUSTOMSHAPE1 ) )
    {
        Sequence< Reference< drafts::com::sun::star::drawing::XCustomShapeHandle > >
            sCustomShapeHandles = GetInteraction( (SdrObjCustomShape*)rDrag.GetUser() );
        USHORT nCustomShapeHdlNum = pHdl->GetPointNum();
        if ( nCustomShapeHdlNum < sCustomShapeHandles.getLength() )
        {
            Reference< drafts::com::sun::star::drawing::XCustomShapeHandle > xHdl
                = sCustomShapeHandles[ nCustomShapeHdlNum ];
            if ( xHdl.is() )
            {
                try
                {
                    Point aPt( rDrag.GetNow() );
                    xHdl->setControllerPosition( com::sun::star::awt::Point( aPt.X(), aPt.Y() ) );
                    bRet = TRUE;
                }
                catch ( const uno::RuntimeException& )
                {
                    // it was not possible to retrieve a handle, maybe the shape type
                    // has been changed, or the shape is not inserted into a draw page
                }
            }
        }
    }
    else
        bRet = SdrTextObj::MovDrag( rDrag );
    return bRet;
}

FASTBOOL SdrObjCustomShape::EndDrag( SdrDragStat& rDrag )
{
    FASTBOOL bRet = TRUE;
    const SdrHdl* pHdl = rDrag.GetHdl();
    if ( pHdl && ( pHdl->GetKind() == HDL_CUSTOMSHAPE1 ) )
    {
        Rectangle aBoundRect0;
        if ( pUserCall )
            aBoundRect0 = GetLastBoundRect();
    //  SendRepaintBroadcast();

        Point aPt( rDrag.GetNow() );
        Sequence< Reference< drafts::com::sun::star::drawing::XCustomShapeHandle > >
            sCustomShapeHandles = GetInteraction( this );
        USHORT nCustomShapeHdlNum = pHdl->GetPointNum();
        if ( nCustomShapeHdlNum < sCustomShapeHandles.getLength() )
        {
            Reference< drafts::com::sun::star::drawing::XCustomShapeHandle > xHdl
                = sCustomShapeHandles[ nCustomShapeHdlNum ];
            if ( xHdl.is() )
            {
                try
                {
                    Point aPt( rDrag.GetNow() );
                    xHdl->setControllerPosition( com::sun::star::awt::Point( aPt.X(), aPt.Y() ) );
                    bRet = TRUE;
                }
                catch ( const uno::RuntimeException& )
                {
                    // it was not possible to retrieve a handle, maybe the shape type
                    // has been changed, or the shape is not inserted into a draw page
                }
            }
        }

        SetRectsDirty();
        delete pRenderedCustomShape, pRenderedCustomShape = NULL;
        SetChanged();
    //  SendRepaintBroadcast();
        BroadcastObjectChange();
        SendUserCall( SDRUSERCALL_RESIZE, aBoundRect0 );
        delete (SdrObjCustomShape*)rDrag.GetUser(); rDrag.SetUser( NULL );
    }
    else
        bRet = SdrTextObj::EndDrag( rDrag );
    return bRet;
}

FASTBOOL SdrObjCustomShape::BegCreate( SdrDragStat& rDrag )
{
    return SdrTextObj::BegCreate( rDrag );
}

void SdrObjCustomShape::BrkDrag( SdrDragStat& rDrag ) const
{
    const SdrHdl* pHdl = rDrag.GetHdl();
    if ( pHdl && ( pHdl->GetKind() == HDL_CUSTOMSHAPE1 ) )
    {
        delete (SdrObjCustomShape*)rDrag.GetUser(); rDrag.SetUser( NULL );
    }
    else
        SdrTextObj::BrkDrag( rDrag );
}

void SdrObjCustomShape::TakeDragPoly(const SdrDragStat& rDrag, XPolyPolygon& rXPP) const
{
    rXPP.Clear();
    FASTBOOL bRet = TRUE;
    const SdrHdl* pHdl = rDrag.GetHdl();
    if ( pHdl && ( pHdl->GetKind() == HDL_CUSTOMSHAPE1 ) )
        GetLineGeometry( rXPP, (SdrObjCustomShape*)rDrag.GetUser(), sal_False );
    else
        SdrTextObj::TakeDragPoly( rDrag, rXPP );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// in context with the SdrObjCustomShape the SdrTextAutoGrowHeightItem == true -> Resize Shape to fit text,
//                                     the SdrTextAutoGrowWidthItem  == true -> Word wrap text in Shape
FASTBOOL SdrObjCustomShape::IsAutoGrowHeight() const
{
    const SfxItemSet& rSet = GetMergedItemSet();
    FASTBOOL bIsAutoGrowHeight = ((SdrTextAutoGrowSizeItem&)(rSet.Get(SDRATTR_TEXT_AUTOGROWSIZE))).GetValue();
    if ( bIsAutoGrowHeight && IsVerticalWriting() )
        bIsAutoGrowHeight = ((SdrTextWordWrapItem&)(rSet.Get(SDRATTR_TEXT_WORDWRAP))).GetValue() == FALSE;
    return bIsAutoGrowHeight;
}
FASTBOOL SdrObjCustomShape::IsAutoGrowWidth() const
{
    const SfxItemSet& rSet = GetMergedItemSet();
    FASTBOOL bIsAutoGrowWidth = ((SdrTextAutoGrowSizeItem&)(rSet.Get(SDRATTR_TEXT_AUTOGROWSIZE))).GetValue();
    if ( bIsAutoGrowWidth && !IsVerticalWriting() )
        bIsAutoGrowWidth = ((SdrTextWordWrapItem&)(rSet.Get(SDRATTR_TEXT_WORDWRAP))).GetValue() == FALSE;
    return bIsAutoGrowWidth;
}

/* The following method is identical to the SdrTextObj::SetVerticalWriting method, the only difference
   is that the SdrAutoGrowWidthItem and SdrAutoGrowHeightItem are not exchanged if the vertical writing
   mode has been changed */

void SdrObjCustomShape::SetVerticalWriting( sal_Bool bVertical )
{
    ForceOutlinerParaObject();

    DBG_ASSERT( pOutlinerParaObject, "SdrTextObj::SetVerticalWriting() without OutlinerParaObject!" );

    if( pOutlinerParaObject )
    {
        if(pOutlinerParaObject->IsVertical() != bVertical)
        {
            // get item settings
            const SfxItemSet& rSet = GetObjectItemSet();

            // #103516# Also exchange hor/ver adjust items
            SdrTextHorzAdjust eHorz = ((SdrTextHorzAdjustItem&)(rSet.Get(SDRATTR_TEXT_HORZADJUST))).GetValue();
            SdrTextVertAdjust eVert = ((SdrTextVertAdjustItem&)(rSet.Get(SDRATTR_TEXT_VERTADJUST))).GetValue();

            // rescue object size
            Rectangle aObjectRect = GetSnapRect();

            // prepare ItemSet to set exchanged width and height items
            SfxItemSet aNewSet(*rSet.GetPool(),
                SDRATTR_TEXT_AUTOGROWHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT,
                // #103516# Expanded item ranges to also support hor and ver adjust.
                SDRATTR_TEXT_VERTADJUST, SDRATTR_TEXT_VERTADJUST,
                SDRATTR_TEXT_AUTOGROWWIDTH, SDRATTR_TEXT_HORZADJUST,
                0, 0);

            aNewSet.Put(rSet);

            // #103516# Exchange horz and vert adjusts
            switch(eVert)
            {
                case SDRTEXTVERTADJUST_TOP: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT)); break;
                case SDRTEXTVERTADJUST_CENTER: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_CENTER)); break;
                case SDRTEXTVERTADJUST_BOTTOM: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_LEFT)); break;
                case SDRTEXTVERTADJUST_BLOCK: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_BLOCK)); break;
            }
            switch(eHorz)
            {
                case SDRTEXTHORZADJUST_LEFT: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_BOTTOM)); break;
                case SDRTEXTHORZADJUST_CENTER: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER)); break;
                case SDRTEXTHORZADJUST_RIGHT: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP)); break;
                case SDRTEXTHORZADJUST_BLOCK: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_BLOCK)); break;
            }

            SetObjectItemSet(aNewSet);

            // set ParaObject orientation accordingly
            pOutlinerParaObject->SetVertical(bVertical);

            // restore object size
            SetSnapRect(aObjectRect);
        }
    }
}
FASTBOOL SdrObjCustomShape::AdjustTextFrameWidthAndHeight(Rectangle& rR, FASTBOOL bHgt, FASTBOOL bWdt) const
{
     if ( pModel && HasText() && !rR.IsEmpty() )
    {
        FASTBOOL bWdtGrow=bWdt && IsAutoGrowWidth();
        FASTBOOL bHgtGrow=bHgt && IsAutoGrowHeight();
        if ( bWdtGrow || bHgtGrow )
        {
            Rectangle aR0(rR);
            long nHgt=0,nMinHgt=0,nMaxHgt=0;
            long nWdt=0,nMinWdt=0,nMaxWdt=0;
            Size aSiz(rR.GetSize()); aSiz.Width()--; aSiz.Height()--;
            Size aMaxSiz(100000,100000);
            Size aTmpSiz(pModel->GetMaxObjSize());
            if (aTmpSiz.Width()!=0) aMaxSiz.Width()=aTmpSiz.Width();
            if (aTmpSiz.Height()!=0) aMaxSiz.Height()=aTmpSiz.Height();
            if (bWdtGrow)
            {
                nMinWdt=GetMinTextFrameWidth();
                nMaxWdt=GetMaxTextFrameWidth();
                if (nMaxWdt==0 || nMaxWdt>aMaxSiz.Width()) nMaxWdt=aMaxSiz.Width();
                if (nMinWdt<=0) nMinWdt=1;
                aSiz.Width()=nMaxWdt;
            }
            if (bHgtGrow)
            {
                nMinHgt=GetMinTextFrameHeight();
                nMaxHgt=GetMaxTextFrameHeight();
                if (nMaxHgt==0 || nMaxHgt>aMaxSiz.Height()) nMaxHgt=aMaxSiz.Height();
                if (nMinHgt<=0) nMinHgt=1;
                aSiz.Height()=nMaxHgt;
            }
            long nHDist=GetTextLeftDistance()+GetTextRightDistance();
            long nVDist=GetTextUpperDistance()+GetTextLowerDistance();
            aSiz.Width()-=nHDist;
            aSiz.Height()-=nVDist;
            if ( aSiz.Width() < 2 )
                aSiz.Width() = 2;   // Mindestgroesse 2
            if ( aSiz.Height() < 2 )
                aSiz.Height() = 2; // Mindestgroesse 2

            if(pEdtOutl)
            {
                pEdtOutl->SetMaxAutoPaperSize( aSiz );
                if (bWdtGrow)
                {
                    Size aSiz(pEdtOutl->CalcTextSize());
                    nWdt=aSiz.Width()+1; // lieber etwas Tolleranz
                    if (bHgtGrow) nHgt=aSiz.Height()+1; // lieber etwas Tolleranz
                } else
                {
                    nHgt=pEdtOutl->GetTextHeight()+1; // lieber etwas Tolleranz
                }
            }
            else
            {
                Outliner& rOutliner=ImpGetDrawOutliner();
                rOutliner.SetPaperSize(aSiz);
                rOutliner.SetUpdateMode(TRUE);
                // !!! hier sollte ich wohl auch noch mal die Optimierung mit
                // bPortionInfoChecked usw einbauen
                if ( pOutlinerParaObject != NULL )
                {
                    rOutliner.SetText(*pOutlinerParaObject);
                    rOutliner.SetFixedCellHeight(((const SdrTextFixedCellHeightItem&)GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT)).GetValue());
                }
                if ( bWdtGrow )
                {
                    Size aSiz(rOutliner.CalcTextSize());
                    nWdt=aSiz.Width()+1; // lieber etwas Tolleranz
                    if ( bHgtGrow )
                        nHgt=aSiz.Height()+1; // lieber etwas Tolleranz
                }
                else
                    nHgt = rOutliner.GetTextHeight()+1; // lieber etwas Tolleranz
                rOutliner.Clear();
            }
            if ( nWdt < nMinWdt )
                nWdt = nMinWdt;
            if ( nWdt > nMaxWdt )
                nWdt = nMaxWdt;
            nWdt += nHDist;
            if ( nWdt < 1 )
                nWdt = 1; // nHDist kann auch negativ sein
            if ( nHgt < nMinHgt )
                nHgt = nMinHgt;
            if ( nHgt > nMaxHgt )
                nHgt = nMaxHgt;
            nHgt+=nVDist;
            if ( nHgt < 1 )
                nHgt = 1; // nVDist kann auch negativ sein
            long nWdtGrow = nWdt-(rR.Right()-rR.Left());
            long nHgtGrow = nHgt-(rR.Bottom()-rR.Top());
            if ( nWdtGrow == 0 )
                bWdtGrow = FALSE;
            if ( nHgtGrow == 0 )
                bHgtGrow=FALSE;
            if ( bWdtGrow || bHgtGrow )
            {
                if ( bWdtGrow )
                {
                    SdrTextHorzAdjust eHAdj=GetTextHorizontalAdjust();
                    if ( eHAdj == SDRTEXTHORZADJUST_LEFT )
                        rR.Right()+=nWdtGrow;
                    else if ( eHAdj == SDRTEXTHORZADJUST_RIGHT )
                        rR.Left()-=nWdtGrow;
                    else
                    {
                        long nWdtGrow2=nWdtGrow/2;
                        rR.Left()-=nWdtGrow2;
                        rR.Right()=rR.Left()+nWdt;
                    }
                }
                if ( bHgtGrow )
                {
                    SdrTextVertAdjust eVAdj=GetTextVerticalAdjust();
                    if ( eVAdj == SDRTEXTVERTADJUST_TOP )
                        rR.Bottom()+=nHgtGrow;
                    else if ( eVAdj == SDRTEXTVERTADJUST_BOTTOM )
                        rR.Top()-=nHgtGrow;
                    else
                    {
                        long nHgtGrow2=nHgtGrow/2;
                        rR.Top()-=nHgtGrow2;
                        rR.Bottom()=rR.Top()+nHgt;
                    }
                }
                if ( aGeo.nDrehWink )
                {
                    Point aD1(rR.TopLeft());
                    aD1-=aR0.TopLeft();
                    Point aD2(aD1);
                    RotatePoint(aD2,Point(),aGeo.nSin,aGeo.nCos);
                    aD2-=aD1;
                    rR.Move(aD2.X(),aD2.Y());
                }
                return TRUE;
            }
        }
    }
    return FALSE;
}
FASTBOOL SdrObjCustomShape::NbcAdjustTextFrameWidthAndHeight(FASTBOOL bHgt, FASTBOOL bWdt)
{
    Rectangle aNewTextRect, aOldTextRect( aRect );
    GetTextBounds( aOldTextRect );
    aNewTextRect = aOldTextRect;
    FASTBOOL bRet = AdjustTextFrameWidthAndHeight( aNewTextRect, bHgt, bWdt );
    if ( aOldTextRect != aNewTextRect )
    {
        if ( bRet )
        {
            // calculating the new text rect
            double fXScale = (double)aRect.GetWidth() / (double)aOldTextRect.GetWidth();
            double fYScale = (double)aRect.GetHeight() / (double)aOldTextRect.GetHeight();
            double fRightDiff = (double)( aNewTextRect.Right() - aOldTextRect.Right() ) * fXScale;
            double fLeftDiff  = (double)( aNewTextRect.Left()  - aOldTextRect.Left()  ) * fXScale;
            double fTopDiff   = (double)( aNewTextRect.Top()   - aOldTextRect.Top()   ) * fYScale;
            double fBottomDiff= (double)( aNewTextRect.Bottom()- aOldTextRect.Bottom()) * fYScale;
            aRect.Left() += (sal_Int32)fLeftDiff;
            aRect.Right() += (sal_Int32)fRightDiff;
            aRect.Top() += (sal_Int32)fTopDiff;
            aRect.Bottom() += (sal_Int32)fBottomDiff;

            SetRectsDirty();
            SetChanged();
            delete pRenderedCustomShape, pRenderedCustomShape = NULL;
        }
    }
    return bRet;
}
FASTBOOL SdrObjCustomShape::AdjustTextFrameWidthAndHeight(FASTBOOL bHgt, FASTBOOL bWdt)
{
    Rectangle aNewTextRect, aOldTextRect( aRect );
    GetTextBounds( aOldTextRect );
    aNewTextRect = aOldTextRect;
    FASTBOOL bRet = AdjustTextFrameWidthAndHeight( aNewTextRect, bHgt, bWdt );
    if ( aOldTextRect != aNewTextRect )
    {
        if ( bRet )
        {
            Rectangle aBoundRect0;
            if ( pUserCall )
                aBoundRect0 = GetCurrentBoundRect();
    //      SendRepaintBroadcast();

            // calculating the new text rect
            double fXScale = (double)aRect.GetWidth() / (double)aOldTextRect.GetWidth();
            double fYScale = (double)aRect.GetHeight() / (double)aOldTextRect.GetHeight();
            double fRightDiff = (double)( aNewTextRect.Right() - aOldTextRect.Right() ) * fXScale;
            double fLeftDiff  = (double)( aNewTextRect.Left()  - aOldTextRect.Left()  ) * fXScale;
            double fTopDiff   = (double)( aNewTextRect.Top()   - aOldTextRect.Top()   ) * fYScale;
            double fBottomDiff= (double)( aNewTextRect.Bottom()- aOldTextRect.Bottom()) * fYScale;
            aRect.Left() += (sal_Int32)fLeftDiff;
            aRect.Right() += (sal_Int32)fRightDiff;
            aRect.Top() += (sal_Int32)fTopDiff;
            aRect.Bottom() += (sal_Int32)fBottomDiff;

            SetRectsDirty();
            delete pRenderedCustomShape, pRenderedCustomShape = NULL;
            SetChanged();
    //      SendRepaintBroadcast();
            BroadcastObjectChange();
            SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
        }
    }
    return bRet;
}
FASTBOOL SdrObjCustomShape::BegTextEdit( SdrOutliner& rOutl )
{
    return SdrTextObj::BegTextEdit( rOutl );
}
void SdrObjCustomShape::TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, Rectangle* pViewInit, Rectangle* pViewMin) const
{
    Size aPaperMin,aPaperMax;
    Rectangle aViewInit;
    TakeTextAnchorRect( aViewInit );
    if ( aGeo.nDrehWink )
    {
        Point aCenter(aViewInit.Center());
        aCenter-=aViewInit.TopLeft();
        Point aCenter0(aCenter);
        RotatePoint(aCenter,Point(),aGeo.nSin,aGeo.nCos);
        aCenter-=aCenter0;
        aViewInit.Move(aCenter.X(),aCenter.Y());
    }
    Size aAnkSiz(aViewInit.GetSize());
    aAnkSiz.Width()--; aAnkSiz.Height()--; // weil GetSize() ein draufaddiert
    Size aMaxSiz(1000000,1000000);
    if (pModel!=NULL) {
        Size aTmpSiz(pModel->GetMaxObjSize());
        if (aTmpSiz.Width()!=0) aMaxSiz.Width()=aTmpSiz.Width();
        if (aTmpSiz.Height()!=0) aMaxSiz.Height()=aTmpSiz.Height();
    }
    SdrTextHorzAdjust eHAdj(GetTextHorizontalAdjust());
    SdrTextVertAdjust eVAdj(GetTextVerticalAdjust());

    long nMinWdt = GetMinTextFrameWidth();
    long nMinHgt = GetMinTextFrameHeight();
    long nMaxWdt = GetMaxTextFrameWidth();
    long nMaxHgt = GetMaxTextFrameHeight();
    if (nMinWdt<1) nMinWdt=1;
    if (nMinHgt<1) nMinHgt=1;
    if ( nMaxWdt == 0 || nMaxWdt > aMaxSiz.Width() )
        nMaxWdt = aMaxSiz.Width();
    if ( nMaxHgt == 0 || nMaxHgt > aMaxSiz.Height() )
        nMaxHgt=aMaxSiz.Height();

    if (((SdrTextWordWrapItem&)(GetMergedItem(SDRATTR_TEXT_WORDWRAP))).GetValue())
    {
        if ( IsVerticalWriting() )
        {
            nMaxHgt = aAnkSiz.Height();
            nMinHgt = nMaxHgt;
        }
        else
        {
            nMaxWdt = aAnkSiz.Width();
            nMinWdt = nMaxWdt;
        }
    }
//      if (!((SdrTextAutoGrowHeightItem&)(GetItem(SDRATTR_TEXT_AUTOGROWHEIGHT))).GetValue())
//      {
//          nMaxHgt = aAnkSiz.Height();
//          nMinHgt = nMaxHgt;
//      }
    aPaperMax.Width()=nMaxWdt;
    aPaperMax.Height()=nMaxHgt;

    aPaperMin.Width()=nMinWdt;
    aPaperMin.Height()=nMinHgt;

    if ( pViewMin )
    {
        *pViewMin = aViewInit;

        long nXFree = aAnkSiz.Width() - aPaperMin.Width();
        if ( eHAdj == SDRTEXTHORZADJUST_LEFT )
            pViewMin->Right() -= nXFree;
        else if ( eHAdj == SDRTEXTHORZADJUST_RIGHT )
            pViewMin->Left() += nXFree;
        else { pViewMin->Left() += nXFree / 2; pViewMin->Right() = pViewMin->Left() + aPaperMin.Width(); }

        long nYFree = aAnkSiz.Height() - aPaperMin.Height();
        if ( eVAdj == SDRTEXTVERTADJUST_TOP )
            pViewMin->Bottom() -= nYFree;
        else if ( eVAdj == SDRTEXTVERTADJUST_BOTTOM )
            pViewMin->Top() += nYFree;
        else { pViewMin->Top() += nYFree / 2; pViewMin->Bottom() = pViewMin->Top() + aPaperMin.Height(); }
    }

    if( IsVerticalWriting() )
        aPaperMin.Width() = 0;
    else
        aPaperMin.Height() = 0; // #33102#

    if( eHAdj != SDRTEXTHORZADJUST_BLOCK )
        aPaperMin.Width()=0;

    // #103516# For complete ver adjust support, set paper min height to 0, here.
    if(SDRTEXTHORZADJUST_BLOCK != eVAdj )
        aPaperMin.Height() = 0;

    if (pPaperMin!=NULL) *pPaperMin=aPaperMin;
    if (pPaperMax!=NULL) *pPaperMax=aPaperMax;
    if (pViewInit!=NULL) *pViewInit=aViewInit;
}
void SdrObjCustomShape::EndTextEdit( SdrOutliner& rOutl )
{
    SdrTextObj::EndTextEdit( rOutl );
    delete pRenderedCustomShape, pRenderedCustomShape = NULL;
}
void SdrObjCustomShape::TakeTextAnchorRect( Rectangle& rAnchorRect ) const
{
    if ( GetTextBounds( rAnchorRect ) )
    {
        Point aRotateRef( maSnapRect.Center() );
        rAnchorRect.Left()   += GetTextLeftDistance();
        rAnchorRect.Top()    += GetTextUpperDistance();
        rAnchorRect.Right()  -= GetTextRightDistance();
        rAnchorRect.Bottom() -= GetTextLowerDistance();
        ImpJustifyRect( rAnchorRect );

        if ( rAnchorRect.GetWidth() < 2 )
            rAnchorRect.Right() = rAnchorRect.Left() + 1;   // minimal width is 2
        if ( rAnchorRect.GetHeight() < 2 )
            rAnchorRect.Bottom() = rAnchorRect.Top() + 1;   // minimal height is 2
        if ( aGeo.nDrehWink )
        {
            Point aP( rAnchorRect.TopLeft() );
            RotatePoint( aP, aRotateRef, aGeo.nSin, aGeo. nCos );
            rAnchorRect.SetPos( aP );
        }
    }
    else
        SdrTextObj::TakeTextAnchorRect( rAnchorRect );
}
void SdrObjCustomShape::TakeTextRect( SdrOutliner& rOutliner, Rectangle& rTextRect, FASTBOOL bNoEditText,
                               Rectangle* pAnchorRect, BOOL bLineWidth ) const
{
    Rectangle aAnkRect; // Rect innerhalb dem geankert wird
    TakeTextAnchorRect(aAnkRect);
    SdrTextVertAdjust eVAdj=GetTextVerticalAdjust();
    SdrTextHorzAdjust eHAdj=GetTextHorizontalAdjust();
    ULONG nStat0=rOutliner.GetControlWord();
    Size aNullSize;

    rOutliner.SetControlWord(nStat0|EE_CNTRL_AUTOPAGESIZE);
    rOutliner.SetMinAutoPaperSize(aNullSize);
    sal_Int32 nMaxAutoPaperWidth = 1000000;
    sal_Int32 nMaxAutoPaperHeight= 1000000;

    long nAnkWdt=aAnkRect.GetWidth();
    long nAnkHgt=aAnkRect.GetHeight();

    if (((SdrTextWordWrapItem&)(GetMergedItem(SDRATTR_TEXT_WORDWRAP))).GetValue())
    {
        if ( IsVerticalWriting() )
            nMaxAutoPaperHeight = nAnkHgt;
        else
            nMaxAutoPaperWidth = nAnkWdt;
    }
//      if (!((SdrTextAutoGrowHeightItem&)(GetItem(SDRATTR_TEXT_AUTOGROWHEIGHT))).GetValue())
//          nMaxAutoPaperHeight = nAnkHgt;
    if(SDRTEXTHORZADJUST_BLOCK == eHAdj && !IsVerticalWriting())
    {
        rOutliner.SetMinAutoPaperSize(Size(nAnkWdt, 0));
    }

    if(SDRTEXTVERTADJUST_BLOCK == eVAdj && IsVerticalWriting())
    {
        rOutliner.SetMinAutoPaperSize(Size(0, nAnkHgt));
    }
    rOutliner.SetMaxAutoPaperSize( Size( nMaxAutoPaperWidth, nMaxAutoPaperHeight ) );
    rOutliner.SetPaperSize( aNullSize );

    // Text in den Outliner stecken - ggf. den aus dem EditOutliner
    OutlinerParaObject* pPara=pOutlinerParaObject;
    if (pEdtOutl && !bNoEditText)
        pPara=pEdtOutl->CreateParaObject();

    if (pPara)
    {
        BOOL bHitTest = FALSE;
        if( pModel )
            bHitTest = &pModel->GetHitTestOutliner() == &rOutliner;

        const SdrTextObj* pTestObj = rOutliner.GetTextObj();
        if( !pTestObj || !bHitTest || pTestObj != this ||
            pTestObj->GetOutlinerParaObject() != pOutlinerParaObject )
        {
            if( bHitTest )
                rOutliner.SetTextObj( this );

            rOutliner.SetUpdateMode(TRUE);
            rOutliner.SetText(*pPara);
        }
    }
    else
    {
        rOutliner.SetTextObj( NULL );
    }
    if (pEdtOutl && !bNoEditText && pPara)
        delete pPara;

    rOutliner.SetUpdateMode(TRUE);
    rOutliner.SetControlWord(nStat0);

    if (!bPortionInfoChecked)
    {
        // Optimierung: ggf. BigTextObject erzeugen
        ((SdrObjCustomShape*)this)->bPortionInfoChecked=TRUE;
        if (pOutlinerParaObject!=NULL && rOutliner.ShouldCreateBigTextObject())
            ((SdrObjCustomShape*)this)->pOutlinerParaObject=rOutliner.CreateParaObject();
    }

    Point aTextPos(aAnkRect.TopLeft());
    Size aTextSiz(rOutliner.GetPaperSize()); // GetPaperSize() hat etwas Toleranz drauf, oder?

    // #106653#
    // For draw objects containing text correct hor/ver alignment if text is bigger
    // than the object itself. Without that correction, the text would always be
        // formatted to the left edge (or top edge when vertical) of the draw object.

    if( !IsTextFrame() )
    {
        if(aAnkRect.GetWidth() < aTextSiz.Width() && !IsVerticalWriting())
        {
            // #110129#
            // Horizontal case here. Correct only if eHAdj == SDRTEXTHORZADJUST_BLOCK,
            // else the alignment is wanted.
            if(SDRTEXTHORZADJUST_BLOCK == eHAdj)
            {
                eHAdj = SDRTEXTHORZADJUST_CENTER;
            }
        }

        if(aAnkRect.GetHeight() < aTextSiz.Height() && IsVerticalWriting())
        {
            // #110129#
            // Vertical case here. Correct only if eHAdj == SDRTEXTVERTADJUST_BLOCK,
            // else the alignment is wanted.
            if(SDRTEXTVERTADJUST_BLOCK == eVAdj)
            {
                eVAdj = SDRTEXTVERTADJUST_CENTER;
            }
        }
    }

    if (eHAdj==SDRTEXTHORZADJUST_CENTER || eHAdj==SDRTEXTHORZADJUST_RIGHT)
    {
        long nFreeWdt=aAnkRect.GetWidth()-aTextSiz.Width();
        if (eHAdj==SDRTEXTHORZADJUST_CENTER)
            aTextPos.X()+=nFreeWdt/2;
        if (eHAdj==SDRTEXTHORZADJUST_RIGHT)
            aTextPos.X()+=nFreeWdt;
    }
    if (eVAdj==SDRTEXTVERTADJUST_CENTER || eVAdj==SDRTEXTVERTADJUST_BOTTOM)
    {
        long nFreeHgt=aAnkRect.GetHeight()-aTextSiz.Height();
        if (eVAdj==SDRTEXTVERTADJUST_CENTER)
            aTextPos.Y()+=nFreeHgt/2;
        if (eVAdj==SDRTEXTVERTADJUST_BOTTOM)
            aTextPos.Y()+=nFreeHgt;
    }
    if (aGeo.nDrehWink!=0)
        RotatePoint(aTextPos,aAnkRect.TopLeft(),aGeo.nSin,aGeo.nCos);

    if (pAnchorRect)
        *pAnchorRect=aAnkRect;

    // rTextRect ist bei ContourFrame in einigen Faellen nicht korrekt
    rTextRect=Rectangle(aTextPos,aTextSiz);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrObjCustomShape::NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject)
{
    SdrTextObj::NbcSetOutlinerParaObject( pTextObject );
    bBoundRectDirty = TRUE;
    SetRectsDirty(TRUE);
    delete pRenderedCustomShape, pRenderedCustomShape = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObject* SdrObjCustomShape::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    SdrObject* pHitObj = NULL;
    const SdrObject* pSdrObject = GetSdrObjectFromCustomShape();
    if ( pSdrObject )
    {
        SdrObjList* pOL = pSdrObject->GetSubList();
        if ( pOL )
        {
            ULONG nObjNum = pOL->GetObjCount();
            while ( ( pHitObj == NULL ) && nObjNum )
            {
                nObjNum--;
                SdrObject* pObj = pOL->GetObj( nObjNum );
                if ( pObj->CheckHit( rPnt, nTol, pVisiLayer ) )
                    pHitObj = (SdrObject*)this;
            }
            if ( !pHitObj && HasText() && SdrTextObj::CheckHit( rPnt, nTol, pVisiLayer ) )
                pHitObj = (SdrObject*)this;
        }
        else
        {
            if ( pSdrObject->CheckHit( rPnt, nTol, pVisiLayer ) )
                pHitObj = (SdrObject*)this;
            else if ( SdrTextObj::CheckHit( rPnt, nTol, pVisiLayer ) )
                pHitObj = (SdrObject*)this;
        }
    }
    return pHitObj;
}

void SdrObjCustomShape::operator=(const SdrObject& rObj)
{
    SdrTextObj::operator=( rObj );
    aName =((SdrObjCustomShape&)rObj).aName;
    fObjectRotation = ((SdrObjCustomShape&)rObj).fObjectRotation;
}


void SdrObjCustomShape::TakeObjNameSingul(XubString& rName) const
{
    rName = ImpGetResStr(STR_ObjNameSingulCUSTOMSHAPE);
    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

void SdrObjCustomShape::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralCUSTOMSHAPE);
}

void SdrObjCustomShape::TakeXorPoly(XPolyPolygon& rPoly, FASTBOOL bDetail) const
{
    GetLineGeometry( rPoly, (SdrObjCustomShape*)this, sal_False );
}

void SdrObjCustomShape::TakeContour(XPolyPolygon& rXPoly ) const
{
    const SdrObject* pSdrObject = GetSdrObjectFromCustomShape();
    if ( pSdrObject )
        pSdrObject->TakeContour( rXPoly );
}

SdrObject* SdrObjCustomShape::DoConvertToPolyObj(BOOL bBezier) const
{
    SdrObject* pGroup = new SdrObjCustomShape;
    pGroup->SetModel(GetModel());
    return pGroup;
}

void SdrObjCustomShape::SetPage( SdrPage* pNewPage )
{
    SdrTextObj::SetPage( pNewPage );

/*  invalidating rectangles by SetRectsDirty is not sufficient,
    AdjustTextFrameWidthAndHeight() also has to be made, both
    actions are done by NbcSetSnapRect !  */
//  SetRectsDirty();
    NbcSetSnapRect( aRect );
}

SdrObjGeoData* SdrObjCustomShape::NewGeoData() const
{
    return new SdrAShapeObjGeoData;
}

void SdrObjCustomShape::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrTextObj::SaveGeoData( rGeo );
    SdrAShapeObjGeoData& rAGeo=(SdrAShapeObjGeoData&)rGeo;
    rAGeo.fObjectRotation = fObjectRotation;
    rAGeo.bMirroredX = IsMirroredX();
    rAGeo.bMirroredY = IsMirroredY();
}

void SdrObjCustomShape::RestGeoData(const SdrObjGeoData& rGeo)
{
    SdrTextObj::RestGeoData( rGeo );
    SdrAShapeObjGeoData& rAGeo=(SdrAShapeObjGeoData&)rGeo;
    fObjectRotation = rAGeo.fObjectRotation;
    SetMirroredX( rAGeo.bMirroredX );
    SetMirroredY( rAGeo.bMirroredY );
    delete pRenderedCustomShape, pRenderedCustomShape = NULL;
}

void SdrObjCustomShape::TRSetBaseGeometry(const Matrix3D& rMat, const XPolyPolygon& rPolyPolygon)
{
    // break up matrix
    Vector2D aScale, aTranslate;
    double fShear, fRotate;
    rMat.DecomposeAndCorrect(aScale, fShear, fRotate, aTranslate);

    // reset object shear and rotations
    aGeo.nDrehWink = 0;
    aGeo.RecalcSinCos();
    aGeo.nShearWink = 0;
    aGeo.RecalcTan();

    // force metric to pool metric
    SfxMapUnit eMapUnit = pModel->GetItemPool().GetMetric(0);
    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        switch(eMapUnit)
        {
            case SFX_MAPUNIT_TWIP :
            {
                // position
                // #104018#
                aTranslate.X() = ImplMMToTwips(aTranslate.X());
                aTranslate.Y() = ImplMMToTwips(aTranslate.Y());

                // size
                // #104018#
                aScale.X() = ImplMMToTwips(aScale.X());
                aScale.Y() = ImplMMToTwips(aScale.Y());

                break;
            }
            default:
            {
                DBG_ERROR("TRSetBaseGeometry: Missing unit translation to PoolMetric!");
            }
        }
    }

    // if anchor is used, make position relative to it
    if( pModel->IsWriter() )
    {
        if(GetAnchorPos().X() != 0 || GetAnchorPos().Y() != 0)
            aTranslate += Vector2D(GetAnchorPos().X(), GetAnchorPos().Y());
    }

    // build and set BaseRect (use scale)
    Point aPoint = Point();
    Size  aSize(FRound(aScale.X()), FRound(aScale.Y()));
    Rectangle aBaseRect(aPoint, aSize);
    SetSnapRect(aBaseRect);

    // shear?
    if(fShear != 0.0)
    {
        GeoStat aGeoStat;
        aGeoStat.nShearWink = FRound((atan(fShear) / F_PI180) * 100.0);
        aGeoStat.RecalcTan();
        Shear(Point(), aGeoStat.nShearWink, aGeoStat.nTan, FALSE);
    }

    // rotation?
    if(fRotate != 0.0)
    {
        GeoStat aGeoStat;
        aGeoStat.nDrehWink = FRound((fRotate / F_PI180) * 100.0);
        aGeoStat.RecalcSinCos();
        Rotate(Point(), aGeoStat.nDrehWink, aGeoStat.nSin, aGeoStat.nCos);
    }

    // translate?
    if(aTranslate.X() != 0.0 || aTranslate.Y() != 0.0)
    {
        Move(Size(
            (sal_Int32)FRound(aTranslate.X()),
            (sal_Int32)FRound(aTranslate.Y())));
    }
}

// taking fObjectRotation instead of aGeo.nWink
BOOL SdrObjCustomShape::TRGetBaseGeometry(Matrix3D& rMat, XPolyPolygon& rPolyPolygon) const
{
    // get turn and shear
//  double fRotate = (aGeo.nDrehWink / 100.0) * F_PI180;

    double fRotate = fObjectRotation * F_PI180;
    double fShear = (aGeo.nShearWink / 100.0) * F_PI180;

    // get aRect, this is the unrotated snaprect
    Rectangle aRectangle(aRect);

    sal_Bool bMirroredX = IsMirroredX();
    sal_Bool bMirroredY = IsMirroredY();
    if ( bMirroredX || bMirroredY )
    {   // we have to retrieve the unmirrored rect

        GeoStat aNewGeo( aGeo );

        if ( bMirroredX )
        {
            Polygon aPol( Rect2Poly( aRect, aNewGeo ) );
            Rectangle aBoundRect( aPol.GetBoundRect() );

            Point aRef1( ( aBoundRect.Left() + aBoundRect.Right() ) >> 1, aBoundRect.Top() );
            Point aRef2( aRef1.X(), aRef1.Y() + 1000 );
            USHORT i;
            USHORT nPntAnz=aPol.GetSize();
            for (i=0; i<nPntAnz; i++)
            {
                MirrorPoint(aPol[i],aRef1,aRef2);
            }
            // Polygon wenden und etwas schieben
            Polygon aPol0(aPol);
            aPol[0]=aPol0[1];
            aPol[1]=aPol0[0];
            aPol[2]=aPol0[3];
            aPol[3]=aPol0[2];
            aPol[4]=aPol0[1];
            Poly2Rect(aPol,aRectangle,aNewGeo);
        }
        if ( bMirroredY )
        {
            Polygon aPol( Rect2Poly( aRectangle, aNewGeo ) );
            Rectangle aBoundRect( aPol.GetBoundRect() );

            Point aRef1( aBoundRect.Left(), ( aBoundRect.Top() + aBoundRect.Bottom() ) >> 1 );
            Point aRef2( aRef1.X() + 1000, aRef1.Y() );
            USHORT i;
            USHORT nPntAnz=aPol.GetSize();
            for (i=0; i<nPntAnz; i++)
            {
                MirrorPoint(aPol[i],aRef1,aRef2);
            }
            // Polygon wenden und etwas schieben
            Polygon aPol0(aPol);
            aPol[0]=aPol0[1];
            aPol[1]=aPol0[0];
            aPol[2]=aPol0[3];
            aPol[3]=aPol0[2];
            aPol[4]=aPol0[1];
            Poly2Rect(aPol,aRectangle,aNewGeo);
        }
    }

    // fill other values
    Vector2D aScale((double)aRectangle.GetWidth(), (double)aRectangle.GetHeight());
    Vector2D aTranslate((double)aRectangle.Left(), (double)aRectangle.Top());

    // position maybe relative to anchorpos, convert
    if( pModel->IsWriter() )
    {
        if(GetAnchorPos().X() != 0 || GetAnchorPos().Y() != 0)
            aTranslate -= Vector2D(GetAnchorPos().X(), GetAnchorPos().Y());
    }

    // force MapUnit to 100th mm
    SfxMapUnit eMapUnit = pModel->GetItemPool().GetMetric(0);
    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        switch(eMapUnit)
        {
            case SFX_MAPUNIT_TWIP :
            {
                // position
                // #104018#
                aTranslate.X() = ImplTwipsToMM(aTranslate.X());
                aTranslate.Y() = ImplTwipsToMM(aTranslate.Y());

                // size
                // #104018#
                aScale.X() = ImplTwipsToMM(aScale.X());
                aScale.Y() = ImplTwipsToMM(aScale.Y());

                break;
            }
            default:
            {
                DBG_ERROR("TRGetBaseGeometry: Missing unit translation to 100th mm!");
            }
        }
    }

    // build matrix
    rMat.Identity();
    if(aScale.X() != 1.0 || aScale.Y() != 1.0)
        rMat.Scale(aScale.X(), aScale.Y());
    if(fShear != 0.0)
        rMat.ShearX(tan(fShear));
    if(fRotate != 0.0)
        rMat.Rotate(fRotate);
    if(aTranslate.X() != 0.0 || aTranslate.Y() != 0.0)
        rMat.Translate(aTranslate.X(), aTranslate.Y());
    return FALSE;
}

void SdrObjCustomShape::WriteData(SvStream& rOut) const
{
}

void SdrObjCustomShape::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
}

sdr::contact::ViewContact* SdrObjCustomShape::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrObjCustomShape(*this);
}

