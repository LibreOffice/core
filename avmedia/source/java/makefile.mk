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



# Builds the Java Canvas implementation.

PRJNAME = avmedia
PRJ     = ..$/..
TARGET  = avmedia
PACKAGE = avmedia

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

.IF "$(GUIBASE)"=="javamedia"

JAVAFILES  = \
    Manager.java            \
    Player.java             \
    PlayerWindow.java       \
    WindowAdapter.java      \
    MediaUno.java           \
    FrameGrabber.java       \
    x11$/SystemWindowAdapter.java   

JARFILES        = jurt.jar unoil.jar ridl.jar juh.jar java_uno.jar jmf.jar
JAVACLASSFILES  = $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:s/.java//).class)

JARTARGET               = $(TARGET).jar
JARCOMPRESS             = TRUE
CUSTOMMANIFESTFILE      = manifest

.ENDIF     # "$(GUIBASE)"=="javamedia" 

# --- Targets ------------------------------------------------------

.INCLUDE: target.mk

ALLTAR : $(MISC)/avmedia.jar.component

$(MISC)/avmedia.jar.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt avmedia.jar.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_JAVA)avmedia.jar' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt avmedia.jar.component
