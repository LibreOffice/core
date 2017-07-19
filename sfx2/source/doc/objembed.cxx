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

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/embed/XEmbedObjectCreator.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>

#include <sfx2/objsh.hxx>
#include <sfx2/app.hxx>
#include "objshimp.hxx"
#include <sfx2/event.hxx>

#include <comphelper/seqstream.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <svtools/embedtransfer.hxx>
#include <tools/fract.hxx>
#include <vcl/outdev.hxx>
#include <vcl/gdimtf.hxx>

using namespace ::com::sun::star;


// TODO/LATER: this workaround must be replaced by API in future if possible
SfxObjectShell* SfxObjectShell::GetParentShellByModel_Impl()
{
    SfxObjectShell* pResult = nullptr;

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


Printer* SfxObjectShell::GetDocumentPrinter()
{
    SfxObjectShell* pParent = GetParentShellByModel_Impl();
    if ( pParent )
        return pParent->GetDocumentPrinter();
    return nullptr;
}


OutputDevice* SfxObjectShell::GetDocumentRefDev()
{
    SfxObjectShell* pParent = GetParentShellByModel_Impl();
    if ( pParent )
        return pParent->GetDocumentRefDev();
    return nullptr;
}


void SfxObjectShell::OnDocumentPrinterChanged( Printer* /*pNewPrinter*/ )
{
    // virtual method
}


tools::Rectangle SfxObjectShell::GetVisArea( sal_uInt16 nAspect ) const
{
    if( nAspect == ASPECT_CONTENT )
        return pImpl->m_aVisArea;
    else if( nAspect == ASPECT_THUMBNAIL )
    {
        tools::Rectangle aRect;
        aRect.SetSize( OutputDevice::LogicToLogic( Size( 5000, 5000 ),
                                         MapUnit::Map100thMM, GetMapUnit() ) );
        return aRect;
    }
    return tools::Rectangle();
}


const tools::Rectangle& SfxObjectShell::GetVisArea() const
{
    pImpl->m_aVisArea = GetVisArea( ASPECT_CONTENT );
    return pImpl->m_aVisArea;
}


void SfxObjectShell::SetVisArea( const tools::Rectangle & rVisArea )
{
    if( pImpl->m_aVisArea != rVisArea )
    {
        pImpl->m_aVisArea = rVisArea;
        if ( GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
        {
            if ( IsEnableSetModified() )
                SetModified();

            SfxGetpApp()->NotifyEvent(SfxEventHint( SfxEventHintId::VisAreaChanged, GlobalEventConfig::GetEventName(GlobalEventId::VISAREACHANGED), this));
        }
    }
}


void SfxObjectShell::SetVisAreaSize( const Size & rVisSize )
{
    SetVisArea( tools::Rectangle( GetVisArea().TopLeft(), rVisSize ) );
}


MapUnit SfxObjectShell::GetMapUnit() const
{
    return pImpl->m_nMapUnit;
}


void SfxObjectShell::SetMapUnit( MapUnit nMapUnit )
{
    pImpl->m_nMapUnit = nMapUnit;
}


void SfxObjectShell::FillTransferableObjectDescriptor( TransferableObjectDescriptor& rDesc ) const
{
    SotClipboardFormatId nClipFormat;
    OUString aAppName, aShortName;
    FillClass( &rDesc.maClassName, &nClipFormat, &aAppName, &rDesc.maTypeName, &aShortName, SOFFICE_FILEFORMAT_CURRENT );

    rDesc.mnViewAspect = ASPECT_CONTENT;
    rDesc.maSize = OutputDevice::LogicToLogic( GetVisArea().GetSize(), GetMapUnit(), MapUnit::Map100thMM );
    rDesc.maDragStartPos = Point();
    rDesc.maDisplayName.clear();
}


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

        DoDraw_Impl( pDev, rObjPos, aXF, aYF, rSetup, nAspect );
    }
}


void SfxObjectShell::DoDraw_Impl( OutputDevice* pDev,
                               const Point & rViewPos,
                               const Fraction & rScaleX,
                               const Fraction & rScaleY,
                               const JobSetup & rSetup,
                               sal_uInt16 nAspect )
{
    tools::Rectangle aVisArea  = GetVisArea( nAspect );
    // MapUnit of the target
    MapMode aMapMode( GetMapUnit() );
    aMapMode.SetScaleX( rScaleX );
    aMapMode.SetScaleY( rScaleY );

    // Target in Pixels
    Point aOrg   = pDev->LogicToLogic( rViewPos, nullptr, &aMapMode );
    Point aDelta = aOrg - aVisArea.TopLeft();

    // Origin moved according to the viewable area
    // Origin set with Scale
    aMapMode.SetOrigin( aDelta );

    // Secure the Device settings
    pDev->Push();

    vcl::Region aRegion;
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
            pMtf = nullptr;
    }
    if( pDev->IsClipRegion() && pDev->GetOutDevType() != OUTDEV_PRINTER )
    {
        aRegion = pDev->PixelToLogic( aRegion );
        pDev->SetClipRegion( aRegion );
    }
    if( pMtf )
        pMtf->Record( pDev );

    Draw( pDev, rSetup, nAspect );

    // Restore Device settings
    pDev->Pop();

}

comphelper::EmbeddedObjectContainer& SfxObjectShell::GetEmbeddedObjectContainer() const
{
    if ( !pImpl->mpObjectContainer )
        pImpl->mpObjectContainer = new comphelper::EmbeddedObjectContainer( const_cast<SfxObjectShell*>(this)->GetStorage(), GetModel() );
    return *pImpl->mpObjectContainer;
}

void SfxObjectShell::ClearEmbeddedObjects()
{
    // frees all space taken by embedded objects
    DELETEZ( pImpl->mpObjectContainer );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
