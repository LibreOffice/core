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


#include <svtools/embedhlp.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/outdev.hxx>
#include <bitmaps.hlst>

#include <sal/log.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/embeddedobjectcontainer.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XChartType.hpp>
#include <tools/globname.hxx>
#include <comphelper/classids.hxx>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XStateChangeListener.hpp>
#include <com/sun/star/embed/XLinkageSupport.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/chart2/XDefaultSizeTransmitter.hpp>
#include <cppuhelper/implbase.hxx>
#include <vcl/svapp.hxx>
#include <tools/diagnose_ex.h>
#include <tools/debug.hxx>
#include <memory>

using namespace com::sun::star;

namespace svt {

class EmbedEventListener_Impl : public ::cppu::WeakImplHelper < embed::XStateChangeListener,
                                                                 document::XEventListener,
                                                                 util::XModifyListener,
                                                                 util::XCloseListener >
{
public:
    EmbeddedObjectRef*          pObject;
    sal_Int32                   nState;

                                explicit EmbedEventListener_Impl( EmbeddedObjectRef* p ) :
                                    pObject(p)
                                    , nState(-1)
                                {}

    static rtl::Reference<EmbedEventListener_Impl> Create( EmbeddedObjectRef* );

    virtual void SAL_CALL changingState( const lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) override;
    virtual void SAL_CALL stateChanged( const lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) override;
    virtual void SAL_CALL queryClosing( const lang::EventObject& Source, sal_Bool GetsOwnership ) override;
    virtual void SAL_CALL notifyClosing( const lang::EventObject& Source ) override;
    virtual void SAL_CALL notifyEvent( const document::EventObject& aEvent ) override;
    virtual void SAL_CALL disposing( const lang::EventObject& aEvent ) override;
    virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) override;
};

rtl::Reference<EmbedEventListener_Impl> EmbedEventListener_Impl::Create( EmbeddedObjectRef* p )
{
    rtl::Reference<EmbedEventListener_Impl> pRet(new EmbedEventListener_Impl( p ));

    if ( p->GetObject().is() )
    {
        p->GetObject()->addStateChangeListener( pRet.get() );

        uno::Reference < util::XCloseable > xClose( p->GetObject(), uno::UNO_QUERY );
        DBG_ASSERT( xClose.is(), "Object does not support XCloseable!" );
        if ( xClose.is() )
            xClose->addCloseListener( pRet.get() );

        uno::Reference < document::XEventBroadcaster > xBrd( p->GetObject(), uno::UNO_QUERY );
        if ( xBrd.is() )
            xBrd->addEventListener( pRet.get() );

        pRet->nState = p->GetObject()->getCurrentState();
        if ( pRet->nState == embed::EmbedStates::RUNNING )
        {
            uno::Reference < util::XModifiable > xMod( p->GetObject()->getComponent(), uno::UNO_QUERY );
            if ( xMod.is() )
                // listen for changes in running state (update replacements in case of changes)
                xMod->addModifyListener( pRet.get() );
        }
    }

    return pRet;
}

void SAL_CALL EmbedEventListener_Impl::changingState( const lang::EventObject&,
                                                    ::sal_Int32,
                                                    ::sal_Int32 )
{
}

void SAL_CALL EmbedEventListener_Impl::stateChanged( const lang::EventObject&,
                                                    ::sal_Int32 nOldState,
                                                    ::sal_Int32 nNewState )
{
    SolarMutexGuard aGuard;
    nState = nNewState;
    if ( !pObject )
        return;

    uno::Reference < util::XModifiable > xMod( pObject->GetObject()->getComponent(), uno::UNO_QUERY );
    if ( nNewState == embed::EmbedStates::RUNNING )
    {
        // TODO/LATER: container must be set before!
        // When is this event created? Who sets the new container when it changed?
        if( ( pObject->GetViewAspect() != embed::Aspects::MSOLE_ICON ) && nOldState != embed::EmbedStates::LOADED && !pObject->IsChart() )
            // get new replacement after deactivation
            pObject->UpdateReplacement();

        if( pObject->IsChart() && nOldState == embed::EmbedStates::UI_ACTIVE )
        {
            //create a new metafile replacement when leaving the edit mode
            //for buggy documents where the old image looks different from the correct one
            if( xMod.is() && !xMod->isModified() )//in case of modification a new replacement will be requested anyhow
                pObject->UpdateReplacementOnDemand();
        }

        if ( xMod.is() && nOldState == embed::EmbedStates::LOADED )
            // listen for changes (update replacements in case of changes)
            xMod->addModifyListener( this );
    }
    else if ( nNewState == embed::EmbedStates::LOADED )
    {
        // in loaded state we can't listen
        if ( xMod.is() )
            xMod->removeModifyListener( this );
    }
}

