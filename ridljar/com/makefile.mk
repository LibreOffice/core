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
