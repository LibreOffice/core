/*************************************************************************
 *
 *  $RCSfile: statusindicator.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-08-10 11:54:14 $
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

#ifndef __FRAMEWORK_HELPER_STATUSINDICATOR_HXX_
#include <helper/statusindicator.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

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

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_2     (   StatusIndicator                             ,
                            OWeakObject                                 ,
                            DIRECT_INTERFACE(css::lang::XTypeProvider   ),
                            DIRECT_INTERFACE(css::task::XStatusIndicator)
                        )

DEFINE_XTYPEPROVIDER_2  (   StatusIndicator                             ,
                            css::lang::XTypeProvider                    ,
                            css::task::XStatusIndicator
                        )

/*-************************************************************************************************************//**
    @short      standard ctor
    @descr      These initialize a new instance of this class with all needed informations for work.

    @seealso    -

    @param      "pFactory", pointer to factory, which has created this instance (XStatusIndicatorFactory!)
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
StatusIndicator::StatusIndicator( StatusIndicatorFactory* pFactory )
        //  Init baseclasses first
        :   ThreadHelpBase      ( &Application::GetSolarMutex() )
        ,   ::cppu::OWeakObject (                               )
        // Init member
        ,   m_pFactory          ( pFactory                      )
        ,   m_xFactoryWeak      ( pFactory                      )
{
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT2( implcp_StatusIndicator( pFactory ), "StatusIndicator::StatusIndicator()", "Invalid parameter detected!" )
}

/*-************************************************************************************************************//**
    @short      standard dtor
    @descr      Forget all references and pointer ... let they die.

    @seealso    -

    @param      -
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
StatusIndicator::~StatusIndicator()
{
    m_pFactory     = NULL                                                           ;
    m_xFactoryWeak = css::uno::WeakReference< css::task::XStatusIndicatorFactory >();
}

/*-************************************************************************************************************//**
    @interface  XStatusIndicator
    @short      These methods implement the functionality of this interface.
    @descr      But instances of this class are wrapper only. They try to get a right references from saved weak reference
                to her StatusIndicatorFactory. If it was successfully - they forward her calls to this factory only.
                These factory decide, which of the caller objects is the most active one and can show his informations.
                Other calls are ignored.

    @seealso    class StatusIndicatorFactory

    @param      "sText" , new text to show it in dialog
    @param      "nRange", new range for progress
    @param      "nValue", new value to show the progress
    @return     -

    @onerror    We do nothing!
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL StatusIndicator::start( const ::rtl::OUString& sText  ,
                                            sal_Int32        nRange ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT2( implcp_start( sText, nRange ), "StatusIndicator::start()", "Invalid parameter detected!" )

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Ready for multithreading
    ResetableGuard aLock( m_aLock );

    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory( m_xFactoryWeak.get(), css::uno::UNO_QUERY );
    if( xFactory.is() == sal_True )
    {
        m_pFactory->start( this, sText, nRange );
    }
}

//*****************************************************************************************************************
void SAL_CALL StatusIndicator::end() throw( css::uno::RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Ready for multithreading
    ResetableGuard aLock( m_aLock );

    // Attention: Our factory forget our reference after calling "end()". So we must protect us against
    // dieing during this operation!
    css::uno::Reference< css::uno::XInterface > xThis( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );

    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory( m_xFactoryWeak.get(), css::uno::UNO_QUERY );
    if( xFactory.is() == sal_True )
    {
        m_pFactory->end( this );
    }
}

//*****************************************************************************************************************
void SAL_CALL StatusIndicator::reset() throw( css::uno::RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Ready for multithreading
    ResetableGuard aLock( m_aLock );

    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory( m_xFactoryWeak.get(), css::uno::UNO_QUERY );
    if( xFactory.is() == sal_True )
    {
        m_pFactory->reset( this );
    }
}

//*****************************************************************************************************************
void SAL_CALL StatusIndicator::setText( const ::rtl::OUString& sText ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT2( implcp_setText( sText ), "StatusIndicator::setText()", "Invalid parameter detected!" )

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Ready for multithreading
    ResetableGuard aLock( m_aLock );

    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory( m_xFactoryWeak.get(), css::uno::UNO_QUERY );
    if( xFactory.is() == sal_True )
    {
        m_pFactory->setText( this, sText );
    }
}

//*****************************************************************************************************************
void SAL_CALL StatusIndicator::setValue( sal_Int32 nValue ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT2( implcp_setValue( nValue ), "StatusIndicator::setValue()", "Invalid parameter detected!" )

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Ready for multithreading
    ResetableGuard aLock( m_aLock );

    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory( m_xFactoryWeak.get(), css::uno::UNO_QUERY );
    if( xFactory.is() == sal_True )
    {
        m_pFactory->setValue( this, nValue );
    }
}

//_________________________________________________________________________________________________________________
//  debug methods
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short      debug-method to check incoming parameter of some other mehods of this class
    @descr      The following methods are used to check parameters for other methods
                of this class. The return value is used directly for an ASSERT(...).

    @seealso    ASSERTs in implementation!

    @param      references to checking variables
    @return     sal_False ,on invalid parameter
    @return     sal_True  ,otherwise

    @onerror    -
*//*-*************************************************************************************************************/
#ifdef ENABLE_ASSERTIONS

//*****************************************************************************************************************
sal_Bool StatusIndicator::implcp_StatusIndicator( StatusIndicatorFactory* pFactory )
{
    return( pFactory == NULL );
}

//*****************************************************************************************************************
sal_Bool StatusIndicator::implcp_start( const ::rtl::OUString&  sText   ,
                                              sal_Int32         nRange  )
{
    return(
            ( &sText  == NULL ) ||
            ( &nRange == NULL ) ||
            ( nRange  <  1    )     // We count from 0 to range ... and range should be greater then 0 ...!
          );
}

//*****************************************************************************************************************
sal_Bool StatusIndicator::implcp_setText( const ::rtl::OUString& sText )
{
    return( &sText == NULL );
}

//*****************************************************************************************************************
sal_Bool StatusIndicator::implcp_setValue( sal_Int32 nValue )
{
    return( nValue < 0 );
}

#endif  //  #ifdef ENABLE_ASSERTIONS

}       //  namespace framework
