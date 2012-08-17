# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Jar_Jar,jurt))

$(eval $(call gb_Jar_use_jars,jurt,\
    ridl \
    unoloader \
))

$(eval $(call gb_Jar_set_packageroot,jurt,com))

$(eval $(call gb_Jar_set_manifest,jurt,$(SRCDIR)/jurt/util/manifest))

$(eval $(call gb_Jar_add_manifest_classpath,jurt, \
    ridl.jar unoloader.jar ../../lib/ ../bin/))

$(eval $(call gb_Jar_add_sourcefiles,jurt,\
    jurt/com/sun/star/comp/bridgefactory/BridgeFactory \
    jurt/com/sun/star/comp/connections/Acceptor \
    jurt/com/sun/star/comp/connections/Connector \
    jurt/com/sun/star/comp/connections/ConstantInstanceProvider \
    jurt/com/sun/star/comp/connections/Implementation \
    jurt/com/sun/star/comp/connections/PipedConnection \
    jurt/com/sun/star/comp/loader/FactoryHelper \
    jurt/com/sun/star/comp/loader/JavaLoader \
    jurt/com/sun/star/comp/loader/JavaLoaderFactory \
    jurt/com/sun/star/comp/loader/RegistrationClassFinder \
    jurt/com/sun/star/comp/servicemanager/ServiceManager \
    jurt/com/sun/star/comp/urlresolver/UrlResolver \
    jurt/com/sun/star/lib/connections/pipe/PipeConnection \
    jurt/com/sun/star/lib/connections/pipe/pipeAcceptor \
    jurt/com/sun/star/lib/connections/pipe/pipeConnector \
    jurt/com/sun/star/lib/connections/socket/ConnectionDescriptor \
    jurt/com/sun/star/lib/connections/socket/SocketConnection \
    jurt/com/sun/star/lib/connections/socket/socketAcceptor \
    jurt/com/sun/star/lib/connections/socket/socketConnector \
    jurt/com/sun/star/lib/uno/Proxy \
    jurt/com/sun/star/lib/uno/bridges/java_remote/BridgedObject \
    jurt/com/sun/star/lib/uno/bridges/java_remote/ProxyFactory \
    jurt/com/sun/star/lib/uno/bridges/java_remote/RequestHandler \
    jurt/com/sun/star/lib/uno/bridges/java_remote/XConnectionInputStream_Adapter \
    jurt/com/sun/star/lib/uno/bridges/java_remote/XConnectionOutputStream_Adapter \
    jurt/com/sun/star/lib/uno/bridges/java_remote/java_remote_bridge \
    jurt/com/sun/star/lib/uno/environments/java/java_environment \
    jurt/com/sun/star/lib/uno/environments/remote/IProtocol \
    jurt/com/sun/star/lib/uno/environments/remote/IReceiver \
    jurt/com/sun/star/lib/uno/environments/remote/IThreadPool \
    jurt/com/sun/star/lib/uno/environments/remote/JavaThreadPool \
    jurt/com/sun/star/lib/uno/environments/remote/JavaThreadPoolFactory \
    jurt/com/sun/star/lib/uno/environments/remote/Job \
    jurt/com/sun/star/lib/uno/environments/remote/JobQueue \
    jurt/com/sun/star/lib/uno/environments/remote/Message \
    jurt/com/sun/star/lib/uno/environments/remote/NativeThreadPool \
    jurt/com/sun/star/lib/uno/environments/remote/ThreadId \
    jurt/com/sun/star/lib/uno/environments/remote/ThreadPoolManager \
    jurt/com/sun/star/lib/uno/environments/remote/remote_environment \
    jurt/com/sun/star/lib/uno/protocols/urp/Cache \
    jurt/com/sun/star/lib/uno/protocols/urp/Marshal \
    jurt/com/sun/star/lib/uno/protocols/urp/PendingRequests \
    jurt/com/sun/star/lib/uno/protocols/urp/Unmarshal \
    jurt/com/sun/star/lib/uno/protocols/urp/UrpMessage \
    jurt/com/sun/star/lib/uno/protocols/urp/urp \
    jurt/com/sun/star/lib/util/AsynchronousFinalizer \
    jurt/com/sun/star/lib/util/NativeLibraryLoader \
    jurt/com/sun/star/lib/util/StringHelper \
    jurt/com/sun/star/lib/util/UrlToFileMapper \
    jurt/com/sun/star/uno/AnyConverter \
    jurt/com/sun/star/uno/Ascii \
    jurt/com/sun/star/uno/AsciiString \
    jurt/com/sun/star/uno/MappingException \
    jurt/com/sun/star/uno/WeakReference \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
