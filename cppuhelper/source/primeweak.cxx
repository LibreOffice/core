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
#include "precompiled_cppuhelper.hxx"

// The only purpose of this file is to workaround a problem in UNO:
// Cppumaker and its brethren emit different implementations for
// the same symbol by design (!) which is quite... unfortunate
// and can confuse the heck out of dynamic linkers, debuggers, etc.
//
// For bootstrapping the comprehensive descriptions of some UNO types are needed.
// The mechanism in this file makes sure that these comprehensive type
// descriptions are used to prime the so-called "WeakMap".
//
// TODO: change cppumaker and its brethren to use different symbol names
//       for different implementations. In particular "cppu_detail_getUnoType"
//       should be divided into full, weak and mini implementations.

// Types that are candidates for this special workaround are the ones mentioned
// by the exceptions thrown from implbase_ex.cxx's __queryDeepNoXInterface()
// that also need to added to the makefile's UNOTYPES define

#define cppu_detail_getUnoType cppu_full_getUnoType
#define InitTypeDesc(T) {(void)cppu_full_getUnoType(static_cast< T * >(NULL));}

// NOTE: the cppuhelper part in the include paths below ensures that the type descriptions
// are comprehensive descriptions and not some weak descriptions e.g. from solver
#include "cppuhelper/com/sun/star/lang/XComponent.hpp"
#include "cppuhelper/com/sun/star/lang/XInitialization.hpp"
#include "cppuhelper/com/sun/star/lang/XSingleServiceFactory.hpp"
#include "cppuhelper/com/sun/star/lang/XSingleComponentFactory.hpp"
#include "cppuhelper/com/sun/star/lang/XMultiServiceFactory.hpp"
#include "cppuhelper/com/sun/star/lang/XMultiComponentFactory.hpp"
#include "cppuhelper/com/sun/star/lang/XServiceInfo.hpp"
#include "cppuhelper/com/sun/star/lang/XEventListener.hpp"
#include "cppuhelper/com/sun/star/lang/XTypeProvider.hpp"
#include "cppuhelper/com/sun/star/lang/XUnoTunnel.hpp"
#include "cppuhelper/com/sun/star/lang/DisposedException.hpp"
#include "cppuhelper/com/sun/star/uno/DeploymentException.hpp"
#include "cppuhelper/com/sun/star/uno/XWeak.hpp"
#include "cppuhelper/com/sun/star/uno/XCurrentContext.hpp"
#include "cppuhelper/com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/com/sun/star/uno/RuntimeException.hpp"
#include "cppuhelper/com/sun/star/util/XMacroExpander.hpp"
#include "cppuhelper/com/sun/star/beans/PropertyValue.hpp"
#include "cppuhelper/com/sun/star/beans/XPropertySet.hpp"
#include "cppuhelper/com/sun/star/beans/XMultiPropertySet.hpp"
#include "cppuhelper/com/sun/star/container/XContentEnumerationAccess.hpp"
#include "cppuhelper/com/sun/star/container/XEnumerationAccess.hpp"
#include "cppuhelper/com/sun/star/container/XEnumeration.hpp"
#include "cppuhelper/com/sun/star/container/XNameAccess.hpp"
#include "cppuhelper/com/sun/star/container/XNameReplace.hpp"
#include "cppuhelper/com/sun/star/container/XNameContainer.hpp"
#include "cppuhelper/com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "cppuhelper/com/sun/star/container/XSet.hpp"
#include "cppuhelper/com/sun/star/bridge/XUnoUrlResolver.hpp"
#include "cppuhelper/com/sun/star/io/IOException.hpp"
#include "cppuhelper/com/sun/star/io/FilePermission.hpp"
#include "cppuhelper/com/sun/star/security/RuntimePermission.hpp"
#include "cppuhelper/com/sun/star/loader/XImplementationLoader.hpp"
#include "cppuhelper/com/sun/star/security/XAccessController.hpp"
#include "cppuhelper/com/sun/star/registry/XRegistryKey.hpp"
#include "cppuhelper/com/sun/star/registry/XSimpleRegistry.hpp"
#include "cppuhelper/com/sun/star/reflection/XTypeDescription.hpp"
#include "cppuhelper/com/sun/star/reflection/XEnumTypeDescription.hpp"
#include "cppuhelper/com/sun/star/reflection/XArrayTypeDescription.hpp"
#include "cppuhelper/com/sun/star/reflection/XStructTypeDescription.hpp"
#include "cppuhelper/com/sun/star/reflection/XUnionTypeDescription.hpp"
#include "cppuhelper/com/sun/star/reflection/XCompoundTypeDescription.hpp"
#include "cppuhelper/com/sun/star/reflection/XIndirectTypeDescription.hpp"
#include "cppuhelper/com/sun/star/reflection/XInterfaceTypeDescription.hpp"
#include "cppuhelper/com/sun/star/reflection/XInterfaceMemberTypeDescription.hpp"
#include "cppuhelper/com/sun/star/reflection/XInterfaceMethodTypeDescription.hpp"
#include "cppuhelper/com/sun/star/reflection/XMethodParameter.hpp"

