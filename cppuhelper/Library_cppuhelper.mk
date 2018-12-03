#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



$(eval $(call gb_Library_Library,cppuhelper))

$(eval $(call gb_Library_add_package_headers,cppuhelper,cppuhelper_inc))

$(eval $(call gb_Library_add_precompiled_header,cppuhelper,$(SRCDIR)/formula/inc/pch/precompiled_cppuhelper))

ifeq ($(COMNAME),msci)
else ifeq ($(COMNAME),mscx)
else ifeq ($(COMNAME),sunpro5)
$(eval $(call gb_Library_set_versionmap,cppuhelper,$(SRCDIR)/cppuhelper/source/cc5_solaris_sparc.map))
else ifeq ($(GUI)$(COMNAME),OS2gcc3)
$(eval $(call gb_Library_set_versionmap,cppuhelper,$(SRCDIR)/cppuhelper/source/gcc3os2.map))
else
$(eval $(call gb_Library_set_versionmap,cppuhelper,$(SRCDIR)/cppuhelper/source/gcc3.map))
endif


$(eval $(call gb_Library_set_include,cppuhelper,\
	$$(INCLUDE) \
	-I$(SRCDIR)/cppuhelper/inc \
	-I$(SRCDIR)/cppuhelper/inc/pch \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_set_private_api,cppuhelper,$(OUTDIR)/bin/types.rdb,\
	$(SRCDIR)/cppuhelper/unotypes/cppuhelper/detail/XExceptionThrower.idl,\
	cppuhelper.detail.XExceptionThrower \
))

$(eval $(call gb_Library_set_private_extract_of_public_api,cppuhelper,$(OUTDIR)/bin/udkapi.rdb,\
                com.sun.star.beans.PropertyAttribute \
                com.sun.star.beans.PropertyValue \
                com.sun.star.beans.XFastPropertySet \
                com.sun.star.beans.XMultiPropertySet \
                com.sun.star.beans.XPropertyAccess \
                com.sun.star.beans.XPropertySet \
                com.sun.star.bridge.UnoUrlResolver \
                com.sun.star.bridge.XUnoUrlResolver \
                com.sun.star.connection.SocketPermission \
                com.sun.star.container.XContentEnumerationAccess \
                com.sun.star.container.XElementAccess \
                com.sun.star.container.XEnumerationAccess \
                com.sun.star.container.XHierarchicalNameAccess \
                com.sun.star.container.XNameAccess \
                com.sun.star.container.XNameReplace \
                com.sun.star.container.XNameContainer \
                com.sun.star.container.XSet \
                com.sun.star.io.FilePermission \
                com.sun.star.io.IOException \
                com.sun.star.lang.DisposedException \
                com.sun.star.lang.WrappedTargetRuntimeException \
                com.sun.star.lang.XComponent \
                com.sun.star.lang.XEventListener \
                com.sun.star.lang.XInitialization \
                com.sun.star.lang.XMultiComponentFactory \
                com.sun.star.lang.XMultiServiceFactory \
                com.sun.star.lang.XServiceInfo \
                com.sun.star.lang.XSingleComponentFactory \
                com.sun.star.lang.XSingleServiceFactory \
                com.sun.star.lang.XUnoTunnel \
                com.sun.star.lang.XTypeProvider \
                com.sun.star.loader.XImplementationLoader \
                com.sun.star.reflection.XArrayTypeDescription \
                com.sun.star.reflection.XCompoundTypeDescription \
                com.sun.star.reflection.XEnumTypeDescription \
                com.sun.star.reflection.XIdlClass \
                com.sun.star.reflection.XIdlClassProvider \
                com.sun.star.reflection.XIdlField2 \
                com.sun.star.reflection.XIdlReflection \
                com.sun.star.reflection.XIndirectTypeDescription \
                com.sun.star.reflection.XInterfaceAttributeTypeDescription \
                com.sun.star.reflection.XInterfaceAttributeTypeDescription2 \
                com.sun.star.reflection.XInterfaceMemberTypeDescription \
                com.sun.star.reflection.XInterfaceMethodTypeDescription \
                com.sun.star.reflection.XInterfaceTypeDescription2 \
                com.sun.star.reflection.XMethodParameter \
                com.sun.star.reflection.XStructTypeDescription \
                com.sun.star.reflection.XTypeDescription \
                com.sun.star.reflection.XUnionTypeDescription \
                com.sun.star.registry.XImplementationRegistration \
                com.sun.star.registry.XRegistryKey \
                com.sun.star.registry.XSimpleRegistry \
                com.sun.star.security.RuntimePermission \
                com.sun.star.security.XAccessController \
                com.sun.star.uno.DeploymentException \
                com.sun.star.uno.RuntimeException \
                com.sun.star.uno.XAggregation \
                com.sun.star.uno.XComponentContext \
                com.sun.star.uno.XCurrentContext \
                com.sun.star.uno.XUnloadingPreference \
                com.sun.star.uno.XWeak \
                com.sun.star.util.XMacroExpander \
))

$(eval $(call gb_Library_add_defs,cppuhelper,\
	-DCPPUHELPER_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,cppuhelper,\
	cppu \
	sal \
	salhelper \
	stl \
	$(gb_STDLIBS) \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_Library_add_linked_libs,cppuhelper,\
        advapi32 \
))
endif

$(eval $(call gb_Library_add_cobjects,cppuhelper,\
	cppuhelper/source/findsofficepath \
))

$(eval $(call gb_Library_add_exception_objects,cppuhelper,\
	cppuhelper/source/access_control \
	cppuhelper/source/bootstrap \
	cppuhelper/source/component \
	cppuhelper/source/component_context \
	cppuhelper/source/exc_thrower \
	cppuhelper/source/factory \
	cppuhelper/source/implbase \
	cppuhelper/source/implbase_ex \
	cppuhelper/source/implementationentry \
	cppuhelper/source/interfacecontainer \
	cppuhelper/source/macro_expander \
	cppuhelper/source/primeweak \
	cppuhelper/source/propertysetmixin \
	cppuhelper/source/propshlp \
	cppuhelper/source/servicefactory \
	cppuhelper/source/stdidlclass \
	cppuhelper/source/shlib \
	cppuhelper/source/tdmgr \
	cppuhelper/source/typeprovider \
	cppuhelper/source/unourl \
	cppuhelper/source/weak \
))

# vim: set noet sw=4 ts=4:

