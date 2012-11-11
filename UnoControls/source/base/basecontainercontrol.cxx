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

#include "basecontainercontrol.hxx"

#include <cppuhelper/typeprovider.hxx>

//____________________________________________________________________________________________________________
//  namespaces
//____________________________________________________________________________________________________________

using namespace ::cppu                      ;
using namespace ::osl                       ;
using namespace ::rtl                       ;
using namespace ::com::sun::star::uno       ;
using namespace ::com::sun::star::lang      ;
using namespace ::com::sun::star::awt       ;
using namespace ::com::sun::star::container ;

namespace unocontrols{

//____________________________________________________________________________________________________________
//  construct/destruct
//____________________________________________________________________________________________________________

BaseContainerControl::BaseContainerControl( const Reference< XComponentContext >& rxContext )
    : BaseControl   ( rxContext  )
    , m_aListeners  ( m_aMutex  )
{
}

BaseContainerControl::~BaseContainerControl()
{
    impl_cleanMemory();
}

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

Any SAL_CALL BaseContainerControl::queryInterface( const Type& rType ) throw( RuntimeException )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.
    Any aReturn ;
    Reference< XInterface > xDel = BaseControl::impl_getDelegator();
    if ( xDel.is() == sal_True )
    {
        // If an delegator exist, forward question to his queryInterface.
        // Delegator will ask his own queryAggregation!
        aReturn = xDel->queryInterface( rType );
    }
    else
    {
        // If an delegator unknown, forward question to own queryAggregation.
        aReturn = queryAggregation( rType );
    }

    return aReturn ;
}

//____________________________________________________________________________________________________________
//  XTypeProvider
//____________________________________________________________________________________________________________

Sequence< Type > SAL_CALL BaseContainerControl::getTypes() throw( RuntimeException )
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pTypeCollection is NULL - for the second call pTypeCollection is different from NULL!
    static OTypeCollection* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        MutexGuard aGuard( Mutex::getGlobalMutex() );

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pTypeCollection == NULL )
        {
            // Create a static typecollection ...
            static OTypeCollection aTypeCollection  (   ::getCppuType(( const Reference< XControlModel      >*)NULL )   ,
                                                          ::getCppuType(( const Reference< XControlContainer    >*)NULL )   ,
                                                        BaseControl::getTypes()
                                                    );
            // ... and set his address to static pointer!
            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes();
}

//____________________________________________________________________________________________________________
//  XAggregation
//____________________________________________________________________________________________________________

Any SAL_CALL BaseContainerControl::queryAggregation( const Type& aType ) throw( RuntimeException )
{
    // Ask for my own supported interfaces ...
    // Attention: XTypeProvider and XInterface are supported by OComponentHelper!
    Any aReturn ( ::cppu::queryInterface(   aType                                       ,
                                               static_cast< XControlModel*      > ( this )  ,
                                               static_cast< XControlContainer*  > ( this )
                                        )
                );

    // If searched interface supported by this class ...
    if ( aReturn.hasValue() == sal_True )
    {
        // ... return this information.
        return aReturn ;
    }
    else
    {
        // Else; ... ask baseclass for interfaces!
        return BaseControl::queryAggregation( aType );
    }
}

//____________________________________________________________________________________________________________
//  XControl
//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::createPeer( const   Reference< XToolkit >&      xToolkit    ,
                                                const   Reference< XWindowPeer >&   xParent     ) throw( RuntimeException )
{
    if ( getPeer().is() == sal_False )
    {
        // create own peer
        BaseControl::createPeer( xToolkit, xParent );

        // create peers at all children
        Sequence< Reference< XControl > >   seqControlList  = getControls();
        sal_uInt32                          nControls       = seqControlList.getLength();

        for ( sal_uInt32 n=0; n<nControls; n++ )
        {
            seqControlList.getArray()[n]->createPeer( xToolkit, getPeer() );
        }

        // activate new tab order
        impl_activateTabControllers();

    }
}

//____________________________________________________________________________________________________________
//  XControl
//____________________________________________________________________________________________________________

sal_Bool SAL_CALL BaseContainerControl::setModel( const Reference< XControlModel >& ) throw( RuntimeException )
{
    // This object has NO model.
    return sal_False ;
}

//____________________________________________________________________________________________________________
//  XControl
//____________________________________________________________________________________________________________

