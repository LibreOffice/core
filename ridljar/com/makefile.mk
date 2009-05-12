#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.3 $
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

PRJ := ..
PRJNAME := ridljar
TARGET := com

PACKAGE := com

.INCLUDE: settings.mk

JAVAFILES = \
    sun$/star$/lib$/uno$/typedesc$/FieldDescription.java \
    sun$/star$/lib$/uno$/typedesc$/MemberDescriptionHelper.java \
    sun$/star$/lib$/uno$/typedesc$/MethodDescription.java \
    sun$/star$/lib$/uno$/typedesc$/TypeDescription.java \
    sun$/star$/lib$/uno$/typeinfo$/AttributeTypeInfo.java \
    sun$/star$/lib$/uno$/typeinfo$/ConstantTypeInfo.java \
    sun$/star$/lib$/uno$/typeinfo$/MemberTypeInfo.java \
    sun$/star$/lib$/uno$/typeinfo$/MethodTypeInfo.java \
    sun$/star$/lib$/uno$/typeinfo$/ParameterTypeInfo.java \
    sun$/star$/lib$/uno$/typeinfo$/TypeInfo.java \
    sun$/star$/lib$/util$/DisposeListener.java \
    sun$/star$/lib$/util$/DisposeNotifier.java \
    sun$/star$/lib$/util$/WeakMap.java \
    sun$/star$/uno$/Any.java \
    sun$/star$/uno$/Enum.java \
    sun$/star$/uno$/IBridge.java \
    sun$/star$/uno$/IEnvironment.java \
    sun$/star$/uno$/IFieldDescription.java \
    sun$/star$/uno$/IMapping.java \
    sun$/star$/uno$/IMemberDescription.java \
    sun$/star$/uno$/IMethodDescription.java \
    sun$/star$/uno$/IQueryInterface.java \
    sun$/star$/uno$/ITypeDescription.java \
    sun$/star$/uno$/Type.java \
    sun$/star$/uno$/Union.java \
    sun$/star$/uno$/UnoRuntime.java

.INCLUDE: target.mk
