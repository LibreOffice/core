#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

PRJNAME=api

TARGET=csslang
PACKAGE=com$/sun$/star$/lang

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    ArrayIndexOutOfBoundsException.idl\
    ClassNotFoundException.idl\
    DisposedException.idl\
    EventObject.idl\
    IllegalAccessException.idl\
    IllegalArgumentException.idl\
    IndexOutOfBoundsException.idl\
    InvalidListenerException.idl\
    ListenerExistException.idl\
    Locale.idl\
    MultiServiceFactory.idl\
    NoSuchFieldException.idl\
    NoSuchMethodException.idl\
    NoSupportException.idl\
    NotInitializedException.idl\
    NullPointerException.idl\
    RegistryServiceManager.idl\
    ServiceManager.idl\
    ServiceNotRegisteredException.idl\
    SystemDependent.idl\
    WrappedTargetException.idl\
    WrappedTargetRuntimeException.idl\
    XComponent.idl\
    XConnectionPoint.idl\
    XConnectionPointContainer.idl\
    XEventListener.idl\
    XInitialization.idl\
    XLocalizable.idl\
    XMultiServiceFactory.idl\
    XMultiComponentFactory.idl\
    XServiceDisplayName.idl\
    XServiceInfo.idl\
    XServiceName.idl\
    XSingleServiceFactory.idl\
    XSingleComponentFactory.idl\
    XTypeProvider.idl\
    XUnoTunnel.idl\
    XMain.idl\

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
