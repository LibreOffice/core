#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.17 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 16:06:40 $
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
