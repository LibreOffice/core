/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _UCBHELPER_COMMANDENVIRONMENTPROXY_HXX
#define _UCBHELPER_COMMANDENVIRONMENTPROXY_HXX

#include <com/sun/star/lang/XTypeProvider.hpp>
#ifndef _COM_SUN_STAR_UCB_XCOMMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>

namespace ucbhelper
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

} /* namespace ucbhelper */

#endif /* !_UCBHELPER_COMMANDENVIRONMENTPROXY_HXX */
