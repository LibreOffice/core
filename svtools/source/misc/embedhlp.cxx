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
#include "precompiled_svtools.hxx"

#include <svtools/embedhlp.hxx>
#include <svtools/filter.hxx>
#include <svtools/svtools.hrc>
#include <svtools/svtdata.hxx>

#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/seqstream.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>

#include <tools/globname.hxx>
#include <sot/clsids.hxx>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/XStateChangeListener.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/chart2/XDefaultSizeTransmitter.hpp>
#include <cppuhelper/implbase4.hxx>
#include "vcl/svapp.hxx"
#include <rtl/logfile.hxx>
#include <osl/mutex.hxx>

using namespace com::sun::star;

namespace svt
{

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

    if ( pObject && aEvent.EventName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("OnVisAreaChanged")) && pObject->GetViewAspect() != embed::Aspects::MSOLE_ICON && !pObject->IsChart() )
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
    EmbedEventListener_Impl*                    xListener;
    ::rtl::OUString                             aPersistName;
    ::rtl::OUString                             aMediaType;
    comphelper::EmbeddedObjectContainer*        pContainer;
    Graphic*                                    pGraphic;
    sal_Int64                                   nViewAspect;
    sal_Bool                                        bIsLocked;
    sal_Bool                                    bNeedUpdate;

    // #i104867#
    sal_uInt32                                  mnGraphicVersion;
    awt::Size                                   aDefaultSizeForChart_In_100TH_MM;//#i103460# charts do not necessaryly have an own size within ODF files, in this case they need to use the size settings from the surrounding frame, which is made available with this member
};

void EmbeddedObjectRef::Construct_Impl()
{
    mpImp = new EmbeddedObjectRef_Impl;
    mpImp->pContainer = 0;
    mpImp->pGraphic = 0;
    mpImp->nViewAspect = embed::Aspects::MSOLE_CONTENT;
    mpImp->bIsLocked = sal_False;
    mpImp->bNeedUpdate = sal_False;
    mpImp->mnGraphicVersion = 0;
    mpImp->aDefaultSizeForChart_In_100TH_MM = awt::Size(8000,7000);
}

EmbeddedObjectRef::EmbeddedObjectRef()
{
    Construct_Impl();
}

EmbeddedObjectRef::EmbeddedObjectRef( const NS_UNO::Reference < NS_EMBED::XEmbeddedObject >& xObj, sal_Int64 nAspect )
{
    Construct_Impl();
    mpImp->nViewAspect = nAspect;
    mxObj = xObj;
    mpImp->xListener = EmbedEventListener_Impl::Create( this );
}

EmbeddedObjectRef::EmbeddedObjectRef( const EmbeddedObjectRef& rObj )
{
    mpImp = new EmbeddedObjectRef_Impl;
    mpImp->pContainer = rObj.mpImp->pContainer;
    mpImp->nViewAspect = rObj.mpImp->nViewAspect;
    mpImp->bIsLocked = rObj.mpImp->bIsLocked;
    mxObj = rObj.mxObj;
    mpImp->xListener = EmbedEventListener_Impl::Create( this );
    mpImp->aPersistName = rObj.mpImp->aPersistName;
    mpImp->aMediaType = rObj.mpImp->aMediaType;
    mpImp->bNeedUpdate = rObj.mpImp->bNeedUpdate;
    mpImp->aDefaultSizeForChart_In_100TH_MM = rObj.mpImp->aDefaultSizeForChart_In_100TH_MM;

    if ( rObj.mpImp->pGraphic && !rObj.mpImp->bNeedUpdate )
        mpImp->pGraphic = new Graphic( *rObj.mpImp->pGraphic );
    else
        mpImp->pGraphic = 0;

    mpImp->mnGraphicVersion = 0;
}

EmbeddedObjectRef::~EmbeddedObjectRef()
{
    delete mpImp->pGraphic;
    Clear();
}

void EmbeddedObjectRef::Assign( const NS_UNO::Reference < NS_EMBED::XEmbeddedObject >& xObj, sal_Int64 nAspect )
{
    DBG_ASSERT( !mxObj.is(), "Never assign an already assigned object!" );

    Clear();
    mpImp->nViewAspect = nAspect;
    mxObj = xObj;
    mpImp->xListener = EmbedEventListener_Impl::Create( this );

    //#i103460#
    if ( IsChart() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::chart2::XDefaultSizeTransmitter > xSizeTransmitter( xObj, uno::UNO_QUERY );
        DBG_ASSERT( xSizeTransmitter.is(), "Object does not support XDefaultSizeTransmitter -> will cause #i103460#!" );
        if( xSizeTransmitter.is() )
            xSizeTransmitter->setDefaultSize( mpImp->aDefaultSizeForChart_In_100TH_MM );
    }
}

