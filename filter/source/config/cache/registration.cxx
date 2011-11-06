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
#include "precompiled_filter.hxx"

#include "registration.hxx"

//_______________________________________________
// includes

#ifdef _FILTER_CONFIG_CONSTANT_HXX_
#  error "Already included constant.hxx"
#else
#  define PROPNAME_IMPL_DECL
#  include "constant.hxx"
#endif
#include <stdio.h>
#include "typedetection.hxx"
#include "filterfactory.hxx"
#include "contenthandlerfactory.hxx"
#include "frameloaderfactory.hxx"
#include "configflush.hxx"

//_______________________________________________
// namespace

namespace filter{
    namespace config{

namespace css = ::com::sun::star;

//_______________________________________________
// definitions

rtl::OUString pFilterStrings[19];
static bool bInitialized = false;

static void InitConstants()
{
    if (!bInitialized)
    {
        ::osl::MutexGuard aGuard(::osl::Mutex::getGlobalMutex());
        if (!bInitialized)
        {
            PROPNAME_NAME;
            PROPNAME_UINAME;
            PROPNAME_UINAMES;
            PROPNAME_PREFERRED;
            PROPNAME_PREFERREDFILTER;
            PROPNAME_DETECTSERVICE;
            PROPNAME_MEDIATYPE;
            PROPNAME_CLIPBOARDFORMAT;
            PROPNAME_URLPATTERN;
            PROPNAME_EXTENSIONS;
            PROPNAME_TYPE;
            PROPNAME_DOCUMENTSERVICE;
            PROPNAME_FILTERSERVICE;
            PROPNAME_UICOMPONENT;
            PROPNAME_FLAGS;
            PROPNAME_USERDATA;
            PROPNAME_TEMPLATENAME;
            PROPNAME_FILEFORMATVERSION;
            PROPNAME_TYPES;
            bInitialized = true;
        }
    }
}

// extern "C" component_getImplementationEnvironment()
_COMPHELPER_COMPONENT_GETIMPLEMENTATIONENVIRONMENT

// extern "C" component_getFactory()
_COMPHELPER_COMPONENT_GETFACTORY
(
    { InitConstants(); },
    _COMPHELPER_MULTIINSTANCEFACTORY( TypeDetection::impl_getImplementationName()   ,
                                      TypeDetection::impl_getSupportedServiceNames(),
                                      TypeDetection::impl_createInstance            )

    _COMPHELPER_MULTIINSTANCEFACTORY( FilterFactory::impl_getImplementationName()   ,
                                      FilterFactory::impl_getSupportedServiceNames(),
                                      FilterFactory::impl_createInstance            )

    _COMPHELPER_MULTIINSTANCEFACTORY( ContentHandlerFactory::impl_getImplementationName()   ,
                                      ContentHandlerFactory::impl_getSupportedServiceNames(),
                                      ContentHandlerFactory::impl_createInstance            )

    _COMPHELPER_MULTIINSTANCEFACTORY( FrameLoaderFactory::impl_getImplementationName()   ,
                                      FrameLoaderFactory::impl_getSupportedServiceNames(),
                                      FrameLoaderFactory::impl_createInstance            )

    _COMPHELPER_ONEINSTANCEFACTORY( ConfigFlush::impl_getImplementationName()   ,
                                    ConfigFlush::impl_getSupportedServiceNames(),
                                    ConfigFlush::impl_createInstance            )
)

    } // namespace config
} // namespace filter