void SAL_CALL EmbedEventListener_Impl::modified( const lang::EventObject& )
{
    SolarMutexGuard aGuard;
    if ( !(pObject && pObject->GetViewAspect() != embed::Aspects::MSOLE_ICON) )
        return;

    if ( nState == embed::EmbedStates::RUNNING )
    {
        // updates only necessary in non-active states
        if( pObject->IsChart() )
            pObject->UpdateReplacementOnDemand();
        else
            pObject->UpdateReplacement();
    }
    else if ( nState == embed::EmbedStates::ACTIVE ||
              nState == embed::EmbedStates::UI_ACTIVE ||
              nState == embed::EmbedStates::INPLACE_ACTIVE )
    {
        // in case the object is inplace or UI active the replacement image should be updated on demand
        pObject->UpdateReplacementOnDemand();
    }
}

void SAL_CALL EmbedEventListener_Impl::notifyEvent( const document::EventObject& aEvent )
{
    SolarMutexGuard aGuard;

    if ( pObject && aEvent.EventName == "OnVisAreaChanged" && pObject->GetViewAspect() != embed::Aspects::MSOLE_ICON && !pObject->IsChart() )
    {
        pObject->UpdateReplacement();
    }
}

void SAL_CALL EmbedEventListener_Impl::queryClosing( const lang::EventObject& Source, sal_Bool )
{
    // An embedded object can be shared between several objects (f.e. for undo purposes)
    // the object will not be closed before the last "customer" is destroyed
    // Now the EmbeddedObjectRef helper class works like a "lock" on the object
    if ( pObject && pObject->IsLocked() && Source.Source == pObject->GetObject() )
        throw util::CloseVetoException();
}

void SAL_CALL EmbedEventListener_Impl::notifyClosing( const lang::EventObject& Source )
{
    if ( pObject && Source.Source == pObject->GetObject() )
    {
        pObject->Clear();
        pObject = nullptr;
    }
}

void SAL_CALL EmbedEventListener_Impl::disposing( const lang::EventObject& aEvent )
{
    if ( pObject && aEvent.Source == pObject->GetObject() )
    {
        pObject->Clear();
        pObject = nullptr;
    }
}

struct EmbeddedObjectRef_Impl
{
    uno::Reference <embed::XEmbeddedObject> mxObj;

    rtl::Reference<EmbedEventListener_Impl>     mxListener;
    OUString                                    aPersistName;
    OUString                                    aMediaType;
    comphelper::EmbeddedObjectContainer*        pContainer;
    std::unique_ptr<Graphic>                    pGraphic;
    sal_Int64                                   nViewAspect;
    bool                                        bIsLocked:1;
    bool                                        bNeedUpdate:1;
    bool                                        bUpdating:1;

    // #i104867#
    sal_uInt32                                  mnGraphicVersion;
    awt::Size                                   aDefaultSizeForChart_In_100TH_MM;//#i103460# charts do not necessaryly have an own size within ODF files, in this case they need to use the size settings from the surrounding frame, which is made available with this member

    EmbeddedObjectRef_Impl() :
        pContainer(nullptr),
        nViewAspect(embed::Aspects::MSOLE_CONTENT),
        bIsLocked(false),
        bNeedUpdate(false),
        bUpdating(false),
        mnGraphicVersion(0),
        aDefaultSizeForChart_In_100TH_MM(awt::Size(8000,7000))
    {}

    EmbeddedObjectRef_Impl( const EmbeddedObjectRef_Impl& r ) :
        mxObj(r.mxObj),
        aPersistName(r.aPersistName),
        aMediaType(r.aMediaType),
        pContainer(r.pContainer),
        nViewAspect(r.nViewAspect),
        bIsLocked(r.bIsLocked),
        bNeedUpdate(r.bNeedUpdate),
        bUpdating(r.bUpdating),
        mnGraphicVersion(0),
        aDefaultSizeForChart_In_100TH_MM(r.aDefaultSizeForChart_In_100TH_MM)
    {
        if (r.pGraphic && !r.bNeedUpdate)
            pGraphic.reset( new Graphic(*r.pGraphic) );
    }
};

