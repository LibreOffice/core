/*************************************************************************
 *
 *  $RCSfile: statusindicatorfactory.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:21:40 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <algorithm>

#ifndef __FRAMEWORK_HELPER_STATUSINDICATORFACTORY_HXX_
#include <helper/statusindicatorfactory.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_STATUSINDICATOR_HXX_
#include <helper/statusindicator.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONGUARD_HXX_
#include <threadhelp/transactionguard.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XCONTROLS_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XLAYOUTCONSTRAINS_HPP_
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_DEVICEINFO_HPP_
#include <com/sun/star/awt/DeviceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_WINDOWATTRIBUTE_HPP_
#include <com/sun/star/awt/WindowAttribute.hpp>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#include <time.h>
//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

#define TIMEOUT_START_RESCHEDULE    10L /* 10th s */

sal_Int32 StatusIndicatorFactory::m_nInReschedule = 0;  /// static counter for rescheduling

//*****************************************************************************************************************
//  XInterface
//*****************************************************************************************************************
DEFINE_XINTERFACE_3     (   StatusIndicatorFactory                              ,
                            OWeakObject                                         ,
                            DIRECT_INTERFACE(css::task::XStatusIndicatorFactory ),
                            DIRECT_INTERFACE(css::awt::XWindowListener          ),
                            DIRECT_INTERFACE(css::lang::XEventListener          )
                        )

/*-************************************************************************************************************//**
    @short      standard ctor
    @descr      These initialize a new instance of this class with all needed informations for work.
                We use given window reference as parent of our status indicator window.
                Please don't release it before you kill us - we can live with that ... but vcl will run into
                problems! He miss his parent vcl window and assert "window with living child destroyed".

    @attention  Normaly we don't need any mutex or lock here. These ctor isn't breakble and couldn't be called twice!

    @seealso    -

    @param      "xFactory"     , reference to uno servicemanager to create own needed services
    @return     "xParentWindow", will be the parent window of our status indicator control!

    @onerror    We do nothing and this instance will not ready for real working.
                Calling of interface methods will throw an UninitializedException then!
    @threadsafe yes
*//*-*************************************************************************************************************/
StatusIndicatorFactory::StatusIndicatorFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory      ,
                                                const css::uno::Reference< css::awt::XWindow >&               xParentWindow ,
                                                      sal_Bool                                                bShowStatusBar)
        //  Init baseclasses first
        :   ThreadHelpBase      ( &Application::GetSolarMutex() )
        ,   TransactionBase     (                               )
        ,   ::cppu::OWeakObject (                               )
        // Init member
        ,   m_xFactory          ( xFactory                      )
        ,   m_xParentWindow     ( xParentWindow                 )
        ,   m_pStatusBar        ( NULL                          )
        ,   m_bProgressMode     ( sal_False                     )
{
    try
    {
        m_xParentWindow->addWindowListener( this );

        // Don't forget to open instance for normal working!
        m_aTransactionManager.setWorkingMode( E_WORK );

        if (bShowStatusBar)
            impl_createStatusBar();
    }
    catch( css::uno::RuntimeException& )
    {
    }
}

