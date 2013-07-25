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

#include <svx/sdr/contact/viewcontactofunocontrol.hxx>
#include <svx/sdr/contact/viewobjectcontactofunocontrol.hxx>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/io/XObjectInputStream.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdmodel.hxx>
#include "svx/svdglob.hxx"  // string cache
#include "svx/svdstr.hrc"   // the object's name
#include <svx/svdetc.hxx>
#include <svx/svdview.hxx>
#include <svx/svdorect.hxx>
#include "svx/svdviter.hxx"
#include <rtl/ref.hxx>
#include <set>
#include <memory>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <tools/diagnose_ex.h>
#include <svx/svdograf.hxx>

using namespace ::com::sun::star;
using namespace ::sdr::contact;

//************************************************************
//   Defines
//************************************************************

//************************************************************
//   Helper class SdrControlEventListenerImpl
//************************************************************
#include <com/sun/star/lang/XEventListener.hpp>

#include <cppuhelper/implbase1.hxx>

// =============================================================================
class SdrControlEventListenerImpl : public ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener >
{
protected:
    SdrUnoObj*                  pObj;

public:
    SdrControlEventListenerImpl(SdrUnoObj* _pObj)
    :   pObj(_pObj)
    {}

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    void StopListening(const uno::Reference< lang::XComponent >& xComp);
    void StartListening(const uno::Reference< lang::XComponent >& xComp);
};

// XEventListener
void SAL_CALL SdrControlEventListenerImpl::disposing( const ::com::sun::star::lang::EventObject& /*Source*/)
    throw(::com::sun::star::uno::RuntimeException)
{
    if (pObj)
    {
        pObj->xUnoControlModel = NULL;
    }
}

void SdrControlEventListenerImpl::StopListening(const uno::Reference< lang::XComponent >& xComp)
{
    if (xComp.is())
        xComp->removeEventListener(this);
}

void SdrControlEventListenerImpl::StartListening(const uno::Reference< lang::XComponent >& xComp)
{
    if (xComp.is())
        xComp->addEventListener(this);
}

// =============================================================================
struct SAL_DLLPRIVATE SdrUnoObjDataHolder
{
    mutable ::rtl::Reference< SdrControlEventListenerImpl >
                                    pEventListener;
};

// =============================================================================
namespace
{
    void lcl_ensureControlVisibility( SdrView* _pView, const SdrUnoObj* _pObject, bool _bVisible )
    {
        OSL_PRECOND( _pObject, "lcl_ensureControlVisibility: no object -> no survival!" );

        SdrPageView* pPageView = _pView ? _pView->GetSdrPageView() : NULL;
        DBG_ASSERT( pPageView, "lcl_ensureControlVisibility: no view found!" );
        if ( !pPageView )
            return;

        ViewContact& rUnoControlContact( _pObject->GetViewContact() );

        for ( sal_uInt32 i = 0; i < pPageView->PageWindowCount(); ++i )
        {
            const SdrPageWindow* pPageWindow = pPageView->GetPageWindow( i );
            DBG_ASSERT( pPageWindow, "lcl_ensureControlVisibility: invalid PageViewWindow!" );
            if ( !pPageWindow )
                continue;

            if ( !pPageWindow->HasObjectContact() )
                continue;

            ObjectContact& rPageViewContact( pPageWindow->GetObjectContact() );
            const ViewObjectContact& rViewObjectContact( rUnoControlContact.GetViewObjectContact( rPageViewContact ) );
            const ViewObjectContactOfUnoControl* pUnoControlContact = dynamic_cast< const ViewObjectContactOfUnoControl* >( &rViewObjectContact );
            DBG_ASSERT( pUnoControlContact, "lcl_ensureControlVisibility: wrong ViewObjectContact type!" );
            if ( !pUnoControlContact )
                continue;

            pUnoControlContact->ensureControlVisibility( _bVisible );
        }
    }
}

//************************************************************
//   SdrUnoObj
//************************************************************

TYPEINIT1(SdrUnoObj, SdrRectObj);

SdrUnoObj::SdrUnoObj(const String& rModelName, sal_Bool _bOwnUnoControlModel)
:   m_pImpl( new SdrUnoObjDataHolder ),
    bOwnUnoControlModel( _bOwnUnoControlModel )
{
    bIsUnoObj = sal_True;

    m_pImpl->pEventListener = new SdrControlEventListenerImpl(this);

    // only an owner may create independently
    if (rModelName.Len())
        CreateUnoControlModel(rModelName);
}

SdrUnoObj::SdrUnoObj(const String& rModelName,
                     const uno::Reference< lang::XMultiServiceFactory >& rxSFac,
                     sal_Bool _bOwnUnoControlModel)
