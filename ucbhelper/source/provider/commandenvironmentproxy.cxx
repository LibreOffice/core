/*************************************************************************
 *
 *  $RCSfile: commandenvironmentproxy.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kso $ $Date: 2001-02-15 13:32:27 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIERFACTORY_HPP_
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDERMANAGER_HPP_
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _UCBHELPER_COMMANDENVIRONMENTPROXY_HXX
#include <ucbhelper/commandenvironmentproxy.hxx>
#endif

using namespace com::sun::star::lang;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace rtl;

namespace ucb
{

//=========================================================================
//=========================================================================
//
// struct CommandEnvironmentProxy_Impl.
//
//=========================================================================
//=========================================================================

struct CommandEnvironmentProxy_Impl
{
    osl::Mutex                       m_aMutex;
    Reference< XCommandEnvironment > m_xEnv;
    Reference< XInteractionHandler > m_xInteractionHandler;
    Reference< XProgressHandler >    m_xProgressHandler;
    sal_Bool m_bGotInteractionHandler;
    sal_Bool m_bGotProgressHandler;

    CommandEnvironmentProxy_Impl(
        const Reference< XCommandEnvironment >& rxEnv )
    : m_xEnv( rxEnv ), m_bGotInteractionHandler( sal_False ),
      m_bGotProgressHandler( sal_False ) {}
};

//=========================================================================
//=========================================================================
//
// CommandEnvironmentProxy Implementation.
//
//=========================================================================
//=========================================================================

CommandEnvironmentProxy::CommandEnvironmentProxy(
                        const Reference< XCommandEnvironment >& rxEnv )
{
    m_pImpl = new CommandEnvironmentProxy_Impl( rxEnv );
}

//=========================================================================
// virtual
CommandEnvironmentProxy::~CommandEnvironmentProxy()
{
    delete m_pImpl;
}

//=========================================================================
//
// XInterface methods
//
//=========================================================================

XINTERFACE_IMPL_2( CommandEnvironmentProxy,
                   XTypeProvider,
                   XCommandEnvironment );

//=========================================================================
//
// XTypeProvider methods
//
//=========================================================================

XTYPEPROVIDER_IMPL_2( CommandEnvironmentProxy,
                      XTypeProvider,
                      XCommandEnvironment );

//=========================================================================
//
// XCommandEnvironemnt methods.
//
//=========================================================================

// virtual
Reference< XInteractionHandler > SAL_CALL
CommandEnvironmentProxy::getInteractionHandler()
    throw ( RuntimeException )
{
    if ( m_pImpl->m_xEnv.is() )
    {
        if ( !m_pImpl->m_bGotInteractionHandler )
        {
            osl::MutexGuard aGuard( m_pImpl->m_aMutex );
            if ( !m_pImpl->m_bGotInteractionHandler )
            {
                m_pImpl->m_xInteractionHandler
                                = m_pImpl->m_xEnv->getInteractionHandler();
                m_pImpl->m_bGotInteractionHandler = sal_True;
            }
        }
    }
    return m_pImpl->m_xInteractionHandler;
}

//=========================================================================
// virtual
Reference< XProgressHandler > SAL_CALL
CommandEnvironmentProxy::getProgressHandler()
    throw ( RuntimeException )
{
    if ( m_pImpl->m_xEnv.is() )
    {
        if ( !m_pImpl->m_bGotProgressHandler )
        {
            osl::MutexGuard aGuard( m_pImpl->m_aMutex );
            if ( !m_pImpl->m_bGotProgressHandler )
            {
                m_pImpl->m_xProgressHandler
                                = m_pImpl->m_xEnv->getProgressHandler();
                m_pImpl->m_bGotProgressHandler = sal_True;
            }
        }
    }
    return m_pImpl->m_xProgressHandler;
}

} /* namespace ucb */

