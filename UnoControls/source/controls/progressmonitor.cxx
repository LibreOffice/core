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

#include "progressmonitor.hxx"

#include <com/sun/star/awt/GradientStyle.hpp>
#include <com/sun/star/awt/RasterOperation.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <tools/debug.hxx>
#include <algorithm>

#include "progressbar.hxx"

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::awt;

using ::std::vector;
using ::std::find;

namespace unocontrols{

ProgressMonitor::ProgressMonitor( const css::uno::Reference< XComponentContext >& rxContext )
    : BaseContainerControl  ( rxContext  )
{
    // Its not allowed to work with member in this method (refcounter !!!)
    // But with a HACK (++refcount) its "OK" :-(
    ++m_refCount;

    // Create instances for fixedtext, button and progress ...

    m_xTopic_Top.set   ( rxContext->getServiceManager()->createInstanceWithContext( FIXEDTEXT_SERVICENAME, rxContext ), UNO_QUERY );
    m_xText_Top.set    ( rxContext->getServiceManager()->createInstanceWithContext( FIXEDTEXT_SERVICENAME, rxContext ), UNO_QUERY );
    m_xTopic_Bottom.set( rxContext->getServiceManager()->createInstanceWithContext( FIXEDTEXT_SERVICENAME, rxContext ), UNO_QUERY );
    m_xText_Bottom.set ( rxContext->getServiceManager()->createInstanceWithContext( FIXEDTEXT_SERVICENAME, rxContext ), UNO_QUERY );
    m_xButton.set      ( rxContext->getServiceManager()->createInstanceWithContext( BUTTON_SERVICENAME, rxContext ), UNO_QUERY );
    m_xProgressBar = VclPtr<ProgressBar>::Create(rxContext);

    // ... cast controls to Reference< XControl >  (for "setModel"!) ...
    css::uno::Reference< XControl >   xRef_Topic_Top      ( m_xTopic_Top    , UNO_QUERY );
    css::uno::Reference< XControl >   xRef_Text_Top       ( m_xText_Top     , UNO_QUERY );
    css::uno::Reference< XControl >   xRef_Topic_Bottom   ( m_xTopic_Bottom , UNO_QUERY );
    css::uno::Reference< XControl >   xRef_Text_Bottom    ( m_xText_Bottom  , UNO_QUERY );
    css::uno::Reference< XControl >   xRef_Button         ( m_xButton       , UNO_QUERY );

    // ... set models ...
    xRef_Topic_Top->setModel    ( css::uno::Reference< XControlModel > ( rxContext->getServiceManager()->createInstanceWithContext( FIXEDTEXT_MODELNAME, rxContext ), UNO_QUERY ) );
    xRef_Text_Top->setModel     ( css::uno::Reference< XControlModel > ( rxContext->getServiceManager()->createInstanceWithContext( FIXEDTEXT_MODELNAME, rxContext ), UNO_QUERY ) );
    xRef_Topic_Bottom->setModel ( css::uno::Reference< XControlModel > ( rxContext->getServiceManager()->createInstanceWithContext( FIXEDTEXT_MODELNAME, rxContext ), UNO_QUERY ) );
    xRef_Text_Bottom->setModel  ( css::uno::Reference< XControlModel > ( rxContext->getServiceManager()->createInstanceWithContext( FIXEDTEXT_MODELNAME, rxContext ), UNO_QUERY ) );
    xRef_Button->setModel       ( css::uno::Reference< XControlModel > ( rxContext->getServiceManager()->createInstanceWithContext( BUTTON_MODELNAME, rxContext ), UNO_QUERY ) );
    // ProgressBar has no model !!!

    // ... and add controls to basecontainercontrol!
    addControl ( CONTROLNAME_TEXT, xRef_Topic_Top           );
    addControl ( CONTROLNAME_TEXT, xRef_Text_Top            );
    addControl ( CONTROLNAME_TEXT, xRef_Topic_Bottom        );
    addControl ( CONTROLNAME_TEXT, xRef_Text_Bottom         );
    addControl ( CONTROLNAME_BUTTON, xRef_Button            );
    addControl ( CONTROLNAME_PROGRESSBAR, m_xProgressBar.get() );

    // FixedText make it automatically visible by himself ... but not the progressbar !!!
    // it must be set explicitly
    m_xProgressBar->setVisible( true );

    // Reset to defaults !!!
    // (progressbar take automatically its own defaults)
    m_xButton->setLabel      ( DEFAULT_BUTTONLABEL );
    m_xTopic_Top->setText    ( PROGRESSMONITOR_DEFAULT_TOPIC );
    m_xText_Top->setText     ( PROGRESSMONITOR_DEFAULT_TEXT );
    m_xTopic_Bottom->setText ( PROGRESSMONITOR_DEFAULT_TOPIC );
    m_xText_Bottom->setText  ( PROGRESSMONITOR_DEFAULT_TEXT );

    --m_refCount;
}

ProgressMonitor::~ProgressMonitor()
{
    impl_cleanMemory ();
}

//  XInterface
Any SAL_CALL ProgressMonitor::queryInterface( const Type& rType ) throw( RuntimeException, std::exception )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.
    Any aReturn;
    css::uno::Reference< XInterface > xDel = BaseContainerControl::impl_getDelegator();
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