:   m_pImpl( new SdrUnoObjDataHolder ),
    bOwnUnoControlModel( _bOwnUnoControlModel )
{
    bIsUnoObj = sal_True;

    m_pImpl->pEventListener = new SdrControlEventListenerImpl(this);

    // only an owner may create independently
    if (rModelName.Len())
        CreateUnoControlModel(rModelName,rxSFac);
}

SdrUnoObj::~SdrUnoObj()
{
    try
    {
        // clean up the control model
        uno::Reference< lang::XComponent > xComp(xUnoControlModel, uno::UNO_QUERY);
        if (xComp.is())
        {
            // is the control model owned by its environment?
            uno::Reference< container::XChild > xContent(xUnoControlModel, uno::UNO_QUERY);
            if (xContent.is() && !xContent->getParent().is())
                xComp->dispose();
            else
                m_pImpl->pEventListener->StopListening(xComp);
        }
    }
    catch( const uno::Exception& )
    {
        OSL_FAIL( "SdrUnoObj::~SdrUnoObj: caught an exception!" );
    }
    delete m_pImpl;
}

void SdrUnoObj::SetModel(SdrModel* pNewModel)
{
    SdrRectObj::SetModel(pNewModel);
}

void SdrUnoObj::SetPage(SdrPage* pNewPage)
{
    SdrRectObj::SetPage(pNewPage);
}

void SdrUnoObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bRotateFreeAllowed        =   sal_False;
    rInfo.bRotate90Allowed          =   sal_False;
    rInfo.bMirrorFreeAllowed        =   sal_False;
    rInfo.bMirror45Allowed          =   sal_False;
    rInfo.bMirror90Allowed          =   sal_False;
    rInfo.bTransparenceAllowed = sal_False;
    rInfo.bGradientAllowed = sal_False;
    rInfo.bShearAllowed             =   sal_False;
    rInfo.bEdgeRadiusAllowed        =   sal_False;
    rInfo.bNoOrthoDesired           =   sal_False;
    rInfo.bCanConvToPath            =   sal_False;
    rInfo.bCanConvToPoly            =   sal_False;
    rInfo.bCanConvToPathLineToArea  =   sal_False;
    rInfo.bCanConvToPolyLineToArea  =   sal_False;
    rInfo.bCanConvToContour = sal_False;
}

sal_uInt16 SdrUnoObj::GetObjIdentifier() const
{
    return sal_uInt16(OBJ_UNO);
}

