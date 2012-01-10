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



PRJNAME	= setup_native
PRJ		= ..$/..$/..$/..$/..$/..
TARGET	= servicetag
PACKAGE	= com$/sun$/star$/servicetag

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(ENABLE_SVCTAGS)" == "YES"

JARFILES = jurt.jar unoil.jar ridl.jar 
JAVAFILES = \
    BrowserSupport.java \
    Installer.java \
    LinuxSystemEnvironment.java \
    RegistrationData.java \
    RegistrationDocument.java \
    Registry.java \
    ServiceTag.java \
    SolarisServiceTag.java \
    SolarisSystemEnvironment.java \
    SunConnection.java \
    SysnetRegistryHelper.java \
    SystemEnvironment.java \
    UnauthorizedAccessException.java \
    Util.java \
    WindowsSystemEnvironment.java

JAVACLASSFILES= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

JARTARGET               = $(TARGET).jar
JARCOMPRESS             = TRUE
JARCLASSDIRS            = $(PACKAGE)

JAVARES= $(CLASSDIR)$/$(PACKAGE)$/resources$/product_registration.xsd

.ENDIF			# "$(ENABLE_SVCTAGS)" == "YES"

# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk

.IF "$(ENABLE_SVCTAGS)" == "YES"
ALLTAR: $(JAVARES)

$(JAVARES) : $$(@:d:d:f)$/$$(@:f)
    $(MKDIRHIER) $(@:d)
    $(COPY) $< $@

.ENDIF			# "$(ENABLE_SVCTAGS)" == "YES"