void EmbeddedObjectRef::Clear()
{
    if ( mxObj.is() && mpImp->xListener )
    {
        mxObj->removeStateChangeListener( mpImp->xListener );

        uno::Reference < util::XCloseable > xClose( mxObj, uno::UNO_QUERY );
        if ( xClose.is() )
            xClose->removeCloseListener( mpImp->xListener );

        uno::Reference < document::XEventBroadcaster > xBrd( mxObj, uno::UNO_QUERY );
        if ( xBrd.is() )
            xBrd->removeEventListener( mpImp->xListener );

        if ( mpImp->bIsLocked )
        {
            if ( xClose.is() )
            {
                try
                {
                    mxObj->changeState( embed::EmbedStates::LOADED );
                    xClose->close( sal_True );
                }
                catch ( util::CloseVetoException& )
                {
                    // there's still someone who needs the object!
                }
                catch ( uno::Exception& )
                {
                    OSL_ENSURE( sal_False, "Error on switching of the object to loaded state and closing!\n" );
                }
            }
        }

        if ( mpImp->xListener )
        {
            mpImp->xListener->pObject = 0;
            mpImp->xListener->release();
            mpImp->xListener = 0;
        }

        mxObj = 0;
        mpImp->bNeedUpdate = sal_False;
    }

    mpImp->pContainer = 0;
    mpImp->bIsLocked = sal_False;
    mpImp->bNeedUpdate = sal_False;
}

void EmbeddedObjectRef::AssignToContainer( comphelper::EmbeddedObjectContainer* pContainer, const ::rtl::OUString& rPersistName )
{
    mpImp->pContainer = pContainer;
    mpImp->aPersistName = rPersistName;

    if ( mpImp->pGraphic && !mpImp->bNeedUpdate && pContainer )
        SetGraphicToContainer( *mpImp->pGraphic, *pContainer, mpImp->aPersistName, ::rtl::OUString() );
}

comphelper::EmbeddedObjectContainer* EmbeddedObjectRef::GetContainer() const
{
    return mpImp->pContainer;
}

::rtl::OUString EmbeddedObjectRef::GetPersistName() const
{
    return mpImp->aPersistName;
}

MapUnit EmbeddedObjectRef::GetMapUnit() const
{
    if ( mpImp->nViewAspect == embed::Aspects::MSOLE_CONTENT )
        return VCLUnoHelper::UnoEmbed2VCLMapUnit( mxObj->getMapUnit( mpImp->nViewAspect ) );
    else
        // TODO/LATER: currently only CONTENT aspect requires communication with the object
        return MAP_100TH_MM;
}

sal_Int64 EmbeddedObjectRef::GetViewAspect() const
{
    return mpImp->nViewAspect;
}

void EmbeddedObjectRef::SetViewAspect( sal_Int64 nAspect )
{
    mpImp->nViewAspect = nAspect;
}

void EmbeddedObjectRef::Lock( sal_Bool bLock )
{
    mpImp->bIsLocked = bLock;
}

sal_Bool EmbeddedObjectRef::IsLocked() const
{
    return mpImp->bIsLocked;
}

void EmbeddedObjectRef::GetReplacement( sal_Bool bUpdate )
{
    if ( bUpdate )
    {
        DELETEZ( mpImp->pGraphic );
        mpImp->aMediaType = ::rtl::OUString();
        mpImp->pGraphic = new Graphic;
        mpImp->mnGraphicVersion++;
    }
    else if ( !mpImp->pGraphic )
    {
        mpImp->pGraphic = new Graphic;
        mpImp->mnGraphicVersion++;
    }
    else
    {
        OSL_FAIL("No update, but replacement exists already!");
        return;
    }

    SvStream* pGraphicStream = GetGraphicStream( bUpdate );
    if ( pGraphicStream )
    {
        GraphicFilter* pGF = GraphicFilter::GetGraphicFilter();
        if( mpImp->pGraphic )
            pGF->ImportGraphic( *mpImp->pGraphic, String(), *pGraphicStream, GRFILTER_FORMAT_DONTKNOW );
        mpImp->mnGraphicVersion++;
        delete pGraphicStream;
    }
}

Graphic* EmbeddedObjectRef::GetGraphic( ::rtl::OUString* pMediaType ) const
{
    if ( mpImp->bNeedUpdate )
        // bNeedUpdate will be set to false while retrieving new replacement
        const_cast < EmbeddedObjectRef* >(this)->GetReplacement( sal_True );
    else if ( !mpImp->pGraphic )
        const_cast < EmbeddedObjectRef* >(this)->GetReplacement( sal_False );

    if ( mpImp->pGraphic && pMediaType )
        *pMediaType = mpImp->aMediaType;
    return mpImp->pGraphic;
}

