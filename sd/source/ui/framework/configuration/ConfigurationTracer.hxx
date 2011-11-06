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



#ifndef SD_FRAMEWORK_CONFIGURATION_TRACER_HXX
#define SD_FRAMEWORK_CONFIGURATION_TRACER_HXX

#include <com/sun/star/drawing/framework/XConfiguration.hpp>

namespace sd { namespace framework {

/** Print debug information about configurations to the standard error
    output channel.
*/
class ConfigurationTracer
{
public:
    static void TraceConfiguration (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XConfiguration>& rxConfiguration,
        const char* pMessage);
#ifdef DEBUG
    static void TraceBoundResources (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XConfiguration>& rxConfiguration,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XResourceId>& rxResourceId,
        const int nIndentation);
#endif
};

} } // end of namespace sd::framework

#endif
