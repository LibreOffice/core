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

#include <statusindicator.hxx>

#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XFixedText.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <progressbar.hxx>

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::task;

constexpr OUStringLiteral FIXEDTEXT_SERVICENAME = u"com.sun.star.awt.UnoControlFixedText";
constexpr OUStringLiteral FIXEDTEXT_MODELNAME = u"com.sun.star.awt.UnoControlFixedTextModel";
constexpr OUStringLiteral CONTROLNAME_TEXT = u"Text"; // identifier the control in container
constexpr OUStringLiteral CONTROLNAME_PROGRESSBAR = u"ProgressBar"; //              -||-

namespace unocontrols {

//  construct/destruct

StatusIndicator::StatusIndicator( const css::uno::Reference< XComponentContext >& rxContext )
    : BaseContainerControl  ( rxContext  )
{
    // It's not allowed to work with member in this method (refcounter !!!)
    // But with a HACK (++refcount) its "OK" :-(
    osl_atomic_increment(&m_refCount);

    // Create instances for fixedtext and progress ...
    m_xText.set( rxContext->getServiceManager()->createInstanceWithContext( FIXEDTEXT_SERVICENAME, rxContext ), UNO_QUERY );
    m_xProgressBar = new ProgressBar(rxContext);
    // ... cast controls to css::uno::Reference< XControl > and set model ...
    // ( ProgressBar has no model !!! )
    css::uno::Reference< XControl > xTextControl      ( m_xText       , UNO_QUERY );
    xTextControl->setModel( css::uno::Reference< XControlModel >( rxContext->getServiceManager()->createInstanceWithContext( FIXEDTEXT_MODELNAME, rxContext ), UNO_QUERY ) );
    // ... and add controls to basecontainercontrol!
    addControl( CONTROLNAME_TEXT, xTextControl    );
    addControl( CONTROLNAME_PROGRESSBAR, m_xProgressBar );
    // FixedText make it automatically visible by himself ... but not the progressbar !!!
    // it must be set explicitly
    m_xProgressBar->setVisible( true );
    // Reset to defaults !!!
    // (progressbar take automatically its own defaults)
    m_xText->setText( u""_ustr );

    osl_atomic_decrement(&m_refCount);
}

StatusIndicator::~StatusIndicator() {}

//  XInterface

Any SAL_CALL StatusIndicator::queryInterface( const Type& rType )
{
    // Ask for my own supported interfaces ...
    // Attention: XTypeProvider and XInterface are supported by WeakComponentImplHelper!
    Any aReturn ( ::cppu::queryInterface( rType                                     ,
                                          static_cast< XLayoutConstrains*   > ( this )  ,
                                          static_cast< XStatusIndicator*    > ( this )
                                        )
                );

    // If searched interface not supported by this class ...
    if ( !aReturn.hasValue() )
    {
        // ... ask baseclasses.
        aReturn = BaseControl::queryInterface( rType );
    }

    return aReturn;
}

//  XInterface

void SAL_CALL StatusIndicator::acquire() noexcept
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    BaseControl::acquire();
}

//  XInterface

void SAL_CALL StatusIndicator::release() noexcept
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    BaseControl::release();
}

//  XTypeProvider

Sequence< Type > SAL_CALL StatusIndicator::getTypes()
{
    static OTypeCollection ourTypeCollection(
                cppu::UnoType<XLayoutConstrains>::get(),
                cppu::UnoType<XStatusIndicator>::get(),
                BaseContainerControl::getTypes() );

    return ourTypeCollection.getTypes();
}

//  XStatusIndicator

void SAL_CALL StatusIndicator::start( const OUString& sText, sal_Int32 nRange )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // Initialize status controls with given values.
    m_xText->setText( sText );
    m_xProgressBar->setRange( 0, nRange );
    // force repaint ... fixedtext has changed !
    impl_recalcLayout ( WindowEvent(getXWeak(),0,0,impl_getWidth(),impl_getHeight(),0,0,0,0) );
}

//  XStatusIndicator

void SAL_CALL StatusIndicator::end()
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // Clear values of status controls.
    m_xText->setText( OUString() );
    m_xProgressBar->setValue( 0 );
    setVisible( false );
}

//  XStatusIndicator

void SAL_CALL StatusIndicator::setText( const OUString& sText )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // Take text on right control
    m_xText->setText( sText );
}

//  XStatusIndicator

