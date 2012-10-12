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


#include <svx/svdoole2.hxx>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XInplaceClient.hpp>
#include <com/sun/star/embed/XInplaceObject.hpp>
#include <com/sun/star/embed/XLinkageSupport.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/embed/XWindowSupplier.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/container/XChild.hpp>
#include "com/sun/star/document/XStorageBasedDocument.hpp"

#include <comphelper/processfactory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/convert.hxx>

#include <svtools/filter.hxx>
#include <svtools/embedhlp.hxx>

#include <sfx2/objsh.hxx>
#include <sfx2/ipclient.hxx>
#include <sfx2/lnkbase.hxx>
#include <tools/stream.hxx>
#include <comphelper/anytostring.hxx>
#include <svx/svdpagv.hxx>
#include <tools/globname.hxx>
#include <vcl/jobset.hxx>
#include <sot/clsids.hxx>

#include <sot/formats.hxx>
#include <sfx2/linkmgr.hxx>
#include <svtools/transfer.hxx>
#include <cppuhelper/implbase5.hxx>

#include <svl/solar.hrc>
#include <svl/urihelper.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include <svx/svdmodel.hxx>
#include "svx/svdglob.hxx"  // Stringcache
#include "svx/svdstr.hrc"   // the object's name
#include <svx/svdetc.hxx>
#include <svx/svdview.hxx>
#include "unomlstr.hxx"
#include <svtools/chartprettypainter.hxx>
#include <svx/sdr/contact/viewcontactofsdrole2obj.hxx>
#include <svx/svdograf.hxx>
#include <svx/sdr/properties/oleproperties.hxx>

// #i100710#
#include <svx/xlnclit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xflbstit.hxx>

using namespace ::rtl;
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

class SdrLightEmbeddedClient_Impl : public ::cppu::WeakImplHelper5
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
    SdrLightEmbeddedClient_Impl( SdrOle2Obj* pObj );
    void Release();

    void SetSizeScale( const Fraction& aScaleWidth, const Fraction& aScaleHeight )
    {
        m_aScaleWidth = aScaleWidth;
        m_aScaleHeight = aScaleHeight;
    }

    Fraction GetScaleWidth() const { return m_aScaleWidth; }
    Fraction GetScaleHeight() const { return m_aScaleHeight; }

    void setWindow(const uno::Reference< awt::XWindow >& _xWindow);

