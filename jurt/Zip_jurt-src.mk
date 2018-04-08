###############################################################
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
###############################################################



$(eval $(call gb_Zip_Zip,jurt_src,$(SRCDIR)/jurt/java/jurt/src/main/java))

$(eval $(call gb_Zip_add_files,jurt_src,\
	com/sun/star/comp/loader/RegistrationClassFinder.java \
	com/sun/star/comp/loader/FactoryHelper.java \
	com/sun/star/comp/loader/JavaLoaderFactory.java \
	com/sun/star/comp/loader/JavaLoader.java \
	com/sun/star/comp/bridgefactory/BridgeFactory.java \
	com/sun/star/comp/connections/ConstantInstanceProvider.java \
	com/sun/star/comp/connections/Implementation.java \
	com/sun/star/comp/connections/PipedConnection.java \
	com/sun/star/comp/connections/Connector.java \
	com/sun/star/comp/connections/Acceptor.java \
	com/sun/star/comp/urlresolver/UrlResolver.java \
	com/sun/star/comp/servicemanager/ServiceManager.java \
	com/sun/star/uno/MappingException.java \
	com/sun/star/uno/AsciiString.java \
	com/sun/star/uno/Ascii.java \
	com/sun/star/uno/WeakReference.java \
	com/sun/star/uno/AnyConverter.java \
	com/sun/star/lib/util/UrlToFileMapper.java \
	com/sun/star/lib/util/AsynchronousFinalizer.java \
	com/sun/star/lib/util/StringHelper.java \
	com/sun/star/lib/util/NativeLibraryLoader.java \
	com/sun/star/lib/connections/socket/socketAcceptor.java \
	com/sun/star/lib/connections/socket/SocketConnection.java \
	com/sun/star/lib/connections/socket/ConnectionDescriptor.java \
	com/sun/star/lib/connections/socket/socketConnector.java \
	com/sun/star/lib/connections/pipe/pipeAcceptor.java \
	com/sun/star/lib/connections/pipe/pipeConnector.java \
	com/sun/star/lib/connections/pipe/PipeConnection.java \
	com/sun/star/lib/uno/environments/remote/JavaThreadPoolFactory.java \
	com/sun/star/lib/uno/environments/remote/IProtocol.java \
	com/sun/star/lib/uno/environments/remote/Job.java \
	com/sun/star/lib/uno/environments/remote/IReceiver.java \
	com/sun/star/lib/uno/environments/remote/remote_environment.java \
	com/sun/star/lib/uno/environments/remote/IThreadPool.java \
	com/sun/star/lib/uno/environments/remote/Message.java \
	com/sun/star/lib/uno/environments/remote/JavaThreadPool.java \
	com/sun/star/lib/uno/environments/remote/JobQueue.java \
	com/sun/star/lib/uno/environments/remote/ThreadId.java \
	com/sun/star/lib/uno/environments/remote/ThreadPoolManager.java \
	com/sun/star/lib/uno/environments/remote/NativeThreadPool.java \
	com/sun/star/lib/uno/environments/java/java_environment.java \
	com/sun/star/lib/uno/bridges/java_remote/XConnectionOutputStream_Adapter.java \
	com/sun/star/lib/uno/bridges/java_remote/RequestHandler.java \
	com/sun/star/lib/uno/bridges/java_remote/XConnectionInputStream_Adapter.java \
	com/sun/star/lib/uno/bridges/java_remote/ProxyFactory.java \
	com/sun/star/lib/uno/bridges/java_remote/BridgedObject.java \
	com/sun/star/lib/uno/bridges/java_remote/java_remote_bridge.java \
	com/sun/star/lib/uno/Proxy.java \
	com/sun/star/lib/uno/protocols/urp/PendingRequests.java \
	com/sun/star/lib/uno/protocols/urp/UrpMessage.java \
	com/sun/star/lib/uno/protocols/urp/Cache.java \
	com/sun/star/lib/uno/protocols/urp/Unmarshal.java \
	com/sun/star/lib/uno/protocols/urp/Marshal.java \
	com/sun/star/lib/uno/protocols/urp/urp.java \
))

# vim: set noet sw=4 ts=4:
