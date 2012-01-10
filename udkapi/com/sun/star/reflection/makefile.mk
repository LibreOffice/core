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

TARGET=cssreflection
PACKAGE=com$/sun$/star$/reflection

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    CoreReflection.idl\
    FieldAccessMode.idl\
    InvalidTypeNameException.idl\
    InvocationTargetException.idl\
    MethodMode.idl\
    NoSuchTypeNameException.idl\
    ParamInfo.idl\
    ParamMode.idl\
    ProxyFactory.idl\
    TypeDescriptionManager.idl\
    TypeDescriptionProvider.idl\
    TypeDescriptionSearchDepth.idl\
    XArrayTypeDescription.idl\
    XCompoundTypeDescription.idl\
    XConstantTypeDescription.idl\
    XConstantsTypeDescription.idl\
    XUnionTypeDescription.idl\
    XEnumTypeDescription.idl\
    XIdlArray.idl\
    XIdlClass.idl\
    XIdlClassProvider.idl\
    XIdlField.idl\
    XIdlField2.idl\
    XIdlMember.idl\
    XIdlMethod.idl\
    XIdlReflection.idl\
    XIndirectTypeDescription.idl\
    XInterfaceAttributeTypeDescription.idl\
    XInterfaceAttributeTypeDescription2.idl \
    XInterfaceMemberTypeDescription.idl\
    XInterfaceMethodTypeDescription.idl\
    XInterfaceTypeDescription.idl\
    XInterfaceTypeDescription2.idl \
    XMethodParameter.idl\
    XModuleTypeDescription.idl\
    XParameter.idl \
    XPropertyTypeDescription.idl\
    XProxyFactory.idl\
    XPublished.idl \
    XServiceConstructorDescription.idl \
    XServiceTypeDescription.idl\
    XServiceTypeDescription2.idl \
    XSingletonTypeDescription.idl\
    XSingletonTypeDescription2.idl \
    XStructTypeDescription.idl \
    XTypeDescription.idl\
    XTypeDescriptionEnumeration.idl\
    XTypeDescriptionEnumerationAccess.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
