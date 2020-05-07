# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Jar_Jar,libreoffice,org.libreoffice.uno))

$(eval $(call gb_Jar_use_customtargets,libreoffice,\
    ridljar/javamaker \
    unoil/javamaker \
))

$(eval $(call gb_Jar_use_jars,libreoffice, \
    unoloader \
))

$(eval $(call gb_Jar_set_packageroot,libreoffice,com))

$(eval $(call gb_Jar_set_manifest,libreoffice,$(SRCDIR)/ridljar/util/manifest))

$(eval $(call gb_Jar_add_manifest_classpath,libreoffice, \
    unoloader.jar \
    $(if $(filter MACOSX,$(OS)),../../Frameworks/,../) \
))

# ugly: the module-info.class is manually added here since it's not in "com" dir
$(eval $(call gb_Jar_add_packagedirs,libreoffice,\
    $(call gb_CustomTarget_get_workdir,ridljar/javamaker)/com \
    $(call gb_CustomTarget_get_workdir,unoil/javamaker)/com \
	$(call gb_JavaClassSet_get_classdir,$(call gb_Jar_get_classsetname,libreoffice))/module-info.class \
))

$(eval $(call gb_Jar_add_sourcefiles_java9,libreoffice,\
    ridljar/source/libreoffice/module-info \
))