    return aReturn;
}

//  XInterface
void SAL_CALL ProgressMonitor::acquire() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    BaseControl::acquire();
}

//  XInterface
void SAL_CALL ProgressMonitor::release() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    BaseControl::release();
}

//  XTypeProvider
Sequence< Type > SAL_CALL ProgressMonitor::getTypes() throw( RuntimeException, std::exception )
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pTypeCollection is NULL - for the second call pTypeCollection is different from NULL!
    static OTypeCollection* pTypeCollection = nullptr;

    if ( pTypeCollection == nullptr )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        MutexGuard aGuard( Mutex::getGlobalMutex() );

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pTypeCollection == nullptr )
        {
            // Create a static typecollection ...
            static OTypeCollection aTypeCollection ( cppu::UnoType<XLayoutConstrains>::get(),
                                                     cppu::UnoType<XButton>::get(),
                                                     cppu::UnoType<XProgressMonitor>::get(),
                                                     BaseContainerControl::getTypes()
                                                   );
            // ... and set his address to static pointer!
            pTypeCollection = &aTypeCollection;
        }
    }

    return pTypeCollection->getTypes();
}

//  XAggregation
Any SAL_CALL ProgressMonitor::queryAggregation( const Type& aType ) throw( RuntimeException, std::exception )
{
    // Ask for my own supported interfaces ...
    // Attention: XTypeProvider and XInterface are supported by OComponentHelper!
    Any aReturn ( ::cppu::queryInterface( aType ,
                                          static_cast< XLayoutConstrains* > ( this ) ,
                                          static_cast< XButton*           > ( this ) ,
                                          static_cast< XProgressMonitor*  > ( this )
                                        )
                );

    // If searched interface not supported by this class ...
    if ( !aReturn.hasValue() )
    {
        // ... ask baseclasses.
        aReturn = BaseControl::queryAggregation( aType );
    }

    return aReturn;
}

//  XProgressMonitor
void SAL_CALL ProgressMonitor::addText(
    const OUString& rTopic,
    const OUString& rText,
    sal_Bool bbeforeProgress
) throw( RuntimeException, std::exception )
{
    // Safe impossible cases
    // Check valid call of this method.
    DBG_ASSERT ( impl_debug_checkParameter ( rTopic, rText, bbeforeProgress )   , "ProgressMonitor::addText()\nCall without valid parameters!\n");
    DBG_ASSERT ( !(impl_searchTopic ( rTopic, bbeforeProgress ) != nullptr )       , "ProgresMonitor::addText()\nThe text already exist.\n"        );

    // Do nothing (in Release), if topic already exist.
    if ( impl_searchTopic ( rTopic, bbeforeProgress ) != nullptr )
    {
        return;
    }

    // Else ... take memory for new item ...
    IMPL_TextlistItem*  pTextItem = new IMPL_TextlistItem;

    // Set values ...
    pTextItem->sTopic   = rTopic;
    pTextItem->sText    = rText;

    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex );

    // ... and insert it in right list.
    if ( bbeforeProgress )
    {
        maTextlist_Top.push_back( pTextItem );
    }
    else
    {
        maTextlist_Bottom.push_back( pTextItem );
    }

    // ... update window
    impl_rebuildFixedText   ();
    impl_recalcLayout       ();
}

