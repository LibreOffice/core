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
# $Revision: 1.13 $
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

TARGET=cssbeans
PACKAGE=com$/sun$/star$/beans

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    Ambiguous.idl\
    Defaulted.idl\
    GetDirectPropertyTolerantResult.idl\
    GetPropertyTolerantResult.idl\
    IllegalTypeException.idl\
    Introspection.idl\
    IntrospectionException.idl\
    MethodConcept.idl\
    NamedValue.idl\
    NotRemoveableException.idl\
    Optional.idl\
    Property.idl\
    PropertyAttribute.idl\
    PropertyBag.idl\
    PropertyChangeEvent.idl\
    PropertyConcept.idl\
    PropertyExistException.idl\
    PropertySet.idl\
    PropertySetInfoChange.idl\
    PropertySetInfoChangeEvent.idl\
    PropertyState.idl\
    PropertyStateChangeEvent.idl\
    PropertyValue.idl\
    PropertyValues.idl\
    PropertyVetoException.idl\
    SetPropertyTolerantFailed.idl\
    StringPair.idl\
    TolerantPropertySetResultType.idl\
    UnknownPropertyException.idl\
    XExactName.idl\
    XFastPropertySet.idl\
    XHierarchicalPropertySet.idl\
    XHierarchicalPropertySetInfo.idl\
    XIntrospection.idl\
    XIntrospectionAccess.idl\
    XIntroTest.idl\
    XMaterialHolder.idl\
    XMultiPropertySet.idl\
    XMultiPropertyStates.idl\
    XMultiHierarchicalPropertySet.idl\
    XPropertiesChangeListener.idl\
    XPropertiesChangeNotifier.idl\
    XProperty.idl\
    XPropertyAccess.idl\
    XPropertyChangeListener.idl\
    XPropertyContainer.idl\
    XPropertySet.idl\
    XPropertySetInfo.idl\
    XPropertySetInfoChangeListener.idl\
    XPropertySetInfoChangeNotifier.idl\
    XPropertyState.idl\
    XPropertyStateChangeListener.idl\
    XPropertyWithState.idl\
    XTolerantMultiPropertySet.idl\
    XVetoableChangeListener.idl\

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
