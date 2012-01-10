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

TARGET=cssbridge
PACKAGE=com$/sun$/star$/bridge

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    ModelDependent.idl\
    OleApplicationRegistration.idl\
    OleBridgeSupplier.idl\
    OleBridgeSupplier2.idl\
    OleBridgeSupplierVar1.idl\
    OleObjectFactory.idl\
    XBridgeSupplier.idl\
    XBridgeSupplier2.idl\
    XBridge.idl \
    XBridgeFactory.idl\
    XInstanceProvider.idl\
    BridgeExistsException.idl \
    XUnoUrlResolver.idl \
    UnoUrlResolver.idl \
    XProtocolProperties.idl \
    ProtocolProperty.idl \
    InvalidProtocolChangeException.idl \
    Bridge.idl \
    BridgeFactory.idl \
    UrpBridge.idl \
    IiopBridge.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