const uno::Reference <embed::XEmbeddedObject>& EmbeddedObjectRef::operator->() const
{
    return mpImpl->mxObj;
}

const uno::Reference <embed::XEmbeddedObject>& EmbeddedObjectRef::GetObject() const
{
    return mpImpl->mxObj;
}

EmbeddedObjectRef::EmbeddedObjectRef() : mpImpl(new EmbeddedObjectRef_Impl) {}

EmbeddedObjectRef::EmbeddedObjectRef( const uno::Reference < embed::XEmbeddedObject >& xObj, sal_Int64 nAspect ) :
    mpImpl(new EmbeddedObjectRef_Impl)
{
    mpImpl->nViewAspect = nAspect;
    mpImpl->mxObj = xObj;
    mpImpl->mxListener = EmbedEventListener_Impl::Create( this );
}

EmbeddedObjectRef::EmbeddedObjectRef( const EmbeddedObjectRef& rObj ) :
    mpImpl(new EmbeddedObjectRef_Impl(*rObj.mpImpl))
{
    mpImpl->mxListener = EmbedEventListener_Impl::Create( this );
}

EmbeddedObjectRef::~EmbeddedObjectRef()
{
    Clear();
}

void EmbeddedObjectRef::Assign( const uno::Reference < embed::XEmbeddedObject >& xObj, sal_Int64 nAspect )
{
    DBG_ASSERT(!mpImpl->mxObj.is(), "Never assign an already assigned object!");

    Clear();
    mpImpl->nViewAspect = nAspect;
    mpImpl->mxObj = xObj;
    mpImpl->mxListener = EmbedEventListener_Impl::Create( this );

    //#i103460#
    if ( IsChart() )
    {
        uno::Reference < chart2::XDefaultSizeTransmitter > xSizeTransmitter( xObj, uno::UNO_QUERY );
        DBG_ASSERT( xSizeTransmitter.is(), "Object does not support XDefaultSizeTransmitter -> will cause #i103460#!" );
        if( xSizeTransmitter.is() )
            xSizeTransmitter->setDefaultSize( mpImpl->aDefaultSizeForChart_In_100TH_MM );
    }
}

void EmbeddedObjectRef::Clear()
{
    if (mpImpl->mxObj.is() && mpImpl->mxListener.is())
    {
        mpImpl->mxObj->removeStateChangeListener(mpImpl->mxListener.get());

        uno::Reference<util::XCloseable> xClose(mpImpl->mxObj, uno::UNO_QUERY);
        if ( xClose.is() )
            xClose->removeCloseListener( mpImpl->mxListener.get() );

        uno::Reference<document::XEventBroadcaster> xBrd(mpImpl->mxObj, uno::UNO_QUERY);
        if ( xBrd.is() )
            xBrd->removeEventListener( mpImpl->mxListener.get() );

        if ( mpImpl->bIsLocked )
        {
            if ( xClose.is() )
            {
                try
                {
                    mpImpl->mxObj->changeState(embed::EmbedStates::LOADED);
                    xClose->close( true );
                }
                catch (const util::CloseVetoException&)
                {
                    // there's still someone who needs the object!
                }
                catch (const uno::Exception& e)
                {
                    SAL_WARN("svtools.misc", "Error on switching of the object to loaded state and closing: \"" << e << "\"");
                }
            }
        }
    }

    if (mpImpl->mxListener.is())
    {
        mpImpl->mxListener->pObject = nullptr;
        mpImpl->mxListener.clear();
    }

    mpImpl->mxObj = nullptr;
    mpImpl->pContainer = nullptr;
    mpImpl->bIsLocked = false;
    mpImpl->bNeedUpdate = false;
}

bool EmbeddedObjectRef::is() const
{
    return mpImpl->mxObj.is();
}

