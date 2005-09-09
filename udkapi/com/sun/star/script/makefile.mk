#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 16:22:44 $
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
    XAllListener.idl\
    XAllListenerAdapterService.idl\
    XDebugging.idl\
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
    XLibraryContainer.idl\
    XLibraryContainer2.idl\
    XLibraryContainerPassword.idl\
    XScriptEventsSupplier.idl\
    XScriptEventsAttacher.idl\

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