void primeWeakMap( void)
{
    InitTypeDesc( com::sun::star::lang::XComponent );
    InitTypeDesc( com::sun::star::lang::XInitialization );
    InitTypeDesc( com::sun::star::lang::XSingleServiceFactory );
    InitTypeDesc( com::sun::star::lang::XSingleComponentFactory );
    InitTypeDesc( com::sun::star::lang::XMultiServiceFactory );
    InitTypeDesc( com::sun::star::lang::XMultiComponentFactory );
    InitTypeDesc( com::sun::star::lang::XServiceInfo );
    InitTypeDesc( com::sun::star::lang::XEventListener );
    InitTypeDesc( com::sun::star::lang::XTypeProvider );
    InitTypeDesc( com::sun::star::lang::XUnoTunnel );
    InitTypeDesc( com::sun::star::lang::DisposedException );
    InitTypeDesc( com::sun::star::uno::XWeak );
    InitTypeDesc( com::sun::star::uno::DeploymentException );
    InitTypeDesc( com::sun::star::uno::XCurrentContext );
    InitTypeDesc( com::sun::star::uno::XComponentContext );
    InitTypeDesc( com::sun::star::uno::RuntimeException );
    InitTypeDesc( com::sun::star::util::XMacroExpander );
    InitTypeDesc( com::sun::star::beans::PropertyState );
    InitTypeDesc( com::sun::star::beans::PropertyValue );
    InitTypeDesc( com::sun::star::beans::XPropertySet );
    InitTypeDesc( com::sun::star::beans::XMultiPropertySet );
    InitTypeDesc( com::sun::star::container::XElementAccess );
    InitTypeDesc( com::sun::star::container::XEnumeration );
    InitTypeDesc( com::sun::star::container::XEnumerationAccess );
    InitTypeDesc( com::sun::star::container::XContentEnumerationAccess );
    InitTypeDesc( com::sun::star::container::XHierarchicalNameAccess );
    InitTypeDesc( com::sun::star::container::XNameAccess );
    InitTypeDesc( com::sun::star::container::XNameReplace );
    InitTypeDesc( com::sun::star::container::XNameContainer );
    InitTypeDesc( com::sun::star::container::XSet );
    InitTypeDesc( com::sun::star::io::IOException );
    InitTypeDesc( com::sun::star::io::FilePermission );
    InitTypeDesc( com::sun::star::security::XAccessController );
    InitTypeDesc( com::sun::star::security::RuntimePermission);
    InitTypeDesc( com::sun::star::loader::XImplementationLoader );
    InitTypeDesc( com::sun::star::bridge::XUnoUrlResolver );
    InitTypeDesc( com::sun::star::registry::XRegistryKey );
    InitTypeDesc( com::sun::star::registry::XSimpleRegistry );
    InitTypeDesc( com::sun::star::reflection::XTypeDescription );
    InitTypeDesc( com::sun::star::reflection::XEnumTypeDescription );
    InitTypeDesc( com::sun::star::reflection::XArrayTypeDescription );
    InitTypeDesc( com::sun::star::reflection::XStructTypeDescription );
    InitTypeDesc( com::sun::star::reflection::XUnionTypeDescription );
    InitTypeDesc( com::sun::star::reflection::XCompoundTypeDescription );
    InitTypeDesc( com::sun::star::reflection::XIndirectTypeDescription );
    InitTypeDesc( com::sun::star::reflection::XInterfaceTypeDescription );
    InitTypeDesc( com::sun::star::reflection::XInterfaceMemberTypeDescription );
    InitTypeDesc( com::sun::star::reflection::XInterfaceMethodTypeDescription );
    InitTypeDesc( com::sun::star::reflection::XMethodParameter );
}