void EmbeddedObjectRef::AssignToContainer( comphelper::EmbeddedObjectContainer* pContainer, const OUString& rPersistName )
{
    mpImpl->pContainer = pContainer;
    mpImpl->aPersistName = rPersistName;

    if ( mpImpl->pGraphic && !mpImpl->bNeedUpdate && pContainer )
        SetGraphicToContainer( *mpImpl->pGraphic, *pContainer, mpImpl->aPersistName, OUString() );
}

comphelper::EmbeddedObjectContainer* EmbeddedObjectRef::GetContainer() const
{
    return mpImpl->pContainer;
}

sal_Int64 EmbeddedObjectRef::GetViewAspect() const
{
    return mpImpl->nViewAspect;
}

void EmbeddedObjectRef::SetViewAspect( sal_Int64 nAspect )
{
    mpImpl->nViewAspect = nAspect;
}

void EmbeddedObjectRef::Lock( bool bLock )
{
    mpImpl->bIsLocked = bLock;
}

bool EmbeddedObjectRef::IsLocked() const
{
    return mpImpl->bIsLocked;
}

void EmbeddedObjectRef::GetReplacement( bool bUpdate )
{
    Graphic aOldGraphic;

    if ( bUpdate )
    {
        if (mpImpl->pGraphic)
            aOldGraphic = *mpImpl->pGraphic;

        mpImpl->pGraphic.reset();
        mpImpl->aMediaType.clear();
        mpImpl->pGraphic.reset( new Graphic );
        mpImpl->mnGraphicVersion++;
    }
    else if ( !mpImpl->pGraphic )
    {
        mpImpl->pGraphic.reset( new Graphic );
        mpImpl->mnGraphicVersion++;
    }
    else
    {
        OSL_FAIL("No update, but replacement exists already!");
        return;
    }

    std::unique_ptr<SvStream> pGraphicStream(GetGraphicStream( bUpdate ));
    if ( pGraphicStream )
    {
        GraphicFilter& rGF = GraphicFilter::GetGraphicFilter();
        if( mpImpl->pGraphic )
            rGF.ImportGraphic( *mpImpl->pGraphic, OUString(), *pGraphicStream );
        mpImpl->mnGraphicVersion++;
    }

    // note that UpdateReplacementOnDemand which resets mpImpl->pGraphic to null may have been called
    // e.g. when exporting ooo58458-1.odt to doc
    if (bUpdate && (!mpImpl->pGraphic || !*mpImpl->pGraphic) && aOldGraphic)
    {
        // We used to have an old graphic, tried to update and the update
        // failed. Go back to the old graphic instead of having no graphic at
        // all.
        mpImpl->pGraphic.reset(new Graphic(aOldGraphic));
        SAL_WARN("svtools.misc", "EmbeddedObjectRef::GetReplacement: update failed");
    }
}

const Graphic* EmbeddedObjectRef::GetGraphic() const
{
    try
    {
        if ( mpImpl->bNeedUpdate )
            // bNeedUpdate will be set to false while retrieving new replacement
            const_cast < EmbeddedObjectRef* >(this)->GetReplacement(true);
        else if ( !mpImpl->pGraphic )
            const_cast < EmbeddedObjectRef* >(this)->GetReplacement(false);
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svtools.misc", "Something went wrong on getting the graphic");
    }

    return mpImpl->pGraphic.get();
}

Size EmbeddedObjectRef::GetSize( MapMode const * pTargetMapMode ) const
{
    MapMode aSourceMapMode( MapUnit::Map100thMM );
    Size aResult;

    if ( mpImpl->nViewAspect == embed::Aspects::MSOLE_ICON )
    {
        const Graphic* pGraphic = GetGraphic();
        if ( pGraphic )
        {
            aSourceMapMode = pGraphic->GetPrefMapMode();
            aResult = pGraphic->GetPrefSize();
        }
        else
            aResult = Size( 2500, 2500 );
    }
    else
    {
        awt::Size aSize;

        if (mpImpl->mxObj.is())
        {
            try
            {
                aSize = mpImpl->mxObj->getVisualAreaSize(mpImpl->nViewAspect);
            }
            catch(const embed::NoVisualAreaSizeException&)
            {
            }
            catch (const uno::Exception& e)
            {
                SAL_WARN("svtools.misc", "Something went wrong on getting of the size of the object: \"" << e << "\"");
            }

            try
            {
                aSourceMapMode = MapMode(VCLUnoHelper::UnoEmbed2VCLMapUnit(mpImpl->mxObj->getMapUnit(mpImpl->nViewAspect)));
            }
            catch (const uno::Exception& e)
            {
                SAL_WARN("svtools.misc", "Can not get the map mode: \"" << e << "\"");
            }
        }

        if ( !aSize.Height && !aSize.Width )
        {
            aSize.Width = 5000;
            aSize.Height = 5000;
        }

        aResult = Size( aSize.Width, aSize.Height );
    }

    if ( pTargetMapMode )
        aResult = OutputDevice::LogicToLogic( aResult, aSourceMapMode, *pTargetMapMode );

    return aResult;
}

