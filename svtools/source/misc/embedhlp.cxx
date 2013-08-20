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
#include <svtools/svtools.hrc>
#include <svtools/svtresid.hxx>

#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/seqstream.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>

#include <tools/globname.hxx>
#include <comphelper/classids.hxx>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XStateChangeListener.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/chart2/XDefaultSizeTransmitter.hpp>
#include <cppuhelper/implbase4.hxx>
#include "vcl/svapp.hxx"
#include <osl/mutex.hxx>

using namespace com::sun::star;

namespace svt {

class EmbedEventListener_Impl : public ::cppu::WeakImplHelper4 < embed::XStateChangeListener,
                                                                 document::XEventListener,
                                                                 util::XModifyListener,
                                                                 util::XCloseListener >
{
public:
    EmbeddedObjectRef*          pObject;
    sal_Int32                   nState;

                                EmbedEventListener_Impl( EmbeddedObjectRef* p ) :
                                    pObject(p)
                                    , nState(-1)
                                {}

    static EmbedEventListener_Impl* Create( EmbeddedObjectRef* );

    virtual void SAL_CALL changingState( const lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState )
                                    throw (embed::WrongStateException, uno::RuntimeException);
    virtual void SAL_CALL stateChanged( const lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState )
                                    throw (uno::RuntimeException);
    virtual void SAL_CALL queryClosing( const lang::EventObject& Source, ::sal_Bool GetsOwnership )
                                    throw (util::CloseVetoException, uno::RuntimeException);
    virtual void SAL_CALL notifyClosing( const lang::EventObject& Source ) throw (uno::RuntimeException);
    virtual void SAL_CALL notifyEvent( const document::EventObject& aEvent ) throw( uno::RuntimeException );
    virtual void SAL_CALL disposing( const lang::EventObject& aEvent ) throw( uno::RuntimeException );
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
};

EmbedEventListener_Impl* EmbedEventListener_Impl::Create( EmbeddedObjectRef* p )
{
    EmbedEventListener_Impl* xRet = new EmbedEventListener_Impl( p );
    xRet->acquire();

    if ( p->GetObject().is() )
    {
        p->GetObject()->addStateChangeListener( xRet );

        uno::Reference < util::XCloseable > xClose( p->GetObject(), uno::UNO_QUERY );
        DBG_ASSERT( xClose.is(), "Object does not support XCloseable!" );
        if ( xClose.is() )
            xClose->addCloseListener( xRet );

        uno::Reference < document::XEventBroadcaster > xBrd( p->GetObject(), uno::UNO_QUERY );
        if ( xBrd.is() )
            xBrd->addEventListener( xRet );

        xRet->nState = p->GetObject()->getCurrentState();
        if ( xRet->nState == embed::EmbedStates::RUNNING )
        {
            uno::Reference < util::XModifiable > xMod( p->GetObject()->getComponent(), uno::UNO_QUERY );
            if ( xMod.is() )
                // listen for changes in running state (update replacements in case of changes)
                xMod->addModifyListener( xRet );
        }
    }

    return xRet;
}

void SAL_CALL EmbedEventListener_Impl::changingState( const lang::EventObject&,
                                                    ::sal_Int32,
                                                    ::sal_Int32 )
    throw ( embed::WrongStateException,
            uno::RuntimeException )
{
}

void SAL_CALL EmbedEventListener_Impl::stateChanged( const lang::EventObject&,
                                                    ::sal_Int32 nOldState,
                                                    ::sal_Int32 nNewState )
    throw ( uno::RuntimeException )
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

void SAL_CALL EmbedEventListener_Impl::modified( const lang::EventObject& ) throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( pObject && pObject->GetViewAspect() != embed::Aspects::MSOLE_ICON )
    {
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
}

void SAL_CALL EmbedEventListener_Impl::notifyEvent( const document::EventObject& aEvent ) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    if ( pObject && aEvent.EventName == "OnVisAreaChanged" && pObject->GetViewAspect() != embed::Aspects::MSOLE_ICON && !pObject->IsChart() )
    {
        pObject->UpdateReplacement();
    }
}

void SAL_CALL EmbedEventListener_Impl::queryClosing( const lang::EventObject& Source, ::sal_Bool )
        throw ( util::CloseVetoException, uno::RuntimeException)
{
    // An embedded object can be shared between several objects (f.e. for undo purposes)
    // the object will not be closed before the last "customer" is destroyed
    // Now the EmbeddedObjectRef helper class works like a "lock" on the object
    if ( pObject && pObject->IsLocked() && Source.Source == pObject->GetObject() )
        throw util::CloseVetoException();
}

void SAL_CALL EmbedEventListener_Impl::notifyClosing( const lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( pObject && Source.Source == pObject->GetObject() )
    {
        pObject->Clear();
        pObject = 0;
    }
}

