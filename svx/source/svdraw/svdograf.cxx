/*************************************************************************
 *
 *  $RCSfile: svdograf.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ka $ $Date: 2000-10-11 15:19:18 $
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

#ifndef _SFXINIMGR_HXX //autogen wg. SfxIniManager
#include <svtools/iniman.hxx>
#endif
#ifndef SVX_LIGHT
#include <so3/lnkbase.hxx>
#endif

#include <math.h>
#include <vcl/salbtype.hxx>
#include <sot/formats.hxx>
#include <tools/urlobj.hxx>
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

#ifndef SVX_LIGHT

// ------------------
// - SdrGraphicLink -
// ------------------

class SdrGraphicLink : public SvBaseLink
{
    SdrGrafObj*         pGrafObj;

public:
                        SdrGraphicLink(SdrGrafObj* pObj);
    virtual             ~SdrGraphicLink();

    virtual void        Closed();
    virtual void        DataChanged(SvData& rData);

    BOOL                Connect() { return 0 != SvBaseLink::GetRealObject(); }
    void                UpdateSynchron();
};

// -----------------------------------------------------------------------------

SdrGraphicLink::SdrGraphicLink(SdrGrafObj* pObj):
    SvBaseLink(LINKUPDATE_ONCALL, SOT_FORMATSTR_ID_SVXB),
    pGrafObj(pObj)
{
    SetSynchron(FALSE);
}

// -----------------------------------------------------------------------------

SdrGraphicLink::~SdrGraphicLink()
{
}

// -----------------------------------------------------------------------------

void SdrGraphicLink::DataChanged(SvData& rData)
{
    SdrModel*       pModel      = pGrafObj==NULL ? NULL : pGrafObj->GetModel();
    SvxLinkManager* pLinkManager= pModel  ==NULL ? NULL : pModel->GetLinkManager();

    if (pLinkManager!=NULL)
    {
        const ULONG nFormat = rData.GetFormat();
        GraphicType eOldGraphicType = pGrafObj->GetGraphicType();  // kein Hereinswappen
        BOOL bIsChanged = pModel->IsChanged();
        pLinkManager->GetDisplayNames(*this,NULL,&pGrafObj->aFileName,NULL,&pGrafObj->aFilterName);

        if (nFormat == SOT_FORMATSTR_ID_SVXB)
        {
            Graphic* pGraphic = NULL;

            if (rData.GetData((SvDataCopyStream**) &pGraphic, Graphic::StaticType(), TRANSFER_REFERENCE))
            {
                pGrafObj->SetGraphic( *pGraphic );

                if (eOldGraphicType != GRAPHIC_NONE)
                    pGrafObj->SetChanged();
                else
                    pModel->SetChanged(bIsChanged);
            }
        }
        else if (rData.GetFormat() != SvxLinkManager::RegisterStatusInfoId())
        {
            switch( nFormat)
            {
                case FORMAT_GDIMETAFILE:
                {
                    GDIMetaFile* pMetaFile=NULL;
                    if (rData.GetData(&pMetaFile,TRANSFER_REFERENCE))
                    {
                        pGrafObj->SetGraphic(*pMetaFile);

                        if (eOldGraphicType != GRAPHIC_NONE)
                            pGrafObj->SetChanged();
                        else
                            pModel->SetChanged(bIsChanged);
                    }
                }
                break;

                case FORMAT_BITMAP:
                {
                    Bitmap* pBmp=NULL;
                    if (rData.GetData(&pBmp,TRANSFER_REFERENCE))
                    {
                        pGrafObj->SetGraphic(*pBmp);

                        if (eOldGraphicType != GRAPHIC_NONE)
                            pGrafObj->SetChanged();
                        else
                            pModel->SetChanged(bIsChanged);
                    }
                }
                break;

                default:
                    pGrafObj->SendRepaintBroadcast();
                break;
            }
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
        SvData aData( GetContentType() );
        aData.SetAspect( ASPECT_DOCPRINT );
        GetObj()->GetData( &aData );
        DataChanged( aData );
    }
}

#else

GraphicFilter* SVX_LIGHT_pGrapicFilter = NULL;

GraphicFilter* SVX_LIGHT_GetGrfFilter_Impl()
{
    if( !SVX_LIGHT_pGrapicFilter )
    {
        SVX_LIGHT_pGrapicFilter = new GraphicFilter;

        INetURLObject aFilterURL;

        aFilterURL.SetSmartProtocol( INET_PROT_FILE );
        aFilterURL.SetSmartURL( GetpApp()->GetAppFileName() );
        aFilterURL.removeSegment();
        aFilterURL.removeFinalSlash();

        INetURLObject aConfigURL( aFilterURL );

        aFilterURL.Append( "filter" );
#ifndef UNX
        aConfigURL.Append( "sop.ini" );
#else
        aConfigURL.Append( "soprc" );
#endif

        SVX_LIGHT_pGrapicFilter->SetFilterPath( aFilterURL.PathToFileName() );
        SVX_LIGHT_pGrapicFilter->SetConfigPath( aConfigURL.PathToFileName() );
    }

    const Link aLink;
    SVX_LIGHT_pGrapicFilter->SetStartFilterHdl( aLink );
    SVX_LIGHT_pGrapicFilter->SetEndFilterHdl( aLink );
    SVX_LIGHT_pGrapicFilter->SetUpdatePercentHdl( aLink );
    return SVX_LIGHT_pGrapicFilter;
}


#endif // SVX_LIGHT

// --------------
// - SdrGrafObj -
// --------------

TYPEINIT1(SdrGrafObj,SdrRectObj);

// -----------------------------------------------------------------------------

SdrGrafObj::SdrGrafObj():
    bMirrored       ( FALSE ),
    pGraphicLink    ( NULL )
{
    pGraphic = new GraphicObject;
    pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), Application::IsRemoteServer() ? 60000 : 20000 );
    nGrafStreamPos=0;
    bSwappedOut = bNotLoaded = FALSE;
    bNoShear=TRUE;

#ifdef GRAFATTR
    pGrafAttr = NULL;
    bCopyToPoolOnAfterRead = FALSE;
#endif // GRAFATTR
}

// -----------------------------------------------------------------------------

SdrGrafObj::SdrGrafObj(const Graphic& rGrf, const Rectangle& rRect):
    SdrRectObj      ( rRect ),
    bMirrored       ( FALSE ),
    pGraphicLink    ( NULL )
{
    pGraphic = new GraphicObject( rGrf );
    pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), Application::IsRemoteServer() ? 60000 : 20000 );
    nGrafStreamPos = 0;
    bSwappedOut = bNotLoaded = FALSE;
    bNoShear = TRUE;

#ifdef GRAFATTR
    pGrafAttr = NULL;
    bCopyToPoolOnAfterRead = FALSE;
#endif // GRAFATTR
}

// -----------------------------------------------------------------------------

SdrGrafObj::SdrGrafObj( const Graphic& rGrf ):
    bMirrored       ( FALSE ),
    pGraphicLink    ( NULL )
{
    pGraphic = new GraphicObject( rGrf );
    pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), Application::IsRemoteServer() ? 60000 : 20000 );
    nGrafStreamPos = 0;
    bSwappedOut = bNotLoaded = FALSE;
    bNoShear = TRUE;

#ifdef GRAFATTR
    pGrafAttr = NULL;
    bCopyToPoolOnAfterRead = FALSE;
#endif // GRAFATTR
}

// -----------------------------------------------------------------------------

SdrGrafObj::~SdrGrafObj()
{
    delete pGraphic;
    ImpLinkAbmeldung();

#ifdef GRAFATTR
    pGrafAttr = (SdrGrafSetItem*) ImpSetNewAttr( pGrafAttr, NULL, FALSE );
#endif // GRAFATTR
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetGraphicObject( const GraphicObject& rGrfObj )
{
    *pGraphic = rGrfObj;
    pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), Application::IsRemoteServer() ? 60000 : 20000 );
    nGrafStreamPos = 0;
    bSwappedOut = bNotLoaded = FALSE;
    SetChanged();
    SendRepaintBroadcast();
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
    nGrafStreamPos = 0;
    bSwappedOut = bNotLoaded = FALSE;
    SetChanged();
    SendRepaintBroadcast();
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
    Graphic     aTransGraphic;
    GraphicType eType = GetGraphicType();

    if( SDRGRAFOBJ_TRANSFORMATTR_NONE == nTransformFlags )
        aTransGraphic = GetGraphic();
    else if( eType != GRAPHIC_NONE )
    {
        ( (SdrGrafObj*) this )->ImpSetAttrToGrafInfo();

        GraphicAttr aActAttr( aGrafInfo );

        if( nTransformFlags & SDRGRAFOBJ_TRANSFORMATTR_MIRROR )
        {
            USHORT      nMirrorCase = ( aGeo.nDrehWink == 18000 ) ? ( bMirrored ? 3 : 4 ) : ( bMirrored ? 2 : 1 );
            FASTBOOL    bHMirr = nMirrorCase == 2 || nMirrorCase == 4;
            FASTBOOL    bVMirr = nMirrorCase == 3 || nMirrorCase == 4;

            aActAttr.SetMirrorFlags( ( bHMirr ? BMP_MIRROR_HORZ : 0 ) | ( bVMirr ? BMP_MIRROR_VERT : 0 ) );
        }

        if( ( nTransformFlags & SDRGRAFOBJ_TRANSFORMATTR_ROTATE ) &&
            ( aGeo.nDrehWink && aGeo.nDrehWink != 18000 ) && ( GRAPHIC_BITMAP == eType ) && !IsAnimated() )
        {
            aActAttr.SetRotation( aGeo.nDrehWink / 10 );
        }

        aTransGraphic = pGraphic->GetTransformedGraphic( &aActAttr );
    }

    return aTransGraphic;
}

// -----------------------------------------------------------------------------

GraphicType SdrGrafObj::GetGraphicType() const
{
    return pGraphic->GetType();
}

// -----------------------------------------------------------------------------

FASTBOOL SdrGrafObj::IsAnimated() const
{
    return pGraphic->IsAnimated();
}

// -----------------------------------------------------------------------------

FASTBOOL SdrGrafObj::IsTransparent() const
{
    return pGraphic->IsTransparent();
}

// -----------------------------------------------------------------------------

FASTBOOL SdrGrafObj::IsEPS() const
{
    return pGraphic->IsEPS();
}

// -----------------------------------------------------------------------------

const MapMode& SdrGrafObj::GetGrafPrefMapMode() const
{
    return pGraphic->GetPrefMapMode();
}

// -----------------------------------------------------------------------------

const Size& SdrGrafObj::GetGrafPrefSize() const
{
    return pGraphic->GetPrefSize();
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
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ForceSwapOut() const
{
    pGraphic->FireSwapOutRequest();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ImpLinkAnmeldung()
{
#ifndef SVX_LIGHT

    SvxLinkManager* pLinkManager = pModel != NULL ? pModel->GetLinkManager() : NULL;

    if( pLinkManager != NULL && pGraphicLink == NULL )
    {
        // Nicht 2x Anmelden
        if(aFileName.Len())
        {
            pGraphicLink = new SdrGraphicLink( this );
            pLinkManager->InsertFileLink( *pGraphicLink, OBJECT_CLIENT_GRF, aFileName,
                (aFilterName.Len() ? &aFilterName : NULL) , NULL );
            pGraphicLink->Connect();

            FASTBOOL bDelayedLoad = pModel != NULL && pModel->IsSwapGraphics();

#ifndef SVX_LIGHT
            if( !bDelayedLoad )
            {
                BOOL bIsChanged = pModel->IsChanged();
                pGraphicLink->UpdateSynchron();
                pModel->SetChanged( bIsChanged );
            }
#endif
        }
    }

#endif // SVX_LIGHT
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ImpLinkAbmeldung()
{
#ifndef SVX_LIGHT

    SvxLinkManager* pLinkManager = pModel != NULL ? pModel->GetLinkManager() : NULL;

    if( pLinkManager != NULL && pGraphicLink!=NULL)
    {
        // Bei Remove wird *pGraphicLink implizit deleted
        pLinkManager->Remove( *pGraphicLink );
        pGraphicLink=NULL;
    }

#endif // SVX_LIGHT
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetGraphicLink( const String& rFileName, const String& rFilterName )
{
    ImpLinkAbmeldung();
    aFileName = rFileName;
    aFilterName = rFilterName;
    ImpLinkAnmeldung();
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
    FASTBOOL bBmp = pGraphic->GetType() == GRAPHIC_BITMAP;
    FASTBOOL bTrans = pGraphic->IsTransparent();
    FASTBOOL bAnim = pGraphic->IsAnimated();
    FASTBOOL bNoPresBmp = bBmp && !bEmptyPresObj;

    rInfo.bResizeFreeAllowed = aGeo.nDrehWink % 9000 == 0 ||
                               aGeo.nDrehWink % 18000 == 0 ||
                               aGeo.nDrehWink % 27000 == 0;

    rInfo.bResizePropAllowed = TRUE;
    rInfo.bRotateFreeAllowed = bNoPresBmp && !bAnim;
    rInfo.bRotate90Allowed = bNoPresBmp && !bAnim;
    rInfo.bMirrorFreeAllowed = bNoPresBmp && !bAnim;
    rInfo.bMirror45Allowed = bNoPresBmp && !bAnim;
    rInfo.bMirror90Allowed = !bEmptyPresObj;
    rInfo.bTransparenceAllowed = FALSE;
    rInfo.bGradientAllowed = FALSE;
    rInfo.bShearAllowed = FALSE;
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

// Liefert FALSE, wenn die Pres-Bitmap zu gross ist
FASTBOOL SdrGrafObj::ImpPaintEmptyPres( OutputDevice* pOutDev ) const
{
    Size        aSizePix( pOutDev->LogicToPixel( pGraphic->GetPrefSize(), pGraphic->GetPrefMapMode() ) );
    Size        aSize( pOutDev->PixelToLogic( aSizePix ) );
    Point       aPos( aRect.Center() );
    FASTBOOL    bRet;

    aPos.X() -= aSize.Width() >> 1;
    aPos.Y() -= aSize.Height() >> 1;

    if( aPos.X() > aRect.Left() && aPos.Y() > aRect.Top())
    {
        const Graphic&  rGraphic = pGraphic->GetGraphic();
        FASTBOOL        bCache=pModel!=NULL && pModel->IsBitmapCaching();
        FASTBOOL        bBmp = pGraphic->GetType() == GRAPHIC_BITMAP;

        if( bBmp )
        {
            const Size aSz( pOutDev->PixelToLogic( rGraphic.GetBitmap().GetSizePixel() ) );
            pGraphic->Draw( pOutDev, aPos, aSz, NULL );
        }
        else
        {
            const ULONG nOldDrawMode = pOutDev->GetDrawMode();

            if( ( nOldDrawMode & DRAWMODE_GRAYBITMAP ) != 0 )
            {
                // Falls Modus GRAYBITMAP, wollen wir auch Mtf's als Graustufen darstellen
                ULONG nNewDrawMode = nOldDrawMode;
                nNewDrawMode &= ~( DRAWMODE_BLACKLINE | DRAWMODE_BLACKFILL | DRAWMODE_WHITEFILL | DRAWMODE_NOFILL );
                pOutDev->SetDrawMode( nNewDrawMode |= DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL  );
            }

            rGraphic.Draw( pOutDev, aPos );
            pOutDev->SetDrawMode( nOldDrawMode );
        }

        pOutDev->SetFillColor();
        bRet = TRUE;
    }
    else
        bRet = FALSE;

    return bRet;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ImpPaintReplacement(OutputDevice* pOutDev, const XubString& rText, const Bitmap* pBmp, FASTBOOL bFill) const
{
    if( bFill )
    {
        pOutDev->SetLineColor();
        pOutDev->SetFillColor( COL_LIGHTGRAY );
    }

    Rectangle aRect1(pOutDev->LogicToPixel(aRect));
    Rectangle aTextRect(aRect1);
    Rectangle aRect2(aRect1);
    aRect2.Left()++; aRect2.Top()++; aRect2.Right()--; aRect2.Bottom()--;
    Point aTopLeft(aRect1.TopLeft());
    Point aBmpPos(aTopLeft);

    if( pBmp != NULL )
    {
        Size        aBmpSize(pBmp->GetSizePixel());
        long        nRectWdt=aTextRect.Right()-aTextRect.Left();
        long        nRectHgt=aTextRect.Bottom()-aTextRect.Top();
        long        nBmpWdt=aBmpSize.Width();
        long        nBmpHgt=aBmpSize.Height();
        BOOL bText(rText.Len());

        long nMinWdt = nBmpWdt;
        long nMinHgt = nBmpHgt;

        if( bText )
        {
            nMinWdt=2 + nBmpWdt * 2 + 3;
            nMinHgt=2 + nBmpHgt * 2 + 3;
        }

        if( nRectWdt < nMinWdt || nRectHgt < nMinHgt )
            pBmp=NULL;
        else
        {
            aTextRect.Left() += nBmpWdt;

            if( bText )
                aTextRect.Left()+=2+3;
        }

        aBmpPos.X() += 2;
        aBmpPos.Y() += 2;

        if( aGeo.nDrehWink!=0 )
        {
            Point aRef(aBmpPos);
            aRef.X()-=aBmpSize.Width()/2+2;
            aRef.Y()-=aBmpSize.Height()/2+2;
            double nSin=sin(aGeo.nDrehWink*nPi180);
            double nCos=cos(aGeo.nDrehWink*nPi180);
            RotatePoint(aBmpPos,aRef,nSin,nCos);
        }
    }

    const BOOL bOldMap = pOutDev->IsMapModeEnabled();
    pOutDev->EnableMapMode( FALSE );

    if( aGeo.nDrehWink == 0 && aGeo.nShearWink == 0 )
    {
        if (bFill)
            pOutDev->DrawRect(aRect);

        if (pBmp!=NULL)
            pOutDev->DrawBitmap(aBmpPos,*pBmp);

        const StyleSettings& rStyleSettings = pOutDev->GetSettings().GetStyleSettings();

        pOutDev->SetFillColor();
        pOutDev->SetLineColor( rStyleSettings.GetShadowColor() );
        pOutDev->DrawLine(aRect1.TopLeft(),aRect1.TopRight()); pOutDev->DrawLine(aRect1.TopLeft(),aRect1.BottomLeft());


        pOutDev->SetLineColor( rStyleSettings.GetLightColor() );
        pOutDev->DrawLine(aRect1.TopRight(),aRect1.BottomRight()); pOutDev->DrawLine(aRect1.BottomLeft(),aRect1.BottomRight());

        pOutDev->SetLineColor( rStyleSettings.GetLightColor() );
        pOutDev->DrawLine(aRect2.TopLeft(),aRect2.TopRight()); pOutDev->DrawLine(aRect2.TopLeft(),aRect2.BottomLeft());

        pOutDev->SetLineColor( rStyleSettings.GetShadowColor() );
        pOutDev->DrawLine(aRect2.TopRight(),aRect2.BottomRight()); pOutDev->DrawLine(aRect2.BottomLeft(),aRect2.BottomRight());
    }
    else
    {
        Polygon aPoly1(Rect2Poly(aRect1,aGeo));
        Polygon aPoly2(5);
        aPoly2[0]=aRect2.TopLeft();
        aPoly2[1]=aRect2.TopRight();
        aPoly2[2]=aRect2.BottomRight();
        aPoly2[3]=aRect2.BottomLeft();
        aPoly2[4]=aRect2.TopLeft();

        if (aGeo.nShearWink != 0)
            ShearPoly(aPoly2,aTopLeft,aGeo.nTan);

        if( aGeo.nDrehWink != 0 )
            RotatePoly(aPoly2,aTopLeft,aGeo.nSin,aGeo.nCos);

        if( bFill )
            pOutDev->DrawPolygon(aPoly1);

        if( pBmp != NULL )
            pOutDev->DrawBitmap( aBmpPos, *pBmp );

        pOutDev->SetFillColor();

        long        nHWink=NormAngle360(aGeo.nDrehWink);
        long        nVWink=NormAngle360(aGeo.nDrehWink-aGeo.nShearWink);
        FASTBOOL    bHorzChg=nHWink>13500 && nHWink<=31500;
        FASTBOOL    bVertChg=nVWink>4500 && nVWink<=22500;

        const StyleSettings& rStyleSettings = pOutDev->GetSettings().GetStyleSettings();
        Color a3DLightColor( rStyleSettings.GetLightColor() );
        Color a3DShadowColor( rStyleSettings.GetShadowColor() );

        pOutDev->SetLineColor( bHorzChg ? a3DShadowColor : a3DLightColor);
        pOutDev->DrawLine(aPoly2[0],aPoly2[1]);

        pOutDev->SetLineColor( bHorzChg ? a3DLightColor  : a3DShadowColor);
        pOutDev->DrawLine(aPoly2[2],aPoly2[3]);

        pOutDev->SetLineColor( bVertChg ? a3DLightColor  : a3DShadowColor);
        pOutDev->DrawLine(aPoly2[1],aPoly2[2]);

        pOutDev->SetLineColor( bVertChg ? a3DShadowColor : a3DLightColor);
        pOutDev->DrawLine(aPoly2[3],aPoly2[4]);

        pOutDev->SetLineColor( bHorzChg ? a3DLightColor  : a3DShadowColor);
        pOutDev->DrawLine(aPoly1[0],aPoly1[1]);

        pOutDev->SetLineColor( bHorzChg ? a3DShadowColor : a3DLightColor);
        pOutDev->DrawLine(aPoly1[2],aPoly1[3]);

        pOutDev->SetLineColor( bVertChg ? a3DShadowColor : a3DLightColor);
        pOutDev->DrawLine(aPoly1[1],aPoly1[2]);

        pOutDev->SetLineColor( bVertChg ? a3DLightColor  : a3DShadowColor);
        pOutDev->DrawLine(aPoly1[3],aPoly1[4]);
    }
    XubString aNam(rText);

    if(aNam.Len())
    {
        Size aOutSize(aTextRect.GetSize()); aOutSize.Width()-=6; aOutSize.Height()-=6;

        if (aOutSize.Width()>=4 || aOutSize.Height()>=4)
        {
            Point aOutPos(aTextRect.TopLeft()); aOutPos.X()+=3; aOutPos.Y()+=3;
            long nMaxOutY=aOutPos.Y()+aOutSize.Height();
            Font aFontMerk(pOutDev->GetFont());
            Font aFont(System::GetStandardFont(STDFONT_SWISS));
            aFont.SetColor(COL_LIGHTRED);
            aFont.SetTransparent(TRUE);
            aFont.SetLineOrientation(USHORT(NormAngle360(aGeo.nDrehWink)/10));

            if (IsLinkedGraphic())
                aFont.SetUnderline(UNDERLINE_SINGLE);
            Size aFontSize(Size(0,aGeo.nDrehWink % 9000==0 ? 12 : 14));

            if (aFontSize.Height()>aOutSize.Height())
                aFontSize.Height()=aOutSize.Height();
            aFont.SetSize(aFontSize);
            pOutDev->SetFont(aFont);
            String aOutStr(aNam);

            while(aOutStr.Len() && aOutPos.Y() <= nMaxOutY)
            {
                String aStr1(aOutStr);
                INT32 nTextWidth = pOutDev->GetTextWidth(aStr1);
                INT32 nTextHeight = pOutDev->GetTextHeight();

                while(aStr1.Len() && nTextWidth > aOutSize.Width())
                {
                    aStr1.Erase(aStr1.Len() - 1);
                    nTextWidth = pOutDev->GetTextWidth(aStr1);
                    nTextHeight = pOutDev->GetTextHeight();
                }
                Point aPos(aOutPos);
                aOutPos.Y() += nTextHeight;

                if (aOutPos.Y()<=nMaxOutY)
                {
                    if (aGeo.nShearWink!=0) ShearPoint(aPos,aTopLeft,aGeo.nTan);
                    if (aGeo.nDrehWink!=0) RotatePoint(aPos,aTopLeft,aGeo.nSin,aGeo.nCos);
                    pOutDev->DrawText(aPos,aStr1);
                    aOutStr.Erase(0, aStr1.Len());
                }
            }
            pOutDev->SetFont(aFontMerk);
        }
    }
    pOutDev->EnableMapMode( bOldMap );
}

// -----------------------------------------------------------------------------

FASTBOOL SdrGrafObj::Paint( ExtOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec ) const
{
    // Hidden objects on masterpages, draw nothing
    if((rInfoRec.nPaintMode & SDRPAINTMODE_MASTERPAGE) && bNotVisibleAsMaster)
        return TRUE;

    // Do not print empty PresObj's
    if(OUTDEV_PRINTER == rOut.GetOutDev()->GetOutDevType() && bEmptyPresObj)
        return TRUE;

    FASTBOOL      bDraft(0 != (rInfoRec.nPaintMode & SDRPAINTMODE_DRAFTGRAF));
    FASTBOOL      bLoading=FALSE;
    OutputDevice* pOutDev=rOut.GetOutDev();
    OutDevType    eOutDevType=pOutDev!=NULL ? pOutDev->GetOutDevType() : OUTDEV_DONTKNOW;
    FASTBOOL      bJustFillCache=pOutDev==NULL;
    FASTBOOL      bPrn=!bJustFillCache && eOutDevType==OUTDEV_PRINTER;
    GDIMetaFile*  pRecMetaFile=!bJustFillCache ? pOutDev->GetConnectMetaFile() : NULL;
    FASTBOOL      bMtfRecording=pRecMetaFile!=NULL && pRecMetaFile->IsRecord() && !pRecMetaFile->IsPause();
    const SdrView* pView=rInfoRec.pPV!=NULL ? &rInfoRec.pPV->GetView() : NULL;

    if( !bDraft && bSwappedOut )
    {
        if( !bPrn && !bMtfRecording && eOutDevType == OUTDEV_WINDOW && pView && pView->IsSwapAsynchron() )
        {
            ( (SdrView*) pView )->ImpAddAsyncObj( this, pOutDev );
            bLoading=TRUE;
        }
        else
            ForceSwapIn();
    }
#ifndef SVX_LIGHT
    else if( !bSwappedOut && pGraphicLink && ( pGraphic->GetType() == GRAPHIC_NONE ) )
    {
        BOOL bIsChanged = pModel->IsChanged();
        pGraphicLink->UpdateSynchron();
        pModel->SetChanged( bIsChanged );
    }
#endif

    GraphicType eType = pGraphic->GetType();

    if( eType == GRAPHIC_NONE || eType == GRAPHIC_DEFAULT || bSwappedOut )
        bDraft=TRUE;

    long          nDrehWink = aGeo.nDrehWink;
    long          nShearWink = aGeo.nShearWink;
    USHORT        nMirrorCase = 0;
    FASTBOOL      bCache = pModel != NULL && pModel->IsBitmapCaching();
    FASTBOOL      bBmp = eType == GRAPHIC_BITMAP;
    FASTBOOL      bRotate = nDrehWink!=0 && nDrehWink!=18000;
    FASTBOOL      bShear = nShearWink!=0;
    FASTBOOL      bMirror = bMirrored;
    FASTBOOL      bHMirr;
    FASTBOOL      bVMirr;
    FASTBOOL      bRota90 = nDrehWink==9000 || nDrehWink==18000 || nDrehWink==27000;
    FASTBOOL      bRota180 = nDrehWink==18000;

    // 4 Faelle:
    //  4 | 3   H&V gespiegelt | nur Vertikal
    // ---+---  ---------------+-----------------
    //  2 | 1   nur Horizontal | nicht gespiegelt
    nMirrorCase=bRota180 ? (bMirror ? 3 : 4) : (bMirror ? 2 : 1);
    bHMirr= nMirrorCase==2 || nMirrorCase==4;
    bVMirr= nMirrorCase==3 || nMirrorCase==4;

    if( !bEmptyPresObj && !bDraft )
    {
        Point   aLogPos( aRect.TopLeft() );
        Size    aLogSize( aRect.GetSize() );

#ifdef GRAFATTR
        GraphicAttr aAttr( aGrafInfo );
#else // GRAFATTR
        GraphicAttr aAttr;
#endif // GRAFATTR

        aAttr.SetMirrorFlags( ( bHMirr ? BMP_MIRROR_HORZ : 0 ) | ( bVMirr ? BMP_MIRROR_VERT : 0 ) );

        if( bBmp )
        {
            if( pGraphic->IsAnimated() )
            {
                SdrAnimationMode    eAnimMode = SDR_ANIMATION_ANIMATE;
                FASTBOOL            bEnable = TRUE;

                if( rInfoRec.pPV != NULL )
                {
                    eAnimMode= ((SdrPaintView&) rInfoRec.pPV->GetView()).GetAnimationMode();
                    bEnable = eAnimMode != SDR_ANIMATION_DISABLE;
                }

                if( bEnable )
                {
                    if( eAnimMode == SDR_ANIMATION_ANIMATE )
                    {
                        pGraphic->SetAnimationNotifyHdl( LINK( this, SdrGrafObj, ImpAnimationHdl ) );
                        pGraphic->StartAnimation( pOutDev, aLogPos, aLogSize, 0, &aAttr );
                    }
                    else if( eAnimMode == SDR_ANIMATION_DONT_ANIMATE )
                        pGraphic->Draw( pOutDev, aLogPos, aLogSize, &aAttr );
                }
            }
            else
            {
                if( bRotate && !bRota180 )
                    aAttr.SetRotation( nDrehWink / 10 );

                pGraphic->Draw( pOutDev, aLogPos, aLogSize, &aAttr );
            }
        }
        else
        {
            // MetaFiles
            const ULONG nOldDrawMode = pOutDev->GetDrawMode();

            if( bRota180 )
            {
                aLogPos.X() -= ( aLogSize.Width() - 1L );
                aLogPos.Y() -= ( aLogSize.Height() - 1L );
            }

            // Falls Modus GRAYBITMAP, wollen wir auch Mtf's als Graustufen darstellen
            if( nOldDrawMode & DRAWMODE_GRAYBITMAP )
            {
                ULONG nNewDrawMode = nOldDrawMode;
                nNewDrawMode &= ~( DRAWMODE_BLACKLINE | DRAWMODE_BLACKFILL | DRAWMODE_WHITEFILL | DRAWMODE_NOFILL );
                pOutDev->SetDrawMode( nNewDrawMode |= DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL  );
            }

            pGraphic->Draw( pOutDev, aLogPos, aLogSize, &aAttr );
            pOutDev->SetDrawMode( nOldDrawMode );
        }
    }

    // auch GRAPHIC_NONE oder SwappedOut( AsyncSwap )
    if( ( bEmptyPresObj || bDraft ) &&
        ( !bDraft || ( ( rInfoRec.nPaintMode & SDRPAINTMODE_HIDEDRAFTGRAF ) == 0 ) ) &&
        !bJustFillCache )
    {
        XubString   aText;
        Bitmap*     pBmp = NULL;
        FASTBOOL    bFill = FALSE;

        if( bEmptyPresObj )
            bFill = !ImpPaintEmptyPres( pOutDev );

        // und nun noch einen grauen Rahmen drum herum, Text rein, ...
        if( !bEmptyPresObj )
        {
            aText = aFileName;

            if(!aText.Len())
            {
                aText = aName;

                if( bLoading )
                {
                    aText.AppendAscii(" ...");
                    //FASTBOOL bNoName=aText.Len()==0;
                    //if (!bNoName) aText.Insert(' ',0);
                    //else aText.Insert("...",0);
                    //aText.Insert("Loading",0);
                }
            }

#ifndef SVX_LIGHT
            pBmp = new Bitmap( ResId ( BMAP_GrafikEi, ImpGetResMgr() ) );
#endif
        }

        ImpPaintReplacement( pOutDev, aText, pBmp, bFill );
        delete pBmp;
    }

    if( bPrn )
        ForceSwapOut();

    return( HasText() ? SdrTextObj::Paint( rOut, rInfoRec ) : TRUE );
}

// -----------------------------------------------------------------------------

SdrObject* SdrGrafObj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    return ImpCheckHit( rPnt, nTol, pVisiLayer, TRUE );
}

// -----------------------------------------------------------------------------

void SdrGrafObj::TakeObjNameSingul(XubString& rName) const
{
    GraphicType eType = pGraphic->GetType();

    if( !IsLinkedGraphic() )
    {
        switch( eType )
        {
            case GRAPHIC_BITMAP     : rName=ImpGetResStr(STR_ObjNameSingulGRAFBMP);  break;
            case GRAPHIC_GDIMETAFILE: rName=ImpGetResStr(STR_ObjNameSingulGRAFMTF);  break;
            case GRAPHIC_NONE       : rName=ImpGetResStr(STR_ObjNameSingulGRAFNONE); break;
            default                 : rName=ImpGetResStr(STR_ObjNameSingulGRAF);     break;
        }
    }
    else
    {
        switch( eType )
        {
            case GRAPHIC_BITMAP     : rName=ImpGetResStr(STR_ObjNameSingulGRAFBMP);  break;
            case GRAPHIC_GDIMETAFILE: rName=ImpGetResStr(STR_ObjNameSingulGRAFMTF);  break;
            case GRAPHIC_NONE       : rName=ImpGetResStr(STR_ObjNameSingulGRAFNONE); break;
            default                 : rName=ImpGetResStr(STR_ObjNameSingulGRAF);     break;
        }
    }

    if(aName.Len())
    {
        rName.AppendAscii(" '");
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

// -----------------------------------------------------------------------------

void SdrGrafObj::TakeObjNamePlural( XubString& rName ) const
{
    const GraphicType eType = pGraphic->GetType();

    if( IsLinkedGraphic() )
    {
        switch( eType )
        {
            case GRAPHIC_BITMAP     : rName=ImpGetResStr(STR_ObjNamePluralGRAFBMP);  break;
            case GRAPHIC_GDIMETAFILE: rName=ImpGetResStr(STR_ObjNamePluralGRAFMTF);  break;
            case GRAPHIC_NONE       : rName=ImpGetResStr(STR_ObjNamePluralGRAFNONE); break;
            default                 : rName=ImpGetResStr(STR_ObjNamePluralGRAF);     break;
        }
    }
    else
    {
        switch( eType )
        {
            case GRAPHIC_BITMAP     : rName=ImpGetResStr(STR_ObjNamePluralGRAFBMP);  break;
            case GRAPHIC_GDIMETAFILE: rName=ImpGetResStr(STR_ObjNamePluralGRAFMTF);  break;
            case GRAPHIC_NONE       : rName=ImpGetResStr(STR_ObjNamePluralGRAFNONE); break;
            default                 : rName=ImpGetResStr(STR_ObjNamePluralGRAF);     break;
        }
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
        SetGraphicLink( aFileName, aFilterName );

#ifdef GRAFATTR
    if( ( pGrafAttr = (SdrGrafSetItem*) ImpSetNewAttr( pGrafAttr, ( (SdrGrafObj&) rObj ).pGrafAttr ) ) != NULL )
        ImpSetAttrToGrafInfo();
#endif // GRAFATTR
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
        if( nGrafStreamPos !=0 )
        {
            ForceSwapIn();
            nGrafStreamPos = 0;
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
    GraphicAttr     aAttr( aGrafInfo );
    USHORT          nMirrorCase = ( aGeo.nDrehWink == 18000 ) ? ( bMirrored ? 3 : 4 ) : ( bMirrored ? 2 : 1 );
    FASTBOOL        bHMirr = nMirrorCase == 2 || nMirrorCase == 4;
    FASTBOOL        bVMirr = nMirrorCase == 3 || nMirrorCase == 4;

    aAttr.SetMirrorFlags( ( bHMirr ? BMP_MIRROR_HORZ : 0 ) | ( bVMirr ? BMP_MIRROR_VERT : 0 ) );
    pGraphic->SetAnimationNotifyHdl( LINK( this, SdrGrafObj, ImpAnimationHdl ) );
    pGraphic->StartAnimation( pOutDev, rPoint, rSize, nExtraData, &aAttr );
}

// -----------------------------------------------------------------------------

void SdrGrafObj::StopAnimation(OutputDevice* pOutDev, long nExtraData)
{
    pGraphic->StopAnimation( pOutDev, nExtraData );
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

// -----------------------------------------------------------------------------

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
            ((SdrGrafObj*)this)->nGrafStreamPos = rOut.Tell();

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
    rOut << (BOOL)aFileName.Len();

#ifdef GRAFATTR
    SfxItemPool* pPool = GetItemPool();

    if(pPool)
        pPool->StoreSurrogate(rOut, pGrafAttr);
    else
        rOut << UINT16( SFX_ITEMS_NULL );
#endif // GRAFATTR

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
        rIn.ReadByteString(aFileName);
    }
    else
    {
        aFilterName = String();
        aFilterName.AppendAscii("BMP - MS Windows");
    }

    if(aFileName.Len())
    {
#ifndef SVX_LIGHT
        GraphicFilter*  pFilter = GetGrfFilter();
        USHORT          nFilter = pFilter->GetImportFormatNumber( aFilterName );
        SvFileStream    aIStm( aFileName, STREAM_READ | STREAM_SHARE_DENYNONE );
        USHORT          nError = pFilter->ImportGraphic( aGraphic, aFileName, aIStm, nFilter );

        SetGraphicLink( aFileName, aFilterName );
#else
        DBG_ERROR("SdrGrafObj::ReadDataTilV10(): SVX_LIGHT kann keine Graphic-Links");
#endif
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

    nGrafStreamPos = 0;
    bSwappedOut = bNotLoaded = FALSE;

    SdrDownCompat   aCompat( rIn, STREAM_READ );
    FASTBOOL        bDelayedLoad = ( pModel != NULL ) && pModel->IsSwapGraphics();

#ifdef DBG_UTIL
    aCompat.SetID("SdrGrafObj");
#endif

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
                bNotLoaded = bSwappedOut = TRUE;

            // Ist die Grafik defekt, oder wurde nur eine leere Graphik eingelesen?
            // Daran soll mein Read jedoch nicht scheitern.
            if( rIn.GetError() != 0 )
                rIn.ResetError();
        }

        rIn >> aCropRect;
        rIn >> bTmp; bMirrored = bTmp;

        rIn.ReadByteString(aName);
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

#ifdef GRAFATTR

        if( aCompat.GetBytesLeft() > 0 )
        {
            SfxItemPool* pPool = GetItemPool();

            if( pPool )
            {
                USHORT nWhich = SDRATTRSET_GRAF;

                pGrafAttr = (const SdrGrafSetItem*) ImpSetNewAttr( pGrafAttr, NULL );
                pGrafAttr = (const SdrGrafSetItem*) pPool->LoadSurrogate( rIn, nWhich, 0 );

                if( pGrafAttr )
                {
                    if( pStyleSheet )
                        ( (SfxItemSet*) &pGrafAttr->GetItemSet() )->SetParent( &pStyleSheet->GetItemSet() );

                    ImpSetAttrToGrafInfo();
                }
            }
            else
            {
                UINT16 nSuroDummy;
                rIn >> nSuroDummy;
            }
        }
        else
            bCopyToPoolOnAfterRead = TRUE;

#endif // GRAFATTR

        if( bGraphicLink && aFileName.Len() && aFilterName.Len() )
        {
#ifndef SVX_LIGHT
            SetGraphicLink( aFileName, aFilterName );

            if( pGraphicLink && !bDelayedLoad )
            {
                BOOL bIsChanged = pModel->IsChanged();
                pGraphicLink->UpdateSynchron();
                pModel->SetChanged( bIsChanged );
            }
            else
                bNotLoaded = bSwappedOut = TRUE;
#else
            Graphic aGraphic;
            GraphicFilter*  pFilter = SVX_LIGHT_GetGrfFilter_Impl();
            USHORT          nFilter = pFilter->GetImportFormatNumber( aFilterName );
            SvFileStream    aIStm( aFileName, STREAM_READ | STREAM_SHARE_DENYNONE );
            USHORT          nError = pFilter->ImportGraphic( aGraphic, aFileName, aIStm, nFilter );

            pGraphic->SetGraphic( aGraphic );
            bGraphicLink = FALSE;
            aFileName = aFilterName = String();

//            DBG_ERROR("SdrGrafObj::ReadData(): SVX_LIGHT kann keine Graphic-Links");

#endif
        }
    }
}

// -----------------------------------------------------------------------------

#ifdef WNT
#pragma optimize ("",on)
#endif

// -----------------------------------------------------------------------------

Rectangle SdrGrafObj::ImpGetOutputRect( const OutputDevice* pOutDev ) const
{
    return GetSnapRect();
}

// -----------------------------------------------------------------------------

Rectangle SdrGrafObj::GetAnimationRect(const OutputDevice* pOutDev) const
{
    return ImpGetOutputRect( pOutDev );
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetAnimationSupervisor( OutputDevice* pDisplayDev, BOOL bObjSupervises )
{
    ForceSwapIn();
    List* pAInfoList = pGraphic->GetAnimationInfoList();

    if ( pAInfoList )
    {
        for( AInfo* pAInfo = (AInfo*) pAInfoList->First(); pAInfo; pAInfo = (AInfo*) pAInfoList->Next() )
        {
            if( pAInfo->pOutDev == pDisplayDev )
            {
                pAInfo->nExtraData = ( bObjSupervises ? 1L : (long) this );

                if ( !bObjSupervises )
                    pAInfo->bPause = FALSE;
            }
        }
    }
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ResetAnimationLoopCount()
{
    if( pGraphic->IsAnimated() )
    {
        ForceSwapIn();
        pGraphic->ResetAnimationLoopCount();
    }
}

// -----------------------------------------------------------------------------

void SdrGrafObj::RestartAnimation(SdrPageView* pPageView) const
{
    // ToDo: hier noch entsprechend implementieren wie im TextObj
    SdrRectObj::RestartAnimation( pPageView );
}

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
                SfxItemSet aSet(GetModel()->GetItemPool());
                TakeAttributes(aSet, FALSE, TRUE);

                // Bitmap als Fuellung holen
                aSet.Put(XFillStyleItem(XFILL_BITMAP));
                Bitmap aBitmap( GetTransformedGraphic().GetBitmap() );
                XOBitmap aXBmp(aBitmap, XBITMAP_STRETCH);
                aSet.Put(XFillBitmapItem(String(), aXBmp));
                aSet.Put(XFillBmpTileItem(FALSE));
                pRetval->NbcSetAttributes(aSet, FALSE);
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

#ifdef GRAFATTR

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

// -----------------------------------------------------------------------------

void SdrGrafObj::ForceDefaultAttr( SfxItemPool* pPool )
{
    SdrRectObj::ForceDefaultAttr( pPool );

    if( pPool )
    {
        if( !pGrafAttr )
        {
            SdrGrafSetItem  aSetItem( pPool );
            SfxItemSet&     rSet = aSetItem.GetItemSet();

            rSet.Put( SdrGrafLuminanceItem( 0 ) );
            rSet.Put( SdrGrafContrastItem( 0 ) );
            rSet.Put( SdrGrafRedItem( 0 ) );
            rSet.Put( SdrGrafGreenItem( 0 ) );
            rSet.Put( SdrGrafBlueItem( 0 ) );
            rSet.Put( SdrGrafGamma100Item( 100 ) );
            rSet.Put( SdrGrafTransparenceItem( 0 ) );
            rSet.Put( SdrGrafInvertItem( FALSE ) );
            rSet.Put( SdrGrafModeItem( GRAPHICDRAWMODE_STANDARD ) );
            rSet.Put( SdrGrafCropItem( 0, 0, 0, 0 ) );

            pGrafAttr = (SdrGrafSetItem*) ImpSetNewAttr( pGrafAttr, &aSetItem, FALSE );
        }
    }
}

// -----------------------------------------------------------------------------

USHORT SdrGrafObj::GetSetItemCount() const
{
    return( 1 + SdrRectObj::GetSetItemCount() );
}

// -----------------------------------------------------------------------------

const SfxSetItem* SdrGrafObj::GetSetItem( USHORT nNum ) const
{
    return( !nNum ? pGrafAttr : SdrRectObj::GetSetItem( --nNum ) );
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetSetItem( USHORT nNum, const SfxSetItem* pAttr )
{
    if( !nNum )
        pGrafAttr = (const SdrGrafSetItem*) pAttr;
    else
        SdrRectObj::SetSetItem( --nNum, pAttr );
}

// -----------------------------------------------------------------------------

SfxSetItem* SdrGrafObj::MakeNewSetItem( USHORT nNum, FASTBOOL bClone ) const
{
    SfxSetItem* pRet;

    if( !nNum )
    {
        if( bClone && pGrafAttr )
            pRet = new SdrGrafSetItem( *pGrafAttr );
        else
            pRet = new SdrGrafSetItem( GetItemPool() );
    }
    else
        pRet = SdrRectObj::MakeNewSetItem( --nNum, bClone );

    return pRet;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcSetStyleSheet( SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr )
{
    SetXPolyDirty();
    SdrRectObj::NbcSetStyleSheet( pNewStyleSheet, bDontRemoveHardAttr );
    ImpSetAttrToGrafInfo();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcSetAttributes( const SfxItemSet& rAttr, FASTBOOL bReplaceAll )
{
    SetXPolyDirty();
    SdrRectObj::NbcSetAttributes( rAttr, bReplaceAll );
    ImpSetAttrToGrafInfo();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ImpSetAttrToGrafInfo()
{
    if( pGrafAttr )
    {
        const SfxItemSet&       rSet = pGrafAttr->GetItemSet();
        const USHORT            nTrans = ( (SdrGrafTransparenceItem&) rSet.Get( SDRATTR_GRAFTRANSPARENCE ) ).GetValue();
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
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ImpSetGrafInfoToAttr()
{
    if( pGrafAttr )
    {
        SdrGrafSetItem  aSetItem( *pGrafAttr );
        SfxItemSet&     rSet = aSetItem.GetItemSet();

        rSet.Put( SdrGrafLuminanceItem( aGrafInfo.GetLuminance() ) );
        rSet.Put( SdrGrafContrastItem( aGrafInfo.GetContrast() ) );
        rSet.Put( SdrGrafRedItem( aGrafInfo.GetChannelR() ) );
        rSet.Put( SdrGrafGreenItem( aGrafInfo.GetChannelG() ) );
        rSet.Put( SdrGrafBlueItem( aGrafInfo.GetChannelB() ) );
        rSet.Put( SdrGrafGamma100Item( FRound( aGrafInfo.GetGamma() * 100.0 ) ) );
        rSet.Put( SdrGrafTransparenceItem( (USHORT) FRound( aGrafInfo.GetTransparency() / 2.55 ) ) );
        rSet.Put( SdrGrafInvertItem( aGrafInfo.IsInvert() ) );
        rSet.Put( SdrGrafModeItem( aGrafInfo.GetDrawMode() ) );
        rSet.Put( SdrGrafCropItem( aGrafInfo.GetLeftCrop(), aGrafInfo.GetTopCrop(), aGrafInfo.GetRightCrop(), aGrafInfo.GetBottomCrop() ) );

        pGrafAttr = (SdrGrafSetItem*) ImpSetNewAttr( pGrafAttr, &aSetItem );
    }
}

#endif // GRAFATTR

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
        if( pModel != NULL && pModel->IsSwapGraphics() && pGraphic->GetSizeBytes() > 20480 )
        {
            SdrViewIter aIter( this );
            SdrView*    pView = aIter.FirstView();
            BOOL        bVisible = FALSE;

            while( !bVisible && pView )
            {
                bVisible = !pView->IsGrafDraft();

                if( !bVisible )
                    pView = aIter.NextView();
            }

            if( !bVisible )
            {
                const ULONG nSwapMode = pModel->GetSwapGraphicsMode();

                if( ( nGrafStreamPos || pGraphicLink ) && ( nSwapMode & SDR_SWAPGRAPHICSMODE_PURGE ) )
                {
                    pRet = NULL;
                    bSwappedOut = bNotLoaded = TRUE;
                }
                else if( nSwapMode & SDR_SWAPGRAPHICSMODE_TEMP )
                {
                    pRet = GRFMGR_AUTOSWAPSTREAM_TEMP;
                    nGrafStreamPos = 0;
                    bSwappedOut = TRUE;
                    bNotLoaded = FALSE;
                }
            }
        }
    }
    else if( pO->IsInSwapIn() )
    {
        if( bSwappedOut )
        {
            if( bNotLoaded )
            {
                // kann aus dem original Doc-Stream nachgeladen werden...
                if( pModel != NULL )
                {
                    if( nGrafStreamPos )
                    {
                        FASTBOOL    bDeleteStream = FALSE;
                        SvStream*   pStream = pModel->GetDocumentStream( bDeleteStream );

                        if( pStream != NULL )
                        {
                            Graphic aGraphic;

                            pStream->Seek( nGrafStreamPos );
                            *pStream >> aGraphic;
                            pGraphic->SetGraphic( aGraphic );

                            if( bDeleteStream )
                                delete pStream;

                            pRet = GRFMGR_AUTOSWAPSTREAM_LOADED;
                            bNotLoaded = bSwappedOut = FALSE;
                        }
                    }
#ifndef SVX_LIGHT
                    else if( pGraphicLink )
                    {
                        BOOL bIsChanged = pModel->IsChanged();
                        pGraphicLink->UpdateSynchron();
                        pModel->SetChanged( bIsChanged );
                        bNotLoaded = bSwappedOut = FALSE;
                    }
#endif
                }
            }
            else
            {
                pRet = GRFMGR_AUTOSWAPSTREAM_TEMP;
                bNotLoaded = bSwappedOut = FALSE;
            }
        }
    }

    return (long)(void*) pRet;
}

// -----------------------------------------------------------------------------

IMPL_LINK( SdrGrafObj, ImpAnimationHdl, Animation*, pAnimation )
{
    // Wenn wir nicht mehr da sind, stoppen wir natuerlich alles und kehren gleich zurueck
    if( !bInserted )
    {
        pAnimation->Stop();
        return 0L;
    }

    List*   pAInfoList = pAnimation->GetAInfoList();
    AInfo*  pAInfo = (AInfo*) pAInfoList->First();
    BOOL    bExtern = FALSE;

    // Alle Extra-Data auf 0 setzen, wenn keine andere ExtraData
    // ausser der eigenen (1) gesetzt;
    // groesser als 1 bedeutet, daß die Animation von aussen gestartet
    // wurde, z.B. von der DiaShow.
    while( pAInfo != NULL )
    {
        if( pAInfo->nExtraData == 1L )
            pAInfo->nExtraData = 0;
        else if( pAInfo->nExtraData > 1L )
            bExtern = TRUE;

        pAInfo = (AInfo*) pAInfoList->Next();
    }

    if( pModel!=NULL && pPage!=NULL && bInserted && pAInfoList->Count() )
    {
        USHORT      nPageNum=pPage->GetPageNum();
        FASTBOOL    bMaster=pPage->IsMasterPage() && !bNotVisibleAsMaster;
        USHORT      nLsAnz=pModel->GetListenerCount();

        for( USHORT nLsNum=0; nLsNum<nLsAnz; nLsNum++ )
        {
            SfxListener* pLs=pModel->GetListener(nLsNum);
            SdrMarkView* pView=PTR_CAST(SdrMarkView,pLs);

            if( pView && ( pView->IsAnimationEnabled() || bExtern ) )
            {
                FASTBOOL    bMrk=pView->IsObjMarked(this);
                USHORT      nPvAnz=pView->GetPageViewCount();

                for (USHORT nPvNum=0; nPvNum<nPvAnz; nPvNum++)
                {
                    SdrPageView*    pPV=pView->GetPageViewPvNum(nPvNum);
                    SdrPage*        pPg=pPV->GetPage();

                    if (pPV->GetVisibleLayers().IsSet(nLayerID))
                    {
                        FASTBOOL bJa=pPg==pPage;

                        if (!bJa && bMaster && !pPg->IsMasterPage())
                        {
                            USHORT nMasterAnz=pPg->GetMasterPageCount();

                            for (USHORT nMasterNum=0; nMasterNum<nMasterAnz && !bJa; nMasterNum++)
                            {
                                const SdrMasterPageDescriptor& rMPD=pPg->GetMasterPageDescriptor(nMasterNum);
                                bJa=nPageNum==rMPD.GetPageNum() && rMPD.GetVisibleLayers().IsSet(nLayerID);
                            }
                        }
                        if (bJa)
                        {
                            USHORT nOutAnz=pView->GetWinCount();

                            for (USHORT nOutNum=0; nOutNum<nOutAnz; nOutNum++)
                            {
                                OutputDevice* pOut=pView->GetWin(nOutNum);

                                if( pOut->GetOutDevType()==OUTDEV_WINDOW )
                                {
                                    Rectangle   aDrawRect( GetAnimationRect( pOut ) );
                                    const Point aOffset( pPV->GetOffset() );
                                    FASTBOOL    bFound = FALSE;

                                    aDrawRect.Move( aOffset.X(), aOffset.Y() );

                                    // Flag am gefundenen Objekt setzen
                                    pAInfo=(AInfo*)pAInfoList->First();
                                    while( pAInfo!=NULL && !bFound )
                                    {
                                        if (pAInfo->aStartOrg==aDrawRect.TopLeft() &&
                                            pAInfo->aStartSize==aDrawRect.GetSize() &&
                                            pAInfo->pOutDev==pOut )
                                        {
                                            if( pAInfo->nExtraData==0 )
                                                pAInfo->nExtraData=1L;

                                            pAInfo->bPause = bMrk;
                                            bFound = TRUE;
                                        }

                                        pAInfo=(AInfo*)pAInfoList->Next();
                                    }

                                    // Falls kein Record gefunden, wird ein neuer erzeugt
                                    // Das passiert z.B., wenn das Obj auf einer MasterPage liegt
                                    // und diese mittels MasterPagePaintCache angezeigt wurde.
                                    if (!bFound)
                                    {
                                        pAInfo = new AInfo;

                                        // erst einmal soviel wie moeglich bekanntes setzen
                                        *pAInfo = *(AInfo*) pAInfoList->GetObject( 0L );
                                        pAInfo->aStartOrg=aDrawRect.TopLeft();
                                        pAInfo->aStartSize=aDrawRect.GetSize();
                                        pAInfo->pOutDev=pOut;
                                        pAInfo->nExtraData=1;
                                        pAInfo->bPause=bMrk;
                                        pAInfoList->Insert(pAInfo);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Alle Objekte mit nicht gesetztem Flag loeschen
    pAInfo = (AInfo*) pAInfoList->First();

    while( pAInfo != NULL )
    {
        if( pAInfo->nExtraData == 0 )
        {
            delete (AInfo*)pAInfoList->Remove(pAInfo);
            pAInfo=(AInfo*)pAInfoList->GetCurObject();
        }
        else
            pAInfo=(AInfo*)pAInfoList->Next();
    }

    return 0;
}