void EmbeddedObjectRef::SetGraphicStream( const uno::Reference< io::XInputStream >& xInGrStream,
                                            const OUString& rMediaType )
{
    mpImpl->pGraphic.reset( new Graphic );
    mpImpl->aMediaType = rMediaType;
    mpImpl->mnGraphicVersion++;

    std::unique_ptr<SvStream> pGraphicStream(::utl::UcbStreamHelper::CreateStream( xInGrStream ));

    if ( pGraphicStream )
    {
        GraphicFilter& rGF = GraphicFilter::GetGraphicFilter();
        rGF.ImportGraphic( *mpImpl->pGraphic, "", *pGraphicStream );
        mpImpl->mnGraphicVersion++;

        if ( mpImpl->pContainer )
        {
            pGraphicStream->Seek( 0 );
            uno::Reference< io::XInputStream > xInSeekGrStream = new ::utl::OSeekableInputStreamWrapper( pGraphicStream.get() );

            mpImpl->pContainer->InsertGraphicStream( xInSeekGrStream, mpImpl->aPersistName, rMediaType );
        }
    }

    mpImpl->bNeedUpdate = false;

}

void EmbeddedObjectRef::SetGraphic( const Graphic& rGraphic, const OUString& rMediaType )
{
    mpImpl->pGraphic.reset( new Graphic( rGraphic ) );
    mpImpl->aMediaType = rMediaType;
    mpImpl->mnGraphicVersion++;

    if ( mpImpl->pContainer )
        SetGraphicToContainer( rGraphic, *mpImpl->pContainer, mpImpl->aPersistName, rMediaType );

    mpImpl->bNeedUpdate = false;
}

std::unique_ptr<SvStream> EmbeddedObjectRef::GetGraphicStream( bool bUpdate ) const
{
    DBG_ASSERT( bUpdate || mpImpl->pContainer, "Can't retrieve current graphic!" );
    uno::Reference < io::XInputStream > xStream;
    if ( mpImpl->pContainer && !bUpdate )
    {
        SAL_INFO( "svtools.misc", "getting stream from container" );
        // try to get graphic stream from container storage
        xStream = mpImpl->pContainer->GetGraphicStream(mpImpl->mxObj, &mpImpl->aMediaType);
        if ( xStream.is() )
        {
            const sal_Int32 nConstBufferSize = 32000;
            std::unique_ptr<SvStream> pStream(new SvMemoryStream( 32000, 32000 ));
            try
            {
                sal_Int32 nRead=0;
                uno::Sequence < sal_Int8 > aSequence ( nConstBufferSize );
                do
                {
                    nRead = xStream->readBytes ( aSequence, nConstBufferSize );
                    pStream->WriteBytes(aSequence.getConstArray(), nRead);
                }
                while ( nRead == nConstBufferSize );
                pStream->Seek(0);
                return pStream;
            }
            catch (const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("svtools.misc", "discarding broken embedded object preview");
                xStream.clear();
            }
        }
    }

    if ( !xStream.is() )
    {
        SAL_INFO( "svtools.misc", "getting stream from object" );
        bool bUpdateAllowed(true);
        const comphelper::EmbeddedObjectContainer* pContainer = GetContainer();

        if(pContainer)
        {
            uno::Reference<embed::XLinkageSupport> const xLinkage(
                    mpImpl->mxObj, uno::UNO_QUERY);
            if (xLinkage.is() && xLinkage->isLink())
            {
                bUpdateAllowed = pContainer->getUserAllowsLinkUpdate();

            }
        }

        if (bUpdateAllowed)
        {
            // update wanted or no stream in container storage available
            xStream = GetGraphicReplacementStream(mpImpl->nViewAspect, mpImpl->mxObj, &mpImpl->aMediaType);

            if(xStream.is())
            {
                if (mpImpl->pContainer)
                    mpImpl->pContainer->InsertGraphicStream(xStream,mpImpl->aPersistName,mpImpl->aMediaType);

                std::unique_ptr<SvStream> pResult = ::utl::UcbStreamHelper::CreateStream( xStream );
                if (pResult && bUpdate)
                    mpImpl->bNeedUpdate = false;

                return pResult;
            }
        }
    }

    return nullptr;
}

