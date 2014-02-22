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

//____________________________________________________________________________________________________________

Any SAL_CALL BaseContainerControl::queryInterface( const Type& rType ) throw( RuntimeException )
{
    
    
    Any aReturn ;
    Reference< XInterface > xDel = BaseControl::impl_getDelegator();
    if ( xDel.is() )
    {
        
        
        aReturn = xDel->queryInterface( rType );
    }
    else
    {
        
        aReturn = queryAggregation( rType );
    }

    return aReturn ;
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

Sequence< Type > SAL_CALL BaseContainerControl::getTypes() throw( RuntimeException )
{
    
    
    
    static OTypeCollection* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        
        MutexGuard aGuard( Mutex::getGlobalMutex() );

        
        if ( pTypeCollection == NULL )
        {
            
            static OTypeCollection aTypeCollection  (   ::getCppuType(( const Reference< XControlModel      >*)NULL )   ,
                                                          ::getCppuType(( const Reference< XControlContainer    >*)NULL )   ,
                                                        BaseControl::getTypes()
                                                    );
            
            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes();
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

Any SAL_CALL BaseContainerControl::queryAggregation( const Type& aType ) throw( RuntimeException )
{
    
    
    Any aReturn ( ::cppu::queryInterface(   aType                                       ,
                                               static_cast< XControlModel*      > ( this )  ,
                                               static_cast< XControlContainer*  > ( this )
                                        )
                );

    
    if ( aReturn.hasValue() )
    {
        
        return aReturn ;
    }
    else
    {
        
        return BaseControl::queryAggregation( aType );
    }
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::createPeer( const   Reference< XToolkit >&      xToolkit    ,
                                                const   Reference< XWindowPeer >&   xParent     ) throw( RuntimeException )
{
    if ( !getPeer().is() )
    {
        
        BaseControl::createPeer( xToolkit, xParent );

        
        Sequence< Reference< XControl > >   seqControlList  = getControls();
        sal_uInt32                          nControls       = seqControlList.getLength();

        for ( sal_uInt32 n=0; n<nControls; n++ )
        {
            seqControlList.getArray()[n]->createPeer( xToolkit, getPeer() );
        }

        
        impl_activateTabControllers();

    }
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

sal_Bool SAL_CALL BaseContainerControl::setModel( const Reference< XControlModel >& ) throw( RuntimeException )
{
    
    return sal_False ;
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

Reference< XControlModel > SAL_CALL BaseContainerControl::getModel() throw( RuntimeException )
{
    
    
    return Reference< XControlModel >();
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::dispose() throw( RuntimeException )
{
    
    

    
    MutexGuard aGuard( m_aMutex );

    
    EventObject aObject ;

    aObject.Source = Reference< XComponent > ( (XControlContainer*)this, UNO_QUERY );
    m_aListeners.disposeAndClear( aObject );

    
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

    
    BaseControl::dispose();
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::disposing( const EventObject& rEvent ) throw( RuntimeException )
{
    Reference< XControl > xControl( rEvent.Source, UNO_QUERY );

    
    removeControl( xControl );
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::addControl ( const OUString& rName, const Reference< XControl > & rControl ) throw( RuntimeException )
{
    if ( !rControl.is () )
        return;

    
    IMPL_ControlInfo* pNewControl = new IMPL_ControlInfo ;

    if (pNewControl!=(IMPL_ControlInfo*)0)
    {
        
        MutexGuard aGuard (m_aMutex) ;

        
        pNewControl->sName      = rName     ;
        pNewControl->xControl   = rControl  ;

        
        maControlInfoList.push_back( pNewControl ) ;

        
        pNewControl->xControl->setContext       ( (OWeakObject*)this    ) ;
        pNewControl->xControl->addEventListener ( static_cast< XEventListener* >( static_cast< XWindowListener* >( this ) ) ) ;

        
        if (getPeer().is())
        {
            
            pNewControl->xControl->createPeer ( getPeer()->getToolkit(), getPeer() ) ;
            impl_activateTabControllers () ;
        }

        
        OInterfaceContainerHelper* pInterfaceContainer = m_aListeners.getContainer( ::getCppuType((const Reference< XContainerListener >*)0) ) ;

        if (pInterfaceContainer)
        {
            
            ContainerEvent  aEvent ;

            aEvent.Source   = *this     ;
            aEvent.Element <<= rControl ;

            
            OInterfaceIteratorHelper    aIterator (*pInterfaceContainer) ;

            
            while ( aIterator.hasMoreElements() )
            {
                ((XContainerListener*)aIterator.next())->elementInserted (aEvent) ;
            }
        }
    }
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::addContainerListener ( const Reference< XContainerListener > & rListener ) throw( RuntimeException )
{
    
    MutexGuard aGuard ( m_aMutex ) ;

    m_aListeners.addInterface ( ::getCppuType((const Reference< XContainerListener >*)0), rListener ) ;
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::removeControl ( const Reference< XControl > & rControl ) throw( RuntimeException )
{
    if ( rControl.is() )
    {
        
        MutexGuard aGuard (m_aMutex) ;

        size_t nControls = maControlInfoList.size();

        for ( size_t n = 0; n < nControls; n++ )
        {
            
            IMPL_ControlInfo* pControl = maControlInfoList[ n ] ;
            if ( rControl == pControl->xControl )
            {
                //.is it found ... remove listener from control
                pControl->xControl->removeEventListener (static_cast< XEventListener* >( static_cast< XWindowListener* >( this ) )) ;
                pControl->xControl->setContext          ( Reference< XInterface >  ()   ) ;

                
                delete pControl ;
                ::std::vector<IMPL_ControlInfo*>::iterator itr = maControlInfoList.begin();
                ::std::advance(itr, n);
                maControlInfoList.erase(itr);

                
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
                
                break ;
            }
        }
    }
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::removeContainerListener ( const Reference< XContainerListener > & rListener ) throw( RuntimeException )
{
    
    MutexGuard aGuard ( m_aMutex ) ;

    m_aListeners.removeInterface ( ::getCppuType((const Reference< XContainerListener >*)0), rListener ) ;
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::setStatusText ( const OUString& rStatusText ) throw( RuntimeException )
{
    
    Reference< XControlContainer >  xContainer ( getContext(), UNO_QUERY ) ;

    if ( xContainer.is () )
    {
        xContainer->setStatusText ( rStatusText ) ;
    }
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

Reference< XControl > SAL_CALL BaseContainerControl::getControl ( const OUString& rName ) throw( RuntimeException )
{
    
    MutexGuard  aGuard ( Mutex::getGlobalMutex() ) ;

    size_t                  nControls   = maControlInfoList.size();

    
    for( size_t nCount = 0; nCount < nControls; ++nCount )
    {
        IMPL_ControlInfo* pSearchControl = maControlInfoList[ nCount ];

        if ( pSearchControl->sName == rName )
        {
            
            
            return pSearchControl->xControl ;
        }
    }

    
    return Reference< XControl >  () ;
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

Sequence< Reference< XControl > > SAL_CALL BaseContainerControl::getControls () throw( RuntimeException )
{
    
    MutexGuard  aGuard ( Mutex::getGlobalMutex() ) ;

    size_t                              nControls       = maControlInfoList.size();
    size_t                              nCount          = 0;
    Sequence< Reference< XControl > >   aDescriptor     ( nControls )                   ;
    Reference< XControl > *             pDestination    = aDescriptor.getArray ()       ;

    
    for( nCount = 0; nCount < nControls; ++nCount )
    {
        IMPL_ControlInfo* pCopyControl = maControlInfoList[ nCount ];
        pDestination [ nCount ] = pCopyControl->xControl ;
    }

    
    return aDescriptor ;
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::addTabController ( const Reference< XTabController > & rTabController ) throw( RuntimeException )
{
    
    MutexGuard aGuard (m_aMutex) ;

    sal_uInt32                                  nOldCount   = m_xTabControllerList.getLength () ;
    Sequence< Reference< XTabController >  >    aNewList    ( nOldCount + 1 )                   ;
    sal_uInt32                                  nCount      = 0                                 ;

    
    for ( nCount = 0; nCount < nOldCount; ++nCount )
    {
        aNewList.getArray () [nCount] = m_xTabControllerList.getConstArray () [nCount] ;
    }

    
    aNewList.getArray () [nOldCount] = rTabController ;

    
    m_xTabControllerList = aNewList ;
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::removeTabController ( const Reference< XTabController > & rTabController ) throw( RuntimeException )
{
    
    MutexGuard aGuard (m_aMutex) ;

    sal_uInt32  nMaxCount   = m_xTabControllerList.getLength () ;
    sal_uInt32  nCount      = 0                                 ;

    
    for ( nCount = 0; nCount < nMaxCount; ++nCount )
    {
        if ( m_xTabControllerList.getConstArray () [nCount] == rTabController )
        {
            
            m_xTabControllerList.getArray()[ nCount ].clear();
            break ;
        }
    }
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::setTabControllers ( const Sequence< Reference< XTabController >  >& rTabControllers ) throw( RuntimeException )
{
    
    MutexGuard aGuard (m_aMutex) ;

    m_xTabControllerList = rTabControllers ;
}

Sequence<Reference< XTabController > > SAL_CALL BaseContainerControl::getTabControllers () throw( RuntimeException )
{
    
    MutexGuard aGuard (m_aMutex) ;

    return m_xTabControllerList ;
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

void SAL_CALL BaseContainerControl::setVisible ( sal_Bool bVisible ) throw( RuntimeException )
{
    
    BaseControl::setVisible ( bVisible ) ;

    
    if ( !getContext().is() && bVisible )
    {
        
        createPeer ( Reference< XToolkit > (), Reference< XWindowPeer > () ) ;
    }
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

WindowDescriptor* BaseContainerControl::impl_getWindowDescriptor ( const Reference< XWindowPeer > & rParentPeer )
{
    
    
    

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

//____________________________________________________________________________________________________________

void BaseContainerControl::impl_paint ( sal_Int32 /*nX*/, sal_Int32 /*nY*/, const Reference< XGraphics > & /*rGraphics*/ )
{
}

//____________________________________________________________________________________________________________

//____________________________________________________________________________________________________________

void BaseContainerControl::impl_activateTabControllers ()
{
    
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

//____________________________________________________________________________________________________________

void BaseContainerControl::impl_cleanMemory ()
{
    
    size_t  nMaxCount   = maControlInfoList.size();
    size_t  nCount      = 0;

    
    for ( nCount = 0; nCount < nMaxCount; ++nCount )
    {
        
        
        

        IMPL_ControlInfo* pSearchControl = maControlInfoList[ nCount ];
        delete pSearchControl;
    }

    
    maControlInfoList.clear ();
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
