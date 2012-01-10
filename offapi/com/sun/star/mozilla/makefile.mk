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



PRJ=..$/..$/..$/..

PRJNAME=offapi

TARGET=cssmozilla
PACKAGE=com$/sun$/star$/mozilla

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    MenuMultipleChange.idl \
    MenuSingleChange.idl \
    MenuProxy.idl \
    MenuProxyListener.idl \
    XMenuProxy.idl \
    XMenuProxyListener.idl \
    XPluginInstance.idl \
    XPluginInstanceNotifySink.idl \
    XPluginInstancePeer.idl \
    XPluginInstanceSyncPeer.idl \
    XPluginWindowPeer.idl \
    XCloseSessionListener.idl \
    XRemoteServiceManagerProvider.idl	\
    MozillaBootstrap.idl \
    MozillaProductType.idl	\
    XMozillaBootstrap.idl \
    XProfileDiscover.idl	\
    XProfileManager.idl	\
    XProxyRunner.idl	\
    XCodeProxy.idl



# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
