/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: commandenvironment.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 17:19:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucbhelper.hxx"

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

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif

#ifndef _UCBHELPER_COMMANDENVIRONMENT_HXX
#include <ucbhelper/commandenvironment.hxx>
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
// struct CommandEnvironment_Impl.
//
//=========================================================================
//=========================================================================

struct CommandEnvironment_Impl
{
    Reference< XInteractionHandler > m_xInteractionHandler;
    Reference< XProgressHandler >    m_xProgressHandler;

    CommandEnvironment_Impl(
        const Reference< XInteractionHandler >& rxInteractionHandler,
        const Reference< XProgressHandler >& rxProgressHandler )
    : m_xInteractionHandler( rxInteractionHandler ),
      m_xProgressHandler( rxProgressHandler ) {}
};

//=========================================================================
//=========================================================================
//
// CommandEnvironment Implementation.
//
//=========================================================================
//=========================================================================

CommandEnvironment::CommandEnvironment(
        const Reference< XInteractionHandler >& rxInteractionHandler,
        const Reference< XProgressHandler >& rxProgressHandler )
{
    m_pImpl = new CommandEnvironment_Impl( rxInteractionHandler,
                                           rxProgressHandler );
}

//=========================================================================
// virtual
CommandEnvironment::~CommandEnvironment()
{
    delete m_pImpl;
}

//=========================================================================
//
// XInterface methods
//
//=========================================================================

XINTERFACE_IMPL_2( CommandEnvironment,
                   XTypeProvider,
                   XCommandEnvironment );

//=========================================================================
//
// XTypeProvider methods
//
//=========================================================================

XTYPEPROVIDER_IMPL_2( CommandEnvironment,
                      XTypeProvider,
                      XCommandEnvironment );

//=========================================================================
//
// XCommandEnvironemnt methods.
//
//=========================================================================

// virtual
Reference< XInteractionHandler > SAL_CALL
CommandEnvironment::getInteractionHandler()
    throw ( RuntimeException )
{
    return m_pImpl->m_xInteractionHandler;
}

//=========================================================================
// virtual
Reference< XProgressHandler > SAL_CALL
CommandEnvironment::getProgressHandler()
    throw ( RuntimeException )
{
    return m_pImpl->m_xProgressHandler;
}

} /* namespace ucb */