private:
    Rectangle impl_getScaledRect_nothrow() const;
    // XStateChangeListener
    virtual void SAL_CALL changingState( const ::com::sun::star::lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (::com::sun::star::embed::WrongStateException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL stateChanged( const ::com::sun::star::lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

    // document::XEventListener
    virtual void SAL_CALL       notifyEvent( const document::EventObject& aEvent ) throw( uno::RuntimeException );

    // XEmbeddedClient
    virtual void SAL_CALL saveObject() throw ( embed::ObjectSaveVetoException, uno::Exception, uno::RuntimeException );
    virtual void SAL_CALL visibilityChanged( sal_Bool bVisible ) throw ( embed::WrongStateException, uno::RuntimeException );

    // XComponentSupplier
    virtual uno::Reference< util::XCloseable > SAL_CALL getComponent() throw ( uno::RuntimeException );

    // XInplaceClient
    virtual sal_Bool SAL_CALL canInplaceActivate() throw ( uno::RuntimeException );
    virtual void SAL_CALL activatingInplace() throw ( embed::WrongStateException, uno::RuntimeException );
    virtual void SAL_CALL activatingUI() throw ( embed::WrongStateException, uno::RuntimeException );
    virtual void SAL_CALL deactivatedInplace() throw ( embed::WrongStateException, uno::RuntimeException );
    virtual void SAL_CALL deactivatedUI() throw ( embed::WrongStateException, uno::RuntimeException );
    virtual uno::Reference< ::com::sun::star::frame::XLayoutManager > SAL_CALL getLayoutManager() throw ( embed::WrongStateException, uno::RuntimeException );
    virtual uno::Reference< frame::XDispatchProvider > SAL_CALL getInplaceDispatchProvider() throw ( embed::WrongStateException, uno::RuntimeException );
    virtual awt::Rectangle SAL_CALL getPlacement() throw ( embed::WrongStateException, uno::RuntimeException );
    virtual awt::Rectangle SAL_CALL getClipRectangle() throw ( embed::WrongStateException, uno::RuntimeException );
    virtual void SAL_CALL translateAccelerators( const uno::Sequence< awt::KeyEvent >& aKeys ) throw ( embed::WrongStateException, uno::RuntimeException );
    virtual void SAL_CALL scrollObject( const awt::Size& aOffset ) throw ( embed::WrongStateException, uno::RuntimeException );
    virtual void SAL_CALL changedPlacement( const awt::Rectangle& aPosRect ) throw ( embed::WrongStateException, uno::Exception, uno::RuntimeException );

    // XWindowSupplier
    virtual uno::Reference< awt::XWindow > SAL_CALL getWindow() throw ( uno::RuntimeException );
};

//--------------------------------------------------------------------
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
//--------------------------------------------------------------------
void SAL_CALL SdrLightEmbeddedClient_Impl::changingState( const ::com::sun::star::lang::EventObject& /*aEvent*/, ::sal_Int32 /*nOldState*/, ::sal_Int32 /*nNewState*/ ) throw (::com::sun::star::embed::WrongStateException, ::com::sun::star::uno::RuntimeException)
{
}

//--------------------------------------------------------------------
void SdrLightEmbeddedClient_Impl::Release()
{
    {
        SolarMutexGuard aGuard;
        mpObj = NULL;
    }

    release();
}

//--------------------------------------------------------------------
void SAL_CALL SdrLightEmbeddedClient_Impl::stateChanged( const ::com::sun::star::lang::EventObject& /*aEvent*/, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (::com::sun::star::uno::RuntimeException)
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

//--------------------------------------------------------------------
void SAL_CALL SdrLightEmbeddedClient_Impl::disposing( const ::com::sun::star::lang::EventObject& /*aEvent*/ ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    GetSdrGlobalData().GetOLEObjCache().RemoveObj(mpObj);
}

//--------------------------------------------------------------------
void SAL_CALL SdrLightEmbeddedClient_Impl::notifyEvent( const document::EventObject& aEvent ) throw( uno::RuntimeException )
{
    // TODO/LATER: when writer uses this implementation the code could be shared with SfxInPlaceClient_Impl

    SolarMutexGuard aGuard;

    // the code currently makes sense only in case there is no other client
    if ( mpObj && mpObj->GetAspect() != embed::Aspects::MSOLE_ICON && aEvent.EventName == "OnVisAreaChanged"
      && mpObj->GetObjRef().is() && mpObj->GetObjRef()->getClientSite() == uno::Reference< embed::XEmbeddedClient >( this ) )
    {
        try
        {
            MapUnit aContainerMapUnit( MAP_100TH_MM );
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

//--------------------------------------------------------------------
void SAL_CALL SdrLightEmbeddedClient_Impl::saveObject()
    throw ( embed::ObjectSaveVetoException,
            uno::Exception,
            uno::RuntimeException )
{
    // TODO/LATER: when writer uses this implementation the code could be shared with SfxInPlaceClient_Impl
    uno::Reference< embed::XCommonEmbedPersist > xPersist;
    uno::Reference< util::XModifiable > xModifiable;

    {
        SolarMutexGuard aGuard;

        if ( !mpObj )
            throw embed::ObjectSaveVetoException();

        // the common persistence is supported by objects and links
        xPersist = uno::Reference< embed::XCommonEmbedPersist >( mpObj->GetObjRef(), uno::UNO_QUERY_THROW );
        xModifiable = uno::Reference< util::XModifiable >( mpObj->GetParentXModel(), uno::UNO_QUERY );
    }

    xPersist->storeOwn();

    if ( xModifiable.is() )
        xModifiable->setModified( sal_True );
}

//--------------------------------------------------------------------
void SAL_CALL SdrLightEmbeddedClient_Impl::visibilityChanged( sal_Bool /*bVisible*/ )
    throw ( embed::WrongStateException,
            uno::RuntimeException )
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

//--------------------------------------------------------------------
uno::Reference< util::XCloseable > SAL_CALL SdrLightEmbeddedClient_Impl::getComponent()
    throw ( uno::RuntimeException )
{
    uno::Reference< util::XCloseable > xResult;

    SolarMutexGuard aGuard;
    if ( mpObj )
        xResult = uno::Reference< util::XCloseable >( mpObj->GetParentXModel(), uno::UNO_QUERY );

    return xResult;
}
// XInplaceClient
//--------------------------------------------------------------------
sal_Bool SAL_CALL SdrLightEmbeddedClient_Impl::canInplaceActivate()
    throw ( uno::RuntimeException )
{
    sal_Bool bRet = sal_False;
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

//--------------------------------------------------------------------
void SAL_CALL SdrLightEmbeddedClient_Impl::activatingInplace()
    throw ( embed::WrongStateException,
            uno::RuntimeException )
{
}

//--------------------------------------------------------------------
void SAL_CALL SdrLightEmbeddedClient_Impl::activatingUI()
    throw ( embed::WrongStateException,
            uno::RuntimeException )
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
                    if ( xObject->getStatus( pObj->GetAspect() ) & embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE )
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
                catch (com::sun::star::uno::Exception& )
                {}
            }
        }
    } // for(sal_Int32 i = nCount-1 ; i >= 0;--i)
}

//--------------------------------------------------------------------
void SAL_CALL SdrLightEmbeddedClient_Impl::deactivatedInplace()
    throw ( embed::WrongStateException,
            uno::RuntimeException )
{
}

//--------------------------------------------------------------------
void SAL_CALL SdrLightEmbeddedClient_Impl::deactivatedUI()
    throw ( embed::WrongStateException,
            uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager(getLayoutManager());
    if ( xLayoutManager.is() )
    {
        const static rtl::OUString aMenuBarURL(  "private:resource/menubar/menubar" );
        if ( !xLayoutManager->isElementVisible( aMenuBarURL ) )
            xLayoutManager->createElement( aMenuBarURL );
    }
}

//--------------------------------------------------------------------
uno::Reference< ::com::sun::star::frame::XLayoutManager > SAL_CALL SdrLightEmbeddedClient_Impl::getLayoutManager()
    throw ( embed::WrongStateException,
            uno::RuntimeException )
{
    uno::Reference< ::com::sun::star::frame::XLayoutManager > xMan;
    SolarMutexGuard aGuard;
    uno::Reference < beans::XPropertySet > xFrame( lcl_getFrame_throw(mpObj));
    try
    {
        xMan.set(xFrame->getPropertyValue( ::rtl::OUString("LayoutManager") ),uno::UNO_QUERY);
    }
    catch ( uno::Exception& )
    {
        throw uno::RuntimeException();
    }

    return xMan;
}

//--------------------------------------------------------------------
uno::Reference< frame::XDispatchProvider > SAL_CALL SdrLightEmbeddedClient_Impl::getInplaceDispatchProvider()
    throw ( embed::WrongStateException,
            uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return uno::Reference < frame::XDispatchProvider >( lcl_getFrame_throw(mpObj), uno::UNO_QUERY_THROW );
}

//--------------------------------------------------------------------
awt::Rectangle SAL_CALL SdrLightEmbeddedClient_Impl::getPlacement()
    throw ( embed::WrongStateException,
            uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if ( !mpObj )
        throw uno::RuntimeException();

    Rectangle aLogicRect = impl_getScaledRect_nothrow();
    MapUnit aContainerMapUnit( MAP_100TH_MM );
    uno::Reference< embed::XVisualObject > xParentVis( mpObj->GetParentXModel(), uno::UNO_QUERY );
    if ( xParentVis.is() )
        aContainerMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xParentVis->getMapUnit( mpObj->GetAspect() ) );

    aLogicRect = Application::GetDefaultDevice()->LogicToPixel(aLogicRect,aContainerMapUnit);
    return AWTRectangle( aLogicRect );
}

//--------------------------------------------------------------------
awt::Rectangle SAL_CALL SdrLightEmbeddedClient_Impl::getClipRectangle()
    throw ( embed::WrongStateException,
            uno::RuntimeException )
{
    return getPlacement();
}

//--------------------------------------------------------------------
void SAL_CALL SdrLightEmbeddedClient_Impl::translateAccelerators( const uno::Sequence< awt::KeyEvent >& /*aKeys*/ )
    throw ( embed::WrongStateException,
            uno::RuntimeException )
{
}

//--------------------------------------------------------------------
void SAL_CALL SdrLightEmbeddedClient_Impl::scrollObject( const awt::Size& /*aOffset*/ )
    throw ( embed::WrongStateException,
            uno::RuntimeException )
{
}

//--------------------------------------------------------------------
void SAL_CALL SdrLightEmbeddedClient_Impl::changedPlacement( const awt::Rectangle& aPosRect )
    throw ( embed::WrongStateException,
            uno::Exception,
            uno::RuntimeException )
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
    MapUnit aContainerMapUnit( MAP_100TH_MM );
    uno::Reference< embed::XVisualObject > xParentVis( mpObj->GetParentXModel(), uno::UNO_QUERY );
    if ( xParentVis.is() )
        aContainerMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xParentVis->getMapUnit( mpObj->GetAspect() ) );

    Rectangle aNewLogicRect = Application::GetDefaultDevice()->PixelToLogic(aNewPixelRect,aContainerMapUnit);
    Rectangle aLogicRect = impl_getScaledRect_nothrow();

    if ( aNewLogicRect != aLogicRect )
    {
        // the calculation of the object area has not changed the object size
        // it should be done here then
        //SfxBooleanFlagGuard aGuard( m_bResizeNoScale, sal_True );

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
//--------------------------------------------------------------------
uno::Reference< awt::XWindow > SAL_CALL SdrLightEmbeddedClient_Impl::getWindow()
    throw ( uno::RuntimeException )
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

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrEmbedObjectLink : public sfx2::SvBaseLink
{
    SdrOle2Obj*         pObj;

public:
                        SdrEmbedObjectLink(SdrOle2Obj* pObj);
    virtual             ~SdrEmbedObjectLink();

    virtual void        Closed();
    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const String& rMimeType, const ::com::sun::star::uno::Any & rValue );

    sal_Bool            Connect() { return GetRealObject() != NULL; }
};

// -----------------------------------------------------------------------------

SdrEmbedObjectLink::SdrEmbedObjectLink(SdrOle2Obj* pObject):
    ::sfx2::SvBaseLink( ::sfx2::LINKUPDATE_ONCALL, SOT_FORMATSTR_ID_SVXB ),
    pObj(pObject)
{
    SetSynchron( sal_False );
}

// -----------------------------------------------------------------------------

SdrEmbedObjectLink::~SdrEmbedObjectLink()
{
}

// -----------------------------------------------------------------------------

::sfx2::SvBaseLink::UpdateResult SdrEmbedObjectLink::DataChanged(
    const String& /*rMimeType*/, const ::com::sun::star::uno::Any & /*rValue*/ )
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

// -----------------------------------------------------------------------------

void SdrEmbedObjectLink::Closed()
{
    pObj->BreakFileLink_Impl();
    SvBaseLink::Closed();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrOle2ObjImpl
{
public:
    // TODO/LATER: do we really need this pointer?
    GraphicObject*  pGraphicObject;
    String          aPersistName;       // name of object in persist
    SdrLightEmbeddedClient_Impl* pLightClient; // must be registered as client only using AddOwnLightClient() call

    // New local var to avoid repeated loading if load of OLE2 fails
    sal_Bool        mbLoadingOLEObjectFailed;
    sal_Bool        mbConnected;

    SdrEmbedObjectLink* mpObjectLink;
    String maLinkURL;

    SdrOle2ObjImpl()
    : pGraphicObject( NULL )
    , pLightClient ( NULL )
    // init to start situation, loading did not fail
    , mbLoadingOLEObjectFailed( sal_False )
    , mbConnected( sal_False )
    , mpObjectLink( NULL )
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////
// BaseProperties section

sdr::properties::BaseProperties* SdrOle2Obj::CreateObjectSpecificProperties()
{
    return new sdr::properties::OleProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* SdrOle2Obj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrOle2Obj(*this);
}

// -----------------------------------------------------------------------------

TYPEINIT1(SdrOle2Obj,SdrRectObj);
DBG_NAME(SdrOle2Obj)
SdrOle2Obj::SdrOle2Obj(bool bFrame_) : m_bTypeAsked(false)
,m_bChart(false)
{
    DBG_CTOR( SdrOle2Obj,NULL);
    bInDestruction = false;
    Init();
    bFrame=bFrame_;
}

// -----------------------------------------------------------------------------

SdrOle2Obj::SdrOle2Obj( const svt::EmbeddedObjectRef&  rNewObjRef, const XubString& rNewObjName, const Rectangle& rNewRect, bool bFrame_)
    : SdrRectObj(rNewRect)
    , xObjRef( rNewObjRef )
    , m_bTypeAsked(false)
    , m_bChart(false)
{
    DBG_CTOR( SdrOle2Obj,NULL);
    bInDestruction = false;
    Init();

    mpImpl->aPersistName = rNewObjName;
    bFrame=bFrame_;

    if ( xObjRef.is() && (xObjRef->getStatus( GetAspect() ) & embed::EmbedMisc::EMBED_NEVERRESIZE ) )
        SetResizeProtect(sal_True);

    // For math objects, set closed state to transparent
    if( ImplIsMathObj( xObjRef.GetObject() ) )
        SetClosedObj( false );
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::Init()
{
    mpImpl = new SdrOle2ObjImpl;
    pModifyListener = NULL;
    pGraphic=NULL;
    mpImpl->pGraphicObject=NULL;
    mpImpl->pLightClient = 0;

    xObjRef.Lock( sal_True );
}

// -----------------------------------------------------------------------------

SdrOle2Obj::~SdrOle2Obj()
{
    DBG_DTOR( SdrOle2Obj,NULL);
    bInDestruction = true;

    if ( mpImpl->mbConnected )
        Disconnect();

    if( pGraphic!=NULL )
        delete pGraphic;

    if(mpImpl->pGraphicObject!=NULL)
        delete mpImpl->pGraphicObject;

    if(pModifyListener)
    {
        pModifyListener->invalidate();
        pModifyListener->release();
    }

    DisconnectFileLink_Impl();

    if ( mpImpl->pLightClient )
    {
        mpImpl->pLightClient->Release();
        mpImpl->pLightClient = NULL;
    }

    delete mpImpl;
}

// -----------------------------------------------------------------------------
void SdrOle2Obj::SetAspect( sal_Int64 nAspect )
{
    xObjRef.SetViewAspect( nAspect );
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::SetGraphic_Impl(const Graphic* pGrf)
{
    if ( pGraphic )
    {
        delete pGraphic;
        pGraphic = NULL;
        delete mpImpl->pGraphicObject;
        mpImpl->pGraphicObject = NULL;
    }

    if (pGrf!=NULL)
    {
        pGraphic = new Graphic(*pGrf);
        mpImpl->pGraphicObject = new GraphicObject( *pGraphic );
    }

    SetChanged();
    BroadcastObjectChange();
}

void SdrOle2Obj::SetGraphic(const Graphic* pGrf)
{
    // only for setting a preview graphic
    SetGraphic_Impl( pGrf );
}

// -----------------------------------------------------------------------------

bool SdrOle2Obj::IsEmpty() const
{
    return !(xObjRef.is());
}

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

sal_Bool SdrOle2Obj::UpdateLinkURL_Impl()
{
    sal_Bool bResult = sal_False;

    if ( mpImpl->mpObjectLink )
    {
        sfx2::LinkManager* pLinkManager = pModel ? pModel->GetLinkManager() : NULL;
        if ( pLinkManager )
        {
            String aNewLinkURL;
            pLinkManager->GetDisplayNames( mpImpl->mpObjectLink, 0, &aNewLinkURL, 0, 0 );
            if ( !aNewLinkURL.EqualsIgnoreCaseAscii( mpImpl->maLinkURL ) )
            {
                const_cast<SdrOle2Obj*>(this)->GetObjRef_Impl();
                uno::Reference< embed::XCommonEmbedPersist > xPersObj( xObjRef.GetObject(), uno::UNO_QUERY );
                OSL_ENSURE( xPersObj.is(), "The object must exist!\n" );
                if ( xPersObj.is() )
                {
                    try
                    {
                        sal_Int32 nCurState = xObjRef->getCurrentState();
                        if ( nCurState != embed::EmbedStates::LOADED )
                            xObjRef->changeState( embed::EmbedStates::LOADED );

                        // TODO/LATER: there should be possible to get current mediadescriptor settings from the object
                        uno::Sequence< beans::PropertyValue > aArgs( 1 );
                        aArgs[0].Name = ::rtl::OUString(  "URL"  );
                        aArgs[0].Value <<= ::rtl::OUString( aNewLinkURL );
                        xPersObj->reload( aArgs, uno::Sequence< beans::PropertyValue >() );

                        mpImpl->maLinkURL = aNewLinkURL;
                        bResult = sal_True;

                        if ( nCurState != embed::EmbedStates::LOADED )
                            xObjRef->changeState( nCurState );
                    }
                    catch( ::com::sun::star::uno::Exception& )
                    {
                        OSL_FAIL(
                            (OString("SdrOle2Obj::UpdateLinkURL_Impl(), "
                                    "exception caught: ") +
                            rtl::OUStringToOString(
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

// -----------------------------------------------------------------------------

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
                uno::Reference< embed::XLinkageSupport > xLinkSupport( xObjRef.GetObject(), uno::UNO_QUERY_THROW );
                xLinkSupport->breakLink( xStorage, mpImpl->aPersistName );
                DisconnectFileLink_Impl();
                mpImpl->maLinkURL = String();
            }
            catch( ::com::sun::star::uno::Exception& )
            {
                OSL_FAIL(
                    (OString("SdrOle2Obj::BreakFileLink_Impl(), "
                            "exception caught: ") +
                    rtl::OUStringToOString(
                        comphelper::anyToString( cppu::getCaughtException() ),
                        RTL_TEXTENCODING_UTF8 )).getStr() );
            }
        }
    }
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::DisconnectFileLink_Impl()
{
    sfx2::LinkManager* pLinkManager = pModel ? pModel->GetLinkManager() : NULL;
    if ( pLinkManager && mpImpl->mpObjectLink )
    {
        pLinkManager->Remove( mpImpl->mpObjectLink );
        mpImpl->mpObjectLink = NULL;
    }
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::CheckFileLink_Impl()
{
    if ( pModel && xObjRef.GetObject().is() && !mpImpl->mpObjectLink )
    {
        try
        {
            uno::Reference< embed::XLinkageSupport > xLinkSupport( xObjRef.GetObject(), uno::UNO_QUERY );
            if ( xLinkSupport.is() && xLinkSupport->isLink() )
            {
                String aLinkURL = xLinkSupport->getLinkURL();
                if ( aLinkURL.Len() )
                {
                    // this is a file link so the model link manager should handle it
                    sfx2::LinkManager* pLinkManager = pModel->GetLinkManager();
                    if ( pLinkManager )
                    {
                        mpImpl->mpObjectLink = new SdrEmbedObjectLink( this );
                        mpImpl->maLinkURL = aLinkURL;
                        pLinkManager->InsertFileLink( *mpImpl->mpObjectLink, OBJECT_CLIENT_OLE, aLinkURL, NULL, NULL );
                        mpImpl->mpObjectLink->Connect();
                    }
                }
            }
        }
        catch( ::com::sun::star::uno::Exception& )
        {
            OSL_FAIL(
                (OString("SdrOle2Obj::CheckFileLink_Impl(), "
                        "exception caught: ") +
                rtl::OUStringToOString(
                    comphelper::anyToString( cppu::getCaughtException() ),
                    RTL_TEXTENCODING_UTF8 )).getStr() );
        }
    }
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::Reconnect_Impl()
{
    DBG_ASSERT( mpImpl->mbConnected, "Assigned unconnected object?!" );
    Connect_Impl();
}

void SdrOle2Obj::Connect_Impl()
{
    if( pModel && mpImpl->aPersistName.Len() )
    {
        try
        {
            ::comphelper::IEmbeddedHelper* pPers = pModel->GetPersist();
            if ( pPers )
            {
                comphelper::EmbeddedObjectContainer& rContainer = pPers->getEmbeddedObjectContainer();
                if ( !rContainer.HasEmbeddedObject( mpImpl->aPersistName )
                  || ( xObjRef.is() && !rContainer.HasEmbeddedObject( xObjRef.GetObject() ) ) )
                {
                    // object not known to container document
                    // No object -> disaster!
                    DBG_ASSERT( xObjRef.is(), "No object in connect!");
                    if ( xObjRef.is() )
                    {
                        // object came from the outside, now add it to the container
                        ::rtl::OUString aTmp;
                        rContainer.InsertEmbeddedObject( xObjRef.GetObject(), aTmp );
                        mpImpl->aPersistName = aTmp;
                    }
                }
                else if ( !xObjRef.is() )
                {
                    xObjRef.Assign( rContainer.GetEmbeddedObject( mpImpl->aPersistName ), xObjRef.GetViewAspect() );
                    m_bTypeAsked = false;
                }

                if ( xObjRef.GetObject().is() )
                {
                    xObjRef.AssignToContainer( &rContainer, mpImpl->aPersistName );
                    mpImpl->mbConnected = true;
                    xObjRef.Lock( sal_True );
                }
            }

            if ( xObjRef.is() )
            {
                if ( !mpImpl->pLightClient )
                {
                    mpImpl->pLightClient = new SdrLightEmbeddedClient_Impl( this );
                    mpImpl->pLightClient->acquire();
                }

                xObjRef->addStateChangeListener( mpImpl->pLightClient );
                xObjRef->addEventListener( uno::Reference< document::XEventListener >( mpImpl->pLightClient ) );

                if ( xObjRef->getCurrentState() != embed::EmbedStates::LOADED )
                    GetSdrGlobalData().GetOLEObjCache().InsertObj(this);

                CheckFileLink_Impl();

                uno::Reference< container::XChild > xChild( xObjRef.GetObject(), uno::UNO_QUERY );
                if( xChild.is() )
                {
                    uno::Reference< uno::XInterface > xParent( pModel->getUnoModel());
                    if( xParent.is())
                        xChild->setParent( pModel->getUnoModel() );
                }

            }
        }
        catch( ::com::sun::star::uno::Exception& )
        {
            OSL_FAIL(
                (OString("SdrOle2Obj::Connect_Impl(), "
                        "exception caught: ") +
                rtl::OUStringToOString(
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
            sal_Bool bModified = (*ppObjRef)->IsModified();
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
    if( xObjRef.is() && xObjRef->getCurrentState() != embed::EmbedStates::LOADED )
    {
        // register modify listener
        if( !pModifyListener )
        {
            ((SdrOle2Obj*)this)->pModifyListener = new SvxUnoShapeModifyListener( (SdrOle2Obj*)this );
            pModifyListener->acquire();
        }

        uno::Reference< util::XModifyBroadcaster > xBC( getXModel(), uno::UNO_QUERY );
        if( xBC.is() && pModifyListener )
        {
            uno::Reference< util::XModifyListener > xListener( pModifyListener );
            xBC->addModifyListener( xListener );
        }
    }
}

// -----------------------------------------------------------------------------

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
    if( xObjRef.is() && mpImpl->aPersistName.Len() )
    {
        try
        {
            sal_Int32 nState = xObjRef->getCurrentState();
            if ( nState != embed::EmbedStates::LOADED )
            {
                uno::Reference< util::XModifyBroadcaster > xBC( getXModel(), uno::UNO_QUERY );
                if( xBC.is() && pModifyListener )
                {
                    uno::Reference< util::XModifyListener > xListener( pModifyListener );
                    xBC->removeModifyListener( xListener );
                }
            }
        }
        catch( ::com::sun::star::uno::Exception& )
        {
            OSL_FAIL(
                (OString("SdrOle2Obj::RemoveListeners_Impl(), "
                        "exception caught: ") +
                rtl::OUStringToOString(
                    comphelper::anyToString( cppu::getCaughtException() ),
                    RTL_TEXTENCODING_UTF8 )).getStr() );
        }
    }
}

void SdrOle2Obj::Disconnect_Impl()
{
    try
    {
        if ( pModel && mpImpl->aPersistName.Len() )
        {
            if( pModel->IsInDestruction() )
            {
                // TODO/LATER: here we must assume that the destruction of the model is enough to make clear that we will not
                // remove the object from the container, even if the DrawingObject itself is not destroyed (unfortunately this
                // There is no real need to do the following removing of the object from the container
                // in case the model has correct persistence, but in case of problems such a removing
                // would make the behavior of the office more stable

                comphelper::EmbeddedObjectContainer* pContainer = xObjRef.GetContainer();
                if ( pContainer )
                {
                    pContainer->CloseEmbeddedObject( xObjRef.GetObject() );
                    xObjRef.AssignToContainer( NULL, mpImpl->aPersistName );
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
                        xClose->close( sal_True );
                    }
                    catch ( util::CloseVetoException& )
                    {
                        // there's still someone who needs the object!
                    }
                }

                xObjRef = NULL;*/
            }
            else if ( xObjRef.is() )
            {
                if ( pModel->getUnoModel().is() )
                {
                    // remove object, but don't close it (that's up to someone else)
                    comphelper::EmbeddedObjectContainer* pContainer = xObjRef.GetContainer();
                    if ( pContainer )
                    {
                        pContainer->RemoveEmbeddedObject( xObjRef.GetObject(), sal_False);

                        // TODO/LATER: mpImpl->aPersistName contains outdated information, to keep it updated
                        // it should be returned from RemoveEmbeddedObject call. Currently it is no problem,
                        // since no container is adjusted, actually the empty string could be provided as a name here
                        xObjRef.AssignToContainer( NULL, mpImpl->aPersistName );
                    }

                    DisconnectFileLink_Impl();
                }
            }
        }

        if ( xObjRef.is() && mpImpl->pLightClient )
        {
            xObjRef->removeStateChangeListener ( mpImpl->pLightClient );
            xObjRef->removeEventListener( uno::Reference< document::XEventListener >( mpImpl->pLightClient ) );
            xObjRef->setClientSite( NULL );

            GetSdrGlobalData().GetOLEObjCache().RemoveObj(this);
        }
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        OSL_FAIL(
            (OString("SdrOle2Obj::Disconnect_Impl(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }

    mpImpl->mbConnected = false;
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::SetModel(SdrModel* pNewModel)
{
    ::comphelper::IEmbeddedHelper* pDestPers = pNewModel ? pNewModel->GetPersist() : 0;
    ::comphelper::IEmbeddedHelper* pSrcPers  = pModel ? pModel->GetPersist() : 0;

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
            ::rtl::OUString aTmp;
            comphelper::EmbeddedObjectContainer& rContainer = pSrcPers->getEmbeddedObjectContainer();
            uno::Reference < embed::XEmbeddedObject > xObj = rContainer.GetEmbeddedObject( mpImpl->aPersistName );
            DBG_ASSERT( !xObjRef.is() || xObjRef.GetObject() == xObj, "Wrong object identity!" );
            if ( xObj.is() )
            {
                pDestPers->getEmbeddedObjectContainer().MoveEmbeddedObject( rContainer, xObj, aTmp );
                mpImpl->aPersistName = aTmp;
                xObjRef.AssignToContainer( &pDestPers->getEmbeddedObjectContainer(), aTmp );
            }
            DBG_ASSERT( !aTmp.isEmpty(), "Copying embedded object failed!" );
        }
        catch( ::com::sun::star::uno::Exception& )
        {
            OSL_FAIL(
                (OString("SdrOle2Obj::SetModel(), "
                        "exception caught: ") +
                rtl::OUStringToOString(
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

// -----------------------------------------------------------------------------

void SdrOle2Obj::SetPage(SdrPage* pNewPage)
{
    bool bRemove=pNewPage==NULL && pPage!=NULL;
    bool bInsert=pNewPage!=NULL && pPage==NULL;

    if (bRemove && mpImpl->mbConnected )
        Disconnect();

    SdrRectObj::SetPage(pNewPage);

    if (bInsert && !mpImpl->mbConnected )
        Connect();
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::SetObjRef( const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >& rNewObjRef )
{
    DBG_ASSERT( !rNewObjRef.is() || !xObjRef.GetObject().is(), "SetObjRef called on already initialized object!");
    if( rNewObjRef == xObjRef.GetObject() )
        return;

    // the caller of the method is responsible to control the old object, it will not be closed here
    // Otherwise WW8 import crashes because it transfers control to OLENode by this method
    if ( xObjRef.GetObject().is() )
        xObjRef.Lock( sal_False );

    // avoid removal of object in Disconnect! It is definitely a HACK to call SetObjRef(0)!
    // This call will try to close the objects; so if anybody else wants to keep it, it must be locked by a CloseListener
    xObjRef.Clear();

    if ( mpImpl->mbConnected )
        Disconnect();

    xObjRef.Assign( rNewObjRef, GetAspect() );
    m_bTypeAsked = false;

    if ( xObjRef.is() )
    {
        DELETEZ( pGraphic );

        if ( (xObjRef->getStatus( GetAspect() ) & embed::EmbedMisc::EMBED_NEVERRESIZE ) )
            SetResizeProtect(sal_True);

        // For math objects, set closed state to transparent
        if( ImplIsMathObj( rNewObjRef ) )
            SetClosedObj( false );

        Connect();
    }

    SetChanged();
    BroadcastObjectChange();
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::SetClosedObj( bool bIsClosed )
{
    // TODO/LATER: do we still need this hack?
    // Allow changes to the closed state of OLE objects
    bClosedObj = bIsClosed;
}

// -----------------------------------------------------------------------------

SdrObject* SdrOle2Obj::getFullDragClone() const
{
    // special handling for OLE. The default handling works, but is too
    // slow when the whole OLE needs to be cloned. Get the Metafile and
    // create a graphic object with it
    Graphic* pOLEGraphic = GetGraphic();
    SdrObject* pClone = 0;

    if(pOLEGraphic)
    {
        pClone = new SdrGrafObj(*pOLEGraphic, GetSnapRect());

        // this would be the place where to copy all attributes
        // when OLE will support fill and line style
        // pClone->SetMergedItem(pOleObject->GetMergedItemSet());
    }
    else
    {
        // #i100710# pOLEGraphic may be zero (no visualisation available),
        // so we need to use the OLE replacement graphic
        pClone = new SdrRectObj(GetSnapRect());

        // gray outline
        pClone->SetMergedItem(XLineStyleItem(XLINE_SOLID));
        const svtools::ColorConfig aColorConfig;
        const svtools::ColorConfigValue aColor(aColorConfig.GetColorValue(svtools::OBJECTBOUNDARIES));
        pClone->SetMergedItem(XLineColorItem(String(), aColor.nColor));

        // bitmap fill
        pClone->SetMergedItem(XFillStyleItem(XFILL_BITMAP));
        pClone->SetMergedItem(XFillBitmapItem(String(), GetEmtyOLEReplacementBitmap()));
        pClone->SetMergedItem(XFillBmpTileItem(false));
        pClone->SetMergedItem(XFillBmpStretchItem(false));
    }

    return pClone;
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::SetPersistName( const String& rPersistName )
{
    DBG_ASSERT( !mpImpl->aPersistName.Len(), "Persist name changed!");

    mpImpl->aPersistName = rPersistName;
    mpImpl->mbLoadingOLEObjectFailed = false;

    Connect();
    SetChanged();
}

void SdrOle2Obj::AbandonObject()
{
    mpImpl->aPersistName.Erase();
    mpImpl->mbLoadingOLEObjectFailed = false;
    SetObjRef(0);
}

// -----------------------------------------------------------------------------

String SdrOle2Obj::GetPersistName() const
{
    return mpImpl->aPersistName;
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bRotateFreeAllowed=sal_False;
    rInfo.bRotate90Allowed  =sal_False;
    rInfo.bMirrorFreeAllowed=sal_False;
    rInfo.bMirror45Allowed  =sal_False;
    rInfo.bMirror90Allowed  =sal_False;
    rInfo.bTransparenceAllowed = sal_False;
    rInfo.bGradientAllowed = sal_False;
    rInfo.bShearAllowed     =sal_False;
    rInfo.bEdgeRadiusAllowed=sal_False;
    rInfo.bNoOrthoDesired   =sal_False;
    rInfo.bCanConvToPath    =sal_False;
    rInfo.bCanConvToPoly    =sal_False;
    rInfo.bCanConvToPathLineToArea=sal_False;
    rInfo.bCanConvToPolyLineToArea=sal_False;
    rInfo.bCanConvToContour = sal_False;
}

// -----------------------------------------------------------------------------

sal_uInt16 SdrOle2Obj::GetObjIdentifier() const
{
    return bFrame ? sal_uInt16(OBJ_FRAME) : sal_uInt16(OBJ_OLE2);
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::TakeObjNameSingul(XubString& rName) const
{
    rName = ImpGetResStr(bFrame ? STR_ObjNameSingulFrame : STR_ObjNameSingulOLE2);

    const String aName(GetName());

    if( aName.Len() )
    {
        rName.AppendAscii(" '");
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(bFrame ? STR_ObjNamePluralFrame : STR_ObjNamePluralOLE2);
}

// -----------------------------------------------------------------------------

SdrOle2Obj* SdrOle2Obj::Clone() const
{
    return CloneHelper< SdrOle2Obj >();
}

SdrOle2Obj& SdrOle2Obj::operator=(const SdrOle2Obj& rObj)
{
    //TODO/LATER: who takes over control of my old object?!
    if( &rObj != this )
    {
        // #116235#
        // ImpAssign( rObj );
        const SdrOle2Obj& rOle2Obj = static_cast< const SdrOle2Obj& >( rObj );

        uno::Reference < util::XCloseable > xClose( xObjRef.GetObject(), uno::UNO_QUERY );

        if( pModel && mpImpl->mbConnected )
            Disconnect();

        SdrRectObj::operator=( rObj );

        // Manually copying bClosedObj attribute
        SetClosedObj( rObj.IsClosedObj() );

        mpImpl->aPersistName = rOle2Obj.mpImpl->aPersistName;
        aProgName = rOle2Obj.aProgName;
        bFrame = rOle2Obj.bFrame;

        if( rOle2Obj.pGraphic )
        {
            if( pGraphic )
            {
                delete pGraphic;
                delete mpImpl->pGraphicObject;
            }

            pGraphic = new Graphic( *rOle2Obj.pGraphic );
            mpImpl->pGraphicObject = new GraphicObject( *pGraphic );
        }

        if( pModel && rObj.GetModel() && !IsEmptyPresObj() )
        {
            ::comphelper::IEmbeddedHelper* pDestPers = pModel->GetPersist();
            ::comphelper::IEmbeddedHelper* pSrcPers = rObj.GetModel()->GetPersist();
            if( pDestPers && pSrcPers )
            {
                DBG_ASSERT( !xObjRef.is(), "Object already existing!" );
                comphelper::EmbeddedObjectContainer& rContainer = pSrcPers->getEmbeddedObjectContainer();
                uno::Reference < embed::XEmbeddedObject > xObj = rContainer.GetEmbeddedObject( mpImpl->aPersistName );
                if ( xObj.is() )
                {
                    ::rtl::OUString aTmp;
                    xObjRef.Assign( pDestPers->getEmbeddedObjectContainer().CopyAndGetEmbeddedObject( rContainer, xObj, aTmp ), rOle2Obj.GetAspect() );
                    m_bTypeAsked = false;
                    mpImpl->aPersistName = aTmp;
                    CheckFileLink_Impl();
                }

                Connect();
            }
        }
    }
    return *this;
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::ImpSetVisAreaSize()
{
    // currently there is no need to recalculate scaling for iconified objects
    // TODO/LATER: it might be needed in future when it is possible to change the icon
    if ( GetAspect() == embed::Aspects::MSOLE_ICON )
        return;

    // the object area of an embedded object was changed, e.g. by user interaction an a selected object
    GetObjRef();
    if ( xObjRef.is() )
    {
        OSL_ASSERT( pModel );
        sal_Int64 nMiscStatus = xObjRef->getStatus( GetAspect() );

        // the client is required to get access to scaling
        SfxInPlaceClient* pClient = SfxInPlaceClient::GetClient( dynamic_cast<SfxObjectShell*>(pModel->GetPersist()), xObjRef.GetObject() );
        sal_Bool bHasOwnClient =
                        ( mpImpl->pLightClient
                        && xObjRef->getClientSite() == uno::Reference< embed::XEmbeddedClient >( mpImpl->pLightClient ) );

        if ( pClient || bHasOwnClient )
        {
            // TODO: IMHO we need to do similar things when object is UIActive or OutplaceActive?!
            if ( ((nMiscStatus & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE) &&
                    svt::EmbeddedObjectRef::TryRunningState( xObjRef.GetObject() ))
                    || xObjRef->getCurrentState() == embed::EmbedStates::INPLACE_ACTIVE
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
                MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObjRef->getMapUnit( GetAspect() ) );
                Size aVisSize( (long)( Fraction( aRect.GetWidth() ) / aScaleWidth ),
                                (long)( Fraction( aRect.GetHeight() ) / aScaleHeight ) );

                aVisSize = OutputDevice::LogicToLogic( aVisSize, pModel->GetScaleUnit(), aMapUnit);
                awt::Size aSz;
                aSz.Width = aVisSize.Width();
                aSz.Height = aVisSize.Height();
                xObjRef->setVisualAreaSize( GetAspect(), aSz );

                try
                {
                    aSz = xObjRef->getVisualAreaSize( GetAspect() );
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
                    MapUnit aNewMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObjRef->getMapUnit( GetAspect() ) );
                    aRect.SetSize(OutputDevice::LogicToLogic( aAcceptedVisArea.GetSize(), aNewMapUnit, pModel->GetScaleUnit()));
                }

                // make the new object area known to the client
                // compared to the "else" branch aRect might have been changed by the object and no additional scaling was applied
                // WHY this -> OSL_ASSERT( pClient );
                if( pClient )
                    pClient->SetObjArea(aRect);

                // we need a new replacement image as the object has resized itself

                //#i79578# don't request a new replacement image for charts to often
                //a chart sends a modified call to the framework if it was changed
                //thus the replacement update is already handled there
                if( !IsChart() )
                    xObjRef.UpdateReplacement();
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
                        Rectangle aScaleRect(aRect.TopLeft(), aObjAreaSize);
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
            svt::EmbeddedObjectRef::TryRunningState( xObjRef.GetObject() ) )
        {
            //also handle not sfx based ole objects e.g. charts
            //#i83860# resizing charts in impress distorts fonts
            uno::Reference< embed::XVisualObject > xVisualObject( this->getXModel(), uno::UNO_QUERY );
            if( xVisualObject.is() )
            {
                MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObjRef->getMapUnit( GetAspect() ) );
                Point aTL( aRect.TopLeft() );
                Point aBR( aRect.BottomRight() );
                Point aTL2( OutputDevice::LogicToLogic( aTL, pModel->GetScaleUnit(), aMapUnit) );
                Point aBR2( OutputDevice::LogicToLogic( aBR, pModel->GetScaleUnit(), aMapUnit) );
                Rectangle aNewRect( aTL2, aBR2 );
                xVisualObject->setVisualAreaSize( GetAspect(), awt::Size( aNewRect.GetWidth(), aNewRect.GetHeight() ) );
            }
        }
    }
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    if( pModel && !pModel->isLocked() )
    {
        GetObjRef();
        if ( xObjRef.is() && ( xObjRef->getStatus( GetAspect() ) & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE ) )
        {
            // if the object needs recompose on resize
            // the client site should be created before the resize will take place
            // check whether there is no client site and create it if necessary
            AddOwnLightClient();
        }
    }

    SdrRectObj::NbcResize(rRef,xFact,yFact);
    if (aGeo.nShearWink!=0 || aGeo.nDrehWink!=0) { // little corrections
        if (aGeo.nDrehWink>=9000 && aGeo.nDrehWink<27000) {
            aRect.Move(aRect.Left()-aRect.Right(),aRect.Top()-aRect.Bottom());
        }
        aGeo.nDrehWink=0;
        aGeo.nShearWink=0;
        aGeo.nSin=0.0;
        aGeo.nCos=1.0;
        aGeo.nTan=0.0;
        SetRectsDirty();
    }
    if( pModel && !pModel->isLocked() )
        ImpSetVisAreaSize();
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::SetGeoData(const SdrObjGeoData& rGeo)
{
    SdrRectObj::SetGeoData(rGeo);
    if( pModel && !pModel->isLocked() )
        ImpSetVisAreaSize();
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::NbcSetSnapRect(const Rectangle& rRect)
{
    SdrRectObj::NbcSetSnapRect(rRect);
    if( pModel && !pModel->isLocked() )
        ImpSetVisAreaSize();

    if ( xObjRef.is() && IsChart() )
    {
        //#i103460# charts do not necessarily have an own size within ODF files,
        //for this case they need to use the size settings from the surrounding frame,
        //which is made available with this method as there is no other way
        xObjRef.SetDefaultSizeForChart( Size( rRect.GetWidth(), rRect.GetHeight() ) );
    }
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::NbcSetLogicRect(const Rectangle& rRect)
{
    SdrRectObj::NbcSetLogicRect(rRect);
    if( pModel && !pModel->isLocked() )
        ImpSetVisAreaSize();
}

Graphic* SdrOle2Obj::GetGraphic() const
{
    if ( xObjRef.is() )
        return xObjRef.GetGraphic();
    return pGraphic;
}

void SdrOle2Obj::GetNewReplacement()
{
    if ( xObjRef.is() )
        xObjRef.UpdateReplacement();
}

// -----------------------------------------------------------------------------

Size SdrOle2Obj::GetOrigObjSize( MapMode* pTargetMapMode ) const
{
    return xObjRef.GetSize( pTargetMapMode );
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::NbcMove(const Size& rSize)
{
    SdrRectObj::NbcMove(rSize);
    if( pModel && !pModel->isLocked() )
        ImpSetVisAreaSize();
}

// -----------------------------------------------------------------------------

sal_Bool SdrOle2Obj::CanUnloadRunningObj( const uno::Reference< embed::XEmbeddedObject >& xObj, sal_Int64 nAspect )
{
    sal_Bool bResult = sal_False;

    sal_Int32 nState = xObj->getCurrentState();
    if ( nState == embed::EmbedStates::LOADED )
    {
        // the object is already unloaded
        bResult = sal_True;
    }
    else
    {
        uno::Reference < util::XModifiable > xModifiable( xObj->getComponent(), uno::UNO_QUERY );
        if ( !xModifiable.is() )
            bResult = sal_True;
        else
        {
            sal_Int64 nMiscStatus = xObj->getStatus( nAspect );

            if ( embed::EmbedMisc::MS_EMBED_ALWAYSRUN != ( nMiscStatus & embed::EmbedMisc::MS_EMBED_ALWAYSRUN ) &&
            embed::EmbedMisc::EMBED_ACTIVATEIMMEDIATELY != ( nMiscStatus & embed::EmbedMisc::EMBED_ACTIVATEIMMEDIATELY ) &&
            !( xModifiable.is() && xModifiable->isModified() ) &&
            !( nState == embed::EmbedStates::INPLACE_ACTIVE || nState == embed::EmbedStates::UI_ACTIVE || nState == embed::EmbedStates::ACTIVE ) )
            {
                bResult = sal_True;
            }
        }
    }

    return bResult;
}

// -----------------------------------------------------------------------------

sal_Bool SdrOle2Obj::Unload( const uno::Reference< embed::XEmbeddedObject >& xObj, sal_Int64 nAspect )
{
    sal_Bool bResult = sal_False;

    if ( CanUnloadRunningObj( xObj, nAspect ) )
    {
        try
        {
            xObj->changeState( embed::EmbedStates::LOADED );
            bResult = sal_True;
        }
        catch( ::com::sun::star::uno::Exception& )
        {
            OSL_FAIL(
                (OString("SdrOle2Obj::Unload=(), "
                        "exception caught: ") +
                rtl::OUStringToOString(
                    comphelper::anyToString( cppu::getCaughtException() ),
                    RTL_TEXTENCODING_UTF8 )).getStr() );
        }
    }

    return bResult;
}

// -----------------------------------------------------------------------------

sal_Bool SdrOle2Obj::Unload()
{
    sal_Bool bUnloaded = sal_False;

    if( xObjRef.is() )
    {
        //TODO/LATER: no refcounting tricks anymore!
        //"customers" must register as state change listeners
        // Not necessary in Doc DTor (MM)
        //sal_uIntPtr nRefCount = (*ppObjRef)->GetRefCount();
        // prevent Unload if there are external references
        //if( nRefCount > 2 )
        //    return sal_False;
        //DBG_ASSERT( nRefCount == 2, "Wrong RefCount for unload" );
    }
    else
        bUnloaded = sal_True;

    if ( pModel && xObjRef.is() )
    {
        bUnloaded = Unload( xObjRef.GetObject(), GetAspect() );
    }

    return bUnloaded;
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::GetObjRef_Impl()
{
    if ( !xObjRef.is() && mpImpl->aPersistName.Len() && pModel && pModel->GetPersist() )
    {
        // Only try loading if it did not went wrong up to now
        if(!mpImpl->mbLoadingOLEObjectFailed)
        {
            xObjRef.Assign( pModel->GetPersist()->getEmbeddedObjectContainer().GetEmbeddedObject( mpImpl->aPersistName ), GetAspect() );
            m_bTypeAsked = false;
            CheckFileLink_Impl();

            // If loading of OLE object failed, remember that to not invoke a endless
            // loop trying to load it again and again.
            if( xObjRef.is() )
            {
                mpImpl->mbLoadingOLEObjectFailed = sal_True;
            }

            // For math objects, set closed state to transparent
            if( ImplIsMathObj( xObjRef.GetObject() ) )
                SetClosedObj( false );
        }

        if ( xObjRef.is() )
        {
            if( !IsEmptyPresObj() )
            {
                // remember modified status of model
                const sal_Bool bWasChanged(pModel ? pModel->IsChanged() : sal_False);

                // perhaps preview not valid anymore
                // This line changes the modified state of the model
                SetGraphic_Impl( NULL );

                // if status was not set before, force it back
                // to not set, so that SetGraphic(0L) above does not
                // set the modified state of the model.
                if(!bWasChanged && pModel && pModel->IsChanged())
                {
                    pModel->SetChanged( sal_False );
                }
            }

            sal_Int64 nMiscStatus = xObjRef->getStatus( GetAspect() );
            (void)nMiscStatus;
            //TODO/LATER: wait until ResizeOnPrinterChange is defined
            //if ( nMiscStatus & SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE )
            {
                if (pModel && pModel->GetRefDevice() &&
                    pModel->GetRefDevice()->GetOutDevType() == OUTDEV_PRINTER)
                {
                    if(!bInDestruction)
                    {
                        //TODO/LATER: printerchange notification
                        /*
                        // prevent SetModified (don't want no update here)
                        sal_Bool bWasEnabled = (*ppObjRef)->IsEnableSetModified();
                        if ( bWasEnabled )
                            (*ppObjRef)->EnableSetModified( sal_False );

                        // Kein RefDevice oder RefDevice kein Printer
                        Printer* pPrinter = (Printer*) pModel->GetRefDevice();
                        (*ppObjRef)->OnDocumentPrinterChanged( pPrinter );

                        // reset state
                        (*ppObjRef)->EnableSetModified( bWasEnabled );*/
                    }
                }
            }
        }

        if ( xObjRef.is() )
            Connect();
    }

    if ( mpImpl->mbConnected )
        // move object to first position in cache
        GetSdrGlobalData().GetOLEObjCache().InsertObj(this);
}

uno::Reference < embed::XEmbeddedObject > SdrOle2Obj::GetObjRef() const
{
    const_cast<SdrOle2Obj*>(this)->GetObjRef_Impl();
    return xObjRef.GetObject();
}

uno::Reference < embed::XEmbeddedObject > SdrOle2Obj::GetObjRef_NoInit() const
{
    return xObjRef.GetObject();
}

// -----------------------------------------------------------------------------

uno::Reference< frame::XModel > SdrOle2Obj::getXModel() const
{
    GetObjRef();
    if ( svt::EmbeddedObjectRef::TryRunningState(xObjRef.GetObject()) )
        return uno::Reference< frame::XModel >( xObjRef->getComponent(), uno::UNO_QUERY );
    else
        return uno::Reference< frame::XModel >();
}

// -----------------------------------------------------------------------------

sal_Bool SdrOle2Obj::IsChart() const
{
    if ( !m_bTypeAsked )
    {
        m_bChart = ChartPrettyPainter::IsChart(xObjRef);
        m_bTypeAsked = true;
    }
    return m_bChart;
}

// -----------------------------------------------------------------------------
void SdrOle2Obj::SetGraphicToObj( const Graphic& aGraphic, const ::rtl::OUString& aMediaType )
{
    xObjRef.SetGraphic( aGraphic, aMediaType );
}

// -----------------------------------------------------------------------------
void SdrOle2Obj::SetGraphicToObj( const uno::Reference< io::XInputStream >& xGrStream, const ::rtl::OUString& aMediaType )
{
    xObjRef.SetGraphicStream( xGrStream, aMediaType );
}

// -----------------------------------------------------------------------------
sal_Bool SdrOle2Obj::IsCalc() const
{
    if ( !xObjRef.is() )
        return false;

    SvGlobalName aObjClsId( xObjRef->getClassID() );
    if(    SvGlobalName(SO3_SC_CLASSID_30) == aObjClsId
        || SvGlobalName(SO3_SC_CLASSID_40) == aObjClsId
        || SvGlobalName(SO3_SC_CLASSID_50) == aObjClsId
        || SvGlobalName(SO3_SC_CLASSID_60) == aObjClsId
        || SvGlobalName(SO3_SC_OLE_EMBED_CLASSID_60) == aObjClsId
        || SvGlobalName(SO3_SC_OLE_EMBED_CLASSID_8) == aObjClsId
        || SvGlobalName(SO3_SC_CLASSID) == aObjClsId )
    {
        return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------------
uno::Reference< frame::XModel > SdrOle2Obj::GetParentXModel() const
{
    uno::Reference< frame::XModel > xDoc;
    if ( pModel )
        xDoc.set( pModel->getUnoModel(),uno::UNO_QUERY);
    return xDoc;
}

// -----------------------------------------------------------------------------
sal_Bool SdrOle2Obj::CalculateNewScaling( Fraction& aScaleWidth, Fraction& aScaleHeight, Size& aObjAreaSize )
{
    // TODO/LEAN: to avoid rounding errors scaling always uses the VisArea.
    // If we don't cache it for own objects also we must load the object here
    if ( !xObjRef.is() || !pModel )
        return sal_False;

    MapMode aMapMode( pModel->GetScaleUnit() );
    aObjAreaSize = xObjRef.GetSize( &aMapMode );

    Size aSize = aRect.GetSize();
    aScaleWidth = Fraction(aSize.Width(),  aObjAreaSize.Width() );
    aScaleHeight = Fraction(aSize.Height(), aObjAreaSize.Height() );

    // reduce to 10 binary digits
    Kuerzen(aScaleHeight, 10);
    Kuerzen(aScaleWidth,  10);

    return sal_True;
}

// -----------------------------------------------------------------------------
sal_Bool SdrOle2Obj::AddOwnLightClient()
{
    // The Own Light Client must be registered in object only using this method!
    if ( !SfxInPlaceClient::GetClient( dynamic_cast<SfxObjectShell*>(pModel->GetPersist()), xObjRef.GetObject() )
      && !( mpImpl->pLightClient && xObjRef->getClientSite() == uno::Reference< embed::XEmbeddedClient >( mpImpl->pLightClient ) ) )
    {
        Connect();

        if ( xObjRef.is() && mpImpl->pLightClient )
        {
            Fraction aScaleWidth;
            Fraction aScaleHeight;
            Size aObjAreaSize;
            if ( CalculateNewScaling( aScaleWidth, aScaleHeight, aObjAreaSize ) )
            {
                mpImpl->pLightClient->SetSizeScale( aScaleWidth, aScaleHeight );
                try {
                    xObjRef->setClientSite( mpImpl->pLightClient );
                    return sal_True;
                } catch( uno::Exception& )
                {}
            }

        }

        return sal_False;
    }

       return sal_True;
}

//////////////////////////////////////////////////////////////////////////////

Bitmap SdrOle2Obj::GetEmtyOLEReplacementBitmap()
{
    return Bitmap(ResId(BMP_SVXOLEOBJ, *ImpGetResMgr()));
}

//////////////////////////////////////////////////////////////////////////////

void SdrOle2Obj::SetWindow(const com::sun::star::uno::Reference < com::sun::star::awt::XWindow >& _xWindow)
{
    if ( xObjRef.is() && mpImpl->pLightClient )
    {
        mpImpl->pLightClient->setWindow(_xWindow);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
