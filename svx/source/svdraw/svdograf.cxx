/*************************************************************************
 *
 *  $RCSfile: svdograf.cxx,v $
 *
 *  $Revision: 1.62 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 13:21:42 $
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

#define _ANIMATION
#define ITEMID_GRF_CROP 0

#include <so3/lnkbase.hxx>
#include <math.h>
#include <vcl/salbtype.hxx>
#include <sot/formats.hxx>
#include <so3/svstor.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <svtools/style.hxx>
#include <svtools/filter.hxx>
#include <svtools/urihelper.hxx>
#include <goodies/grfmgr.hxx>
#include "linkmgr.hxx"
#include "svdxout.hxx"
#include "svdio.hxx"
#include "svdetc.hxx"
#include "svdglob.hxx"
#include "svdstr.hrc"
#include "svdpool.hxx"
#include "svdmodel.hxx"
#include "svdpage.hxx"
#include "svdmrkv.hxx"
#include "svdpagv.hxx"
#include "svdviter.hxx"
#include "svdview.hxx"
#include "impgrf.hxx"
#include "svdograf.hxx"
#include "svdogrp.hxx"
#include "xbitmap.hxx"
#include "xbtmpit.hxx"
#include "xflbmtit.hxx"
#include "svdundo.hxx"
#include "svdfmtf.hxx"
#include "sdgcpitm.hxx"

#ifndef _EEITEM_HXX
#include "eeitem.hxx"
#endif

#ifndef _SDR_PROPERTIES_GRAPHICPROPERTIES_HXX
#include <svx/sdr/properties/graphicproperties.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACTOFGRAPHIC_HXX
#include <svx/sdr/contact/viewcontactofgraphic.hxx>
#endif

// -----------
// - Defines -
// -----------

#define GRAFSTREAMPOS_INVALID 0xffffffff

// ------------------
// - SdrGraphicLink -
// ------------------

class SdrGraphicLink : public so3::SvBaseLink
{
    SdrGrafObj*         pGrafObj;

public:
                        SdrGraphicLink(SdrGrafObj* pObj);
    virtual             ~SdrGraphicLink();

    virtual void        Closed();
    virtual void        DataChanged( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue );

    BOOL                Connect() { return 0 != GetRealObject(); }
    void                UpdateSynchron();
};

// -----------------------------------------------------------------------------

SdrGraphicLink::SdrGraphicLink(SdrGrafObj* pObj):
    ::so3::SvBaseLink( ::so3::LINKUPDATE_ONCALL, SOT_FORMATSTR_ID_SVXB ),
    pGrafObj(pObj)
{
    SetSynchron( FALSE );
}

// -----------------------------------------------------------------------------

SdrGraphicLink::~SdrGraphicLink()
{
}

// -----------------------------------------------------------------------------

void SdrGraphicLink::DataChanged( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue )
{
    SdrModel*       pModel      = pGrafObj ? pGrafObj->GetModel() : 0;
    SvxLinkManager* pLinkManager= pModel  ? pModel->GetLinkManager() : 0;

    if( pLinkManager && rValue.hasValue() )
    {
        pLinkManager->GetDisplayNames( this, 0, &pGrafObj->aFileName, 0, &pGrafObj->aFilterName );

        Graphic aGraphic;
        if( SvxLinkManager::GetGraphicFromAny( rMimeType, rValue, aGraphic ))
        {
            GraphicType eOldGraphicType = pGrafObj->GetGraphicType();  // kein Hereinswappen
            const sal_Bool bIsChanged = pModel->IsChanged();

            pGrafObj->SetGraphic( aGraphic );
            if( GRAPHIC_NONE != eOldGraphicType )
                pGrafObj->SetChanged();
            else
                pModel->SetChanged( bIsChanged );
        }
        else if( SotExchange::GetFormatIdFromMimeType( rMimeType ) !=
                    SvxLinkManager::RegisterStatusInfoId() )
        {
            // only repaint, no objectchange
            pGrafObj->ActionChanged();
            // pGrafObj->BroadcastObjectChange();
        }
    }
}

// -----------------------------------------------------------------------------

void SdrGraphicLink::Closed()
{
    // Die Verbindung wird aufgehoben; pLink des Objekts auf NULL setzen, da die Link-Instanz ja gerade destruiert wird.
    pGrafObj->ForceSwapIn();
    pGrafObj->pGraphicLink=NULL;
    pGrafObj->ReleaseGraphicLink();
    SvBaseLink::Closed();
}

// -----------------------------------------------------------------------------

void SdrGraphicLink::UpdateSynchron()
{
    if( GetObj() )
    {
        String aMimeType( SotExchange::GetFormatMimeType( GetContentType() ));
        ::com::sun::star::uno::Any aValue;
        GetObj()->GetData( aValue, aMimeType, TRUE );
        DataChanged( aMimeType, aValue );
    }
}

// --------------
// - SdrGrafObj -
// --------------

//////////////////////////////////////////////////////////////////////////////
// BaseProperties section

sdr::properties::BaseProperties* SdrGrafObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::GraphicProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* SdrGrafObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfGraphic(*this);
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrGrafObj,SdrRectObj);

// -----------------------------------------------------------------------------

SdrGrafObj::SdrGrafObj():
    bMirrored       ( FALSE ),
    pGraphicLink    ( NULL )
{
    pGraphic = new GraphicObject;
    pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), 20000 );
    nGrafStreamPos = GRAFSTREAMPOS_INVALID;
    bNoShear = TRUE;
    bCopyToPoolOnAfterRead = FALSE;

    // #111096#
    mbGrafAnimationAllowed = sal_True;

    // #i25616#
    mbLineIsOutsideGeometry = sal_True;
    mbInsidePaint = sal_False;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = sal_False;
}

// -----------------------------------------------------------------------------

SdrGrafObj::SdrGrafObj(const Graphic& rGrf, const Rectangle& rRect):
    SdrRectObj      ( rRect ),
    bMirrored       ( FALSE ),
    pGraphicLink    ( NULL )
{
    pGraphic = new GraphicObject( rGrf );
    pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), 20000 );
    nGrafStreamPos = GRAFSTREAMPOS_INVALID;
    bNoShear = TRUE;
    bCopyToPoolOnAfterRead = FALSE;

    // #111096#
    mbGrafAnimationAllowed = sal_True;

    // #i25616#
    mbLineIsOutsideGeometry = sal_True;
    mbInsidePaint = sal_False;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = sal_False;
}

// -----------------------------------------------------------------------------

SdrGrafObj::SdrGrafObj( const Graphic& rGrf ):
    bMirrored       ( FALSE ),
    pGraphicLink    ( NULL )
{
    pGraphic = new GraphicObject( rGrf );
    pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), 20000 );
    nGrafStreamPos = GRAFSTREAMPOS_INVALID;
    bNoShear = TRUE;
    bCopyToPoolOnAfterRead = FALSE;

    // #111096#
    mbGrafAnimationAllowed = sal_True;

    // #i25616#
    mbLineIsOutsideGeometry = sal_True;
    mbInsidePaint = sal_False;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = sal_False;
}

// -----------------------------------------------------------------------------

SdrGrafObj::~SdrGrafObj()
{
    delete pGraphic;
    ImpLinkAbmeldung();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetGraphicObject( const GraphicObject& rGrfObj )
{
    *pGraphic = rGrfObj;
    pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), 20000 );
    pGraphic->SetUserData();
    nGrafStreamPos = GRAFSTREAMPOS_INVALID;
    SetChanged();
    BroadcastObjectChange();
}

// -----------------------------------------------------------------------------

const GraphicObject& SdrGrafObj::GetGraphicObject() const
{
    ForceSwapIn();
    return *pGraphic;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetGraphic( const Graphic& rGrf )
{
    pGraphic->SetGraphic( rGrf );
    pGraphic->SetUserData();
    nGrafStreamPos = GRAFSTREAMPOS_INVALID;
    SetChanged();
    BroadcastObjectChange();
}

// -----------------------------------------------------------------------------

const Graphic& SdrGrafObj::GetGraphic() const
{
    ForceSwapIn();
    return pGraphic->GetGraphic();
}

// -----------------------------------------------------------------------------

Graphic SdrGrafObj::GetTransformedGraphic( ULONG nTransformFlags ) const
{
    // #107947# Refactored most of the code to GraphicObject, where
    // everybody can use e.g. the cropping functionality

    GraphicType     eType = GetGraphicType();
    MapMode         aDestMap( pModel->GetScaleUnit(), Point(), pModel->GetScaleFraction(), pModel->GetScaleFraction() );
    const Size      aDestSize( GetLogicRect().GetSize() );
    const BOOL      bMirror = ( nTransformFlags & SDRGRAFOBJ_TRANSFORMATTR_MIRROR ) != 0;
    const BOOL      bRotate = ( ( nTransformFlags & SDRGRAFOBJ_TRANSFORMATTR_ROTATE ) != 0 ) &&
        ( aGeo.nDrehWink && aGeo.nDrehWink != 18000 ) && ( GRAPHIC_NONE != eType );

    // #104115# Need cropping info earlier
    ( (SdrGrafObj*) this )->ImpSetAttrToGrafInfo();
    GraphicAttr aActAttr;

    if( SDRGRAFOBJ_TRANSFORMATTR_NONE != nTransformFlags &&
        GRAPHIC_NONE != eType )
    {
        // actually transform the graphic only in this case. On the
        // other hand, cropping will always happen
        aActAttr = aGrafInfo;

        if( bMirror )
        {
            USHORT      nMirrorCase = ( aGeo.nDrehWink == 18000 ) ? ( bMirrored ? 3 : 4 ) : ( bMirrored ? 2 : 1 );
            FASTBOOL    bHMirr = nMirrorCase == 2 || nMirrorCase == 4;
            FASTBOOL    bVMirr = nMirrorCase == 3 || nMirrorCase == 4;

            aActAttr.SetMirrorFlags( ( bHMirr ? BMP_MIRROR_HORZ : 0 ) | ( bVMirr ? BMP_MIRROR_VERT : 0 ) );
        }

        if( bRotate )
            aActAttr.SetRotation( sal_uInt16(aGeo.nDrehWink / 10) );
    }

    // #107947# Delegate to moved code in GraphicObject
    return GetGraphicObject().GetTransformedGraphic( aDestSize, aDestMap, aActAttr );
}

// -----------------------------------------------------------------------------

GraphicType SdrGrafObj::GetGraphicType() const
{
    return pGraphic->GetType();
}

sal_Bool SdrGrafObj::IsAnimated() const
{
    return pGraphic->IsAnimated();
}

sal_Bool SdrGrafObj::IsTransparent() const
{
    return pGraphic->IsTransparent();
}

sal_Bool SdrGrafObj::IsEPS() const
{
    return pGraphic->IsEPS();
}

sal_Bool SdrGrafObj::IsSwappedOut() const
{
    return pGraphic->IsSwappedOut();
}

const MapMode& SdrGrafObj::GetGrafPrefMapMode() const
{
    return pGraphic->GetPrefMapMode();
}

const Size& SdrGrafObj::GetGrafPrefSize() const
{
    return pGraphic->GetPrefSize();
}

sal_Bool SdrGrafObj::DrawGraphic(OutputDevice* pOut, const Point& rPt, const Size& rSz,
    const GraphicAttr* pAttr, sal_uInt32 nFlags) const
{
    return pGraphic->Draw(pOut, rPt, rSz, pAttr, nFlags);
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetGrafStreamURL( const String& rGraphicStreamURL )
{
    if( !rGraphicStreamURL.Len() )
    {
        pGraphic->SetUserData();
        nGrafStreamPos = GRAFSTREAMPOS_INVALID;
    }
    else if( pModel->IsSwapGraphics() )
    {
        pGraphic->SetUserData( rGraphicStreamURL );
        nGrafStreamPos = GRAFSTREAMPOS_INVALID;

        // set state of graphic object to 'swapped out'
        if( pGraphic->GetType() == GRAPHIC_NONE )
            pGraphic->SetSwapState();
    }
}

// -----------------------------------------------------------------------------

String SdrGrafObj::GetGrafStreamURL() const
{
    return pGraphic->GetUserData();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetFileName(const String& rFileName)
{
    aFileName = rFileName;
    SetChanged();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetFilterName(const String& rFilterName)
{
    aFilterName = rFilterName;
    SetChanged();
}

// -----------------------------------------------------------------------------

FASTBOOL SdrGrafObj::HasSetName() const
{
    return TRUE;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetName(const XubString& rStr)
{
    aName = rStr;
    SetChanged();
}

// -----------------------------------------------------------------------------

XubString SdrGrafObj::GetName() const
{
    return aName;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ForceSwapIn() const
{
    pGraphic->FireSwapInRequest();

    if( pGraphic->IsSwappedOut() ||
        ( pGraphic->GetType() == GRAPHIC_NONE ) ||
        ( pGraphic->GetType() == GRAPHIC_DEFAULT ) )
    {
        Graphic aDefaultGraphic;
        aDefaultGraphic.SetDefaultType();
        pGraphic->SetGraphic( aDefaultGraphic );
    }
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ForceSwapOut() const
{
    pGraphic->FireSwapOutRequest();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ImpLinkAnmeldung()
{
    SvxLinkManager* pLinkManager = pModel != NULL ? pModel->GetLinkManager() : NULL;

    if( pLinkManager != NULL && pGraphicLink == NULL )
    {
        if( aFileName.Len() )
        {
            pGraphicLink = new SdrGraphicLink( this );
            pLinkManager->InsertFileLink( *pGraphicLink, OBJECT_CLIENT_GRF, aFileName, ( aFilterName.Len() ? &aFilterName : NULL ), NULL );
            pGraphicLink->Connect();
        }
    }
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ImpLinkAbmeldung()
{
    SvxLinkManager* pLinkManager = pModel != NULL ? pModel->GetLinkManager() : NULL;

    if( pLinkManager != NULL && pGraphicLink!=NULL)
    {
        // Bei Remove wird *pGraphicLink implizit deleted
        pLinkManager->Remove( pGraphicLink );
        pGraphicLink=NULL;
    }
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetGraphicLink( const String& rFileName, const String& rFilterName )
{
    ImpLinkAbmeldung();
    aFileName = rFileName;
    aFilterName = rFilterName;
    ImpLinkAnmeldung();
    pGraphic->SetUserData();

    // #92205# A linked graphic is per definition swapped out (has to be loaded)
    pGraphic->SetSwapState();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ReleaseGraphicLink()
{
    ImpLinkAbmeldung();
    aFileName = String();
    aFilterName = String();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    FASTBOOL bTrans = pGraphic->IsTransparent();
    FASTBOOL bAnim = pGraphic->IsAnimated();
    FASTBOOL bNoPresGrf = ( pGraphic->GetType() != GRAPHIC_NONE ) && !bEmptyPresObj;

    rInfo.bResizeFreeAllowed = aGeo.nDrehWink % 9000 == 0 ||
                               aGeo.nDrehWink % 18000 == 0 ||
                               aGeo.nDrehWink % 27000 == 0;

    rInfo.bResizePropAllowed = TRUE;
    rInfo.bRotateFreeAllowed = bNoPresGrf && !bAnim;
    rInfo.bRotate90Allowed = bNoPresGrf && !bAnim;
    rInfo.bMirrorFreeAllowed = bNoPresGrf && !bAnim;
    rInfo.bMirror45Allowed = bNoPresGrf && !bAnim;
    rInfo.bMirror90Allowed = !bEmptyPresObj;
    rInfo.bTransparenceAllowed = FALSE;
    rInfo.bGradientAllowed = FALSE;
    rInfo.bShearAllowed = FALSE;
    rInfo.bEdgeRadiusAllowed=FALSE;
    rInfo.bCanConvToPath = FALSE;
    rInfo.bCanConvToPathLineToArea = FALSE;
    rInfo.bCanConvToPolyLineToArea = FALSE;
    rInfo.bCanConvToPoly = !IsEPS();
    rInfo.bCanConvToContour = (rInfo.bCanConvToPoly || LineGeometryUsageIsNecessary());
}

// -----------------------------------------------------------------------------

UINT16 SdrGrafObj::GetObjIdentifier() const
{
    return UINT16( OBJ_GRAF );
}

// -----------------------------------------------------------------------------

sal_Bool SdrGrafObj::ImpUpdateGraphicLink() const
{
    sal_Bool    bRet = sal_False;

    if( pGraphicLink )
    {
        const sal_Bool bIsChanged = pModel->IsChanged();
        pGraphicLink->UpdateSynchron();
        pModel->SetChanged( bIsChanged );

        bRet = sal_True;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

// Liefert FALSE, wenn die Pres-Bitmap zu gross ist
//FASTBOOL SdrGrafObj::ImpPaintEmptyPres( OutputDevice* pOutDev ) const
//{
//  const MapMode   aDstMapMode( pOutDev->GetMapMode().GetMapUnit() );
//  Point           aPos( aRect.Center() );
//  Size            aSize;
//    FASTBOOL        bRet;
//
//  if( pGraphic->GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
//      aSize = pOutDev->PixelToLogic( pGraphic->GetPrefSize(), aDstMapMode );
//  else
//      aSize = pOutDev->LogicToLogic( pGraphic->GetPrefSize(), pGraphic->GetPrefMapMode(), aDstMapMode );
//
//  aPos.X() -= ( aSize.Width() >> 1 );
//  aPos.Y() -= ( aSize.Height() >> 1 );
//
//    if( aPos.X() >= aRect.Left() && aPos.Y() >= aRect.Top() )
//  {
//      const Graphic& rGraphic = pGraphic->GetGraphic();
//
//      if( pGraphic->GetType() == GRAPHIC_BITMAP )
//      {
//          DrawGraphic(pOutDev, aPos, aSize);
//      }
//      else
//      {
//          const ULONG nOldDrawMode = pOutDev->GetDrawMode();
//
//          if( ( nOldDrawMode & DRAWMODE_GRAYBITMAP ) != 0 )
//          {
//              // Falls Modus GRAYBITMAP, wollen wir auch Mtf's als Graustufen darstellen
//              ULONG nNewDrawMode = nOldDrawMode;
//              nNewDrawMode &= ~( DRAWMODE_BLACKLINE | DRAWMODE_BLACKFILL | DRAWMODE_WHITEFILL | DRAWMODE_NOFILL );
//              pOutDev->SetDrawMode( nNewDrawMode |= DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL  );
//          }
//
//          rGraphic.Draw( pOutDev, aPos, aSize );
//          pOutDev->SetDrawMode( nOldDrawMode );
//      }
//
//      bRet = TRUE;
//  }
//  else
//      bRet = FALSE;
//
//  return bRet;
//}

// -----------------------------------------------------------------------------

//void SdrGrafObj::ImpPaintReplacement(OutputDevice* pOutDev, const XubString& rText, const Bitmap* pBmp, FASTBOOL bFill) const
//{
//    Size aPixelSize( 1, 1 );
//    Size aBmpSize;
//
//    aPixelSize = Application::GetDefaultDevice()->PixelToLogic( aPixelSize, pOutDev->GetMapMode() );
//
//  if( bFill )
//  {
//      pOutDev->SetLineColor();
//      pOutDev->SetFillColor( COL_LIGHTGRAY );
//  }
//
//  Rectangle   aRect1( aRect );
//  Rectangle   aRect2( aRect1 );
//  Rectangle   aTextRect( aRect1 );
//  Point       aTopLeft( aRect1.TopLeft() );
//  Point       aBmpPos( aTopLeft );
//
//    aRect2.Left() += aPixelSize.Width();
//    aRect2.Top() += aPixelSize.Height();
//    aRect2.Right() -= aPixelSize.Width();
//    aRect2.Bottom() -= aPixelSize.Height();
//
//  if( pBmp != NULL )
//  {
//      aBmpSize = Size( Application::GetDefaultDevice()->PixelToLogic( pBmp->GetSizePixel(), pOutDev->GetMapMode() ) );
//
//      long        nRectWdt = aTextRect.Right() - aTextRect.Left();
//      long        nRectHgt = aTextRect.Bottom() - aTextRect.Top();
//      long        nBmpWdt = aBmpSize.Width();
//      long        nBmpHgt = aBmpSize.Height();
//      long        nMinWdt = nBmpWdt;
//      long        nMinHgt = nBmpHgt;
//      BOOL        bText = rText.Len() > 0;
//
//      if( bText )
//      {
//          nMinWdt= 2 * nBmpWdt + 5 * aPixelSize.Width();
//          nMinHgt= 2 * nBmpHgt + 5 * aPixelSize.Height();
//      }
//
//      if( nRectWdt < nMinWdt || nRectHgt < nMinHgt )
//          pBmp=NULL;
//      else
//      {
//          aTextRect.Left() += nBmpWdt;
//
//          if( bText )
//              aTextRect.Left() += 5 * aPixelSize.Width();
//      }
//
//      aBmpPos.X() += 2 * aPixelSize.Width();
//      aBmpPos.Y() += 2 * aPixelSize.Height();
//
//      if( aGeo.nDrehWink != 0 )
//      {
//          Point   aRef( aBmpPos.X() - aBmpSize.Width() / 2 + 2 * aPixelSize.Width(),
//                        aBmpPos.Y() - aBmpSize.Height() / 2 + 2 * aPixelSize.Height() );
//          double  nSin = sin( aGeo.nDrehWink * nPi180 );
//          double  nCos = cos( aGeo.nDrehWink * nPi180 );
//
//          RotatePoint( aBmpPos, aRef, nSin, nCos );
//      }
//  }
//
//  if( aGeo.nDrehWink == 0 && aGeo.nShearWink == 0 )
//  {
//      const StyleSettings& rStyleSettings = pOutDev->GetSettings().GetStyleSettings();
//
//      if( bFill )
//          pOutDev->DrawRect( aRect );
//
//      if( pBmp!=NULL )
//          pOutDev->DrawBitmap( aBmpPos, aBmpSize, *pBmp );
//
//      pOutDev->SetFillColor();
//      pOutDev->SetLineColor( rStyleSettings.GetShadowColor() );
//      pOutDev->DrawLine( aRect1.TopLeft(), aRect1.TopRight() );
//      pOutDev->DrawLine( aRect1.TopLeft(), aRect1.BottomLeft() );
//
//      pOutDev->SetLineColor( rStyleSettings.GetLightColor() );
//      pOutDev->DrawLine( aRect1.TopRight(), aRect1.BottomRight() );
//      pOutDev->DrawLine( aRect1.BottomLeft(), aRect1.BottomRight() );
//
//      pOutDev->SetLineColor( rStyleSettings.GetLightColor() );
//      pOutDev->DrawLine( aRect2.TopLeft(), aRect2.TopRight() );
//      pOutDev->DrawLine( aRect2.TopLeft(), aRect2.BottomLeft() );
//
//      pOutDev->SetLineColor( rStyleSettings.GetShadowColor() );
//      pOutDev->DrawLine( aRect2.TopRight(), aRect2.BottomRight() );
//      pOutDev->DrawLine( aRect2.BottomLeft(), aRect2.BottomRight() );
//  }
//  else
//  {
//      Polygon aPoly1( Rect2Poly( aRect1, aGeo ) );
//      Polygon aPoly2(5);
//
//      aPoly2[0] = aRect2.TopLeft();
//      aPoly2[1] = aRect2.TopRight();
//      aPoly2[2] = aRect2.BottomRight();
//      aPoly2[3] = aRect2.BottomLeft();
//      aPoly2[4] = aRect2.TopLeft();
//
//      if( aGeo.nShearWink != 0 )
//          ShearPoly( aPoly2, aTopLeft, aGeo.nTan );
//
//      if( aGeo.nDrehWink != 0 )
//          RotatePoly( aPoly2, aTopLeft, aGeo.nSin, aGeo.nCos );
//
//      if( bFill )
//          pOutDev->DrawPolygon( aPoly1 );
//
//      if( pBmp != NULL )
//          pOutDev->DrawBitmap( aBmpPos, aBmpSize, *pBmp );
//
//      pOutDev->SetFillColor();
//
//      const StyleSettings&    rStyleSettings = pOutDev->GetSettings().GetStyleSettings();
//      Color                   a3DLightColor( rStyleSettings.GetLightColor() );
//      Color                   a3DShadowColor( rStyleSettings.GetShadowColor() );
//      long                    nHWink=NormAngle360( aGeo.nDrehWink );
//      long                    nVWink=NormAngle360( aGeo.nDrehWink-aGeo.nShearWink );
//      FASTBOOL                bHorzChg=nHWink>13500 && nHWink<=31500;
//      FASTBOOL                bVertChg=nVWink>4500 && nVWink<=22500;
//
//      pOutDev->SetLineColor( bHorzChg ? a3DShadowColor : a3DLightColor);
//      pOutDev->DrawLine( aPoly2[0], aPoly2[1] );
//
//      pOutDev->SetLineColor( bHorzChg ? a3DLightColor  : a3DShadowColor);
//      pOutDev->DrawLine( aPoly2[2], aPoly2[3] );
//
//      pOutDev->SetLineColor( bVertChg ? a3DLightColor  : a3DShadowColor);
//      pOutDev->DrawLine( aPoly2[1], aPoly2[2] );
//
//      pOutDev->SetLineColor( bVertChg ? a3DShadowColor : a3DLightColor);
//      pOutDev->DrawLine( aPoly2[3], aPoly2[4] );
//
//      pOutDev->SetLineColor( bHorzChg ? a3DLightColor  : a3DShadowColor);
//      pOutDev->DrawLine( aPoly1[0], aPoly1[1] );
//
//      pOutDev->SetLineColor( bHorzChg ? a3DShadowColor : a3DLightColor);
//      pOutDev->DrawLine( aPoly1[2], aPoly1[3] );
//
//      pOutDev->SetLineColor( bVertChg ? a3DShadowColor : a3DLightColor);
//      pOutDev->DrawLine( aPoly1[1], aPoly1[2] );
//
//      pOutDev->SetLineColor( bVertChg ? a3DLightColor  : a3DShadowColor);
//      pOutDev->DrawLine( aPoly1[3], aPoly1[4] );
//  }
//
//  XubString aNam( rText );
//
//  if( aNam.Len() )
//  {
//      Size aOutSize( aTextRect.GetWidth() - 6 * aPixelSize.Width(),
//                     aTextRect.GetHeight() - 6 * aPixelSize.Height() );
//
//      if( aOutSize.Width() >= ( 4 * aPixelSize.Width() ) ||
//          aOutSize.Height() >=  ( 4 * aPixelSize.Height() ) )
//      {
//          Point   aOutPos( aTextRect.Left() + 3 * aPixelSize.Width(),
//                           aTextRect.Top() + 3 * aPixelSize.Height() );
//          long    nMaxOutY = aOutPos.Y() + aOutSize.Height();
//          Font    aFontMerk( pOutDev->GetFont() );
//          Font    aFont( OutputDevice::GetDefaultFont( DEFAULTFONT_SANS_UNICODE, LANGUAGE_SYSTEM, DEFAULTFONT_FLAGS_ONLYONE ) );
//
//          aFont.SetColor( COL_LIGHTRED );
//          aFont.SetTransparent( TRUE );
//          aFont.SetLineOrientation( USHORT( NormAngle360( aGeo.nDrehWink ) / 10 ) );
//
//          if( IsLinkedGraphic() )
//              aFont.SetUnderline( UNDERLINE_SINGLE );
//
//          Size aFontSize( 0, ( aGeo.nDrehWink % 9000 == 0 ? 12 : 14 ) * aPixelSize.Height() );
//
//          if( aFontSize.Height() > aOutSize.Height() )
//              aFontSize.Height() = aOutSize.Height();
//
//          aFont.SetSize( aFontSize );
//          pOutDev->SetFont( aFont );
//          String aOutStr( aNam );
//
//          while( aOutStr.Len() && aOutPos.Y() <= nMaxOutY )
//          {
//              String  aStr1( aOutStr );
//              INT32   nTextWidth = pOutDev->GetTextWidth( aStr1 );
//              INT32   nTextHeight = pOutDev->GetTextHeight();
//
//              while( aStr1.Len() && nTextWidth > aOutSize.Width() )
//              {
//                  aStr1.Erase( aStr1.Len() - 1 );
//                  nTextWidth = pOutDev->GetTextWidth( aStr1 );
//                  nTextHeight = pOutDev->GetTextHeight();
//              }
//
//              Point aPos( aOutPos );
//              aOutPos.Y() += nTextHeight;
//
//              if( aOutPos.Y() <= nMaxOutY )
//              {
//                  if( aGeo.nShearWink != 0 )
//                      ShearPoint( aPos, aTopLeft, aGeo.nTan );
//
//                  if( aGeo.nDrehWink != 0 )
//                      RotatePoint( aPos, aTopLeft, aGeo.nSin, aGeo.nCos );
//
//                  pOutDev->DrawText( aPos, aStr1 );
//                  aOutStr.Erase( 0, aStr1.Len() );
//              }
//          }
//
//          pOutDev->SetFont( aFontMerk );
//      }
//  }
//}

// -----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// #i25616#

void SdrGrafObj::ImpDoPaintGrafObjShadow( ExtOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec ) const
{
    const sal_Bool bShadOn(((SdrShadowItem&)(GetObjectItem(SDRATTR_SHADOW))).GetValue());

    if(bShadOn)
    {
        const sal_Bool bDoPaintFilling(IsObjectTransparent());
        const sal_Bool bDoPaintLine(0L != ImpGetLineWdt());

        if(bDoPaintFilling || bDoPaintLine)
        {
            ((SdrGrafObj*)this)->mbInsidePaint = sal_True;
            SdrRectObj::ImpDoPaintRectObjShadow(rOut, rInfoRec, bDoPaintFilling, bDoPaintLine);
            ((SdrGrafObj*)this)->mbInsidePaint = sal_False;
        }

        const sal_Bool bIsFillDraft(0 != (rInfoRec.nPaintMode & SDRPAINTMODE_DRAFTFILL));

        if(!bIsFillDraft)
        {
            OutputDevice* pOutDev = rOut.GetOutDev();
            const sal_uInt32 nXDist(((SdrShadowXDistItem&)GetObjectItem(SDRATTR_SHADOWXDIST)).GetValue());
            const sal_uInt32 nYDist(((SdrShadowYDistItem&)GetObjectItem(SDRATTR_SHADOWYDIST)).GetValue());
            const Color aShadowColor(((SdrShadowColorItem&)GetObjectItem(SDRATTR_SHADOWCOLOR)).GetValue());
            const sal_uInt16 nShadowTransparence(((SdrShadowTransparenceItem&)GetObjectItem(SDRATTR_SHADOWTRANSPARENCE)).GetValue());

            if(IsObjectTransparent())
            {
                // object which casts the shadow is somehow transparent, create a transparent
                // shadow
                Rectangle aSnapRect(GetSnapRect());
                aSnapRect.Move(nXDist, nYDist);
                const Rectangle aSnapRectPixel(pOutDev->LogicToPixel(aSnapRect));

                // get BitmapEx
                Graphic aTempGraphic = GetTransformedGraphic();
                Size aPixelSize(aSnapRectPixel.GetSize());
                BitmapEx aTempBitmapEx = aTempGraphic.GetBitmapEx(&aPixelSize);

                // paint
                if(aTempBitmapEx.IsTransparent())
                {
                    if(aTempBitmapEx.IsAlpha())
                    {
                        BitmapPalette aTempBitmapPalette(2);
                        aTempBitmapPalette[0] = aShadowColor;
                        aTempBitmapPalette[1] = Color(COL_BLACK);
                        Bitmap aSingleColorBitmap(aPixelSize, 1, &aTempBitmapPalette);
                        AlphaMask aTempAlphaMask(aTempBitmapEx.GetAlpha());
                        BitmapEx aTempMaskedBitmap(aSingleColorBitmap, aTempAlphaMask);
                        pOutDev->DrawBitmapEx(aSnapRect.TopLeft(), aTempMaskedBitmap);
                    }
                    else
                    {
                        if(0 != nShadowTransparence && 100 > nShadowTransparence)
                        {
                            Bitmap aTempBitmapMask(aTempBitmapEx.GetMask());
                            aTempBitmapMask.Invert();
                            BYTE bEraseValue(0xff);
                            AlphaMask aTempAlphaMask(aPixelSize, &bEraseValue);
                            BYTE aReplaceTransparency((nShadowTransparence * 0x00ff) / 100);
                            aTempAlphaMask.Replace(aTempBitmapMask, aReplaceTransparency);

                            BitmapPalette aTempBitmapPalette(2);
                            aTempBitmapPalette[0] = aShadowColor;
                            aTempBitmapPalette[1] = Color(COL_BLACK);
                            Bitmap aSingleColorBitmap(aPixelSize, 1, &aTempBitmapPalette);

                            BitmapEx aTempMaskedBitmap(aSingleColorBitmap, aTempAlphaMask);
                            pOutDev->DrawBitmapEx(aSnapRect.TopLeft(), aTempMaskedBitmap);
                        }
                        else
                        {
                            Bitmap aTempBitmapMask(aTempBitmapEx.GetMask());
                            pOutDev->DrawMask(aSnapRect.TopLeft(), aTempBitmapMask, aShadowColor);
                        }
                    }
                }
            }
            else
            {
                // shadow for the whole rectangle
                pOutDev->SetFillColor(aShadowColor);
                pOutDev->SetLineColor();
                Polygon aOutputPoly(XOutCreatePolygon(GetXPoly(), pOutDev));
                aOutputPoly.Move(nXDist, nYDist);

                if(0 != nShadowTransparence && 100 > nShadowTransparence)
                {
                    PolyPolygon aPolyPoly(aOutputPoly);
                    pOutDev->DrawTransparent(aPolyPoly, nShadowTransparence);
                }
                else
                {
                    pOutDev->DrawPolygon(aOutputPoly);
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// #i25616#

void SdrGrafObj::ImpDoPaintGrafObj( ExtOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec ) const
{
    const sal_Bool bDoPaintFilling(IsObjectTransparent());
    const sal_Bool bDoPaintLine(XLINE_NONE != ((XLineStyleItem&)(GetObjectItem(XATTR_LINESTYLE))).GetValue());

    if(bDoPaintFilling || bDoPaintLine)
    {
        ((SdrGrafObj*)this)->mbInsidePaint = sal_True;
        SdrRectObj::ImpDoPaintRectObj(rOut, rInfoRec, bDoPaintFilling, bDoPaintLine);
        ((SdrGrafObj*)this)->mbInsidePaint = sal_False;
    }
}

//////////////////////////////////////////////////////////////////////////////

sal_Bool SdrGrafObj::DoPaintObject( ExtOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec ) const
{
    sal_Bool bRetval(sal_False);

    if( IsEmptyPresObj()
        || pGraphic->IsSwappedOut()
        || GRAPHIC_NONE == pGraphic->GetType()
        || GRAPHIC_DEFAULT == pGraphic->GetType())
    {
        // This should not happen. If it happens, someone is using DoPaintObject
        // directly. This would require a draft paint. All this cases are handled
        // from the DrawContact object.
    }
    else
    {
        OutputDevice* pOutDev = rOut.GetOutDev();
        Point aLogPos(aRect.TopLeft());

        // #116639# Simply take the logic size, forward/backward conversion is no longer necessary
        // and may make the pixel size one too big.
        Size aLogSize(aRect.GetSize());

        GraphicAttr aAttr(aGrafInfo);
        const SdrView* pView = (rInfoRec.pPV ? &rInfoRec.pPV->GetView() : 0L);
        const sal_uInt32 nGraphicManagerDrawMode(pView ? pView->GetGraphicManagerDrawMode() : GRFMGR_DRAW_STANDARD);

        sal_Int32 nDrehWink(aGeo.nDrehWink);
        sal_Int32 nShearWink(aGeo.nShearWink);
        sal_Bool bRotate(nDrehWink != 0 && nDrehWink != 18000);
        sal_Bool bShear(nShearWink != 0);
        sal_Bool bRota180(nDrehWink == 18000);
        sal_uInt16 nMirrorCase(bRota180 ? (bMirrored ? 3 : 4) : (bMirrored ? 2 : 1));
        sal_Bool bHMirr((2 == nMirrorCase ) || (4 == nMirrorCase));
        sal_Bool bVMirr((3 == nMirrorCase ) || (4 == nMirrorCase));

        // #i25616#
        ImpDoPaintGrafObjShadow( rOut, rInfoRec );

        // #i25616#
        ImpDoPaintGrafObj( rOut, rInfoRec );

        aAttr.SetMirrorFlags((bHMirr ? BMP_MIRROR_HORZ : 0)|(bVMirr ? BMP_MIRROR_VERT : 0));

        if(bRota180)
        {
            aLogPos.X() -= (aLogSize.Width() - 1L);
            aLogPos.Y() -= (aLogSize.Height() - 1L);
        }

        if(GRAPHIC_BITMAP == pGraphic->GetType())
        {
            // #111096#
            // paint animated graphic?
            if(rInfoRec.mbUseBitmapEx)
            {
                Point aDestPos = pOutDev->LogicToPixel(aLogPos);
                Size aDestSize = pOutDev->LogicToPixel(aLogSize);
                sal_Bool bMapModeWasEnabled(pOutDev->IsMapModeEnabled());

                pOutDev->EnableMapMode(sal_False);
                pOutDev->DrawBitmapEx(aDestPos, aDestSize, rInfoRec.maBitmapEx);
                pOutDev->EnableMapMode(bMapModeWasEnabled);
            }
            else
            {
                if(bRotate && !bRota180)
                {
                    aAttr.SetRotation((sal_uInt16)(nDrehWink / 10));
                }

                DrawGraphic(pOutDev, aLogPos, aLogSize, &aAttr, nGraphicManagerDrawMode);
            }
        }
        else
        {
            // MetaFiles
            const sal_uInt32 nOldDrawMode(pOutDev->GetDrawMode());

            // For mode GRAYBITMAP, we want to show metafiles gray, too.
            if(nOldDrawMode & DRAWMODE_GRAYBITMAP)
            {
                sal_uInt32 nNewDrawMode(nOldDrawMode);
                nNewDrawMode &= ~(DRAWMODE_BLACKLINE | DRAWMODE_BLACKFILL | DRAWMODE_WHITEFILL | DRAWMODE_NOFILL);
                nNewDrawMode |= DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL;
                pOutDev->SetDrawMode(nNewDrawMode);
            }

            if(bRotate && !bRota180)
            {
                aAttr.SetRotation((sal_uInt16)(nDrehWink / 10));
            }

            DrawGraphic(pOutDev, aLogPos, aLogSize, &aAttr, nGraphicManagerDrawMode);
            pOutDev->SetDrawMode(nOldDrawMode);
        }

        bRetval = sal_True;
    }

    if(HasText())
    {
        bRetval |= SdrTextObj::DoPaintObject(rOut, rInfoRec);
    }

    return bRetval;
}

// -----------------------------------------------------------------------------

SdrObject* SdrGrafObj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    return ImpCheckHit( rPnt, nTol, pVisiLayer, TRUE );
}

// -----------------------------------------------------------------------------

void SdrGrafObj::TakeObjNameSingul(XubString& rName) const
{
    switch( pGraphic->GetType() )
    {
        case GRAPHIC_BITMAP:
        {
            const USHORT nId = ( ( IsTransparent() || ( (const SdrGrafTransparenceItem&) GetObjectItem( SDRATTR_GRAFTRANSPARENCE ) ).GetValue() ) ?
                                 ( IsLinkedGraphic() ? STR_ObjNameSingulGRAFBMPTRANSLNK : STR_ObjNameSingulGRAFBMPTRANS ) :
                                 ( IsLinkedGraphic() ? STR_ObjNameSingulGRAFBMPLNK : STR_ObjNameSingulGRAFBMP ) );

            rName=ImpGetResStr( nId );
        }
        break;

        case GRAPHIC_GDIMETAFILE:
            rName=ImpGetResStr( IsLinkedGraphic() ? STR_ObjNameSingulGRAFMTFLNK : STR_ObjNameSingulGRAFMTF );
        break;

        case GRAPHIC_NONE:
            rName=ImpGetResStr( IsLinkedGraphic() ? STR_ObjNameSingulGRAFNONELNK : STR_ObjNameSingulGRAFNONE );
        break;

        default:
            rName=ImpGetResStr(  IsLinkedGraphic() ? STR_ObjNameSingulGRAFLNK : STR_ObjNameSingulGRAF );
        break;
    }

    if( aName.Len() )
    {
        rName.AppendAscii( " '" );
        rName += aName;
        rName += sal_Unicode( '\'' );
    }
}

// -----------------------------------------------------------------------------

void SdrGrafObj::TakeObjNamePlural( XubString& rName ) const
{
    switch( pGraphic->GetType() )
    {
        case GRAPHIC_BITMAP:
        {
            const USHORT nId = ( ( IsTransparent() || ( (const SdrGrafTransparenceItem&) GetObjectItem( SDRATTR_GRAFTRANSPARENCE ) ).GetValue() ) ?
                                 ( IsLinkedGraphic() ? STR_ObjNamePluralGRAFBMPTRANSLNK : STR_ObjNamePluralGRAFBMPTRANS ) :
                                 ( IsLinkedGraphic() ? STR_ObjNamePluralGRAFBMPLNK : STR_ObjNamePluralGRAFBMP ) );

            rName=ImpGetResStr( nId );
        }
        break;

        case GRAPHIC_GDIMETAFILE:
            rName=ImpGetResStr( IsLinkedGraphic() ? STR_ObjNamePluralGRAFMTFLNK : STR_ObjNamePluralGRAFMTF );
        break;

        case GRAPHIC_NONE:
            rName=ImpGetResStr( IsLinkedGraphic() ? STR_ObjNamePluralGRAFNONELNK : STR_ObjNamePluralGRAFNONE );
        break;

        default:
            rName=ImpGetResStr(  IsLinkedGraphic() ? STR_ObjNamePluralGRAFLNK : STR_ObjNamePluralGRAF );
        break;
    }

    if( aName.Len() )
    {
        rName.AppendAscii( " '" );
        rName += aName;
        rName += sal_Unicode( '\'' );
    }
}

// -----------------------------------------------------------------------------

void SdrGrafObj::operator=( const SdrObject& rObj )
{
    SdrRectObj::operator=( rObj );

    const SdrGrafObj& rGraf = (SdrGrafObj&) rObj;

    pGraphic->SetGraphic( rGraf.GetGraphic() );
    aCropRect = rGraf.aCropRect;
    aFileName = rGraf.aFileName;
    aFilterName = rGraf.aFilterName;
    aName = rGraf.aName;
    bMirrored = rGraf.bMirrored;

    if( rGraf.pGraphicLink != NULL)
    {
        SetGraphicLink( aFileName, aFilterName );
    }

    ImpSetAttrToGrafInfo();
}

// -----------------------------------------------------------------------------
// #i25616#

void SdrGrafObj::TakeXorPoly(XPolyPolygon& rPoly, FASTBOOL bDetail) const
{
    if(mbInsidePaint)
    {
        // take grown rectangle
        const sal_Int32 nHalfLineWidth(ImpGetLineWdt() / 2);
        const Rectangle aGrownRect(
            aRect.Left() - nHalfLineWidth,
            aRect.Top() - nHalfLineWidth,
            aRect.Right() + nHalfLineWidth,
            aRect.Bottom() + nHalfLineWidth);

        rPoly = XPolyPolygon(ImpCalcXPoly(aGrownRect, GetEckenradius()));
    }
    else
    {
        // call parent
        SdrRectObj::TakeXorPoly(rPoly, bDetail);
    }
}

// -----------------------------------------------------------------------------

FASTBOOL SdrGrafObj::HasSpecialDrag() const
{
    return TRUE;
}

// -----------------------------------------------------------------------------

USHORT SdrGrafObj::GetHdlCount() const
{
    return 8;
}

// -----------------------------------------------------------------------------

SdrHdl* SdrGrafObj::GetHdl(USHORT nHdlNum) const
{
    return SdrRectObj::GetHdl( nHdlNum + 1 );
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    SdrRectObj::NbcResize( rRef, xFact, yFact );

    FASTBOOL bMirrX = xFact.GetNumerator() < 0;
    FASTBOOL bMirrY = yFact.GetNumerator() < 0;

    if( bMirrX != bMirrY )
        bMirrored = !bMirrored;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
    SdrRectObj::NbcRotate(rRef,nWink,sn,cs);
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SdrRectObj::NbcMirror(rRef1,rRef2);
    bMirrored = !bMirrored;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
{
    SdrRectObj::NbcRotate( rRef, nWink, tn, bVShear );
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcSetSnapRect(const Rectangle& rRect)
{
    SdrRectObj::NbcSetSnapRect(rRect);
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcSetLogicRect( const Rectangle& rRect)
{
    FASTBOOL bChg=rRect.GetSize()!=aRect.GetSize();
    SdrRectObj::NbcSetLogicRect(rRect);
}

// -----------------------------------------------------------------------------

SdrObjGeoData* SdrGrafObj::NewGeoData() const
{
    return new SdrGrafObjGeoData;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrRectObj::SaveGeoData(rGeo);
    SdrGrafObjGeoData& rGGeo=(SdrGrafObjGeoData&)rGeo;
    rGGeo.bMirrored=bMirrored;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    long        nDrehMerk = aGeo.nDrehWink;
    long        nShearMerk = aGeo.nShearWink;
    FASTBOOL    bMirrMerk = bMirrored;
    Size        aSizMerk( aRect.GetSize() );

    SdrRectObj::RestGeoData(rGeo);
    SdrGrafObjGeoData& rGGeo=(SdrGrafObjGeoData&)rGeo;
    bMirrored=rGGeo.bMirrored;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetPage( SdrPage* pNewPage )
{
    FASTBOOL bRemove = pNewPage == NULL && pPage != NULL;
    FASTBOOL bInsert = pNewPage != NULL && pPage == NULL;

    if( bRemove )
    {
        // hier kein SwapIn noetig, weil wenn nicht geladen, dann auch nicht animiert.
        if( pGraphic->IsAnimated())
            pGraphic->StopAnimation();

        if( pGraphicLink != NULL )
            ImpLinkAbmeldung();
    }

    SdrRectObj::SetPage( pNewPage );

    if(aFileName.Len() && bInsert)
        ImpLinkAnmeldung();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetModel( SdrModel* pNewModel )
{
    FASTBOOL bChg = pNewModel != pModel;

    if( bChg )
    {
        if( ( GRAFSTREAMPOS_INVALID != nGrafStreamPos ) || pGraphic->HasUserData() )
        {
            ForceSwapIn();
            pGraphic->SetUserData();
            nGrafStreamPos = GRAFSTREAMPOS_INVALID;
        }

        if( pGraphicLink != NULL )
            ImpLinkAbmeldung();
    }

    // Model umsetzen
    SdrRectObj::SetModel(pNewModel);

    if( bChg && aFileName.Len() )
        ImpLinkAnmeldung();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::StartAnimation( OutputDevice* pOutDev, const Point& rPoint, const Size& rSize, long nExtraData )
{
    // #111096#
    // use new graf animation
    SetGrafAnimationAllowed(sal_True);
}

// -----------------------------------------------------------------------------

void SdrGrafObj::StopAnimation(OutputDevice* pOutDev, long nExtraData)
{
    // #111096#
    // use new graf animation
    SetGrafAnimationAllowed(sal_False);
}

// -----------------------------------------------------------------------------

FASTBOOL SdrGrafObj::HasGDIMetaFile() const
{
    return( pGraphic->GetType() == GRAPHIC_GDIMETAFILE );
}

// -----------------------------------------------------------------------------

const GDIMetaFile* SdrGrafObj::GetGDIMetaFile() const
{
    DBG_ERROR( "Invalid return value! Don't use it! (KA)" );
    return &GetGraphic().GetGDIMetaFile();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrGrafObj::WriteData(SvStream& rOut) const
{
    ForceSwapIn();
    SdrRectObj::WriteData( rOut );

    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
    SdrDownCompat aCompat( rOut, STREAM_WRITE );

#ifdef DBG_UTIL
    aCompat.SetID( "SdrGrafObj" );
#endif

    GraphicType eType( pGraphic->GetType() );
    BOOL        bHasGraphic( !aFileName.Len() && eType != GRAPHIC_NONE );

    // dieses Flag wird ab V11 rausgeschrieben
    rOut << bHasGraphic;

    if(bHasGraphic)
    {
        // Graphik ist nicht gelinkt: ggf. komprimiert speichern:
        // seit V11 eingapackt
        SdrDownCompat aGrafCompat(rOut, STREAM_WRITE);
        BOOL bZCompr(pModel && pModel->IsSaveCompressed() && eType == GRAPHIC_BITMAP);
        BOOL bNCompr(pModel && pModel->IsSaveNative());
        const UINT16 nOldComprMode(rOut.GetCompressMode());
        UINT16 nNewComprMode(nOldComprMode);

#ifdef DBG_UTIL
        aGrafCompat.SetID( "SdrGrafObj(Graphic)" );
#endif

        if(pModel->IsSwapGraphics() && (pModel->GetSwapGraphicsMode() & SDR_SWAPGRAPHICSMODE_DOC))
        {
            ((SdrGrafObj*)this)->pGraphic->SetUserData();
            ((SdrGrafObj*)this)->nGrafStreamPos = rOut.Tell();
        }

        if(bZCompr)
            nNewComprMode |= COMPRESSMODE_ZBITMAP;

        if(bNCompr)
            nNewComprMode |= COMPRESSMODE_NATIVE;

        rOut.SetCompressMode( nNewComprMode );
        rOut << pGraphic->GetGraphic();
        rOut.SetCompressMode( nOldComprMode );
    }

    rOut << aCropRect;
    rOut << BOOL(bMirrored);

    rOut.WriteByteString(aName);

    String aRelFileName;

    if( aFileName.Len() )
    {
        aRelFileName = INetURLObject::AbsToRel( aFileName,
                                                INetURLObject::WAS_ENCODED,
                                                INetURLObject::DECODE_UNAMBIGUOUS );
    }

    rOut.WriteByteString( aRelFileName );

    // UNICODE: rOut << aFilterName;
    rOut.WriteByteString(aFilterName);

    // ab V11
    rOut << (BOOL)( aFileName.Len() != 0 );

    SfxItemPool* pPool = GetItemPool();

    if(pPool)
    {
        const SfxItemSet& rSet = GetObjectItemSet();

        pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_GRAF));
    }
    else
        rOut << UINT16( SFX_ITEMS_NULL );

    ForceSwapOut();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ReadDataTilV10( const SdrObjIOHeader& rHead, SvStream& rIn )
{
    Graphic aGraphic;

    // Import von allem mit Version <= 10
    rIn >> aGraphic;

    ULONG nError = rIn.GetError();

    // Ist die Graphik defekt, oder wurde nur eine leere Graphik eingelesen? (was bei gelinkten Graphiken der Fall ist)
    if( nError != 0)
        rIn.ResetError();

    if( rHead.GetVersion() >= 6)
        rIn >> aCropRect;

    if(rHead.GetVersion() >= 8)
    {
        // UNICODE: rIn>>aFileName;
        rIn.ReadByteString(aFileName);
    }

    if(rHead.GetVersion() >= 9)
    {
        // UNICODE: rIn >> aFilterName;
        rIn.ReadByteString(aFilterName);
    }
    else
        aFilterName = String( RTL_CONSTASCII_USTRINGPARAM( "BMP - MS Windows" ) );

    if( aFileName.Len() )
    {
        String aFileURLStr;

        if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aFileName, aFileURLStr ) )
        {
            SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aFileURLStr, STREAM_READ | STREAM_SHARE_DENYNONE );

            if( pIStm )
            {
                GraphicFilter*  pFilter = GetGrfFilter();
                USHORT          nError = pFilter->ImportGraphic( aGraphic, aFileURLStr, *pIStm );

                SetGraphicLink( aFileURLStr, aFilterName );

                delete pIStm;
            }
        }
    }
    else if( nError != 0 )
        rIn.SetError(nError);


    if( !rIn.GetError() )
        pGraphic->SetGraphic( aGraphic );
}

// -----------------------------------------------------------------------------

#ifdef WNT
#pragma optimize ("",off)
#endif

void SdrGrafObj::ReadData( const SdrObjIOHeader& rHead, SvStream& rIn )
{
    if( rIn.GetError() )
        return;

    SdrRectObj::ReadData( rHead, rIn );

    SdrDownCompat   aCompat( rIn, STREAM_READ );
    FASTBOOL        bDelayedLoad = ( pModel != NULL ) && pModel->IsSwapGraphics();

#ifdef DBG_UTIL
    aCompat.SetID("SdrGrafObj");
#endif

    pGraphic->SetUserData();
    nGrafStreamPos = GRAFSTREAMPOS_INVALID;

    if( rHead.GetVersion() < 11 )
        ReadDataTilV10( rHead, rIn );
    else
    {
        String  aFileNameRel;
        BOOL    bHasGraphic;
        BOOL    bTmp;
        BOOL    bGraphicLink;

        rIn >> bHasGraphic;

        if( bHasGraphic )
        {
            SdrDownCompat aGrafCompat( rIn,STREAM_READ );

#ifdef DBG_UTIL
            aGrafCompat.SetID("SdrGrafObj(Graphic)");
#endif

            nGrafStreamPos = rIn.Tell();

            if( !bDelayedLoad )
            {
                Graphic aGraphic;
                rIn >> aGraphic;
                pGraphic->SetGraphic( aGraphic );
            }
            else
                pGraphic->SetSwapState();

            // Ist die Grafik defekt, oder wurde nur eine leere Graphik eingelesen?
            // Daran soll mein Read jedoch nicht scheitern.
            if( rIn.GetError() != 0 )
                rIn.ResetError();
        }

        rIn >> aCropRect;
        rIn >> bTmp; bMirrored = bTmp;

        rIn.ReadByteString(aName);
        // #85414# since there seems to be some documents wich have an illegal
        // character inside the name of a graphic object we have to fix this
        // here on load time or it will crash our xml later.
        const xub_StrLen nLen = aName.Len();
        for( xub_StrLen nIndex = 0; nIndex < nLen; nIndex++ )
        {
            if( aName.GetChar( nIndex ) < ' ' )
                aName.SetChar( nIndex, '?' );
        }

        rIn.ReadByteString(aFileNameRel);

        if( aFileNameRel.Len() )
        {
            aFileName = ::URIHelper::SmartRelToAbs( aFileNameRel, FALSE,
                                                    INetURLObject::WAS_ENCODED,
                                                    INetURLObject::DECODE_UNAMBIGUOUS );
        }
        else
            aFileName.Erase();

        // UNICODE: rIn >> aFilterName;
        rIn.ReadByteString(aFilterName);

        rIn >> bGraphicLink;                    // auch dieses Flag ist neu in V11

        if( aCompat.GetBytesLeft() > 0 )
        {
            SfxItemPool* pPool = GetItemPool();

            if( pPool )
            {
                sal_uInt16 nSetID = SDRATTRSET_GRAF;
                const SdrGrafSetItem* pGrafAttr = (const SdrGrafSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
                if(pGrafAttr)
                    SetObjectItemSet(pGrafAttr->GetItemSet());
                    ImpSetAttrToGrafInfo();
            }
            else
            {
                UINT16 nSuroDummy;
                rIn >> nSuroDummy;
            }
        }
        else
            bCopyToPoolOnAfterRead = TRUE;

        if( bGraphicLink && aFileName.Len() )
        {
            SetGraphicLink( aFileName, aFilterName );

            if( !bDelayedLoad )
                ImpUpdateGraphicLink();
        }
    }
}

// -----------------------------------------------------------------------------

#ifdef WNT
#pragma optimize ("",on)
#endif

// -----------------------------------------------------------------------------

//#111096#
//Rectangle SdrGrafObj::GetAnimationRect(const OutputDevice* pOutDev) const
//{
//  return GetSnapRect();
//}

// -----------------------------------------------------------------------------

//#111096#
//void SdrGrafObj::SetAnimationSupervisor( OutputDevice* pDisplayDev, BOOL bObjSupervises )
//{
//  ForceSwapIn();
//  List* pAInfoList = pGraphic->GetAnimationInfoList();
//
//  if ( pAInfoList )
//  {
//      for( AInfo* pAInfo = (AInfo*) pAInfoList->First(); pAInfo; pAInfo = (AInfo*) pAInfoList->Next() )
//      {
//          if( pAInfo->pOutDev == pDisplayDev )
//          {
//              pAInfo->nExtraData = ( bObjSupervises ? 1L : (long) this );
//
//              if ( !bObjSupervises )
//                  pAInfo->bPause = FALSE;
//          }
//      }
//  }
//}

// -----------------------------------------------------------------------------

//#111096#
//void SdrGrafObj::ResetAnimationLoopCount()
//{
//  if( pGraphic->IsAnimated() )
//  {
//      ForceSwapIn();
//      pGraphic->ResetAnimationLoopCount();
//  }
//}

// -----------------------------------------------------------------------------

SdrObject* SdrGrafObj::DoConvertToPolyObj(BOOL bBezier) const
{
    SdrObject* pRetval = NULL;

    switch( GetGraphicType() )
    {
        case GRAPHIC_GDIMETAFILE:
        {
            // NUR die aus dem MetaFile erzeugbaren Objekte in eine Gruppe packen und zurueckliefern
            SdrObjGroup*            pGrp = new SdrObjGroup();
            ImpSdrGDIMetaFileImport aFilter(*GetModel());
            Point                   aOutPos( aRect.TopLeft() );
            const Size              aOutSiz( aRect.GetSize() );

            aFilter.SetScaleRect(GetSnapRect());
            aFilter.SetLayer(GetLayer());

            UINT32 nInsAnz = aFilter.DoImport(GetTransformedGraphic().GetGDIMetaFile(), *pGrp->GetSubList(), 0);
            if(nInsAnz)
            {
                pRetval = pGrp;
                pGrp->NbcSetLayer(GetLayer());
                pGrp->SetModel(GetModel());
                pRetval = ImpConvertAddText(pRetval, bBezier);

                // convert all children
                if( pRetval )
                {
                    SdrObject* pHalfDone = pRetval;
                    pRetval = pHalfDone->DoConvertToPolyObj(bBezier);
                    delete pHalfDone; // resulting object is newly created

                    if( pRetval )
                    {
                        // flatten subgroups. As we call
                        // DoConvertToPolyObj() on the resulting group
                        // objects, subgroups can exist (e.g. text is
                        // a group object for every line).
                        SdrObjList* pList = pRetval->GetSubList();
                        if( pList )
                            pList->FlattenGroups();
                    }
                }
            }
            else
                delete pGrp;
            break;
        }
        case GRAPHIC_BITMAP:
        {
            // Grundobjekt kreieren und Fuellung ergaenzen
            pRetval = SdrRectObj::DoConvertToPolyObj(bBezier);

            // Bitmap als Attribut retten
            if(pRetval)
            {
                // Bitmap als Fuellung holen
                SfxItemSet aSet(GetObjectItemSet());

                aSet.Put(XFillStyleItem(XFILL_BITMAP));
                Bitmap aBitmap( GetTransformedGraphic().GetBitmap() );
                XOBitmap aXBmp(aBitmap, XBITMAP_STRETCH);
                aSet.Put(XFillBitmapItem(String(), aXBmp));
                aSet.Put(XFillBmpTileItem(FALSE));

                pRetval->SetMergedItemSet(aSet);
            }
            break;
        }
        case GRAPHIC_NONE:
        case GRAPHIC_DEFAULT:
        {
            pRetval = SdrRectObj::DoConvertToPolyObj(bBezier);
            break;
        }
    }

    return pRetval;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::AfterRead()
{
    SdrRectObj::AfterRead();

    if( bCopyToPoolOnAfterRead )
    {
        ImpSetGrafInfoToAttr();
        bCopyToPoolOnAfterRead = FALSE;
    }
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType )
{
    SetXPolyDirty();
    SdrRectObj::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
    ImpSetAttrToGrafInfo();
}

void SdrGrafObj::ImpSetAttrToGrafInfo()
{
    const SfxItemSet& rSet = GetObjectItemSet();
    const sal_uInt16 nTrans = ( (SdrGrafTransparenceItem&) rSet.Get( SDRATTR_GRAFTRANSPARENCE ) ).GetValue();
    const SdrGrafCropItem&  rCrop = (const SdrGrafCropItem&) rSet.Get( SDRATTR_GRAFCROP );

    aGrafInfo.SetLuminance( ( (SdrGrafLuminanceItem&) rSet.Get( SDRATTR_GRAFLUMINANCE ) ).GetValue() );
    aGrafInfo.SetContrast( ( (SdrGrafContrastItem&) rSet.Get( SDRATTR_GRAFCONTRAST ) ).GetValue() );
    aGrafInfo.SetChannelR( ( (SdrGrafRedItem&) rSet.Get( SDRATTR_GRAFRED ) ).GetValue() );
    aGrafInfo.SetChannelG( ( (SdrGrafGreenItem&) rSet.Get( SDRATTR_GRAFGREEN ) ).GetValue() );
    aGrafInfo.SetChannelB( ( (SdrGrafBlueItem&) rSet.Get( SDRATTR_GRAFBLUE ) ).GetValue() );
    aGrafInfo.SetGamma( ( (SdrGrafGamma100Item&) rSet.Get( SDRATTR_GRAFGAMMA ) ).GetValue() * 0.01 );
    aGrafInfo.SetTransparency( (BYTE) FRound( Min( nTrans, (USHORT) 100 )  * 2.55 ) );
    aGrafInfo.SetInvert( ( (SdrGrafInvertItem&) rSet.Get( SDRATTR_GRAFINVERT ) ).GetValue() );
    aGrafInfo.SetDrawMode( ( (SdrGrafModeItem&) rSet.Get( SDRATTR_GRAFMODE ) ).GetValue() );
    aGrafInfo.SetCrop( rCrop.GetLeft(), rCrop.GetTop(), rCrop.GetRight(), rCrop.GetBottom() );

    SetXPolyDirty();
    SetRectsDirty();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ImpSetGrafInfoToAttr()
{
    SetObjectItem( SdrGrafLuminanceItem( aGrafInfo.GetLuminance() ) );
    SetObjectItem( SdrGrafContrastItem( aGrafInfo.GetContrast() ) );
    SetObjectItem( SdrGrafRedItem( aGrafInfo.GetChannelR() ) );
    SetObjectItem( SdrGrafGreenItem( aGrafInfo.GetChannelG() ) );
    SetObjectItem( SdrGrafBlueItem( aGrafInfo.GetChannelB() ) );
    SetObjectItem( SdrGrafGamma100Item( FRound( aGrafInfo.GetGamma() * 100.0 ) ) );
    SetObjectItem( SdrGrafTransparenceItem( (USHORT) FRound( aGrafInfo.GetTransparency() / 2.55 ) ) );
    SetObjectItem( SdrGrafInvertItem( aGrafInfo.IsInvert() ) );
    SetObjectItem( SdrGrafModeItem( aGrafInfo.GetDrawMode() ) );
    SetObjectItem( SdrGrafCropItem( aGrafInfo.GetLeftCrop(), aGrafInfo.GetTopCrop(), aGrafInfo.GetRightCrop(), aGrafInfo.GetBottomCrop() ) );
}

// -----------------------------------------------------------------------------

void SdrGrafObj::AdjustToMaxRect( const Rectangle& rMaxRect, BOOL bShrinkOnly )
{
    Size aSize;
    Size aMaxSize( rMaxRect.GetSize() );
    if ( pGraphic->GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
        aSize = Application::GetDefaultDevice()->PixelToLogic( pGraphic->GetPrefSize(), MAP_100TH_MM );
    else
        aSize = OutputDevice::LogicToLogic( pGraphic->GetPrefSize(),
                                            pGraphic->GetPrefMapMode(),
                                            MapMode( MAP_100TH_MM ) );

    if( aSize.Height() != 0 && aSize.Width() != 0 )
    {
        Point aPos( rMaxRect.TopLeft() );

        // Falls Grafik zu gross, wird die Grafik
        // in die Seite eingepasst
        if ( (!bShrinkOnly                          ||
             ( aSize.Height() > aMaxSize.Height() ) ||
             ( aSize.Width()  > aMaxSize.Width()  ) )&&
             aSize.Height() && aMaxSize.Height() )
        {
            float fGrfWH =  (float)aSize.Width() /
                            (float)aSize.Height();
            float fWinWH =  (float)aMaxSize.Width() /
                            (float)aMaxSize.Height();

            // Grafik an Pagesize anpassen (skaliert)
            if ( fGrfWH < fWinWH )
            {
                aSize.Width() = (long)(aMaxSize.Height() * fGrfWH);
                aSize.Height()= aMaxSize.Height();
            }
            else if ( fGrfWH > 0.F )
            {
                aSize.Width() = aMaxSize.Width();
                aSize.Height()= (long)(aMaxSize.Width() / fGrfWH);
            }

            aPos = rMaxRect.Center();
        }

        if( bShrinkOnly )
            aPos = aRect.TopLeft();

        aPos.X() -= aSize.Width() / 2;
        aPos.Y() -= aSize.Height() / 2;
        SetLogicRect( Rectangle( aPos, aSize ) );
    }
}

// -----------------------------------------------------------------------------

IMPL_LINK( SdrGrafObj, ImpSwapHdl, GraphicObject*, pO )
{
    SvStream* pRet = GRFMGR_AUTOSWAPSTREAM_NONE;

    if( pO->IsInSwapOut() )
    {
        if( pModel && pModel->IsSwapGraphics() && pGraphic->GetSizeBytes() > 20480 )
        {
            // test if this object is visualized from someone
            if(!GetViewContact().HasViewObjectContacts())
            {
//          SdrViewIter aIter( this );
//          SdrView*    pView = aIter.FirstView();
//          BOOL        bVisible = FALSE;
//
//          while( !bVisible && pView )
//          {
//              bVisible = !pView->IsGrafDraft();
//
//              if( !bVisible )
//                  pView = aIter.NextView();
//          }
//
//          if( !bVisible )
//          {
                const ULONG nSwapMode = pModel->GetSwapGraphicsMode();

                if( ( ( GRAFSTREAMPOS_INVALID != nGrafStreamPos ) || pGraphic->HasUserData() || pGraphicLink ) &&
                    ( nSwapMode & SDR_SWAPGRAPHICSMODE_PURGE ) )
                {
                    pRet = NULL;
                }
                else if( nSwapMode & SDR_SWAPGRAPHICSMODE_TEMP )
                {
                    pRet = GRFMGR_AUTOSWAPSTREAM_TEMP;
                    pGraphic->SetUserData();
                    nGrafStreamPos = GRAFSTREAMPOS_INVALID;
                }
            }
        }
    }
    else if( pO->IsInSwapIn() )
    {
        // kann aus dem original Doc-Stream nachgeladen werden...
        if( pModel != NULL )
        {
            if( ( GRAFSTREAMPOS_INVALID != nGrafStreamPos ) || pGraphic->HasUserData() )
            {
                SdrDocumentStreamInfo aStreamInfo;

                aStreamInfo.mbDeleteAfterUse = FALSE;
                aStreamInfo.maUserData = pGraphic->GetUserData();
                aStreamInfo.mpStorageRef = NULL;

                SvStream* pStream = pModel->GetDocumentStream( aStreamInfo );

                if( pStream != NULL )
                {
                    Graphic aGraphic;

                    if( pGraphic->HasUserData() )
                    {
                        if( !GetGrfFilter()->ImportGraphic( aGraphic, String(), *pStream ) )
                        {
                            const String aUserData( pGraphic->GetUserData() );

                            pGraphic->SetGraphic( aGraphic );
                            pGraphic->SetUserData( aUserData );
                            pRet = GRFMGR_AUTOSWAPSTREAM_LOADED;
                        }
                    }
                    else
                    {
                        pStream->Seek( nGrafStreamPos );
                        *pStream >> aGraphic;
                        pGraphic->SetGraphic( aGraphic );

                        if( !pStream->GetError() )
                            pRet = GRFMGR_AUTOSWAPSTREAM_LOADED;
                    }

                    pStream->ResetError();

                    if( aStreamInfo.mbDeleteAfterUse || aStreamInfo.mpStorageRef )
                    {
                        delete pStream;
                        delete aStreamInfo.mpStorageRef;
                    }
                }
            }
            else if( !ImpUpdateGraphicLink() )
            {
                pRet = GRFMGR_AUTOSWAPSTREAM_TEMP;
            }
            else
            {
                pRet = GRFMGR_AUTOSWAPSTREAM_LOADED;
            }
        }
        else
            pRet = GRFMGR_AUTOSWAPSTREAM_TEMP;
    }

    return (long)(void*) pRet;
}

// -----------------------------------------------------------------------------

// #111096#
// Access to GrafAnimationAllowed flag
sal_Bool SdrGrafObj::IsGrafAnimationAllowed() const
{
    return mbGrafAnimationAllowed;
}

void SdrGrafObj::SetGrafAnimationAllowed(sal_Bool bNew)
{
    if(mbGrafAnimationAllowed != bNew)
    {
        mbGrafAnimationAllowed = bNew;
        ActionChanged();
    }
}

// #i25616#
sal_Bool SdrGrafObj::IsObjectTransparent() const
{
    if(((const SdrGrafTransparenceItem&)GetObjectItem(SDRATTR_GRAFTRANSPARENCE)).GetValue()
        || IsTransparent())
    {
        return sal_True;
    }

    return sal_False;
}

// eof
