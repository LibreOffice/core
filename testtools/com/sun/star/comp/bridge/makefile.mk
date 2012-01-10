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



PRJ=..$/..$/..$/..$/..
PRJNAME=testtools
PACKAGE=com$/sun$/star$/comp$/bridge
TARGET=com_sun_star_comp_bridge


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------


JARFILES 		= ridl.jar jurt.jar juh.jar


JAVACLASSFILES= \
    $(CLASSDIR)$/$(PACKAGE)$/CurrentContextChecker.class \
    $(CLASSDIR)$/$(PACKAGE)$/TestComponent.class \
    $(CLASSDIR)$/$(PACKAGE)$/TestComponentMain.class

JAVAFILES		= $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES))) 


JARCLASSDIRS	= $(PACKAGE) test$/testtools$/bridgetest
JARTARGET		= testComponent.jar
JARCOMPRESS 	= TRUE
CUSTOMMANIFESTFILE = manifest

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk


