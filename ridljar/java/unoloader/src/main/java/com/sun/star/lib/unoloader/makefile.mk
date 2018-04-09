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



PRJ := ..$/..$/..$/..$/..$/..$/..
PRJNAME := ridljar

TARGET := unoloader_cssl_unoloader
PACKAGE := com$/sun$/star$/lib$/unoloader

.INCLUDE: settings.mk

# Use a non-default CLASSDIR, so that the util/makefile.mk can use
# JARCLASSDIRS=com and nevertheless not include this package in ridl.jar:
CLASSDIR != $(OUT)$/class$/unoloader

JAVAFILES = \
    UnoClassLoader.java \
    UnoLoader.java

JARTARGET = unoloader.jar
JARCLASSDIRS = $(PACKAGE)
CUSTOMMANIFESTFILE = manifest

.INCLUDE: target.mk

.IF "$(SOLAR_JAVA)" != ""

.IF "$(depend)" == ""
$(JAVACLASSFILES): $(MISC)$/$(TARGET).mkdir.done
.ENDIF

$(MISC)$/$(TARGET).mkdir.done .ERRREMOVE:
    $(MKDIRHIER) $(CLASSDIR)
    $(TOUCH) $@

.ENDIF