//  XProgressMonitor
void SAL_CALL ProgressMonitor::removeText ( const OUString& rTopic, sal_Bool bbeforeProgress ) throw( RuntimeException, std::exception )
{
    // Safe impossible cases
    // Check valid call of this method.
    DBG_ASSERT ( impl_debug_checkParameter ( rTopic, bbeforeProgress ), "ProgressMonitor::removeText()\nCall without valid parameters!\n" );

    // Search the topic ...
    IMPL_TextlistItem* pSearchItem = impl_searchTopic ( rTopic, bbeforeProgress );

    if ( pSearchItem != nullptr )
    {
        // Ready for multithreading
        MutexGuard aGuard ( m_aMutex );

        // ... delete item from right list ...
        if ( bbeforeProgress )
        {
            vector< IMPL_TextlistItem* >::iterator
                itr = find( maTextlist_Top.begin(), maTextlist_Top.end(), pSearchItem );
            if (itr != maTextlist_Top.end())
                maTextlist_Top.erase(itr);
        }
        else
        {
            vector< IMPL_TextlistItem* >::iterator
                itr = find( maTextlist_Bottom.begin(), maTextlist_Bottom.end(), pSearchItem );
            if (itr != maTextlist_Bottom.end())
                maTextlist_Bottom.erase(itr);
        }

        delete pSearchItem;

        // ... and update window.
        impl_rebuildFixedText   ();
        impl_recalcLayout       ();
    }
}

//  XProgressMonitor
void SAL_CALL ProgressMonitor::updateText (
    const OUString& rTopic,
    const OUString& rText,
    sal_Bool bbeforeProgress
) throw( RuntimeException, std::exception )
{
    // Safe impossible cases
    // Check valid call of this method.
    DBG_ASSERT ( impl_debug_checkParameter ( rTopic, rText, bbeforeProgress ), "ProgressMonitor::updateText()\nCall without valid parameters!\n" );

    // Search topic ...
    IMPL_TextlistItem* pSearchItem = impl_searchTopic ( rTopic, bbeforeProgress );

    if ( pSearchItem != nullptr )
    {
        // Ready for multithreading
        MutexGuard aGuard ( m_aMutex );

        // ... update text ...
        pSearchItem->sText = rText;

        // ... and update window.
        impl_rebuildFixedText   ();
        impl_recalcLayout       ();
    }
}

//  XProgressBar
void SAL_CALL ProgressMonitor::setForegroundColor ( sal_Int32 nColor ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex );

    m_xProgressBar->setForegroundColor ( nColor );
}

//  XProgressBar
void SAL_CALL ProgressMonitor::setBackgroundColor ( sal_Int32 nColor ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex );

    m_xProgressBar->setBackgroundColor ( nColor );
}

//  XProgressBar
void SAL_CALL ProgressMonitor::setValue ( sal_Int32 nValue ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex );

    m_xProgressBar->setValue ( nValue );
}

//  XProgressBar
void SAL_CALL ProgressMonitor::setRange ( sal_Int32 nMin, sal_Int32 nMax ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex );

    m_xProgressBar->setRange ( nMin, nMax );
}

//  XProgressBar
sal_Int32 SAL_CALL ProgressMonitor::getValue () throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex );

    return m_xProgressBar->getValue ();
}

//  XButton
void SAL_CALL ProgressMonitor::addActionListener ( const css::uno::Reference< XActionListener > & rListener ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex );

    if ( m_xButton.is () )
    {
        m_xButton->addActionListener ( rListener );
    }
}

//  XButton
void SAL_CALL ProgressMonitor::removeActionListener ( const css::uno::Reference< XActionListener > & rListener ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex );

    if ( m_xButton.is () )
    {
        m_xButton->removeActionListener ( rListener );
    }
}

//  XButton
void SAL_CALL ProgressMonitor::setLabel ( const OUString& rLabel ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex );

    if ( m_xButton.is () )
    {
        m_xButton->setLabel ( rLabel );
    }
}

//  XButton
void SAL_CALL ProgressMonitor::setActionCommand ( const OUString& rCommand ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex );

    if ( m_xButton.is () )
    {
        m_xButton->setActionCommand ( rCommand );
    }
}

//  XLayoutConstrains
Size SAL_CALL ProgressMonitor::getMinimumSize () throw( RuntimeException, std::exception )
{
    return Size (PROGRESSMONITOR_DEFAULT_WIDTH, PROGRESSMONITOR_DEFAULT_HEIGHT);
}