Reference< XControlModel > SAL_CALL BaseContainerControl::getModel() throw( RuntimeException )
{
    // This object has NO model.
    // return (XControlModel*)this ;
    return Reference< XControlModel >();
}

//____________________________________________________________________________________________________________
//  XComponent
//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::dispose() throw( RuntimeException )
{
    // Tell everything that this container is now gone.
    // It's faster if you listen to both the control and the container.

    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // remove listeners
    EventObject aObject ;

    aObject.Source = Reference< XComponent > ( (XControlContainer*)this, UNO_QUERY );
    m_aListeners.disposeAndClear( aObject );

    // remove controls
    Sequence< Reference< XControl > >   seqCtrls    =   getControls();
    Reference< XControl > *             pCtrls      =   seqCtrls.getArray();
    sal_uInt32                          nCtrls      =   seqCtrls.getLength();
    size_t                              nMaxCount   =   maControlInfoList.size();
    size_t                              nCount      =   0;

    for ( nCount = 0; nCount < nMaxCount; ++nCount )
    {
        delete maControlInfoList[ nCount ];
    }
    maControlInfoList.clear();

    for ( nCount = 0; nCount < nCtrls; ++nCount )
    {
        pCtrls [ nCount ] -> removeEventListener    ( static_cast< XEventListener* >( static_cast< XWindowListener* >( this ) ) ) ;
        pCtrls [ nCount ] -> dispose                (       ) ;
    }

    // call baseclass
    BaseControl::dispose();
}

//____________________________________________________________________________________________________________
//  XEventListener
//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::disposing( const EventObject& rEvent ) throw( RuntimeException )
{
    Reference< XControl > xControl( rEvent.Source, UNO_QUERY );

    // "removeControl" remove only, when control is an active control
    removeControl( xControl );
}

//____________________________________________________________________________________________________________
//  XControlContainer
//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::addControl ( const OUString& rName, const Reference< XControl > & rControl ) throw( RuntimeException )
{
    if ( !rControl.is () )
        return;

    // take memory for new item
    IMPL_ControlInfo* pNewControl = new IMPL_ControlInfo ;

    if (pNewControl!=(IMPL_ControlInfo*)0)
    {
        // Ready for multithreading
        MutexGuard aGuard (m_aMutex) ;

        // set control
        pNewControl->sName      = rName     ;
        pNewControl->xControl   = rControl  ;

        // and insert in list
        maControlInfoList.push_back( pNewControl ) ;

        // initialize new control
        pNewControl->xControl->setContext       ( (OWeakObject*)this    ) ;
        pNewControl->xControl->addEventListener ( static_cast< XEventListener* >( static_cast< XWindowListener* >( this ) ) ) ;

        // when container has a peer ...
        if (getPeer().is())
        {
            // .. then create a peer on child
            pNewControl->xControl->createPeer ( getPeer()->getToolkit(), getPeer() ) ;
            impl_activateTabControllers () ;
        }

        // Send message to all listener
        OInterfaceContainerHelper* pInterfaceContainer = m_aListeners.getContainer( ::getCppuType((const Reference< XContainerListener >*)0) ) ;

        if (pInterfaceContainer)
        {
            // Build event
            ContainerEvent  aEvent ;

            aEvent.Source   = *this     ;
            aEvent.Element <<= rControl ;

            // Get all listener
            OInterfaceIteratorHelper    aIterator (*pInterfaceContainer) ;

            // Send event
            while ( aIterator.hasMoreElements() )
            {
                ((XContainerListener*)aIterator.next())->elementInserted (aEvent) ;
            }
        }
    }
}

//____________________________________________________________________________________________________________
//  XControlContainer
//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::addContainerListener ( const Reference< XContainerListener > & rListener ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex ) ;

    m_aListeners.addInterface ( ::getCppuType((const Reference< XContainerListener >*)0), rListener ) ;
}

