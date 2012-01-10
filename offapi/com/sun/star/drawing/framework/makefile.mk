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



PRJ=..$/..$/..$/..$/..

PRJNAME=api

TARGET=cssdframework
PACKAGE=com$/sun$/star$/drawing$/framework

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=									\
    AnchorBindingMode.idl					\
    BasicPaneFactory.idl					\
    BasicToolBarFactory.idl					\
    BasicViewFactory.idl					\
    Configuration.idl						\
    ConfigurationChangeEvent.idl			\
    ConfigurationController.idl				\
    ModuleController.idl					\
    ResourceActivationMode.idl				\
    ResourceId.idl							\
    TabBarButton.idl						\
    XConfiguration.idl						\
    XConfigurationChangeListener.idl		\
    XConfigurationChangeRequest.idl			\
    XConfigurationController.idl			\
    XConfigurationControllerBroadcaster.idl	\
    XConfigurationControllerRequestQueue.idl\
    XControllerManager.idl					\
    XModuleController.idl					\
    XPane.idl								\
    XPane2.idl								\
    XPaneBorderPainter.idl					\
    XRelocatableResource.idl				\
    XResource.idl							\
    XResourceId.idl							\
    XResourceFactory.idl					\
    XResourceFactoryManager.idl				\
    XTabBar.idl								\
    XToolBar.idl							\
    XView.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
