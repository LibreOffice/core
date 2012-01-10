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



PRJ	= ..$/..$/..$/..$/..
PRJNAME = beans
TARGET  = com_sun_star_comp_beans
PACKAGE = com$/sun$/star$/comp$/beans

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk
.IF "$(L10N_framework)"==""
.IF "$(OS)"=="MACOSX"

dummy:
    @echo "Nothing to build for OS $(OS)"

.ELSE		# "$(OS)"=="MACOSX"

JARFILES 		= ridl.jar unoil.jar jurt.jar juh.jar

# --- Sources --------------------------------------------------------

JAVAFILES=  \
    ContainerFactory.java \
    Controller.java \
    Frame.java \
    HasConnectionException.java \
    InvalidArgumentException.java \
    JavaWindowPeerFake.java \
    LocalOfficeConnection.java \
    LocalOfficeWindow.java \
    NativeConnection.java \
    NativeService.java \
    NoConnectionException.java \
    NoDocumentException.java \
    OfficeConnection.java \
    OfficeDocument.java \
    OfficeWindow.java \
    OOoBean.java \
    SystemWindowException.java \
    Wrapper.java \
    CallWatchThread.java

JAVACLASSFILES= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

.ENDIF

# --- Targets ------------------------------------------------------
.ENDIF # L10N_framework

.INCLUDE :  target.mk
