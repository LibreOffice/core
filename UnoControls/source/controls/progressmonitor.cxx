/*************************************************************************
 *
 *  $RCSfile: progressmonitor.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:11:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//____________________________________________________________________________________________________________
//  my own includes
//____________________________________________________________________________________________________________

#ifndef _UNOCONTROLS_PROGRESSMONITOR_CTRL_HXX
#include "progressmonitor.hxx"
#endif

//____________________________________________________________________________________________________________
//  includes of other projects
//____________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_AWT_GRADIENTSTYLE_HPP_
#include <com/sun/star/awt/GradientStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_RASTEROPERATION_HPP_
#include <com/sun/star/awt/RasterOperation.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XGRAPHICS_HPP_
#include <com/sun/star/awt/XGraphics.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

//____________________________________________________________________________________________________________
//  includes of my project
//____________________________________________________________________________________________________________

#ifndef _UNOCONTROLS_PROGRESSBAR_CTRL_HXX
#include "progressbar.hxx"
#endif

//____________________________________________________________________________________________________________
//  namespace
//____________________________________________________________________________________________________________

using namespace ::cppu                  ;
using namespace ::osl                   ;
using namespace ::rtl                   ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::lang  ;
using namespace ::com::sun::star::awt   ;

namespace unocontrols{

//____________________________________________________________________________________________________________
//  construct/destruct
//____________________________________________________________________________________________________________

ProgressMonitor::ProgressMonitor( const Reference< XMultiServiceFactory >& xFactory )
    : BaseContainerControl  ( xFactory  )
{
    // Its not allowed to work with member in this method (refcounter !!!)
    // But with a HACK (++refcount) its "OK" :-(
    ++m_refCount ;

    // Create instances for fixedtext, button and progress ...
    m_xTopic_Top    = Reference< XFixedText >       ( xFactory->createInstance ( OUString::createFromAscii( FIXEDTEXT_SERVICENAME   ) ), UNO_QUERY ) ;
    m_xText_Top     = Reference< XFixedText >       ( xFactory->createInstance ( OUString::createFromAscii( FIXEDTEXT_SERVICENAME   ) ), UNO_QUERY ) ;
    m_xTopic_Bottom = Reference< XFixedText >       ( xFactory->createInstance ( OUString::createFromAscii( FIXEDTEXT_SERVICENAME   ) ), UNO_QUERY ) ;
    m_xText_Bottom  = Reference< XFixedText >       ( xFactory->createInstance ( OUString::createFromAscii( FIXEDTEXT_SERVICENAME   ) ), UNO_QUERY ) ;
    m_xButton       = Reference< XButton >          ( xFactory->createInstance ( OUString::createFromAscii( BUTTON_SERVICENAME      ) ), UNO_QUERY ) ;
    m_xProgressBar  = Reference< XProgressBar >     ( xFactory->createInstance ( OUString::createFromAscii( SERVICENAME_PROGRESSBAR ) ), UNO_QUERY ) ;

    // ... cast controls to Reference< XControl >  (for "setModel"!) ...
    Reference< XControl >   xRef_Topic_Top      ( m_xTopic_Top    , UNO_QUERY ) ;
    Reference< XControl >   xRef_Text_Top       ( m_xText_Top     , UNO_QUERY ) ;
    Reference< XControl >   xRef_Topic_Bottom   ( m_xTopic_Bottom , UNO_QUERY ) ;
    Reference< XControl >   xRef_Text_Bottom    ( m_xText_Bottom  , UNO_QUERY ) ;
    Reference< XControl >   xRef_Button         ( m_xButton       , UNO_QUERY ) ;
    Reference< XControl >   xRef_ProgressBar    ( m_xProgressBar  , UNO_QUERY ) ;

    // ... set models ...
    xRef_Topic_Top->setModel        ( Reference< XControlModel >  ( xFactory->createInstance ( OUString::createFromAscii( FIXEDTEXT_MODELNAME   ) ), UNO_QUERY ) ) ;
    xRef_Text_Top->setModel         ( Reference< XControlModel >  ( xFactory->createInstance ( OUString::createFromAscii( FIXEDTEXT_MODELNAME   ) ), UNO_QUERY ) ) ;
    xRef_Topic_Bottom->setModel     ( Reference< XControlModel >  ( xFactory->createInstance ( OUString::createFromAscii( FIXEDTEXT_MODELNAME   ) ), UNO_QUERY ) ) ;
    xRef_Text_Bottom->setModel      ( Reference< XControlModel >  ( xFactory->createInstance ( OUString::createFromAscii( FIXEDTEXT_MODELNAME   ) ), UNO_QUERY ) ) ;
    xRef_Button->setModel           ( Reference< XControlModel >  ( xFactory->createInstance ( OUString::createFromAscii( BUTTON_MODELNAME      ) ), UNO_QUERY ) ) ;
    // ProgressBar has no model !!!

    // ... and add controls to basecontainercontrol!
    addControl ( OUString::createFromAscii( CONTROLNAME_TEXT        ) , xRef_Topic_Top      ) ;
    addControl ( OUString::createFromAscii( CONTROLNAME_TEXT        ) , xRef_Text_Top       ) ;
    addControl ( OUString::createFromAscii( CONTROLNAME_TEXT        ) , xRef_Topic_Bottom   ) ;
    addControl ( OUString::createFromAscii( CONTROLNAME_TEXT        ) , xRef_Text_Bottom    ) ;
    addControl ( OUString::createFromAscii( CONTROLNAME_BUTTON      ) , xRef_Button         ) ;
    addControl ( OUString::createFromAscii( CONTROLNAME_PROGRESSBAR ) , xRef_ProgressBar    ) ;

    // FixedText make it automaticly visible by himself ... but not the progressbar !!!
    // it must be set explicitly
    Reference< XWindow > xWindowRef_ProgressBar( m_xProgressBar, UNO_QUERY );
    xWindowRef_ProgressBar->setVisible( sal_True );

    // Reset to defaults !!!
    // (progressbar take automaticly its own defaults)
    m_xButton->setLabel         ( OUString::createFromAscii( DEFAULT_BUTTONLABEL    ) ) ;
    m_xTopic_Top->setText       ( OUString::createFromAscii( DEFAULT_TOPIC          ) ) ;
    m_xText_Top->setText        ( OUString::createFromAscii( DEFAULT_TEXT           ) ) ;
    m_xTopic_Bottom->setText    ( OUString::createFromAscii( DEFAULT_TOPIC          ) ) ;
    m_xText_Bottom->setText     ( OUString::createFromAscii( DEFAULT_TEXT           ) ) ;

    --m_refCount ;

    // Initialize info lists for fixedtext's
    m_pTextlist_Top     = new IMPL_Textlist ;
    m_pTextlist_Bottom  = new IMPL_Textlist ;
}

ProgressMonitor::~ProgressMonitor()
{
    impl_cleanMemory () ;
}

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

Any SAL_CALL ProgressMonitor::queryInterface( const Type& rType ) throw( RuntimeException )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.
    Any aReturn ;
    Reference< XInterface > xDelegator = BaseContainerControl::impl_getDelegator();
    if ( xDelegator.is() == sal_True )
    {
        // If an delegator exist, forward question to his queryInterface.
        // Delegator will ask his own queryAggregation!
        aReturn = xDelegator->queryInterface( rType );
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

void SAL_CALL ProgressMonitor::acquire() throw( RuntimeException )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    BaseControl::acquire();
}

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

void SAL_CALL ProgressMonitor::release() throw( RuntimeException )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    BaseControl::release();
}

//____________________________________________________________________________________________________________
//  XTypeProvider
//____________________________________________________________________________________________________________

Sequence< Type > SAL_CALL ProgressMonitor::getTypes() throw( RuntimeException )
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
                                                          ::getCppuType(( const Reference< XButton          >*)NULL )   ,
                                                          ::getCppuType(( const Reference< XProgressMonitor >*)NULL )   ,
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

Any SAL_CALL ProgressMonitor::queryAggregation( const Type& aType ) throw( RuntimeException )
{
    // Ask for my own supported interfaces ...
    // Attention: XTypeProvider and XInterface are supported by OComponentHelper!
    Any aReturn ( ::cppu::queryInterface(   aType                                       ,
                                               static_cast< XLayoutConstrains*  > ( this )  ,
                                               static_cast< XButton*            > ( this )  ,
                                               static_cast< XProgressMonitor*   > ( this )
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
//  XProgressMonitor
//____________________________________________________________________________________________________________

void SAL_CALL ProgressMonitor::addText( const OUString& rTopic, const OUString& rText, sal_Bool bbeforeProgress ) throw( RuntimeException )
{
    // Safe impossible cases
    // Check valid call of this method.
    DBG_ASSERT ( impl_debug_checkParameter ( rTopic, rText, bbeforeProgress )   , "ProgressMonitor::addText()\nCall without valid parameters!\n") ;
    DBG_ASSERT ( !(impl_searchTopic ( rTopic, bbeforeProgress ) != NULL )       , "ProgresMonitor::addText()\nThe text already exist.\n"        ) ;

    // Do nothing (in Release), if topic already exist.
    if ( impl_searchTopic ( rTopic, bbeforeProgress ) != NULL )
    {
        return ;
    }

    // Else ... take memory for new item ...
    IMPL_TextlistItem*  pTextItem = new IMPL_TextlistItem ;

    if ( pTextItem != NULL )
    {
        // Set values ...
        pTextItem->sTopic   = rTopic ;
        pTextItem->sText    = rText ;

        // Ready for multithreading
        MutexGuard aGuard ( m_aMutex ) ;

        // ... and insert it in right list.
        if ( bbeforeProgress == sal_True )
        {
            m_pTextlist_Top->Insert    ( pTextItem, LIST_APPEND ) ;
        }
        else
        {
            m_pTextlist_Bottom->Insert ( pTextItem, LIST_APPEND ) ;
        }
    }

    // ... update window
    impl_rebuildFixedText   () ;
    impl_recalcLayout       () ;
}

//____________________________________________________________________________________________________________
//  XProgressMonitor
//____________________________________________________________________________________________________________

void SAL_CALL ProgressMonitor::removeText ( const OUString& rTopic, sal_Bool bbeforeProgress ) throw( RuntimeException )
{
    // Safe impossible cases
    // Check valid call of this method.
    DBG_ASSERT ( impl_debug_checkParameter ( rTopic, bbeforeProgress ), "ProgressMonitor::removeText()\nCall without valid parameters!\n" ) ;

    // Search the topic ...
    IMPL_TextlistItem* pSearchItem = impl_searchTopic ( rTopic, bbeforeProgress ) ;

    if ( pSearchItem != NULL )
    {
        // Ready for multithreading
        MutexGuard aGuard ( m_aMutex ) ;

        // ... delete item from right list ...
        if ( bbeforeProgress == sal_True )
        {
            m_pTextlist_Top->Remove    ( pSearchItem ) ;
        }
        else
        {
            m_pTextlist_Bottom->Remove ( pSearchItem ) ;
        }

        delete pSearchItem ;

        // ... and update window.
        impl_rebuildFixedText   () ;
        impl_recalcLayout       () ;
    }
}

//____________________________________________________________________________________________________________
//  XProgressMonitor
//____________________________________________________________________________________________________________

void SAL_CALL ProgressMonitor::updateText ( const OUString& rTopic, const OUString& rText, sal_Bool bbeforeProgress ) throw( RuntimeException )
{
    // Safe impossible cases
    // Check valid call of this method.
    DBG_ASSERT ( impl_debug_checkParameter ( rTopic, rText, bbeforeProgress ), "ProgressMonitor::updateText()\nCall without valid parameters!\n" ) ;

    // Search topic ...
    IMPL_TextlistItem* pSearchItem = impl_searchTopic ( rTopic, bbeforeProgress ) ;

    if ( pSearchItem != NULL )
    {
        // Ready for multithreading
        MutexGuard aGuard ( m_aMutex ) ;

        // ... update text ...
        pSearchItem->sText = rText ;

        // ... and update window.
        impl_rebuildFixedText   () ;
        impl_recalcLayout       () ;
    }
}

//____________________________________________________________________________________________________________
//  XProgressBar
//____________________________________________________________________________________________________________

void SAL_CALL ProgressMonitor::setForegroundColor ( sal_Int32 nColor ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex ) ;

    if ( m_xProgressBar.is () )
    {
        m_xProgressBar->setForegroundColor ( nColor ) ;
    }
}

//____________________________________________________________________________________________________________
//  XProgressBar
//____________________________________________________________________________________________________________

void SAL_CALL ProgressMonitor::setBackgroundColor ( sal_Int32 nColor ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex ) ;

    if ( m_xProgressBar.is () )
    {
        m_xProgressBar->setBackgroundColor ( nColor ) ;
    }
}

//____________________________________________________________________________________________________________
//  XProgressBar
//____________________________________________________________________________________________________________

void SAL_CALL ProgressMonitor::setValue ( sal_Int32 nValue ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex ) ;

    if ( m_xProgressBar.is () )
    {
        m_xProgressBar->setValue ( nValue ) ;
    }
}

//____________________________________________________________________________________________________________
//  XProgressBar
//____________________________________________________________________________________________________________

void SAL_CALL ProgressMonitor::setRange ( sal_Int32 nMin, sal_Int32 nMax ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex ) ;

    if ( m_xProgressBar.is () )
    {
        m_xProgressBar->setRange ( nMin, nMax ) ;
    }
}

//____________________________________________________________________________________________________________
//  XProgressBar
//____________________________________________________________________________________________________________

sal_Int32 SAL_CALL ProgressMonitor::getValue () throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex ) ;

    if (m_xProgressBar.is())
    {
        return m_xProgressBar->getValue () ;
    }

    return 0 ;
}

//____________________________________________________________________________________________________________
//  XButton
//____________________________________________________________________________________________________________

void SAL_CALL ProgressMonitor::addActionListener ( const Reference< XActionListener > & rListener ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex ) ;

    if ( m_xButton.is () )
    {
        m_xButton->addActionListener ( rListener ) ;
    }
}

//____________________________________________________________________________________________________________
//  XButton
//____________________________________________________________________________________________________________

void SAL_CALL ProgressMonitor::removeActionListener ( const Reference< XActionListener > & rListener ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex ) ;

    if ( m_xButton.is () )
    {
        m_xButton->removeActionListener ( rListener ) ;
    }
}

//____________________________________________________________________________________________________________
//  XButton
//____________________________________________________________________________________________________________

void SAL_CALL ProgressMonitor::setLabel ( const OUString& rLabel ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex ) ;

    if ( m_xButton.is () )
    {
        m_xButton->setLabel ( rLabel ) ;
    }
}

//____________________________________________________________________________________________________________
//  XButton
//____________________________________________________________________________________________________________

void SAL_CALL ProgressMonitor::setActionCommand ( const OUString& rCommand ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex ) ;

    if ( m_xButton.is () )
    {
        m_xButton->setActionCommand ( rCommand ) ;
    }
}

//____________________________________________________________________________________________________________
//  XLayoutConstrains
//____________________________________________________________________________________________________________

Size SAL_CALL ProgressMonitor::getMinimumSize () throw( RuntimeException )
{
    return Size (DEFAULT_WIDTH, DEFAULT_HEIGHT) ;
}

//____________________________________________________________________________________________________________
//  XLayoutConstrains
//____________________________________________________________________________________________________________

Size SAL_CALL ProgressMonitor::getPreferredSize () throw( RuntimeException )
{
    // Ready for multithreading
    ClearableMutexGuard aGuard ( m_aMutex ) ;

    // get information about required place of child controls
    Reference< XLayoutConstrains >  xTopicLayout_Top        ( m_xTopic_Top      , UNO_QUERY ) ;
    Reference< XLayoutConstrains >  xTopicLayout_Bottom     ( m_xTopic_Bottom   , UNO_QUERY ) ;
    Reference< XLayoutConstrains >  xButtonLayout           ( m_xButton         , UNO_QUERY ) ;
    Reference< XWindow >            xProgressBarWindow      ( m_xProgressBar    , UNO_QUERY ) ;

    Size        aTopicSize_Top      =   xTopicLayout_Top->getPreferredSize          ();
    Size        aTopicSize_Bottom   =   xTopicLayout_Bottom->getPreferredSize       ();
    Size        aButtonSize         =   xButtonLayout->getPreferredSize             ();
    Rectangle   aTempRectangle      =   xProgressBarWindow->getPosSize              ();
    Size        aProgressBarSize    =   Size( aTempRectangle.Width, aTempRectangle.Height );

    aGuard.clear () ;

    // calc preferred size of progressmonitor
    sal_Int32   nWidth  =   0 ;
    sal_Int32   nHeight =   0 ;

    nWidth   =  3 * FREEBORDER          ;
    nWidth  +=  aProgressBarSize.Width  ;

    nHeight  =  6 * FREEBORDER          ;
    nHeight +=  aTopicSize_Top.Height   ;
    nHeight +=  aProgressBarSize.Height ;
    nHeight +=  aTopicSize_Bottom.Height;
    nHeight +=  2                       ;   // 1 for black line, 1 for white line = 3D-Line!
    nHeight +=  aButtonSize.Height      ;

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

Size SAL_CALL ProgressMonitor::calcAdjustedSize ( const Size& rNewSize ) throw( RuntimeException )
{
    return getPreferredSize () ;
}

//____________________________________________________________________________________________________________
//  XControl
//____________________________________________________________________________________________________________

void SAL_CALL ProgressMonitor::createPeer ( const Reference< XToolkit > & rToolkit, const Reference< XWindowPeer > & rParent    ) throw( RuntimeException )
{
    if (!getPeer().is())
    {
        BaseContainerControl::createPeer ( rToolkit, rParent ) ;

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

sal_Bool SAL_CALL ProgressMonitor::setModel ( const Reference< XControlModel > & rModel ) throw( RuntimeException )
{
    // We have no model.
    return sal_False ;
}

//____________________________________________________________________________________________________________
//  XControl
//____________________________________________________________________________________________________________

Reference< XControlModel > SAL_CALL ProgressMonitor::getModel () throw( RuntimeException )
{
    // We have no model.
    // return (XControlModel*)this ;
    return Reference< XControlModel >  () ;
}

//____________________________________________________________________________________________________________
//  XComponent
//____________________________________________________________________________________________________________

void SAL_CALL ProgressMonitor::dispose () throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex ) ;

    // "removeControl()" control the state of a reference
    Reference< XControl >  xRef_Topic_Top       ( m_xTopic_Top      , UNO_QUERY ) ;
    Reference< XControl >  xRef_Text_Top        ( m_xText_Top       , UNO_QUERY ) ;
    Reference< XControl >  xRef_Topic_Bottom    ( m_xTopic_Bottom   , UNO_QUERY ) ;
    Reference< XControl >  xRef_Text_Bottom     ( m_xText_Bottom    , UNO_QUERY ) ;
    Reference< XControl >  xRef_Button          ( m_xButton         , UNO_QUERY ) ;
    Reference< XControl >  xRef_ProgressBar     ( m_xProgressBar    , UNO_QUERY ) ;

    removeControl ( xRef_Topic_Top      ) ;
    removeControl ( xRef_Text_Top       ) ;
    removeControl ( xRef_Topic_Bottom   ) ;
    removeControl ( xRef_Text_Bottom    ) ;
    removeControl ( xRef_Button         ) ;
    removeControl ( xRef_ProgressBar    ) ;

    // do'nt use "...->clear ()" or "... = XFixedText ()"
    // when other hold a reference at this object !!!
    xRef_Topic_Top->dispose     () ;
    xRef_Text_Top->dispose      () ;
    xRef_Topic_Bottom->dispose  () ;
    xRef_Text_Bottom->dispose   () ;
    xRef_Button->dispose        () ;
    xRef_ProgressBar->dispose   () ;

    BaseContainerControl::dispose () ;
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void SAL_CALL ProgressMonitor::setPosSize ( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int16 nFlags ) throw( RuntimeException )
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
        impl_recalcLayout () ;
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

const Sequence< OUString > ProgressMonitor::impl_getStaticSupportedServiceNames()
{
    MutexGuard aGuard( Mutex::getGlobalMutex() );
    Sequence< OUString > seqServiceNames( 1 );
    seqServiceNames.getArray() [0] = OUString::createFromAscii( SERVICENAME_PROGRESSMONITOR );
    return seqServiceNames ;
}

//____________________________________________________________________________________________________________
//  impl but public method to register service
//____________________________________________________________________________________________________________

const OUString ProgressMonitor::impl_getStaticImplementationName()
{
    return OUString::createFromAscii( IMPLEMENTATIONNAME_PROGRESSMONITOR );
}

//____________________________________________________________________________________________________________
//  protected method
//____________________________________________________________________________________________________________

void ProgressMonitor::impl_paint ( sal_Int32 nX, sal_Int32 nY, const Reference< XGraphics > & rGraphics )
{
    if (rGraphics.is())
    {
        // Ready for multithreading
        MutexGuard aGuard ( m_aMutex ) ;

        // paint shadowed border around the progressmonitor
        rGraphics->setLineColor ( LINECOLOR_SHADOW                                                              ) ;
        rGraphics->drawLine     ( impl_getWidth()-1, impl_getHeight()-1, impl_getWidth()-1, nY                  ) ;
        rGraphics->drawLine     ( impl_getWidth()-1, impl_getHeight()-1, nX               , impl_getHeight()-1  ) ;

        rGraphics->setLineColor ( LINECOLOR_BRIGHT                          ) ;
        rGraphics->drawLine     ( nX, nY, impl_getWidth(), nY               ) ;
        rGraphics->drawLine     ( nX, nY, nX             , impl_getHeight() ) ;

        // Paint 3D-line
        rGraphics->setLineColor ( LINECOLOR_SHADOW  ) ;
        rGraphics->drawLine     ( m_a3DLine.X, m_a3DLine.Y, m_a3DLine.X+m_a3DLine.Width, m_a3DLine.Y ) ;

        rGraphics->setLineColor ( LINECOLOR_BRIGHT  ) ;
        rGraphics->drawLine     ( m_a3DLine.X, m_a3DLine.Y+1, m_a3DLine.X+m_a3DLine.Width, m_a3DLine.Y+1 ) ;
    }
}

//____________________________________________________________________________________________________________
//  private method
//____________________________________________________________________________________________________________

void ProgressMonitor::impl_recalcLayout ()
{
    sal_Int32   nX_Button               ;
    sal_Int32   nY_Button               ;
    sal_Int32   nWidth_Button           ;
    sal_Int32   nHeight_Button          ;

    sal_Int32   nX_ProgressBar          ;
    sal_Int32   nY_ProgressBar          ;
    sal_Int32   nWidth_ProgressBar      ;
    sal_Int32   nHeight_ProgressBar     ;

    sal_Int32   nX_3DLine               ;
    sal_Int32   nY_3DLine               ;
    sal_Int32   nWidth_3DLine           ;
    sal_Int32   nHeight_3DLine          ;

    sal_Int32   nX_Text_Top             ;
    sal_Int32   nY_Text_Top             ;
    sal_Int32   nWidth_Text_Top         ;
    sal_Int32   nHeight_Text_Top        ;

    sal_Int32   nX_Topic_Top            ;
    sal_Int32   nY_Topic_Top            ;
    sal_Int32   nWidth_Topic_Top        ;
    sal_Int32   nHeight_Topic_Top       ;

    sal_Int32   nX_Text_Bottom          ;
    sal_Int32   nY_Text_Bottom          ;
    sal_Int32   nWidth_Text_Bottom      ;
    sal_Int32   nHeight_Text_Bottom     ;

    sal_Int32   nX_Topic_Bottom         ;
    sal_Int32   nY_Topic_Bottom         ;
    sal_Int32   nWidth_Topic_Bottom     ;
    sal_Int32   nHeight_Topic_Bottom    ;

    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex ) ;

    // get information about required place of child controls
    Reference< XLayoutConstrains >  xTopicLayout_Top    ( m_xTopic_Top      , UNO_QUERY ) ;
    Reference< XLayoutConstrains >  xTextLayout_Top     ( m_xText_Top       , UNO_QUERY ) ;
    Reference< XLayoutConstrains >  xTopicLayout_Bottom ( m_xTopic_Bottom   , UNO_QUERY ) ;
    Reference< XLayoutConstrains >  xTextLayout_Bottom  ( m_xText_Bottom    , UNO_QUERY ) ;
    Reference< XLayoutConstrains >  xButtonLayout       ( m_xButton         , UNO_QUERY ) ;

    Size    aTopicSize_Top      =   xTopicLayout_Top->getPreferredSize      () ;
    Size    aTextSize_Top       =   xTextLayout_Top->getPreferredSize       () ;
    Size    aTopicSize_Bottom   =   xTopicLayout_Bottom->getPreferredSize   () ;
    Size    aTextSize_Bottom    =   xTextLayout_Bottom->getPreferredSize    () ;
    Size    aButtonSize         =   xButtonLayout->getPreferredSize         () ;

    // calc position and size of child controls
    // Button has preferred size!
    nWidth_Button           =   aButtonSize.Width                                               ;
    nHeight_Button          =   aButtonSize.Height                                              ;

    // Left column before progressbar has preferred size and fixed position.
    // But "Width" is oriented on left column below progressbar to!!! "max(...)"
    nX_Topic_Top            =   FREEBORDER                                                      ;
    nY_Topic_Top            =   FREEBORDER                                                      ;
    nWidth_Topic_Top        =   Max ( aTopicSize_Top.Width, aTopicSize_Bottom.Width )           ;
    nHeight_Topic_Top       =   aTopicSize_Top.Height                                           ;

    // Right column before progressbar has relativ position to left column ...
    // ... and a size as rest of dialog size!
    nX_Text_Top             =   nX_Topic_Top+nWidth_Topic_Top+FREEBORDER                        ;
    nY_Text_Top             =   nY_Topic_Top                                                    ;
    nWidth_Text_Top         =   Max ( aTextSize_Top.Width, aTextSize_Bottom.Width )             ;
    // Fix size of this column to minimum!
    sal_Int32 nSummaryWidth = nWidth_Text_Top+nWidth_Topic_Top+(3*FREEBORDER) ;
    if ( nSummaryWidth < DEFAULT_WIDTH )
        nWidth_Text_Top     =   DEFAULT_WIDTH-nWidth_Topic_Top-(3*FREEBORDER);
    // Fix size of column to maximum!
    if ( nSummaryWidth > impl_getWidth() )
        nWidth_Text_Top     =   impl_getWidth()-nWidth_Topic_Top-(3*FREEBORDER)                 ;
    nHeight_Text_Top        =   nHeight_Topic_Top                                               ;

    // Position of progressbar is relativ to columns before.
    // Progressbar.Width  = Dialog.Width !!!
    // Progressbar.Height = Button.Height
    nX_ProgressBar          =   nX_Topic_Top                                                    ;
    nY_ProgressBar          =   nY_Topic_Top+nHeight_Topic_Top+FREEBORDER                       ;
    nWidth_ProgressBar      =   FREEBORDER+nWidth_Topic_Top+nWidth_Text_Top                     ;
    nHeight_ProgressBar     =   nHeight_Button                                                  ;

    // Oriented by left column before progressbar.
    nX_Topic_Bottom         =   nX_Topic_Top                                                    ;
    nY_Topic_Bottom         =   nY_ProgressBar+nHeight_ProgressBar+FREEBORDER                   ;
    nWidth_Topic_Bottom     =   nWidth_Topic_Top                                                ;
    nHeight_Topic_Bottom    =   aTopicSize_Bottom.Height                                        ;

    // Oriented by right column before progressbar.
    nX_Text_Bottom          =   nX_Topic_Bottom+nWidth_Topic_Bottom+FREEBORDER                  ;
    nY_Text_Bottom          =   nY_Topic_Bottom                                                 ;
    nWidth_Text_Bottom      =   nWidth_Text_Top                                                 ;
    nHeight_Text_Bottom     =   nHeight_Topic_Bottom                                            ;

    // Oriented by progressbar.
    nX_3DLine               =   nX_Topic_Top                                                    ;
    nY_3DLine               =   nY_Topic_Bottom+nHeight_Topic_Bottom+(FREEBORDER/2)             ;
    nWidth_3DLine           =   nWidth_ProgressBar                                              ;
    nHeight_3DLine          =   1                                                               ;   // Height for ONE line ! (But we paint two lines!)

    // Oriented by progressbar.
    nX_Button               =   nX_ProgressBar+nWidth_ProgressBar-nWidth_Button                 ;
    nY_Button               =   nY_Topic_Bottom+nHeight_Topic_Bottom+FREEBORDER                 ;

    // Calc offsets to center controls
    sal_Int32   nDx ;
    sal_Int32   nDy ;

    nDx =   ( (2*FREEBORDER)+nWidth_ProgressBar                                                             ) ;
    nDy =   ( (6*FREEBORDER)+nHeight_Topic_Top+nHeight_ProgressBar+nHeight_Topic_Bottom+2+nHeight_Button    ) ;

    // At this point use original dialog size to center controls!
    nDx =   (impl_getWidth ()/2)-(nDx/2)    ;
    nDy =   (impl_getHeight()/2)-(nDy/2)    ;

    if ( nDx<0 )
    {
        nDx=0 ;
    }
    if ( nDy<0 )
    {
        nDy=0 ;
    }

    // Set new position and size on all controls
    Reference< XWindow >  xRef_Topic_Top        ( m_xTopic_Top      , UNO_QUERY ) ;
    Reference< XWindow >  xRef_Text_Top         ( m_xText_Top       , UNO_QUERY ) ;
    Reference< XWindow >  xRef_Topic_Bottom     ( m_xTopic_Bottom   , UNO_QUERY ) ;
    Reference< XWindow >  xRef_Text_Bottom      ( m_xText_Bottom    , UNO_QUERY ) ;
    Reference< XWindow >  xRef_Button           ( m_xButton         , UNO_QUERY ) ;
    Reference< XWindow >  xRef_ProgressBar      ( m_xProgressBar    , UNO_QUERY ) ;

    xRef_Topic_Top->setPosSize      ( nDx+nX_Topic_Top      , nDy+nY_Topic_Top      , nWidth_Topic_Top      , nHeight_Topic_Top     , 15 ) ;
    xRef_Text_Top->setPosSize       ( nDx+nX_Text_Top       , nDy+nY_Text_Top       , nWidth_Text_Top       , nHeight_Text_Top      , 15 ) ;
    xRef_Topic_Bottom->setPosSize   ( nDx+nX_Topic_Bottom   , nDy+nY_Topic_Bottom   , nWidth_Topic_Bottom   , nHeight_Topic_Bottom  , 15 ) ;
    xRef_Text_Bottom->setPosSize    ( nDx+nX_Text_Bottom    , nDy+nY_Text_Bottom    , nWidth_Text_Bottom    , nHeight_Text_Bottom   , 15 ) ;
    xRef_Button->setPosSize         ( nDx+nX_Button         , nDy+nY_Button         , nWidth_Button         , nHeight_Button        , 15 ) ;
    xRef_ProgressBar->setPosSize    ( nDx+nX_ProgressBar    , nDy+nY_ProgressBar    , nWidth_ProgressBar    , nHeight_ProgressBar   , 15 ) ;

    m_a3DLine.X         = nDx+nX_Topic_Top                                          ;
    m_a3DLine.Y         = nDy+nY_Topic_Bottom+nHeight_Topic_Bottom+(FREEBORDER/2)   ;
    m_a3DLine.Width     = nWidth_ProgressBar                                        ;
    m_a3DLine.Height    = nHeight_ProgressBar                                       ;

    // All childcontrols make an implicit repaint in setPosSize()!
    // Make it also for this 3D-line ...
    Reference< XGraphics >  xGraphics = impl_getGraphicsPeer () ;

    xGraphics->setLineColor ( LINECOLOR_SHADOW  ) ;
    xGraphics->drawLine     ( m_a3DLine.X, m_a3DLine.Y, m_a3DLine.X+m_a3DLine.Width, m_a3DLine.Y ) ;

    xGraphics->setLineColor ( LINECOLOR_BRIGHT  ) ;
    xGraphics->drawLine     ( m_a3DLine.X, m_a3DLine.Y+1, m_a3DLine.X+m_a3DLine.Width, m_a3DLine.Y+1 ) ;
}

//____________________________________________________________________________________________________________
//  private method
//____________________________________________________________________________________________________________

void ProgressMonitor::impl_rebuildFixedText ()
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex ) ;

    // Rebuild fixedtext before progress

    // Rebuild left site of text
    if (m_xTopic_Top.is())
    {
        OUString        aCollectString  ;

        // Collect all topics from list and format text.
        // "\n" MUST BE at the end of line!!! => Else ... topic and his text are not in the same line!!!
        for ( sal_uInt32 n=0; n<m_pTextlist_Top->Count(); ++n )
        {
            IMPL_TextlistItem* pSearchItem = m_pTextlist_Top->GetObject (n) ;
            aCollectString  +=  pSearchItem->sTopic ;
            aCollectString  +=  OUString::createFromAscii("\n")             ;
        }
        aCollectString  +=  OUString::createFromAscii("\0") ;   // It's better :-)

        m_xTopic_Top->setText ( aCollectString ) ;
    }

    // Rebuild right site of text
    if (m_xText_Top.is())
    {
        OUString        aCollectString  ;

        // Collect all topics from list and format text.
        // "\n" MUST BE at the end of line!!! => Else ... topic and his text are not in the same line!!!
        for ( sal_uInt32 n=0; n<m_pTextlist_Top->Count(); ++n )
        {
            IMPL_TextlistItem* pSearchItem = m_pTextlist_Top->GetObject (n) ;
            aCollectString  +=  pSearchItem->sText ;
            aCollectString  +=  OUString::createFromAscii("\n")            ;
        }
        aCollectString  +=  OUString::createFromAscii("\0") ;   // It's better :-)

        m_xText_Top->setText ( aCollectString ) ;
    }

    // Rebuild fixedtext below progress

    // Rebuild left site of text
    if (m_xTopic_Bottom.is())
    {
        OUString        aCollectString  ;

        // Collect all topics from list and format text.
        // "\n" MUST BE at the end of line!!! => Else ... topic and his text are not in the same line!!!
        for ( sal_uInt32 n=0; n<m_pTextlist_Bottom->Count(); ++n )
        {
            IMPL_TextlistItem* pSearchItem = m_pTextlist_Bottom->GetObject (n) ;
            aCollectString  +=  pSearchItem->sTopic ;
            aCollectString  +=  OUString::createFromAscii("\n")             ;
        }
        aCollectString  +=  OUString::createFromAscii("\0") ;   // It's better :-)

        m_xTopic_Bottom->setText ( aCollectString ) ;
    }

    // Rebuild right site of text
    if (m_xText_Bottom.is())
    {
        OUString        aCollectString  ;

        // Collect all topics from list and format text.
        // "\n" MUST BE at the end of line!!! => Else ... topic and his text are not in the same line!!!
        for ( sal_uInt32 n=0; n<m_pTextlist_Bottom->Count(); ++n )
        {
            IMPL_TextlistItem* pSearchItem = m_pTextlist_Bottom->GetObject (n) ;
            aCollectString  +=  pSearchItem->sText ;
            aCollectString  +=  OUString::createFromAscii("\n")            ;
        }
        aCollectString  +=  OUString::createFromAscii("\0") ;   // It's better :-)

        m_xText_Bottom->setText ( aCollectString ) ;
    }
}

//____________________________________________________________________________________________________________
//  private method
//____________________________________________________________________________________________________________

void ProgressMonitor::impl_cleanMemory ()
{
    // Ready for multithreading
    MutexGuard aGuard ( m_aMutex ) ;

    // Delete all of lists.

    sal_uInt32 nPosition ;

    for ( nPosition = 0; nPosition < m_pTextlist_Top->Count () ; ++nPosition )
    {
        IMPL_TextlistItem* pSearchItem = m_pTextlist_Top->GetObject ( nPosition ) ;
        delete pSearchItem ;
    }
    m_pTextlist_Top->Clear () ;
    delete m_pTextlist_Top ;

    for ( nPosition = 0; nPosition < m_pTextlist_Bottom->Count () ; ++nPosition )
    {
        IMPL_TextlistItem* pSearchItem = m_pTextlist_Bottom->GetObject ( nPosition ) ;
        delete pSearchItem ;
    }
    m_pTextlist_Bottom->Clear () ;
    delete m_pTextlist_Bottom ;
}

//____________________________________________________________________________________________________________
//  private method
//____________________________________________________________________________________________________________

IMPL_TextlistItem* ProgressMonitor::impl_searchTopic ( const OUString& rTopic, sal_Bool bbeforeProgress )
{
    // Get right textlist for following operations.
    IMPL_Textlist* pTextList ;

    // Ready for multithreading
    ClearableMutexGuard aGuard ( m_aMutex ) ;

    if ( bbeforeProgress == sal_True )
    {
        pTextList = m_pTextlist_Top    ;
    }
    else
    {
        pTextList = m_pTextlist_Bottom ;
    }

    // Switch off guard.
    aGuard.clear () ;

    // Search the topic in textlist.
    sal_uInt32  nPosition   =   0                   ;
    sal_uInt32  nCount      =   pTextList->Count () ;

    for ( nPosition = 0; nPosition < nCount ; ++nPosition )
    {
        IMPL_TextlistItem* pSearchItem = pTextList->GetObject ( nPosition ) ;

        if ( pSearchItem->sTopic == rTopic )
        {
            // We have found this topic ... return a valid pointer.
            return pSearchItem ;
        }
    }

    // We have'nt found this topic ... return a nonvalid pointer.
    return NULL ;
}

//____________________________________________________________________________________________________________
//  debug methods
//____________________________________________________________________________________________________________

#ifdef DBG_UTIL

// addText, updateText
sal_Bool ProgressMonitor::impl_debug_checkParameter ( const OUString& rTopic, const OUString& rText, sal_Bool bbeforeProgress )
{
    // Check "rTopic"
    if ( &rTopic        ==  NULL    ) return sal_False ;    // NULL-pointer for reference ???!!!
    if ( rTopic.len ()  <   1       ) return sal_False ;    // ""

    // Check "rText"
    if ( &rText         ==  NULL    ) return sal_False ;    // NULL-pointer for reference ???!!!
    if ( rText.len ()   <   1       ) return sal_False ;    // ""

    // "bbeforeProgress" is valid in everyway!

    // Parameter OK ... return sal_True.
    return sal_True ;
}

// removeText
sal_Bool ProgressMonitor::impl_debug_checkParameter ( const OUString& rTopic, sal_Bool bbeforeProgress )
{
    // Check "rTopic"
    if ( &rTopic        ==  NULL    ) return sal_False ;    // NULL-pointer for reference ???!!!
    if ( rTopic.len ()  <   1       ) return sal_False ;    // ""

    // "bbeforeProgress" is valid in everyway!

    // Parameter OK ... return sal_True.
    return sal_True ;
}

#endif  // #ifdef DBG_UTIL

}   // namespace unocontrols