//  XLayoutConstrains
Size SAL_CALL ProgressMonitor::getPreferredSize () throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    ClearableMutexGuard aGuard ( m_aMutex );

    // get information about required place of child controls
    css::uno::Reference< XLayoutConstrains >  xTopicLayout_Top        ( m_xTopic_Top      , UNO_QUERY );
    css::uno::Reference< XLayoutConstrains >  xTopicLayout_Bottom     ( m_xTopic_Bottom   , UNO_QUERY );
    css::uno::Reference< XLayoutConstrains >  xButtonLayout           ( m_xButton         , UNO_QUERY );

    Size        aTopicSize_Top      =   xTopicLayout_Top->getPreferredSize          ();
    Size        aTopicSize_Bottom   =   xTopicLayout_Bottom->getPreferredSize       ();
    Size        aButtonSize         =   xButtonLayout->getPreferredSize             ();
    Rectangle   aTempRectangle      = m_xProgressBar->getPosSize();
    Size        aProgressBarSize( aTempRectangle.Width, aTempRectangle.Height );

    aGuard.clear ();

    // calc preferred size of progressmonitor
    sal_Int32 nWidth   =  3 * PROGRESSMONITOR_FREEBORDER;
    nWidth  +=  aProgressBarSize.Width;

    sal_Int32 nHeight  =  6 * PROGRESSMONITOR_FREEBORDER;
    nHeight +=  aTopicSize_Top.Height;
    nHeight +=  aProgressBarSize.Height;
    nHeight +=  aTopicSize_Bottom.Height;
    nHeight +=  2;   // 1 for black line, 1 for white line = 3D-Line!
    nHeight +=  aButtonSize.Height;

    // norm to minimum
    if ( nWidth < PROGRESSMONITOR_DEFAULT_WIDTH )
    {
        nWidth = PROGRESSMONITOR_DEFAULT_WIDTH;
    }
    if ( nHeight < PROGRESSMONITOR_DEFAULT_HEIGHT )
    {
        nHeight = PROGRESSMONITOR_DEFAULT_HEIGHT;
    }

    // return to caller
    return Size ( nWidth, nHeight );
}

//  XLayoutConstrains
Size SAL_CALL ProgressMonitor::calcAdjustedSize ( const Size& /*rNewSize*/ ) throw( RuntimeException, std::exception )
{
    return getPreferredSize ();
}

//  XControl
void SAL_CALL ProgressMonitor::createPeer ( const css::uno::Reference< XToolkit > & rToolkit, const css::uno::Reference< XWindowPeer > & rParent    ) throw( RuntimeException, std::exception )
{
    if (!getPeer().is())
    {
        BaseContainerControl::createPeer ( rToolkit, rParent );

        // If user forget to call "setPosSize()", we have still a correct size.
        // And a "MinimumSize" IS A "MinimumSize"!
        // We change not the position of control at this point.
        Size aDefaultSize = getMinimumSize ();
        setPosSize ( 0, 0, aDefaultSize.Width, aDefaultSize.Height, PosSize::SIZE );
    }
}

//  XControl
sal_Bool SAL_CALL ProgressMonitor::setModel ( const css::uno::Reference< XControlModel > & /*rModel*/ ) throw( RuntimeException, std::exception )
{
    // We have no model.
    return false;
}

//  XControl
css::uno::Reference< XControlModel > SAL_CALL ProgressMonitor::getModel () throw( RuntimeException, std::exception )
{
    // We have no model.
    // return (XControlModel*)this;
    return css::uno::Reference< XControlModel >  ();
}