void SAL_CALL EmbedEventListener_Impl::disposing( const lang::EventObject& aEvent ) throw( uno::RuntimeException )
{
    if ( pObject && aEvent.Source == pObject->GetObject() )
    {
        pObject->Clear();
        pObject = 0;
    }
}

struct EmbeddedObjectRef_Impl
{
    uno::Reference <embed::XEmbeddedObject> mxObj;

    EmbedEventListener_Impl*                    xListener;
    OUString                             aPersistName;
    OUString                             aMediaType;
    comphelper::EmbeddedObjectContainer*        pContainer;
    Graphic*                                    pGraphic;
    sal_Int64                                   nViewAspect;
    bool                                        bIsLocked:1;
    bool                                        bNeedUpdate:1;

    // #i104867#
    sal_uInt32                                  mnGraphicVersion;
    awt::Size                                   aDefaultSizeForChart_In_100TH_MM;//#i103460# charts do not necessaryly have an own size within ODF files, in this case they need to use the size settings from the surrounding frame, which is made available with this member

    EmbeddedObjectRef_Impl() :
        xListener(0),
        pContainer(NULL),
        pGraphic(NULL),
        nViewAspect(embed::Aspects::MSOLE_CONTENT),
        bIsLocked(false),
        bNeedUpdate(false),
        mnGraphicVersion(0),
        aDefaultSizeForChart_In_100TH_MM(awt::Size(8000,7000))
    {}

    EmbeddedObjectRef_Impl( const EmbeddedObjectRef_Impl& r ) :
        mxObj(r.mxObj),
        xListener(0),
        aPersistName(r.aPersistName),
        aMediaType(r.aMediaType),
        pContainer(r.pContainer),
        pGraphic(NULL),
        nViewAspect(r.nViewAspect),
        bIsLocked(r.bIsLocked),
        bNeedUpdate(r.bNeedUpdate),
        mnGraphicVersion(0),
        aDefaultSizeForChart_In_100TH_MM(r.aDefaultSizeForChart_In_100TH_MM)
    {
        if (r.pGraphic && !r.bNeedUpdate)
            pGraphic = new Graphic(*r.pGraphic);
    }

    ~EmbeddedObjectRef_Impl()
    {
        delete pGraphic;
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
    mpImpl->xListener = EmbedEventListener_Impl::Create( this );
}

EmbeddedObjectRef::EmbeddedObjectRef( const EmbeddedObjectRef& rObj ) :
    mpImpl(new EmbeddedObjectRef_Impl(*rObj.mpImpl))
{
    mpImpl->xListener = EmbedEventListener_Impl::Create( this );
}

EmbeddedObjectRef::~EmbeddedObjectRef()
{
    Clear();
    delete mpImpl;
}

void EmbeddedObjectRef::Assign( const uno::Reference < embed::XEmbeddedObject >& xObj, sal_Int64 nAspect )
{
    DBG_ASSERT(!mpImpl->mxObj.is(), "Never assign an already assigned object!");

    Clear();
    mpImpl->nViewAspect = nAspect;
    mpImpl->mxObj = xObj;
    mpImpl->xListener = EmbedEventListener_Impl::Create( this );

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
    if (mpImpl->mxObj.is() && mpImpl->xListener)
    {
        mpImpl->mxObj->removeStateChangeListener(mpImpl->xListener);

        uno::Reference<util::XCloseable> xClose(mpImpl->mxObj, uno::UNO_QUERY);
        if ( xClose.is() )
            xClose->removeCloseListener( mpImpl->xListener );

        uno::Reference<document::XEventBroadcaster> xBrd(mpImpl->mxObj, uno::UNO_QUERY);
        if ( xBrd.is() )
            xBrd->removeEventListener( mpImpl->xListener );

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
                catch (const uno::Exception&)
                {
                    OSL_FAIL( "Error on switching of the object to loaded state and closing!\n" );
                }
            }
        }

        if ( mpImpl->xListener )
        {
            mpImpl->xListener->pObject = 0;
            mpImpl->xListener->release();
            mpImpl->xListener = 0;
        }

        mpImpl->mxObj = NULL;
        mpImpl->bNeedUpdate = false;
    }

    mpImpl->pContainer = 0;
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
    if ( bUpdate )
    {
        DELETEZ( mpImpl->pGraphic );
        mpImpl->aMediaType = OUString();
        mpImpl->pGraphic = new Graphic;
        mpImpl->mnGraphicVersion++;
    }
    else if ( !mpImpl->pGraphic )
    {
        mpImpl->pGraphic = new Graphic;
        mpImpl->mnGraphicVersion++;
    }
    else
    {
        OSL_FAIL("No update, but replacement exists already!");
        return;
    }

    SvStream* pGraphicStream = GetGraphicStream( bUpdate );
    if ( pGraphicStream )
    {
        GraphicFilter& rGF = GraphicFilter::GetGraphicFilter();
        if( mpImpl->pGraphic )
            rGF.ImportGraphic( *mpImpl->pGraphic, OUString(), *pGraphicStream, GRFILTER_FORMAT_DONTKNOW );
        mpImpl->mnGraphicVersion++;
        delete pGraphicStream;
    }
}

