/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

// extern "C" component_writeInfo()
_COMPHELPER_COMPONENT_WRITEINFO
(
    _COMPHELPER_COMPONENTINFO( TypeDetection                                 ,
                               TypeDetection::impl_getImplementationName()   ,
                               TypeDetection::impl_getSupportedServiceNames())

    _COMPHELPER_COMPONENTINFO( FilterFactory                                 ,
                               FilterFactory::impl_getImplementationName()   ,
                               FilterFactory::impl_getSupportedServiceNames())

    _COMPHELPER_COMPONENTINFO( ContentHandlerFactory                                 ,
                               ContentHandlerFactory::impl_getImplementationName()   ,
                               ContentHandlerFactory::impl_getSupportedServiceNames())

    _COMPHELPER_COMPONENTINFO( FrameLoaderFactory                                 ,
                               FrameLoaderFactory::impl_getImplementationName()   ,
                               FrameLoaderFactory::impl_getSupportedServiceNames())

    _COMPHELPER_COMPONENTINFO( ConfigFlush                                 ,
                               ConfigFlush::impl_getImplementationName()   ,
                               ConfigFlush::impl_getSupportedServiceNames())
)

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
