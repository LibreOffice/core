/*************************************************************************
 *
 *  $RCSfile: registration.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-20 09:29:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "registration.hxx"

//_______________________________________________
// includes

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
