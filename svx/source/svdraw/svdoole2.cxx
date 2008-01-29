/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdoole2.cxx,v $
 *
 *  $Revision: 1.85 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:39:20 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdoole2.hxx>

#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODE_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDMISC_HPP_
#include <com/sun/star/embed/EmbedMisc.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XLINKAGESUPPORT_HPP_
#include <com/sun/star/embed/XLinkageSupport.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_NOVISUALAREASIZEEXCEPTION_HPP_
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif

#include <comphelper/processfactory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svtools/filter.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/embedhlp.hxx>

#include <sfx2/objsh.hxx>
#include <sfx2/ipclient.hxx>
#include <sfx2/lnkbase.hxx>
#include <tools/stream.hxx>
#include <comphelper/anytostring.hxx>

#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif
#ifndef _GLOBNAME_HXX
#include <tools/globname.hxx>
#endif
#ifndef _SV_JOBSET_HXX
#include <vcl/jobset.hxx>
#endif
#ifndef _SO_CLSIDS_HXX
#include <sot/clsids.hxx>
#endif

#include <sot/formats.hxx>

#ifndef _SVXLINKMGR_HXX //autogen
#include <linkmgr.hxx>
#endif
#ifndef SVTOOLS_TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

#include <cppuhelper/implbase3.hxx>

#include <svtools/solar.hrc>
#include "svdxout.hxx"
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#include <svx/svdpagv.hxx>
#include <svx/svdmodel.hxx>
#include "svdglob.hxx"  // Stringcache
#include "svdstr.hrc"   // Objektname
#include <svx/svdetc.hxx>
#include <svx/svdview.hxx>
#include "unomlstr.hxx"
#include "impgrf.hxx"
#include <svx/chartprettypainter.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;

class SdrLightEmbeddedClient_Impl : public ::cppu::WeakImplHelper3
                                                            < embed::XStateChangeListener
                                                            , document::XEventListener
                                                            , embed::XEmbeddedClient >
{
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

    Fraction GetScaleWidth() { return m_aScaleWidth; }
    Fraction GetScaleHeight() { return m_aScaleHeight; }

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
};

//--------------------------------------------------------------------
SdrLightEmbeddedClient_Impl::SdrLightEmbeddedClient_Impl( SdrOle2Obj* pObj )
: mpObj( pObj )
{
}

//--------------------------------------------------------------------
void SAL_CALL SdrLightEmbeddedClient_Impl::changingState( const ::com::sun::star::lang::EventObject& /*aEvent*/, ::sal_Int32 /*nOldState*/, ::sal_Int32 /*nNewState*/ ) throw (::com::sun::star::embed::WrongStateException, ::com::sun::star::uno::RuntimeException)
{
}

//--------------------------------------------------------------------
void SdrLightEmbeddedClient_Impl::Release()
{
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        mpObj = NULL;
    }

    release();
}

//--------------------------------------------------------------------
void SAL_CALL SdrLightEmbeddedClient_Impl::stateChanged( const ::com::sun::star::lang::EventObject& /*aEvent*/, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

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
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    GetSdrGlobalData().GetOLEObjCache().RemoveObj(mpObj);
}

//--------------------------------------------------------------------
void SAL_CALL SdrLightEmbeddedClient_Impl::notifyEvent( const document::EventObject& aEvent ) throw( uno::RuntimeException )
{
    // TODO/LATER: when writer uses this implementation the code could be shared with SfxInPlaceClient_Impl

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // the code currently makes sence only in case there is no other client
    if ( mpObj && mpObj->GetAspect() != embed::Aspects::MSOLE_ICON && aEvent.EventName.equalsAscii("OnVisAreaChanged")
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
                OSL_ENSURE( sal_False, "No visual area size!\n" );
                aSz.Width = 5000;
                aSz.Height = 5000;
            }
            catch( uno::Exception& )
            {
                OSL_ENSURE( sal_False, "Unexpected exception!\n" );
                aSz.Width = 5000;
                aSz.Height = 5000;
            }

            aVisArea.SetSize( Size( aSz.Width, aSz.Height ) );
            aVisArea = OutputDevice::LogicToLogic( aVisArea, aObjMapUnit, aContainerMapUnit );
            Rectangle aLogicRect( mpObj->GetLogicRect() );
            Size aScaledSize( static_cast< long >( m_aScaleWidth * Fraction( aVisArea.GetWidth() ) ),
                                static_cast< long >( m_aScaleHeight * Fraction( aVisArea.GetHeight() ) ) );

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
            OSL_ENSURE( sal_False, "Unexpected exception!\n" );
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
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        if ( !mpObj )
            throw embed::ObjectSaveVetoException();

        // the common persistance is supported by objects and links
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
}