void EmbeddedObjectRef::DrawPaintReplacement( const tools::Rectangle &rRect, const OUString &rText, OutputDevice *pOut )
{
    MapMode aMM( MapUnit::MapAppFont );
    Size aAppFontSz = pOut->LogicToLogic( Size( 0, 8 ), &aMM, nullptr );
    vcl::Font aFnt( "Helvetica", aAppFontSz );
    aFnt.SetTransparent( true );
    aFnt.SetColor( COL_LIGHTRED );
    aFnt.SetWeight( WEIGHT_BOLD );
    aFnt.SetFamily( FAMILY_SWISS );

    pOut->Push();
    pOut->SetBackground();
    pOut->SetFont( aFnt );

    Point aPt;

    // Now scale text such that it fits in the rectangle
    // We start with the default size and decrease 1-AppFont
    for( sal_uInt16 i = 8; i > 2; i-- )
    {
        aPt.setX( (rRect.GetWidth()  - pOut->GetTextWidth( rText )) / 2 );
        aPt.setY( (rRect.GetHeight() - pOut->GetTextHeight()) / 2 );

        bool bTiny = false;
        if( aPt.X() < 0 )
        {
            bTiny = true;
            aPt.setX( 0 );
        }
        if( aPt.Y() < 0 )
        {
            bTiny = true;
            aPt.setY( 0 );
        }
        if( bTiny )
        {
            // decrease for small images
            aFnt.SetFontSize( Size( 0, aAppFontSz.Height() * i / 8 ) );
            pOut->SetFont( aFnt );
        }
        else
            break;
    }

    BitmapEx aBmp(BMP_PLUGIN);
    long nHeight = rRect.GetHeight() - pOut->GetTextHeight();
    long nWidth = rRect.GetWidth();
    if(nHeight > 0 && nWidth > 0 && aBmp.GetSizePixel().Width() > 0)
    {
        aPt.setY( nHeight );
        Point   aP = rRect.TopLeft();
        Size    aBmpSize = aBmp.GetSizePixel();
        // fit bitmap in
        if( nHeight * 10 / nWidth
          > aBmpSize.Height() * 10 / aBmpSize.Width() )
        {
            // adjust to the width
            // keep proportions
            long nH = nWidth * aBmpSize.Height() / aBmpSize.Width();
            // center
            aP.AdjustY((nHeight - nH) / 2 );
            nHeight = nH;
        }
        else
        {
            // adjust to the height
            // keep proportions
            long nW = nHeight * aBmpSize.Width() / aBmpSize.Height();
            // center
            aP.AdjustX((nWidth - nW) / 2 );
            nWidth = nW;
        }

        pOut->DrawBitmap(aP, Size( nWidth, nHeight ), aBmp.GetBitmap());
    }

    pOut->IntersectClipRegion( rRect );
    aPt += rRect.TopLeft();
    pOut->DrawText( aPt, rText );
    pOut->Pop();
}

void EmbeddedObjectRef::DrawShading( const tools::Rectangle &rRect, OutputDevice *pOut )
{
    GDIMetaFile * pMtf = pOut->GetConnectMetaFile();
    if( pMtf && pMtf->IsRecord() )
        return;

    pOut->Push();
    pOut->SetLineColor( COL_BLACK );

    Size aPixSize = pOut->LogicToPixel( rRect.GetSize() );
    aPixSize.AdjustWidth( -1 );
    aPixSize.AdjustHeight( -1 );
    Point aPixViewPos = pOut->LogicToPixel( rRect.TopLeft() );
    sal_Int32 nMax = aPixSize.Width() + aPixSize.Height();
    for( sal_Int32 i = 5; i < nMax; i += 5 )
    {
        Point a1( aPixViewPos ), a2( aPixViewPos );
        if( i > aPixSize.Width() )
            a1 += Point( aPixSize.Width(), i - aPixSize.Width() );
        else
            a1 += Point( i, 0 );
        if( i > aPixSize.Height() )
            a2 += Point( i - aPixSize.Height(), aPixSize.Height() );
        else
            a2 += Point( 0, i );

        pOut->DrawLine( pOut->PixelToLogic( a1 ), pOut->PixelToLogic( a2 ) );
    }

    pOut->Pop();

}

