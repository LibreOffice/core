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



PRJ		= ..
PRJNAME	= jurt
TARGET	= jurt

# --- Settings -----------------------------------------------------

.INCLUDE: makefile.pmk

JARCLASSDIRS	= com
JARTARGET		= $(TARGET).jar
JARCOMPRESS 	= TRUE
JARCLASSPATH = $(JARFILES) ../../lib/ ../bin/
CUSTOMMANIFESTFILE = manifest

# Special work necessary for building java reference with javadoc.
# The source of puplic APIs must be delivered and used later in the
# odk module.
ZIP1TARGET=$(TARGET)_src
ZIP1FLAGS=-u -r
ZIP1DIR=$(PRJ)
ZIP1LIST=com -x "*makefile.mk"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
