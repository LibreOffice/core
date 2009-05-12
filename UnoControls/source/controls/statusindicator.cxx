/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: statusindicator.cxx,v $
 * $Revision: 1.7 $
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

#include "statusindicator.hxx"

//____________________________________________________________________________________________________________
//  includes of other projects
//____________________________________________________________________________________________________________
#include <com/sun/star/awt/InvalidateStyle.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <tools/debug.hxx>

//____________________________________________________________________________________________________________
//  includes of my project
//____________________________________________________________________________________________________________
#include "progressbar.hxx"

//____________________________________________________________________________________________________________
//  namespace
//____________________________________________________________________________________________________________

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

StatusIndicator::StatusIndicator( const Reference< XMultiServiceFactory >& xFactory )
    : BaseContainerControl  ( xFactory  )
{
    // Its not allowed to work with member in this method (refcounter !!!)
    // But with a HACK (++refcount) its "OK" :-(
    ++m_refCount ;

    // Create instances for fixedtext and progress ...
    m_xText         = Reference< XFixedText >   ( xFactory->createInstance( OUString::createFromAscii( FIXEDTEXT_SERVICENAME    ) ), UNO_QUERY );
    m_xProgressBar  = Reference< XProgressBar > ( xFactory->createInstance( OUString::createFromAscii( SERVICENAME_PROGRESSBAR  ) ), UNO_QUERY );
    // ... cast controls to Reference< XControl > and set model ...
    // ( ProgressBar has no model !!! )
    Reference< XControl > xTextControl      ( m_xText       , UNO_QUERY );
    Reference< XControl > xProgressControl  ( m_xProgressBar, UNO_QUERY );
    xTextControl->setModel( Reference< XControlModel >( xFactory->createInstance( OUString::createFromAscii( FIXEDTEXT_MODELNAME ) ), UNO_QUERY ) );
    // ... and add controls to basecontainercontrol!
    addControl( OUString::createFromAscii( CONTROLNAME_TEXT         ), xTextControl     );
    addControl( OUString::createFromAscii( CONTROLNAME_PROGRESSBAR  ), xProgressControl );
    // FixedText make it automaticly visible by himself ... but not the progressbar !!!
    // it must be set explicitly
    Reference< XWindow > xProgressWindow( m_xProgressBar, UNO_QUERY );
    xProgressWindow->setVisible( sal_True );
    // Reset to defaults !!!
    // (progressbar take automaticly its own defaults)
    m_xText->setText( OUString::createFromAscii( DEFAULT_TEXT ) );

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
            static OTypeCollection aTypeCollection  (   ::getCppuType(( const Reference< XLayoutConstrains  >*)NULL )   ,
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
    Any aReturn ( ::cppu::queryInterface(   aType                                       ,
                                               static_cast< XLayoutConstrains*  > ( this )  ,
                                               static_cast< XStatusIndicator*   > ( this )
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
    return Size (DEFAULT_WIDTH, DEFAULT_HEIGHT) ;
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
    sal_Int32 nHeight = (2*FREEBORDER)+aTextSize.Height ;

    // norm to minimum
    if ( nWidth<DEFAULT_WIDTH )
    {
        nWidth = DEFAULT_WIDTH ;
    }
    if ( nHeight<DEFAULT_HEIGHT )
    {
        nHeight = DEFAULT_HEIGHT ;
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

void SAL_CALL StatusIndicator::createPeer ( const Reference< XToolkit > & rToolkit, const Reference< XWindowPeer > & rParent    ) throw( RuntimeException )
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

void SAL_CALL StatusIndicator::setPosSize ( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int16 nFlags ) throw( RuntimeException )
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
        // [Childs was repainted in "recalcLayout" by setPosSize() automaticly!]
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
    seqServiceNames.getArray() [0] = OUString::createFromAscii( SERVICENAME_STATUSINDICATOR );
    return seqServiceNames ;
}

//____________________________________________________________________________________________________________
//  impl but public method to register service
//____________________________________________________________________________________________________________

const OUString StatusIndicator::impl_getStaticImplementationName()
{
    return OUString::createFromAscii( IMPLEMENTATIONNAME_STATUSINDICATOR );
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
    pDescriptor->WindowServiceName  =   OUString::createFromAscii( "floatingwindow" )   ;
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
            xPeer->setBackground( BACKGROUNDCOLOR );

        // FixedText background = gray
        Reference< XControl > xTextControl( m_xText, UNO_QUERY );
        xPeer = xTextControl->getPeer();
        if( xPeer.is() == sal_True )
            xPeer->setBackground( BACKGROUNDCOLOR );

        // Progress background = gray
        xPeer = Reference< XWindowPeer >( m_xProgressBar, UNO_QUERY );
        if( xPeer.is() == sal_True )
            xPeer->setBackground( BACKGROUNDCOLOR );

        // paint shadow border
        rGraphics->setLineColor ( LINECOLOR_BRIGHT                          );
        rGraphics->drawLine     ( nX, nY, impl_getWidth(), nY               );
        rGraphics->drawLine     ( nX, nY, nX             , impl_getHeight() );

        rGraphics->setLineColor ( LINECOLOR_SHADOW                                                              );
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

    if( aWindowSize.Width < DEFAULT_WIDTH )
    {
        aWindowSize.Width = DEFAULT_WIDTH;
    }
    if( aWindowSize.Height < DEFAULT_HEIGHT )
    {
        aWindowSize.Height = DEFAULT_HEIGHT;
    }

    // calc position and size of child controls
    nX_Text             = FREEBORDER                                    ;
    nY_Text             = FREEBORDER                                    ;
    nWidth_Text         = aTextSize.Width                               ;
    nHeight_Text        = aTextSize.Height                              ;

    nX_ProgressBar      = nX_Text+nWidth_Text+FREEBORDER                ;
    nY_ProgressBar      = nY_Text                                       ;
    nWidth_ProgressBar  = aWindowSize.Width-nWidth_Text-(3*FREEBORDER)  ;
    nHeight_ProgressBar = nHeight_Text                                  ;

    // Set new position and size on all controls
    Reference< XWindow >  xTextWindow       ( m_xText       , UNO_QUERY );
    Reference< XWindow >  xProgressWindow   ( m_xProgressBar, UNO_QUERY );

    xTextWindow->setPosSize     ( nX_Text       , nY_Text       , nWidth_Text       , nHeight_Text          , 15 ) ;
    xProgressWindow->setPosSize ( nX_ProgressBar, nY_ProgressBar, nWidth_ProgressBar, nHeight_ProgressBar   , 15 ) ;
}

//____________________________________________________________________________________________________________
//  debug methods
//____________________________________________________________________________________________________________

#if OSL_DEBUG_LEVEL > 1

#endif  // #if OSL_DEBUG_LEVEL > 1

}   // namespace unocontrols
