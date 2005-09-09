/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: commandenvironmentproxy.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:25:35 $
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

#ifndef _UCBHELPER_COMMANDENVIRONMENTPROXY_HXX
#define _UCBHELPER_COMMANDENVIRONMENTPROXY_HXX

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif

namespace ucb
{

struct CommandEnvironmentProxy_Impl;

//=========================================================================

/**
  * This class implements the interface
  * com::sun::star::ucb::XCommandEnvironement.
  *
  * Instances of this class can be used to create a (local) proxy for (remote)
  * command environment implementations. This implementation caches the
  * (remote) interfaces supplied by the given environment in order to avoid the
  * overhead produced by multiple (remote) calls to methods of the given
  * (remote) command environment.
  */
class CommandEnvironmentProxy : public cppu::OWeakObject,
                                   public com::sun::star::lang::XTypeProvider,
                                   public com::sun::star::ucb::XCommandEnvironment
{
    CommandEnvironmentProxy_Impl* m_pImpl;

private:
    CommandEnvironmentProxy( const CommandEnvironmentProxy& );            // n.i.
    CommandEnvironmentProxy& operator=( const CommandEnvironmentProxy& ); // n.i.

public:
    /**
      * Constructor.
      *
      * @param rxEnv is the implementation of a (remote) command environment.
      */
    CommandEnvironmentProxy(
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XCommandEnvironment >& rxEnv );
    /**
      * Destructor.
      */
    virtual ~CommandEnvironmentProxy();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

     // XCommandEnvironemnt
    virtual com::sun::star::uno::Reference<
                com::sun::star::task::XInteractionHandler > SAL_CALL
    getInteractionHandler()
        throw ( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XProgressHandler > SAL_CALL
    getProgressHandler()
        throw ( com::sun::star::uno::RuntimeException );
};

} /* namespace ucb */

#endif /* !_UCBHELPER_COMMANDENVIRONMENTPROXY_HXX */
