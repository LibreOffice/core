#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.14 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

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
    ContextMenuExecuteEvent.idl                 \
    ContextMenuInterceptorAction.idl            \
    DockingArea.idl                             \
    GlobalAcceleratorConfiguration.idl          \
    ImageType.idl                               \
    ItemDescriptor.idl                          \
    ItemStyle.idl                               \
    ItemType.idl                                \
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
    WindowStateConfiguration.idl                \
    XAcceleratorConfiguration.idl               \
    XContextMenuInterception.idl                \
    XContextMenuInterceptor.idl                 \
    XDockingAreaAcceptor.idl                    \
    XImageManager.idl                           \
    XModuleUIConfigurationManager.idl           \
    XModuleUIConfigurationManagerSupplier.idl   \
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
    XUIFunctionListener.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
