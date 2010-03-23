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
