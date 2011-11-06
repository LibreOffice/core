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



#include "precompiled_sd.hxx"

#include "GenericConfigurationChangeRequest.hxx"

#include "framework/FrameworkHelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;

namespace sd { namespace framework {

GenericConfigurationChangeRequest::GenericConfigurationChangeRequest (
    const Reference<XResourceId>& rxResourceId,
    const Mode eMode) throw(::com::sun::star::lang::IllegalArgumentException)
    : GenericConfigurationChangeRequestInterfaceBase(MutexOwner::maMutex),
      mxResourceId(rxResourceId),
      meMode(eMode)
{
    if ( ! rxResourceId.is() || rxResourceId->getResourceURL().getLength()==0)
        throw ::com::sun::star::lang::IllegalArgumentException();
}




GenericConfigurationChangeRequest::~GenericConfigurationChangeRequest (void) throw()
{
}




void SAL_CALL GenericConfigurationChangeRequest::execute (
    const Reference<XConfiguration>& rxConfiguration)
    throw (RuntimeException)
{
    if (rxConfiguration.is())
    {
        switch (meMode)
        {
            case Activation:
                rxConfiguration->addResource(mxResourceId);
                break;

            case Deactivation:
                rxConfiguration->removeResource(mxResourceId);
                break;
        }
    }
}




OUString SAL_CALL GenericConfigurationChangeRequest::getName (void)
    throw (RuntimeException)
{
    return OUString::createFromAscii("GenericConfigurationChangeRequest ")
        + OUString::createFromAscii(meMode==Activation ? "activate " : "deactivate ")
        + FrameworkHelper::ResourceIdToString(mxResourceId);
}




void SAL_CALL GenericConfigurationChangeRequest::setName (const OUString& rsName)
    throw (RuntimeException)
{
    (void)rsName;
    // Ignored.
}

} } // end of namespace sd::framework

