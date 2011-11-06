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
#include "precompiled_connectivity.hxx"

#include "HTerminateListener.hxx"
#include "hsqldb/HDriver.hxx"

//........................................................................
namespace connectivity
{
//........................................................................
    using namespace hsqldb;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::lang;
    using namespace ::rtl;

// XEventListener
void SAL_CALL OConnectionController::disposing( const EventObject& /*Source*/ )
throw( RuntimeException )
{
}

// XTerminateListener
void SAL_CALL OConnectionController::queryTermination( const EventObject& /*aEvent*/ )
throw( TerminationVetoException, RuntimeException )
{
    m_pDriver->flushConnections();
}

void SAL_CALL OConnectionController::notifyTermination( const EventObject& /*aEvent*/ )
throw( RuntimeException )
{
    m_pDriver->shutdownConnections();
}

//........................................................................
}   // namespace connectivity
//........................................................................
