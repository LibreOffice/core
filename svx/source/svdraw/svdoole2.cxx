/*************************************************************************
 *
 *  $RCSfile: svdoole2.cxx,v $
 *
 *  $Revision: 1.60 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-12 12:14:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "svdoole2.hxx"

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
#include <svdpagv.hxx>
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

#include <cppuhelper/implbase2.hxx>

#include <svtools/solar.hrc>
#include "svdxout.hxx"
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif

#include "svdpagv.hxx"
#include "svdmodel.hxx"
#include "svdio.hxx"
#include "svdglob.hxx"  // Stringcache
#include "svdstr.hrc"   // Objektname
#include "svdetc.hxx"
#include "svdview.hxx"
#include "unomlstr.hxx"
#include "impgrf.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;

class SdrEmbeddedObjectStateListener_Impl : public ::cppu::WeakImplHelper1< embed::XStateChangeListener >
{
    SdrOle2Obj* mpObj;
public:
    SdrEmbeddedObjectStateListener_Impl( SdrOle2Obj* pObj );
    void Release();
    virtual void SAL_CALL changingState( const ::com::sun::star::lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (::com::sun::star::embed::WrongStateException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL stateChanged( const ::com::sun::star::lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
};

SdrEmbeddedObjectStateListener_Impl::SdrEmbeddedObjectStateListener_Impl( SdrOle2Obj* pObj )
: mpObj( pObj )
{
}

void SAL_CALL SdrEmbeddedObjectStateListener_Impl::changingState( const ::com::sun::star::lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (::com::sun::star::embed::WrongStateException, ::com::sun::star::uno::RuntimeException)
{
}

void SdrEmbeddedObjectStateListener_Impl::Release()
{
    mpObj=0;
    release();
}

void SAL_CALL SdrEmbeddedObjectStateListener_Impl::stateChanged( const ::com::sun::star::lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (::com::sun::star::uno::RuntimeException)
{
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

void SAL_CALL SdrEmbeddedObjectStateListener_Impl::disposing( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException)
{
    GetSdrGlobalData().GetOLEObjCache().RemoveObj(mpObj);
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

void SdrEmbedObjectLink::DataChanged( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue )
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
    SdrEmbeddedObjectStateListener_Impl* pStateListener;

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
    mpImpl->pStateListener = 0;

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

    if ( mpImpl->pStateListener )
        mpImpl->pStateListener->Release();

    delete mpImpl;
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
                if ( !rContainer.HasEmbeddedObject( mpImpl->aPersistName ) )
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
                if ( !mpImpl->pStateListener )
                {
                    mpImpl->pStateListener = new SdrEmbeddedObjectStateListener_Impl( this );
                    mpImpl->pStateListener->acquire();
                }

                xObjRef->addStateChangeListener ( mpImpl->pStateListener );
                if ( xObjRef->getCurrentState() != embed::EmbedStates::LOADED )
                    GetSdrGlobalData().GetOLEObjCache().InsertObj(this);

                CheckFileLink_Impl();
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

        if ( xObjRef.is() && mpImpl->pStateListener )
        {
            xObjRef->removeStateChangeListener ( mpImpl->pStateListener );
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
    if( pModel && !pModel->isLocked() )
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

FASTBOOL SdrOle2Obj::HasSetName() const
{
    return TRUE;
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::SetName(const XubString& rStr)
{
    aName = rStr;
    SetChanged();
}

// -----------------------------------------------------------------------------

XubString SdrOle2Obj::GetName() const
{
    return aName;
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
            if ((rInfoRec.nPaintMode & SDRPAINTMODE_DRAFTGRAF) ==0)
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
            else if( ( rInfoRec.nPaintMode & SDRPAINTMODE_HIDEDRAFTGRAF ) == 0 )
            { // sonst SDRPAINTMODE_DRAFTGRAF
                Polygon aPoly(Rect2Poly(aRect,aGeo));
                pOut->SetLineColor(Color(COL_BLACK));
                pOut->DrawPolyLine(aPoly);
                pOut->DrawLine(aPoly[0],aPoly[2]);
                pOut->DrawLine(aPoly[1],aPoly[3]);
            }
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
        if(!rInfoRec.bPrinter && rInfoRec.aPaintLayer.IsSet((sal_uInt8)nLayerId))
        {
            OutputDevice* pOutDev = rOut.GetOutDev();

            pOutDev->SetFillColor();
            pOutDev->SetLineColor(Color(COL_LIGHTGRAY));
            pOutDev->DrawRect(aOutRect);

            Bitmap aBitmap(ResId(BMP_OLEOBJ, ImpGetResMgr()));
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

void SdrOle2Obj::PaintGraphic_Impl( XOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec, sal_Bool bActive ) const
{
    Graphic* pGr = GetGraphic();
    OutputDevice* pOutDev=rOut.GetOutDev();
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

        aName = rOle2Obj.aName;
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

        if( pModel && rObj.GetModel() )
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
                    xObjRef.Assign( pDestPers->GetEmbeddedObjectContainer().CopyAndGetEmbeddedObject( rContainer, xObj, aTmp ) );
                    mpImpl->aPersistName = aTmp;
                    CheckFileLink_Impl();
                }

                Connect();

                if ( xObjRef.is() && rOle2Obj.xObjRef.is() )
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
                }
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
    GetObjRef();
    if (xObjRef.is())
    {
        OSL_ASSERT( pModel );
        sal_Int64 nMiscStatus = xObjRef->getStatus( GetAspect() );
        if ( (nMiscStatus & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE) && svt::EmbeddedObjectRef::TryRunningState( xObjRef.GetObject() ) )
        {
            // server wants to resize itself (f.e. Chart wants to recalculate the layout)
            MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObjRef->getMapUnit( GetAspect() ) );
            Rectangle aVisArea = OutputDevice::LogicToLogic( aRect, pModel->GetScaleUnit(), aMapUnit);
            awt::Size aSz;
            aSz.Width = aVisArea.GetSize().Width();
            aSz.Height = aVisArea.GetSize().Height();
            xObjRef->setVisualAreaSize( GetAspect(), aSz );

            try
            {
                aSz = xObjRef->getVisualAreaSize( GetAspect() );
            }
            catch( embed::NoVisualAreaSizeException& )
            {}

            Rectangle aAcceptedVisArea;
            aAcceptedVisArea.SetSize( Size( aSz.Width, aSz.Height ) );
            //Rectangle aAcceptedVisArea(rIPRef->GetVisArea());
            if (aVisArea.GetSize() != aAcceptedVisArea.GetSize())
            {
                // server changed VisArea to its liking
                MapUnit aNewMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObjRef->getMapUnit( GetAspect() ) );
                aRect.SetSize(OutputDevice::LogicToLogic( aAcceptedVisArea.GetSize(), aNewMapUnit, pModel->GetScaleUnit()));
            }

            xObjRef.UpdateReplacement();
        }
        else
        {
            // change object scaling
            SfxInPlaceClient* pClient = SfxInPlaceClient::GetClient( pModel->GetPersist(), xObjRef.GetObject() );
            if ( pClient && xObjRef.is() )
            {
                // TODO/LEAN: to avoid rounding errors scaling always uses the VisArea.
                // If we don't cache it for own objects also we must load the object here
                awt::Size aObjSize;
                try
                {
                    aObjSize = xObjRef->getVisualAreaSize( GetAspect() );
                }
                catch( embed::NoVisualAreaSizeException& )
                {
                    OSL_ENSURE( sal_False, "Can not get visual area size!\n" );
                    aObjSize.Width = 5000;
                    aObjSize.Height = 5000;
                }

                MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObjRef->getMapUnit( GetAspect() ) );
                Size aObjAreaSize(aObjSize.Width, aObjSize.Height);
                aObjAreaSize = OutputDevice::LogicToLogic( aObjAreaSize, aMapUnit, pModel->GetScaleUnit() );

                Size aSize = aRect.GetSize();
                Fraction aScaleWidth (aSize.Width(),  aObjAreaSize.Width() );
                Fraction aScaleHeight(aSize.Height(), aObjAreaSize.Height() );

                // reduce to 10 binary digits
                Kuerzen(aScaleHeight, 10);
                Kuerzen(aScaleWidth,  10);

                pClient->SetSizeScale(aScaleWidth, aScaleHeight);

                Rectangle aScaleRect(aRect.TopLeft(), aObjAreaSize);
                pClient->SetObjArea(aScaleRect);
            }
        }
    }
}

// -----------------------------------------------------------------------------

void SdrOle2Obj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
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

void SdrOle2Obj::NbcMove(const Size& rSize)
{
    SdrRectObj::NbcMove(rSize);
    if( pModel && !pModel->isLocked() )
        ImpSetVisAreaSize();
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
        sal_Int64 nMiscStatus = xObjRef->getStatus( GetAspect() );
        sal_Int32 nState = xObjRef->getCurrentState();
        if ( nState == embed::EmbedStates::LOADED )
        {
            bUnloaded = TRUE;
            return bUnloaded;
        }
        else
        {
            uno::Reference < util::XModifiable > xModifiable( xObjRef->getComponent(), uno::UNO_QUERY );

            if ( embed::EmbedMisc::MS_EMBED_ALWAYSRUN != nMiscStatus    &&
            //TODO/LATER: no refcounting tricks anymore!
            //1 < (*ppObjRef)->GetRefCount()                   &&
            !( xModifiable.is() && xModifiable->isModified() ) &&
            !( nState == embed::EmbedStates::INPLACE_ACTIVE || nState == embed::EmbedStates::UI_ACTIVE ) )
            {
                try
                {
                    xObjRef->changeState( embed::EmbedStates::LOADED );
                    bUnloaded = TRUE;
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
            xObjRef.Assign( pModel->GetPersist()->GetEmbeddedObjectContainer().GetEmbeddedObject( mpImpl->aPersistName ) );
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

// eof