//  XComponent
void SAL_CALL ProgressMonitor::dispose () throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex );

    // "removeControl()" control the state of a reference
    css::uno::Reference< XControl >  xRef_Topic_Top       ( m_xTopic_Top      , UNO_QUERY );
    css::uno::Reference< XControl >  xRef_Text_Top        ( m_xText_Top       , UNO_QUERY );
    css::uno::Reference< XControl >  xRef_Topic_Bottom    ( m_xTopic_Bottom   , UNO_QUERY );
    css::uno::Reference< XControl >  xRef_Text_Bottom     ( m_xText_Bottom    , UNO_QUERY );
    css::uno::Reference< XControl >  xRef_Button          ( m_xButton         , UNO_QUERY );

    removeControl ( xRef_Topic_Top      );
    removeControl ( xRef_Text_Top       );
    removeControl ( xRef_Topic_Bottom   );
    removeControl ( xRef_Text_Bottom    );
    removeControl ( xRef_Button         );
    removeControl ( m_xProgressBar.get() );

    // do'nt use "...->clear ()" or "... = XFixedText ()"
    // when other hold a reference at this object !!!
    xRef_Topic_Top->dispose     ();
    xRef_Text_Top->dispose      ();
    xRef_Topic_Bottom->dispose  ();
    xRef_Text_Bottom->dispose   ();
    xRef_Button->dispose        ();
    m_xProgressBar->dispose();

    BaseContainerControl::dispose ();
}

//  XWindow
void SAL_CALL ProgressMonitor::setPosSize ( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int16 nFlags ) throw( RuntimeException, std::exception )
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
        impl_recalcLayout ();
        // clear background (!)
        // [Children were repainted in "recalcLayout" by setPosSize() automatically!]
        getPeer()->invalidate(2);
        // and repaint the control
        impl_paint ( 0, 0, impl_getGraphicsPeer() );
    }
}

//  impl but public method to register service
const Sequence< OUString > ProgressMonitor::impl_getStaticSupportedServiceNames()
{
    return css::uno::Sequence<OUString>();
}

//  impl but public method to register service
const OUString ProgressMonitor::impl_getStaticImplementationName()
{
    return OUString("stardiv.UnoControls.ProgressMonitor");
}

//  protected method
void ProgressMonitor::impl_paint ( sal_Int32 nX, sal_Int32 nY, const css::uno::Reference< XGraphics > & rGraphics )
{
    if (rGraphics.is())
    {
        // Ready for multithreading
        MutexGuard aGuard ( m_aMutex );

        // paint shadowed border around the progressmonitor
        rGraphics->setLineColor ( PROGRESSMONITOR_LINECOLOR_SHADOW                                                              );
        rGraphics->drawLine     ( impl_getWidth()-1, impl_getHeight()-1, impl_getWidth()-1, nY                  );
        rGraphics->drawLine     ( impl_getWidth()-1, impl_getHeight()-1, nX               , impl_getHeight()-1  );

        rGraphics->setLineColor ( PROGRESSMONITOR_LINECOLOR_BRIGHT                          );
        rGraphics->drawLine     ( nX, nY, impl_getWidth(), nY               );
        rGraphics->drawLine     ( nX, nY, nX             , impl_getHeight() );

        // Paint 3D-line
        rGraphics->setLineColor ( PROGRESSMONITOR_LINECOLOR_SHADOW  );
        rGraphics->drawLine     ( m_a3DLine.X, m_a3DLine.Y, m_a3DLine.X+m_a3DLine.Width, m_a3DLine.Y );

        rGraphics->setLineColor ( PROGRESSMONITOR_LINECOLOR_BRIGHT  );
        rGraphics->drawLine     ( m_a3DLine.X, m_a3DLine.Y+1, m_a3DLine.X+m_a3DLine.Width, m_a3DLine.Y+1 );
    }
}

