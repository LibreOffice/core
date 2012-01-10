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



PRJ := ..$/..$/..
PRJNAME := bean
TARGET := oooapplet
PACKAGE = oooapplet

.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES = officebean.jar ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar 
JAVAFILES       = OOoViewer.java
JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)


JARCLASSDIRS    = \
    oooapplet

JARTARGET       = $(TARGET).jar
JARCOMPRESS     = TRUE

#----- make a jar from compiled files ------------------------------


.INCLUDE: target.mk


ALLTAR : \
    COPY_FILES \
    RUNINSTRUCTIONS


COPY_FILES: example.html 
    $(GNUCOPY) -p $< $(CLASSDIR)
# --- Targets ------------------------------------------------------


.IF "$(GUI)"=="WNT"
RUN:
    firefox "$(CLASSDIR)$/example.html?$(office)"
.ELSE
TESTURL="file:///$(PWD)$/$(CLASSDIR)$/example.html?$(office)"
RUN:
    firefox ${TESTURL:s/\///}
.ENDIF

run: RUN



RUNINSTRUCTIONS : 
    @echo .
    @echo ###########################   N O T E  ######################################
    @echo . 
    @echo "Add to the java runtime settings for applets in the control panel these lines:"
    @echo "-Djava.security.policy=$(PWD)$/bean.policy"
    @echo "To run the test you have to provide the office location."
    @echo Example:
    @echo dmake run office="d:\\myOffice"
    @echo .
  

