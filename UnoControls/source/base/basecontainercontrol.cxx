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

#include <basecontainercontrol.hxx>

#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <com/sun/star/container/ContainerEvent.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>

//  namespaces

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::container;

namespace unocontrols {

//  construct/destruct

BaseContainerControl::BaseContainerControl( const Reference< XComponentContext >& rxContext )
    : BaseControl   ( rxContext  )
    , m_aListeners  ( m_aMutex  )
{
}

BaseContainerControl::~BaseContainerControl()
{
}

//  XInterface

Any SAL_CALL BaseContainerControl::queryInterface( const Type& rType )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.
    Any aReturn;
    Reference< XInterface > xDel = BaseControl::impl_getDelegator();
    if ( xDel.is() )
    {
        // If a delegator exists, forward question to its queryInterface.
        // Delegator will ask its own queryAggregation!
        aReturn = xDel->queryInterface( rType );
    }
    else
    {
        // If a delegator is unknown, forward question to own queryAggregation.
        aReturn = queryAggregation( rType );
    }

    return aReturn;
}

//  XTypeProvider

Sequence< Type > SAL_CALL BaseContainerControl::getTypes()
{
    static OTypeCollection ourTypeCollection(
                cppu::UnoType<XControlModel>::get(),
                cppu::UnoType<XControlContainer>::get(),
                BaseControl::getTypes() );

    return ourTypeCollection.getTypes();
}

//  XAggregation

Any SAL_CALL BaseContainerControl::queryAggregation( const Type& aType )
{
    // Ask for my own supported interfaces ...
    // Attention: XTypeProvider and XInterface are supported by OComponentHelper!
    Any aReturn ( ::cppu::queryInterface(   aType                                       ,
                                               static_cast< XControlModel*      > ( this )  ,
                                               static_cast< XControlContainer*  > ( this )
                                        )
                );

    // If searched interface supported by this class ...
    if ( aReturn.hasValue() )
    {
        // ... return this information.
        return aReturn;
    }
    else
    {
        // Else; ... ask baseclass for interfaces!
        return BaseControl::queryAggregation( aType );
    }
}

//  XControl

void SAL_CALL BaseContainerControl::createPeer( const   Reference< XToolkit >&      xToolkit    ,
                                                const   Reference< XWindowPeer >&   xParent     )
{
    if ( getPeer().is() )
        return;

    // create own peer
    BaseControl::createPeer( xToolkit, xParent );

    // create peers at all children
    Sequence< Reference< XControl > >   seqControlList  = getControls();

    for ( auto& rxControl : seqControlList )
    {
        rxControl->createPeer( xToolkit, getPeer() );
    }
}

//  XControl

sal_Bool SAL_CALL BaseContainerControl::setModel( const Reference< XControlModel >& )
{
    // This object has NO model.
    return false;
}

//  XControl

Reference< XControlModel > SAL_CALL BaseContainerControl::getModel()
{
    // This object has NO model.
    // return (XControlModel*)this;
    return Reference< XControlModel >();
}

//  XComponent

void SAL_CALL BaseContainerControl::dispose()
{
    // Tell everything that this container is now gone.
    // It's faster if you listen to both the control and the container.

    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // remove listeners
    EventObject aObject;

    aObject.Source.set( static_cast<XControlContainer*>(this), UNO_QUERY );
    m_aListeners.disposeAndClear( aObject );

    // remove controls
    const Sequence< Reference< XControl > >   seqCtrls    =   getControls();

    maControlInfoList.clear();

    for ( Reference< XControl > const & control : seqCtrls )
    {
        control->removeEventListener    ( static_cast< XEventListener* >( static_cast< XWindowListener* >( this ) ) );
        control->dispose                (       );
    }

    // call baseclass
    BaseControl::dispose();
}

//  XEventListener

void SAL_CALL BaseContainerControl::disposing( const EventObject& rEvent )
{
    Reference< XControl > xControl( rEvent.Source, UNO_QUERY );

    // "removeControl" remove only, when control is an active control
    removeControl( xControl );
}

//  XControlContainer

void SAL_CALL BaseContainerControl::addControl ( const OUString& rName, const Reference< XControl > & rControl )
{
    if ( !rControl.is () )
        return;

    // take memory for new item
    IMPL_ControlInfo* pNewControl = new IMPL_ControlInfo;

    // Ready for multithreading
    MutexGuard aGuard (m_aMutex);

    // set control
    pNewControl->sName      = rName;
    pNewControl->xControl   = rControl;

    // and insert in list
    maControlInfoList.emplace_back( pNewControl );

    // initialize new control
    pNewControl->xControl->setContext       ( static_cast<OWeakObject*>(this)    );
    pNewControl->xControl->addEventListener ( static_cast< XEventListener* >( static_cast< XWindowListener* >( this ) ) );

    // when container has a peer...
    if (getPeer().is())
    {
        // ... then create a peer on child
        pNewControl->xControl->createPeer ( getPeer()->getToolkit(), getPeer() );
    }

    // Send message to all listener
    OInterfaceContainerHelper* pInterfaceContainer = m_aListeners.getContainer( cppu::UnoType<XContainerListener>::get());

    if (!pInterfaceContainer)
        return;

    // Build event
    ContainerEvent  aEvent;

    aEvent.Source   = *this;
    aEvent.Element <<= rControl;

    // Get all listener
    OInterfaceIteratorHelper    aIterator (*pInterfaceContainer);

    // Send event
    while ( aIterator.hasMoreElements() )
    {
        static_cast<XContainerListener*>(aIterator.next())->elementInserted (aEvent);
    }
}

