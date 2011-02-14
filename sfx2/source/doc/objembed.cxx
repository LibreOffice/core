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
#include "precompiled_sfx2.hxx"
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/embed/XEmbedObjectCreator.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>

#include <sfx2/objsh.hxx>
#include <sfx2/app.hxx>
#include "objshimp.hxx"
#include <sfx2/sfx.hrc>
#include <sfx2/event.hxx>

#include <comphelper/seqstream.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <svtools/embedtransfer.hxx>
#include <vcl/outdev.hxx>
#include <vcl/gdimtf.hxx>

using namespace ::com::sun::star;

// -----------------------------------------------------------------------
// TODO/LATER: this workaround must be replaced by API in future if possible
SfxObjectShell* SfxObjectShell::GetParentShellByModel_Impl()
{
    SfxObjectShell* pResult = NULL;

    try {
        uno::Reference< container::XChild > xChildModel( GetModel(), uno::UNO_QUERY );
        if ( xChildModel.is() )
        {
            uno::Reference< lang::XUnoTunnel > xParentTunnel( xChildModel->getParent(), uno::UNO_QUERY );
            if ( xParentTunnel.is() )
            {
                SvGlobalName aSfxIdent( SFX_GLOBAL_CLASSID );
                pResult = reinterpret_cast<SfxObjectShell*>(xParentTunnel->getSomething(
                                                uno::Sequence< sal_Int8 >( aSfxIdent.GetByteSequence() ) ) );
            }
        }
    }
    catch( uno::Exception& )
    {
        // TODO: error handling
    }

    return pResult;
}

// -----------------------------------------------------------------------
Printer* SfxObjectShell::GetDocumentPrinter()
{
    SfxObjectShell* pParent = GetParentShellByModel_Impl();
    if ( pParent )
        return pParent->GetDocumentPrinter();
    return NULL;
}

// -----------------------------------------------------------------------
OutputDevice* SfxObjectShell::GetDocumentRefDev()
{
    SfxObjectShell* pParent = GetParentShellByModel_Impl();
    if ( pParent )
        return pParent->GetDocumentRefDev();
    return NULL;
}

// -----------------------------------------------------------------------
void SfxObjectShell::OnDocumentPrinterChanged( Printer* /*pNewPrinter*/ )
{
    // virtual method
}

// -----------------------------------------------------------------------
Rectangle SfxObjectShell::GetVisArea( sal_uInt16 nAspect ) const
{
    if( nAspect == ASPECT_CONTENT )
        return pImp->m_aVisArea;
    else if( nAspect == ASPECT_THUMBNAIL )
    {
        Rectangle aRect;
        aRect.SetSize( OutputDevice::LogicToLogic( Size( 5000, 5000 ),
                                         MAP_100TH_MM, GetMapUnit() ) );
        return aRect;
    }
    return Rectangle();
}

// -----------------------------------------------------------------------
const Rectangle& SfxObjectShell::GetVisArea() const
{
    pImp->m_aVisArea = GetVisArea( ASPECT_CONTENT );
    return pImp->m_aVisArea;
}

