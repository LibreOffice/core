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

#include <sdr/contact/viewcontactofunocontrol.hxx>
#include <sdr/contact/viewobjectcontactofunocontrol.hxx>
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
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdmodel.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svdetc.hxx>
#include <svx/svdview.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdviter.hxx>
#include <rtl/ref.hxx>
#include <set>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <tools/diagnose_ex.h>
#include <svx/svdograf.hxx>

using namespace ::com::sun::star;
using namespace sdr::contact;


//   Defines


//   Helper class SdrControlEventListenerImpl

#include <com/sun/star/lang/XEventListener.hpp>

#include <cppuhelper/implbase.hxx>


class SdrControlEventListenerImpl : public ::cppu::WeakImplHelper< css::lang::XEventListener >
{
protected:
    SdrUnoObj*                  pObj;

public:
    explicit SdrControlEventListenerImpl(SdrUnoObj* _pObj)
    :   pObj(_pObj)
    {}

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    void StopListening(const uno::Reference< lang::XComponent >& xComp);
    void StartListening(const uno::Reference< lang::XComponent >& xComp);
};

// XEventListener
void SAL_CALL SdrControlEventListenerImpl::disposing( const css::lang::EventObject& /*Source*/)
{
    if (pObj)
    {
        pObj->xUnoControlModel = nullptr;
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


struct SdrUnoObjDataHolder
{
    mutable ::rtl::Reference< SdrControlEventListenerImpl >
                                    pEventListener;
};


namespace
{
    void lcl_ensureControlVisibility( SdrView const * _pView, const SdrUnoObj* _pObject, bool _bVisible )
    {
        OSL_PRECOND( _pObject, "lcl_ensureControlVisibility: no object -> no survival!" );

        SdrPageView* pPageView = _pView ? _pView->GetSdrPageView() : nullptr;
        DBG_ASSERT( pPageView, "lcl_ensureControlVisibility: no view found!" );
        if ( !pPageView )
            return;

        ViewContact& rUnoControlContact( _pObject->GetViewContact() );

        for ( sal_uInt32 i = 0; i < pPageView->PageWindowCount(); ++i )
        {
            SdrPageWindow* pPageWindow = pPageView->GetPageWindow( i );
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

SdrUnoObj::SdrUnoObj(
    SdrModel& rSdrModel,
    const OUString& rModelName)
:   SdrRectObj(rSdrModel),
    m_pImpl( new SdrUnoObjDataHolder )
{
    bIsUnoObj = true;

    m_pImpl->pEventListener = new SdrControlEventListenerImpl(this);

    // only an owner may create independently
    if (!rModelName.isEmpty())
        CreateUnoControlModel(rModelName);
}

SdrUnoObj::SdrUnoObj(
    SdrModel& rSdrModel,
    const OUString& rModelName,
    const uno::Reference< lang::XMultiServiceFactory >& rxSFac)
:   SdrRectObj(rSdrModel),
    m_pImpl( new SdrUnoObjDataHolder )
{
    bIsUnoObj = true;

    m_pImpl->pEventListener = new SdrControlEventListenerImpl(this);

    // only an owner may create independently
    if (!rModelName.isEmpty())
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
}

void SdrUnoObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bRotateFreeAllowed        =   false;
    rInfo.bRotate90Allowed          =   false;
    rInfo.bMirrorFreeAllowed        =   false;
    rInfo.bMirror45Allowed          =   false;
    rInfo.bMirror90Allowed          =   false;
    rInfo.bTransparenceAllowed = false;
    rInfo.bShearAllowed             =   false;
    rInfo.bEdgeRadiusAllowed        =   false;
    rInfo.bNoOrthoDesired           =   false;
    rInfo.bCanConvToPath            =   false;
    rInfo.bCanConvToPoly            =   false;
    rInfo.bCanConvToPathLineToArea  =   false;
    rInfo.bCanConvToPolyLineToArea  =   false;
    rInfo.bCanConvToContour = false;
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
        xModelProperties->setPropertyValue( "ContextWritingMode", uno::makeAny( _nContextWritingMode ) );
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
}

OUString SdrUnoObj::TakeObjNameSingul() const
{
    OUStringBuffer sName(SvxResId(STR_ObjNameSingulUno));

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

OUString SdrUnoObj::TakeObjNamePlural() const
{
    return SvxResId(STR_ObjNamePluralUno);
}

SdrUnoObj* SdrUnoObj::CloneSdrObject(SdrModel& rTargetModel) const
{
    return CloneHelper< SdrUnoObj >(rTargetModel);
}

SdrUnoObj& SdrUnoObj::operator= (const SdrUnoObj& rObj)
{
    if( this == &rObj )
        return *this;
    SdrRectObj::operator= (rObj);

    // release the reference to the current control model
    SetUnoControlModel( nullptr );

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
            DBG_UNHANDLED_EXCEPTION("svx");
        }
    }

    // get service name of the control from the control model
    uno::Reference< beans::XPropertySet > xSet(xUnoControlModel, uno::UNO_QUERY);
    if (xSet.is())
    {
        uno::Any aValue( xSet->getPropertyValue("DefaultControl") );
        OUString aStr;

        if( aValue >>= aStr )
            aUnoControlTypeName = aStr;
    }

    uno::Reference< lang::XComponent > xComp(xUnoControlModel, uno::UNO_QUERY);
    if (xComp.is())
        m_pImpl->pEventListener->StartListening(xComp);
    return *this;
}

void SdrUnoObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    SdrRectObj::NbcResize(rRef,xFact,yFact);

    if (aGeo.nShearAngle!=0 || aGeo.nRotationAngle!=0)
    {
        // small correctors
        if (aGeo.nRotationAngle>=9000 && aGeo.nRotationAngle<27000)
        {
            maRect.Move(maRect.Left()-maRect.Right(),maRect.Top()-maRect.Bottom());
        }

        aGeo.nRotationAngle  = 0;
        aGeo.nShearAngle = 0;
        aGeo.nSin       = 0.0;
        aGeo.nCos       = 1.0;
        aGeo.nTan       = 0.0;
        SetRectsDirty();
    }
}


bool SdrUnoObj::hasSpecialDrag() const
{
    // no special drag; we have no rounding rect and
    // do want frame handles
    return false;
}

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
    for (const auto& rpView : aPreviouslyVisible)
    {
        lcl_ensureControlVisibility( rpView, this, false );
    }

    // and aNewlyVisible all views where we became visible
    for (const auto& rpView : aNewlyVisible)
    {
        lcl_ensureControlVisibility( rpView, this, true );
    }
}

void SdrUnoObj::CreateUnoControlModel(const OUString& rModelName)
{
    DBG_ASSERT(!xUnoControlModel.is(), "model already exists");

    aUnoControlModelTypeName = rModelName;

    uno::Reference< awt::XControlModel >   xModel;
    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    if (!aUnoControlModelTypeName.isEmpty() )
    {
        xModel.set(xContext->getServiceManager()->createInstanceWithContext(
            aUnoControlModelTypeName, xContext), uno::UNO_QUERY);

        if (xModel.is())
            SetChanged();
    }

    SetUnoControlModel(xModel);
}

void SdrUnoObj::CreateUnoControlModel(const OUString& rModelName,
                                      const uno::Reference< lang::XMultiServiceFactory >& rxSFac)
{
    DBG_ASSERT(!xUnoControlModel.is(), "model already exists");

    aUnoControlModelTypeName = rModelName;

    uno::Reference< awt::XControlModel >   xModel;
    if (!aUnoControlModelTypeName.isEmpty() && rxSFac.is() )
    {
        xModel.set(rxSFac->createInstance(aUnoControlModelTypeName), uno::UNO_QUERY);

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
            uno::Any aValue( xSet->getPropertyValue("DefaultControl") );
            OUString aStr;
            if( aValue >>= aStr )
                aUnoControlTypeName = aStr;
        }

        uno::Reference< lang::XComponent > xComp(xUnoControlModel, uno::UNO_QUERY);
        if (xComp.is())
            m_pImpl->pEventListener->StartListening(xComp);
    }

    // invalidate all ViewObject contacts
    ViewContactOfUnoControl* pVC = nullptr;
    if ( impl_getViewContact( pVC ) )
    {
        // flushViewObjectContacts() removes all existing VOCs for the local DrawHierarchy. This
        // is always allowed since they will be re-created on demand (and with the changed model)
        GetViewContact().flushViewObjectContacts();
    }
}