void SAL_CALL StatusIndicator::setValue( sal_Int32 nValue )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // Take value on right control
    m_xProgressBar->setValue( nValue );
}

//  XStatusIndicator

void SAL_CALL StatusIndicator::reset()
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // Clear values of status controls.
    // (Don't hide the window! User will reset current values ... but he will not finish using of indicator!)
    m_xText->setText( OUString() );
    m_xProgressBar->setValue( 0 );
}

//  XLayoutConstrains

Size SAL_CALL StatusIndicator::getMinimumSize ()
{
    return Size (STATUSINDICATOR_DEFAULT_WIDTH, STATUSINDICATOR_DEFAULT_HEIGHT);
}

//  XLayoutConstrains

Size SAL_CALL StatusIndicator::getPreferredSize ()
{
    // Ready for multithreading
    ClearableMutexGuard aGuard ( m_aMutex );

    // get information about required place of child controls
    css::uno::Reference< XLayoutConstrains >  xTextLayout ( m_xText, UNO_QUERY );
    Size                            aTextSize   = xTextLayout->getPreferredSize();

    aGuard.clear ();

    // calc preferred size of status indicator
    sal_Int32 nWidth  = impl_getWidth();
    sal_Int32 nHeight = (2*STATUSINDICATOR_FREEBORDER)+aTextSize.Height;

    // norm to minimum
    if ( nWidth<STATUSINDICATOR_DEFAULT_WIDTH )
    {
        nWidth = STATUSINDICATOR_DEFAULT_WIDTH;
    }
    if ( nHeight<STATUSINDICATOR_DEFAULT_HEIGHT )
    {
        nHeight = STATUSINDICATOR_DEFAULT_HEIGHT;
    }

    // return to caller
    return Size ( nWidth, nHeight );
}

//  XLayoutConstrains

Size SAL_CALL StatusIndicator::calcAdjustedSize ( const Size& /*rNewSize*/ )
{
    return getPreferredSize ();
}

//  XControl

void SAL_CALL StatusIndicator::createPeer (
    const css::uno::Reference< XToolkit > & rToolkit,
    const css::uno::Reference< XWindowPeer > & rParent
)
{
    if( !getPeer().is() )
    {
        BaseContainerControl::createPeer( rToolkit, rParent );

        // If user forget to call "setPosSize()", we have still a correct size.
        // And a "MinimumSize" IS A "MinimumSize"!
        // We change not the position of control at this point.
        Size aDefaultSize = getMinimumSize ();
        setPosSize ( 0, 0, aDefaultSize.Width, aDefaultSize.Height, PosSize::SIZE );
    }
}

//  XControl

sal_Bool SAL_CALL StatusIndicator::setModel ( const css::uno::Reference< XControlModel > & /*rModel*/ )
{
    // We have no model.
    return false;
}

//  XControl

css::uno::Reference< XControlModel > SAL_CALL StatusIndicator::getModel ()
{
    // We have no model.
    // return (XControlModel*)this;
    return css::uno::Reference< XControlModel >  ();
}

//  XComponent

void SAL_CALL StatusIndicator::dispose ()
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex );

    // "removeControl()" control the state of a reference
    css::uno::Reference< XControl >  xTextControl     ( m_xText       , UNO_QUERY );

    removeControl( xTextControl     );
    removeControl( m_xProgressBar );

    // don't use "...->clear ()" or "... = XFixedText ()"
    // when other hold a reference at this object !!!
    xTextControl->dispose();
    m_xProgressBar->dispose();
    m_xProgressBar.clear();
    m_xText.clear();
    BaseContainerControl::dispose();
}

//  XWindow

void SAL_CALL StatusIndicator::setPosSize (
    sal_Int32 nX,
    sal_Int32 nY,
    sal_Int32 nWidth,
    sal_Int32 nHeight,
    sal_Int16 nFlags
)
{
    Rectangle   aBasePosSize = getPosSize ();
    BaseContainerControl::setPosSize (nX, nY, nWidth, nHeight, nFlags);

    // if position or size changed
    if (
        ( nWidth  != aBasePosSize.Width ) ||
        ( nHeight != aBasePosSize.Height)
       )
    {
        // calc new layout for controls
        impl_recalcLayout ( WindowEvent(getXWeak(),0,0,nWidth,nHeight,0,0,0,0) );
        // clear background (!)
        // [Children were repainted in "recalcLayout" by setPosSize() automatically!]
        getPeer()->invalidate(2);
        // and repaint the control
        impl_paint ( 0, 0, impl_getGraphicsPeer() );
    }
}

