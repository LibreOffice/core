#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: rt $ $Date: 2008-01-30 09:34:08 $
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