//____________________________________________________________________________________________________________
//  XControlContainer
//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::removeControl ( const Reference< XControl > & rControl ) throw( RuntimeException )
{
    if ( rControl.is() )
    {
        // Ready for multithreading
        MutexGuard aGuard (m_aMutex) ;

        size_t nControls = maControlInfoList.size();

        for ( size_t n = 0; n < nControls; n++ )
        {
            // Search for right control
            IMPL_ControlInfo* pControl = maControlInfoList[ n ] ;
            if ( rControl == pControl->xControl )
            {
                //.is it found ... remove listener from control
                pControl->xControl->removeEventListener (static_cast< XEventListener* >( static_cast< XWindowListener* >( this ) )) ;
                pControl->xControl->setContext          ( Reference< XInterface >  ()   ) ;

                // ... free memory
                delete pControl ;
                ::std::vector<IMPL_ControlInfo*>::iterator itr = maControlInfoList.begin();
                ::std::advance(itr, n);
                maControlInfoList.erase(itr);

                // Send message to all other listener
                OInterfaceContainerHelper * pInterfaceContainer = m_aListeners.getContainer( ::getCppuType((const Reference< XContainerListener >*)0) ) ;

                if (pInterfaceContainer)
                {
                    ContainerEvent  aEvent ;

                    aEvent.Source    = *this    ;
                    aEvent.Element <<= rControl ;

                    OInterfaceIteratorHelper    aIterator (*pInterfaceContainer) ;

                    while ( aIterator.hasMoreElements() )
                    {
                        ((XContainerListener*)aIterator.next())->elementRemoved (aEvent) ;
                    }
                }
                // Break "for" !
                break ;
            }
        }
    }
}

//____________________________________________________________________________________________________________
//  XControlContainer
//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::removeContainerListener ( const Reference< XContainerListener > & rListener ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex ) ;

    m_aListeners.removeInterface ( ::getCppuType((const Reference< XContainerListener >*)0), rListener ) ;
}

//____________________________________________________________________________________________________________
//  XControlContainer
//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::setStatusText ( const OUString& rStatusText ) throw( RuntimeException )
{
    // go down to each parent
    Reference< XControlContainer >  xContainer ( getContext(), UNO_QUERY ) ;

    if ( xContainer.is () )
    {
        xContainer->setStatusText ( rStatusText ) ;
    }
}

//____________________________________________________________________________________________________________
//  XControlContainer
//____________________________________________________________________________________________________________

Reference< XControl > SAL_CALL BaseContainerControl::getControl ( const OUString& rName ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard  aGuard ( Mutex::getGlobalMutex() ) ;

    Reference< XControl >   xRetControl = Reference< XControl > ();
    size_t                  nControls   = maControlInfoList.size();

    // Search for right control
    for( size_t nCount = 0; nCount < nControls; ++nCount )
    {
        IMPL_ControlInfo* pSearchControl = maControlInfoList[ nCount ];

        if ( pSearchControl->sName == rName )
        {
            // We have found it ...
            // Break operation and return.
            return pSearchControl->xControl ;
        }
    }

    // We have not found it ... return NULL.
    return Reference< XControl >  () ;
}

//____________________________________________________________________________________________________________
//  XControlContainer
//____________________________________________________________________________________________________________

Sequence< Reference< XControl > > SAL_CALL BaseContainerControl::getControls () throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard  aGuard ( Mutex::getGlobalMutex() ) ;

    size_t                              nControls       = maControlInfoList.size();
    size_t                              nCount          = 0;
    Sequence< Reference< XControl > >   aDescriptor     ( nControls )                   ;
    Reference< XControl > *             pDestination    = aDescriptor.getArray ()       ;

    // Copy controls to sequence
    for( nCount = 0; nCount < nControls; ++nCount )
    {
        IMPL_ControlInfo* pCopyControl = maControlInfoList[ nCount ];
        pDestination [ nCount ] = pCopyControl->xControl ;
    }

    // Return sequence
    return aDescriptor ;
}

//____________________________________________________________________________________________________________
//  XUnoControlContainer
//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::addTabController ( const Reference< XTabController > & rTabController ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard (m_aMutex) ;

    sal_uInt32                                  nOldCount   = m_xTabControllerList.getLength () ;
    Sequence< Reference< XTabController >  >    aNewList    ( nOldCount + 1 )                   ;
    sal_uInt32                                  nCount      = 0                                 ;

    // Copy old elements of sequence to new list.
    for ( nCount = 0; nCount < nOldCount; ++nCount )
    {
        aNewList.getArray () [nCount] = m_xTabControllerList.getConstArray () [nCount] ;
    }

    // Add new controller
    aNewList.getArray () [nOldCount] = rTabController ;

    // change old and new list
    m_xTabControllerList = aNewList ;
}