bool EmbeddedObjectRef::TryRunningState( const uno::Reference < embed::XEmbeddedObject >& xEmbObj )
{
    if ( !xEmbObj.is() )
        return false;

    try
    {
        if ( xEmbObj->getCurrentState() == embed::EmbedStates::LOADED )
            xEmbObj->changeState( embed::EmbedStates::RUNNING );
    }
    catch (const uno::Exception&)
    {
        return false;
    }

    return true;
}

void EmbeddedObjectRef::SetGraphicToContainer( const Graphic& rGraphic,
                                                comphelper::EmbeddedObjectContainer& aContainer,
                                                const OUString& aName,
                                                const OUString& aMediaType )
{
    SvMemoryStream aStream;
    aStream.SetVersion( SOFFICE_FILEFORMAT_CURRENT );
    if ( rGraphic.ExportNative( aStream ) )
    {
        aStream.Seek( 0 );

        uno::Reference < io::XInputStream > xStream = new ::utl::OSeekableInputStreamWrapper( aStream );
        aContainer.InsertGraphicStream( xStream, aName, aMediaType );
    }
    else
        OSL_FAIL( "Export of graphic is failed!" );
}

uno::Reference< io::XInputStream > EmbeddedObjectRef::GetGraphicReplacementStream(
                                                                sal_Int64 nViewAspect,
                                                                const uno::Reference< embed::XEmbeddedObject >& xObj,
                                                                OUString* pMediaType )
    throw()
{
    return ::comphelper::EmbeddedObjectContainer::GetGraphicReplacementStream(nViewAspect,xObj,pMediaType);
}

bool EmbeddedObjectRef::IsChart(const css::uno::Reference < css::embed::XEmbeddedObject >& xObj)
{
    SvGlobalName aObjClsId(xObj->getClassID());
    return SvGlobalName(SO3_SCH_CLASSID_30) == aObjClsId
        || SvGlobalName(SO3_SCH_CLASSID_40) == aObjClsId
        || SvGlobalName(SO3_SCH_CLASSID_50) == aObjClsId
        || SvGlobalName(SO3_SCH_CLASSID_60) == aObjClsId;
}

void EmbeddedObjectRef::UpdateReplacement()
{
    if (mpImpl->bUpdating)
    {
        SAL_WARN("svtools.misc", "UpdateReplacement called while UpdateReplacement already underway");
        return;
    }
    mpImpl->bUpdating = true;
    GetReplacement(true);
    mpImpl->bUpdating = false;
}

void EmbeddedObjectRef::UpdateReplacementOnDemand()
{
    mpImpl->pGraphic.reset();
    mpImpl->bNeedUpdate = true;
    mpImpl->mnGraphicVersion++;

    if( mpImpl->pContainer )
    {
        //remove graphic from container thus a new up to date one is requested on save
        mpImpl->pContainer->RemoveGraphicStream( mpImpl->aPersistName );
    }
}

bool EmbeddedObjectRef::IsChart() const
{
    //todo maybe for 3.0:
    //if the changes work good for chart
    //we should apply them for all own ole objects

    //#i83708# #i81857# #i79578# request an ole replacement image only if really necessary
    //as this call can be very expensive and does block the user interface as long at it takes

    if (!mpImpl->mxObj.is())
        return false;

    return EmbeddedObjectRef::IsChart(mpImpl->mxObj);
}

