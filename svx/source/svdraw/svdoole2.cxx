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

#include <svx/svdoole2.hxx>

#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/ObjectSaveVetoException.hpp>
#include <com/sun/star/embed/XEmbedPersist2.hpp>
#include <com/sun/star/embed/XInplaceClient.hpp>
#include <com/sun/star/embed/XInplaceObject.hpp>
#include <com/sun/star/embed/XLinkageSupport.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/embed/XWindowSupplier.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/convert.hxx>

#include <vcl/graphicfilter.hxx>
#include <svtools/embedhlp.hxx>

#include <sfx2/objsh.hxx>
#include <sfx2/ipclient.hxx>
#include <sfx2/lnkbase.hxx>
#include <tools/stream.hxx>
#include <comphelper/anytostring.hxx>
#include <svx/svdpagv.hxx>
#include <tools/globname.hxx>
#include <vcl/jobset.hxx>
#include <comphelper/classids.hxx>

#include <sot/formats.hxx>
#include <svtools/transfer.hxx>
#include <cppuhelper/implbase.hxx>

#include <svl/solar.hrc>
#include <svl/urihelper.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include <svx/charthelper.hxx>
#include <svx/svdmodel.hxx>
#include "svdglob.hxx"
#include "svx/svdstr.hrc"
#include <svx/svdetc.hxx>
#include <svx/svdview.hxx>
#include "unomlstr.hxx"
#include <sdr/contact/viewcontactofsdrole2obj.hxx>
#include <svx/svdograf.hxx>
#include <sdr/properties/oleproperties.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xflbstit.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdpage.hxx>
#include <rtl/ref.hxx>

using namespace ::com::sun::star;

static uno::Reference < beans::XPropertySet > lcl_getFrame_throw(const SdrOle2Obj* _pObject)
{
    uno::Reference < beans::XPropertySet > xFrame;
    if ( _pObject )
    {
        uno::Reference< frame::XController> xController = _pObject->GetParentXModel()->getCurrentController();
        if ( xController.is() )
        {
            xFrame.set( xController->getFrame(),uno::UNO_QUERY_THROW);
        }
    } // if ( _pObject )
    return xFrame;
}

class SdrLightEmbeddedClient_Impl : public ::cppu::WeakImplHelper
                                                            < embed::XStateChangeListener
                                                            , document::XEventListener
                                                            , embed::XInplaceClient
                                                            , embed::XEmbeddedClient
                                                            , embed::XWindowSupplier
                                                            >
{
    uno::Reference< awt::XWindow > m_xWindow;
    SdrOle2Obj* mpObj;

    Fraction m_aScaleWidth;
    Fraction m_aScaleHeight;


public:
    explicit SdrLightEmbeddedClient_Impl( SdrOle2Obj* pObj );
    void Release();

    void SetSizeScale( const Fraction& aScaleWidth, const Fraction& aScaleHeight )
    {
        m_aScaleWidth = aScaleWidth;
        m_aScaleHeight = aScaleHeight;
    }

    const Fraction& GetScaleWidth() const { return m_aScaleWidth; }
    const Fraction& GetScaleHeight() const { return m_aScaleHeight; }

    void setWindow(const uno::Reference< awt::XWindow >& _xWindow);

private:
    Rectangle impl_getScaledRect_nothrow() const;
    // XStateChangeListener
    virtual void SAL_CALL changingState( const css::lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) override;
    virtual void SAL_CALL stateChanged( const css::lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) override;

    // document::XEventListener
    virtual void SAL_CALL       notifyEvent( const document::EventObject& aEvent ) override;

    // XEmbeddedClient
    virtual void SAL_CALL saveObject() override;
    virtual void SAL_CALL visibilityChanged( sal_Bool bVisible ) override;

    // XComponentSupplier
    virtual uno::Reference< util::XCloseable > SAL_CALL getComponent() override;

    // XInplaceClient
    virtual sal_Bool SAL_CALL canInplaceActivate() override;
    virtual void SAL_CALL activatingInplace() override;
    virtual void SAL_CALL activatingUI() override;
    virtual void SAL_CALL deactivatedInplace() override;
    virtual void SAL_CALL deactivatedUI() override;
    virtual uno::Reference< css::frame::XLayoutManager > SAL_CALL getLayoutManager() override;
    virtual uno::Reference< frame::XDispatchProvider > SAL_CALL getInplaceDispatchProvider() override;
    virtual awt::Rectangle SAL_CALL getPlacement() override;
    virtual awt::Rectangle SAL_CALL getClipRectangle() override;
    virtual void SAL_CALL translateAccelerators( const uno::Sequence< awt::KeyEvent >& aKeys ) override;
    virtual void SAL_CALL scrollObject( const awt::Size& aOffset ) override;
    virtual void SAL_CALL changedPlacement( const awt::Rectangle& aPosRect ) override;

    // XWindowSupplier
    virtual uno::Reference< awt::XWindow > SAL_CALL getWindow() override;
};

SdrLightEmbeddedClient_Impl::SdrLightEmbeddedClient_Impl( SdrOle2Obj* pObj )
: mpObj( pObj )
{
}
Rectangle SdrLightEmbeddedClient_Impl::impl_getScaledRect_nothrow() const
{
    Rectangle aLogicRect( mpObj->GetLogicRect() );
    // apply scaling to object area and convert to pixels
    aLogicRect.SetSize( Size( Fraction( aLogicRect.GetWidth() ) * m_aScaleWidth,
                                Fraction( aLogicRect.GetHeight() ) * m_aScaleHeight ) );
    return aLogicRect;
}

void SAL_CALL SdrLightEmbeddedClient_Impl::changingState( const css::lang::EventObject& /*aEvent*/, ::sal_Int32 /*nOldState*/, ::sal_Int32 /*nNewState*/ )
{
}

void SdrLightEmbeddedClient_Impl::Release()
{
    {
        SolarMutexGuard aGuard;
        mpObj = nullptr;
    }

    release();
}

void SAL_CALL SdrLightEmbeddedClient_Impl::stateChanged( const css::lang::EventObject& /*aEvent*/, ::sal_Int32 nOldState, ::sal_Int32 nNewState )
{
    SolarMutexGuard aGuard;

    if ( mpObj && nOldState == embed::EmbedStates::LOADED && nNewState == embed::EmbedStates::RUNNING )
    {
        mpObj->ObjectLoaded();
        GetSdrGlobalData().GetOLEObjCache().InsertObj(mpObj);
    }
    else if ( mpObj && nNewState == embed::EmbedStates::LOADED && nOldState == embed::EmbedStates::RUNNING )
    {
        GetSdrGlobalData().GetOLEObjCache().RemoveObj(mpObj);
    }
}

void SAL_CALL SdrLightEmbeddedClient_Impl::disposing( const css::lang::EventObject& /*aEvent*/ )
{
    SolarMutexGuard aGuard;

    GetSdrGlobalData().GetOLEObjCache().RemoveObj(mpObj);
}

void SAL_CALL SdrLightEmbeddedClient_Impl::notifyEvent( const document::EventObject& aEvent )
{
    // TODO/LATER: when writer uses this implementation the code could be shared with SfxInPlaceClient_Impl

    SolarMutexGuard aGuard;

    // the code currently makes sense only in case there is no other client
    if ( mpObj && mpObj->GetAspect() != embed::Aspects::MSOLE_ICON && aEvent.EventName == "OnVisAreaChanged"
      && mpObj->GetObjRef().is() && mpObj->GetObjRef()->getClientSite() == uno::Reference< embed::XEmbeddedClient >( this ) )
    {
        try
        {
            MapUnit aContainerMapUnit( MapUnit::Map100thMM );
            uno::Reference< embed::XVisualObject > xParentVis( mpObj->GetParentXModel(), uno::UNO_QUERY );
            if ( xParentVis.is() )
                aContainerMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xParentVis->getMapUnit( mpObj->GetAspect() ) );

            MapUnit aObjMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( mpObj->GetObjRef()->getMapUnit( mpObj->GetAspect() ) );

            Rectangle          aVisArea;
            awt::Size aSz;
            try
            {
                aSz = mpObj->GetObjRef()->getVisualAreaSize( mpObj->GetAspect() );
            }
            catch( embed::NoVisualAreaSizeException& )
            {
                OSL_FAIL( "No visual area size!\n" );
                aSz.Width = 5000;
                aSz.Height = 5000;
            }
            catch( uno::Exception& )
            {
                OSL_FAIL( "Unexpected exception!\n" );
                aSz.Width = 5000;
                aSz.Height = 5000;
            }

            aVisArea.SetSize( Size( aSz.Width, aSz.Height ) );
            aVisArea = OutputDevice::LogicToLogic( aVisArea, aObjMapUnit, aContainerMapUnit );
            Size aScaledSize( static_cast< long >( m_aScaleWidth * Fraction( aVisArea.GetWidth() ) ),
                                static_cast< long >( m_aScaleHeight * Fraction( aVisArea.GetHeight() ) ) );
            Rectangle aLogicRect( mpObj->GetLogicRect() );

            // react to the change if the difference is bigger than one pixel
            Size aPixelDiff =
                Application::GetDefaultDevice()->LogicToPixel(
                    Size( aLogicRect.GetWidth() - aScaledSize.Width(),
                          aLogicRect.GetHeight() - aScaledSize.Height() ),
                    aContainerMapUnit );
            if( aPixelDiff.Width() || aPixelDiff.Height() )
            {
                mpObj->SetLogicRect( Rectangle( aLogicRect.TopLeft(), aScaledSize ) );
                mpObj->BroadcastObjectChange();
            }
            else
                mpObj->ActionChanged();
        }
        catch( uno::Exception& )
        {
            OSL_FAIL( "Unexpected exception!\n" );
        }
    }
}

