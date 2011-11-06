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



#ifndef EXTENSIONS_LOGGERCONFIG_HXX
#define EXTENSIONS_LOGGERCONFIG_HXX

/** === begin UNO includes === **/
#include <com/sun/star/logging/XLogger.hpp>
/** === end UNO includes === **/

namespace comphelper
{
    class ComponentContext;
}

//........................................................................
namespace logging
{
//........................................................................

    /** initializes the given logger from the configuration

        The configuration node /org.openoffice.Office.Logging/Settings/<logger_name>
        is examined for this. If it does not yet exist, it will be created.

        The function creates a default handler and a default formatter, as specified in the
        configuration.

        This function is currently external to the logger instance. Perhaps it can, on the long
        run, be moved to the logger implementation - not sure if it's the best place.
    */
    void    initializeLoggerFromConfiguration(
                const ::comphelper::ComponentContext& _rContext,
                const ::com::sun::star::uno::Reference< ::com::sun::star::logging::XLogger >& _rxLogger
            );

//........................................................................
} // namespace logging
//........................................................................

#endif // EXTENSIONS_LOGGERCONFIG_HXX
