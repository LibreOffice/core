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



PRJNAME	= accessibility
PRJ		= ..$/..$/..$/..
TARGET	= java_uno_accessbridge
PACKAGE	= org$/openoffice$/accessibility

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

JARFILES = jurt.jar unoil.jar ridl.jar 
JAVAFILES = \
    AccessBridge.java \
    KeyHandler.java \
    PopupWindow.java \
    WindowsAccessBridgeAdapter.java

JAVACLASSFILES= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

JARTARGET               = $(TARGET).jar
JARCOMPRESS             = TRUE
JARCLASSDIRS            = $(PACKAGE) org/openoffice/java/accessibility 
CUSTOMMANIFESTFILE      = manifest

# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk

ALLTAR : $(MISC)/java_uno_accessbridge.component

$(MISC)/java_uno_accessbridge.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt java_uno_accessbridge.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_JAVA)$(JARTARGET)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt java_uno_accessbridge.component
