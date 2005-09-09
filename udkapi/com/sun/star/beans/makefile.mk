#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 14:58:54 $
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
