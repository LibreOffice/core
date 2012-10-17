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

#include "statusindicator.hxx"

#include <com/sun/star/awt/InvalidateStyle.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <cppuhelper/typeprovider.hxx>

#include "progressbar.hxx"

using namespace ::cppu                  ;
using namespace ::osl                   ;
using namespace ::rtl                   ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::lang  ;
using namespace ::com::sun::star::awt   ;
using namespace ::com::sun::star::task  ;

namespace unocontrols{

//____________________________________________________________________________________________________________
//  construct/destruct
//____________________________________________________________________________________________________________

StatusIndicator::StatusIndicator( const Reference< XComponentContext >& rxContext )
    : BaseContainerControl  ( rxContext  )
{
    // Its not allowed to work with member in this method (refcounter !!!)
    // But with a HACK (++refcount) its "OK" :-(
    ++m_refCount ;

    // Create instances for fixedtext and progress ...
    m_xText         = Reference< XFixedText >   ( rxContext->getServiceManager()->createInstanceWithContext( FIXEDTEXT_SERVICENAME, rxContext ), UNO_QUERY );
    m_xProgressBar  = Reference< XProgressBar > ( rxContext->getServiceManager()->createInstanceWithContext( SERVICENAME_PROGRESSBAR, rxContext ), UNO_QUERY );
    // ... cast controls to Reference< XControl > and set model ...
    // ( ProgressBar has no model !!! )
    Reference< XControl > xTextControl      ( m_xText       , UNO_QUERY );
    Reference< XControl > xProgressControl  ( m_xProgressBar, UNO_QUERY );
    xTextControl->setModel( Reference< XControlModel >( rxContext->getServiceManager()->createInstanceWithContext( FIXEDTEXT_MODELNAME, rxContext ), UNO_QUERY ) );
    // ... and add controls to basecontainercontrol!
    addControl( CONTROLNAME_TEXT, xTextControl    );
    addControl( CONTROLNAME_PROGRESSBAR, xProgressControl    );
    // FixedText make it automaticly visible by himself ... but not the progressbar !!!
    // it must be set explicitly
    Reference< XWindow > xProgressWindow( m_xProgressBar, UNO_QUERY );
    xProgressWindow->setVisible( sal_True );
    // Reset to defaults !!!
    // (progressbar take automaticly its own defaults)
    m_xText->setText( STATUSINDICATOR_DEFAULT_TEXT );

    --m_refCount ;
}

StatusIndicator::~StatusIndicator()
{
    // Release all references
    m_xText         = Reference< XFixedText >();
    m_xProgressBar  = Reference< XProgressBar >();
}

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

Any SAL_CALL StatusIndicator::queryInterface( const Type& rType ) throw( RuntimeException )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.
    Any aReturn ;
    Reference< XInterface > xDel = BaseContainerControl::impl_getDelegator();
    if ( xDel.is() )
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
//  XInterface
//____________________________________________________________________________________________________________

void SAL_CALL StatusIndicator::acquire() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    BaseControl::acquire();
}

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

void SAL_CALL StatusIndicator::release() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    BaseControl::release();
}

//____________________________________________________________________________________________________________
//  XTypeProvider
//____________________________________________________________________________________________________________

Sequence< Type > SAL_CALL StatusIndicator::getTypes() throw( RuntimeException )
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
            static OTypeCollection aTypeCollection  ( ::getCppuType(( const Reference< XLayoutConstrains    >*)NULL )   ,
                                                      ::getCppuType(( const Reference< XStatusIndicator >*)NULL )   ,
                                                      BaseContainerControl::getTypes()
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

Any SAL_CALL StatusIndicator::queryAggregation( const Type& aType ) throw( RuntimeException )
{
    // Ask for my own supported interfaces ...
    // Attention: XTypeProvider and XInterface are supported by OComponentHelper!
    Any aReturn ( ::cppu::queryInterface( aType                                     ,
                                          static_cast< XLayoutConstrains*   > ( this )  ,
                                          static_cast< XStatusIndicator*    > ( this )
                                        )
                );

    // If searched interface not supported by this class ...
    if ( aReturn.hasValue() == sal_False )
    {
        // ... ask baseclasses.
        aReturn = BaseControl::queryAggregation( aType );
    }

    return aReturn ;
}

//____________________________________________________________________________________________________________
//  XStatusIndicator
//____________________________________________________________________________________________________________

void SAL_CALL StatusIndicator::start( const OUString& sText, sal_Int32 nRange ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // Initialize status controls with given values.
    m_xText->setText( sText );
    m_xProgressBar->setRange( 0, nRange );
    // force repaint ... fixedtext has changed !
    impl_recalcLayout ( WindowEvent(static_cast< OWeakObject* >(this),0,0,impl_getWidth(),impl_getHeight(),0,0,0,0) ) ;
}

//____________________________________________________________________________________________________________
//  XStatusIndicator
//____________________________________________________________________________________________________________

void SAL_CALL StatusIndicator::end() throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // Clear values of status controls.
    m_xText->setText( OUString() );
    m_xProgressBar->setValue( 0 );
    setVisible( sal_False );
}

