/*************************************************************************
 *
 *  $RCSfile: interceptionhelper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-07-02 13:24:41 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_DISPATCH_INTERCEPTIONHELPER_HXX_
#include <dispatch/interceptionhelper.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_WILDCARD_HXX_
#include <classes/wildcard.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XINTERCEPTORINFO_HPP_
#include <com/sun/star/frame/XInterceptorInfo.hpp>
#endif

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

using namespace ::com::sun::star::frame         ;
using namespace ::com::sun::star::lang          ;
using namespace ::com::sun::star::uno           ;
using namespace ::com::sun::star::util          ;
using namespace ::cppu                          ;
using namespace ::osl                           ;
using namespace ::rtl                           ;
using namespace ::std                           ;

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
//  constructor
//*****************************************************************************************************************
InterceptionHelper::InterceptionHelper( const Reference< XFrame >&            rFrame           ,
                                          const Reference< XDispatchProvider >& xSlaveDispatcher )
        //  Init baseclasses first
        :   ThreadHelpBase      ( &Application::GetSolarMutex() )
        ,   OWeakObject         (                               )
        // Init member
        ,   m_xSlaveDispatcher  ( xSlaveDispatcher              )
        ,   m_xOwnerWeak        ( rFrame                        )
{
    // Safe impossible cases
    // Method is not defined for ALL incoming parameters!
    LOG_ASSERT( impldbg_checkParameter_InterceptionHelper( xSlaveDispatcher ), "InterceptionHelper::InterceptionHelper()\nInvalid parameter detected!\n" )
}

//*****************************************************************************************************************
//  (proteced!) destructor
//*****************************************************************************************************************
InterceptionHelper::~InterceptionHelper()
{
}

//*****************************************************************************************************************
//  XInterface
//*****************************************************************************************************************
DEFINE_XINTERFACE_3     (   InterceptionHelper                             ,
                            OWeakObject                                     ,
                            DIRECT_INTERFACE(XDispatchProvider              ),
                            DIRECT_INTERFACE(XDispatchProviderInterception  ),
                            DIRECT_INTERFACE(XEventListener                 )
                        )

//*****************************************************************************************************************
//   XDispatchProvider
//*****************************************************************************************************************
Reference< XDispatch > SAL_CALL InterceptionHelper::queryDispatch( const   URL&        aURL            ,
                                                                    const   OUString&   sTargetFrameName,
                                                                            sal_Int32   nSearchFlags    ) throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT( impldbg_checkParameter_queryDispatch( aURL, sTargetFrameName, nSearchFlags ), "InterceptionHelper::queryDispatch()\nInvalid parameter detected.\n" )

    // Set default return value.
    Reference< XDispatch > xReturn;

    // For better performance:
    // Search interceptor which has registered himself with a pattern which match given URL!
    Reference< XDispatchProvider > xInterceptor( impl_searchMatchingInterceptor( aURL.Complete ), UNO_QUERY );
    if( xInterceptor.is() == sal_True )
    {
        // If so an interceptor exist forward dispatch to it.
        xReturn = xInterceptor->queryDispatch( aURL, sTargetFrameName, nSearchFlags );
    }
    else
    {
        // Otherwise; use ouer own dispatch helper to do this and set results for return.
        xReturn = m_xSlaveDispatcher->queryDispatch( aURL, sTargetFrameName, nSearchFlags );
    }

    // Return results of this operation.
    return xReturn;
}

//*****************************************************************************************************************
//   XDispatchProvider
//*****************************************************************************************************************
Sequence< Reference< XDispatch > > SAL_CALL InterceptionHelper::queryDispatches( const Sequence< DispatchDescriptor >& seqDescriptor ) throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT( impldbg_checkParameter_queryDispatches( seqDescriptor ), "InterceptionHelper::queryDispatches()\nInvalid parameter detected.\n" )

    // Set default return value.
    Sequence< Reference< XDispatch > > seqReturn;

    /*ATTENTION

        Don't use our performance mechanism to search registered interceptors for given URLs!
        What will you do - call different interceptor objects for different URLs ...
        ... we implement these mechanism to save (remote) calls to an interceptor ...
    */

    // If any interceptor exist ...
    if( m_aInterceptorList.empty() == sal_False )
    {
        // ... forward query to these instance and set results for return.
        Reference< XDispatchProvider> xInterceptor( m_aInterceptorList.begin()->xInterceptor, UNO_QUERY );
        seqReturn = xInterceptor->queryDispatches( seqDescriptor );
    }
    else
    {
        // No; Then use ouer own dispatchhelper to do this and set results for return.
        seqReturn = m_xSlaveDispatcher->queryDispatches( seqDescriptor );
    }

    // Return results of this operation.
    return seqReturn;
}