void SAL_CALL SdrLightEmbeddedClient_Impl::saveObject()
{
    // TODO/LATER: when writer uses this implementation the code could be shared with SfxInPlaceClient_Impl
    uno::Reference< embed::XCommonEmbedPersist > xPersist;
    uno::Reference< util::XModifiable > xModifiable;

    {
        SolarMutexGuard aGuard;

        if ( !mpObj )
            throw embed::ObjectSaveVetoException();

        // the common persistence is supported by objects and links
        xPersist.set( mpObj->GetObjRef(), uno::UNO_QUERY_THROW );
        xModifiable.set( mpObj->GetParentXModel(), uno::UNO_QUERY );
    }

    xPersist->storeOwn();

    if ( xModifiable.is() )
        xModifiable->setModified( true );
}

void SAL_CALL SdrLightEmbeddedClient_Impl::visibilityChanged( sal_Bool /*bVisible*/ )
{
    // nothing to do currently
    // TODO/LATER: when writer uses this implementation the code could be shared with SfxInPlaceClient_Impl
    if ( mpObj )
    {
        Rectangle aLogicRect( mpObj->GetLogicRect() );
        Size aLogicSize( aLogicRect.GetWidth(), aLogicRect.GetHeight() );

        if( mpObj->IsChart() )
        {
            //charts never should be stretched see #i84323# for example
            mpObj->SetLogicRect( Rectangle( aLogicRect.TopLeft(), aLogicSize ) );
            mpObj->BroadcastObjectChange();
        } // if( mpObj->IsChart() )
    }
}

uno::Reference< util::XCloseable > SAL_CALL SdrLightEmbeddedClient_Impl::getComponent()
{
    uno::Reference< util::XCloseable > xResult;

    SolarMutexGuard aGuard;
    if ( mpObj )
        xResult.set( mpObj->GetParentXModel(), uno::UNO_QUERY );

    return xResult;
}
// XInplaceClient

sal_Bool SAL_CALL SdrLightEmbeddedClient_Impl::canInplaceActivate()
{
    bool bRet = false;
    SolarMutexGuard aGuard;
    if ( mpObj )
    {
        uno::Reference< embed::XEmbeddedObject > xObject = mpObj->GetObjRef();
        if ( !xObject.is() )
            throw uno::RuntimeException();
        // we don't want to switch directly from outplace to inplace mode
        bRet = !( xObject->getCurrentState() == embed::EmbedStates::ACTIVE || mpObj->GetAspect() == embed::Aspects::MSOLE_ICON );
    } // if ( mpObj )
    return bRet;
}

void SAL_CALL SdrLightEmbeddedClient_Impl::activatingInplace()
{
}

void SAL_CALL SdrLightEmbeddedClient_Impl::activatingUI()
{
    SolarMutexGuard aGuard;

    uno::Reference < beans::XPropertySet > xFrame( lcl_getFrame_throw(mpObj));
    uno::Reference < frame::XFrame > xOwnFrame( xFrame,uno::UNO_QUERY);
    uno::Reference < frame::XFramesSupplier > xParentFrame( xOwnFrame->getCreator(), uno::UNO_QUERY );
    if ( xParentFrame.is() )
        xParentFrame->setActiveFrame( xOwnFrame );

    OLEObjCache& rObjCache = GetSdrGlobalData().GetOLEObjCache();
    const sal_uIntPtr nCount = rObjCache.size();
    for(sal_Int32 i = nCount-1 ; i >= 0;--i)
    {
        SdrOle2Obj* pObj = rObjCache[i];
        if ( pObj != mpObj )
        {
            // only deactivate ole objects which belongs to the same frame
            if ( xFrame == lcl_getFrame_throw(pObj) )
            {
                uno::Reference< embed::XEmbeddedObject > xObject = pObj->GetObjRef();
                try
                {
                    if ( (xObject->getStatus( pObj->GetAspect() ) & embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE) ||
                         svt::EmbeddedObjectRef::IsGLChart(xObject) )
                        xObject->changeState( embed::EmbedStates::INPLACE_ACTIVE );
                    else
                    {
                        // the links should not stay in running state for long time because of locking
                        uno::Reference< embed::XLinkageSupport > xLink( xObject, uno::UNO_QUERY );
                        if ( xLink.is() && xLink->isLink() )
                            xObject->changeState( embed::EmbedStates::LOADED );
                        else
                            xObject->changeState( embed::EmbedStates::RUNNING );
                    }
                }
                catch (css::uno::Exception& )
                {}
            }
        }
    } // for(sal_Int32 i = nCount-1 ; i >= 0;--i)
}

void SAL_CALL SdrLightEmbeddedClient_Impl::deactivatedInplace()
{
}

void SAL_CALL SdrLightEmbeddedClient_Impl::deactivatedUI()
{
    SolarMutexGuard aGuard;
    css::uno::Reference< css::frame::XLayoutManager > xLayoutManager(getLayoutManager());
    if ( xLayoutManager.is() )
    {
        static const char aMenuBarURL[] = "private:resource/menubar/menubar";
        if ( !xLayoutManager->isElementVisible( aMenuBarURL ) )
            xLayoutManager->createElement( aMenuBarURL );
    }
}

uno::Reference< css::frame::XLayoutManager > SAL_CALL SdrLightEmbeddedClient_Impl::getLayoutManager()
{
    uno::Reference< css::frame::XLayoutManager > xMan;
    SolarMutexGuard aGuard;
    uno::Reference < beans::XPropertySet > xFrame( lcl_getFrame_throw(mpObj));
    try
    {
        xMan.set(xFrame->getPropertyValue("LayoutManager"),uno::UNO_QUERY);
    }
    catch ( uno::Exception& )
    {
        throw uno::RuntimeException();
    }

    return xMan;
}

uno::Reference< frame::XDispatchProvider > SAL_CALL SdrLightEmbeddedClient_Impl::getInplaceDispatchProvider()
{
    SolarMutexGuard aGuard;
    return uno::Reference < frame::XDispatchProvider >( lcl_getFrame_throw(mpObj), uno::UNO_QUERY_THROW );
}

awt::Rectangle SAL_CALL SdrLightEmbeddedClient_Impl::getPlacement()
{
    SolarMutexGuard aGuard;
    if ( !mpObj )
        throw uno::RuntimeException();

    Rectangle aLogicRect = impl_getScaledRect_nothrow();
    MapUnit aContainerMapUnit( MapUnit::Map100thMM );
    uno::Reference< embed::XVisualObject > xParentVis( mpObj->GetParentXModel(), uno::UNO_QUERY );
    if ( xParentVis.is() )
        aContainerMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xParentVis->getMapUnit( mpObj->GetAspect() ) );

    aLogicRect = Application::GetDefaultDevice()->LogicToPixel(aLogicRect,aContainerMapUnit);
    return AWTRectangle( aLogicRect );
}

awt::Rectangle SAL_CALL SdrLightEmbeddedClient_Impl::getClipRectangle()
{
    return getPlacement();
}

void SAL_CALL SdrLightEmbeddedClient_Impl::translateAccelerators( const uno::Sequence< awt::KeyEvent >& /*aKeys*/ )
{
}

void SAL_CALL SdrLightEmbeddedClient_Impl::scrollObject( const awt::Size& /*aOffset*/ )
{
}

void SAL_CALL SdrLightEmbeddedClient_Impl::changedPlacement( const awt::Rectangle& aPosRect )
{
    SolarMutexGuard aGuard;
    if ( !mpObj )
        throw uno::RuntimeException();

    uno::Reference< embed::XInplaceObject > xInplace( mpObj->GetObjRef(), uno::UNO_QUERY );
    if ( !xInplace.is() )
        throw uno::RuntimeException();

    // check if the change is at least one pixel in size
    awt::Rectangle aOldRect = getPlacement();
    Rectangle aNewPixelRect = VCLRectangle( aPosRect );
    Rectangle aOldPixelRect = VCLRectangle( aOldRect );
    if ( aOldPixelRect == aNewPixelRect )
        // nothing has changed
        return;

    // new scaled object area
    MapUnit aContainerMapUnit( MapUnit::Map100thMM );
    uno::Reference< embed::XVisualObject > xParentVis( mpObj->GetParentXModel(), uno::UNO_QUERY );
    if ( xParentVis.is() )
        aContainerMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xParentVis->getMapUnit( mpObj->GetAspect() ) );

    Rectangle aNewLogicRect = Application::GetDefaultDevice()->PixelToLogic(aNewPixelRect,aContainerMapUnit);
    Rectangle aLogicRect = impl_getScaledRect_nothrow();

    if ( aNewLogicRect != aLogicRect )
    {
        // the calculation of the object area has not changed the object size
        // it should be done here then
        //SfxBooleanFlagGuard aGuard( m_bResizeNoScale, true );

        // new size of the object area without scaling
        Size aNewObjSize( Fraction( aNewLogicRect.GetWidth() ) / m_aScaleWidth,
                          Fraction( aNewLogicRect.GetHeight() ) / m_aScaleHeight );

        // now remove scaling from new placement and keep this a the new object area
        aNewLogicRect.SetSize( aNewObjSize );
        // react to the change if the difference is bigger than one pixel
        Size aPixelDiff =
            Application::GetDefaultDevice()->LogicToPixel(
                Size( aLogicRect.GetWidth() - aNewObjSize.Width(),
                      aLogicRect.GetHeight() - aNewObjSize.Height() ),
                aContainerMapUnit );
        if( aPixelDiff.Width() || aPixelDiff.Height() )
        {
            mpObj->SetLogicRect( Rectangle( aLogicRect.TopLeft(), aNewObjSize ) );
            mpObj->BroadcastObjectChange();
        }
        else
            mpObj->ActionChanged();
    }
}
// XWindowSupplier