//  private method
void ProgressMonitor::impl_recalcLayout ()
{
    sal_Int32   nX_Button;
    sal_Int32   nY_Button;
    sal_Int32   nWidth_Button;
    sal_Int32   nHeight_Button;

    sal_Int32   nX_ProgressBar;
    sal_Int32   nY_ProgressBar;
    sal_Int32   nWidth_ProgressBar;
    sal_Int32   nHeight_ProgressBar;

    sal_Int32   nX_Text_Top;
    sal_Int32   nY_Text_Top;
    sal_Int32   nWidth_Text_Top;
    sal_Int32   nHeight_Text_Top;

    sal_Int32   nX_Topic_Top;
    sal_Int32   nY_Topic_Top;
    sal_Int32   nWidth_Topic_Top;
    sal_Int32   nHeight_Topic_Top;

    sal_Int32   nX_Text_Bottom;
    sal_Int32   nY_Text_Bottom;
    sal_Int32   nWidth_Text_Bottom;
    sal_Int32   nHeight_Text_Bottom;

    sal_Int32   nX_Topic_Bottom;
    sal_Int32   nY_Topic_Bottom;
    sal_Int32   nWidth_Topic_Bottom;
    sal_Int32   nHeight_Topic_Bottom;

    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex );

    // get information about required place of child controls
    css::uno::Reference< XLayoutConstrains >  xTopicLayout_Top    ( m_xTopic_Top      , UNO_QUERY );
    css::uno::Reference< XLayoutConstrains >  xTextLayout_Top     ( m_xText_Top       , UNO_QUERY );
    css::uno::Reference< XLayoutConstrains >  xTopicLayout_Bottom ( m_xTopic_Bottom   , UNO_QUERY );
    css::uno::Reference< XLayoutConstrains >  xTextLayout_Bottom  ( m_xText_Bottom    , UNO_QUERY );
    css::uno::Reference< XLayoutConstrains >  xButtonLayout       ( m_xButton         , UNO_QUERY );

    Size    aTopicSize_Top      =   xTopicLayout_Top->getPreferredSize      ();
    Size    aTextSize_Top       =   xTextLayout_Top->getPreferredSize       ();
    Size    aTopicSize_Bottom   =   xTopicLayout_Bottom->getPreferredSize   ();
    Size    aTextSize_Bottom    =   xTextLayout_Bottom->getPreferredSize    ();
    Size    aButtonSize         =   xButtonLayout->getPreferredSize         ();

    // calc position and size of child controls
    // Button has preferred size!
    nWidth_Button           =   aButtonSize.Width;
    nHeight_Button          =   aButtonSize.Height;

    // Left column before progressbar has preferred size and fixed position.
    // But "Width" is oriented on left column below progressbar to!!! "max(...)"
    nX_Topic_Top            =   PROGRESSMONITOR_FREEBORDER;
    nY_Topic_Top            =   PROGRESSMONITOR_FREEBORDER;
    nWidth_Topic_Top        =   std::max( aTopicSize_Top.Width, aTopicSize_Bottom.Width );
    nHeight_Topic_Top       =   aTopicSize_Top.Height;

    // Right column before progressbar has relativ position to left column ...
    // ... and a size as rest of dialog size!
    nX_Text_Top             =   nX_Topic_Top+nWidth_Topic_Top+PROGRESSMONITOR_FREEBORDER;
    nY_Text_Top             =   nY_Topic_Top;
    nWidth_Text_Top         =   std::max ( aTextSize_Top.Width, aTextSize_Bottom.Width );
    // Fix size of this column to minimum!
    sal_Int32 nSummaryWidth = nWidth_Text_Top+nWidth_Topic_Top+(3*PROGRESSMONITOR_FREEBORDER);
    if ( nSummaryWidth < PROGRESSMONITOR_DEFAULT_WIDTH )
        nWidth_Text_Top     =   PROGRESSMONITOR_DEFAULT_WIDTH-nWidth_Topic_Top-(3*PROGRESSMONITOR_FREEBORDER);
    // Fix size of column to maximum!
    if ( nSummaryWidth > impl_getWidth() )
        nWidth_Text_Top     =   impl_getWidth()-nWidth_Topic_Top-(3*PROGRESSMONITOR_FREEBORDER);
    nHeight_Text_Top        =   nHeight_Topic_Top;

    // Position of progressbar is relativ to columns before.
    // Progressbar.Width  = Dialog.Width !!!
    // Progressbar.Height = Button.Height
    nX_ProgressBar          =   nX_Topic_Top;
    nY_ProgressBar          =   nY_Topic_Top+nHeight_Topic_Top+PROGRESSMONITOR_FREEBORDER;
    nWidth_ProgressBar      =   PROGRESSMONITOR_FREEBORDER+nWidth_Topic_Top+nWidth_Text_Top;
    nHeight_ProgressBar     =   nHeight_Button;

    // Oriented by left column before progressbar.
    nX_Topic_Bottom         =   nX_Topic_Top;
    nY_Topic_Bottom         =   nY_ProgressBar+nHeight_ProgressBar+PROGRESSMONITOR_FREEBORDER;
    nWidth_Topic_Bottom     =   nWidth_Topic_Top;
    nHeight_Topic_Bottom    =   aTopicSize_Bottom.Height;

    // Oriented by right column before progressbar.
    nX_Text_Bottom          =   nX_Topic_Bottom+nWidth_Topic_Bottom+PROGRESSMONITOR_FREEBORDER;
    nY_Text_Bottom          =   nY_Topic_Bottom;
    nWidth_Text_Bottom      =   nWidth_Text_Top;
    nHeight_Text_Bottom     =   nHeight_Topic_Bottom;

    // Oriented by progressbar.
    nX_Button               =   nX_ProgressBar+nWidth_ProgressBar-nWidth_Button;
    nY_Button               =   nY_Topic_Bottom+nHeight_Topic_Bottom+PROGRESSMONITOR_FREEBORDER;

    // Calc offsets to center controls
    sal_Int32   nDx;
    sal_Int32   nDy;

    nDx =   ( (2*PROGRESSMONITOR_FREEBORDER)+nWidth_ProgressBar                                                             );
    nDy =   ( (6*PROGRESSMONITOR_FREEBORDER)+nHeight_Topic_Top+nHeight_ProgressBar+nHeight_Topic_Bottom+2+nHeight_Button    );

    // At this point use original dialog size to center controls!
    nDx =   (impl_getWidth ()/2)-(nDx/2);
    nDy =   (impl_getHeight()/2)-(nDy/2);

    if ( nDx<0 )
    {
        nDx=0;
    }
    if ( nDy<0 )
    {
        nDy=0;
    }

    // Set new position and size on all controls
    css::uno::Reference< XWindow >  xRef_Topic_Top        ( m_xTopic_Top      , UNO_QUERY );
    css::uno::Reference< XWindow >  xRef_Text_Top         ( m_xText_Top       , UNO_QUERY );
    css::uno::Reference< XWindow >  xRef_Topic_Bottom     ( m_xTopic_Bottom   , UNO_QUERY );
    css::uno::Reference< XWindow >  xRef_Text_Bottom      ( m_xText_Bottom    , UNO_QUERY );
    css::uno::Reference< XWindow >  xRef_Button           ( m_xButton         , UNO_QUERY );

    xRef_Topic_Top->setPosSize    ( nDx+nX_Topic_Top    , nDy+nY_Topic_Top    , nWidth_Topic_Top    , nHeight_Topic_Top    , 15 );
    xRef_Text_Top->setPosSize     ( nDx+nX_Text_Top     , nDy+nY_Text_Top     , nWidth_Text_Top     , nHeight_Text_Top     , 15 );
    xRef_Topic_Bottom->setPosSize ( nDx+nX_Topic_Bottom , nDy+nY_Topic_Bottom , nWidth_Topic_Bottom , nHeight_Topic_Bottom , 15 );
    xRef_Text_Bottom->setPosSize  ( nDx+nX_Text_Bottom  , nDy+nY_Text_Bottom  , nWidth_Text_Bottom  , nHeight_Text_Bottom  , 15 );
    xRef_Button->setPosSize       ( nDx+nX_Button       , nDy+nY_Button       , nWidth_Button       , nHeight_Button       , 15 );
    m_xProgressBar->setPosSize( nDx+nX_ProgressBar, nDy+nY_ProgressBar, nWidth_ProgressBar, nHeight_ProgressBar, 15 );

    m_a3DLine.X      = nDx+nX_Topic_Top;
    m_a3DLine.Y      = nDy+nY_Topic_Bottom+nHeight_Topic_Bottom+(PROGRESSMONITOR_FREEBORDER/2);
    m_a3DLine.Width  = nWidth_ProgressBar;
    m_a3DLine.Height = nHeight_ProgressBar;

    // All childcontrols make an implicit repaint in setPosSize()!
    // Make it also for this 3D-line ...
    css::uno::Reference< XGraphics >  xGraphics = impl_getGraphicsPeer ();

    xGraphics->setLineColor ( PROGRESSMONITOR_LINECOLOR_SHADOW  );
    xGraphics->drawLine     ( m_a3DLine.X, m_a3DLine.Y, m_a3DLine.X+m_a3DLine.Width, m_a3DLine.Y );

    xGraphics->setLineColor ( PROGRESSMONITOR_LINECOLOR_BRIGHT  );
    xGraphics->drawLine     ( m_a3DLine.X, m_a3DLine.Y+1, m_a3DLine.X+m_a3DLine.Width, m_a3DLine.Y+1 );
}

