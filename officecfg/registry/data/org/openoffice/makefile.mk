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


PRJ=..$/..$/..$/..

PRJNAME=officecfg
TARGET=data_oopenoffice
PACKAGE=org.openoffice

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Targets ------------------------------------------------------

XCUFILES= \
    FirstStartWizard.xcu \
    Inet.xcu \
    Interaction.xcu \
    Setup.xcu \
    System.xcu \
    UserProfile.xcu \
    VCL.xcu \


MODULEFILES= \
    Inet-macosx.xcu \
    Inet-unixdesktop.xcu \
    Inet-wnt.xcu \
    Setup-brand.xcu \
    Setup-writer.xcu   \
    Setup-calc.xcu   \
    Setup-draw.xcu   \
    Setup-impress.xcu   \
    Setup-base.xcu   \
    Setup-math.xcu \
    Setup-report.xcu \
    Setup-start.xcu \
    UserProfile-gconflockdown.xcu \
    VCL-gconflockdown.xcu \
    VCL-unixdesktop.xcu

LOCALIZEDFILES=Setup.xcu

.INCLUDE :  target.mk

LANGUAGEPACKS=$(MISC)$/registry$/spool$/Langpack-{$(alllangiso)}.xcu

ALLTAR : $(LANGUAGEPACKS)