uno::Reference< awt::XWindow > SAL_CALL SdrLightEmbeddedClient_Impl::getWindow()
{
    SolarMutexGuard aGuard;
    uno::Reference< awt::XWindow > xCurrent = m_xWindow;
    if ( !xCurrent.is() )
    {
        if ( !mpObj )
            throw uno::RuntimeException();
        uno::Reference< frame::XFrame> xFrame(lcl_getFrame_throw(mpObj),uno::UNO_QUERY_THROW);
        xCurrent = xFrame->getComponentWindow();
    } // if ( !xCurrent.is() )
    return xCurrent;
}
void SdrLightEmbeddedClient_Impl::setWindow(const uno::Reference< awt::XWindow >& _xWindow)
{
    m_xWindow = _xWindow;
}

SdrEmbedObjectLink::SdrEmbedObjectLink(SdrOle2Obj* pObject):
    ::sfx2::SvBaseLink( ::SfxLinkUpdateMode::ONCALL, SotClipboardFormatId::SVXB ),
    pObj(pObject)
{
    SetSynchron( false );
}

SdrEmbedObjectLink::~SdrEmbedObjectLink()
{
}

::sfx2::SvBaseLink::UpdateResult SdrEmbedObjectLink::DataChanged(
    const OUString& /*rMimeType*/, const css::uno::Any & /*rValue*/ )
{
    if ( !pObj->UpdateLinkURL_Impl() )
    {
        // the link URL was not changed
        uno::Reference< embed::XEmbeddedObject > xObject = pObj->GetObjRef();
        OSL_ENSURE( xObject.is(), "The object must exist always!\n" );
        if ( xObject.is() )
        {
            // let the object reload the link
            // TODO/LATER: reload call could be used for this case

            try
            {
                sal_Int32 nState = xObject->getCurrentState();
                if ( nState != embed::EmbedStates::LOADED )
                {
                    // in some cases the linked file probably is not locked so it could be changed
                    xObject->changeState( embed::EmbedStates::LOADED );
                    xObject->changeState( nState );
                }
            }
            catch ( uno::Exception& )
            {
            }
        }
    }

    pObj->GetNewReplacement();
    pObj->SetChanged();

    return SUCCESS;
}

void SdrEmbedObjectLink::Closed()
{
    pObj->BreakFileLink_Impl();
    SvBaseLink::Closed();
}

class SdrOle2ObjImpl
{
public:
    svt::EmbeddedObjectRef mxObjRef;

    Graphic* mpGraphic;
    // TODO/LATER: do we really need this pointer?
    GraphicObject* mpGraphicObject;
    OUString maProgName;
    OUString        aPersistName;       // name of object in persist
    SdrLightEmbeddedClient_Impl* pLightClient; // must be registered as client only using AddOwnLightClient() call

    bool mbFrame:1; // Due to compatibility at SdrTextObj for now
    bool mbInDestruction:1;
    bool mbSuppressSetVisAreaSize:1; // #i118524#
    mutable bool mbTypeAsked:1;
    mutable bool mbIsChart:1;
    bool mbLoadingOLEObjectFailed:1; // New local var to avoid repeated loading if load of OLE2 fails
    bool mbConnected:1;

    SdrEmbedObjectLink* mpObjectLink;
    OUString maLinkURL;

    rtl::Reference<SvxUnoShapeModifyListener> mxModifyListener;

    explicit SdrOle2ObjImpl( bool bFrame ) :
        mpGraphic(nullptr),
        mpGraphicObject(nullptr),
        pLightClient (nullptr),
        mbFrame(bFrame),
        mbInDestruction(false),
        mbSuppressSetVisAreaSize(false),
        mbTypeAsked(false),
        mbIsChart(false),
        mbLoadingOLEObjectFailed(false),
        mbConnected(false),
        mpObjectLink(nullptr)
    {
        mxObjRef.Lock();
    }

    SdrOle2ObjImpl( bool bFrame, const svt::EmbeddedObjectRef& rObjRef ) :
        mxObjRef(rObjRef),
        mpGraphic(nullptr),
        mpGraphicObject(nullptr),
        pLightClient (nullptr),
        mbFrame(bFrame),
        mbInDestruction(false),
        mbSuppressSetVisAreaSize(false),
        mbTypeAsked(false),
        mbIsChart(false),
        mbLoadingOLEObjectFailed(false),
        mbConnected(false),
        mpObjectLink(nullptr)
    {
        mxObjRef.Lock();
    }

    ~SdrOle2ObjImpl()
    {
        delete mpGraphic;
        delete mpGraphicObject;

        if (mxModifyListener.is())
        {
            mxModifyListener->invalidate();
        }
    }
};

// Predicate determining whether the given OLE is an internal math
// object
static bool ImplIsMathObj( const uno::Reference < embed::XEmbeddedObject >& rObjRef )
{
    if ( !rObjRef.is() )
        return false;

    SvGlobalName aClassName( rObjRef->getClassID() );
    if( aClassName == SvGlobalName(SO3_SM_CLASSID_30) ||
        aClassName == SvGlobalName(SO3_SM_CLASSID_40) ||
        aClassName == SvGlobalName(SO3_SM_CLASSID_50) ||
        aClassName == SvGlobalName(SO3_SM_CLASSID_60) ||
        aClassName == SvGlobalName(SO3_SM_CLASSID)      )
    {
        return true;
    }
    else
    {
        return false;
    }
}

// BaseProperties section

sdr::properties::BaseProperties* SdrOle2Obj::CreateObjectSpecificProperties()
{
    return new sdr::properties::OleProperties(*this);
}

// DrawContact section

sdr::contact::ViewContact* SdrOle2Obj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrOle2Obj(*this);
}

SdrOle2Obj::SdrOle2Obj( bool bFrame_ ) :
    mpImpl(new SdrOle2ObjImpl(bFrame_))
{
}

SdrOle2Obj::SdrOle2Obj( const svt::EmbeddedObjectRef&  rNewObjRef, const OUString& rNewObjName, const Rectangle& rNewRect) :
    SdrRectObj(rNewRect),
    mpImpl(new SdrOle2ObjImpl(false/*bFrame_*/, rNewObjRef))
{
    mpImpl->aPersistName = rNewObjName;

    if (mpImpl->mxObjRef.is() && (mpImpl->mxObjRef->getStatus( GetAspect() ) & embed::EmbedMisc::EMBED_NEVERRESIZE ) )
        SetResizeProtect(true);

    // For math objects, set closed state to transparent
    SetClosedObj(!ImplIsMathObj( mpImpl->mxObjRef.GetObject() ));
}

OUString SdrOle2Obj::GetStyleString()
{
    OUString strStyle;
    if (mpImpl->mxObjRef.is() && mpImpl->mxObjRef.IsChart())
    {
        strStyle = mpImpl->mxObjRef.GetChartType();
    }
    return strStyle;
}

SdrOle2Obj::~SdrOle2Obj()
{
    mpImpl->mbInDestruction = true;

    if ( mpImpl->mbConnected )
        Disconnect();

    DisconnectFileLink_Impl();

    if ( mpImpl->pLightClient )
    {
        mpImpl->pLightClient->Release();
        mpImpl->pLightClient = nullptr;
    }
}

void SdrOle2Obj::SetAspect( sal_Int64 nAspect )
{
    mpImpl->mxObjRef.SetViewAspect( nAspect );
}

const svt::EmbeddedObjectRef& SdrOle2Obj::getEmbeddedObjectRef() const
{
    return mpImpl->mxObjRef;
}

sal_Int64 SdrOle2Obj::GetAspect() const
{
    return mpImpl->mxObjRef.GetViewAspect();
}

bool SdrOle2Obj::isInplaceActive() const
{
    return mpImpl->mxObjRef.is() && embed::EmbedStates::INPLACE_ACTIVE == mpImpl->mxObjRef->getCurrentState();
}

bool SdrOle2Obj::isUiActive() const
{
    return mpImpl->mxObjRef.is() && embed::EmbedStates::UI_ACTIVE == mpImpl->mxObjRef->getCurrentState();
}

void SdrOle2Obj::SetGraphic_Impl(const Graphic* pGrf)
{
    if (mpImpl->mpGraphic)
    {
        delete mpImpl->mpGraphic;
        mpImpl->mpGraphic = nullptr;
        delete mpImpl->mpGraphicObject;
        mpImpl->mpGraphicObject = nullptr;
    }

    if (pGrf)
    {
        mpImpl->mpGraphic = new Graphic(*pGrf);
        mpImpl->mpGraphicObject = new GraphicObject(*mpImpl->mpGraphic);
    }

    SetChanged();
    BroadcastObjectChange();
}

void SdrOle2Obj::SetGraphic(const Graphic* pGrf)
{
    // only for setting a preview graphic
    SetGraphic_Impl( pGrf );
}

void SdrOle2Obj::SetProgName( const OUString& rName )
{
    mpImpl->maProgName = rName;
}

const OUString& SdrOle2Obj::GetProgName() const
{
    return mpImpl->maProgName;
}

bool SdrOle2Obj::IsEmpty() const
{
    return !mpImpl->mxObjRef.is();
}

void SdrOle2Obj::Connect()
{
    if( IsEmptyPresObj() )
        return;

    if( mpImpl->mbConnected )
    {
        // currently there are situations where it seems to be unavoidable to have multiple connects
        // changing this would need a larger code rewrite, so for now I remove the assertion
        // OSL_FAIL("Connect() called on connected object!");
        return;
    }

    Connect_Impl();
    AddListeners_Impl();
}

