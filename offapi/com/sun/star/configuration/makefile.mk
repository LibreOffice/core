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

TARGET=cssconfiguration
PACKAGE=com$/sun$/star$/configuration

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    AdministrationProvider.idl \
    DefaultProvider.idl \
    CannotLoadConfigurationException.idl \
    CorruptedConfigurationException.idl \
    CorruptedUIConfigurationException.idl \
    InvalidBootstrapFileException.idl \
    MissingBootstrapFileException.idl \
    InstallationIncompleteException.idl \
    ConfigurationAccess.idl\
    ConfigurationProvider.idl\
    ConfigurationUpdateAccess.idl\
    ConfigurationRegistry.idl\
    HierarchyElement.idl\
    AccessRootElement.idl\
    UpdateRootElement.idl\
    GroupElement.idl\
    SetElement.idl\
    HierarchyAccess.idl\
    GroupAccess.idl\
    GroupUpdate.idl\
    SetAccess.idl\
    SetUpdate.idl\
    SimpleSetAccess.idl\
    SimpleSetUpdate.idl\
    PropertyHierarchy.idl\
    XTemplateContainer.idl\
    XTemplateInstance.idl\
    Update.idl \
    XUpdate.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
