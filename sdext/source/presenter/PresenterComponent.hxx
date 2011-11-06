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



#ifndef SDEXT_PRESENTER_COMPONENT_HXX
#define SDEXT_PRESENTER_COMPONENT_HXX

#include <com/sun/star/uno/XComponentContext.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

/** This string is replaced automatically by the makefile during the
    building of this extension.
*/
class PresenterComponent
{
public:
    const static ::rtl::OUString gsExtensionIdentifier;

    static ::rtl::OUString GetBasePath (
        const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext);
    static ::rtl::OUString GetBasePath (
        const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
        const ::rtl::OUString& rsExtensionIdentifier);
};


} }

#endif