const Graphic* EmbeddedObjectRef::GetGraphic( OUString* pMediaType ) const
{
    try
    {
        if ( mpImpl->bNeedUpdate )
            // bNeedUpdate will be set to false while retrieving new replacement
            const_cast < EmbeddedObjectRef* >(this)->GetReplacement(true);
        else if ( !mpImpl->pGraphic )
            const_cast < EmbeddedObjectRef* >(this)->GetReplacement(false);
    }
    catch( const uno::Exception& ex )
    {
        SAL_WARN("svtools.misc", "Something went wrong on getting the graphic: " << ex.Message);
    }

    if ( mpImpl->pGraphic && pMediaType )
        *pMediaType = mpImpl->aMediaType;
    return mpImpl->pGraphic;
}

Size EmbeddedObjectRef::GetSize( MapMode* pTargetMapMode ) const
{
    MapMode aSourceMapMode( MAP_100TH_MM );
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
            catch(const uno::Exception&)
            {
                OSL_FAIL( "Something went wrong on getting of the size of the object!" );
            }

            try
            {
                aSourceMapMode = VCLUnoHelper::UnoEmbed2VCLMapUnit(mpImpl->mxObj->getMapUnit(mpImpl->nViewAspect));
            }
            catch(const uno::Exception&)
            {
                OSL_FAIL( "Can not get the map mode!" );
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
    if ( mpImpl->pGraphic )
        delete mpImpl->pGraphic;
    mpImpl->pGraphic = new Graphic();
    mpImpl->aMediaType = rMediaType;
    mpImpl->mnGraphicVersion++;

    SvStream* pGraphicStream = ::utl::UcbStreamHelper::CreateStream( xInGrStream );

    if ( pGraphicStream )
    {
        GraphicFilter& rGF = GraphicFilter::GetGraphicFilter();
        rGF.ImportGraphic( *mpImpl->pGraphic, "", *pGraphicStream, GRFILTER_FORMAT_DONTKNOW );
        mpImpl->mnGraphicVersion++;

        if ( mpImpl->pContainer )
        {
            pGraphicStream->Seek( 0 );
            uno::Reference< io::XInputStream > xInSeekGrStream = new ::utl::OSeekableInputStreamWrapper( pGraphicStream );

            mpImpl->pContainer->InsertGraphicStream( xInSeekGrStream, mpImpl->aPersistName, rMediaType );
        }

        delete pGraphicStream;
    }

    mpImpl->bNeedUpdate = false;

}

void EmbeddedObjectRef::SetGraphic( const Graphic& rGraphic, const OUString& rMediaType )
{
    if ( mpImpl->pGraphic )
        delete mpImpl->pGraphic;
    mpImpl->pGraphic = new Graphic( rGraphic );
    mpImpl->aMediaType = rMediaType;
    mpImpl->mnGraphicVersion++;

    if ( mpImpl->pContainer )
        SetGraphicToContainer( rGraphic, *mpImpl->pContainer, mpImpl->aPersistName, rMediaType );

    mpImpl->bNeedUpdate = false;
}

SvStream* EmbeddedObjectRef::GetGraphicStream( bool bUpdate ) const
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
            SvStream *pStream = new SvMemoryStream( 32000, 32000 );
            try
            {
                sal_Int32 nRead=0;
                uno::Sequence < sal_Int8 > aSequence ( nConstBufferSize );
                do
                {
                    nRead = xStream->readBytes ( aSequence, nConstBufferSize );
                    pStream->Write( aSequence.getConstArray(), nRead );
                }
                while ( nRead == nConstBufferSize );
                pStream->Seek(0);
                return pStream;
            }
            catch (const uno::Exception& ex)
            {
                SAL_WARN("svtools.misc", "discarding broken embedded object preview: " << ex.Message);
                delete pStream;
                xStream.clear();
            }
        }
    }

    if ( !xStream.is() )
    {
        SAL_INFO( "svtools.misc", "getting stream from object" );
        // update wanted or no stream in container storage available
        xStream = GetGraphicReplacementStream(mpImpl->nViewAspect, mpImpl->mxObj, &mpImpl->aMediaType);

        if ( xStream.is() )
        {
            if ( mpImpl->pContainer )
                mpImpl->pContainer->InsertGraphicStream( xStream, mpImpl->aPersistName, mpImpl->aMediaType );

            SvStream* pResult = ::utl::UcbStreamHelper::CreateStream( xStream );
            if ( pResult && bUpdate )
                mpImpl->bNeedUpdate = false;

            return pResult;
        }
    }

    return NULL;
}