//____________________________________________________________________________________________________________
//  XStatusIndicator
//____________________________________________________________________________________________________________

void SAL_CALL StatusIndicator::setText( const OUString& sText ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // Take text on right control
    m_xText->setText( sText );
}

//____________________________________________________________________________________________________________
//  XStatusIndicator
//____________________________________________________________________________________________________________

void SAL_CALL StatusIndicator::setValue( sal_Int32 nValue ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // Take value on right control
    m_xProgressBar->setValue( nValue );
}

//____________________________________________________________________________________________________________
//  XStatusIndicator
//____________________________________________________________________________________________________________

void SAL_CALL StatusIndicator::reset() throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // Clear values of status controls.
    // (Don't hide the window! User will reset current values ... but he will not finish using of indicator!)
    m_xText->setText( OUString() );
    m_xProgressBar->setValue( 0 );
}

//____________________________________________________________________________________________________________
//  XLayoutConstrains
//____________________________________________________________________________________________________________

Size SAL_CALL StatusIndicator::getMinimumSize () throw( RuntimeException )
{
    return Size (STATUSINDICATOR_DEFAULT_WIDTH, STATUSINDICATOR_DEFAULT_HEIGHT) ;
}

//____________________________________________________________________________________________________________
//  XLayoutConstrains
//____________________________________________________________________________________________________________

Size SAL_CALL StatusIndicator::getPreferredSize () throw( RuntimeException )
{
    // Ready for multithreading
    ClearableMutexGuard aGuard ( m_aMutex ) ;

    // get information about required place of child controls
    Reference< XLayoutConstrains >  xTextLayout ( m_xText, UNO_QUERY );
    Size                            aTextSize   = xTextLayout->getPreferredSize();

    aGuard.clear () ;

    // calc preferred size of status indicator
    sal_Int32 nWidth  = impl_getWidth()                 ;
    sal_Int32 nHeight = (2*STATUSINDICATOR_FREEBORDER)+aTextSize.Height ;

    // norm to minimum
    if ( nWidth<STATUSINDICATOR_DEFAULT_WIDTH )
    {
        nWidth = STATUSINDICATOR_DEFAULT_WIDTH ;
    }
    if ( nHeight<STATUSINDICATOR_DEFAULT_HEIGHT )
    {
        nHeight = STATUSINDICATOR_DEFAULT_HEIGHT ;
    }

    // return to caller
    return Size ( nWidth, nHeight ) ;
}

//____________________________________________________________________________________________________________
//  XLayoutConstrains
//____________________________________________________________________________________________________________

Size SAL_CALL StatusIndicator::calcAdjustedSize ( const Size& /*rNewSize*/ ) throw( RuntimeException )
{
    return getPreferredSize () ;
}

//____________________________________________________________________________________________________________
//  XControl
//____________________________________________________________________________________________________________

void SAL_CALL StatusIndicator::createPeer (
    const Reference< XToolkit > & rToolkit,
    const Reference< XWindowPeer > & rParent
) throw( RuntimeException )
{
    if( getPeer().is() == sal_False )
    {
        BaseContainerControl::createPeer( rToolkit, rParent );

        // If user forget to call "setPosSize()", we have still a correct size.
        // And a "MinimumSize" IS A "MinimumSize"!
        // We change not the position of control at this point.
        Size aDefaultSize = getMinimumSize () ;
        setPosSize ( 0, 0, aDefaultSize.Width, aDefaultSize.Height, PosSize::SIZE ) ;
    }
}