// -----------------------------------------------------------------------
void SfxObjectShell::SetVisArea( const Rectangle & rVisArea )
{
    if( pImp->m_aVisArea != rVisArea )
    {
        pImp->m_aVisArea = rVisArea;
        if ( GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
        {
            if ( IsEnableSetModified() )
                SetModified( sal_True );

               SFX_APP()->NotifyEvent(SfxEventHint( SFX_EVENT_VISAREACHANGED, GlobalEventConfig::GetEventName(STR_EVENT_VISAREACHANGED), this));

            /*
            Size aSize (GetVisArea().GetSize());
            if ( GetIPEnv() && GetIPEnv()->GetEditWin() )
                ViewChanged( ASPECT_CONTENT );
            */


            // OutPlace die Gr"o\se des MDI-Fensters anpassen
            // Unbedingt den Gr"o\senvergleich machen, spart nicht nur Zeit, sondern
            // vermeidet auch Rundungsfehler !
            /*
                // in case of ole outplace editing the frame should be found
                SfxViewFrame* pFrameToResize = pFrame ? pFrame : SfxViewFrame::GetFirst( GetObjectShell() );

                if ( pFrameToResize && !pIPF && rRect.GetSize() != aSize &&
                    !pFrameToResize->IsAdjustPosSizePixelLocked_Impl() )

                {
                    // Zuerst die logischen Koordinaten von IP-Objekt und EditWindow
                    // ber"ucksichtigen
                    SfxViewShell *pShell = pFrameToResize->GetViewShell();
                    Window *pWindow = pShell->GetWindow();

                    // Da in den Applikationen bei der R"ucktransformation immer die
                    // Eckpunkte tranformiert werden und nicht die Size (um die Ecken
                    // alignen zu k"onnen), transformieren wir hier auch die Punkte, um
                    // m"oglichst wenig Rundungsfehler zu erhalten.
                    Rectangle aRect = pWindow->LogicToPixel( rRect );
                    Size aSize = aRect.GetSize();
                    pShell->GetWindow()->SetSizePixel( aSize );
                    pFrameToResize->DoAdjustPosSizePixel( pShell, Point(), aSize );
                }

            // bei InPlace die View skalieren
            if ( GetIPEnv() && GetIPEnv()->GetEditWin() && !bDisableViewScaling && pIPF )
                pIPF->GetEnv_Impl()->MakeScale( rRect.GetSize(), GetMapUnit(),
                            pIPF->GetViewShell()->GetWindow()->GetOutputSizePixel() );
           */
        }
    }
}

// -----------------------------------------------------------------------
void SfxObjectShell::SetVisAreaSize( const Size & rVisSize )
{
    SetVisArea( Rectangle( GetVisArea().TopLeft(), rVisSize ) );
}

// -----------------------------------------------------------------------
sal_uIntPtr SfxObjectShell::GetMiscStatus() const
{
    return 0;
}

// -----------------------------------------------------------------------
MapUnit SfxObjectShell::GetMapUnit() const
{
    return pImp->m_nMapUnit;
}

// -----------------------------------------------------------------------
void SfxObjectShell::SetMapUnit( MapUnit nMapUnit )
{
    pImp->m_nMapUnit = nMapUnit;
}

// -----------------------------------------------------------------------
void SfxObjectShell::FillTransferableObjectDescriptor( TransferableObjectDescriptor& rDesc ) const
{
    sal_uInt32 nClipFormat;
    String aAppName, aShortName;
    FillClass( &rDesc.maClassName, &nClipFormat, &aAppName, &rDesc.maTypeName, &aShortName, SOFFICE_FILEFORMAT_CURRENT );

    rDesc.mnViewAspect = ASPECT_CONTENT;
    rDesc.mnOle2Misc = GetMiscStatus();
    rDesc.maSize = OutputDevice::LogicToLogic( GetVisArea().GetSize(), GetMapUnit(), MAP_100TH_MM );
    rDesc.maDragStartPos = Point();
    rDesc.maDisplayName = String();
    rDesc.mbCanLink = sal_False;
}

// -----------------------------------------------------------------------
void SfxObjectShell::DoDraw( OutputDevice* pDev,
                            const Point & rObjPos,
                            const Size & rSize,
                            const JobSetup & rSetup,
                            sal_uInt16 nAspect )
{
    MapMode aMod = pDev->GetMapMode();
    Size aSize = GetVisArea( nAspect ).GetSize();
    MapMode aWilliMode( GetMapUnit() );
    aSize = pDev->LogicToLogic( aSize, &aWilliMode, &aMod );
    if( aSize.Width() && aSize.Height() )
    {
        Fraction aXF( rSize.Width(), aSize.Width() );
        Fraction aYF( rSize.Height(), aSize.Height() );

//REMOVE            Point aOrg = rObjPos;
//REMOVE            aMod.SetMapUnit( MAP_100TH_MM );
//REMOVE            aSize = pDev->LogicToLogic( GetVisArea( nAspect ).GetSize(), &aMod, &aWilliMode );
        DoDraw_Impl( pDev, rObjPos, aXF, aYF, rSetup, nAspect );
    }
}

// -----------------------------------------------------------------------
void SfxObjectShell::DoDraw_Impl( OutputDevice* pDev,
                               const Point & rViewPos,
                               const Fraction & rScaleX,
                               const Fraction & rScaleY,
                               const JobSetup & rSetup,
                               sal_uInt16 nAspect )
{
    Rectangle aVisArea  = GetVisArea( nAspect );
    // MapUnit des Ziels
    MapMode aMapMode( GetMapUnit() );
    aMapMode.SetScaleX( rScaleX );
    aMapMode.SetScaleY( rScaleY );

    // Ziel in Pixel
    Point aOrg   = pDev->LogicToLogic( rViewPos, NULL, &aMapMode );
    Point aDelta = aOrg - aVisArea.TopLeft();

    // Origin entsprechend zum sichtbaren Bereich verschieben
    // Origin mit Scale setzen
    aMapMode.SetOrigin( aDelta );

    // Deviceeinstellungen sichern
    pDev->Push();

    Region aRegion;
    if( pDev->IsClipRegion() && pDev->GetOutDevType() != OUTDEV_PRINTER )
    {
        aRegion = pDev->GetClipRegion();
        aRegion = pDev->LogicToPixel( aRegion );
    }
    pDev->SetRelativeMapMode( aMapMode );

    GDIMetaFile * pMtf = pDev->GetConnectMetaFile();
    if( pMtf )
    {
        if( pMtf->IsRecord() && pDev->GetOutDevType() != OUTDEV_PRINTER )
            pMtf->Stop();
        else
            pMtf = NULL;
    }
// #ifndef UNX
    if( pDev->IsClipRegion() && pDev->GetOutDevType() != OUTDEV_PRINTER )
// #endif
    {
        aRegion = pDev->PixelToLogic( aRegion );
        pDev->SetClipRegion( aRegion );
    }
    if( pMtf )
        pMtf->Record( pDev );

//REMOVE        SvOutPlaceObjectRef xOutRef( this );
//REMOVE        if ( xOutRef.Is() )
//REMOVE            xOutRef->DrawObject( pDev, rSetup, rSize, nAspect );
//REMOVE        else
        Draw( pDev, rSetup, nAspect );
//REMOVE        DrawHatch( pDev, aVisArea.TopLeft(), aVisArea.GetSize() );

    // Deviceeinstellungen wieder herstellen
    pDev->Pop();

}

comphelper::EmbeddedObjectContainer& SfxObjectShell::GetEmbeddedObjectContainer() const
{
    if ( !pImp->mpObjectContainer )
        pImp->mpObjectContainer = new comphelper::EmbeddedObjectContainer( ((SfxObjectShell*)this)->GetStorage(), GetModel() );
    return *pImp->mpObjectContainer;
}

void SfxObjectShell::ClearEmbeddedObjects()
{
    // frees alle space taken by embedded objects
    DELETEZ( pImp->mpObjectContainer );
}

