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

#define _ANIMATION
#define ITEMID_GRF_CROP 0

#ifndef SVX_LIGHT
#include <bf_so3/lnkbase.hxx>
#else
#endif

#include <math.h>
#include <vcl/salbtype.hxx>
#include <sot/formats.hxx>
#include <bf_so3/svstor.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <bf_svtools/style.hxx>
#include <bf_svtools/urihelper.hxx>
#include "linkmgr.hxx"
#include "svdio.hxx"
#include "svdglob.hxx"
#include "svdstr.hrc"
#include "svdpool.hxx"
#include "svdpagv.hxx"
#include "svdviter.hxx"
#include "svdview.hxx"
#include "impgrf.hxx"
#include "svdograf.hxx"
#include "sdgcpitm.hxx"

#ifndef _EEITEM_HXX
#include "eeitem.hxx"
#endif

#ifndef _XOUTX_HXX
#include "xoutx.hxx"
#endif

#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

// -----------
// - Defines -
// -----------

#define GRAFSTREAMPOS_INVALID 0xffffffff

#ifndef SVX_LIGHT

// ------------------
// - SdrGraphicLink	-
// ------------------

/*N*/ class SdrGraphicLink : public ::binfilter::SvBaseLink
/*N*/ {
/*N*/ 	SdrGrafObj*			pGrafObj;
/*N*/ 
/*N*/ public:
/*N*/ 						SdrGraphicLink(SdrGrafObj* pObj);
/*N*/ 	virtual				~SdrGraphicLink();
/*N*/ 
/*N*/ 	virtual void		Closed();
/*N*/ 	virtual void		DataChanged( const String& rMimeType,
/*N*/ 								const ::com::sun::star::uno::Any & rValue );
/*N*/ 
/*N*/ 	BOOL				Connect() { return 0 != GetRealObject(); }
/*N*/ 	void				UpdateSynchron();
/*N*/ };

// -----------------------------------------------------------------------------

/*N*/ SdrGraphicLink::SdrGraphicLink(SdrGrafObj* pObj):
/*N*/ 	::binfilter::SvBaseLink( ::binfilter::LINKUPDATE_ONCALL, SOT_FORMATSTR_ID_SVXB ),
/*N*/ 	pGrafObj(pObj)
/*N*/ {
/*N*/ 	SetSynchron( FALSE );
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SdrGraphicLink::~SdrGraphicLink()
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGraphicLink::DataChanged( const String& rMimeType,
/*N*/ 								const ::com::sun::star::uno::Any & rValue )
/*N*/ {
/*N*/ 	SdrModel*       pModel      = pGrafObj ? pGrafObj->GetModel() : 0;
/*N*/ 	SvxLinkManager* pLinkManager= pModel  ? pModel->GetLinkManager() : 0;
/*N*/ 
/*N*/ 	if( pLinkManager && rValue.hasValue() )
/*N*/ 	{
/*N*/ 		pLinkManager->GetDisplayNames( this, 0, &pGrafObj->aFileName, 0, &pGrafObj->aFilterName );
/*N*/ 
/*N*/ 		Graphic aGraphic;
/*N*/ 		if( SvxLinkManager::GetGraphicFromAny( rMimeType, rValue, aGraphic ))
/*N*/ 		{
/*?*/ 			GraphicType eOldGraphicType = pGrafObj->GetGraphicType();  // kein Hereinswappen
/*?*/ 			BOOL bIsChanged = pModel->IsChanged();
/*?*/ 
/*?*/ 			pGrafObj->SetGraphic( aGraphic );
/*?*/ 			if( GRAPHIC_NONE != eOldGraphicType )
/*?*/ 				pGrafObj->SetChanged();
/*?*/ 			else
/*?*/ 				pModel->SetChanged( bIsChanged );
/*N*/ 		}
/*N*/ 		else if( SotExchange::GetFormatIdFromMimeType( rMimeType ) !=
/*N*/ 					SvxLinkManager::RegisterStatusInfoId() )
/*?*/ 			pGrafObj->SendRepaintBroadcast();
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGraphicLink::Closed()
/*N*/ {
/*N*/ 	// Die Verbindung wird aufgehoben; pLink des Objekts auf NULL setzen, da die Link-Instanz ja gerade destruiert wird.
/*N*/ 	pGrafObj->ForceSwapIn();
/*N*/ 	pGrafObj->pGraphicLink=NULL;
/*N*/ 	pGrafObj->ReleaseGraphicLink();
/*N*/ 	SvBaseLink::Closed();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGraphicLink::UpdateSynchron()
/*N*/ {
/*N*/ 	if( GetObj() )
/*N*/ 	{
/*N*/ 		String aMimeType( SotExchange::GetFormatMimeType( GetContentType() ));
/*N*/ 		::com::sun::star::uno::Any aValue;
/*N*/ 		GetObj()->GetData( aValue, aMimeType, TRUE );
/*N*/ 		DataChanged( aMimeType, aValue );
/*N*/ 	}
/*N*/ }

#else

/*?*/ GraphicFilter* SVX_LIGHT_pGrapicFilter = NULL;
/*?*/ 
/*?*/ GraphicFilter* GetGrfFilter()
/*?*/ {
/*?*/ 	if( !SVX_LIGHT_pGrapicFilter )
/*?*/     {
/*?*/ 		const SvtPathOptions aPathOptions;
/*?*/ 
/*?*/         SVX_LIGHT_pGrapicFilter = new GraphicFilter( FALSE );
/*?*/         SVX_LIGHT_pGrapicFilter->SetFilterPath( aPathOptions.GetFilterPath() );
/*?*/     }
/*?*/     
/*?*/ 	return SVX_LIGHT_pGrapicFilter;
/*?*/ }


#endif // SVX_LIGHT

// --------------
// - SdrGrafObj -
// --------------

/*N*/ TYPEINIT1(SdrGrafObj,SdrRectObj);

// -----------------------------------------------------------------------------

/*N*/ SdrGrafObj::SdrGrafObj():
/*N*/ 	bMirrored		( FALSE ),
/*N*/ 	pGraphicLink	( NULL )
/*N*/ {
/*N*/ 	pGraphic = new BfGraphicObject;
/*N*/ 	pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), 20000 );
/*N*/ 	nGrafStreamPos = GRAFSTREAMPOS_INVALID;
/*N*/ 	bNoShear = TRUE;
/*N*/ 	bCopyToPoolOnAfterRead = FALSE;
/*N*/ }

// -----------------------------------------------------------------------------

/*?*/ SdrGrafObj::SdrGrafObj( const Graphic& rGrf ):
/*?*/ 	bMirrored		( FALSE ),
/*?*/ 	pGraphicLink	( NULL )
/*?*/ {
/*?*/ 	pGraphic = new BfGraphicObject( rGrf );
/*?*/ 	pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), 20000 );
/*?*/ 	nGrafStreamPos = GRAFSTREAMPOS_INVALID;
/*?*/ 	bNoShear = TRUE;
/*?*/ 	bCopyToPoolOnAfterRead = FALSE;
/*?*/ }

// -----------------------------------------------------------------------------

