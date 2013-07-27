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

TARGET=cssui
PACKAGE=com$/sun$/star$/ui

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    ActionTrigger.idl                           \
    ActionTriggerContainer.idl                  \
    ActionTriggerSeparator.idl                  \
    ActionTriggerSeparatorType.idl              \
    ConfigurableUIElement.idl                   \
    ConfigurationEvent.idl                      \
    ContextChangeEventMultiplexer.idl			\
    ContextChangeEventObject.idl				\
    ContextMenuExecuteEvent.idl                 \
    ContextMenuInterceptorAction.idl            \
    DockingArea.idl                             \
    GlobalAcceleratorConfiguration.idl          \
    ImageType.idl                               \
    ItemDescriptor.idl                          \
    ItemStyle.idl                               \
    ItemType.idl                                \
    LayoutSize.idl								\
    ModuleUICategoryDescription.idl             \
    ModuleUICommandDescription.idl              \
    ModuleUIConfigurationManager.idl            \
    ModuleUIConfigurationManagerSupplier.idl    \
    ModuleWindowStateConfiguration.idl          \
    UICategoryDescription.idl                   \
    UICommandDescription.idl                    \
    UIConfigurationManager.idl                  \
    UIElement.idl                               \
    UIElementFactory.idl                        \
    UIElementFactoryManager.idl                 \
    UIElementSettings.idl                       \
    UIElementType.idl                           \
    WindowContentFactory.idl					\
    WindowStateConfiguration.idl                \
    XAcceleratorConfiguration.idl               \
    XContextMenuInterception.idl                \
    XContextMenuInterceptor.idl                 \
    XContextChangeEventListener.idl				\
    XContextChangeEventMultiplexer.idl			\
    XDockingAreaAcceptor.idl                    \
    XImageManager.idl                           \
    XModuleUIConfigurationManager.idl           \
    XModuleUIConfigurationManagerSupplier.idl   \
    XSidebar.idl								\
    XSidebarPanel.idl							\
    XStatusbarItem.idl                          \
    XUIConfiguration.idl                        \
    XUIConfigurationListener.idl                \
    XUIConfigurationManager.idl                 \
    XUIConfigurationManagerSupplier.idl         \
    XUIConfigurationPersistence.idl             \
    XUIConfigurationStorage.idl                 \
    XUIElement.idl                              \
    XUIElementFactory.idl                       \
    XUIElementFactoryRegistration.idl           \
    XUIElementSettings.idl                      \
    XUIFunctionListener.idl                     \
    XToolPanel.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