//____________________________________________________________________________________________________________
//  XControl
//____________________________________________________________________________________________________________

sal_Bool SAL_CALL StatusIndicator::setModel ( const Reference< XControlModel > & /*rModel*/ ) throw( RuntimeException )
{
    // We have no model.
    return sal_False ;
}

//____________________________________________________________________________________________________________
//  XControl
//____________________________________________________________________________________________________________

Reference< XControlModel > SAL_CALL StatusIndicator::getModel () throw( RuntimeException )
{
    // We have no model.
    // return (XControlModel*)this ;
    return Reference< XControlModel >  () ;
}

//____________________________________________________________________________________________________________
//  XComponent
//____________________________________________________________________________________________________________

void SAL_CALL StatusIndicator::dispose () throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex ) ;

    // "removeControl()" control the state of a reference
    Reference< XControl >  xTextControl     ( m_xText       , UNO_QUERY );
    Reference< XControl >  xProgressControl ( m_xProgressBar, UNO_QUERY );

    removeControl( xTextControl     );
    removeControl( xProgressControl );

    // do'nt use "...->clear ()" or "... = XFixedText ()"
    // when other hold a reference at this object !!!
    xTextControl->dispose();
    xProgressControl->dispose();
    BaseContainerControl::dispose();
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void SAL_CALL StatusIndicator::setPosSize (
    sal_Int32 nX,
    sal_Int32 nY,
    sal_Int32 nWidth,
    sal_Int32 nHeight,
    sal_Int16 nFlags
) throw( RuntimeException )
{
    Rectangle   aBasePosSize = getPosSize () ;
    BaseContainerControl::setPosSize (nX, nY, nWidth, nHeight, nFlags) ;

    // if position or size changed
    if (
        ( nWidth  != aBasePosSize.Width ) ||
        ( nHeight != aBasePosSize.Height)
       )
    {
        // calc new layout for controls
        impl_recalcLayout ( WindowEvent(static_cast< OWeakObject* >(this),0,0,nWidth,nHeight,0,0,0,0) ) ;
        // clear background (!)
        // [Children were repainted in "recalcLayout" by setPosSize() automaticly!]
        getPeer()->invalidate(2);
        // and repaint the control
        impl_paint ( 0, 0, impl_getGraphicsPeer() ) ;
    }
}

//____________________________________________________________________________________________________________
//  impl but public method to register service
//____________________________________________________________________________________________________________

const Sequence< OUString > StatusIndicator::impl_getStaticSupportedServiceNames()
{
    MutexGuard aGuard( Mutex::getGlobalMutex() );
    Sequence< OUString > seqServiceNames( 1 );
    seqServiceNames.getArray() [0] = SERVICENAME_STATUSINDICATOR;
    return seqServiceNames ;
}

//____________________________________________________________________________________________________________
//  impl but public method to register service
//____________________________________________________________________________________________________________

const OUString StatusIndicator::impl_getStaticImplementationName()
{
    return OUString(IMPLEMENTATIONNAME_STATUSINDICATOR);
}

//____________________________________________________________________________________________________________
//  protected method
//____________________________________________________________________________________________________________

WindowDescriptor* StatusIndicator::impl_getWindowDescriptor( const Reference< XWindowPeer >& xParentPeer )
{
    // - used from "createPeer()" to set the values of an ::com::sun::star::awt::WindowDescriptor !!!
    // - if you will change the descriptor-values, you must override this virtuell function
    // - the caller must release the memory for this dynamical descriptor !!!

    WindowDescriptor* pDescriptor = new WindowDescriptor ;

    pDescriptor->Type               =   WindowClass_SIMPLE                              ;
    pDescriptor->WindowServiceName  =   "floatingwindow"                                ;
    pDescriptor->ParentIndex        =   -1                                              ;
    pDescriptor->Parent             =   xParentPeer                                     ;
    pDescriptor->Bounds             =   getPosSize ()                                   ;

    return pDescriptor ;
}

//____________________________________________________________________________________________________________
//  protected method
//____________________________________________________________________________________________________________

