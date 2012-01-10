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

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
