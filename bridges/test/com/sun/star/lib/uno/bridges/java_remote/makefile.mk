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



PRJ := ..$/..$/..$/..$/..$/..$/..$/..
PRJNAME := bridges
TARGET := test_com_sun_star_lib_uno_bridges_javaremote

PACKAGE := com$/sun$/star$/lib$/uno$/bridges$/javaremote
JAVATESTFILES := \
    Bug51323_Test.java \
    Bug92174_Test.java \
    Bug97697_Test.java \
    Bug98508_Test.java \
    Bug107753_Test.java \
    Bug108825_Test.java \
    Bug110892_Test.java \
    Bug111153_Test.java \
    Bug114133_Test.java \
    MethodIdTest.java \
    PolyStructTest.java \
    StopMessageDispatcherTest.java
IDLTESTFILES := \
    Bug98508_Test.idl \
    PolyStructTest.idl
JARFILES := juh.jar jurt.jar ridl.jar

.INCLUDE: javaunittest.mk