void StatusIndicator::impl_paint ( sal_Int32 nX, sal_Int32 nY, const Reference< XGraphics > & rGraphics )
{
    // This paint method ist not buffered !!
    // Every request paint the completely control. ( but only, if peer exist )
     if ( rGraphics.is () )
    {
        MutexGuard  aGuard (m_aMutex) ;

        // background = gray
        Reference< XWindowPeer > xPeer( impl_getPeerWindow(), UNO_QUERY );
        if( xPeer.is() == sal_True )
            xPeer->setBackground( STATUSINDICATOR_BACKGROUNDCOLOR );

        // FixedText background = gray
        Reference< XControl > xTextControl( m_xText, UNO_QUERY );
        xPeer = xTextControl->getPeer();
        if( xPeer.is() == sal_True )
            xPeer->setBackground( STATUSINDICATOR_BACKGROUNDCOLOR );

        // Progress background = gray
        xPeer = Reference< XWindowPeer >( m_xProgressBar, UNO_QUERY );
        if( xPeer.is() == sal_True )
            xPeer->setBackground( STATUSINDICATOR_BACKGROUNDCOLOR );

        // paint shadow border
        rGraphics->setLineColor ( STATUSINDICATOR_LINECOLOR_BRIGHT                          );
        rGraphics->drawLine     ( nX, nY, impl_getWidth(), nY               );
        rGraphics->drawLine     ( nX, nY, nX             , impl_getHeight() );

        rGraphics->setLineColor ( STATUSINDICATOR_LINECOLOR_SHADOW                                                              );
        rGraphics->drawLine     ( impl_getWidth()-1, impl_getHeight()-1, impl_getWidth()-1, nY                  );
        rGraphics->drawLine     ( impl_getWidth()-1, impl_getHeight()-1, nX               , impl_getHeight()-1  );
    }
}

//____________________________________________________________________________________________________________
//  protected method
//____________________________________________________________________________________________________________

void StatusIndicator::impl_recalcLayout ( const WindowEvent& aEvent )
{
    sal_Int32   nX_ProgressBar          ;
    sal_Int32   nY_ProgressBar          ;
    sal_Int32   nWidth_ProgressBar      ;
    sal_Int32   nHeight_ProgressBar     ;
    sal_Int32   nX_Text                 ;
    sal_Int32   nY_Text                 ;
    sal_Int32   nWidth_Text             ;
    sal_Int32   nHeight_Text            ;

    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex ) ;

    // get information about required place of child controls
    Size                            aWindowSize     ( aEvent.Width, aEvent.Height );
    Reference< XLayoutConstrains >  xTextLayout     ( m_xText, UNO_QUERY );
    Size                            aTextSize       = xTextLayout->getPreferredSize();

    if( aWindowSize.Width < STATUSINDICATOR_DEFAULT_WIDTH )
    {
        aWindowSize.Width = STATUSINDICATOR_DEFAULT_WIDTH;
    }
    if( aWindowSize.Height < STATUSINDICATOR_DEFAULT_HEIGHT )
    {
        aWindowSize.Height = STATUSINDICATOR_DEFAULT_HEIGHT;
    }

    // calc position and size of child controls
    nX_Text             = STATUSINDICATOR_FREEBORDER                                    ;
    nY_Text             = STATUSINDICATOR_FREEBORDER                                    ;
    nWidth_Text         = aTextSize.Width                               ;
    nHeight_Text        = aTextSize.Height                              ;

    nX_ProgressBar      = nX_Text+nWidth_Text+STATUSINDICATOR_FREEBORDER                ;
    nY_ProgressBar      = nY_Text                                       ;
    nWidth_ProgressBar  = aWindowSize.Width-nWidth_Text-(3*STATUSINDICATOR_FREEBORDER)  ;
    nHeight_ProgressBar = nHeight_Text                                  ;

    // Set new position and size on all controls
    Reference< XWindow >  xTextWindow       ( m_xText       , UNO_QUERY );
    Reference< XWindow >  xProgressWindow   ( m_xProgressBar, UNO_QUERY );

    xTextWindow->setPosSize     ( nX_Text       , nY_Text       , nWidth_Text       , nHeight_Text          , 15 ) ;
    xProgressWindow->setPosSize ( nX_ProgressBar, nY_ProgressBar, nWidth_ProgressBar, nHeight_ProgressBar   , 15 ) ;
}

}   // namespace unocontrols

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