void SdrUnoObj::SetContextWritingMode( const sal_Int16 _nContextWritingMode )
{
    try
    {
        uno::Reference< beans::XPropertySet > xModelProperties( GetUnoControlModel(), uno::UNO_QUERY_THROW );
        xModelProperties->setPropertyValue(
            OUString(  "ContextWritingMode"  ),
            uno::makeAny( _nContextWritingMode )
        );
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

OUString SdrUnoObj::TakeObjNameSingul() const
{
    OUStringBuffer sName(ImpGetResStr(STR_ObjNameSingulUno));

    OUString aName(GetName());
    if (!aName.isEmpty())
    {
        sName.append(' ');
        sName.append('\'');
        sName.append(aName);
        sName.append('\'');
    }

    return sName.makeStringAndClear();
}

void SdrUnoObj::TakeObjNamePlural(XubString& rName) const
{
    rName = ImpGetResStr(STR_ObjNamePluralUno);
}

SdrUnoObj* SdrUnoObj::Clone() const
{
    return CloneHelper< SdrUnoObj >();
}

SdrUnoObj& SdrUnoObj::operator= (const SdrUnoObj& rObj)
{
    if( this == &rObj )
        return *this;
    SdrRectObj::operator= (rObj);

    // release the reference to the current control model
    SetUnoControlModel( NULL );

    aUnoControlModelTypeName = rObj.aUnoControlModelTypeName;
    aUnoControlTypeName = rObj.aUnoControlTypeName;

    // copy the uno control model
    const uno::Reference< awt::XControlModel > xSourceControlModel( rObj.GetUnoControlModel(), uno::UNO_QUERY );
    if ( xSourceControlModel.is() )
    {
        try
        {
            uno::Reference< util::XCloneable > xClone( xSourceControlModel, uno::UNO_QUERY_THROW );
            xUnoControlModel.set( xClone->createClone(), uno::UNO_QUERY_THROW );
        }
        catch( const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    // get service name of the control from the control model
    uno::Reference< beans::XPropertySet > xSet(xUnoControlModel, uno::UNO_QUERY);
    if (xSet.is())
    {
        uno::Any aValue( xSet->getPropertyValue("DefaultControl") );
        OUString aStr;

        if( aValue >>= aStr )
            aUnoControlTypeName = String(aStr);
    }

    uno::Reference< lang::XComponent > xComp(xUnoControlModel, uno::UNO_QUERY);
    if (xComp.is())
        m_pImpl->pEventListener->StartListening(xComp);
    return *this;
}

void SdrUnoObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    SdrRectObj::NbcResize(rRef,xFact,yFact);

    if (aGeo.nShearWink!=0 || aGeo.nDrehWink!=0)
    {
        // small correctures
        if (aGeo.nDrehWink>=9000 && aGeo.nDrehWink<27000)
        {
            aRect.Move(aRect.Left()-aRect.Right(),aRect.Top()-aRect.Bottom());
        }

        aGeo.nDrehWink  = 0;
        aGeo.nShearWink = 0;
        aGeo.nSin       = 0.0;
        aGeo.nCos       = 1.0;
        aGeo.nTan       = 0.0;
        SetRectsDirty();
    }
}

// -----------------------------------------------------------------------------

bool SdrUnoObj::hasSpecialDrag() const
{
    // no special drag; we have no rounding rect and
    // do want frame handles
    return false;
}

bool SdrUnoObj::supportsFullDrag() const
{
    // overloaded to have the possibility to enable/disable in debug and
    // to ckeck some things out. Current solution is working, so default is
    // enabled
    static bool bDoSupportFullDrag(true);

    return bDoSupportFullDrag;
}

SdrObject* SdrUnoObj::getFullDragClone() const
{
    SdrObject* pRetval = 0;
    static bool bHandleSpecial(false);

    if(bHandleSpecial)
    {
        // special handling for SdrUnoObj (FormControl). Create a SdrGrafObj
        // for drag containing the graphical representation. This does not work too
        // well, so the default is to simply clone
        pRetval = new SdrGrafObj(SdrDragView::GetObjGraphic(GetModel(), this), GetLogicRect());
    }
    else
    {
        // call parent (simply clone)
        pRetval = SdrRectObj::getFullDragClone();
    }

    return pRetval;
}

// -----------------------------------------------------------------------------
void SdrUnoObj::NbcSetLayer( SdrLayerID _nLayer )
{
    if ( GetLayer() == _nLayer )
    {   // redundant call -> not interested in doing anything here
        SdrRectObj::NbcSetLayer( _nLayer );
        return;
    }

    // we need some special handling here in case we're moved from an invisible layer
    // to a visible one, or vice versa
    // (relative to a layer. Remember that the visibility of a layer is a view attribute
    // - the same layer can be visible in one view, and invisible in another view, at the
    // same time)

    // collect all views in which our old layer is visible
    ::std::set< SdrView* > aPreviouslyVisible;

    {
        SdrViewIter aIter( this );
        for ( SdrView* pView = aIter.FirstView(); pView; pView = aIter.NextView() )
            aPreviouslyVisible.insert( pView );
    }

    SdrRectObj::NbcSetLayer( _nLayer );

    // collect all views in which our new layer is visible
    ::std::set< SdrView* > aNewlyVisible;

    {
        SdrViewIter aIter( this );
        for ( SdrView* pView = aIter.FirstView(); pView; pView = aIter.NextView() )
        {
            ::std::set< SdrView* >::const_iterator aPrevPos = aPreviouslyVisible.find( pView );
            if ( aPreviouslyVisible.end() != aPrevPos )
            {   // in pView, we were visible _before_ the layer change, and are
                // visible _after_ the layer change, too
                // -> we're not interested in this view at all
                aPreviouslyVisible.erase( aPrevPos );
            }
            else
            {
                // in pView, we were visible _before_ the layer change, and are
                // _not_ visible after the layer change
                // => remember this view, as our visibility there changed
                aNewlyVisible.insert( pView );
            }
        }
    }

    // now aPreviouslyVisible contains all views where we became invisible
    ::std::set< SdrView* >::const_iterator aLoopViews;
    for (   aLoopViews = aPreviouslyVisible.begin();
            aLoopViews != aPreviouslyVisible.end();
            ++aLoopViews
        )
    {
        lcl_ensureControlVisibility( *aLoopViews, this, false );
    }

    // and aNewlyVisible all views where we became visible
    for (   aLoopViews = aNewlyVisible.begin();
            aLoopViews != aNewlyVisible.end();
            ++aLoopViews
        )
    {
        lcl_ensureControlVisibility( *aLoopViews, this, true );
    }
}

void SdrUnoObj::CreateUnoControlModel(const String& rModelName)
{
    DBG_ASSERT(!xUnoControlModel.is(), "model already exists");

    aUnoControlModelTypeName = rModelName;

    uno::Reference< awt::XControlModel >   xModel;
    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    if (aUnoControlModelTypeName.Len() )
    {
        xModel = uno::Reference< awt::XControlModel >(xContext->getServiceManager()->createInstanceWithContext(
            aUnoControlModelTypeName, xContext), uno::UNO_QUERY);

        if (xModel.is())
            SetChanged();
    }

    SetUnoControlModel(xModel);
}

void SdrUnoObj::CreateUnoControlModel(const String& rModelName,
                                      const uno::Reference< lang::XMultiServiceFactory >& rxSFac)
{
    DBG_ASSERT(!xUnoControlModel.is(), "model already exists");

    aUnoControlModelTypeName = rModelName;

    uno::Reference< awt::XControlModel >   xModel;
    if (aUnoControlModelTypeName.Len() && rxSFac.is() )
    {
        xModel = uno::Reference< awt::XControlModel >(rxSFac->createInstance(
            aUnoControlModelTypeName), uno::UNO_QUERY);

        if (xModel.is())
            SetChanged();
    }

    SetUnoControlModel(xModel);
}

void SdrUnoObj::SetUnoControlModel( const uno::Reference< awt::XControlModel >& xModel)
{
    if (xUnoControlModel.is())
    {
        uno::Reference< lang::XComponent > xComp(xUnoControlModel, uno::UNO_QUERY);
        if (xComp.is())
            m_pImpl->pEventListener->StopListening(xComp);
    }

    xUnoControlModel = xModel;

    // control model has to contain service name of the control
    if (xUnoControlModel.is())
    {
        uno::Reference< beans::XPropertySet > xSet(xUnoControlModel, uno::UNO_QUERY);
        if (xSet.is())
        {
            uno::Any aValue( xSet->getPropertyValue(String("DefaultControl", osl_getThreadTextEncoding())) );
            OUString aStr;
            if( aValue >>= aStr )
                aUnoControlTypeName = String(aStr);
        }

        uno::Reference< lang::XComponent > xComp(xUnoControlModel, uno::UNO_QUERY);
        if (xComp.is())
            m_pImpl->pEventListener->StartListening(xComp);
    }

    // invalidate all ViewObject contacts
    ViewContactOfUnoControl* pVC = NULL;
    if ( impl_getViewContact( pVC ) )
    {
        // flushViewObjectContacts() removes all existing VOCs for the local DrawHierarchy. This
        // is always allowed since they will be re-created on demand (and with the changed model)
        GetViewContact().flushViewObjectContacts(true);
    }
}

//------------------------------------------------------------------------
uno::Reference< awt::XControl > SdrUnoObj::GetUnoControl(const SdrView& _rView, const OutputDevice& _rOut) const
{
    uno::Reference< awt::XControl > xControl;

    SdrPageView* pPageView = _rView.GetSdrPageView();
    OSL_ENSURE( GetPage() == pPageView->GetPage(), "SdrUnoObj::GetUnoControl: This object is not displayed in that particular view!" );
    if ( GetPage() != pPageView->GetPage() )
        return NULL;

    SdrPageWindow* pPageWindow = pPageView ? pPageView->FindPageWindow( _rOut ) : NULL;
    OSL_ENSURE( pPageWindow, "SdrUnoObj::GetUnoControl: did not find my SdrPageWindow!" );
    if ( !pPageWindow )
        return NULL;

    ViewObjectContact& rViewObjectContact( GetViewContact().GetViewObjectContact( pPageWindow->GetObjectContact() ) );
    ViewObjectContactOfUnoControl* pUnoContact = dynamic_cast< ViewObjectContactOfUnoControl* >( &rViewObjectContact );
    OSL_ENSURE( pUnoContact, "SdrUnoObj::GetUnoControl: wrong contact type!" );
    if ( pUnoContact )
        xControl = pUnoContact->getControl();

    return xControl;
}

//------------------------------------------------------------------------
uno::Reference< awt::XControl > SdrUnoObj::GetTemporaryControlForWindow(
    const Window& _rWindow, uno::Reference< awt::XControlContainer >& _inout_ControlContainer ) const
{
    uno::Reference< awt::XControl > xControl;

    ViewContactOfUnoControl* pVC = NULL;
    if ( impl_getViewContact( pVC ) )
        xControl = pVC->getTemporaryControlForWindow( _rWindow, _inout_ControlContainer );

    return xControl;
}

//------------------------------------------------------------------------
bool SdrUnoObj::impl_getViewContact( ViewContactOfUnoControl*& _out_rpContact ) const
{
    ViewContact& rViewContact( GetViewContact() );
    _out_rpContact = dynamic_cast< ViewContactOfUnoControl* >( &rViewContact );
    DBG_ASSERT( _out_rpContact, "SdrUnoObj::impl_getViewContact: could not find my ViewContact!" );
    return ( _out_rpContact != NULL );
}

//------------------------------------------------------------------------
::sdr::contact::ViewContact* SdrUnoObj::CreateObjectSpecificViewContact()
{
  return new ::sdr::contact::ViewContactOfUnoControl( *this );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