Size EmbeddedObjectRef::GetSize( MapMode* pTargetMapMode ) const
{
    MapMode aSourceMapMode( MAP_100TH_MM );
    Size aResult;

    if ( mpImp->nViewAspect == embed::Aspects::MSOLE_ICON )
    {
        Graphic* pGraphic = GetGraphic();
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

        if ( mxObj.is() )
        {
            try
            {
                aSize = mxObj->getVisualAreaSize( mpImp->nViewAspect );
            }
            catch( embed::NoVisualAreaSizeException& )
            {
            }
            catch( uno::Exception& )
            {
                OSL_ENSURE( sal_False, "Something went wrong on getting of the size of the object!" );
            }

            try
            {
                aSourceMapMode = VCLUnoHelper::UnoEmbed2VCLMapUnit( mxObj->getMapUnit( mpImp->nViewAspect ) );
            }
            catch( uno::Exception )
            {
                OSL_ENSURE( sal_False, "Can not get the map mode!" );
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
                                            const ::rtl::OUString& rMediaType )
{
    if ( mpImp->pGraphic )
        delete mpImp->pGraphic;
    mpImp->pGraphic = new Graphic();
    mpImp->aMediaType = rMediaType;
    mpImp->mnGraphicVersion++;

    SvStream* pGraphicStream = ::utl::UcbStreamHelper::CreateStream( xInGrStream );

    if ( pGraphicStream )
    {
        GraphicFilter* pGF = GraphicFilter::GetGraphicFilter();
        pGF->ImportGraphic( *mpImp->pGraphic, String(), *pGraphicStream, GRFILTER_FORMAT_DONTKNOW );
        mpImp->mnGraphicVersion++;

        if ( mpImp->pContainer )
        {
            pGraphicStream->Seek( 0 );
            uno::Reference< io::XInputStream > xInSeekGrStream = new ::utl::OSeekableInputStreamWrapper( pGraphicStream );

            mpImp->pContainer->InsertGraphicStream( xInSeekGrStream, mpImp->aPersistName, rMediaType );
        }

        delete pGraphicStream;
    }

    mpImp->bNeedUpdate = sal_False;

}

void EmbeddedObjectRef::SetGraphic( const Graphic& rGraphic, const ::rtl::OUString& rMediaType )
{
    if ( mpImp->pGraphic )
        delete mpImp->pGraphic;
    mpImp->pGraphic = new Graphic( rGraphic );
    mpImp->aMediaType = rMediaType;
    mpImp->mnGraphicVersion++;

    if ( mpImp->pContainer )
        SetGraphicToContainer( rGraphic, *mpImp->pContainer, mpImp->aPersistName, rMediaType );

    mpImp->bNeedUpdate = sal_False;
}

SvStream* EmbeddedObjectRef::GetGraphicStream( sal_Bool bUpdate ) const
{
    RTL_LOGFILE_CONTEXT( aLog, "svtools (mv76033) svt::EmbeddedObjectRef::GetGraphicStream" );
    DBG_ASSERT( bUpdate || mpImp->pContainer, "Can't retrieve current graphic!" );
    uno::Reference < io::XInputStream > xStream;
    if ( mpImp->pContainer && !bUpdate )
    {
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "getting stream from container" );
        // try to get graphic stream from container storage
        xStream = mpImp->pContainer->GetGraphicStream( mxObj, &mpImp->aMediaType );
        if ( xStream.is() )
        {
            const sal_Int32 nConstBufferSize = 32000;
            SvStream *pStream = new SvMemoryStream( 32000, 32000 );
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
    }

    if ( !xStream.is() )
    {
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "getting stream from object" );
        // update wanted or no stream in container storage available
        xStream = GetGraphicReplacementStream( mpImp->nViewAspect, mxObj, &mpImp->aMediaType );

        if ( xStream.is() )
        {
            if ( mpImp->pContainer )
                mpImp->pContainer->InsertGraphicStream( xStream, mpImp->aPersistName, mpImp->aMediaType );

            SvStream* pResult = ::utl::UcbStreamHelper::CreateStream( xStream );
            if ( pResult && bUpdate )
                mpImp->bNeedUpdate = sal_False;

            return pResult;
        }
    }

    return NULL;
}

void EmbeddedObjectRef::DrawPaintReplacement( const Rectangle &rRect, const String &rText, OutputDevice *pOut )
{
    MapMode aMM( MAP_APPFONT );
    Size aAppFontSz = pOut->LogicToLogic( Size( 0, 8 ), &aMM, NULL );
    Font aFnt( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Helvetica" ) ), aAppFontSz );
    aFnt.SetTransparent( sal_True );
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

        sal_Bool bTiny = sal_False;
        if( aPt.X() < 0 ) bTiny = sal_True, aPt.X() = 0;
        if( aPt.Y() < 0 ) bTiny = sal_True, aPt.Y() = 0;
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
    if( nHeight > 0 )
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

sal_Bool EmbeddedObjectRef::TryRunningState()
{
    return TryRunningState( mxObj );
}

sal_Bool EmbeddedObjectRef::TryRunningState( const uno::Reference < embed::XEmbeddedObject >& xEmbObj )
{
    if ( !xEmbObj.is() )
        return sal_False;

    try
    {
        if ( xEmbObj->getCurrentState() == embed::EmbedStates::LOADED )
            xEmbObj->changeState( embed::EmbedStates::RUNNING );
    }
    catch ( uno::Exception& )
    {
        return sal_False;
    }

    return sal_True;
}

void EmbeddedObjectRef::SetGraphicToContainer( const Graphic& rGraphic,
                                                comphelper::EmbeddedObjectContainer& aContainer,
                                                const ::rtl::OUString& aName,
                                                const ::rtl::OUString& aMediaType )
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
        OSL_ENSURE( sal_False, "Export of graphic is failed!\n" );
}

sal_Bool EmbeddedObjectRef::ObjectIsModified( const uno::Reference< embed::XEmbeddedObject >& xObj )
    throw( uno::Exception )
{
    sal_Bool bResult = sal_False;

    sal_Int32 nState = xObj->getCurrentState();
    if ( nState != embed::EmbedStates::LOADED && nState != embed::EmbedStates::RUNNING )
    {
        // the object is active so if the model is modified the replacement
        // should be retrieved from the object
        uno::Reference< util::XModifiable > xModifiable( xObj->getComponent(), uno::UNO_QUERY );
        if ( xModifiable.is() )
            bResult = xModifiable->isModified();
    }

    return bResult;
}

uno::Reference< io::XInputStream > EmbeddedObjectRef::GetGraphicReplacementStream(
                                                                sal_Int64 nViewAspect,
                                                                const uno::Reference< embed::XEmbeddedObject >& xObj,
                                                                ::rtl::OUString* pMediaType )
    throw()
{
    return ::comphelper::EmbeddedObjectContainer::GetGraphicReplacementStream(nViewAspect,xObj,pMediaType);
}

void EmbeddedObjectRef::UpdateReplacementOnDemand()
{
    DELETEZ( mpImp->pGraphic );
    mpImp->bNeedUpdate = sal_True;
    mpImp->mnGraphicVersion++;

    if( mpImp->pContainer )
    {
        //remove graphic from container thus a new up to date one is requested on save
        mpImp->pContainer->RemoveGraphicStream( mpImp->aPersistName );
    }
}

sal_Bool EmbeddedObjectRef::IsChart() const
{
    //todo maybe for 3.0:
    //if the changes work good for chart
    //we should apply them for all own ole objects

    //#i83708# #i81857# #i79578# request an ole replacement image only if really necessary
    //as this call can be very expensive and does block the user interface as long at it takes

    if ( !mxObj.is() )
        return false;

    SvGlobalName aObjClsId( mxObj->getClassID() );
    if(
        SvGlobalName(SO3_SCH_CLASSID_30) == aObjClsId
        || SvGlobalName(SO3_SCH_CLASSID_40) == aObjClsId
        || SvGlobalName(SO3_SCH_CLASSID_50) == aObjClsId
        || SvGlobalName(SO3_SCH_CLASSID_60) == aObjClsId)
    {
        return sal_True;
    }

    return sal_False;
}

// #i104867#
sal_uInt32 EmbeddedObjectRef::getGraphicVersion() const
{
    return mpImp->mnGraphicVersion;
}

void EmbeddedObjectRef::SetDefaultSizeForChart( const Size& rSizeIn_100TH_MM )
{
    //#i103460# charts do not necessaryly have an own size within ODF files,
    //for this case they need to use the size settings from the surrounding frame,
    //which is made available with this method

    mpImp->aDefaultSizeForChart_In_100TH_MM = awt::Size( rSizeIn_100TH_MM.getWidth(), rSizeIn_100TH_MM.getHeight() );

    ::com::sun::star::uno::Reference < ::com::sun::star::chart2::XDefaultSizeTransmitter > xSizeTransmitter( mxObj, uno::UNO_QUERY );
    DBG_ASSERT( xSizeTransmitter.is(), "Object does not support XDefaultSizeTransmitter -> will cause #i103460#!" );
    if( xSizeTransmitter.is() )
        xSizeTransmitter->setDefaultSize( mpImp->aDefaultSizeForChart_In_100TH_MM );
}

} // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