/*-************************************************************************************************************//**
    @short      standard destructor
    @descr      We do nothing here! Releasing of our used references should be implemented in disposing() method.
                We check calling of this method and state of this object here only and assert wrong using of class.

    @seealso    method disposing()

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
StatusIndicatorFactory::~StatusIndicatorFactory()
{
    LOG_ASSERT2( m_aTransactionManager.getWorkingMode()!=E_CLOSE, "StatusIndicatorFactory::~StatusIndicatorFactory()", "Object wasn't disposed! WHY?" )
}

/*-************************************************************************************************************//**
    @interface  com.sun.star.task.XStatusIndicatorFactory
    @short      create new status indicator
    @descr      New created indicator objects share same progress window to show her informations. Only
                last created one "is visible". All other are ignored. If this most active one is gone ...
                the next one from stack can show his informations. He mustn't know anything about this mechanism.
                It works automaticly! If a non visible indicator object is gone before he was visible ... next one
                can work ...

    @seealso    member m_xActiveIndicator
    @seealso    member m_xSharedStatusIndicator

    @param      -
    @return     Reference to new created indicator interface object.

    @onerror    We return NULL.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::task::XStatusIndicator > SAL_CALL StatusIndicatorFactory::createStatusIndicator() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Create a new indicator, initialize and add it to our stack ...
    // ... but do it only, if created indicator is valid.
    // Invalid objects are ignored! Change active indicator too. Only last created one could show his information.
    // Attention: These StatusIndicator childs hold a weakreference to us. They mustn't be a listener at this instance.
    // If we release all our references, they will die automaticly!
    StatusIndicator* pIndicator = new StatusIndicator( this );
    css::uno::Reference< css::task::XStatusIndicator > xIndicator( static_cast< ::cppu::OWeakObject* >(pIndicator), css::uno::UNO_QUERY );

    return xIndicator;
}

/*-************************************************************************************************************//**
    @interface  com.sun.star.awt.XWindowListener
    @short      some listener callbacks for window events
    @descr      We know our parent and the window of our shared status indicator to show our informations realy.
                If somewhere change the parent window, we must actualize the progress window too. That's why
                we are listener for some events and use it to recalc our layout.

    @attention  Impl-Method is threadsafe by herself. We don't need any lock here.

    @seealso    method impl_recalcLayout()

    @param      "aEvent", describe source of this callback.
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL StatusIndicatorFactory::windowResized( const css::awt::WindowEvent& aEvent ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    implts_recalcLayout();
}

//*****************************************************************************************************************
void SAL_CALL StatusIndicatorFactory::windowMoved( const css::awt::WindowEvent& aEvent ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    implts_recalcLayout();
}

//*****************************************************************************************************************
void SAL_CALL StatusIndicatorFactory::windowShown( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException )
{
    // Our progress window is always visible. Parent window regulate visible state of both windows by his own state.
    // So it works automaticly!
}

//*****************************************************************************************************************
void SAL_CALL StatusIndicatorFactory::windowHidden( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException )
{
    // Our progress window is always visible. Parent window regulate visible state of both windows by his own state.
    // So it works automaticly!
}

/*-************************************************************************************************************//**
    @interface  com.sun.star.lang.XEventListener
    @short      information callback for disposing of our owner
    @descr      If our owner frame will die - he tell us this by calling this method. So we should dispose all our
                child indicator objects and forget references to used services and our owner. After that this instance
                isn't ready for working any longer.

    @attention  Don't look on event source. We are listener on different services. But if one of them will die we can't work
                any longer. So we can forget ALL references and die insteandly. It's better then checking for valid members
                in every interface method!

    @seealso    -

    @param      "aEvent", describe source of this callback.
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL StatusIndicatorFactory::disposing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );

    // Prevent code against multiple disposing calls!
    // Do it by using exclusiv access (write lock) ... !!! Otherwhise we cant guarantee
    // right calling of this method in a multithreaded environment.
    // May be two calls could pass this barrier before first call stop real working of this object by calling of
    // "setWorkingMode()".
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Close object for real working. We are dead. Only some (realy) neccessary calls on this instance are allowed.
    // => look for "SOFTEXCEPTIONS" in this code!
    // This call blocks till all currently registered transactions are gone.
    // That's why we must release our own transaction before!!!
    aTransaction.stop();
    m_aTransactionManager.setWorkingMode( E_BEFORECLOSE );

    // Prevent us against dieing during this method is called!
    // May be last owner of this instance release all references to us.
    css::uno::Reference< css::uno::XInterface > xThis( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );

    // Lock isn't neccessary any longer. We should be alone now!
    // All further calls are rejected with an exception ...
    aLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    try
    {
        // Let owner, parent and all other references die ...
        m_xParentWindow     = css::uno::Reference< css::awt::XWindow >();
        m_xFactory          = css::uno::Reference< css::lang::XMultiServiceFactory >();
        m_xActiveIndicator  = css::uno::Reference< css::task::XStatusIndicator >();

        // Forget all currently registered child indicator objects.
        // We mustn't dispose these objects! The hold weak references to us ...
        // and if these references are not valid ... they will die automaticly.
        m_aStack.clear();

        if (m_pStatusBar)
        {
            delete m_pStatusBar;
            m_pStatusBar = NULL;
        }

        // Mark object as dead!
        m_aTransactionManager.setWorkingMode( E_CLOSE );
    }
    catch( css::uno::RuntimeException& )
    {
    }
}

/*-************************************************************************************************************//**
    @interface  -
    @short      public helper methods!
    @descr      Our indicator childs has one shared status indicator window only to show her informations.
                These window is hold by us! So they must call us. We decide, if calling child is the most active one and if
                indicator is available. If both cases are true, we let him work. Otherwhise we do nothing.

                Follow methods append new child to our internal stack by calling start(), release it from stack by calling end()
                and let him work by calling setText/setValue/reset(), if he is the most active one.

    @attention  a)

Normaly we could give our child indicators the shared indicator interface directly ... but nobody
                decide then, which instance ist the most active one! On the other side we couldn't  implement
                the XStatusIndicator interface. Then we have no informations about caller and couldn't dispatch
                calls from multiple childs ... So we implement it as normal C++ public functions. Child objects
                check her weak references and if this reference is valid they could call the implmentation directly.
                This works for remote cases too. Because we created this childs by using "new operator". So they
                must be included in our local environment too ...

                b)
                Make it exclusiv! Because may be we must change some thing here ... but in every case
                we must be shure, that our most active child is realy the most active one. We must synchronize us
                with "createStatusIndicator()"!!!
                That's why we must have an exclusiv access on this member ... Create method use a write lock too.
                He SET this member ...

    @seealso    class StatusIndicator

    @param      "xChild", child indicator object, which whish to show his information in shared status indicator
    @parem      "sText" , new text for showing in dialog
    @parem      "nRange", progress range of these child
    @parem      "nValue", new progress value for showing in dialog
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
void StatusIndicatorFactory::start( const css::uno::Reference< css::task::XStatusIndicator >& xChild ,
                                    const ::rtl::OUString&                                    sText  ,
                                          sal_Int32                                           nRange )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );

    IndicatorStack::iterator pItem = ::std::find( m_aStack.begin(), m_aStack.end(), xChild );
    if( pItem != m_aStack.end() )
    {
        m_aStack.erase( pItem );
    }
    IndicatorInfo aInfo( xChild, sText, nRange );
    m_aStack.push_back ( aInfo                 );

    m_xActiveIndicator = xChild;

    try
    {
        vos::OGuard aGuard( Application::GetSolarMutex() );

        // Create status indicator window to shared it for all created indictaor objects by this factory.
        if(!m_pStatusBar)
            impl_createStatusBar();

        if(!m_bProgressMode)
        {
            m_xParentWindow->setVisible( sal_True );
            implts_recalcLayout();
            m_pStatusBar->Show();
            m_pStatusBar->StartProgressMode( sText );
            m_bProgressMode = sal_True;
        }

        m_nStartTime = impl_get10ThSec();
    }
    catch( css::uno::RuntimeException& )
    {
    }

    aLock.unlock();
    impl_reschedule();
}

void StatusIndicatorFactory::impl_createStatusBar()
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );

    Window* pParentWindow = VCLUnoHelper::GetWindow( m_xParentWindow );
    if ( pParentWindow )
    {
        /* SOLAR SAFE { */
        ::vos::OClearableGuard aSolarLock(Application::GetSolarMutex());
        m_pStatusBar = new StatusBar( pParentWindow, WB_3DLOOK|WB_BORDER );
        implts_recalcLayout();
        m_pStatusBar->Show();
        // force repaint!
        pParentWindow->Show();
        pParentWindow->Invalidate( INVALIDATE_CHILDREN );
        pParentWindow->Flush();
        aSolarLock.clear();
        /* } SOLAR SAFE */
    }

    aLock.unlock();
}