//____________________________________________________________________________________________________________
//  XUnoControlContainer
//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::removeTabController ( const Reference< XTabController > & rTabController ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard (m_aMutex) ;

    sal_uInt32  nMaxCount   = m_xTabControllerList.getLength () ;
    sal_uInt32  nCount      = 0                                 ;

    // Search right tabcontroller ...
    for ( nCount = 0; nCount < nMaxCount; ++nCount )
    {
        if ( m_xTabControllerList.getConstArray () [nCount] == rTabController )
        {
            // ... if is it found ... remove it from list.
            m_xTabControllerList.getArray()[ nCount ] = Reference< XTabController >() ;
            break ;
        }
    }
}

//____________________________________________________________________________________________________________
//  XUnoControlContainer
//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::setTabControllers ( const Sequence< Reference< XTabController >  >& rTabControllers ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard (m_aMutex) ;

    m_xTabControllerList = rTabControllers ;
}

Sequence<Reference< XTabController > > SAL_CALL BaseContainerControl::getTabControllers () throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard (m_aMutex) ;

    return m_xTabControllerList ;
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::setVisible ( sal_Bool bVisible ) throw( RuntimeException )
{
    // override baseclass definition
    BaseControl::setVisible ( bVisible ) ;

    // is it a top window ?
    if ( !getContext().is() && bVisible )
    {
        // then show it automaticly
        createPeer ( Reference< XToolkit > (), Reference< XWindowPeer > () ) ;
    }
}

//____________________________________________________________________________________________________________
//  protected method
//____________________________________________________________________________________________________________

WindowDescriptor* BaseContainerControl::impl_getWindowDescriptor ( const Reference< XWindowPeer > & rParentPeer )
{
    // - used from "createPeer()" to set the values of an WindowDescriptor !!!
    // - if you will change the descriptor-values, you must override thid virtuell function
    // - the caller must release the memory for this dynamical descriptor !!!

    WindowDescriptor    *   aDescriptor = new WindowDescriptor ;

    aDescriptor->Type               = WindowClass_CONTAINER                             ;
    aDescriptor->WindowServiceName  = "window"                                          ;
    aDescriptor->ParentIndex        = -1                                                ;
    aDescriptor->Parent             = rParentPeer                                       ;
    aDescriptor->Bounds             = getPosSize ()                                     ;
    aDescriptor->WindowAttributes   = 0                                                 ;

    return aDescriptor ;
}

//____________________________________________________________________________________________________________
//  protected method
//____________________________________________________________________________________________________________

void BaseContainerControl::impl_paint ( sal_Int32 /*nX*/, sal_Int32 /*nY*/, const Reference< XGraphics > & /*rGraphics*/ )
{
}

//____________________________________________________________________________________________________________
//  private method
//____________________________________________________________________________________________________________

void BaseContainerControl::impl_activateTabControllers ()
{
    // Ready for multithreading
    MutexGuard aGuard (m_aMutex) ;

    sal_uInt32  nMaxCount   =   m_xTabControllerList.getLength ()   ;
    sal_uInt32  nCount      =   0                                   ;

    for ( nCount = 0; nCount < nMaxCount; ++nCount )
    {
         m_xTabControllerList.getArray () [nCount]->setContainer        ( this  ) ;
         m_xTabControllerList.getArray () [nCount]->activateTabOrder    (       ) ;
    }
}

//____________________________________________________________________________________________________________
//  private method
//____________________________________________________________________________________________________________

void BaseContainerControl::impl_cleanMemory ()
{
    // Get count of listitems.
    size_t  nMaxCount   = maControlInfoList.size();
    size_t  nCount      = 0;

    // Delete all items.
    for ( nCount = 0; nCount < nMaxCount; ++nCount )
    {
        // Delete everytime first element of list!
        // We count from 0 to MAX, where "MAX=count of items" BEFORE we delete some elements!
        // If we use "GetObject ( nCount )" ... it can be, that we have an index greater then count of current elements!

        IMPL_ControlInfo* pSearchControl = maControlInfoList[ nCount ];
        delete pSearchControl;
    }

    // Delete list himself.
    maControlInfoList.clear ();
}

} // namespace unocontrols

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
