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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/XVclContainerPeer.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <controls/stdtabcontroller.hxx>
#include <helper/servicenames.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <comphelper/sequence.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;



StdTabController::StdTabController()
{
}

StdTabController::~StdTabController()
{
}

bool StdTabController::ImplCreateComponentSequence(
        Sequence< Reference< XControl > >&              rControls,
        const Sequence< Reference< XControlModel > >&   rModels,
        Sequence< Reference< XWindow > >&               rComponents,
        Sequence< Any>*                                 pTabStops,
        bool bPeerComponent )
{
    // Get only the requested controls
    sal_Int32 nModels = rModels.getLength();
    if (nModels != rControls.getLength())
    {
        Sequence< Reference< XControl > > aSeq( nModels );
        Reference< XControl >  xCurrentControl;

        sal_Int32 nRealControls = 0;
        for (const Reference< XControlModel >& rModel : rModels)
        {
            xCurrentControl = FindControl(rControls, rModel);
            if (xCurrentControl.is())
                aSeq[nRealControls++] = xCurrentControl;
        }
        aSeq.realloc(nRealControls);
        rControls = aSeq;
    }

    // there may be less controls than models, but never more controls than models
    assert(rControls.getLength() <= rModels.getLength());

    sal_Int32 nCtrls = rControls.getLength();
    rComponents.realloc( nCtrls );
    Reference< XWindow > * pComps = rComponents.getArray();
    Any* pTabs = nullptr;


    if ( pTabStops )
    {
        *pTabStops = Sequence< Any>( nCtrls );
        pTabs = pTabStops->getArray();
    }

    bool bOK = true;
    for ( const Reference< XControl >& xCtrl : std::as_const(rControls) )
    {
        // Get the matching control for this model
        if ( !xCtrl.is() )
        {
            SAL_WARN("toolkit", "Control not found" );
            bOK = false;
            break;
        }

        if (bPeerComponent)
            pComps->set(xCtrl->getPeer(), UNO_QUERY);
        else
            pComps->set(xCtrl, UNO_QUERY);

        // TabStop-Property
        if ( pTabs )
        {
            // opt: Constant String for TabStop name
            static constexpr OUStringLiteral aTabStopName = u"Tabstop";

            Reference< XPropertySet >  xPSet( xCtrl->getModel(), UNO_QUERY );
            Reference< XPropertySetInfo >  xInfo = xPSet->getPropertySetInfo();
            if( xInfo->hasPropertyByName( aTabStopName ) )
                *pTabs++ = xPSet->getPropertyValue( aTabStopName );
            else
                ++pTabs;
        }

        ++pComps;
    }
    return bOK;
}

void StdTabController::ImplActivateControl( bool bFirst ) const
{
    // HACK due to bug #53688#, map controls onto an interface if remote controls may occur
    Sequence< Reference< XControl > > aCtrls = const_cast<StdTabController*>(this)->getControls();
    const Reference< XControl > * pControls = aCtrls.getConstArray();
    sal_uInt32 nCount = aCtrls.getLength();

    for ( sal_uInt32 n = bFirst ? 0 : nCount; bFirst ? n < nCount : n != 0; )
    {
        sal_uInt32 nCtrl = bFirst ? n++ : --n;
        DBG_ASSERT( pControls[nCtrl].is(), "Control not in Container!" );
        if ( pControls[nCtrl].is() )
        {
            Reference< XWindowPeer >  xCP = pControls[nCtrl]->getPeer();
            if ( xCP.is() )
            {
                VCLXWindow* pC = comphelper::getUnoTunnelImplementation<VCLXWindow>( xCP );
                if ( pC && pC->GetWindow() && ( pC->GetWindow()->GetStyle() & WB_TABSTOP ) )
                {
                    pC->GetWindow()->GrabFocus();
                    break;
                }
            }
        }
    }
}

// XInterface
Any StdTabController::queryAggregation( const Type & rType )
{
    Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< XTabController* >(this),
                                        static_cast< XServiceInfo* >(this),
                                        static_cast< XTypeProvider* >(this) );
    return (aRet.hasValue() ? aRet : OWeakAggObject::queryAggregation( rType ));
}

