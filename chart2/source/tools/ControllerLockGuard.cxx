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
#include "precompiled_chart2.hxx"

#include "ControllerLockGuard.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

ControllerLockGuard::ControllerLockGuard( const Reference< frame::XModel > & xModel ) :
        m_xModel( xModel )
{
    if( m_xModel.is())
        m_xModel->lockControllers();
}

ControllerLockGuard::~ControllerLockGuard()
{
    if( m_xModel.is())
        m_xModel->unlockControllers();
}

// ================================================================================

ControllerLockHelper::ControllerLockHelper( const Reference< frame::XModel > & xModel ) :
        m_xModel( xModel )
{}

ControllerLockHelper::~ControllerLockHelper()
{}

void ControllerLockHelper::lockControllers()
{
    if( m_xModel.is())
        m_xModel->lockControllers();
}

void ControllerLockHelper::unlockControllers()
{
    if( m_xModel.is())
        m_xModel->unlockControllers();
}

// ================================================================================

ControllerLockHelperGuard::ControllerLockHelperGuard( ControllerLockHelper & rHelper ) :
        m_rHelper( rHelper )
{
    m_rHelper.lockControllers();
}

ControllerLockHelperGuard::~ControllerLockHelperGuard()
{
    m_rHelper.unlockControllers();
}

} //  namespace chart
