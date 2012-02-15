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

#include "ConfigurationTracer.hxx"

#include <cstdio>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sd { namespace framework {

void ConfigurationTracer::TraceConfiguration (
    const Reference<XConfiguration>& rxConfiguration,
    const char* pMessage)
{
#ifdef DEBUG
    OSL_TRACE("%s at %p {", pMessage, rxConfiguration.get());
    if (rxConfiguration.is())
    {
        TraceBoundResources(rxConfiguration, NULL, 0);
    }
    else
    {
        OSL_TRACE("    empty");
    }
    OSL_TRACE("}");
#else
    (void)rxConfiguration;
    (void)pMessage;
#endif
}




#ifdef DEBUG
void ConfigurationTracer::TraceBoundResources (
    const Reference<XConfiguration>& rxConfiguration,
    const Reference<XResourceId>& rxResourceId,
    const int nIndentation)
{
    Sequence<Reference<XResourceId> > aResourceList (
        rxConfiguration->getResources(rxResourceId, ::rtl::OUString(), AnchorBindingMode_DIRECT));
    const ::rtl::OUString sIndentation (::rtl::OUString::createFromAscii("    "));
    for (sal_Int32 nIndex=0; nIndex<aResourceList.getLength(); ++nIndex)
    {
        ::rtl::OUString sLine (aResourceList[nIndex]->getResourceURL());
        for (int i=0; i<nIndentation; ++i)
            sLine = sIndentation + sLine;
        OSL_TRACE("%s", OUStringToOString(sLine, RTL_TEXTENCODING_UTF8).getStr());
        TraceBoundResources(rxConfiguration, aResourceList[nIndex], nIndentation+1);
    }
}
#endif

} } // end of namespace sd::framework
