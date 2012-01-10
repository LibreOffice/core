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



PRJ = ../../../..
TARGET  = TransientDocument
PRJNAME = $(TARGET)
PACKAGE = complex/tdoc/interfaces

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk


#----- compile .java files -----------------------------------------

JARFILES = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunner.jar
JAVAFILES = _XChild.java                         \
            _XCommandInfoChangeNotifier.java     \
            _XCommandProcessor.java              \
            _XComponent.java                     \
            _XContent.java                       \
            _XPropertiesChangeNotifier.java      \
            _XPropertyContainer.java             \
            _XPropertySetInfoChangeNotifier.java \
            _XServiceInfo.java                   \
            _XTypeProvider.java

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS  = $(PACKAGE)
JARTARGET     = $(TARGET).jar
JARCOMPRESS   = TRUE

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