uno::Reference< awt::XControl > SdrUnoObj::GetUnoControl(const SdrView& _rView, const OutputDevice& _rOut) const
{
    uno::Reference< awt::XControl > xControl;

    SdrPageView* pPageView = _rView.GetSdrPageView();
    OSL_ENSURE( pPageView && getSdrPageFromSdrObject() == pPageView->GetPage(), "SdrUnoObj::GetUnoControl: This object is not displayed in that particular view!" );
    if ( !pPageView || getSdrPageFromSdrObject() != pPageView->GetPage() )
        return nullptr;

    SdrPageWindow* pPageWindow = pPageView->FindPageWindow( _rOut );
    OSL_ENSURE( pPageWindow, "SdrUnoObj::GetUnoControl: did not find my SdrPageWindow!" );
    if ( !pPageWindow )
        return nullptr;

    ViewObjectContact& rViewObjectContact( GetViewContact().GetViewObjectContact( pPageWindow->GetObjectContact() ) );
    ViewObjectContactOfUnoControl* pUnoContact = dynamic_cast< ViewObjectContactOfUnoControl* >( &rViewObjectContact );
    OSL_ENSURE( pUnoContact, "SdrUnoObj::GetUnoControl: wrong contact type!" );
    if ( pUnoContact )
        xControl = pUnoContact->getControl();

    return xControl;
}


uno::Reference< awt::XControl > SdrUnoObj::GetTemporaryControlForWindow(
    const vcl::Window& _rWindow, uno::Reference< awt::XControlContainer >& _inout_ControlContainer ) const
{
    uno::Reference< awt::XControl > xControl;

    ViewContactOfUnoControl* pVC = nullptr;
    if ( impl_getViewContact( pVC ) )
        xControl = pVC->getTemporaryControlForWindow( _rWindow, _inout_ControlContainer );

    return xControl;
}


bool SdrUnoObj::impl_getViewContact( ViewContactOfUnoControl*& _out_rpContact ) const
{
    ViewContact& rViewContact( GetViewContact() );
    _out_rpContact = dynamic_cast< ViewContactOfUnoControl* >( &rViewContact );
    DBG_ASSERT( _out_rpContact, "SdrUnoObj::impl_getViewContact: could not find my ViewContact!" );
    return ( _out_rpContact != nullptr );
}


std::unique_ptr<sdr::contact::ViewContact> SdrUnoObj::CreateObjectSpecificViewContact()
{
  return std::make_unique<sdr::contact::ViewContactOfUnoControl>( *this );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