$(eval $(call gb_Jar_add_sourcefiles,libreoffice,\
    ridljar/com/sun/star/comp/bridgefactory/BridgeFactory \
    ridljar/com/sun/star/comp/connections/Acceptor \
    ridljar/com/sun/star/comp/connections/Connector \
    ridljar/com/sun/star/comp/connections/ConstantInstanceProvider \
    ridljar/com/sun/star/comp/connections/Implementation \
    ridljar/com/sun/star/comp/connections/PipedConnection \
    ridljar/com/sun/star/comp/helper/Bootstrap \
    ridljar/com/sun/star/comp/helper/BootstrapException \
    ridljar/com/sun/star/comp/helper/ComponentContext \
    ridljar/com/sun/star/comp/helper/ComponentContextEntry \
    ridljar/com/sun/star/comp/helper/SharedLibraryLoader \
    ridljar/com/sun/star/comp/loader/FactoryHelper \
    ridljar/com/sun/star/comp/loader/JavaLoader \
    ridljar/com/sun/star/comp/loader/JavaLoaderFactory \
    ridljar/com/sun/star/comp/loader/RegistrationClassFinder \
    ridljar/com/sun/star/comp/servicemanager/ServiceManager \
    ridljar/com/sun/star/comp/urlresolver/UrlResolver \
    ridljar/com/sun/star/lib/connections/pipe/PipeConnection \
    ridljar/com/sun/star/lib/connections/pipe/pipeAcceptor \
    ridljar/com/sun/star/lib/connections/pipe/pipeConnector \
    ridljar/com/sun/star/lib/connections/socket/ConnectionDescriptor \
    ridljar/com/sun/star/lib/connections/socket/SocketConnection \
    ridljar/com/sun/star/lib/connections/socket/socketAcceptor \
    ridljar/com/sun/star/lib/connections/socket/socketConnector \
    ridljar/com/sun/star/lib/uno/Proxy \
    ridljar/com/sun/star/lib/uno/adapter/ByteArrayToXInputStreamAdapter \
    ridljar/com/sun/star/lib/uno/adapter/InputStreamToXInputStreamAdapter \
    ridljar/com/sun/star/lib/uno/adapter/OutputStreamToXOutputStreamAdapter \
    ridljar/com/sun/star/lib/uno/adapter/XInputStreamToInputStreamAdapter \
    ridljar/com/sun/star/lib/uno/adapter/XOutputStreamToByteArrayAdapter \
    ridljar/com/sun/star/lib/uno/adapter/XOutputStreamToOutputStreamAdapter \
    ridljar/com/sun/star/lib/uno/bridges/java_remote/BridgedObject \
    ridljar/com/sun/star/lib/uno/bridges/java_remote/ProxyFactory \
    ridljar/com/sun/star/lib/uno/bridges/java_remote/RequestHandler \
    ridljar/com/sun/star/lib/uno/bridges/java_remote/XConnectionInputStream_Adapter \
    ridljar/com/sun/star/lib/uno/bridges/java_remote/XConnectionOutputStream_Adapter \
    ridljar/com/sun/star/lib/uno/bridges/java_remote/java_remote_bridge \
    ridljar/com/sun/star/lib/uno/environments/java/java_environment \
    ridljar/com/sun/star/lib/uno/environments/remote/IProtocol \
    ridljar/com/sun/star/lib/uno/environments/remote/IReceiver \
    ridljar/com/sun/star/lib/uno/environments/remote/IThreadPool \
    ridljar/com/sun/star/lib/uno/environments/remote/JavaThreadPool \
    ridljar/com/sun/star/lib/uno/environments/remote/JavaThreadPoolFactory \
    ridljar/com/sun/star/lib/uno/environments/remote/Job \
    ridljar/com/sun/star/lib/uno/environments/remote/JobQueue \
    ridljar/com/sun/star/lib/uno/environments/remote/Message \
    ridljar/com/sun/star/lib/uno/environments/remote/NativeThreadPool \
    ridljar/com/sun/star/lib/uno/environments/remote/ThreadId \
    ridljar/com/sun/star/lib/uno/environments/remote/ThreadPoolManager \
    ridljar/com/sun/star/lib/uno/environments/remote/remote_environment \
    ridljar/com/sun/star/lib/uno/helper/ComponentBase \
    ridljar/com/sun/star/lib/uno/helper/Factory \
    ridljar/com/sun/star/lib/uno/helper/InterfaceContainer \
    ridljar/com/sun/star/lib/uno/helper/MultiTypeInterfaceContainer \
    ridljar/com/sun/star/lib/uno/helper/PropertySet \
    ridljar/com/sun/star/lib/uno/helper/PropertySetMixin \
    ridljar/com/sun/star/lib/uno/helper/UnoUrl \
    ridljar/com/sun/star/lib/uno/helper/WeakAdapter \
    ridljar/com/sun/star/lib/uno/helper/WeakBase \
    ridljar/com/sun/star/lib/uno/protocols/urp/Cache \
    ridljar/com/sun/star/lib/uno/protocols/urp/Marshal \
    ridljar/com/sun/star/lib/uno/protocols/urp/PendingRequests \
    ridljar/com/sun/star/lib/uno/protocols/urp/Unmarshal \
    ridljar/com/sun/star/lib/uno/protocols/urp/UrpMessage \
    ridljar/com/sun/star/lib/uno/protocols/urp/urp \
    ridljar/com/sun/star/lib/uno/typedesc/FieldDescription \
    ridljar/com/sun/star/lib/uno/typedesc/MemberDescriptionHelper \
    ridljar/com/sun/star/lib/uno/typedesc/MethodDescription \
    ridljar/com/sun/star/lib/uno/typedesc/TypeDescription \
    ridljar/com/sun/star/lib/uno/typeinfo/AttributeTypeInfo \
    ridljar/com/sun/star/lib/uno/typeinfo/ConstantTypeInfo \
    ridljar/com/sun/star/lib/uno/typeinfo/MemberTypeInfo \
    ridljar/com/sun/star/lib/uno/typeinfo/MethodTypeInfo \
    ridljar/com/sun/star/lib/uno/typeinfo/ParameterTypeInfo \
    ridljar/com/sun/star/lib/uno/typeinfo/TypeInfo \
    ridljar/com/sun/star/lib/util/AsynchronousFinalizer \
    ridljar/com/sun/star/lib/util/DisposeListener \
    ridljar/com/sun/star/lib/util/DisposeNotifier \
    ridljar/com/sun/star/lib/util/NativeLibraryLoader \
    ridljar/com/sun/star/lib/util/StringHelper \
    ridljar/com/sun/star/lib/util/UrlToFileMapper \
    ridljar/com/sun/star/lib/util/WeakMap \
    ridljar/com/sun/star/uno/Any \
    ridljar/com/sun/star/uno/AnyConverter \
    ridljar/com/sun/star/uno/Ascii \
    ridljar/com/sun/star/uno/AsciiString \
    ridljar/com/sun/star/uno/Enum \
    ridljar/com/sun/star/uno/IBridge \
    ridljar/com/sun/star/uno/IEnvironment \
    ridljar/com/sun/star/uno/IMapping \
    ridljar/com/sun/star/uno/IQueryInterface \
    ridljar/com/sun/star/uno/MappingException \
    ridljar/com/sun/star/uno/Type \
    ridljar/com/sun/star/uno/UnoRuntime \
    ridljar/com/sun/star/uno/WeakReference \
))

# vim:set noet sw=4 ts=4:
