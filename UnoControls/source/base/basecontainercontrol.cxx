/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: basecontainercontrol.cxx,v $
 * $Revision: 1.6 $
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

//____________________________________________________________________________________________________________
//  my own includes
//____________________________________________________________________________________________________________

#include "basecontainercontrol.hxx"

//____________________________________________________________________________________________________________
//  includes of other projects
//____________________________________________________________________________________________________________
#include <cppuhelper/typeprovider.hxx>

//____________________________________________________________________________________________________________
//  includes of my own project
//____________________________________________________________________________________________________________

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

BaseContainerControl::BaseContainerControl( const Reference< XMultiServiceFactory >& xFactory )
    : BaseControl   ( xFactory  )
    , m_aListeners  ( m_aMutex  )
{
    // initialize info list for controls
    m_pControlInfoList = new IMPL_ControlInfoList ;
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

        // create peers at all childs
        Sequence< Reference< XControl > >   seqControlList  = getControls();
        sal_uInt32                          nControls       = seqControlList.getLength();

        for ( sal_uInt32 n=0; n<nControls; n++ )
        {
            seqControlList.getArray()[n]->createPeer( xToolkit, getPeer() );
        }

        // activate new tab order
        impl_activateTabControllers();

/*
        Reference< XVclContainerPeer > xC;
        mxPeer->queryInterface( ::getCppuType((const Reference< XVclContainerPeer >*)0), xC );
        xC->enableDialogControl( sal_True );
*/

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
    // Zuerst der Welt mitteilen, da� der Container wegfliegt. Dieses ist um einiges
    // schneller wenn die Welt sowohl an den Controls als auch am Container horcht

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
    sal_uInt32                          nMaxCount   =   m_pControlInfoList->Count();
    sal_uInt32                          nCount      =   0;

    for ( nCount = 0; nCount < nMaxCount; ++nCount )
    {
        delete m_pControlInfoList->GetObject( 0 );
    }
    m_pControlInfoList->Clear();


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
        m_pControlInfoList->Insert ( pNewControl, LIST_APPEND ) ;

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

        sal_uInt32 nControls = m_pControlInfoList->Count () ;

        for ( sal_uInt32 n=0; n<nControls; n++ )
        {
            // Search for right control
            IMPL_ControlInfo* pControl = m_pControlInfoList->GetObject (n) ;
            if ( rControl == pControl->xControl )
            {
                //.is it found ... remove listener from control
                pControl->xControl->removeEventListener (static_cast< XEventListener* >( static_cast< XWindowListener* >( this ) )) ;
                pControl->xControl->setContext          ( Reference< XInterface >  ()   ) ;

                // ... free memory
                delete pControl ;
                m_pControlInfoList->Remove (n) ;

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

    Reference< XControl >  xRetControl  =   Reference< XControl >       () ;
    sal_uInt32              nControls   =   m_pControlInfoList->Count   () ;

    // Search for right control
    for( sal_uInt32 nCount = 0; nCount < nControls; ++nCount )
    {
        IMPL_ControlInfo* pSearchControl = m_pControlInfoList->GetObject ( nCount ) ;

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

    sal_uInt32                          nControls       = m_pControlInfoList->Count ()  ;
    Sequence< Reference< XControl > >   aDescriptor     ( nControls )                   ;
    Reference< XControl > *             pDestination    = aDescriptor.getArray ()       ;
    sal_uInt32                          nCount          = 0                             ;

    // Copy controls to sequence
    for( nCount = 0; nCount < nControls; ++nCount )
    {
        IMPL_ControlInfo* pCopyControl = m_pControlInfoList->GetObject ( nCount ) ;
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
    aDescriptor->WindowServiceName  = OUString(RTL_CONSTASCII_USTRINGPARAM("window"))   ;
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
/*
    if (rGraphics.is())
    {
        for ( sal_uInt32 n=m_pControlInfoList->Count(); n; )
        {
            ControlInfo* pSearchControl = m_pControlInfoList->GetObject (--n) ;

            pSearchControl->xControl->paint ( nX, nY, rGraphics ) ;
        }
    }
*/
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
    sal_uInt32  nMaxCount   =   m_pControlInfoList->Count ()    ;
    sal_uInt32  nCount      =   0                               ;

    // Delete all items.
    for ( nCount = 0; nCount < nMaxCount; ++nCount )
    {
        // Delete everytime first element of list!
        // We count from 0 to MAX, where "MAX=count of items" BEFORE we delete some elements!
        // If we use "GetObject ( nCount )" ... it can be, that we have an index greater then count of current elements!

        IMPL_ControlInfo* pSearchControl = m_pControlInfoList->GetObject ( 0 ) ;
        delete pSearchControl ;
    }

    // Delete list himself.
    m_pControlInfoList->Clear () ;
    delete m_pControlInfoList ;
}

} // namespace unocontrols