IMPL_IMPLEMENTATION_ID( StdTabController )

// XTypeProvider
css::uno::Sequence< css::uno::Type > StdTabController::getTypes()
{
    static const css::uno::Sequence< css::uno::Type > aTypeList {
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<XTabController>::get(),
        cppu::UnoType<XServiceInfo>::get()
    };
    return aTypeList;
}

void StdTabController::setModel( const Reference< XTabControllerModel >& Model )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    mxModel = Model;
}

Reference< XTabControllerModel > StdTabController::getModel(  )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mxModel;
}

void StdTabController::setContainer( const Reference< XControlContainer >& Container )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    mxControlContainer = Container;
}

Reference< XControlContainer > StdTabController::getContainer(  )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mxControlContainer;
}

Sequence< Reference< XControl > > StdTabController::getControls(  )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    Sequence< Reference< XControl > > aSeq;

    if ( mxControlContainer.is() )
    {
        Sequence< Reference< XControlModel > > aModels = mxModel->getControlModels();

        Sequence< Reference< XControl > > xCtrls = mxControlContainer->getControls();

        sal_Int32 nCtrls = aModels.getLength();
        aSeq = Sequence< Reference< XControl > >( nCtrls );
        std::transform(aModels.begin(), aModels.end(), aSeq.begin(),
            [&xCtrls](const Reference< XControlModel >& xCtrlModel) -> Reference< XControl > {
                return FindControl( xCtrls, xCtrlModel ); });
    }
    return aSeq;
}

namespace {

struct ComponentEntry
{
    css::awt::XWindow*  pComponent;
    ::Point             aPos;
};

}

void StdTabController::autoTabOrder(  )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    DBG_ASSERT( mxControlContainer.is(), "autoTabOrder: No ControlContainer!" );
    if ( !mxControlContainer.is() )
        return;

    Sequence< Reference< XControlModel > > aSeq = mxModel->getControlModels();
    Sequence< Reference< XWindow > > aCompSeq;

    // This may return a TabController, which returns desired list of controls faster
    Sequence< Reference< XControl > > aControls = getControls();

    // #58317# Some Models may be missing from the Container. Plus there is a
    // autoTabOrder call later on.
    if( !ImplCreateComponentSequence( aControls, aSeq, aCompSeq, nullptr, false ) )
        return;

    sal_uInt32 nCtrls = aCompSeq.getLength();

    // insert sort algorithm
    std::vector< ComponentEntry > aCtrls;
    aCtrls.reserve(nCtrls);
    for ( const Reference< XWindow >& rComponent : std::as_const(aCompSeq) )
    {
        XWindow* pC = rComponent.get();
        ComponentEntry newEntry;
        newEntry.pComponent = pC;
        awt::Rectangle aPosSize = pC->getPosSize();
        newEntry.aPos.setX( aPosSize.X );
        newEntry.aPos.setY( aPosSize.Y );

        decltype(aCtrls)::size_type nPos;
        for ( nPos = 0; nPos < aCtrls.size(); nPos++ )
        {
            ComponentEntry& rEntry = aCtrls[ nPos ];
            if ( ( rEntry.aPos.Y() > newEntry.aPos.Y() ) ||
                 ( ( rEntry.aPos.Y() == newEntry.aPos.Y() ) && ( rEntry.aPos.X() > newEntry.aPos.X() ) ) )
                    break;
        }
        if ( nPos < aCtrls.size() ) {
            aCtrls.insert( aCtrls.begin() + nPos, newEntry );
        } else {
            aCtrls.push_back( newEntry );
        }
    }

    Sequence< Reference< XControlModel > > aNewSeq( nCtrls );
    std::transform(aCtrls.begin(), aCtrls.end(), aNewSeq.begin(),
        [](const ComponentEntry& rEntry) -> Reference< XControlModel > {
            Reference< XControl >  xUC( rEntry.pComponent, UNO_QUERY );
            return xUC->getModel();
        });

    mxModel->setControlModels( aNewSeq );
}