//--------------------------------------------------------------------
uno::Reference< util::XCloseable > SAL_CALL SdrLightEmbeddedClient_Impl::getComponent()
    throw ( uno::RuntimeException )
{
    uno::Reference< util::XCloseable > xResult;

    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( mpObj )
        xResult = uno::Reference< util::XCloseable >( mpObj->GetParentXModel(), uno::UNO_QUERY );

    return xResult;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrEmbedObjectLink : public sfx2::SvBaseLink
{
    SdrOle2Obj*         pObj;

public:
                        SdrEmbedObjectLink(SdrOle2Obj* pObj);
    virtual             ~SdrEmbedObjectLink();

    virtual void        Closed();
    virtual void        DataChanged( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue );

    sal_Bool            Connect() { return GetRealObject() != NULL; }
};

// -----------------------------------------------------------------------------

SdrEmbedObjectLink::SdrEmbedObjectLink(SdrOle2Obj* pObject):
    ::sfx2::SvBaseLink( ::sfx2::LINKUPDATE_ONCALL, SOT_FORMATSTR_ID_SVXB ),
    pObj(pObject)
{
    SetSynchron( FALSE );
}

// -----------------------------------------------------------------------------

SdrEmbedObjectLink::~SdrEmbedObjectLink()
{
}

// -----------------------------------------------------------------------------

void SdrEmbedObjectLink::DataChanged( const String& /*rMimeType*/,
                                const ::com::sun::star::uno::Any & /*rValue*/ )
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
                if ( nState == embed::EmbedStates::LOADED )
                    xObject->changeState( embed::EmbedStates::RUNNING );
                else
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

    // #107645#
    // New local var to avoid repeated loading if load of OLE2 fails
    sal_Bool        mbLoadingOLEObjectFailed;
    sal_Bool        mbConnected;

    SdrEmbedObjectLink* mpObjectLink;
    String maLinkURL;

    SdrOle2ObjImpl()
    : pGraphicObject( NULL )
    // #107645#
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

// -----------------------------------------------------------------------------

TYPEINIT1(SdrOle2Obj,SdrRectObj);

SdrOle2Obj::SdrOle2Obj(FASTBOOL bFrame_)
{
    bInDestruction = FALSE;
    Init();
    bFrame=bFrame_;
}

// -----------------------------------------------------------------------------
SdrOle2Obj::SdrOle2Obj( const svt::EmbeddedObjectRef& rNewObjRef, FASTBOOL bFrame_)
    : xObjRef( rNewObjRef )
{
    bInDestruction = FALSE;
    Init();

    bFrame=bFrame_;

    if ( xObjRef.is() && (xObjRef->getStatus( GetAspect() ) & embed::EmbedMisc::EMBED_NEVERRESIZE ) )
        SetResizeProtect(TRUE);

    // #108759# For math objects, set closed state to transparent
    if( ImplIsMathObj( xObjRef.GetObject() ) )
        SetClosedObj( false );
}

// -----------------------------------------------------------------------------

SdrOle2Obj::SdrOle2Obj( const svt::EmbeddedObjectRef& rNewObjRef, const XubString& rNewObjName, FASTBOOL bFrame_)
    : xObjRef( rNewObjRef )
{
    bInDestruction = FALSE;
    Init();

    mpImpl->aPersistName = rNewObjName;
    bFrame=bFrame_;

    if ( xObjRef.is() && (xObjRef->getStatus( GetAspect() ) & embed::EmbedMisc::EMBED_NEVERRESIZE ) )
        SetResizeProtect(TRUE);

    // #108759# For math objects, set closed state to transparent
    if( ImplIsMathObj( xObjRef.GetObject() ) )
        SetClosedObj( false );
}

// -----------------------------------------------------------------------------

SdrOle2Obj::SdrOle2Obj( const svt::EmbeddedObjectRef&  rNewObjRef, const XubString& rNewObjName, const Rectangle& rNewRect, FASTBOOL bFrame_)
    : SdrRectObj(rNewRect)
    , xObjRef( rNewObjRef )
{
    bInDestruction = FALSE;
    Init();

    mpImpl->aPersistName = rNewObjName;
    bFrame=bFrame_;

    if ( xObjRef.is() && (xObjRef->getStatus( GetAspect() ) & embed::EmbedMisc::EMBED_NEVERRESIZE ) )
        SetResizeProtect(TRUE);

    // #108759# For math objects, set closed state to transparent
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

    xObjRef.Lock( TRUE );
}

// -----------------------------------------------------------------------------

SdrOle2Obj::~SdrOle2Obj()
{
    bInDestruction = TRUE;

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

    //if ( ppObjRef->Is() && pGrf )
    //  BroadcastObjectChange();
}

void SdrOle2Obj::SetGraphic(const Graphic* pGrf)
{
    // only for setting a preview graphic
    SetGraphic_Impl( pGrf );
}

// -----------------------------------------------------------------------------

FASTBOOL SdrOle2Obj::IsEmpty() const
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
        // mba: currently there are situations where it seems to be unavoidable to have multiple connects
        // changing this would need a larger code rewrite, so for now I remove the assertion
        // DBG_ERROR("Connect() called on connected object!");
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
        SvxLinkManager* pLinkManager = pModel ? pModel->GetLinkManager() : NULL;
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
                        aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );
                        aArgs[0].Value <<= ::rtl::OUString( aNewLinkURL );
                        xPersObj->reload( aArgs, uno::Sequence< beans::PropertyValue >() );

                        mpImpl->maLinkURL = aNewLinkURL;
                        bResult = sal_True;

                        if ( nCurState != embed::EmbedStates::LOADED )
                            xObjRef->changeState( nCurState );
                    }
                    catch( ::com::sun::star::uno::Exception& e )
                    {
                        (void)e;
                        DBG_ERROR(
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

    SfxObjectShell* pPers = pModel ? pModel->GetPersist() : NULL;

    if ( pPers )
    {
        uno::Reference< embed::XStorage > xStorage = pPers->GetStorage();
        if ( xStorage.is() )
        {
            try
            {
                uno::Reference< embed::XLinkageSupport > xLinkSupport( xObjRef.GetObject(), uno::UNO_QUERY_THROW );
                xLinkSupport->breakLink( xStorage, mpImpl->aPersistName );
                DisconnectFileLink_Impl();
                mpImpl->maLinkURL = String();
            }
            catch( ::com::sun::star::uno::Exception& e )
            {
                (void)e;
                DBG_ERROR(
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
    SvxLinkManager* pLinkManager = pModel ? pModel->GetLinkManager() : NULL;
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
                    SvxLinkManager* pLinkManager = pModel->GetLinkManager();
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
        catch( ::com::sun::star::uno::Exception& e )
        {
            (void)e;
            DBG_ERROR(
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
            SfxObjectShell* pPers=pModel->GetPersist();
            if ( pPers )
            {
                comphelper::EmbeddedObjectContainer& rContainer = pPers->GetEmbeddedObjectContainer();
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
                    xObjRef.Assign( rContainer.GetEmbeddedObject( mpImpl->aPersistName ), xObjRef.GetViewAspect() );

                if ( xObjRef.GetObject().is() )
                {
                    xObjRef.AssignToContainer( &rContainer, mpImpl->aPersistName );
                    mpImpl->mbConnected = true;
                    xObjRef.Lock( TRUE );
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
        catch( ::com::sun::star::uno::Exception& e )
        {
            (void)e;
            DBG_ERROR(
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
            BOOL bModified = (*ppObjRef)->IsModified();
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
        DBG_ERROR("Disconnect() called on disconnected object!");
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
        catch( ::com::sun::star::uno::Exception& e )
        {
            (void)e;
            DBG_ERROR(
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
                // in case the model has correct persistance, but in case of problems such a removing
                // would make the behaviour of the office more stable

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
                SfxObjectShell* pPers = pModel->GetPersist();
                if ( pPers )
                {
                    // remove object, but don't close it (that's up to someone else)
                    comphelper::EmbeddedObjectContainer* pContainer = xObjRef.GetContainer();
                    if ( pContainer )
                    {
                        pContainer->RemoveEmbeddedObject( xObjRef.GetObject(), sal_False);

                        // TODO/LATER: mpImpl->aPersistName contains outdated information, to have it uptodate
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
    catch( ::com::sun::star::uno::Exception& e )
    {
        (void)e;
        DBG_ERROR(
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
    SfxObjectShell* pDestPers = pNewModel ? pNewModel->GetPersist() : 0;
    SfxObjectShell* pSrcPers  = pModel ? pModel->GetPersist() : 0;

    if ( pNewModel == pModel )
    {
        // don't know if this is necessary or if it will ever happen, but who know?!
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
            // move the objects' storage; ObjectRef remains the same, but PersistName may change
            ::rtl::OUString aTmp;
            comphelper::EmbeddedObjectContainer& rContainer = pSrcPers->GetEmbeddedObjectContainer();
            uno::Reference < embed::XEmbeddedObject > xObj = rContainer.GetEmbeddedObject( mpImpl->aPersistName );
            DBG_ASSERT( !xObjRef.is() || xObjRef.GetObject() == xObj, "Wrong object identity!" );
            if ( xObj.is() )
            {
                pDestPers->GetEmbeddedObjectContainer().MoveEmbeddedObject( rContainer, xObj, aTmp );
                mpImpl->aPersistName = aTmp;
                xObjRef.AssignToContainer( &pDestPers->GetEmbeddedObjectContainer(), aTmp );
            }
            DBG_ASSERT( aTmp.getLength(), "Copying embedded object failed!" );
        }
        catch( ::com::sun::star::uno::Exception& e )
        {
            (void)e;
            DBG_ERROR(
                (OString("SdrOle2Obj::SetModel(), "
                        "exception caught: ") +
                rtl::OUStringToOString(
                    comphelper::anyToString( cppu::getCaughtException() ),
                    RTL_TEXTENCODING_UTF8 )).getStr() );
        }
    }

    SdrRectObj::SetModel( pNewModel );

    // #i43086#
    // #i85304 redo the change for charts for the above bugfix, as #i43086# does not ocur anymore
    //so maybe the ImpSetVisAreaSize call can be removed here completely
    //Nevertheless I leave it in for other objects as I am not sure about the side effects when removing now
    if( pModel && !pModel->isLocked() && !IsChart() )
        ImpSetVisAreaSize();

    if( pDestPers && !IsEmptyPresObj() )
    {
        if ( !pSrcPers || IsEmptyPresObj() )
            // object wasn't connected, now it should
            Connect_Impl();
        else
            Reconnect_Impl();
    }

    AddListeners_Impl();
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::SetPage(SdrPage* pNewPage)
{
    FASTBOOL bRemove=pNewPage==NULL && pPage!=NULL;
    FASTBOOL bInsert=pNewPage!=NULL && pPage==NULL;

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

    // MBA: the caller of the method is responsible to control the old object, it will not be closed here
    // Otherwise WW8 import crashes because it tranfers control to OLENode by this method
    if ( xObjRef.GetObject().is() )
        xObjRef.Lock( FALSE );

    // MBA: avoid removal of object in Disconnect! It is definitely a HACK to call SetObjRef(0)!
    // This call will try to close the objects; so if anybody else wants to keep it, it must be locked by a CloseListener
    xObjRef.Clear();

    if ( mpImpl->mbConnected )
        Disconnect();

    xObjRef.Assign( rNewObjRef, GetAspect() );

    if ( xObjRef.is() )
    {
        DELETEZ( pGraphic );

        if ( (xObjRef->getStatus( GetAspect() ) & embed::EmbedMisc::EMBED_NEVERRESIZE ) )
            SetResizeProtect(TRUE);

        // #108759# For math objects, set closed state to transparent
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
    // #108759# Allow changes to the closed state of OLE objects
    bClosedObj = bIsClosed;
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
    rInfo.bRotateFreeAllowed=FALSE;
    rInfo.bRotate90Allowed  =FALSE;
    rInfo.bMirrorFreeAllowed=FALSE;
    rInfo.bMirror45Allowed  =FALSE;
    rInfo.bMirror90Allowed  =FALSE;
    rInfo.bTransparenceAllowed = FALSE;
    rInfo.bGradientAllowed = FALSE;
    rInfo.bShearAllowed     =FALSE;
    rInfo.bEdgeRadiusAllowed=FALSE;
    rInfo.bNoOrthoDesired   =FALSE;
    rInfo.bCanConvToPath    =FALSE;
    rInfo.bCanConvToPoly    =FALSE;
    rInfo.bCanConvToPathLineToArea=FALSE;
    rInfo.bCanConvToPolyLineToArea=FALSE;
    rInfo.bCanConvToContour = FALSE;
}

// -----------------------------------------------------------------------------

UINT16 SdrOle2Obj::GetObjIdentifier() const
{
    return bFrame ? UINT16(OBJ_FRAME) : UINT16(OBJ_OLE2);
}

// -----------------------------------------------------------------------------

sal_Bool SdrOle2Obj::DoPaintObject(XOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec) const
{
    sal_Bool bOk(sal_True);

    //charts must be painted resolution dependent!! #i82893#, #i75867#
    if( ChartPrettyPainter::IsChart(xObjRef) && ChartPrettyPainter::ShouldPrettyPaintChartOnThisDevice( rOut.GetOutDev() ) )
        if( !rOut.GetOffset().nA && !rOut.GetOffset().nB )//offset!=0 is the scenario 'copy -> past special gdi metafile' which does not work with direct painting so far
            if( ChartPrettyPainter::DoPrettyPaintChart( this->getXModel(), rOut.GetOutDev(), aRect ) )
                return bOk;

    if( !GetGraphic() )
        ( (SdrOle2Obj*) this)->GetObjRef_Impl();    // try to create embedded object

    if ( xObjRef.is() )
    {
        sal_Int64 nMiscStatus = xObjRef->getStatus( GetAspect() );
        if( !bSizProt && (nMiscStatus & embed::EmbedMisc::EMBED_NEVERRESIZE) )
            ( (SdrOle2Obj*) this)->bSizProt = TRUE;

        OutputDevice* pOut = rOut.GetOutDev();

        //TODO/LATER: currently it's not possible to compare the windows, the XOutDev contains a virtual device
        sal_Int32 nState = xObjRef->getCurrentState();
        //if ( ( nState != embed::EmbedStates::INPLACE_ACTIVE && nState != embed::EmbedStates::UI_ACTIVE ) ||
        //       pModel && SfxInPlaceClient::GetActiveWindow( pModel->GetPersist(), xObjRef ) != pOut )
        {
            if ( nMiscStatus & embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE )
            {
                // PlugIn-Objekt connecten
                if (rInfoRec.pPV!=NULL)
                {
                    SdrOle2Obj* pOle2Obj = (SdrOle2Obj*) this;
                    SdrView* pSdrView = (SdrView*) &rInfoRec.pPV->GetView();
                    pSdrView->DoConnect(pOle2Obj);
                }
            }

            // #108759# Temporarily set the current background
            // color, since OLEs rely on that during
            // auto-colored text rendering
            Wallpaper aOldBg( pOut->GetBackground() );

            if( rInfoRec.pPV && GetPage() )
                pOut->SetBackground( rInfoRec.pPV->GetView().CalcBackgroundColor( GetSnapRect(),
                                                                                  rInfoRec.pPV->GetVisibleLayers(),
                                                                                  *GetPage() ) );

            pOut->Push( PUSH_CLIPREGION );
            pOut->IntersectClipRegion( aRect );

            GetGraphic();
            PaintGraphic_Impl( rOut, rInfoRec, nState == embed::EmbedStates::ACTIVE );

            /*
            if ( !mpImpl->pMetaFile )
                GetGDIMetaFile();
            if ( mpImpl->pMetaFile )
                mpImpl->pMetaFile->Play( pOut, aRect.TopLeft(), aRect.GetSize() );
            //(*ppObjRef)->DoDraw(pOut,aRect.TopLeft(),aRect.GetSize(),JobSetup());
            */

            pOut->Pop();

            // #108759# Restore old background
            pOut->SetBackground( aOldBg );
        }
    }
    else if ( GetGraphic() )
    {
        PaintGraphic_Impl( rOut, rInfoRec );
    }
    // #100499# OLE without context and without bitmap, do the same as
    // for empty groups, additionally draw empty OLE bitmap
    else
    {
        if(!rInfoRec.bPrinter && rInfoRec.aPaintLayer.IsSet(GetLayer()))
        {
            OutputDevice* pOutDev = rOut.GetOutDev();

            pOutDev->SetFillColor();
            pOutDev->SetLineColor(Color(COL_LIGHTGRAY));
            pOutDev->DrawRect(aOutRect);

            Bitmap aBitmap(ResId(BMP_SVXOLEOBJ, *ImpGetResMgr()));
            Rectangle aSnapRect(GetSnapRect());
            Size aBitmapSize(pOutDev->PixelToLogic(aBitmap.GetSizePixel()));

            pOutDev->DrawBitmap(
                aSnapRect.Center() - Point(aBitmapSize.Width() / 2, aBitmapSize.Height() / 2),
                aBitmapSize,
                aBitmap);
        }
    }

    if (HasText())
    {
        bOk = SdrTextObj::DoPaintObject(rOut, rInfoRec);
    }
    return bOk;
}

void SdrOle2Obj::PaintGraphic_Impl( XOutputDevice& rOut, const SdrPaintInfoRec& /*rInfoRec*/, sal_Bool bActive ) const
{
    OutputDevice* pOutDev = rOut.GetOutDev();
    OSL_ENSURE( pOutDev, "The device must be provided!\n" );
    if ( !pOutDev )
        return;

    // In case High Contrast mode is requested try to get a graphical representation in this mode
    // if it is not possible the replacement image should be used
    Graphic* pGr = NULL;
    if ( ( pOutDev->GetDrawMode() & DRAWMODE_SETTINGSFILL ) && xObjRef.is() )
        pGr = xObjRef.GetHCGraphic();

    if ( !pGr )
        pGr = GetGraphic();

    if ( pGr && pGr->GetType() != GRAPHIC_NONE )
    {
        if( IsEmptyPresObj() )
        {
            const MapMode   aDstMapMode( pOutDev->GetMapMode().GetMapUnit() );
            Point           aPos(aRect.Center());
            Size            aDstSize;

            if( pGr->GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
                aDstSize = pOutDev->PixelToLogic( pGr->GetPrefSize(), aDstMapMode );
            else
                aDstSize = pOutDev->LogicToLogic( pGr->GetPrefSize(), pGr->GetPrefMapMode(), aDstMapMode );

            aPos.X()-=aDstSize.Width() /2;
            aPos.Y()-=aDstSize.Height()/2;
            if (aPos.X() >= aRect.Left() && aPos.Y() >= aRect.Top())
                pGr->Draw(pOutDev,aPos, aDstSize);

            svtools::ColorConfig aColorConfig;
            svtools::ColorConfigValue aColor( aColorConfig.GetColorValue( svtools::OBJECTBOUNDARIES ) );

            if( aColor.bIsVisible )
            {
                pOutDev->SetFillColor();
                pOutDev->SetLineColor( aColor.nColor );
                pOutDev->DrawRect(aRect);
            }
        }
        else
            pGr->Draw( pOutDev, aRect.TopLeft(), aRect.GetSize() );

        // shade the representation if the object is activated outplace
        if ( bActive )
            ::svt::EmbeddedObjectRef::DrawShading( aRect, pOutDev );
    }
    else
        ::svt::EmbeddedObjectRef::DrawPaintReplacement( aRect, mpImpl->aPersistName, pOutDev );
}

// -----------------------------------------------------------------------------

SdrObject* SdrOle2Obj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    return ImpCheckHit(rPnt,nTol,pVisiLayer,TRUE,TRUE);
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

void SdrOle2Obj::operator=(const SdrObject& rObj)
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

        // #108867# Manually copying bClosedObj attribute
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
            SfxObjectShell* pDestPers = pModel->GetPersist();
            SfxObjectShell* pSrcPers = rObj.GetModel()->GetPersist();
            if( pDestPers && pSrcPers )
            {
                DBG_ASSERT( !xObjRef.is(), "Object already existing!" );
                comphelper::EmbeddedObjectContainer& rContainer = pSrcPers->GetEmbeddedObjectContainer();
                uno::Reference < embed::XEmbeddedObject > xObj = rContainer.GetEmbeddedObject( mpImpl->aPersistName );
                if ( xObj.is() )
                {
                    ::rtl::OUString aTmp;
                    xObjRef.Assign( pDestPers->GetEmbeddedObjectContainer().CopyAndGetEmbeddedObject( rContainer, xObj, aTmp ), rOle2Obj.GetAspect() );
                    mpImpl->aPersistName = aTmp;
                    CheckFileLink_Impl();
                }

                Connect();

                /* only needed for MSOLE-Objects, now handled inside implementation of Object
                if ( xObjRef.is() && rOle2Obj.xObjRef.is() && rOle2Obj.GetAspect() != embed::Aspects::MSOLE_ICON )
                {
                    try
                    {
                        awt::Size aVisSize = rOle2Obj.xObjRef->getVisualAreaSize( rOle2Obj.GetAspect() );
                        if( rOle2Obj.xObjRef->getMapUnit( rOle2Obj.GetAspect() ) == xObjRef->getMapUnit( GetAspect() ) )
                        xObjRef->setVisualAreaSize( GetAspect(), aVisSize );
                    }
                    catch ( embed::WrongStateException& )
                    {
                        // setting of VisArea not necessary for objects that don't cache it in loaded state
                    }
                    catch( embed::NoVisualAreaSizeException& )
                    {
                        // objects my not have visual areas
                    }
                    catch( uno::Exception& e )
                    {
                        (void)e;
                        DBG_ERROR( "SdrOle2Obj::operator=(), unexcpected exception caught!" );
                    }
                }                                                                            */
            }
        }
    }
}

// -----------------------------------------------------------------------------

FASTBOOL SdrOle2Obj::HasSpecialDrag() const
{
    return FALSE;
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
        SfxInPlaceClient* pClient = SfxInPlaceClient::GetClient( pModel->GetPersist(), xObjRef.GetObject() );
        sal_Bool bHasOwnClient =
                        ( mpImpl->pLightClient
                        && xObjRef->getClientSite() == uno::Reference< embed::XEmbeddedClient >( mpImpl->pLightClient ) );

        if ( pClient || bHasOwnClient )
        {
            // TODO/LATER: IMHO we need to do similar things when object is UIActive or OutplaceActive?! (MBA)
            if ( (nMiscStatus & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE) &&
                    svt::EmbeddedObjectRef::TryRunningState( xObjRef.GetObject() )
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
                OSL_ASSERT( pClient );
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
            if ( !SfxInPlaceClient::GetClient( pModel->GetPersist(), xObjRef.GetObject() )
              && !( mpImpl->pLightClient && xObjRef->getClientSite() == uno::Reference< embed::XEmbeddedClient >( mpImpl->pLightClient ) ) )
            {
                AddOwnLightClient();
            }
        }
    }

    SdrRectObj::NbcResize(rRef,xFact,yFact);
    if (aGeo.nShearWink!=0 || aGeo.nDrehWink!=0) { // kleine Korrekturen
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

sal_Bool SdrOle2Obj::Unload( const uno::Reference< embed::XEmbeddedObject >& xObj, sal_Int64 nAspect )
{
    sal_Bool bResult = sal_False;

    sal_Int32 nState = xObj->getCurrentState();
    if ( nState == embed::EmbedStates::LOADED )
    {
        bResult = sal_True;
    }
    else
    {
        sal_Int64 nMiscStatus = xObj->getStatus( nAspect );
        uno::Reference < util::XModifiable > xModifiable( xObj->getComponent(), uno::UNO_QUERY );

        if ( embed::EmbedMisc::MS_EMBED_ALWAYSRUN != ( nMiscStatus & embed::EmbedMisc::MS_EMBED_ALWAYSRUN ) &&
        embed::EmbedMisc::EMBED_ACTIVATEIMMEDIATELY != ( nMiscStatus & embed::EmbedMisc::EMBED_ACTIVATEIMMEDIATELY ) &&
        !( xModifiable.is() && xModifiable->isModified() ) &&
        !( nState == embed::EmbedStates::INPLACE_ACTIVE || nState == embed::EmbedStates::UI_ACTIVE || nState == embed::EmbedStates::ACTIVE ) )
        {
            try
            {
                xObj->changeState( embed::EmbedStates::LOADED );
                bResult = sal_True;
            }
            catch( ::com::sun::star::uno::Exception& e )
            {
                (void)e;
                DBG_ERROR(
                    (OString("SdrOle2Obj::Unload=(), "
                            "exception caught: ") +
                    rtl::OUStringToOString(
                        comphelper::anyToString( cppu::getCaughtException() ),
                        RTL_TEXTENCODING_UTF8 )).getStr() );
            }
        }
    }

    return bResult;
}

// -----------------------------------------------------------------------------

BOOL SdrOle2Obj::Unload()
{
    BOOL bUnloaded = FALSE;

    if( xObjRef.is() )
    {
        //TODO/LATER: no refcounting tricks anymore!
        //"customers" must register as state change listeners
        //Nicht notwendig im Doc DTor (MM)
        //ULONG nRefCount = (*ppObjRef)->GetRefCount();
        // prevent Unload if there are external references
        //if( nRefCount > 2 )
        //    return FALSE;
        //DBG_ASSERT( nRefCount == 2, "Wrong RefCount for unload" );
    }
    else
        bUnloaded = TRUE;

    if ( pModel && xObjRef.is() )
    {
        bUnloaded = Unload( xObjRef.GetObject(), GetAspect() );
    }

    return bUnloaded;
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::CreatePersistName( SfxObjectShell* pPers )
{
    mpImpl->aPersistName = pPers->GetEmbeddedObjectContainer().CreateUniqueObjectName();
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::GetObjRef_Impl()
{
    if ( !xObjRef.is() && mpImpl->aPersistName.Len() && pModel && pModel->GetPersist() )
    {
        // #107645#
        // Only try loading if it did not went wrong up to now
        if(!mpImpl->mbLoadingOLEObjectFailed)
        {
            xObjRef.Assign( pModel->GetPersist()->GetEmbeddedObjectContainer().GetEmbeddedObject( mpImpl->aPersistName ), GetAspect() );
            CheckFileLink_Impl();

            // #107645#
            // If loading of OLE object failed, remember that to not invoke a endless
            // loop trying to load it again and again.
            if( xObjRef.is() )
            {
                mpImpl->mbLoadingOLEObjectFailed = sal_True;
            }

            // #108759# For math objects, set closed state to transparent
            if( ImplIsMathObj( xObjRef.GetObject() ) )
                SetClosedObj( false );
        }

        if ( xObjRef.is() )
        {
            if( !IsEmptyPresObj() )
            {
                // #75637# remember modified status of model
                const sal_Bool bWasChanged(pModel ? pModel->IsChanged() : sal_False);

                // perhaps preview not valid anymore
                // #75637# This line changes the modified state of the model
                SetGraphic_Impl( NULL );

                // #75637# if status was not set before, force it back
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

// #109985#
sal_Bool SdrOle2Obj::IsChart() const
{
    if ( !xObjRef.is() )
        return false;

    SvGlobalName aObjClsId( xObjRef->getClassID() );
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
uno::Reference< frame::XModel > SdrOle2Obj::GetParentXModel()
{
    uno::Reference< frame::XModel > xResult;

    SfxObjectShell* pPers = pModel ? pModel->GetPersist() : NULL;
    if ( pPers )
        xResult = pPers->GetModel();

    return xResult;
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

// eof
