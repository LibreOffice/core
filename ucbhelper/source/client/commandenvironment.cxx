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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucbhelper.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <ucbhelper/commandenvironment.hxx>

using namespace com::sun::star::lang;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace rtl;

namespace ucbhelper
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

} /* namespace ucbhelper */

