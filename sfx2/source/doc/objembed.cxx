/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
            if ( IsEnableSetModified() )
                SetModified( TRUE );

               SFX_APP()->NotifyEvent(SfxEventHint( SFX_EVENT_VISAREACHANGED, GlobalEventConfig::GetEventName(STR_EVENT_VISAREACHANGED), this));
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
    {
        if( pMtf->IsRecord() && pDev->GetOutDevType() != OUTDEV_PRINTER )
            pMtf->Stop();
        else
            pMtf = NULL;
    }
    if( pDev->IsClipRegion() && pDev->GetOutDevType() != OUTDEV_PRINTER )
    {
        aRegion = pDev->PixelToLogic( aRegion );
        pDev->SetClipRegion( aRegion );
    }
    if( pMtf )
        pMtf->Record( pDev );

        Draw( pDev, rSetup, nAspect );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