//*****************************************************************************************************************
//   XDispatchProviderInterception
//*****************************************************************************************************************
void SAL_CALL InterceptionHelper::registerDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& xInterceptor ) throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT( impldbg_checkParameter_registerDispatchProviderInterceptor( xInterceptor ), "InterceptionHelper::registerDispatchProviderInterceptor()\nInvalid parameter detected.\n" )

    // Fill a new info structure for new interceptor.
    // Save his reference and try to get an additional URL/pattern list from him.
    // If no list exist register these interceptor for all dispatch events with "*"!
    IMPL_TInterceptorInfo aInfo;
    aInfo.xInterceptor = xInterceptor;
    Reference< XInterceptorInfo > xInfoInterface( xInterceptor, UNO_QUERY );
    if( xInfoInterface.is() == sal_True )
    {
        aInfo.seqPatternList = xInfoInterface->getInterceptedURLs();
    }
    else
    {
        aInfo.seqPatternList.realloc(1);
        aInfo.seqPatternList[0] = DECLARE_ASCII("*");
    }

    // Add new interceptor to front of list and initialize master/slave relations with old top one.
    // If anyone already exist!
    if( m_aInterceptorList.size() < 1 )
    {
        // a)   No interceptor exist before.
        //      Take it as the first one and pass our own dispatch helper as his slave.
        //      (For economical code writing - we add info structure at later time! Otherwise we must duplicate these code for every IF/ELSE branch!)
        aInfo.xInterceptor->setSlaveDispatchProvider( m_xSlaveDispatcher );
    }
    else
    {
        // b)   There is already an interceptor; current will be the slave of the new one.
        //      Insert it on the top of list.
        //      (For economical code writing - we add info structure at later time! Otherwise we must duplicate these code for every IF/ELSE branch!)
        Reference< XDispatchProviderInterceptor > xOldTopInterceptor = m_aInterceptorList.begin()->xInterceptor;
        aInfo.xInterceptor->setSlaveDispatchProvider    ( Reference< XDispatchProvider >( xOldTopInterceptor, UNO_QUERY ) );
        xOldTopInterceptor->setMasterDispatchProvider   ( Reference< XDispatchProvider >( aInfo.xInterceptor, UNO_QUERY ) );
    }

    // The new interceptor must be a slave of us too!
    aInfo.xInterceptor->setMasterDispatchProvider( this );
    // Now we can add our info structure to the list.
    m_aInterceptorList.push_front( aInfo );

    Reference < XFrame > xOwner( m_xOwnerWeak.get(), UNO_QUERY );
    if ( xOwner.is() )
        xOwner->contextChanged();
}

//*****************************************************************************************************************
//   XDispatchProviderInterception
//*****************************************************************************************************************
void SAL_CALL InterceptionHelper::releaseDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& xInterceptor ) throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT( impldbg_checkParameter_releaseDispatchProviderInterceptor( xInterceptor ), "InterceptionHelper::releaseDispatchProviderInterceptor()\nInvalid parameter detected.\n" )

    // Search for existing interceptor in list.
    IMPL_CInterceptorList::iterator aItem = m_aInterceptorList.find( xInterceptor );
    // Safe impossible cases
    // We can't remove an interceptor, which we don't know ...
    LOG_ASSERT( !(aItem==m_aInterceptorList.end()), "InterceptionHelper::releaseDispatchProviderInterceptor()\nCan't remove interceptor which I don't know!\n")
    if( aItem != m_aInterceptorList.end() )
    {
        // Rebuild master/slave relations of interceptor list.
        // Get slave and master of given interceptor.
        Reference< XDispatchProvider >              xSlave              ( xInterceptor->getSlaveDispatchProvider()  , UNO_QUERY );
        Reference< XDispatchProviderInterceptor >   xSlaveInterceptor   ( xSlave                                    , UNO_QUERY );
        Reference< XDispatchProvider >              xMaster             ( xInterceptor->getMasterDispatchProvider() , UNO_QUERY );
        Reference< XDispatchProviderInterceptor >   xMasterInterceptor  ( xMaster                                   , UNO_QUERY );
        // Safe impossible cases.
        // A master must exist - but it must not an interceptor!
        // => We set us as a master of highest registered interecptor; but we don't implement the interceptor interface!
        // The same must valid for slaves.
        // => We set our dispatch helper as slave of lowest registered interceptor - but they don't implement the interceptor interface!
        // ( see register function for further informations! )
        LOG_ASSERT( !(xMaster.is()==sal_False || xSlave.is()==sal_False), "InterceptionHelper::releaseDispatchProviderInterceptor()\nCan't find a master or slave of registered interceptor. Taht could'nt be!\n" )
        // Reconnect slave relation.
        if( xMasterInterceptor.is() == sal_True )
        {
            xMasterInterceptor->setSlaveDispatchProvider( xSlave );
        }
        // Reconnect master relation.
        if( xSlaveInterceptor.is() == sal_True )
        {
            xSlaveInterceptor->setMasterDispatchProvider( xMaster );
        }
        // Unchain the interceptor that has to be removed.
        xInterceptor->setSlaveDispatchProvider  ( Reference< XDispatchProvider >() );
        xInterceptor->setMasterDispatchProvider ( Reference< XDispatchProvider >() );
        // Remove interceptor from our list.
        m_aInterceptorList.erase( aItem );
    }

    Reference < XFrame > xOwner( m_xOwnerWeak.get(), UNO_QUERY );
    if ( xOwner.is() )
        xOwner->contextChanged();
}