//  private method
void ProgressMonitor::impl_rebuildFixedText ()
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex );

    // Rebuild fixedtext before progress

    // Rebuild left site of text
    if (m_xTopic_Top.is())
    {
        OUString aCollectString;

        // Collect all topics from list and format text.
        // "\n" MUST BE at the end of line!!! => Else ... topic and his text are not in the same line!!!
        for (IMPL_TextlistItem* pSearchItem : maTextlist_Top)
        {
            aCollectString  +=  pSearchItem->sTopic;
            aCollectString  +=  "\n";
        }

        m_xTopic_Top->setText ( aCollectString );
    }

    // Rebuild right site of text
    if (m_xText_Top.is())
    {
        OUString        aCollectString;

        // Collect all topics from list and format text.
        // "\n" MUST BE at the end of line!!! => Else ... topic and his text are not in the same line!!!
        for (IMPL_TextlistItem* pSearchItem : maTextlist_Top)
        {
            aCollectString  +=  pSearchItem->sText;
            aCollectString  +=  "\n";
        }

        m_xText_Top->setText ( aCollectString );
    }

    // Rebuild fixedtext below progress

    // Rebuild left site of text
    if (m_xTopic_Bottom.is())
    {
        OUString        aCollectString;

        // Collect all topics from list and format text.
        // "\n" MUST BE at the end of line!!! => Else ... topic and his text are not in the same line!!!
        for (IMPL_TextlistItem* pSearchItem : maTextlist_Bottom)
        {
            aCollectString  +=  pSearchItem->sTopic;
            aCollectString  +=  "\n";
        }

        m_xTopic_Bottom->setText ( aCollectString );
    }

    // Rebuild right site of text
    if (m_xText_Bottom.is())
    {
        OUString        aCollectString;

        // Collect all topics from list and format text.
        // "\n" MUST BE at the end of line!!! => Else ... topic and his text are not in the same line!!!
        for (IMPL_TextlistItem* pSearchItem : maTextlist_Bottom)
        {
            aCollectString  +=  pSearchItem->sText;
            aCollectString  +=  "\n";
        }

        m_xText_Bottom->setText ( aCollectString );
    }
}