/*-************************************************************************************************************//**
    @interface  -
    @short      public helper methods!
    @descr      Any child indicator will die and finish his work. Delete it from stack everytime.
                We mustn't know it any longer.
                But if child is the most active one - we must search another one from stack.
                If we found anyone - take his values into the shared dialog ...
                if no other one could be found - disable shared dialog!

    @seealso    class StatusIndicator

    @param      "xChild", child indicator object, which whish to show his information in shared status indicator
    @parem      "sText" , new text for showing in dialog
    @parem      "nValue", new progress value for showing in dialog
    @return     -

    @onerror    We do nothing!
    @threadsafe yes
*//*-*************************************************************************************************************/
void StatusIndicatorFactory::end( const css::uno::Reference< css::task::XStatusIndicator >& xChild )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );

    IndicatorStack::iterator pItem = ::std::find( m_aStack.begin(), m_aStack.end(), xChild );
    if( pItem != m_aStack.end() )
    {
        m_aStack.erase( pItem );
    }

    if( xChild == m_xActiveIndicator )
    {
        try
        {
            vos::OGuard aGuard( Application::GetSolarMutex() );

            IndicatorStack::reverse_iterator pInfo = m_aStack.rbegin();
            if( pInfo != m_aStack.rend() )
            {
                m_xActiveIndicator = pInfo->m_xIndicator;
                m_pStatusBar->SetProgressValue( (USHORT)pInfo->m_nValue );
                m_pStatusBar->SetText( pInfo->m_sText );
            }
            else
            {
                m_pStatusBar->EndProgressMode();
                m_pStatusBar->Show( sal_False );

                // Destroy shared status indicator.
                // Attention: Don't do it after destroying of parent or indicator window!
                // Otherwhise vcl say: "parent with living child destroyed ..."
                delete m_pStatusBar;
                m_pStatusBar = NULL;

                m_xActiveIndicator = css::uno::Reference< css::task::XStatusIndicator >();

                m_bProgressMode = sal_False;
            }
        }
        catch( css::uno::RuntimeException& )
        {
        }
    }

    aLock.unlock();
    impl_reschedule();
}

