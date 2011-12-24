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

$(eval $(call gb_Zip_Zip,jurt_src,$(SRCDIR)/jurt))

$(eval $(call gb_Zip_add_files,jurt_src,\
    com/sun/star/comp/bridgefactory/BridgeFactory.java \
    com/sun/star/comp/connections/Acceptor.java \
    com/sun/star/comp/connections/Connector.java \
    com/sun/star/comp/connections/ConstantInstanceProvider.java \
    com/sun/star/comp/connections/Implementation.java \
    com/sun/star/comp/connections/PipedConnection.java \
    com/sun/star/comp/loader/FactoryHelper.java \
    com/sun/star/comp/loader/JavaLoader.java \
    com/sun/star/comp/loader/JavaLoaderFactory.java \
    com/sun/star/comp/loader/RegistrationClassFinder.java \
    com/sun/star/comp/servicemanager/ServiceManager.java \
    com/sun/star/comp/urlresolver/UrlResolver.java \
    com/sun/star/lib/connections/pipe/PipeConnection.java \
    com/sun/star/lib/connections/pipe/pipeAcceptor.java \
    com/sun/star/lib/connections/pipe/pipeConnector.java \
    com/sun/star/lib/connections/socket/ConnectionDescriptor.java \
    com/sun/star/lib/connections/socket/SocketConnection.java \
    com/sun/star/lib/connections/socket/socketAcceptor.java \
    com/sun/star/lib/connections/socket/socketConnector.java \
    com/sun/star/lib/uno/Proxy.java \
    com/sun/star/lib/uno/bridges/java_remote/BridgedObject.java \
    com/sun/star/lib/uno/bridges/java_remote/ProxyFactory.java \
    com/sun/star/lib/uno/bridges/java_remote/RequestHandler.java \
    com/sun/star/lib/uno/bridges/java_remote/XConnectionInputStream_Adapter.java \
    com/sun/star/lib/uno/bridges/java_remote/XConnectionOutputStream_Adapter.java \
    com/sun/star/lib/uno/bridges/java_remote/java_remote_bridge.java \
    com/sun/star/lib/uno/environments/java/java_environment.java \
    com/sun/star/lib/uno/environments/remote/IProtocol.java \
    com/sun/star/lib/uno/environments/remote/IReceiver.java \
    com/sun/star/lib/uno/environments/remote/IThreadPool.java \
    com/sun/star/lib/uno/environments/remote/JavaThreadPool.java \
    com/sun/star/lib/uno/environments/remote/JavaThreadPoolFactory.java \
    com/sun/star/lib/uno/environments/remote/Job.java \
    com/sun/star/lib/uno/environments/remote/JobQueue.java \
    com/sun/star/lib/uno/environments/remote/Message.java \
    com/sun/star/lib/uno/environments/remote/NativeThreadPool.java \
    com/sun/star/lib/uno/environments/remote/ThreadId.java \
    com/sun/star/lib/uno/environments/remote/ThreadPoolManager.java \
    com/sun/star/lib/uno/environments/remote/remote_environment.java \
    com/sun/star/lib/uno/protocols/urp/Cache.java \
    com/sun/star/lib/uno/protocols/urp/Marshal.java \
    com/sun/star/lib/uno/protocols/urp/PendingRequests.java \
    com/sun/star/lib/uno/protocols/urp/Unmarshal.java \
    com/sun/star/lib/uno/protocols/urp/UrpMessage.java \
    com/sun/star/lib/uno/protocols/urp/urp.java \
    com/sun/star/lib/util/AsynchronousFinalizer.java \
    com/sun/star/lib/util/NativeLibraryLoader.java \
    com/sun/star/lib/util/StringHelper.java \
    com/sun/star/lib/util/UrlToFileMapper.java \
    com/sun/star/uno/AnyConverter.java \
    com/sun/star/uno/Ascii.java \
    com/sun/star/uno/AsciiString.java \
    com/sun/star/uno/MappingException.java \
    com/sun/star/uno/WeakReference.java \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
