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

TARGET=csscontainer
PACKAGE=com$/sun$/star$/container

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    XIdentifierAccess.idl\
    XIdentifierReplace.idl\
    XIdentifierContainer.idl\
    ContainerEvent.idl\
    ElementExistException.idl\
    EnumerableMap.idl\
    NoSuchElementException.idl\
    XChild.idl\
    XUniqueIDAccess.idl\
    XComponentEnumeration.idl\
    XComponentEnumerationAccess.idl\
    XContainer.idl\
    XContainerApproveBroadcaster.idl\
    XContainerApproveListener.idl\
    XContainerListener.idl\
    XContainerQuery.idl\
    XContentEnumerationAccess.idl\
    XElementAccess.idl\
    XEnumerableMap.idl\
    XEnumeration.idl\
    XEnumerationAccess.idl\
    XHierarchicalName.idl\
    XHierarchicalNameAccess.idl\
    XHierarchicalNameReplace.idl\
    XHierarchicalNameContainer.idl\
    XImplicitIDAccess.idl\
    XImplicitIDContainer.idl\
    XImplicitIDReplace.idl\
    XIndexAccess.idl\
    XIndexContainer.idl\
    XIndexReplace.idl\
    XMap.idl\
    XNameAccess.idl\
    XNameContainer.idl\
    XNamed.idl\
    XNameReplace.idl\
    XSet.idl\
    XStringKeyMap.idl\

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