bool SdrOle2Obj::UpdateLinkURL_Impl()
{
    bool bResult = false;

    if ( mpImpl->mpObjectLink )
    {
        sfx2::LinkManager* pLinkManager = pModel ? pModel->GetLinkManager() : nullptr;
        if ( pLinkManager )
        {
            OUString aNewLinkURL;
            sfx2::LinkManager::GetDisplayNames( mpImpl->mpObjectLink, nullptr, &aNewLinkURL );
            if ( !aNewLinkURL.equalsIgnoreAsciiCase( mpImpl->maLinkURL ) )
            {
                GetObjRef_Impl();
                uno::Reference<embed::XCommonEmbedPersist> xPersObj( mpImpl->mxObjRef.GetObject(), uno::UNO_QUERY );
                OSL_ENSURE( xPersObj.is(), "The object must exist!\n" );
                if ( xPersObj.is() )
                {
                    try
                    {
                        sal_Int32 nCurState = mpImpl->mxObjRef->getCurrentState();
                        if ( nCurState != embed::EmbedStates::LOADED )
                            mpImpl->mxObjRef->changeState(embed::EmbedStates::LOADED);

                        // TODO/LATER: there should be possible to get current mediadescriptor settings from the object
                        uno::Sequence< beans::PropertyValue > aArgs( 1 );
                        aArgs[0].Name = "URL";
                        aArgs[0].Value <<= OUString( aNewLinkURL );
                        xPersObj->reload( aArgs, uno::Sequence< beans::PropertyValue >() );

                        mpImpl->maLinkURL = aNewLinkURL;
                        bResult = true;

                        if ( nCurState != embed::EmbedStates::LOADED )
                            mpImpl->mxObjRef->changeState(nCurState);
                    }
                    catch( css::uno::Exception& )
                    {
                        OSL_FAIL(
                            OString(OString("SdrOle2Obj::UpdateLinkURL_Impl(), "
                                    "exception caught: ") +
                            OUStringToOString(
                                comphelper::anyToString( cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 )).getStr() );
                    }
                }

                if ( !bResult )
                {
                    // TODO/LATER: return the old name to the link manager, is it possible?
                }
            }
        }
    }

    return bResult;
}

void SdrOle2Obj::BreakFileLink_Impl()
{
    uno::Reference<document::XStorageBasedDocument> xDoc;
    if ( pModel )
        xDoc.set( pModel->getUnoModel(),uno::UNO_QUERY);

    if ( xDoc.is() )
    {
        uno::Reference< embed::XStorage > xStorage = xDoc->getDocumentStorage();
        if ( xStorage.is() )
        {
            try
            {
                uno::Reference< embed::XLinkageSupport > xLinkSupport( mpImpl->mxObjRef.GetObject(), uno::UNO_QUERY_THROW );
                xLinkSupport->breakLink( xStorage, mpImpl->aPersistName );
                DisconnectFileLink_Impl();
                mpImpl->maLinkURL.clear();
            }
            catch( css::uno::Exception& )
            {
                OSL_FAIL(
                    OString(OString("SdrOle2Obj::BreakFileLink_Impl(), "
                            "exception caught: ") +
                    OUStringToOString(
                        comphelper::anyToString( cppu::getCaughtException() ),
                        RTL_TEXTENCODING_UTF8 )).getStr() );
            }
        }
    }
}

void SdrOle2Obj::DisconnectFileLink_Impl()
{
    sfx2::LinkManager* pLinkManager = pModel ? pModel->GetLinkManager() : nullptr;
    if ( pLinkManager && mpImpl->mpObjectLink )
    {
        pLinkManager->Remove( mpImpl->mpObjectLink );
        mpImpl->mpObjectLink = nullptr;
    }
}

void SdrOle2Obj::CheckFileLink_Impl()
{
    if (pModel && mpImpl->mxObjRef.GetObject().is() && !mpImpl->mpObjectLink)
    {
        try
        {
            uno::Reference< embed::XLinkageSupport > xLinkSupport( mpImpl->mxObjRef.GetObject(), uno::UNO_QUERY );
            if ( xLinkSupport.is() && xLinkSupport->isLink() )
            {
                OUString aLinkURL = xLinkSupport->getLinkURL();
                if ( !aLinkURL.isEmpty() )
                {
                    // this is a file link so the model link manager should handle it
                    sfx2::LinkManager* pLinkManager = pModel->GetLinkManager();
                    if ( pLinkManager )
                    {
                        mpImpl->mpObjectLink = new SdrEmbedObjectLink( this );
                        mpImpl->maLinkURL = aLinkURL;
                        pLinkManager->InsertFileLink( *mpImpl->mpObjectLink, OBJECT_CLIENT_OLE, aLinkURL );
                        mpImpl->mpObjectLink->Connect();
                    }
                }
            }
        }
        catch( css::uno::Exception& )
        {
            OSL_FAIL(
                OString(OString("SdrOle2Obj::CheckFileLink_Impl(), "
                        "exception caught: ") +
                OUStringToOString(
                    comphelper::anyToString( cppu::getCaughtException() ),
                    RTL_TEXTENCODING_UTF8 )).getStr() );
        }
    }
}

void SdrOle2Obj::Reconnect_Impl()
{
    DBG_ASSERT( mpImpl->mbConnected, "Assigned unconnected object?!" );
    Connect_Impl();
}

void SdrOle2Obj::Connect_Impl()
{
    if( pModel && !mpImpl->aPersistName.isEmpty() )
    {
        try
        {
            ::comphelper::IEmbeddedHelper* pPers = pModel->GetPersist();
            if ( pPers )
            {
                comphelper::EmbeddedObjectContainer& rContainer = pPers->getEmbeddedObjectContainer();
                if ( !rContainer.HasEmbeddedObject( mpImpl->aPersistName )
                  || ( mpImpl->mxObjRef.is() && !rContainer.HasEmbeddedObject( mpImpl->mxObjRef.GetObject() ) ) )
                {
                    // object not known to container document
                    // No object -> disaster!
                    DBG_ASSERT( mpImpl->mxObjRef.is(), "No object in connect!");
                    if ( mpImpl->mxObjRef.is() )
                    {
                        // object came from the outside, now add it to the container
                        OUString aTmp;
                        rContainer.InsertEmbeddedObject( mpImpl->mxObjRef.GetObject(), aTmp );
                        mpImpl->aPersistName = aTmp;
                    }
                }
                else if ( !mpImpl->mxObjRef.is() )
                {
                    mpImpl->mxObjRef.Assign( rContainer.GetEmbeddedObject( mpImpl->aPersistName ), mpImpl->mxObjRef.GetViewAspect() );
                    mpImpl->mbTypeAsked = false;
                }

                if ( mpImpl->mxObjRef.GetObject().is() )
                {
                    mpImpl->mxObjRef.AssignToContainer( &rContainer, mpImpl->aPersistName );
                    mpImpl->mbConnected = true;
                    mpImpl->mxObjRef.Lock();
                }
            }

            if ( mpImpl->mxObjRef.is() )
            {
                if ( !mpImpl->pLightClient )
                {
                    mpImpl->pLightClient = new SdrLightEmbeddedClient_Impl( this );
                    mpImpl->pLightClient->acquire();
                }

                mpImpl->mxObjRef->addStateChangeListener( mpImpl->pLightClient );
                mpImpl->mxObjRef->addEventListener( uno::Reference< document::XEventListener >( mpImpl->pLightClient ) );

                if ( mpImpl->mxObjRef->getCurrentState() != embed::EmbedStates::LOADED )
                    GetSdrGlobalData().GetOLEObjCache().InsertObj(this);

                CheckFileLink_Impl();

                uno::Reference< container::XChild > xChild( mpImpl->mxObjRef.GetObject(), uno::UNO_QUERY );
                if( xChild.is() )
                {
                    uno::Reference< uno::XInterface > xParent( pModel->getUnoModel());
                    if( xParent.is())
                        xChild->setParent( pModel->getUnoModel() );
                }

            }
        }
        catch( css::uno::Exception& )
        {
            OSL_FAIL(
                OString(OString("SdrOle2Obj::Connect_Impl(), "
                        "exception caught: ") +
                OUStringToOString(
                    comphelper::anyToString( cppu::getCaughtException() ),
                    RTL_TEXTENCODING_UTF8 )).getStr() );
        }
    }

    //TODO/LATER: wait for definition of MiscStatus RESIZEONPRINTERCHANGE
    //if ( xObjRef.is() && (*ppObjRef)->GetMiscStatus() & SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE )
    {
        //TODO/LATER: needs a new handling for OnPrinterChanged
        /*
        if (pModel && pModel->GetRefDevice() &&
            pModel->GetRefDevice()->GetOutDevType() == OUTDEV_PRINTER)
        {
            // Kein RefDevice oder RefDevice kein Printer
            bool bModified = (*ppObjRef)->IsModified();
            Printer* pPrinter = (Printer*) pModel->GetRefDevice();
            (*ppObjRef)->OnDocumentPrinterChanged( pPrinter );
            (*ppObjRef)->SetModified( bModified );
        }*/
    }
}

void SdrOle2Obj::ObjectLoaded()
{
    AddListeners_Impl();
}

void SdrOle2Obj::AddListeners_Impl()
{
    if( mpImpl->mxObjRef.is() && mpImpl->mxObjRef->getCurrentState() != embed::EmbedStates::LOADED )
    {
        // register modify listener
        if (!mpImpl->mxModifyListener.is())
        {
            mpImpl->mxModifyListener = new SvxUnoShapeModifyListener(this);
        }

        uno::Reference< util::XModifyBroadcaster > xBC( getXModel(), uno::UNO_QUERY );
        if (xBC.is())
        {
            uno::Reference<util::XModifyListener> xListener(mpImpl->mxModifyListener.get());
            xBC->addModifyListener( xListener );
        }
    }
}

void SdrOle2Obj::Disconnect()
{
    if( IsEmptyPresObj() )
        return;

    if( !mpImpl->mbConnected )
    {
        OSL_FAIL("Disconnect() called on disconnected object!");
        return;
    }

    RemoveListeners_Impl();
    Disconnect_Impl();
}

void SdrOle2Obj::RemoveListeners_Impl()
{
    if ( mpImpl->mxObjRef.is() && !mpImpl->aPersistName.isEmpty() )
    {
        try
        {
            sal_Int32 nState = mpImpl->mxObjRef->getCurrentState();
            if ( nState != embed::EmbedStates::LOADED )
            {
                uno::Reference< util::XModifyBroadcaster > xBC( getXModel(), uno::UNO_QUERY );
                if (xBC.is() && mpImpl->mxModifyListener.is())
                {
                    uno::Reference<util::XModifyListener> xListener(mpImpl->mxModifyListener.get());
                    xBC->removeModifyListener( xListener );
                }
            }
        }
        catch( css::uno::Exception& )
        {
            OSL_FAIL(
                OString(OString("SdrOle2Obj::RemoveListeners_Impl(), "
                        "exception caught: ") +
                OUStringToOString(
                    comphelper::anyToString( cppu::getCaughtException() ),
                    RTL_TEXTENCODING_UTF8 )).getStr() );
        }
    }
}

void SdrOle2Obj::Disconnect_Impl()
{
    try
    {
        if ( pModel && !mpImpl->aPersistName.isEmpty() )
        {
            if( pModel->IsInDestruction() )
            {
                // TODO/LATER: here we must assume that the destruction of the model is enough to make clear that we will not
                // remove the object from the container, even if the DrawingObject itself is not destroyed (unfortunately this
                // There is no real need to do the following removing of the object from the container
                // in case the model has correct persistence, but in case of problems such a removing
                // would make the behavior of the office more stable

                comphelper::EmbeddedObjectContainer* pContainer = mpImpl->mxObjRef.GetContainer();
                if ( pContainer )
                {
                    pContainer->CloseEmbeddedObject( mpImpl->mxObjRef.GetObject() );
                    mpImpl->mxObjRef.AssignToContainer( nullptr, mpImpl->aPersistName );
                }

                // happens later than the destruction of the model, so we can't assert that).
                //DBG_ASSERT( bInDestruction, "Model is destroyed, but not me?!" );
                //TODO/LATER: should be make sure that the ObjectShell also forgets the object, because we will close it soon?
                /*
                uno::Reference < util::XCloseable > xClose( xObjRef, uno::UNO_QUERY );
                if ( xClose.is() )
                {
                    try
                    {
                        xClose->close( true );
                    }
                    catch ( util::CloseVetoException& )
                    {
                        // there's still someone who needs the object!
                    }
                }

                xObjRef = NULL;*/
            }
            else if ( mpImpl->mxObjRef.is() )
            {
                if ( pModel->getUnoModel().is() )
                {
                    // remove object, but don't close it (that's up to someone else)
                    comphelper::EmbeddedObjectContainer* pContainer = mpImpl->mxObjRef.GetContainer();
                    if ( pContainer )
                    {
                        pContainer->RemoveEmbeddedObject( mpImpl->mxObjRef.GetObject() );

                        // TODO/LATER: mpImpl->aPersistName contains outdated information, to keep it updated
                        // it should be returned from RemoveEmbeddedObject call. Currently it is no problem,
                        // since no container is adjusted, actually the empty string could be provided as a name here
                        mpImpl->mxObjRef.AssignToContainer( nullptr, mpImpl->aPersistName );
                    }

                    DisconnectFileLink_Impl();
                }
            }
        }

        if ( mpImpl->mxObjRef.is() && mpImpl->pLightClient )
        {
            mpImpl->mxObjRef->removeStateChangeListener ( mpImpl->pLightClient );
            mpImpl->mxObjRef->removeEventListener( uno::Reference< document::XEventListener >( mpImpl->pLightClient ) );
            mpImpl->mxObjRef->setClientSite( nullptr );

            GetSdrGlobalData().GetOLEObjCache().RemoveObj(this);
        }
    }
    catch( css::uno::Exception& )
    {
        OSL_FAIL(
            OString(OString("SdrOle2Obj::Disconnect_Impl(), "
                    "exception caught: ") +
            OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }

    mpImpl->mbConnected = false;
}

SdrObject* SdrOle2Obj::createSdrGrafObjReplacement(bool bAddText, bool /* bUseHCGraphic */) const
{
    const Graphic* pOLEGraphic = GetGraphic();

    if(pOLEGraphic)
    {
        // #i118485# allow creating a SdrGrafObj representation
        SdrGrafObj* pClone = new SdrGrafObj(*pOLEGraphic);
        pClone->SetModel(GetModel());

        // copy transformation
        basegfx::B2DHomMatrix aMatrix;
        basegfx::B2DPolyPolygon aPolyPolygon;

        TRGetBaseGeometry(aMatrix, aPolyPolygon);
        pClone->TRSetBaseGeometry(aMatrix, aPolyPolygon);

        // copy all attributes to support graphic styles for OLEs
        pClone->SetStyleSheet(GetStyleSheet(), false);
        pClone->SetMergedItemSet(GetMergedItemSet());

        if(bAddText)
        {
            // #i118485# copy text (Caution! Model needed, as guaranteed in aw080)
            OutlinerParaObject* pOPO = GetOutlinerParaObject();

            if(pOPO && GetModel())
            {
                pClone->NbcSetOutlinerParaObject(new OutlinerParaObject(*pOPO));
            }
        }

        return pClone;
    }
    else
    {
        // #i100710# pOLEGraphic may be zero (no visualisation available),
        // so we need to use the OLE replacement graphic
        SdrRectObj* pClone = new SdrRectObj(GetSnapRect());
        pClone->SetModel(GetModel());

        // gray outline
        pClone->SetMergedItem(XLineStyleItem(css::drawing::LineStyle_SOLID));
        const svtools::ColorConfig aColorConfig;
        const svtools::ColorConfigValue aColor(aColorConfig.GetColorValue(svtools::OBJECTBOUNDARIES));
        pClone->SetMergedItem(XLineColorItem(OUString(), aColor.nColor));

        // bitmap fill
        pClone->SetMergedItem(XFillStyleItem(drawing::FillStyle_BITMAP));
        pClone->SetMergedItem(XFillBitmapItem(OUString(), GetEmptyOLEReplacementGraphic()));
        pClone->SetMergedItem(XFillBmpTileItem(false));
        pClone->SetMergedItem(XFillBmpStretchItem(false));

        return pClone;
    }
}

SdrObject* SdrOle2Obj::DoConvertToPolyObj(bool bBezier, bool bAddText) const
{
    // #i118485# missing converter added
    if(GetModel())
    {
        SdrObject* pRetval = createSdrGrafObjReplacement(true, false);

        if(pRetval)
        {
            SdrObject* pRetval2 = pRetval->DoConvertToPolyObj(bBezier, bAddText);
            SdrObject::Free(pRetval);

            return pRetval2;
        }
    }

    return nullptr;
}

void SdrOle2Obj::SetModel(SdrModel* pNewModel)
{
    ::comphelper::IEmbeddedHelper* pDestPers = pNewModel ? pNewModel->GetPersist() : nullptr;
    ::comphelper::IEmbeddedHelper* pSrcPers  = pModel ? pModel->GetPersist() : nullptr;

    if ( pNewModel == pModel )
    {
        // don't know if this is necessary or if it will ever happen, but who knows?!
        SdrRectObj::SetModel( pNewModel );
        return;
    }

    // assignment to model has changed
    DBG_ASSERT( pSrcPers || !mpImpl->mbConnected, "Connected object without a model?!" );

    DBG_ASSERT( pDestPers, "The destination model must have a persistence! Please submit an issue!" );
    DBG_ASSERT( pDestPers != pSrcPers, "The source and the destination models should have different persistences! Problems are possible!" );

    // this is a bug if the target model has no persistence
    // no error handling is possible so just do nothing in this method
    if ( !pDestPers )
        return;

    RemoveListeners_Impl();

    if( pDestPers && pSrcPers && !IsEmptyPresObj() )
    {
        try
        {
            // move the object's storage; ObjectRef remains the same, but PersistName may change
            OUString aTmp;
            comphelper::EmbeddedObjectContainer& rContainer = pSrcPers->getEmbeddedObjectContainer();
            uno::Reference < embed::XEmbeddedObject > xObj = rContainer.GetEmbeddedObject( mpImpl->aPersistName );
            DBG_ASSERT( !mpImpl->mxObjRef.is() || mpImpl->mxObjRef.GetObject() == xObj, "Wrong object identity!" );
            if ( xObj.is() )
            {
                pDestPers->getEmbeddedObjectContainer().MoveEmbeddedObject( rContainer, xObj, aTmp );
                mpImpl->aPersistName = aTmp;
                mpImpl->mxObjRef.AssignToContainer( &pDestPers->getEmbeddedObjectContainer(), aTmp );
            }
            DBG_ASSERT( !aTmp.isEmpty(), "Copying embedded object failed!" );
        }
        catch( css::uno::Exception& )
        {
            OSL_FAIL(
                OString(OString("SdrOle2Obj::SetModel(), "
                        "exception caught: ") +
                OUStringToOString(
                    comphelper::anyToString( cppu::getCaughtException() ),
                    RTL_TEXTENCODING_UTF8 )).getStr() );
        }
    }

    SdrRectObj::SetModel( pNewModel );

    // #i43086#
    // #i85304 redo the change for charts for the above bugfix, as #i43086# does not occur anymore
    //so maybe the ImpSetVisAreaSize call can be removed here completely
    //Nevertheless I leave it in for other objects as I am not sure about the side effects when removing now
    if( pModel && !pModel->isLocked() && !IsChart() )
        ImpSetVisAreaSize();

    if( pDestPers && !IsEmptyPresObj() )
    {
        if ( !pSrcPers || IsEmptyPresObj() )
            // object wasn't connected, now it should be
            Connect_Impl();
        else
            Reconnect_Impl();
    }

    AddListeners_Impl();
}

void SdrOle2Obj::SetPage(SdrPage* pNewPage)
{
    bool bRemove=pNewPage==nullptr && pPage!=nullptr;
    bool bInsert=pNewPage!=nullptr && pPage==nullptr;

    if (bRemove && mpImpl->mbConnected )
        Disconnect();

    if(!pModel && !GetStyleSheet() && pNewPage && pNewPage->GetModel())
    {
        // #i119287# Set default StyleSheet for SdrGrafObj here, it is different from 'Default'. This
        // needs to be done before the style 'Default' is set from the :SetModel() call which is triggered
        // from the following :SetPage().
        // TTTT: Needs to be moved in branch aw080 due to having a SdrModel from the beginning, is at this
        // place for convenience currently (works in both versions, is not in the way)
        SfxStyleSheet* pSheet = pNewPage->GetModel()->GetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj();

        if(pSheet)
        {
            SetStyleSheet(pSheet, false);
        }
        else
        {
            SetMergedItem(XFillStyleItem(drawing::FillStyle_NONE));
            SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
        }
    }

    SdrRectObj::SetPage(pNewPage);

    if (bInsert && !mpImpl->mbConnected )
        Connect();
}

void SdrOle2Obj::SetObjRef( const css::uno::Reference < css::embed::XEmbeddedObject >& rNewObjRef )
{
    DBG_ASSERT( !rNewObjRef.is() || !mpImpl->mxObjRef.GetObject().is(), "SetObjRef called on already initialized object!");
    if( rNewObjRef == mpImpl->mxObjRef.GetObject() )
        return;

    // the caller of the method is responsible to control the old object, it will not be closed here
    // Otherwise WW8 import crashes because it transfers control to OLENode by this method
    if ( mpImpl->mxObjRef.GetObject().is() )
        mpImpl->mxObjRef.Lock( false );

    // avoid removal of object in Disconnect! It is definitely a HACK to call SetObjRef(0)!
    // This call will try to close the objects; so if anybody else wants to keep it, it must be locked by a CloseListener
    mpImpl->mxObjRef.Clear();

    if ( mpImpl->mbConnected )
        Disconnect();

    mpImpl->mxObjRef.Assign( rNewObjRef, GetAspect() );
    mpImpl->mbTypeAsked = false;

    if ( mpImpl->mxObjRef.is() )
    {
        DELETEZ(mpImpl->mpGraphic);

        if ( (mpImpl->mxObjRef->getStatus( GetAspect() ) & embed::EmbedMisc::EMBED_NEVERRESIZE ) )
            SetResizeProtect(true);

        // For math objects, set closed state to transparent
        SetClosedObj(!ImplIsMathObj( rNewObjRef ));

        Connect();
    }

    SetChanged();
    BroadcastObjectChange();
}

void SdrOle2Obj::SetClosedObj( bool bIsClosed )
{
    // TODO/LATER: do we still need this hack?
    // Allow changes to the closed state of OLE objects
    bClosedObj = bIsClosed;
}

SdrObject* SdrOle2Obj::getFullDragClone() const
{
    // #i118485# use central replacement generator
    return createSdrGrafObjReplacement(false, true);
}

void SdrOle2Obj::SetPersistName( const OUString& rPersistName )
{
    DBG_ASSERT( mpImpl->aPersistName.isEmpty(), "Persist name changed!");

    mpImpl->aPersistName = rPersistName;
    mpImpl->mbLoadingOLEObjectFailed = false;

    Connect();
    SetChanged();
}

void SdrOle2Obj::AbandonObject()
{
    mpImpl->aPersistName.clear();
    mpImpl->mbLoadingOLEObjectFailed = false;
    SetObjRef(nullptr);
}

const OUString& SdrOle2Obj::GetPersistName() const
{
    return mpImpl->aPersistName;
}

void SdrOle2Obj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    // #i118485# Allowing much more attributes for OLEs
    rInfo.bRotateFreeAllowed = true;
    rInfo.bRotate90Allowed = true;
    rInfo.bMirrorFreeAllowed = true;
    rInfo.bMirror45Allowed = true;
    rInfo.bMirror90Allowed = true;
    rInfo.bTransparenceAllowed = true;
    rInfo.bGradientAllowed = true;
    rInfo.bShearAllowed = true;
    rInfo.bEdgeRadiusAllowed = false;
    rInfo.bNoOrthoDesired = false;
    rInfo.bCanConvToPath = true;
    rInfo.bCanConvToPoly = true;
    rInfo.bCanConvToPathLineToArea = false;
    rInfo.bCanConvToPolyLineToArea = false;
    rInfo.bCanConvToContour = true;
}

sal_uInt16 SdrOle2Obj::GetObjIdentifier() const
{
    return mpImpl->mbFrame ? sal_uInt16(OBJ_FRAME) : sal_uInt16(OBJ_OLE2);
}

OUString SdrOle2Obj::TakeObjNameSingul() const
{
    OUStringBuffer sName(ImpGetResStr(mpImpl->mbFrame ? STR_ObjNameSingulFrame : STR_ObjNameSingulOLE2));

    const OUString aName(GetName());

    if (!aName.isEmpty())
    {
        sName.append(" '");
        sName.append(aName);
        sName.append('\'');
    }

    return sName.makeStringAndClear();
}

OUString SdrOle2Obj::TakeObjNamePlural() const
{
    return ImpGetResStr(mpImpl->mbFrame ? STR_ObjNamePluralFrame : STR_ObjNamePluralOLE2);
}

SdrOle2Obj* SdrOle2Obj::Clone() const
{
    return CloneHelper< SdrOle2Obj >();
}

SdrOle2Obj& SdrOle2Obj::assignFrom(const SdrOle2Obj& rObj)
{
    //TODO/LATER: who takes over control of my old object?!
    if( &rObj != this )
    {
        // ImpAssign( rObj );
        const SdrOle2Obj& rOle2Obj = static_cast< const SdrOle2Obj& >( rObj );

        uno::Reference<util::XCloseable> xClose(mpImpl->mxObjRef.GetObject(), uno::UNO_QUERY);

        if( pModel && mpImpl->mbConnected )
            Disconnect();

        SdrRectObj::operator=( rObj );

        // Manually copying bClosedObj attribute
        SetClosedObj( rObj.IsClosedObj() );

        mpImpl->aPersistName = rOle2Obj.mpImpl->aPersistName;
        mpImpl->maProgName = rOle2Obj.mpImpl->maProgName;
        mpImpl->mbFrame = rOle2Obj.mpImpl->mbFrame;

        if (rOle2Obj.mpImpl->mpGraphic)
        {
            if (mpImpl->mpGraphic)
            {
                delete mpImpl->mpGraphic;
                delete mpImpl->mpGraphicObject;
            }

            mpImpl->mpGraphic = new Graphic(*rOle2Obj.mpImpl->mpGraphic);
            mpImpl->mpGraphicObject = new GraphicObject(*mpImpl->mpGraphic);
        }

        if( pModel && rObj.GetModel() && !IsEmptyPresObj() )
        {
            ::comphelper::IEmbeddedHelper* pDestPers = pModel->GetPersist();
            ::comphelper::IEmbeddedHelper* pSrcPers = rObj.GetModel()->GetPersist();
            if( pDestPers && pSrcPers )
            {
                DBG_ASSERT( !mpImpl->mxObjRef.is(), "Object already existing!" );
                comphelper::EmbeddedObjectContainer& rContainer = pSrcPers->getEmbeddedObjectContainer();
                uno::Reference < embed::XEmbeddedObject > xObj = rContainer.GetEmbeddedObject( mpImpl->aPersistName );
                if ( xObj.is() )
                {
                    OUString aTmp;
                    mpImpl->mxObjRef.Assign( pDestPers->getEmbeddedObjectContainer().CopyAndGetEmbeddedObject(
                        rContainer, xObj, aTmp, pSrcPers->getDocumentBaseURL(), pDestPers->getDocumentBaseURL()), rOle2Obj.GetAspect());
                    mpImpl->mbTypeAsked = false;
                    mpImpl->aPersistName = aTmp;
                    CheckFileLink_Impl();
                }

                Connect();
            }
        }
    }
    return *this;
}

SdrOle2Obj& SdrOle2Obj::operator=(const SdrOle2Obj& rObj)
{
    return assignFrom(rObj);
}

void SdrOle2Obj::ImpSetVisAreaSize()
{
    // #i118524# do not again set VisAreaSize when the call comes from OLE client (e.g. ObjectAreaChanged)
    if (mpImpl->mbSuppressSetVisAreaSize)
        return;

    // currently there is no need to recalculate scaling for iconified objects
    // TODO/LATER: it might be needed in future when it is possible to change the icon
    if ( GetAspect() == embed::Aspects::MSOLE_ICON )
        return;

    // the object area of an embedded object was changed, e.g. by user interaction an a selected object
    GetObjRef();
    if (mpImpl->mxObjRef.is())
    {
        OSL_ASSERT( pModel );
        sal_Int64 nMiscStatus = mpImpl->mxObjRef->getStatus( GetAspect() );

        // the client is required to get access to scaling
        SfxInPlaceClient* pClient = SfxInPlaceClient::GetClient( dynamic_cast<SfxObjectShell*>(pModel->GetPersist()), mpImpl->mxObjRef.GetObject() );
        bool bHasOwnClient =
                        ( mpImpl->pLightClient
                        && mpImpl->mxObjRef->getClientSite() == uno::Reference< embed::XEmbeddedClient >( mpImpl->pLightClient ) );

        if ( pClient || bHasOwnClient )
        {
            // TODO: IMHO we need to do similar things when object is UIActive or OutplaceActive?!
            if ( ((nMiscStatus & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE) &&
                    svt::EmbeddedObjectRef::TryRunningState( mpImpl->mxObjRef.GetObject() ))
                    || mpImpl->mxObjRef->getCurrentState() == embed::EmbedStates::INPLACE_ACTIVE
                    )
            {
                Fraction aScaleWidth;
                Fraction aScaleHeight;
                if ( pClient )
                {
                    aScaleWidth = pClient->GetScaleWidth();
                    aScaleHeight = pClient->GetScaleHeight();
                }
                else
                {
                    aScaleWidth = mpImpl->pLightClient->GetScaleWidth();
                    aScaleHeight = mpImpl->pLightClient->GetScaleHeight();
                }

                // The object wants to resize itself (f.e. Chart wants to recalculate the layout)
                // or object is inplace active and so has a window that must be resized also
                // In these cases the change in the object area size will be reflected in a change of the
                // objects' visual area. The scaling will not change, but it might exist already and must
                // be used in calculations
                MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( mpImpl->mxObjRef->getMapUnit( GetAspect() ) );
                Size aVisSize( (long)( Fraction( maRect.GetWidth() ) / aScaleWidth ),
                                (long)( Fraction( maRect.GetHeight() ) / aScaleHeight ) );

                aVisSize = OutputDevice::LogicToLogic( aVisSize, pModel->GetScaleUnit(), aMapUnit);
                awt::Size aSz;
                aSz.Width = aVisSize.Width();
                aSz.Height = aVisSize.Height();
                mpImpl->mxObjRef->setVisualAreaSize( GetAspect(), aSz );

                try
                {
                    aSz = mpImpl->mxObjRef->getVisualAreaSize( GetAspect() );
                }
                catch( embed::NoVisualAreaSizeException& )
                {}

                Rectangle aAcceptedVisArea;
                aAcceptedVisArea.SetSize( Size( (long)( Fraction( long( aSz.Width ) ) * aScaleWidth ),
                                                (long)( Fraction( long( aSz.Height ) ) * aScaleHeight ) ) );
                if (aVisSize != aAcceptedVisArea.GetSize())
                {
                    // server changed VisArea to its liking and the VisArea is different than the suggested one
                    // store the new value as given by the object
                    MapUnit aNewMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( mpImpl->mxObjRef->getMapUnit( GetAspect() ) );
                    maRect.SetSize(OutputDevice::LogicToLogic( aAcceptedVisArea.GetSize(), aNewMapUnit, pModel->GetScaleUnit()));
                }

                // make the new object area known to the client
                // compared to the "else" branch aRect might have been changed by the object and no additional scaling was applied
                // WHY this -> OSL_ASSERT( pClient );
                if( pClient )
                    pClient->SetObjArea(maRect);

                // we need a new replacement image as the object has resized itself

                //#i79578# don't request a new replacement image for charts to often
                //a chart sends a modified call to the framework if it was changed
                //thus the replacement update is already handled there
                if( !IsChart() )
                    mpImpl->mxObjRef.UpdateReplacement();
            }
            else
            {
                // The object isn't active and does not want to resize itself so the changed object area size
                // will be reflected in a changed object scaling
                Fraction aScaleWidth;
                Fraction aScaleHeight;
                Size aObjAreaSize;
                if ( CalculateNewScaling( aScaleWidth, aScaleHeight, aObjAreaSize ) )
                {
                    if ( pClient )
                    {
                        Rectangle aScaleRect(maRect.TopLeft(), aObjAreaSize);
                        pClient->SetObjAreaAndScale( aScaleRect, aScaleWidth, aScaleHeight);
                    }
                    else
                    {
                        mpImpl->pLightClient->SetSizeScale( aScaleWidth, aScaleHeight );
                    }
                }
            }
        }
        else if( (nMiscStatus & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE) &&
            svt::EmbeddedObjectRef::TryRunningState( mpImpl->mxObjRef.GetObject() ) )
        {
            //also handle not sfx based ole objects e.g. charts
            //#i83860# resizing charts in impress distorts fonts
            uno::Reference< embed::XVisualObject > xVisualObject( this->getXModel(), uno::UNO_QUERY );
            if( xVisualObject.is() )
            {
                MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( mpImpl->mxObjRef->getMapUnit( GetAspect() ) );
                Point aTL( maRect.TopLeft() );
                Point aBR( maRect.BottomRight() );
                Point aTL2( OutputDevice::LogicToLogic( aTL, pModel->GetScaleUnit(), aMapUnit) );
                Point aBR2( OutputDevice::LogicToLogic( aBR, pModel->GetScaleUnit(), aMapUnit) );
                Rectangle aNewRect( aTL2, aBR2 );
                xVisualObject->setVisualAreaSize( GetAspect(), awt::Size( aNewRect.GetWidth(), aNewRect.GetHeight() ) );
            }
        }
    }
}

void SdrOle2Obj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    if( pModel && !pModel->isLocked() )
    {
        GetObjRef();
        if ( mpImpl->mxObjRef.is() && ( mpImpl->mxObjRef->getStatus( GetAspect() ) & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE ) )
        {
            // if the object needs recompose on resize
            // the client site should be created before the resize will take place
            // check whether there is no client site and create it if necessary
            AddOwnLightClient();
        }
    }

    SdrRectObj::NbcResize(rRef,xFact,yFact);
    if( pModel && !pModel->isLocked() )
        ImpSetVisAreaSize();
}

void SdrOle2Obj::SetGeoData(const SdrObjGeoData& rGeo)
{
    SdrRectObj::SetGeoData(rGeo);

    if( pModel && !pModel->isLocked() )
        ImpSetVisAreaSize();
}

void SdrOle2Obj::NbcSetSnapRect(const Rectangle& rRect)
{
    SdrRectObj::NbcSetSnapRect(rRect);

    if( pModel && !pModel->isLocked() )
        ImpSetVisAreaSize();

    if ( mpImpl->mxObjRef.is() && IsChart() )
    {
        //#i103460# charts do not necessarily have an own size within ODF files,
        //for this case they need to use the size settings from the surrounding frame,
        //which is made available with this method as there is no other way
        mpImpl->mxObjRef.SetDefaultSizeForChart( Size( rRect.GetWidth(), rRect.GetHeight() ) );
    }
}

void SdrOle2Obj::NbcSetLogicRect(const Rectangle& rRect)
{
    SdrRectObj::NbcSetLogicRect(rRect);

    if( pModel && !pModel->isLocked() )
        ImpSetVisAreaSize();
}

const Graphic* SdrOle2Obj::GetGraphic() const
{
    if ( mpImpl->mxObjRef.is() )
        return mpImpl->mxObjRef.GetGraphic();
    return mpImpl->mpGraphic;
}

void SdrOle2Obj::GetNewReplacement()
{
    if ( mpImpl->mxObjRef.is() )
        mpImpl->mxObjRef.UpdateReplacement();
}

Size SdrOle2Obj::GetOrigObjSize( MapMode* pTargetMapMode ) const
{
    return mpImpl->mxObjRef.GetSize( pTargetMapMode );
}

void SdrOle2Obj::setSuppressSetVisAreaSize( bool bNew )
{
    mpImpl->mbSuppressSetVisAreaSize = bNew;
}

void SdrOle2Obj::NbcMove(const Size& rSize)
{
    SdrRectObj::NbcMove(rSize);

    if( pModel && !pModel->isLocked() )
        ImpSetVisAreaSize();
}

bool SdrOle2Obj::CanUnloadRunningObj( const uno::Reference< embed::XEmbeddedObject >& xObj, sal_Int64 nAspect )
{
    uno::Reference<embed::XEmbedPersist2> xPersist(xObj, uno::UNO_QUERY);
    if (xPersist.is())
    {
        if (!xPersist->isStored())
            // It doesn't have persistent storage.  We can't unload this.
            return false;
    }

    bool bResult = false;

    sal_Int32 nState = xObj->getCurrentState();
    if ( nState == embed::EmbedStates::LOADED )
    {
        // the object is already unloaded
        bResult = true;
    }
    else
    {
        uno::Reference < util::XModifiable > xModifiable( xObj->getComponent(), uno::UNO_QUERY );
        if ( !xModifiable.is() )
            bResult = true;
        else
        {
            sal_Int64 nMiscStatus = xObj->getStatus( nAspect );

            if ( embed::EmbedMisc::MS_EMBED_ALWAYSRUN != ( nMiscStatus & embed::EmbedMisc::MS_EMBED_ALWAYSRUN ) &&
            embed::EmbedMisc::EMBED_ACTIVATEIMMEDIATELY != ( nMiscStatus & embed::EmbedMisc::EMBED_ACTIVATEIMMEDIATELY ) &&
            !( xModifiable.is() && xModifiable->isModified() ) &&
            !( nState == embed::EmbedStates::INPLACE_ACTIVE || nState == embed::EmbedStates::UI_ACTIVE || nState == embed::EmbedStates::ACTIVE ) )
            {
                bResult = true;
            }
        }
    }

    return bResult;
}

bool SdrOle2Obj::Unload( const uno::Reference< embed::XEmbeddedObject >& xObj, sal_Int64 nAspect )
{
    bool bResult = false;

    if ( CanUnloadRunningObj( xObj, nAspect ) )
    {
        try
        {
            xObj->changeState( embed::EmbedStates::LOADED );
            bResult = true;
        }
        catch( css::uno::Exception& )
        {
            OSL_FAIL(
                OString(OString("SdrOle2Obj::Unload=(), "
                        "exception caught: ") +
                OUStringToOString(
                    comphelper::anyToString( cppu::getCaughtException() ),
                    RTL_TEXTENCODING_UTF8 )).getStr() );
        }
    }

    return bResult;
}

bool SdrOle2Obj::Unload()
{
    if (!mpImpl->mxObjRef.is())
        // Already unloaded.
        return true;

    bool bUnloaded = false;
    if ( pModel && mpImpl->mxObjRef.is() )
    {
        bUnloaded = Unload( mpImpl->mxObjRef.GetObject(), GetAspect() );
    }

    return bUnloaded;
}

void SdrOle2Obj::GetObjRef_Impl()
{
    if ( !mpImpl->mxObjRef.is() && !mpImpl->aPersistName.isEmpty() && pModel && pModel->GetPersist() )
    {
        // Only try loading if it did not went wrong up to now
        if(!mpImpl->mbLoadingOLEObjectFailed)
        {
            mpImpl->mxObjRef.Assign( pModel->GetPersist()->getEmbeddedObjectContainer().GetEmbeddedObject( mpImpl->aPersistName ), GetAspect() );
            mpImpl->mbTypeAsked = false;
            CheckFileLink_Impl();

            // If loading of OLE object failed, remember that to not invoke a endless
            // loop trying to load it again and again.
            if( mpImpl->mxObjRef.is() )
            {
                mpImpl->mbLoadingOLEObjectFailed = true;
            }

            // For math objects, set closed state to transparent
            SetClosedObj(!ImplIsMathObj( mpImpl->mxObjRef.GetObject() ));
        }

        if ( mpImpl->mxObjRef.is() )
        {
            if( !IsEmptyPresObj() )
            {
                // remember modified status of model
                const bool bWasChanged = pModel && pModel->IsChanged();

                // perhaps preview not valid anymore
                // This line changes the modified state of the model
                SetGraphic_Impl( nullptr );

                // if status was not set before, force it back
                // to not set, so that SetGraphic(0L) above does not
                // set the modified state of the model.
                if(!bWasChanged && pModel && pModel->IsChanged())
                {
                    pModel->SetChanged( false );
                }
            }

            sal_Int64 nMiscStatus = mpImpl->mxObjRef->getStatus( GetAspect() );
            (void)nMiscStatus;
            //TODO/LATER: wait until ResizeOnPrinterChange is defined
            //if ( nMiscStatus & SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE )
            {
                if (pModel && pModel->GetRefDevice() &&
                    pModel->GetRefDevice()->GetOutDevType() == OUTDEV_PRINTER)
                {
                    if (!mpImpl->mbInDestruction)
                    {
                        //TODO/LATER: printerchange notification
                        /*
                        // prevent SetModified (don't want no update here)
                        bool bWasEnabled = (*ppObjRef)->IsEnableSetModified();
                        if ( bWasEnabled )
                            (*ppObjRef)->EnableSetModified( false );

                        // Kein RefDevice oder RefDevice kein Printer
                        Printer* pPrinter = (Printer*) pModel->GetRefDevice();
                        (*ppObjRef)->OnDocumentPrinterChanged( pPrinter );

                        // reset state
                        (*ppObjRef)->EnableSetModified( bWasEnabled );*/
                    }
                }
            }
        }

        if ( mpImpl->mxObjRef.is() )
            Connect();
    }

    if ( mpImpl->mbConnected )
        // move object to first position in cache
        GetSdrGlobalData().GetOLEObjCache().InsertObj(this);
}

uno::Reference < embed::XEmbeddedObject > SdrOle2Obj::GetObjRef() const
{
    const_cast<SdrOle2Obj*>(this)->GetObjRef_Impl();
    return mpImpl->mxObjRef.GetObject();
}

uno::Reference < embed::XEmbeddedObject > SdrOle2Obj::GetObjRef_NoInit() const
{
    return mpImpl->mxObjRef.GetObject();
}

uno::Reference< frame::XModel > SdrOle2Obj::getXModel() const
{
    GetObjRef();
    if ( svt::EmbeddedObjectRef::TryRunningState(mpImpl->mxObjRef.GetObject()) )
        return uno::Reference< frame::XModel >( mpImpl->mxObjRef->getComponent(), uno::UNO_QUERY );
    else
        return uno::Reference< frame::XModel >();
}

bool SdrOle2Obj::IsChart() const
{
    if (!mpImpl->mbTypeAsked)
    {
        mpImpl->mbIsChart = mpImpl->mxObjRef.IsChart();
        mpImpl->mbTypeAsked = true;
    }
    return mpImpl->mbIsChart;
}

bool SdrOle2Obj::IsReal3DChart() const
{
    if (!IsChart())
        return false;

    uno::Reference<chart2::XChartDocument> xChart2Document(getXModel(), uno::UNO_QUERY);
    uno::Reference<chart2::XDiagram> xChart2Diagram(xChart2Document->getFirstDiagram(), uno::UNO_QUERY);

    if (!xChart2Diagram.is())
        return false;

    return ChartHelper::isGL3DDiagram(xChart2Diagram);
}

void SdrOle2Obj::SetGraphicToObj( const Graphic& aGraphic, const OUString& aMediaType )
{
    mpImpl->mxObjRef.SetGraphic( aGraphic, aMediaType );
}

void SdrOle2Obj::SetGraphicToObj( const uno::Reference< io::XInputStream >& xGrStream, const OUString& aMediaType )
{
    mpImpl->mxObjRef.SetGraphicStream( xGrStream, aMediaType );
}

bool SdrOle2Obj::IsCalc() const
{
    if ( !mpImpl->mxObjRef.is() )
        return false;

    SvGlobalName aObjClsId( mpImpl->mxObjRef->getClassID() );
    if(    SvGlobalName(SO3_SC_CLASSID_30) == aObjClsId
        || SvGlobalName(SO3_SC_CLASSID_40) == aObjClsId
        || SvGlobalName(SO3_SC_CLASSID_50) == aObjClsId
        || SvGlobalName(SO3_SC_CLASSID_60) == aObjClsId
        || SvGlobalName(SO3_SC_OLE_EMBED_CLASSID_60) == aObjClsId
        || SvGlobalName(SO3_SC_OLE_EMBED_CLASSID_8) == aObjClsId
        || SvGlobalName(SO3_SC_CLASSID) == aObjClsId )
    {
        return true;
    }

    return false;
}

uno::Reference< frame::XModel > SdrOle2Obj::GetParentXModel() const
{
    uno::Reference< frame::XModel > xDoc;
    if ( pModel )
        xDoc.set( pModel->getUnoModel(),uno::UNO_QUERY);
    return xDoc;
}

bool SdrOle2Obj::CalculateNewScaling( Fraction& aScaleWidth, Fraction& aScaleHeight, Size& aObjAreaSize )
{
    // TODO/LEAN: to avoid rounding errors scaling always uses the VisArea.
    // If we don't cache it for own objects also we must load the object here
    if ( !mpImpl->mxObjRef.is() || !pModel )
        return false;

    MapMode aMapMode( pModel->GetScaleUnit() );
    aObjAreaSize = mpImpl->mxObjRef.GetSize( &aMapMode );

    Size aSize = maRect.GetSize();
    aScaleWidth = Fraction(aSize.Width(),  aObjAreaSize.Width() );
    aScaleHeight = Fraction(aSize.Height(), aObjAreaSize.Height() );

    // reduce to 10 binary digits
    aScaleHeight.ReduceInaccurate(10);
    aScaleWidth.ReduceInaccurate(10);

    return true;
}

bool SdrOle2Obj::AddOwnLightClient()
{
    // The Own Light Client must be registered in object only using this method!
    if ( !SfxInPlaceClient::GetClient( dynamic_cast<SfxObjectShell*>(pModel->GetPersist()), mpImpl->mxObjRef.GetObject() )
      && !( mpImpl->pLightClient && mpImpl->mxObjRef->getClientSite() == uno::Reference< embed::XEmbeddedClient >( mpImpl->pLightClient ) ) )
    {
        Connect();

        if ( mpImpl->mxObjRef.is() && mpImpl->pLightClient )
        {
            Fraction aScaleWidth;
            Fraction aScaleHeight;
            Size aObjAreaSize;
            if ( CalculateNewScaling( aScaleWidth, aScaleHeight, aObjAreaSize ) )
            {
                mpImpl->pLightClient->SetSizeScale( aScaleWidth, aScaleHeight );
                try {
                    mpImpl->mxObjRef->setClientSite( mpImpl->pLightClient );
                    return true;
                } catch( uno::Exception& )
                {}
            }

        }

        return false;
    }

    return true;
}

Graphic SdrOle2Obj::GetEmptyOLEReplacementGraphic()
{
    return Graphic(BitmapEx(ResId(BMP_SVXOLEOBJ, *ImpGetResMgr())));
}

void SdrOle2Obj::SetWindow(const css::uno::Reference < css::awt::XWindow >& _xWindow)
{
    if ( mpImpl->mxObjRef.is() && mpImpl->pLightClient )
    {
        mpImpl->pLightClient->setWindow(_xWindow);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