void StdTabController::activateTabOrder(  )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    // Activate tab order for the control container

    Reference< XControl >  xC( mxControlContainer, UNO_QUERY );
    Reference< XVclContainerPeer >  xVclContainerPeer;
    if ( xC.is() )
        xVclContainerPeer.set(xC->getPeer(), css::uno::UNO_QUERY);
    if ( !xC.is() || !xVclContainerPeer.is() )
        return;

    // This may return a TabController, which returns desired list of controls faster
    // (the dreaded UNO aggregation, retrieve the thing that we are part of)
    Reference<XTabController> xTabController( static_cast<XTabController*>(this), UNO_QUERY );

    // Get a flattened list of controls sequences
    Sequence< Reference< XControlModel > > aModels = mxModel->getControlModels();
    Sequence< Reference< XWindow > > aCompSeq;
    Sequence< Any> aTabSeq;

    // DG: For the sake of optimization, retrieve Controls from getControls(),
    // this may sound counterproductive, but leads to performance improvements
    // in practical scenarios (Forms)
    Sequence< Reference< XControl > > aControls = xTabController->getControls();

    // #58317# Some Models may be missing from the Container. Plus there is a
    // autoTabOrder call later on.
    if( !ImplCreateComponentSequence( aControls, aModels, aCompSeq, &aTabSeq, true ) )
        return;

    xVclContainerPeer->setTabOrder( aCompSeq, aTabSeq, mxModel->getGroupControl() );

    OUString aName;
    Sequence< Reference< XControlModel > >  aThisGroupModels;
    Sequence< Reference< XWindow > >        aControlComponents;

    sal_uInt32 nGroups = mxModel->getGroupCount();
    for ( sal_uInt32 nG = 0; nG < nGroups; nG++ )
    {
        mxModel->getGroup( nG, aThisGroupModels, aName );

        aControls = xTabController->getControls();
            // ImplCreateComponentSequence has a really strange semantics regarding it's first parameter:
            // upon method entry, it expects a super set of the controls which it returns
            // this means we need to completely fill this sequence with all available controls before
            // calling into ImplCreateComponentSequence

        aControlComponents.realloc( 0 );

        ImplCreateComponentSequence( aControls, aThisGroupModels, aControlComponents, nullptr, true );
        xVclContainerPeer->setGroup( aControlComponents );
    }
}

void StdTabController::activateFirst(  )
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() ); //TODO: necessary?

    ImplActivateControl( true );
}

void StdTabController::activateLast(  )
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() ); //TODO: necessary?

    ImplActivateControl( false );
}

OUString StdTabController::getImplementationName()
{
    return "stardiv.Toolkit.StdTabController";
}

sal_Bool StdTabController::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> StdTabController::getSupportedServiceNames()
{
    return css::uno::Sequence<OUString>{
        "com.sun.star.awt.TabController",
        "stardiv.vcl.control.TabController"};
}

Reference< XControl >  StdTabController::FindControl( Sequence< Reference< XControl > >& rCtrls,
 const Reference< XControlModel > & rxCtrlModel )
{
    if (!rxCtrlModel.is())
        throw lang::IllegalArgumentException("No valid XControlModel",
                                             uno::Reference<uno::XInterface>(), 0);

    auto pCtrl = std::find_if(rCtrls.begin(), rCtrls.end(),
        [&rxCtrlModel](const Reference< XControl >& rCtrl) {
            Reference< XControlModel >  xModel(rCtrl.is() ? rCtrl->getModel() : Reference< XControlModel > ());
            return xModel.get() == rxCtrlModel.get();
        });
    if (pCtrl != rCtrls.end())
    {
        auto n = static_cast<sal_Int32>(std::distance(rCtrls.begin(), pCtrl));
        Reference< XControl >  xCtrl( *pCtrl );
        ::comphelper::removeElementAt( rCtrls, n );
        return xCtrl;
    }
    return Reference< XControl > ();
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_StdTabController_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new StdTabController());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