// MT: Only used for getting accessible attributes, which are not localized
OUString EmbeddedObjectRef::GetChartType()
{
    OUString Style;
    if ( mpImpl->mxObj.is() )
    {
        if ( IsChart() )
        {
            if ( svt::EmbeddedObjectRef::TryRunningState( mpImpl->mxObj ) )
            {
                uno::Reference< chart2::XChartDocument > xChart( mpImpl->mxObj->getComponent(), uno::UNO_QUERY );
                if (xChart.is())
                {
                    uno::Reference< chart2::XDiagram > xDiagram( xChart->getFirstDiagram());
                    if( ! xDiagram.is())
                        return OUString();
                    uno::Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xDiagram, uno::UNO_QUERY_THROW );
                    uno::Sequence< uno::Reference< chart2::XCoordinateSystem > > aCooSysSeq( xCooSysCnt->getCoordinateSystems());
                    // IA2 CWS. Unused: int nCoordinateCount = aCooSysSeq.getLength();
                    bool bGetChartType = false;
                    for( sal_Int32 nCooSysIdx=0; nCooSysIdx<aCooSysSeq.getLength(); ++nCooSysIdx )
                    {
                        uno::Reference< chart2::XChartTypeContainer > xCTCnt( aCooSysSeq[nCooSysIdx], uno::UNO_QUERY_THROW );
                        uno::Sequence< uno::Reference< chart2::XChartType > > aChartTypes( xCTCnt->getChartTypes());
                        int nDimesionCount = aCooSysSeq[nCooSysIdx]->getDimension();
                        if( nDimesionCount == 3 )
                            Style += "3D ";
                        else
                            Style += "2D ";
                        for( sal_Int32 nCTIdx=0; nCTIdx<aChartTypes.getLength(); ++nCTIdx )
                        {
                            OUString strChartType = aChartTypes[nCTIdx]->getChartType();
                            if (strChartType == "com.sun.star.chart2.AreaChartType")
                            {
                                Style += "Areas";
                                bGetChartType = true;
                            }
                            else if (strChartType == "com.sun.star.chart2.BarChartType")
                            {
                                Style += "Bars";
                                bGetChartType = true;
                            }
                            else if (strChartType == "com.sun.star.chart2.ColumnChartType")
                            {
                                uno::Reference< beans::XPropertySet > xProp( aCooSysSeq[nCooSysIdx], uno::UNO_QUERY );
                                if( xProp.is())
                                {
                                    bool bCurrent = false;
                                    if( xProp->getPropertyValue( "SwapXAndYAxis" ) >>= bCurrent )
                                    {
                                        if (bCurrent)
                                            Style += "Bars";
                                        else
                                            Style += "Columns";
                                        bGetChartType = true;
                                    }
                                }
                            }
                            else if (strChartType == "com.sun.star.chart2.LineChartType")
                            {
                                Style += "Lines";
                                bGetChartType = true;
                            }
                            else if (strChartType == "com.sun.star.chart2.ScatterChartType")
                            {
                                Style += "XY Chart";
                                bGetChartType = true;
                            }
                            else if (strChartType == "com.sun.star.chart2.PieChartType")
                            {
                                Style += "Pies";
                                bGetChartType = true;
                            }
                            else if (strChartType == "com.sun.star.chart2.NetChartType")
                            {
                                Style += "Radar";
                                bGetChartType = true;
                            }
                            else if (strChartType == "com.sun.star.chart2.CandleStickChartType")
                            {
                                Style += "Candle Stick Chart";
                                bGetChartType = true;
                            }
                            if (bGetChartType)
                                return Style;
                        }
                    }
                }
            }
        }
    }
    return Style;
}

// #i104867#
sal_uInt32 EmbeddedObjectRef::getGraphicVersion() const
{
    return mpImpl->mnGraphicVersion;
}

void EmbeddedObjectRef::SetDefaultSizeForChart( const Size& rSizeIn_100TH_MM )
{
    //#i103460# charts do not necessaryly have an own size within ODF files,
    //for this case they need to use the size settings from the surrounding frame,
    //which is made available with this method

    mpImpl->aDefaultSizeForChart_In_100TH_MM = awt::Size( rSizeIn_100TH_MM.getWidth(), rSizeIn_100TH_MM.getHeight() );

    uno::Reference<chart2::XDefaultSizeTransmitter> xSizeTransmitter(mpImpl->mxObj, uno::UNO_QUERY);
    DBG_ASSERT( xSizeTransmitter.is(), "Object does not support XDefaultSizeTransmitter -> will cause #i103460#!" );
    if( xSizeTransmitter.is() )
        xSizeTransmitter->setDefaultSize( mpImpl->aDefaultSizeForChart_In_100TH_MM );
}

} // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