/*-************************************************************************************************************//**
    @interface  -
    @short      public helper methods!
    @descr      Any child can try to show his information at our one shared dialog control.
                But only the most active one could do that realy. The other ones don't recognize, that
                her values are supressed! We select it automaticly. But her values are safed for later using.
                If the current active object will gone - we must use next indicator from stack and use his values!

    @attention  It doesn't matter, if child is the most active one or not. We must
                actualize his text value. Because; if he will be the next active one (if current one will gone)
                we must know right value for "Text" to show it on our status window!!!
                Normal we mustn't check result of ::std::find. These child MUST exist in stack. Otherwise our code is wrong!!!


    @seealso    class StatusIndicator

    @param      "xChild", child indicator object, which whish to show his information in shared status indicator
    @parem      "sText" , new text for showing in dialog
    @parem      "nValue", new progress value for showing in dialog
    @return     -

    @onerror    We do nothing!
    @threadsafe yes
*//*-*************************************************************************************************************/
void StatusIndicatorFactory::reset( const css::uno::Reference< css::task::XStatusIndicator >& xChild )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );

    IndicatorStack::iterator pItem = ::std::find( m_aStack.begin(), m_aStack.end(), xChild );
    pItem->reset();

    if( xChild == m_xActiveIndicator )
    {
        try
        {
            vos::OGuard aGuard( Application::GetSolarMutex() );
            m_pStatusBar->SetProgressValue( 0 );
            m_pStatusBar->SetText( String() );
        }
        catch( css::uno::RuntimeException& )
        {
        }
    }

    aLock.unlock();
    impl_reschedule();
}

