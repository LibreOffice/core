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



PRJ=..$/..

PRJNAME=ucbhelper
TARGET=ucbexplorer
TARGETTYPE=GUI
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings ---

.INCLUDE : settings.mk

# --- Files ---

OBJFILES=$(OBJ)$/$(TARGET).obj

SRS1NAME=$(TARGET)
SRC1FILES=$(TARGET).src

#
# UCBEXPLORER
#

APP1TARGET=$(TARGET)

RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES=$(SRS)$/$(TARGET).srs

APP1OBJS=$(OBJ)$/$(TARGET).obj
APP1RES=$(RES)$/$(TARGET).res

APP1STDLIBS=$(SALLIB) 	 \
        $(CPPULIB) 	 	 \
        $(CPPUHELPERLIB) \
        $(TOOLSLIB)    	 \
        $(SVTOOLLIB)	 \
        $(COMPHELPERLIB) \
        $(UCBHELPERLIB)  \
        $(VCLLIB)

# --- Targets ---

.INCLUDE : target.mk