//  protected method

WindowDescriptor StatusIndicator::impl_getWindowDescriptor( const css::uno::Reference< XWindowPeer >& xParentPeer )
{
    WindowDescriptor aDescriptor;

    aDescriptor.Type               =   WindowClass_SIMPLE;
    aDescriptor.WindowServiceName  =   "floatingwindow";
    aDescriptor.ParentIndex        =   -1;
    aDescriptor.Parent             =   xParentPeer;
    aDescriptor.Bounds             =   getPosSize ();

    return aDescriptor;
}

//  protected method

void StatusIndicator::impl_paint ( sal_Int32 nX, sal_Int32 nY, const css::uno::Reference< XGraphics > & rGraphics )
{
    // This paint method is not buffered!
    // Every request paint the completely control. (But only, if peer exist)
    if ( !rGraphics.is () )
        return;

    MutexGuard  aGuard (m_aMutex);

    // background = gray
    css::uno::Reference< XWindowPeer > xPeer( impl_getPeerWindow(), UNO_QUERY );
    if( xPeer.is() )
        xPeer->setBackground( STATUSINDICATOR_BACKGROUNDCOLOR );

    // FixedText background = gray
    css::uno::Reference< XControl > xTextControl( m_xText, UNO_QUERY );
    xPeer = xTextControl->getPeer();
    if( xPeer.is() )
        xPeer->setBackground( STATUSINDICATOR_BACKGROUNDCOLOR );

    // Progress background = gray
    xPeer = m_xProgressBar->getPeer();
    if( xPeer.is() )
        xPeer->setBackground( STATUSINDICATOR_BACKGROUNDCOLOR );

    // paint shadow border
    rGraphics->setLineColor ( STATUSINDICATOR_LINECOLOR_BRIGHT                          );
    rGraphics->drawLine     ( nX, nY, impl_getWidth(), nY               );
    rGraphics->drawLine     ( nX, nY, nX             , impl_getHeight() );

    rGraphics->setLineColor ( STATUSINDICATOR_LINECOLOR_SHADOW                                                              );
    rGraphics->drawLine     ( impl_getWidth()-1, impl_getHeight()-1, impl_getWidth()-1, nY                  );
    rGraphics->drawLine     ( impl_getWidth()-1, impl_getHeight()-1, nX               , impl_getHeight()-1  );
}

//  protected method

void StatusIndicator::impl_recalcLayout ( const WindowEvent& aEvent )
{
    sal_Int32   nX_ProgressBar;
    sal_Int32   nY_ProgressBar;
    sal_Int32   nWidth_ProgressBar;
    sal_Int32   nHeight_ProgressBar;
    sal_Int32   nX_Text;
    sal_Int32   nY_Text;
    sal_Int32   nWidth_Text;
    sal_Int32   nHeight_Text;

    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex );

    // get information about required place of child controls
    Size                            aWindowSize     ( aEvent.Width, aEvent.Height );
    css::uno::Reference< XLayoutConstrains >  xTextLayout     ( m_xText, UNO_QUERY );
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
    nX_Text             = STATUSINDICATOR_FREEBORDER;
    nY_Text             = STATUSINDICATOR_FREEBORDER;
    nWidth_Text         = aTextSize.Width;
    nHeight_Text        = aTextSize.Height;

    nX_ProgressBar      = nX_Text+nWidth_Text+STATUSINDICATOR_FREEBORDER;
    nY_ProgressBar      = nY_Text;
    nWidth_ProgressBar  = aWindowSize.Width-nWidth_Text-(3*STATUSINDICATOR_FREEBORDER);
    nHeight_ProgressBar = nHeight_Text;

    // Set new position and size on all controls
    css::uno::Reference< XWindow >  xTextWindow       ( m_xText       , UNO_QUERY );

    xTextWindow->setPosSize     ( nX_Text       , nY_Text       , nWidth_Text       , nHeight_Text          , 15 );
    m_xProgressBar->setPosSize( nX_ProgressBar, nY_ProgressBar, nWidth_ProgressBar, nHeight_ProgressBar, 15 );
}

}   // namespace unocontrols

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
stardiv_UnoControls_StatusIndicator_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new unocontrols::StatusIndicator(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
