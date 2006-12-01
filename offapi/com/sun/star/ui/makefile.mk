#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: rt $ $Date: 2006-12-01 17:18:21 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
