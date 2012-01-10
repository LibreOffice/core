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



PRJ := ..$/..$/..$/..$/..$/..
PRJNAME := ucb
PACKAGE := com$/sun$/star$/comp$/ucb
TARGET := test_com_sun_star_comp_ucb

.INCLUDE : settings.mk

JAVAFILES = $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES)))

CLASSDIR !:= $(CLASSDIR)$/test

JARFILES = OOoRunner.jar \
           ridl.jar \
           unoil.jar \
           juh.jar \
           jurt.jar

JAVACLASSFILES = $(CLASSDIR)$/$(PACKAGE)$/GlobalTransfer_Test.class

.INCLUDE: target.mk

$(JAVAFILES): $(MISC)$/$(TARGET).createdclassdir

$(MISC)$/$(TARGET).createdclassdir:
    - $(MKDIR) $(CLASSDIR)
    $(TOUCH) $@

#ALLTAR .PHONY:
runtest:
    java -classpath $(CLASSPATH) org.openoffice.Runner -TestBase java_complex \
        -AppExecutionCommand "c:\staroffice8.m13\program\soffice -accept=socket,host=localhost,port=8100;urp;" \
        -o $(subst,$/,. $(PACKAGE)$/GlobalTransfer_Test)