//  private method
void ProgressMonitor::impl_cleanMemory ()
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex );

    // Delete all of lists.

    for (IMPL_TextlistItem* pSearchItem : maTextlist_Top)
    {
        delete pSearchItem;
    }
    maTextlist_Top.clear();

    for (IMPL_TextlistItem* pSearchItem : maTextlist_Bottom)
    {
        delete pSearchItem;
    }
    maTextlist_Bottom.clear();
}

//  private method
IMPL_TextlistItem* ProgressMonitor::impl_searchTopic ( const OUString& rTopic, bool bbeforeProgress )
{
    // Get right textlist for following operations.
    ::std::vector< IMPL_TextlistItem* >* pTextList;

    // Ready for multithreading
    ClearableMutexGuard aGuard ( m_aMutex );

    if ( bbeforeProgress )
    {
        pTextList = &maTextlist_Top;
    }
    else
    {
        pTextList = &maTextlist_Bottom;
    }

    // Switch off guard.
    aGuard.clear ();

    // Search the topic in textlist.
    size_t nPosition    = 0;
    size_t nCount       = pTextList->size();

    for ( nPosition = 0; nPosition < nCount; ++nPosition )
    {
        IMPL_TextlistItem* pSearchItem = pTextList->at( nPosition );

        if ( pSearchItem->sTopic == rTopic )
        {
            // We have found this topic... return a valid pointer.
            return pSearchItem;
        }
    }

    // We haven't found this topic... return a nonvalid pointer.
    return nullptr;
}

//  debug methods

// addText, updateText
bool ProgressMonitor::impl_debug_checkParameter (
    const OUString& rTopic,
    const OUString& rText,
    bool /*bbeforeProgress*/
) {
    if ( rTopic.isEmpty()       ) return false;    // ""

    if ( rText.isEmpty()       ) return false;    // ""

    // "bbeforeProgress" is valid in everyway!

    // Parameter OK ... return true.
    return true;
}

// removeText
bool ProgressMonitor::impl_debug_checkParameter ( const OUString& rTopic, bool /*bbeforeProgress*/ )
{
    if ( rTopic.isEmpty()      ) return false;    // ""

    // "bbeforeProgress" is valid in everyway!

    // Parameter OK ... return true.
    return true;
}

}   // namespace unocontrols

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
