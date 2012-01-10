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
    Pair.idl\
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
