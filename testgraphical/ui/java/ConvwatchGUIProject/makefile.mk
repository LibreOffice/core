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



PRJ=../../..

PRJNAME=gfxcmp_ui_java_convwatchgui
TARGET=notargetyet

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- ANT build environment  ---------------------------------------

.INCLUDE : antsettings.mk

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(SHOW)" == ""
nothing .PHONY:

.ELSE


ALLTAR: $(CLASSDIR)/ConvwatchGUIProject.jar

$(CLASSDIR)/ConvwatchGUIProject.jar: src/ConvwatchGUI.java src/IniFile.java
# .if $(JDK_VERSION) < 160
# 	echo "You need at least java 6"
# 	error
# .endif
#
.IF "$(GUI)"=="WNT"
    $(ANT) -DJAR_OUTPUT_DIRECTORY=$(CLASSDIR) jar
.ELSE
    ant
.END

.END

.INCLUDE : $(PRJ)$/util$/makefile.pmk

clean:
    $(ANT) clean

