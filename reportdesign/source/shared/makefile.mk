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
PRJINC=$(PRJ)$/source
PRJNAME=reportdesign
LIBTARGET=NO
TARGET=shared
LIB1TARGET=$(SLB)$/coreshared.lib
LIB2TARGET=$(SLB)$/xmlshared.lib
LIB3TARGET=$(SLB)$/uishared.lib

#ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Types -------------------------------------

# --- Files -------------------------------------
    
SLOFILES=	\
        $(SLO)$/corestrings.obj		\
        $(SLO)$/uistrings.obj		\
        $(SLO)$/xmlstrings.obj		\


LIB1OBJFILES=	\
        $(SLO)$/corestrings.obj


LIB2OBJFILES=	\
        $(SLO)$/xmlstrings.obj
        
LIB3OBJFILES=	\
        $(SLO)$/uistrings.obj		
        

# --- Targets ----------------------------------

.INCLUDE : target.mk

