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

TARGET=cssscript
PACKAGE=com$/sun$/star$/script

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    AllEventObject.idl\
    AllListenerAdapter.idl\
    BasicErrorException.idl\
    CannotConvertException.idl\
    CannotCreateAdapterException.idl\
    ContextInformation.idl\
    Converter.idl\
    Engine.idl\
    FailReason.idl\
    FinishEngineEvent.idl\
    FinishReason.idl\
    InterruptEngineEvent.idl\
    InterruptReason.idl\
    Invocation.idl\
    InvocationAdapterFactory.idl\
    JavaScript.idl\
    ScriptEvent.idl\
    ScriptEventDescriptor.idl\
    ArrayWrapper.idl\
    XAllListener.idl\
    XAllListenerAdapterService.idl\
    XDebugging.idl\
    XDirectInvocation.idl\
    XEngine.idl\
    XEngineListener.idl\
    XEventAttacher.idl\
    XEventAttacherManager.idl\
    XInvocationAdapterFactory.idl\
    XInvocationAdapterFactory2.idl\
    XInvocation.idl\
    XInvocation2.idl\
    InvocationInfo.idl\
    MemberType.idl\
    XLibraryAccess.idl\
    XScriptListener.idl\
    XTypeConverter.idl\
    XStarBasicAccess.idl\
    XStarBasicModuleInfo.idl\
    XStarBasicDialogInfo.idl\
    XStarBasicLibraryInfo.idl\
    XScriptEventsSupplier.idl\
    XScriptEventsAttacher.idl\
    XDefaultMethod.idl\
    XDefaultProperty.idl\
	XAutomationInvocation.idl\
    XErrorQuery.idl\

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