//*****************************************************************************************************************
//   XDispatchProviderInterception
//*****************************************************************************************************************
Reference< XDispatchProviderInterceptor > InterceptionHelper::impl_searchMatchingInterceptor( const OUString& sURL )
{
    // Step over all items and return interceptor, which URL list match given one at first.
    sal_Int32 nInterceptorCount = (sal_Int32)(m_aInterceptorList.size());
    for( sal_Int32 nInterceptor=0; nInterceptor<nInterceptorCount; ++nInterceptor )
    {
        // Use reference to list for faster access! (Don't use index operator for every element.)
        // Don't check index - we know what we do ...
        const Sequence< OUString >& seqPatternList = m_aInterceptorList[nInterceptor].seqPatternList;
        sal_Int32 nPatternCount = seqPatternList.getLength();
        for( sal_Int32 nPattern=0; nPattern<nPatternCount; ++nPattern )
        {
            // Try to match given URL with current pattern.
            // If it match true we have found an interceptor and can return it.
            // ( For better performance we return immediately! )
            if( Wildcard::match( sURL, seqPatternList[nPattern] ) == sal_True )
            {
                return m_aInterceptorList[nInterceptor].xInterceptor;
            }
        }
    }
    // Return with an empty result of operation!
    // No pattern and no interceptor was found.
    return Reference< XDispatchProviderInterceptor >();
}

//*****************************************************************************************************************
//   XEventListener
//*****************************************************************************************************************
void SAL_CALL InterceptionHelper::disposing( const EventObject& Source ) throw ( RuntimeException )
{
    Reference< XEventListener >( m_xSlaveDispatcher, UNO_QUERY )->disposing( Source );
    m_xSlaveDispatcher = 0;
}

//_________________________________________________________________________________________________________________
//  debug methods
//_________________________________________________________________________________________________________________

/*-----------------------------------------------------------------------------------------------------------------
    The follow methods checks the parameter for other functions. If a parameter or his value is non valid,
    we return "sal_False". (else sal_True) This mechanism is used to throw an ASSERT!

    ATTENTION

        If you miss a test for one of this parameters, contact the autor or add it himself !(?)
        But ... look for right testing! See using of this methods!
-----------------------------------------------------------------------------------------------------------------*/

#ifdef ENABLE_ASSERTIONS

//*****************************************************************************************************************
// An instance of this class can only work with valid initialization.
sal_Bool InterceptionHelper::impldbg_checkParameter_InterceptionHelper( const Reference< XDispatchProvider >& xSlaveDispatcher )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xSlaveDispatcher     ==  NULL        )   ||
            ( xSlaveDispatcher.is() ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// We accept non zero URLs only. Target name can be empty!
// And we can't test search flags in all combinations ...
sal_Bool InterceptionHelper::impldbg_checkParameter_queryDispatch( const   URL&        aURL            ,
                                                                    const   OUString&   sTargetFrameName,
                                                                            sal_Int32   nSearchFlags    )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &aURL                         ==  NULL    )   ||
            ( &sTargetFrameName             ==  NULL    )   ||
            ( aURL.Complete.getLength()     <   1       )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// We check the same like queryDispatch() before but for a list of description items.
sal_Bool InterceptionHelper::impldbg_checkParameter_queryDispatches( const Sequence< DispatchDescriptor >& seqDescriptor )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if( &seqDescriptor == NULL )
    {
        bOK = sal_False;
    }
    else
    {
        sal_Int32 nCount = seqDescriptor.getLength();
        if( nCount < 1 )
        {
            bOK = sal_False;
        }
        else
        {
            for( sal_Int32 nPosition=0; nPosition<nCount; ++nPosition )
            {
                if  ( impldbg_checkParameter_queryDispatch  (   seqDescriptor[nPosition].FeatureURL ,
                                                                seqDescriptor[nPosition].FrameName  ,
                                                                seqDescriptor[nPosition].SearchFlags
                                                            ) == sal_False
                    )
                {
                    bOK = sal_False;
                    break;
                }
            }
        }
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// A valid interceptor must given and he must support the XDispatchProvider interface too!
sal_Bool InterceptionHelper::impldbg_checkParameter_registerDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& xInterceptor )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xInterceptor                                                     ==  NULL        )   ||
            ( xInterceptor.is()                                                 ==  sal_False   )   ||
            ( Reference< XDispatchProvider >( xInterceptor, UNO_QUERY ).is()    ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// A valid interceptor must given and he must support the XDispatchProvider interface too!
sal_Bool InterceptionHelper::impldbg_checkParameter_releaseDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& xInterceptor )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xInterceptor                                                     ==  NULL        )   ||
            ( xInterceptor.is()                                                 ==  sal_False   )   ||
            ( Reference< XDispatchProvider >( xInterceptor, UNO_QUERY ).is()    ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

#endif  //  #ifdef ENABLE_ASSERTIONS

}       //  namespace framework