/*N*/ SdrGrafObj::~SdrGrafObj()
/*N*/ {
/*N*/ 	delete pGraphic;
/*N*/ 	ImpLinkAbmeldung();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::SetGraphicObject( const BfGraphicObject& rGrfObj )
/*N*/ {
/*N*/ 	*pGraphic = rGrfObj;
/*N*/ 	pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), 20000 );
/*N*/ 	pGraphic->SetUserData();
/*N*/ 	nGrafStreamPos = GRAFSTREAMPOS_INVALID;
/*N*/ 	SetChanged();
/*N*/ 	SendRepaintBroadcast();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ const BfGraphicObject& SdrGrafObj::GetGraphicObject() const
/*N*/ {
/*N*/ 	ForceSwapIn();
/*N*/ 	return *pGraphic;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::SetGraphic( const Graphic& rGrf )
/*N*/ {
/*N*/ 	pGraphic->SetGraphic( rGrf );
/*N*/ 	pGraphic->SetUserData();
/*N*/ 	nGrafStreamPos = GRAFSTREAMPOS_INVALID;
/*N*/ 	SetChanged();
/*N*/ 	SendRepaintBroadcast();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ const Graphic& SdrGrafObj::GetGraphic() const
/*N*/ {
/*N*/ 	ForceSwapIn();
/*N*/ 	return pGraphic->GetGraphic();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ GraphicType SdrGrafObj::GetGraphicType() const
/*N*/ {
/*N*/ 	return pGraphic->GetType();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::SetGrafStreamURL( const String& rGraphicStreamURL )
/*N*/ {
/*N*/ 	if( !rGraphicStreamURL.Len() )
/*N*/ 	{
/*N*/ 		pGraphic->SetUserData();
/*N*/ 		nGrafStreamPos = GRAFSTREAMPOS_INVALID;
/*N*/ 	}
/*N*/ 	else if( pModel->IsSwapGraphics() )
/*N*/ 	{
/*N*/ 		pGraphic->SetUserData( rGraphicStreamURL );
/*N*/ 		nGrafStreamPos = GRAFSTREAMPOS_INVALID;
/*N*/ 
/*N*/ 		// set state of graphic object to 'swapped out'
/*N*/ 		if( pGraphic->GetType() == GRAPHIC_NONE )
/*?*/ 			pGraphic->SetSwapState();
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ String SdrGrafObj::GetGrafStreamURL() const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); String aString; return aString;//STRIP001 
/*N*/ }

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::SetName(const XubString& rStr)
/*N*/ {
/*N*/ 	aName = rStr;
/*N*/ 	SetChanged();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ XubString SdrGrafObj::GetName() const
/*N*/ {
/*N*/ 	return aName;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::ForceSwapIn() const
/*N*/ {
/*N*/ 	pGraphic->FireSwapInRequest();
/*N*/ 
/*N*/ 	if( pGraphic->IsSwappedOut() ||
/*N*/ 	    ( pGraphic->GetType() == GRAPHIC_NONE ) ||
/*N*/ 		( pGraphic->GetType() == GRAPHIC_DEFAULT ) )
/*N*/ 	{
/*N*/ 		Graphic aDefaultGraphic;
/*N*/ 		aDefaultGraphic.SetDefaultType();
/*N*/ 		pGraphic->SetGraphic( aDefaultGraphic );
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::ForceSwapOut() const
/*N*/ {
/*N*/ 	pGraphic->FireSwapOutRequest();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::ImpLinkAnmeldung()
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 
/*N*/ 	SvxLinkManager* pLinkManager = pModel != NULL ? pModel->GetLinkManager() : NULL;
/*N*/ 
/*N*/ 	if( pLinkManager != NULL && pGraphicLink == NULL )
/*N*/ 	{
/*N*/ 		if( aFileName.Len() )
/*N*/ 		{
/*N*/ 			pGraphicLink = new SdrGraphicLink( this );
/*N*/ 			pLinkManager->InsertFileLink( *pGraphicLink, OBJECT_CLIENT_GRF, aFileName, ( aFilterName.Len() ? &aFilterName : NULL ), NULL );
/*N*/ 			pGraphicLink->Connect();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ #endif // SVX_LIGHT
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::ImpLinkAbmeldung()
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 
/*N*/ 	SvxLinkManager* pLinkManager = pModel != NULL ? pModel->GetLinkManager() : NULL;
/*N*/ 
/*N*/ 	if( pLinkManager != NULL && pGraphicLink!=NULL)
/*N*/ 	{
/*?*/ 		// Bei Remove wird *pGraphicLink implizit deleted
/*?*/ 		pLinkManager->Remove( pGraphicLink );
/*?*/ 		pGraphicLink=NULL;
/*N*/ 	}
/*N*/ 
/*N*/ #endif // SVX_LIGHT
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::SetGraphicLink( const String& rFileName, const String& rFilterName )
/*N*/ {
/*N*/ 	ImpLinkAbmeldung();
/*N*/ 	aFileName = rFileName;
/*N*/ 	aFilterName = rFilterName;
/*N*/ 	ImpLinkAnmeldung();
/*N*/ 	pGraphic->SetUserData();
/*N*/ 
/*N*/     // #92205# A linked graphic is per definition swapped out (has to be loaded)
/*N*/     pGraphic->SetSwapState();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::ReleaseGraphicLink()
/*N*/ {
/*N*/ 	ImpLinkAbmeldung();
/*N*/ 	aFileName = String();
/*N*/ 	aFilterName = String();
/*N*/ }

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*N*/ UINT16 SdrGrafObj::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return UINT16( OBJ_GRAF );
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ sal_Bool SdrGrafObj::ImpUpdateGraphicLink() const
/*N*/ {
/*N*/     sal_Bool	bRet = sal_False;
/*N*/ 
/*N*/ #ifndef SVX_LIGHT    
/*N*/     if( pGraphicLink )
/*N*/     {
/*N*/         BOOL bIsChanged = pModel->IsChanged();
/*N*/         pGraphicLink->UpdateSynchron();
/*N*/         pModel->SetChanged( bIsChanged );
/*N*/ 
/*N*/         bRet = sal_True;
/*N*/     }
/*N*/ #else
/*N*/     if( aFileName.Len() )
/*N*/     {
/*N*/         // #92205# Load linked graphics for player
/*N*/         SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aFileName, STREAM_READ | STREAM_SHARE_DENYNONE );
/*N*/         
/*N*/         if( pIStm )
/*N*/         {
/*N*/             Graphic			aGraphic;
/*N*/             GraphicFilter*	pFilter = GetGrfFilter();
/*N*/             USHORT			nError = pFilter->ImportGraphic( aGraphic, aFileName, *pIStm );
/*N*/             
/*N*/             pGraphic->SetGraphic( aGraphic );
/*N*/             
/*N*/             delete pIStm;
/*N*/         }
/*N*/             
/*N*/         bRet = sal_True;
/*N*/     }
/*N*/ #endif
/*N*/ 
/*N*/     return bRet;
/*N*/ }

// -----------------------------------------------------------------------------

// Liefert FALSE, wenn die Pres-Bitmap zu gross ist

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::ImpPaintReplacement(OutputDevice* pOutDev, const XubString& rText, const Bitmap* pBmp, FASTBOOL bFill) const
/*N*/ {
/*N*/     Size aPixelSize( 1, 1 );
/*N*/     Size aBmpSize;
/*N*/     
/*N*/     aPixelSize = Application::GetDefaultDevice()->PixelToLogic( aPixelSize, pOutDev->GetMapMode() );
/*N*/ 
/*N*/ 	if( bFill )
/*N*/ 	{
/*?*/ 		pOutDev->SetLineColor();
/*?*/ 		pOutDev->SetFillColor( COL_LIGHTGRAY );
/*N*/ 	}
/*N*/ 
/*N*/ 	Rectangle   aRect1( aRect );
/*N*/ 	Rectangle   aRect2( aRect1 );
/*N*/ 	Rectangle   aTextRect( aRect1 );
/*N*/ 	Point       aTopLeft( aRect1.TopLeft() );
/*N*/ 	Point       aBmpPos( aTopLeft );
/*N*/ 	
/*N*/     aRect2.Left() += aPixelSize.Width();
/*N*/     aRect2.Top() += aPixelSize.Height();
/*N*/     aRect2.Right() -= aPixelSize.Width();
/*N*/     aRect2.Bottom() -= aPixelSize.Height();
/*N*/ 
/*N*/ 	if( pBmp != NULL )
/*N*/ 	{
/*N*/ 		aBmpSize = Size( Application::GetDefaultDevice()->PixelToLogic( pBmp->GetSizePixel(), pOutDev->GetMapMode() ) );
/*N*/ 
/*N*/ 		long		nRectWdt = aTextRect.Right() - aTextRect.Left();
/*N*/ 		long		nRectHgt = aTextRect.Bottom() - aTextRect.Top();
/*N*/ 		long		nBmpWdt = aBmpSize.Width();
/*N*/ 		long		nBmpHgt = aBmpSize.Height();
/*N*/ 		long        nMinWdt = nBmpWdt;
/*N*/ 		long        nMinHgt = nBmpHgt;
/*N*/ 		BOOL		bText = rText.Len() > 0;
/*N*/ 
/*N*/ 		if( bText )
/*N*/ 		{
/*N*/ 			nMinWdt= 2 * nBmpWdt + 5 * aPixelSize.Width();
/*N*/ 			nMinHgt= 2 * nBmpHgt + 5 * aPixelSize.Height();
/*N*/ 		}
/*N*/ 
/*N*/ 		if( nRectWdt < nMinWdt || nRectHgt < nMinHgt )
/*N*/ 			pBmp=NULL;
/*N*/ 		else
/*N*/ 		{
/*N*/ 			aTextRect.Left() += nBmpWdt;
/*N*/ 
/*N*/ 			if( bText )
/*N*/ 				aTextRect.Left() += 5 * aPixelSize.Width();
/*N*/ 		}
/*N*/ 
/*N*/ 		aBmpPos.X() += 2 * aPixelSize.Width();
/*N*/ 		aBmpPos.Y() += 2 * aPixelSize.Height();
/*N*/ 
/*N*/ 		if( aGeo.nDrehWink != 0 )
/*N*/ 		{
/*N*/ 			Point   aRef( aBmpPos.X() - aBmpSize.Width() / 2 + 2 * aPixelSize.Width(),
/*N*/ 			              aBmpPos.Y() - aBmpSize.Height() / 2 + 2 * aPixelSize.Height() );
/*N*/ 			double  nSin = sin( aGeo.nDrehWink * nPi180 );
/*N*/ 			double  nCos = cos( aGeo.nDrehWink * nPi180 );
/*N*/ 			
/*N*/ 			RotatePoint( aBmpPos, aRef, nSin, nCos );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( aGeo.nDrehWink == 0 && aGeo.nShearWink == 0 )
/*N*/ 	{
/*N*/ 		const StyleSettings& rStyleSettings = pOutDev->GetSettings().GetStyleSettings();
/*N*/ 
/*N*/ 		if( bFill )
/*N*/ 			pOutDev->DrawRect( aRect );
/*N*/ 
/*N*/ 		if( pBmp!=NULL )
/*N*/ 			pOutDev->DrawBitmap( aBmpPos, aBmpSize, *pBmp );
/*N*/ 
/*N*/ 		pOutDev->SetFillColor();
/*N*/ 		pOutDev->SetLineColor( rStyleSettings.GetShadowColor() );
/*N*/ 		pOutDev->DrawLine( aRect1.TopLeft(), aRect1.TopRight() );
/*N*/ 		pOutDev->DrawLine( aRect1.TopLeft(), aRect1.BottomLeft() );
/*N*/ 
/*N*/ 		pOutDev->SetLineColor( rStyleSettings.GetLightColor() );
/*N*/ 		pOutDev->DrawLine( aRect1.TopRight(), aRect1.BottomRight() );
/*N*/ 		pOutDev->DrawLine( aRect1.BottomLeft(), aRect1.BottomRight() );
/*N*/ 
/*N*/ 		pOutDev->SetLineColor( rStyleSettings.GetLightColor() );
/*N*/ 		pOutDev->DrawLine( aRect2.TopLeft(), aRect2.TopRight() );
/*N*/ 		pOutDev->DrawLine( aRect2.TopLeft(), aRect2.BottomLeft() );
/*N*/ 
/*N*/ 		pOutDev->SetLineColor( rStyleSettings.GetShadowColor() );
/*N*/ 		pOutDev->DrawLine( aRect2.TopRight(), aRect2.BottomRight() );
/*N*/ 		pOutDev->DrawLine( aRect2.BottomLeft(), aRect2.BottomRight() );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		Polygon aPoly1( Rect2Poly( aRect1, aGeo ) );
/*N*/ 		Polygon aPoly2(5);
/*N*/ 		
/*N*/ 		aPoly2[0] = aRect2.TopLeft();
/*N*/ 		aPoly2[1] = aRect2.TopRight();
/*N*/ 		aPoly2[2] = aRect2.BottomRight();
/*N*/ 		aPoly2[3] = aRect2.BottomLeft();
/*N*/ 		aPoly2[4] = aRect2.TopLeft();
/*N*/ 
/*N*/ 		if( aGeo.nShearWink != 0 )
/*?*/ 			ShearPoly( aPoly2, aTopLeft, aGeo.nTan );
/*N*/ 
/*N*/ 		if( aGeo.nDrehWink != 0 )
/*N*/ 			RotatePoly( aPoly2, aTopLeft, aGeo.nSin, aGeo.nCos );
/*N*/ 
/*N*/ 		if( bFill )
/*?*/ 			pOutDev->DrawPolygon( aPoly1 );
/*N*/ 
/*N*/ 		if( pBmp != NULL )
/*N*/ 			pOutDev->DrawBitmap( aBmpPos, aBmpSize, *pBmp );
/*N*/ 
/*N*/ 		pOutDev->SetFillColor();
/*N*/ 
/*N*/ 		const StyleSettings&    rStyleSettings = pOutDev->GetSettings().GetStyleSettings();
/*N*/ 		Color                   a3DLightColor( rStyleSettings.GetLightColor() );
/*N*/ 		Color                   a3DShadowColor( rStyleSettings.GetShadowColor() );
/*N*/ 		long		            nHWink=NormAngle360( aGeo.nDrehWink );
/*N*/ 		long		            nVWink=NormAngle360( aGeo.nDrehWink-aGeo.nShearWink );
/*N*/ 		FASTBOOL	            bHorzChg=nHWink>13500 && nHWink<=31500;
/*N*/ 		FASTBOOL	            bVertChg=nVWink>4500 && nVWink<=22500;
/*N*/ 
/*N*/ 		pOutDev->SetLineColor( bHorzChg ? a3DShadowColor : a3DLightColor);
/*N*/ 		pOutDev->DrawLine( aPoly2[0], aPoly2[1] );
/*N*/ 
/*N*/ 		pOutDev->SetLineColor( bHorzChg ? a3DLightColor  : a3DShadowColor);
/*N*/ 		pOutDev->DrawLine( aPoly2[2], aPoly2[3] );
/*N*/ 
/*N*/ 		pOutDev->SetLineColor( bVertChg ? a3DLightColor  : a3DShadowColor);
/*N*/ 		pOutDev->DrawLine( aPoly2[1], aPoly2[2] );
/*N*/ 
/*N*/ 		pOutDev->SetLineColor( bVertChg ? a3DShadowColor : a3DLightColor);
/*N*/ 		pOutDev->DrawLine( aPoly2[3], aPoly2[4] );
/*N*/ 
/*N*/ 		pOutDev->SetLineColor( bHorzChg ? a3DLightColor  : a3DShadowColor);
/*N*/ 		pOutDev->DrawLine( aPoly1[0], aPoly1[1] );
/*N*/ 
/*N*/ 		pOutDev->SetLineColor( bHorzChg ? a3DShadowColor : a3DLightColor);
/*N*/ 		pOutDev->DrawLine( aPoly1[2], aPoly1[3] );
/*N*/ 
/*N*/ 		pOutDev->SetLineColor( bVertChg ? a3DShadowColor : a3DLightColor);
/*N*/ 		pOutDev->DrawLine( aPoly1[1], aPoly1[2] );
/*N*/ 
/*N*/ 		pOutDev->SetLineColor( bVertChg ? a3DLightColor  : a3DShadowColor);
/*N*/ 		pOutDev->DrawLine( aPoly1[3], aPoly1[4] );
/*N*/ 	}
/*N*/ 	
/*N*/ 	XubString aNam( rText );
/*N*/ 
/*N*/ 	if( aNam.Len() )
/*N*/ 	{
/*?*/ 		Size aOutSize( aTextRect.GetWidth() - 6 * aPixelSize.Width(),
/*?*/ 		               aTextRect.GetHeight() - 6 * aPixelSize.Height() ); 
/*?*/ 		
/*?*/ 		if( aOutSize.Width() >= ( 4 * aPixelSize.Width() ) || 
/*?*/ 		    aOutSize.Height() >=  ( 4 * aPixelSize.Height() ) )
/*?*/ 		{
/*?*/ 			Point   aOutPos( aTextRect.Left() + 3 * aPixelSize.Width(),
/*?*/ 			                 aTextRect.Top() + 3 * aPixelSize.Height() );
/*?*/ 			long    nMaxOutY = aOutPos.Y() + aOutSize.Height();
/*?*/ 			Font    aFontMerk( pOutDev->GetFont() );
/*?*/ 			Font    aFont( OutputDevice::GetDefaultFont( DEFAULTFONT_SANS_UNICODE, LANGUAGE_SYSTEM, DEFAULTFONT_FLAGS_ONLYONE ) );
/*?*/ 			
/*?*/ 			aFont.SetColor( COL_LIGHTRED );
/*?*/ 			aFont.SetTransparent( TRUE );
/*?*/ 			aFont.SetOrientation( USHORT( NormAngle360( aGeo.nDrehWink ) / 10 ) );
/*?*/ 
/*?*/ 			if( IsLinkedGraphic() )
/*?*/ 				aFont.SetUnderline( UNDERLINE_SINGLE );
/*?*/ 			
/*?*/ 			Size aFontSize( 0, ( aGeo.nDrehWink % 9000 == 0 ? 12 : 14 ) * aPixelSize.Height() );
/*?*/ 
/*?*/ 			if( aFontSize.Height() > aOutSize.Height() )
/*?*/ 				aFontSize.Height() = aOutSize.Height();
/*?*/ 
/*?*/ 			aFont.SetSize( aFontSize );
/*?*/ 			pOutDev->SetFont( aFont );
/*?*/ 			String aOutStr( aNam );
/*?*/ 
/*?*/ 			while( aOutStr.Len() && aOutPos.Y() <= nMaxOutY )
/*?*/ 			{
/*?*/ 				String  aStr1( aOutStr );
/*?*/ 				INT32   nTextWidth = pOutDev->GetTextWidth( aStr1 );
/*?*/ 				INT32   nTextHeight = pOutDev->GetTextHeight();
/*?*/ 
/*?*/ 				while( aStr1.Len() && nTextWidth > aOutSize.Width() )
/*?*/ 				{
/*?*/ 					aStr1.Erase( aStr1.Len() - 1 );
/*?*/ 					nTextWidth = pOutDev->GetTextWidth( aStr1 );
/*?*/ 					nTextHeight = pOutDev->GetTextHeight();
/*?*/ 				}
/*?*/ 				
/*?*/ 				Point aPos( aOutPos );
/*?*/ 				aOutPos.Y() += nTextHeight;
/*?*/ 
/*?*/ 				if( aOutPos.Y() <= nMaxOutY )
/*?*/ 				{
/*?*/ 					if( aGeo.nShearWink != 0 )
/*?*/ 					    ShearPoint( aPos, aTopLeft, aGeo.nTan );
/*?*/ 					    
/*?*/ 					if( aGeo.nDrehWink != 0 )
/*?*/ 					    RotatePoint( aPos, aTopLeft, aGeo.nSin, aGeo.nCos );
/*?*/ 					
/*?*/ 					pOutDev->DrawText( aPos, aStr1 );
/*?*/ 					aOutStr.Erase( 0, aStr1.Len() );
/*?*/ 				}
/*?*/ 			}
/*?*/ 			
/*?*/ 			pOutDev->SetFont( aFontMerk );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ FASTBOOL SdrGrafObj::Paint( ExtOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec ) const
/*N*/ {
/*N*/ 	// Hidden objects on masterpages, draw nothing
/*N*/ 	if( ( ( rInfoRec.nPaintMode & SDRPAINTMODE_MASTERPAGE ) && bNotVisibleAsMaster ) ||
/*N*/ 		( ( OUTDEV_PRINTER == rOut.GetOutDev()->GetOutDevType() ) && bEmptyPresObj ) )
/*N*/ 	{
/*?*/ 		return TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	FASTBOOL		bDraft = ( 0 != ( rInfoRec.nPaintMode & SDRPAINTMODE_DRAFTGRAF ) );
/*N*/ 	FASTBOOL		bSwappedOut = pGraphic->IsSwappedOut() || ( pGraphic->GetType() == GRAPHIC_NONE );
/*N*/ 	FASTBOOL		bLoading = FALSE;
/*N*/ 	OutputDevice*	pOutDev = rOut.GetOutDev();
/*N*/ 	GDIMetaFile*	pRecMetaFile = pOutDev->GetConnectMetaFile();
/*N*/ 	FASTBOOL		bMtfRecording = ( pRecMetaFile && pRecMetaFile->IsRecord() && !pRecMetaFile->IsPause() );
/*N*/ 	const SdrView*	pView = ( rInfoRec.pPV ? &rInfoRec.pPV->GetView() : NULL );
/*N*/ 
/*N*/ 	if( bSwappedOut && !bDraft )
/*N*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}

/*N*/ 	if( pGraphic->IsSwappedOut() ||	( pGraphic->GetType() == GRAPHIC_NONE ) || ( pGraphic->GetType() == GRAPHIC_DEFAULT ) )
/*?*/ 		bDraft=TRUE;
/*N*/ 
/*N*/ 	long          nDrehWink = aGeo.nDrehWink, nShearWink = aGeo.nShearWink;
/*N*/ 	FASTBOOL      bRotate = ( nDrehWink != 0 && nDrehWink != 18000 );
/*N*/ 	FASTBOOL      bShear = ( nShearWink != 0 );
/*N*/ 	FASTBOOL      bRota180 = nDrehWink == 18000;
/*N*/ 	USHORT        nMirrorCase = ( bRota180 ? ( bMirrored ? 3 : 4 ) : ( bMirrored ? 2 : 1 ) );	//  4 | 3   H&V gespiegelt | nur Vertikal
/*N*/ 	FASTBOOL      bHMirr = ( ( 2 == nMirrorCase ) || ( 4 == nMirrorCase ) );					// ---+---  ---------------+-----------------
/*N*/ 	FASTBOOL      bVMirr = ( ( 3 == nMirrorCase ) || ( 4 == nMirrorCase ) );					//  2 | 1   nur Horizontal | nicht gespiegelt
/*N*/ 
/*N*/ 	if( !bEmptyPresObj && !bDraft )
/*N*/ 	{
/*?*/ 		Point		aLogPos( aRect.TopLeft() );
/*?*/ 		Size		aLogSize( pOutDev->PixelToLogic( pOutDev->LogicToPixel( aRect ).GetSize() ) );
/*?*/ 		BfGraphicAttr aAttr( aGrafInfo );
/*?*/ 		const ULONG nGraphicManagerDrawMode = ( pView ? pView->GetGraphicManagerDrawMode() : GRFMGR_DRAW_STANDARD );
/*?*/ 
/*?*/ 		aAttr.SetMirrorFlags( ( bHMirr ? BMP_MIRROR_HORZ : 0 ) | ( bVMirr ? BMP_MIRROR_VERT : 0 ) );
/*?*/ 
/*?*/ 		if( bRota180 )
/*?*/ 		{
/*?*/ 			aLogPos.X() -= ( aLogSize.Width() - 1L );
/*?*/ 			aLogPos.Y() -= ( aLogSize.Height() - 1L );
/*?*/ 		}
/*?*/ 
/*?*/         bool bDidPaint( false );
/*?*/         
/*?*/ 		if( pGraphic->GetType() == GRAPHIC_BITMAP )
/*?*/ 		{
/*?*/ 			if( pGraphic->IsAnimated() )
/*?*/ 			{
/*?*/ 				SdrAnimationMode    eAnimMode = SDR_ANIMATION_ANIMATE;
/*?*/ 				FASTBOOL            bEnable = TRUE;
/*?*/ 
/*?*/ 				if( pView )
/*?*/ 				{
/*?*/ 					eAnimMode= pView->GetAnimationMode();
/*?*/ 					bEnable = ( eAnimMode != SDR_ANIMATION_DISABLE );
/*?*/     				
/*?*/     				if( bEnable )
/*?*/ 				    {{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 
/*?*/ 				    }
/*?*/ 				}
/*?*/ 
/*?*/ 				if( bEnable )
/*?*/ 				{
/*?*/ 					if( eAnimMode == SDR_ANIMATION_ANIMATE )
/*?*/ 					{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ 					}
/*?*/ 					else if( eAnimMode == SDR_ANIMATION_DONT_ANIMATE )
/*?*/ 						pGraphic->Draw( pOutDev, aLogPos, aLogSize, &aAttr, nGraphicManagerDrawMode );
/*?*/ 
/*?*/                     bDidPaint = true;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				if( bRotate && !bRota180 )
/*?*/ 					aAttr.SetRotation( nDrehWink / 10 );
/*?*/ 
/*?*/ 				pGraphic->Draw( pOutDev, aLogPos, aLogSize, &aAttr, nGraphicManagerDrawMode );
/*?*/                 bDidPaint = true;
/*?*/ 			}
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			// MetaFiles
/*?*/ 			const ULONG nOldDrawMode = pOutDev->GetDrawMode();
/*?*/ 
/*?*/ 			// Falls Modus GRAYBITMAP, wollen wir auch Mtf's als Graustufen darstellen
/*?*/ 			if( nOldDrawMode & DRAWMODE_GRAYBITMAP )
/*?*/ 			{
/*?*/ 				ULONG nNewDrawMode = nOldDrawMode;
/*?*/ 				nNewDrawMode &= ~( DRAWMODE_BLACKLINE | DRAWMODE_BLACKFILL | DRAWMODE_WHITEFILL | DRAWMODE_NOFILL );
/*?*/ 				pOutDev->SetDrawMode( nNewDrawMode |= DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL  );
/*?*/ 			}
/*?*/ 
/*?*/ 			if( bRotate && !bRota180 )
/*?*/ 				aAttr.SetRotation( nDrehWink / 10 );
/*?*/ 
/*?*/ 			pGraphic->Draw( pOutDev, aLogPos, aLogSize, &aAttr, nGraphicManagerDrawMode );
/*?*/ 			pOutDev->SetDrawMode( nOldDrawMode );
/*?*/ 
/*?*/             bDidPaint = true;
/*?*/ 		}
/*?*/         
/*?*/         // #110290# Remove the SdrGraphObj from the list of objects to be removed on 
/*?*/         // page switch. This is permissible, as the Draw above reenabled the swapout
/*?*/         // timer.
/*?*/         // #110573# Occasionally, the view is NULL (e.g. previews in impress and calc)
/*?*/         if( bDidPaint && pView )
/*?*/             ( (SdrView*) pView )->ImpAsyncPaintDone( this );
/*N*/ 	}

    // auch GRAPHIC_NONE oder SwappedOut( AsyncSwap )
/*N*/ 	if( ( bEmptyPresObj || bDraft ) && ( !bDraft || !( rInfoRec.nPaintMode & SDRPAINTMODE_HIDEDRAFTGRAF ) ) )
/*N*/ 	{
/*N*/ 		XubString	aText;
/*N*/ 		Bitmap*		pBmp = NULL;
/*N*/ 		FASTBOOL	bFill = FALSE;
/*N*/ 
/*N*/ 		if( bEmptyPresObj )
/*?*/ 			{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 bFill = !ImpPaintEmptyPres( pOutDev );
/*N*/ 
/*N*/ 		// und nun noch einen grauen Rahmen drum herum, Text rein, ...
/*N*/ 		if( !bEmptyPresObj )
/*N*/ 		{
/*N*/ 			aText = aFileName;
/*N*/ 
/*N*/ 			if(!aText.Len())
/*N*/ 			{
/*N*/ 				aText = aName;
/*N*/ 
/*N*/ 				if( bLoading )
/*N*/ 				{
/*?*/ 					aText.AppendAscii(" ...");
/*?*/ 					//FASTBOOL bNoName=aText.Len()==0;
/*?*/ 					//if (!bNoName) aText.Insert(' ',0);
/*?*/ 					//else aText.Insert("...",0);
/*?*/ 					//aText.Insert("Loading",0);
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ #ifndef SVX_LIGHT
/*N*/ 			pBmp = new Bitmap( ResId ( BMAP_GrafikEi, *ImpGetResMgr() ) );
/*N*/ #endif
/*N*/ 		}
/*N*/ 
/*N*/ 		ImpPaintReplacement( pOutDev, aText, pBmp, bFill );
/*N*/ 		delete pBmp;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( OUTDEV_PRINTER == pOutDev->GetOutDevType() )
/*?*/ 		ForceSwapOut();
/*N*/ 
/*N*/ 	return( HasText() ? SdrTextObj::Paint( rOut, rInfoRec ) : TRUE );
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SdrObject* SdrGrafObj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
/*N*/ {
/*N*/ 	return ImpCheckHit( rPnt, nTol, pVisiLayer, TRUE );
/*N*/ }

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::operator=( const SdrObject& rObj )
/*N*/ {
/*N*/ 	SdrRectObj::operator=( rObj );
/*N*/ 
/*N*/ 	const SdrGrafObj& rGraf = (SdrGrafObj&) rObj;
/*N*/ 
/*N*/ 	pGraphic->SetGraphic( rGraf.GetGraphic() );
/*N*/ 	aCropRect = rGraf.aCropRect;
/*N*/ 	aFileName = rGraf.aFileName;
/*N*/ 	aFilterName = rGraf.aFilterName;
/*N*/ 	aName = rGraf.aName;
/*N*/ 	bMirrored = rGraf.bMirrored;
/*N*/ 
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	if( rGraf.pGraphicLink != NULL)
/*N*/ #else
/*N*/ 	if( rGraf.aFileName.Len() )
/*N*/ #endif
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 SetGraphicLink( aFileName, aFilterName );
/*N*/ 
/*N*/ 	ImpSetAttrToGrafInfo();
/*N*/ }

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
/*N*/ {
/*N*/ 	SdrRectObj::NbcResize( rRef, xFact, yFact );
/*N*/ 
/*N*/ 	FASTBOOL bMirrX = xFact.GetNumerator() < 0;
/*N*/ 	FASTBOOL bMirrY = yFact.GetNumerator() < 0;
/*N*/ 
/*N*/ 	if( bMirrX != bMirrY )
/*?*/ 		bMirrored = !bMirrored;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
/*N*/ {
/*N*/ 	SdrRectObj::NbcRotate(rRef,nWink,sn,cs);
/*N*/ }

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::NbcSetSnapRect(const Rectangle& rRect)
/*N*/ {
/*N*/ 	SdrRectObj::NbcSetSnapRect(rRect);
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::NbcSetLogicRect( const Rectangle& rRect)
/*N*/ {
/*N*/ 	FASTBOOL bChg=rRect.GetSize()!=aRect.GetSize();
/*N*/ 	SdrRectObj::NbcSetLogicRect(rRect);
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SdrObjGeoData* SdrGrafObj::NewGeoData() const
/*N*/ {
/*N*/ 	return new SdrGrafObjGeoData;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::SaveGeoData(SdrObjGeoData& rGeo) const
/*N*/ {
/*N*/ 	SdrRectObj::SaveGeoData(rGeo);
/*N*/ 	SdrGrafObjGeoData& rGGeo=(SdrGrafObjGeoData&)rGeo;
/*N*/ 	rGGeo.bMirrored=bMirrored;
/*N*/ }

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::SetPage( SdrPage* pNewPage )
/*N*/ {
/*N*/ 	FASTBOOL bRemove = pNewPage == NULL && pPage != NULL;
/*N*/ 	FASTBOOL bInsert = pNewPage != NULL && pPage == NULL;
/*N*/ 
/*N*/ 	if( bRemove )
/*N*/ 	{
/*N*/ 		// hier kein SwapIn noetig, weil wenn nicht geladen, dann auch nicht animiert.
/*N*/ 		if( pGraphic->IsAnimated())
/*?*/ 			pGraphic->StopAnimation();
/*N*/ 
/*N*/ 		if( pGraphicLink != NULL )
/*?*/ 			ImpLinkAbmeldung();
/*N*/ 	}
/*N*/ 
/*N*/ 	SdrRectObj::SetPage( pNewPage );
/*N*/ 
/*N*/ 	if(aFileName.Len() && bInsert)
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 ImpLinkAnmeldung();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::SetModel( SdrModel* pNewModel )
/*N*/ {
/*N*/ 	FASTBOOL bChg = pNewModel != pModel;
/*N*/ 
/*N*/ 	if( bChg )
/*N*/ 	{
/*N*/ 		if( ( GRAFSTREAMPOS_INVALID != nGrafStreamPos ) || pGraphic->HasUserData() )
/*N*/ 		{
/*?*/ 			ForceSwapIn();
/*?*/ 			pGraphic->SetUserData();
/*?*/ 			nGrafStreamPos = GRAFSTREAMPOS_INVALID;
/*N*/ 		}
/*N*/ 
/*N*/ 		if( pGraphicLink != NULL )
/*?*/ 			ImpLinkAbmeldung();
/*N*/ 	}

    // Model umsetzen
/*N*/ 	SdrRectObj::SetModel(pNewModel);
/*N*/ 
/*N*/ 	if( bChg && aFileName.Len() )
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 ImpLinkAnmeldung();
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// pre- and postprocessing for objects for saving

/*N*/ void SdrGrafObj::PreSave()
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrRectObj::PreSave();
/*N*/ 
/*N*/ 	// prepare SetItems for storage
/*N*/ 	const SfxItemSet& rSet = GetUnmergedItemSet();
/*N*/ 	const SfxItemSet* pParent = GetStyleSheet() ? &GetStyleSheet()->GetItemSet() : 0L;
/*N*/ 	SdrGrafSetItem aGrafAttr(rSet.GetPool());
/*N*/ 	aGrafAttr.GetItemSet().Put(rSet);
/*N*/ 	aGrafAttr.GetItemSet().SetParent(pParent);
/*N*/ 	mpObjectItemSet->Put(aGrafAttr);
/*N*/ }

/*N*/ void SdrGrafObj::PostSave()
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrRectObj::PostSave();
/*N*/ 
/*N*/ 	// remove SetItems from local itemset
/*N*/ 	mpObjectItemSet->ClearItem(SDRATTRSET_GRAF);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrGrafObj::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ 	ForceSwapIn();
/*N*/ 	SdrRectObj::WriteData( rOut );
/*N*/ 
/*N*/ 	// Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ 	SdrDownCompat aCompat( rOut, STREAM_WRITE );
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID( "SdrGrafObj" );
/*N*/ #endif
/*N*/ 
/*N*/ 	GraphicType eType( pGraphic->GetType() );
/*N*/ 	BOOL		bHasGraphic( !aFileName.Len() && eType != GRAPHIC_NONE );
/*N*/ 
/*N*/ 	// dieses Flag wird ab V11 rausgeschrieben
/*N*/ 	rOut << bHasGraphic;
/*N*/ 
/*N*/ 	if(bHasGraphic)
/*N*/ 	{
/*N*/ 		// Graphik ist nicht gelinkt: ggf. komprimiert speichern:
/*N*/ 		// seit V11 eingapackt
/*N*/ 		SdrDownCompat aGrafCompat(rOut, STREAM_WRITE);
/*N*/ 		BOOL bZCompr(pModel && pModel->IsSaveCompressed() && eType == GRAPHIC_BITMAP);
/*N*/ 		BOOL bNCompr(pModel && pModel->IsSaveNative());
/*N*/ 		const UINT16 nOldComprMode(rOut.GetCompressMode());
/*N*/ 		UINT16 nNewComprMode(nOldComprMode);
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 		aGrafCompat.SetID( "SdrGrafObj(Graphic)" );
/*N*/ #endif
/*N*/ 
/*N*/ 		if(pModel->IsSwapGraphics() && (pModel->GetSwapGraphicsMode() & SDR_SWAPGRAPHICSMODE_DOC))
/*N*/ 		{
/*N*/ 			((SdrGrafObj*)this)->pGraphic->SetUserData();
/*N*/ 			((SdrGrafObj*)this)->nGrafStreamPos = rOut.Tell();
/*N*/ 		}
/*N*/ 
/*N*/ 		if(bZCompr)
/*N*/ 			nNewComprMode |= COMPRESSMODE_ZBITMAP;
/*N*/ 
/*N*/ 		if(bNCompr)
/*N*/ 			nNewComprMode |= COMPRESSMODE_NATIVE;
/*N*/ 
/*N*/ 		rOut.SetCompressMode( nNewComprMode );
/*N*/ 		rOut << pGraphic->GetGraphic();
/*N*/ 		rOut.SetCompressMode( nOldComprMode );
/*N*/ 	}
/*N*/ 
/*N*/ 	rOut << aCropRect;
/*N*/ 	rOut << BOOL(bMirrored);
/*N*/ 
/*N*/ 	rOut.WriteByteString(aName);
/*N*/ 
/*N*/ 	String aRelFileName;
/*N*/ 
/*N*/ 	if( aFileName.Len() )
/*N*/ 	{
/*?*/ 		aRelFileName = ::binfilter::StaticBaseUrl::AbsToRel( aFileName,
/*?*/ 												INetURLObject::WAS_ENCODED,
/*?*/ 												INetURLObject::DECODE_UNAMBIGUOUS );
/*N*/ 	}
/*N*/ 
/*N*/ 	rOut.WriteByteString( aRelFileName );
/*N*/ 
/*N*/ 	// UNICODE: rOut << aFilterName;
/*N*/ 	rOut.WriteByteString(aFilterName);
/*N*/ 
/*N*/ 	// ab V11
/*N*/ 	rOut << (BOOL)( aFileName.Len() != 0 );
/*N*/ 
/*N*/ 	SfxItemPool* pPool = GetItemPool();
/*N*/ 
/*N*/ 	if(pPool)
/*N*/ 	{
/*N*/ 		const SfxItemSet& rSet = GetUnmergedItemSet();
/*N*/ 
/*N*/ 		pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_GRAF));
/*N*/ 	}
/*N*/ 	else
/*?*/ 		rOut << UINT16( SFX_ITEMS_NULL );
/*N*/ 
/*N*/ 	ForceSwapOut();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::ReadDataTilV10( const SdrObjIOHeader& rHead, SvStream& rIn )
/*N*/ {
/*N*/ 	Graphic aGraphic;
/*N*/ 
/*N*/ 	// Import von allem mit Version <= 10
/*N*/ 	rIn >> aGraphic;
/*N*/ 
/*N*/ 	ULONG nError = rIn.GetError();
/*N*/ 
/*N*/ 	// Ist die Graphik defekt, oder wurde nur eine leere Graphik eingelesen? (was bei gelinkten Graphiken der Fall ist)
/*N*/ 	if( nError != 0)
/*?*/ 		rIn.ResetError();
/*N*/ 
/*N*/ 	if( rHead.GetVersion() >= 6)
/*N*/ 		rIn >> aCropRect;
/*N*/ 
/*N*/ 	if(rHead.GetVersion() >= 8)
/*N*/ 	{
/*N*/ 		// UNICODE: rIn>>aFileName;
/*N*/ 		rIn.ReadByteString(aFileName);
/*N*/ 	}
/*N*/ 
/*N*/ 	if(rHead.GetVersion() >= 9)
/*N*/ 	{
/*N*/ 		// UNICODE: rIn >> aFilterName;
/*N*/ 		rIn.ReadByteString(aFilterName);
/*N*/ 	}
/*N*/ 	else
/*?*/ 		aFilterName = String( RTL_CONSTASCII_USTRINGPARAM( "BMP - MS Windows" ) );
/*N*/ 
/*N*/ 	if( aFileName.Len() )
/*N*/ 	{
/*?*/ #ifndef SVX_LIGHT
/*?*/ 		String aFileURLStr;
/*?*/ 
/*?*/ 		if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aFileName, aFileURLStr ) )
/*?*/ 		{
/*?*/ 			SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aFileURLStr, STREAM_READ | STREAM_SHARE_DENYNONE );
/*?*/ 
/*?*/ 			if( pIStm )
/*?*/ 			{
/*?*/ 				GraphicFilter*	pFilter = GetGrfFilter();
/*?*/ 				USHORT			nError = pFilter->ImportGraphic( aGraphic, aFileURLStr, *pIStm );
/*?*/ 
/*?*/ 				SetGraphicLink( aFileURLStr, aFilterName );
/*?*/ 
/*?*/ 				delete pIStm;
/*?*/ 			}
/*?*/ 		}
/*?*/ #else
/*?*/ 		DBG_ERROR("SdrGrafObj::ReadDataTilV10(): SVX_LIGHT kann keine Graphic-Links");
/*?*/ #endif
/*N*/ 	}
/*N*/ 	else if( nError != 0 )
/*?*/ 		rIn.SetError(nError);
/*N*/ 
/*N*/ 
/*N*/ 	if( !rIn.GetError() )
/*N*/ 		pGraphic->SetGraphic( aGraphic );
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ #ifdef _MSC_VER
/*N*/ #pragma optimize ("",off)
/*N*/ #endif

/*N*/ void SdrGrafObj::ReadData( const SdrObjIOHeader& rHead, SvStream& rIn )
/*N*/ {
/*N*/ 	if( rIn.GetError() )
/*?*/ 		return;
/*N*/ 
/*N*/ 	SdrRectObj::ReadData( rHead, rIn );
/*N*/ 
/*N*/ 	SdrDownCompat	aCompat( rIn, STREAM_READ );
/*N*/ 	FASTBOOL		bDelayedLoad = ( pModel != NULL ) && pModel->IsSwapGraphics();
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrGrafObj");
/*N*/ #endif
/*N*/ 
/*N*/ 	pGraphic->SetUserData();
/*N*/ 	nGrafStreamPos = GRAFSTREAMPOS_INVALID;
/*N*/ 
/*N*/ 	if( rHead.GetVersion() < 11 )
/*N*/ 		ReadDataTilV10( rHead, rIn );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		String	aFileNameRel;
/*N*/ 		BOOL	bHasGraphic;
/*N*/ 		BOOL	bTmp;
/*N*/ 		BOOL	bGraphicLink;
/*N*/ 
/*N*/ 		rIn >> bHasGraphic;
/*N*/ 
/*N*/ 		if( bHasGraphic )
/*N*/ 		{
/*N*/ 			SdrDownCompat aGrafCompat( rIn,STREAM_READ );
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 			aGrafCompat.SetID("SdrGrafObj(Graphic)");
/*N*/ #endif
/*N*/ 
/*N*/ 			nGrafStreamPos = rIn.Tell();
/*N*/ 
/*N*/ 			if( !bDelayedLoad )
/*N*/ 			{
/*?*/ 				Graphic	aGraphic;
/*?*/ 				rIn >> aGraphic;
/*?*/ 				pGraphic->SetGraphic( aGraphic );
/*N*/ 			}
/*N*/ 			else
/*N*/ 				pGraphic->SetSwapState();
/*N*/ 
/*N*/ 			// Ist die Grafik defekt, oder wurde nur eine leere Graphik eingelesen?
/*N*/ 			// Daran soll mein Read jedoch nicht scheitern.
/*N*/ 			if( rIn.GetError() != 0 )
/*?*/ 				rIn.ResetError();
/*N*/ 		}
/*N*/ 
/*N*/ 		rIn >> aCropRect;
/*N*/ 		rIn >> bTmp; bMirrored = bTmp;
/*N*/ 
/*N*/ 		rIn.ReadByteString(aName);
/*N*/ 		// #85414# since there seems to be some documents wich have an illegal
/*N*/ 		// character inside the name of a graphic object we have to fix this
/*N*/ 		// here on load time or it will crash our xml later.
/*N*/ 		const xub_StrLen nLen = aName.Len();
/*N*/ 		for( xub_StrLen nIndex = 0; nIndex < nLen; nIndex++ )
/*N*/ 		{
/*N*/ 			if( aName.GetChar( nIndex ) < ' ' )
/*?*/ 				aName.SetChar( nIndex, '?' );
/*N*/ 		}
/*N*/ 
/*N*/ 		rIn.ReadByteString(aFileNameRel);
/*N*/ 
/*N*/ 		if( aFileNameRel.Len() )
/*N*/ 		{
/*N*/ 			aFileName = ::binfilter::StaticBaseUrl::SmartRelToAbs( aFileNameRel, FALSE,
/*N*/ 													INetURLObject::WAS_ENCODED,
/*N*/ 													INetURLObject::DECODE_UNAMBIGUOUS );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			aFileName.Erase();
/*N*/ 
/*N*/ 		// UNICODE: rIn >> aFilterName;
/*N*/ 		rIn.ReadByteString(aFilterName);
/*N*/ 
/*N*/ 		rIn >> bGraphicLink;					// auch dieses Flag ist neu in V11
/*N*/ 
/*N*/ 		if( aCompat.GetBytesLeft() > 0 )
/*N*/ 		{
/*N*/ 			SfxItemPool* pPool = GetItemPool();
/*N*/ 
/*N*/ 			if( pPool )
/*N*/ 			{
/*N*/ 				sal_uInt16 nSetID = SDRATTRSET_GRAF;
/*N*/ 				const SdrGrafSetItem* pGrafAttr = (const SdrGrafSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
/*N*/ 				if(pGrafAttr)
/*N*/ 					SetItemSet(pGrafAttr->GetItemSet());
/*N*/ 					ImpSetAttrToGrafInfo();
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*?*/ 				UINT16 nSuroDummy;
/*?*/ 				rIn >> nSuroDummy;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bCopyToPoolOnAfterRead = TRUE;
/*N*/ 
/*N*/ 		if( bGraphicLink && aFileName.Len() )
/*N*/ 		{
/*?*/ 			SetGraphicLink( aFileName, aFilterName );
/*?*/ 
/*?*/ 			if( !bDelayedLoad )
/*?*/                 ImpUpdateGraphicLink();
/*N*/ 		}
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ #ifdef _MSC_VER
/*N*/ #pragma optimize ("",on)
/*N*/ #endif

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::RestartAnimation(SdrPageView* pPageView) const
/*N*/ {
/*N*/ 	// ToDo: hier noch entsprechend implementieren wie im TextObj
/*N*/ 	SdrRectObj::RestartAnimation( pPageView );
/*N*/ }

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::AfterRead()
/*N*/ {
/*N*/ 	SdrRectObj::AfterRead();
/*N*/ 
/*N*/ 	if( bCopyToPoolOnAfterRead )
/*N*/ 	{
/*N*/ 		ImpSetGrafInfoToAttr();
/*N*/ 		bCopyToPoolOnAfterRead = FALSE;
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
/*N*/ 							 const SfxHint& rHint, const TypeId& rHintType )
/*N*/ {
/*N*/ 	SetXPolyDirty();
/*N*/ 	SdrRectObj::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
/*N*/ 	ImpSetAttrToGrafInfo();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::ForceDefaultAttr()
/*N*/ {
/*N*/ 	SdrRectObj::ForceDefaultAttr();
/*N*/ 
/*N*/ 	ImpForceItemSet();
/*N*/ 	mpObjectItemSet->Put( SdrGrafLuminanceItem( 0 ) );
/*N*/ 	mpObjectItemSet->Put( SdrGrafContrastItem( 0 ) );
/*N*/ 	mpObjectItemSet->Put( SdrGrafRedItem( 0 ) );
/*N*/ 	mpObjectItemSet->Put( SdrGrafGreenItem( 0 ) );
/*N*/ 	mpObjectItemSet->Put( SdrGrafBlueItem( 0 ) );
/*N*/ 	mpObjectItemSet->Put( SdrGrafGamma100Item( 100 ) );
/*N*/ 	mpObjectItemSet->Put( SdrGrafTransparenceItem( 0 ) );
/*N*/ 	mpObjectItemSet->Put( SdrGrafInvertItem( FALSE ) );
/*N*/ 	mpObjectItemSet->Put( SdrGrafModeItem( GRAPHICDRAWMODE_STANDARD ) );
/*N*/ 	mpObjectItemSet->Put( SdrGrafCropItem( 0, 0, 0, 0 ) );
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::NbcSetStyleSheet( SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr )
/*N*/ {
/*N*/ 	SetXPolyDirty();
/*N*/ 	SdrRectObj::NbcSetStyleSheet( pNewStyleSheet, bDontRemoveHardAttr );
/*N*/ 	ImpSetAttrToGrafInfo();
/*N*/ }

// -----------------------------------------------------------------------------
// ItemSet access

/*N*/ SfxItemSet* SdrGrafObj::CreateNewItemSet(SfxItemPool& rPool)
/*N*/ {
/*N*/ 	// include ALL items, 2D and 3D
/*N*/ 	return new SfxItemSet(rPool,
/*N*/ 		// ranges from SdrAttrObj
/*N*/ 		SDRATTR_START, SDRATTRSET_SHADOW,
/*N*/ 		SDRATTRSET_OUTLINER, SDRATTRSET_MISC,
/*N*/ 		SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,
/*N*/ 
/*N*/ 		// graf attributes
/*N*/ 		SDRATTR_GRAF_FIRST, SDRATTRSET_GRAF,
/*N*/ 
/*N*/ 		// outliner and end
/*N*/ 		EE_ITEMS_START, EE_ITEMS_END,
/*N*/ 		0, 0);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access
/*N*/ void SdrGrafObj::ItemSetChanged(const SfxItemSet& rSet)
/*N*/ {
/*N*/ 	// local changes
/*N*/ 	SetXPolyDirty();
/*N*/ 
/*N*/ 	// call parent
/*N*/ 	SdrRectObj::ItemSetChanged(rSet);
/*N*/ 
/*N*/ 	// local changes
/*N*/ 	ImpSetAttrToGrafInfo();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::ImpSetAttrToGrafInfo()
/*N*/ {
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 	const sal_uInt16 nTrans = ( (SdrGrafTransparenceItem&) rSet.Get( SDRATTR_GRAFTRANSPARENCE ) ).GetValue();
/*N*/ 	const SdrGrafCropItem&	rCrop = (const SdrGrafCropItem&) rSet.Get( SDRATTR_GRAFCROP );
/*N*/ 
/*N*/ 	aGrafInfo.SetLuminance( ( (SdrGrafLuminanceItem&) rSet.Get( SDRATTR_GRAFLUMINANCE ) ).GetValue() );
/*N*/ 	aGrafInfo.SetContrast( ( (SdrGrafContrastItem&) rSet.Get( SDRATTR_GRAFCONTRAST ) ).GetValue() );
/*N*/ 	aGrafInfo.SetChannelR( ( (SdrGrafRedItem&) rSet.Get( SDRATTR_GRAFRED ) ).GetValue() );
/*N*/ 	aGrafInfo.SetChannelG( ( (SdrGrafGreenItem&) rSet.Get( SDRATTR_GRAFGREEN ) ).GetValue() );
/*N*/ 	aGrafInfo.SetChannelB( ( (SdrGrafBlueItem&) rSet.Get( SDRATTR_GRAFBLUE ) ).GetValue() );
/*N*/ 	aGrafInfo.SetGamma( ( (SdrGrafGamma100Item&) rSet.Get( SDRATTR_GRAFGAMMA ) ).GetValue() * 0.01 );
/*N*/ 	aGrafInfo.SetTransparency( (BYTE) FRound( Min( nTrans, (USHORT) 100 )  * 2.55 ) );
/*N*/ 	aGrafInfo.SetInvert( ( (SdrGrafInvertItem&) rSet.Get( SDRATTR_GRAFINVERT ) ).GetValue() );
/*N*/ 	aGrafInfo.SetDrawMode( ( (SdrGrafModeItem&) rSet.Get( SDRATTR_GRAFMODE ) ).GetValue() );
/*N*/ 	aGrafInfo.SetCrop( rCrop.GetLeft(), rCrop.GetTop(), rCrop.GetRight(), rCrop.GetBottom() );
/*N*/ 
/*N*/ 	SetXPolyDirty();
/*N*/ 	SetRectsDirty();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::ImpSetGrafInfoToAttr()
/*N*/ {
/*N*/ 	SetItem( SdrGrafLuminanceItem( aGrafInfo.GetLuminance() ) );
/*N*/ 	SetItem( SdrGrafContrastItem( aGrafInfo.GetContrast() ) );
/*N*/ 	SetItem( SdrGrafRedItem( aGrafInfo.GetChannelR() ) );
/*N*/ 	SetItem( SdrGrafGreenItem( aGrafInfo.GetChannelG() ) );
/*N*/ 	SetItem( SdrGrafBlueItem( aGrafInfo.GetChannelB() ) );
/*N*/ 	SetItem( SdrGrafGamma100Item( FRound( aGrafInfo.GetGamma() * 100.0 ) ) );
/*N*/ 	SetItem( SdrGrafTransparenceItem( (USHORT) FRound( aGrafInfo.GetTransparency() / 2.55 ) ) );
/*N*/ 	SetItem( SdrGrafInvertItem( aGrafInfo.IsInvert() ) );
/*N*/ 	SetItem( SdrGrafModeItem( aGrafInfo.GetDrawMode() ) );
/*N*/ 	SetItem( SdrGrafCropItem( aGrafInfo.GetLeftCrop(), aGrafInfo.GetTopCrop(), aGrafInfo.GetRightCrop(), aGrafInfo.GetBottomCrop() ) );
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdrGrafObj::AdjustToMaxRect( const Rectangle& rMaxRect, BOOL bShrinkOnly )
/*N*/ {
/*N*/ 	Size aSize;
/*N*/ 	Size aMaxSize( rMaxRect.GetSize() );
/*N*/ 	if ( pGraphic->GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
/*N*/ 		aSize = Application::GetDefaultDevice()->PixelToLogic( pGraphic->GetPrefSize(), MAP_100TH_MM );
/*N*/ 	else
/*N*/ 		aSize = OutputDevice::LogicToLogic( pGraphic->GetPrefSize(),
/*N*/ 										    pGraphic->GetPrefMapMode(),
/*N*/ 										    MapMode( MAP_100TH_MM ) );
/*N*/ 
/*N*/ 	if( aSize.Height() != 0 && aSize.Width() != 0 )
/*N*/ 	{
/*N*/ 		Point aPos( rMaxRect.TopLeft() );
/*N*/ 
/*N*/ 		// Falls Grafik zu gross, wird die Grafik
/*N*/ 		// in die Seite eingepasst
/*N*/ 		if ( (!bShrinkOnly                          ||
/*N*/ 	    	 ( aSize.Height() > aMaxSize.Height() ) ||
/*N*/ 		 	( aSize.Width()  > aMaxSize.Width()  ) )&&
/*N*/ 		 	aSize.Height() && aMaxSize.Height() )
/*N*/ 		{
/*N*/ 			float fGrfWH =	(float)aSize.Width() /
/*N*/ 							(float)aSize.Height();
/*N*/ 			float fWinWH =	(float)aMaxSize.Width() /
/*N*/ 							(float)aMaxSize.Height();
/*N*/ 
/*N*/ 			// Grafik an Pagesize anpassen (skaliert)
/*N*/ 			if ( fGrfWH < fWinWH )
/*N*/ 			{
/*N*/ 				aSize.Width() = (long)(aMaxSize.Height() * fGrfWH);
/*N*/ 				aSize.Height()= aMaxSize.Height();
/*N*/ 			}
/*N*/ 			else if ( fGrfWH > 0.F )
/*N*/ 			{
/*N*/ 				aSize.Width() = aMaxSize.Width();
/*N*/ 				aSize.Height()= (long)(aMaxSize.Width() / fGrfWH);
/*N*/ 			}
/*N*/ 
/*N*/ 			aPos = rMaxRect.Center();
/*N*/ 		}
/*N*/ 
/*N*/ 		if( bShrinkOnly )
/*N*/ 			aPos = aRect.TopLeft();
/*N*/ 
/*N*/ 		aPos.X() -= aSize.Width() / 2;
/*N*/ 		aPos.Y() -= aSize.Height() / 2;
/*N*/ 		SetLogicRect( Rectangle( aPos, aSize ) );
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ IMPL_LINK( SdrGrafObj, ImpSwapHdl, BfGraphicObject*, pO )
/*N*/ {
/*N*/ 	SvStream* pRet = GRFMGR_AUTOSWAPSTREAM_NONE;
/*N*/ 
/*N*/ 	if( pO->IsInSwapOut() )
/*N*/ 	{
/*N*/ 		if( pModel != NULL && pModel->IsSwapGraphics() && pGraphic->GetSizeBytes() > 20480 )
/*N*/ 		{
/*N*/ 			SdrViewIter aIter( this );
/*N*/ 			SdrView*	pView = aIter.FirstView();
/*N*/ 			BOOL		bVisible = FALSE;
/*N*/ 
/*N*/ 			while( !bVisible && pView )
/*N*/ 			{
/*N*/ 				bVisible = !pView->IsGrafDraft();
/*N*/ 
/*N*/ 				if( !bVisible )
/*?*/ 					pView = aIter.NextView();
/*N*/ 			}
/*N*/ 
/*N*/ 			if( !bVisible )
/*N*/ 			{
/*N*/ 				const ULONG	nSwapMode = pModel->GetSwapGraphicsMode();
/*N*/ 
/*N*/ #ifndef SVX_LIGHT
/*N*/ 				if( ( ( GRAFSTREAMPOS_INVALID != nGrafStreamPos ) || pGraphic->HasUserData() || pGraphicLink ) &&
/*N*/ 					( nSwapMode & SDR_SWAPGRAPHICSMODE_PURGE ) )
/*N*/ #else
/*N*/ 				if( ( ( GRAFSTREAMPOS_INVALID != nGrafStreamPos ) || pGraphic->HasUserData() || aFileName.Len() ) &&
/*N*/ 					( nSwapMode & SDR_SWAPGRAPHICSMODE_PURGE ) )
/*N*/ #endif
/*N*/ 				{
/*N*/ 					pRet = NULL;
/*N*/ 				}
/*N*/ 				else if( nSwapMode & SDR_SWAPGRAPHICSMODE_TEMP )
/*N*/ 				{
/*N*/ 					pRet = GRFMGR_AUTOSWAPSTREAM_TEMP;
/*N*/ 					pGraphic->SetUserData();
/*N*/ 					nGrafStreamPos = GRAFSTREAMPOS_INVALID;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if( pO->IsInSwapIn() )
/*N*/ 	{
/*N*/ 		// kann aus dem original Doc-Stream nachgeladen werden...
/*N*/ 		if( pModel != NULL )
/*N*/ 		{
/*N*/ 			if( ( GRAFSTREAMPOS_INVALID != nGrafStreamPos ) || pGraphic->HasUserData() )
/*N*/ 			{
/*N*/ 				SdrDocumentStreamInfo aStreamInfo;
/*N*/ 
/*N*/ 				aStreamInfo.mbDeleteAfterUse = FALSE;
/*N*/ 				aStreamInfo.maUserData = pGraphic->GetUserData();
/*N*/                 aStreamInfo.mpStorageRef = NULL;
/*N*/ 
/*N*/ 				SvStream* pStream = pModel->GetDocumentStream( aStreamInfo );
/*N*/ 
/*N*/ 				if( pStream != NULL )
/*N*/ 				{
/*N*/ 					Graphic aGraphic;
/*N*/ 
/*N*/ 					if( pGraphic->HasUserData() )
/*N*/ 					{
/*?*/ 						if( !GetGrfFilter()->ImportGraphic( aGraphic, String(), *pStream ) )
/*?*/ 						{
/*?*/ 							const String aUserData( pGraphic->GetUserData() );
/*?*/ 
/*?*/ 							pGraphic->SetGraphic( aGraphic );
/*?*/ 							pGraphic->SetUserData( aUserData );
/*?*/         					pRet = GRFMGR_AUTOSWAPSTREAM_LOADED;
/*N*/ 						}
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						pStream->Seek( nGrafStreamPos );
/*N*/ 						*pStream >> aGraphic;
/*N*/ 						pGraphic->SetGraphic( aGraphic );
/*N*/ 						
/*N*/ 						if( !pStream->GetError() )
/*N*/ 						    pRet = GRFMGR_AUTOSWAPSTREAM_LOADED;
/*N*/ 					}
/*N*/ 
/*N*/ 					pStream->ResetError();
/*N*/ 
/*N*/ 					if( aStreamInfo.mbDeleteAfterUse || aStreamInfo.mpStorageRef )
/*N*/                     {
/*N*/ 						delete pStream;
/*N*/                         delete aStreamInfo.mpStorageRef;
/*N*/                     }
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else if( !ImpUpdateGraphicLink() )
/*N*/             {
/*?*/ 				pRet = GRFMGR_AUTOSWAPSTREAM_TEMP;
/*N*/             }
/*N*/ 			else
/*N*/             {
/*?*/                 pRet = GRFMGR_AUTOSWAPSTREAM_LOADED;
/*N*/             }
/*N*/ 		}
/*N*/ 		else
/*?*/ 			pRet = GRFMGR_AUTOSWAPSTREAM_TEMP;
/*N*/ 	}
/*N*/ 
/*N*/ 	return (long)(void*) pRet;
/*N*/ }

// -----------------------------------------------------------------------------


}