void EmbeddedObjectRef::DrawPaintReplacement( const Rectangle &rRect, const OUString &rText, OutputDevice *pOut )
{
    MapMode aMM( MAP_APPFONT );
    Size aAppFontSz = pOut->LogicToLogic( Size( 0, 8 ), &aMM, NULL );
    Font aFnt( OUString("Helvetica"), aAppFontSz );
    aFnt.SetTransparent( true );
    aFnt.SetColor( Color( COL_LIGHTRED ) );
    aFnt.SetWeight( WEIGHT_BOLD );
    aFnt.SetFamily( FAMILY_SWISS );

    pOut->Push();
    pOut->SetBackground();
    pOut->SetFont( aFnt );

    Point aPt;
    // Nun den Text so skalieren, dass er in das Rect passt.
    // Wir fangen mit der Defaultsize an und gehen 1-AppFont runter
    for( sal_uInt16 i = 8; i > 2; i-- )
    {
        aPt.X() = (rRect.GetWidth()  - pOut->GetTextWidth( rText )) / 2;
        aPt.Y() = (rRect.GetHeight() - pOut->GetTextHeight()) / 2;

        bool bTiny = false;
        if( aPt.X() < 0 ) bTiny = true, aPt.X() = 0;
        if( aPt.Y() < 0 ) bTiny = true, aPt.Y() = 0;
        if( bTiny )
        {
            // heruntergehen bei kleinen Bildern
            aFnt.SetSize( Size( 0, aAppFontSz.Height() * i / 8 ) );
            pOut->SetFont( aFnt );
        }
        else
            break;
    }

    Bitmap aBmp( SvtResId( BMP_PLUGIN ) );
    long nHeight = rRect.GetHeight() - pOut->GetTextHeight();
    long nWidth = rRect.GetWidth();
    if( nHeight > 0 && aBmp.GetSizePixel().Width() > 0 )
    {
        aPt.Y() = nHeight;
        Point   aP = rRect.TopLeft();
        Size    aBmpSize = aBmp.GetSizePixel();
        // Bitmap einpassen
        if( nHeight * 10 / nWidth
          > aBmpSize.Height() * 10 / aBmpSize.Width() )
        {
            // nach der Breite ausrichten
            // Proportion beibehalten
            long nH = nWidth * aBmpSize.Height() / aBmpSize.Width();
            // zentrieren
            aP.Y() += (nHeight - nH) / 2;
            nHeight = nH;
        }
        else
        {
            // nach der H"ohe ausrichten
            // Proportion beibehalten
            long nW = nHeight * aBmpSize.Width() / aBmpSize.Height();
            // zentrieren
            aP.X() += (nWidth - nW) / 2;
            nWidth = nW;
        }

        pOut->DrawBitmap( aP, Size( nWidth, nHeight ), aBmp );
    }

    pOut->IntersectClipRegion( rRect );
    aPt += rRect.TopLeft();
    pOut->DrawText( aPt, rText );
    pOut->Pop();
}

void EmbeddedObjectRef::DrawShading( const Rectangle &rRect, OutputDevice *pOut )
{
    GDIMetaFile * pMtf = pOut->GetConnectMetaFile();
    if( pMtf && pMtf->IsRecord() )
        return;

    pOut->Push();
    pOut->SetLineColor( Color( COL_BLACK ) );

    Size aPixSize = pOut->LogicToPixel( rRect.GetSize() );
    aPixSize.Width() -= 1;
    aPixSize.Height() -= 1;
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
        OSL_FAIL( "Export of graphic is failed!\n" );
}

uno::Reference< io::XInputStream > EmbeddedObjectRef::GetGraphicReplacementStream(
                                                                sal_Int64 nViewAspect,
                                                                const uno::Reference< embed::XEmbeddedObject >& xObj,
                                                                OUString* pMediaType )
    throw()
{
    return ::comphelper::EmbeddedObjectContainer::GetGraphicReplacementStream(nViewAspect,xObj,pMediaType);
}

void EmbeddedObjectRef::UpdateReplacement()
{
    GetReplacement( true );
}

void EmbeddedObjectRef::UpdateReplacementOnDemand()
{
    DELETEZ( mpImpl->pGraphic );
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

    SvGlobalName aObjClsId(mpImpl->mxObj->getClassID());
    if(
        SvGlobalName(SO3_SCH_CLASSID_30) == aObjClsId
        || SvGlobalName(SO3_SCH_CLASSID_40) == aObjClsId
        || SvGlobalName(SO3_SCH_CLASSID_50) == aObjClsId
        || SvGlobalName(SO3_SCH_CLASSID_60) == aObjClsId)
    {
        return true;
    }

    return false;
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
