/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objembed.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:46:21 $
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

#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDOBJECTCREATOR_HPP_
#include <com/sun/star/embed/XEmbedObjectCreator.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XCOMPONENTSUPPLIER_HPP_
#include <com/sun/star/embed/XComponentSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDPERSIST_HPP_
#include <com/sun/star/embed/XEmbedPersist.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif

#include "objsh.hxx"
#include "app.hxx"
#include "objshimp.hxx"
#include "sfx.hrc"
#include "event.hxx"

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
                pResult = (SfxObjectShell*)(sal_Int32)( xParentTunnel->getSomething(
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
void SfxObjectShell::OnDocumentPrinterChanged( Printer * pNewPrinter )
{
    // virtual method
}

// -----------------------------------------------------------------------
Rectangle SfxObjectShell::GetVisArea( USHORT nAspect ) const
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
            SetModified( TRUE );
            SFX_APP()->NotifyEvent(SfxEventHint( SFX_EVENT_VISAREACHANGED, this));

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
ULONG SfxObjectShell::GetMiscStatus() const
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
    rDesc.mbCanLink = FALSE;
}

// -----------------------------------------------------------------------
void SfxObjectShell::DoDraw( OutputDevice* pDev,
                            const Point & rObjPos,
                            const Size & rSize,
                            const JobSetup & rSetup,
                            USHORT nAspect )
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
                               USHORT nAspect )
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
        if( pMtf->IsRecord() && pDev->GetOutDevType() != OUTDEV_PRINTER )
            pMtf->Stop();
        else
            pMtf = NULL;
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
        pImp->mpObjectContainer = new comphelper::EmbeddedObjectContainer( ((SfxObjectShell*)this)->GetStorage() );
    return *pImp->mpObjectContainer;
}

void SfxObjectShell::ClearEmbeddedObjects()
{
    // frees alle space taken by embedded objects
    DELETEZ( pImp->mpObjectContainer );
}

