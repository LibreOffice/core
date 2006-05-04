#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2006-05-04 08:03:26 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2006 by Sun Microsystems, Inc.
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