//*****************************************************************************************************************
void StatusIndicatorFactory::setText( const css::uno::Reference< css::task::XStatusIndicator >& xChild ,
                                      const ::rtl::OUString&                                    sText  )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );

    IndicatorStack::iterator pItem = ::std::find( m_aStack.begin(), m_aStack.end(), xChild );
    pItem->m_sText = sText;

    if( xChild == m_xActiveIndicator )
    {
        try
        {
            vos::OGuard aGuard( Application::GetSolarMutex() );
            m_pStatusBar->SetText( sText );
        }
        catch( css::uno::RuntimeException& )
        {
        }
    }

    aLock.unlock();
    impl_reschedule();
}

//*****************************************************************************************************************
void StatusIndicatorFactory::setValue( const css::uno::Reference< css::task::XStatusIndicator >& xChild ,
                                             sal_Int32                                           nValue )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );

    IndicatorStack::iterator pItem = ::std::find( m_aStack.begin(), m_aStack.end(), xChild );

    USHORT nOldPercentage = (USHORT)pItem->calcPercentage();
    pItem->m_nValue = nValue;

    if( xChild == m_xActiveIndicator )
    {
        try
        {
            USHORT nNewPercentage = (USHORT)pItem->calcPercentage();

            // Set new value only if its new! StatusBar implementation redraws despite the fact
            // that the value isn't new!!!
            if ( nNewPercentage != nOldPercentage )
            {
                vos::OGuard aGuard( Application::GetSolarMutex() );
                m_pStatusBar->SetProgressValue( nNewPercentage );
            }
        }
        catch( css::uno::RuntimeException& )
        {
        }
    }

    // We start rescheduling only after 1 second - this code was successfully introduced by the sfx2
    // implementation of the progress bar.
    sal_Bool bReschedule = (( impl_get10ThSec() - m_nStartTime ) > TIMEOUT_START_RESCHEDULE );

    aLock.unlock();

    if ( bReschedule )
        impl_reschedule();
}

/*-************************************************************************************************************//**
    @interface  -
    @short      helper to optimize the reschedule scheme of the status indicator
    @descr      This is a helper implementation to call reschedule only if are not running inside
                our own reschedule call.

    @param      -
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
void StatusIndicatorFactory::impl_reschedule()
{
    ResetableGuard aGlobalLock( LockHelper::getGlobalLock() );
    if ( m_nInReschedule == 0 )
    {
        ++m_nInReschedule;
        aGlobalLock.unlock();
        Application::Reschedule();
        aGlobalLock.lock();
        --m_nInReschedule;
    }
}

/*-************************************************************************************************************//**
    @interface  -
    @short
    @descr


    @param      -
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
sal_uInt32 StatusIndicatorFactory::impl_get10ThSec()
{
    sal_uInt32 n10Ticks = 10 * (sal_uInt32)clock();
    return n10Ticks / CLOCKS_PER_SEC;
}

/*-************************************************************************************************************//**
    @interface  -
    @short      helper to resize/move indicator window relative to parent window
    @descr      We are listener on our parent window for move/resize events. So we can re-layout our indicator
                window.

    @seealso    method windowResized()
    @seealso    method windowMoved()

    @param      -
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
void StatusIndicatorFactory::implts_recalcLayout()
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );

    try
    {
        vos::OGuard aGuard( Application::GetSolarMutex() );
        {
            if( m_pStatusBar != NULL )
            {
                css::awt::Rectangle   aParentSize     = m_xParentWindow->getPosSize(); // bug on toolkit! They doesn't use solar mutex by herself so we do it here.
                Size                  aStatusBarSize  = m_pStatusBar->GetSizePixel();

                m_pStatusBar->SetPosSizePixel( 0, aParentSize.Height-aStatusBarSize.Height(), aParentSize.Width, aStatusBarSize.Height() );
            }
        }
    }
    catch( css::uno::RuntimeException& )
    {
    }
}

}       //  namespace framework