//  XControlContainer

void SAL_CALL BaseContainerControl::removeControl ( const Reference< XControl > & rControl )
{
    if ( !rControl.is() )
        return;

    // Ready for multithreading
    MutexGuard aGuard (m_aMutex);

    size_t nControls = maControlInfoList.size();

    for ( size_t n = 0; n < nControls; n++ )
    {
        // Search for right control
        IMPL_ControlInfo* pControl = maControlInfoList[ n ].get();
        if ( rControl == pControl->xControl )
        {
            //.is it found ... remove listener from control
            pControl->xControl->removeEventListener (static_cast< XEventListener* >( static_cast< XWindowListener* >( this ) ));
            pControl->xControl->setContext          ( Reference< XInterface >  ()   );

            // ... free memory
            maControlInfoList.erase(maControlInfoList.begin() + n);

            // Send message to all other listener
            OInterfaceContainerHelper * pInterfaceContainer = m_aListeners.getContainer( cppu::UnoType<XContainerListener>::get());

            if (pInterfaceContainer)
            {
                ContainerEvent  aEvent;

                aEvent.Source    = *this;
                aEvent.Element <<= rControl;

                OInterfaceIteratorHelper    aIterator (*pInterfaceContainer);

                while ( aIterator.hasMoreElements() )
                {
                    static_cast<XContainerListener*>(aIterator.next())->elementRemoved (aEvent);
                }
            }
            // Break "for" !
            break;
        }
    }
}

//  XControlContainer

void SAL_CALL BaseContainerControl::setStatusText ( const OUString& rStatusText )
{
    // go down to each parent
    Reference< XControlContainer >  xContainer ( getContext(), UNO_QUERY );

    if ( xContainer.is () )
    {
        xContainer->setStatusText ( rStatusText );
    }
}

//  XControlContainer

Reference< XControl > SAL_CALL BaseContainerControl::getControl ( const OUString& rName )
{
    // Ready for multithreading
    MutexGuard  aGuard ( Mutex::getGlobalMutex() );

    size_t                  nControls   = maControlInfoList.size();

    // Search for right control
    for( size_t nCount = 0; nCount < nControls; ++nCount )
    {
        IMPL_ControlInfo* pSearchControl = maControlInfoList[ nCount ].get();

        if ( pSearchControl->sName == rName )
        {
            // We have found it ...
            // Break operation and return.
            return pSearchControl->xControl;
        }
    }

    // We have not found it ... return NULL.
    return Reference< XControl >  ();
}

//  XControlContainer

Sequence< Reference< XControl > > SAL_CALL BaseContainerControl::getControls ()
{
    // Ready for multithreading
    MutexGuard  aGuard ( Mutex::getGlobalMutex() );

    size_t                              nControls       = maControlInfoList.size();
    size_t                              nCount          = 0;
    Sequence< Reference< XControl > >   aDescriptor     ( nControls );
    Reference< XControl > *             pDestination    = aDescriptor.getArray ();

    // Copy controls to sequence
    for( nCount = 0; nCount < nControls; ++nCount )
    {
        IMPL_ControlInfo* pCopyControl = maControlInfoList[ nCount ].get();
        pDestination [ nCount ] = pCopyControl->xControl;
    }

    // Return sequence
    return aDescriptor;
}

//  XWindow

void SAL_CALL BaseContainerControl::setVisible ( sal_Bool bVisible )
{
    // override baseclass definition
    BaseControl::setVisible ( bVisible );

    // is it a top window ?
    if ( !getContext().is() && bVisible )
    {
        // then show it automatically
        createPeer ( Reference< XToolkit > (), Reference< XWindowPeer > () );
    }
}

//  protected method

WindowDescriptor BaseContainerControl::impl_getWindowDescriptor ( const Reference< XWindowPeer > & rParentPeer )
{
    WindowDescriptor aDescriptor;

    aDescriptor.Type               = WindowClass_CONTAINER;
    aDescriptor.WindowServiceName  = "window";
    aDescriptor.ParentIndex        = -1;
    aDescriptor.Parent             = rParentPeer;
    aDescriptor.Bounds             = getPosSize ();
    aDescriptor.WindowAttributes   = 0;

    return aDescriptor;
}

//  protected method

void BaseContainerControl::impl_paint ( sal_Int32 /*nX*/, sal_Int32 /*nY*/, const Reference< XGraphics > & /*rGraphics*/ )
{
}

} // namespace unocontrols

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
