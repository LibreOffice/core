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



PRJ = ..$/..$/..$/..
PRJNAME = JobExecutor
TARGET  = $(PRJNAME)
PACKAGE = test

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES        = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunner.jar
JAVAFILES       = Job.java
JAVACLASSFILES  = $(CLASSDIR)$/$(PACKAGE)$/Job.class

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS    = test
JARTARGET       = $(TARGET).jar
JARCOMPRESS 	= TRUE
CUSTOMMANIFESTFILE = manifest


# --- Files --------------------------------------------------------

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL : \
    ALLTAR
.ELSE
ALL: 	ALLDEP
.ENDIF

.INCLUDE :  target.mk

