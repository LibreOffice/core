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



.IF "$(OOO_SUBSEQUENT_TESTS)" == ""
nothing .PHONY:
.ELSE

PRJ := ..$/..$/..$/..$/..$/..$/..$/..
PRJNAME := jurt
TARGET := test_com_sun_star_lib_uno_protocols_urp

.IF "$(OOO_JUNIT_JAR)" != ""
PACKAGE := com$/sun$/star$/lib$/uno$/protocols$/urp
JAVATESTFILES := \
    Cache_Test.java \
    Marshaling_Test.java \
    Protocol_Test.java
JAVAFILES := \
    TestBridge.java \
    TestObject.java
JARFILES := ridl.jar
IDLTESTFILES := interfaces.idl
.END

.INCLUDE: javaunittest.mk

.END
